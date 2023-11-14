#include "interpreter.h"
#include "variables.h"
#include "tokens.h"
#include "symbols.h"

__striped Value		estack[32];
char				esp;

struct StmtStack
{
	const char * tk;
}	stmstack[16];

char ssp;

struct CallStack
{
	StmtStack		smts[16];
	char			ssp;
	const char	*	tk;
	char			ti;
}	callstack[16];

char csp;

char * prepare_expression(char * tk, bool var)
{
	char 	t = *tk;
	bool	left = true;
	while (t != TK_END)
	{
		switch (t & 0xf0)
		{
		case TK_SMALL_INT:
			tk += 2;
			left = false;
			break;
		case TK_NUMBER:
			tk += 5;
			left = false;
			break;

		case TK_IDENT:
		case TK_CONST:	
		case TK_GLOBAL:	
		case TK_LOCAL:
			{
				char tt = t & 0xf0;
				unsigned ti = ((t & 0x0f) << 8) | tk[1];
				if (tt == TK_IDENT)
				{
					if (left && var)
						global_add(ti);

					unsigned	vi = global_find(ti);
					if (vi < num_globals)
					{
						tk[0] = (vi >> 8) | TK_GLOBAL;
						tk[1] = vi & 0xff;
					}
				}
			tk += 2;
			left = false;
			}	break;

		case TK_POSTFIX:
		case TK_STRUCTURE:
			tk += 2;
			left = false;
			break;

		case TK_CONTROL:
			{
				switch (t)
				{
				case TK_COMMA:
					tk++;
					left = true;
					break;
				case TK_STRING:
					tk += tk[1] + 2;
					left = false;
					break;
				default:
					tk++;
				}
			} break;

		default:
			tk++;
			left = false;
		}
		t = *tk;
	}
	return tk + 1;
}

char * prepare_function(char * tk)
{
	unsigned ti = ((tk[0] & 0x0f) << 8) | tk[1];
	unsigned vi = global_add(ti);

	globals[vi].v.type = TYPE_FUNCTION;
	globals[vi].v.value = unsigned(tk + 2);
	while ((tk[0] & 0xf0) == TK_IDENT)
		tk += 2;
	return tk + 2;
}

void prepare_statements(char * tk)
{
	exectk = tk;
	ssp = 0;
	esp = 0;
	csp = 0;
	num_globals = 0;

	char 		pl = 1;
	char 		l = *tk;
	char 	*	pt = nullptr;

	while (l > 0)
	{		
		while (l < pl)
		{
			char * ppt = (char *)(pt[0] + (pt[1] << 8));
			pt[0] = (unsigned)tk & 0xff;
			pt[1] = (unsigned)tk >> 8;
			pt = ppt;
			pl--;
		}

		tk++;
		char 	t = *tk++;
		switch (t)
		{
		case STMT_EXPRESSION:
		case STMT_RETURN:
			tk = prepare_expression(tk, false);
			break;
		case STMT_VAR:
			tk = prepare_expression(tk, true);
			break;
		case STMT_DEF:
			tk[0] = (unsigned)pt & 0xff;
			tk[1] = (unsigned)pt >> 8;
			pt = tk;
			tk = prepare_function(tk + 2);
			pl++;
			break;		
		case STMT_NONE:
			break;
		case STMT_WHILE:
		case STMT_IF:
		case STMT_ELSIF:
			tk[0] = (unsigned)pt & 0xff;
			tk[1] = (unsigned)pt >> 8;
			pt = tk;
			tk = prepare_expression(tk + 2, false);
			pl++;
			break;		
		case STMT_ELSE:
			tk[0] = (unsigned)pt & 0xff;
			tk[1] = (unsigned)pt >> 8;
			pt = tk;
			tk += 2;
			pl++;
			break;					
		}
		l = *tk;
	}
}

char * restore_expression(char * tk, bool var)
{
	char 	t = *tk;
	while (t != TK_END)
	{
		switch (t & 0xf0)
		{
		case TK_SMALL_INT:
			tk += 2;
			break;
		case TK_NUMBER:
			tk += 5;
			break;

		case TK_IDENT:
		case TK_CONST:	
		case TK_GLOBAL:	
		case TK_LOCAL:
			{
				char tt = t & 0xf0;
				unsigned ti = ((t & 0x0f) << 8) | tk[1];
				if (tt == TK_GLOBAL)
				{
					unsigned vi = globals[ti].symbol;
					tk[0] = (vi >> 8) | TK_IDENT;
					tk[1] = vi & 0xff;
				}
			tk += 2;
			}	break;

		case TK_POSTFIX:
		case TK_STRUCTURE:
			tk += 2;
			break;

		case TK_CONTROL:
			{
				switch (t)
				{
				case TK_STRING:
					tk += tk[1] + 2;
					break;
				default:
					tk++;
				}
			} break;

		default:
			tk++;
		}
		t = *tk;
	}
	return tk + 1;
}

char * restore_function(char * tk)
{
	while ((tk[0] & 0xf0) == TK_IDENT)
		tk += 2;
	return tk + 2;
}


void restore_statements(char * tk)
{
	char	l = *tk;
	while (l > 0)
	{		
		tk++;
		char 	t = *tk++;
		switch (t)
		{
		case STMT_EXPRESSION:
			tk = restore_expression(tk, false);
			break;
		case STMT_VAR:
			tk = restore_expression(tk, true);
			break;
		case STMT_NONE:
			break;
		case STMT_WHILE:
		case STMT_IF:
		case STMT_ELSIF:
			tk[0] = 0;
			tk[1] = 0;
			tk = restore_expression(tk + 2, false);
			break;		
		case STMT_ELSE:
			tk[0] = 0;
			tk[1] = 0;
			tk += 2;
			break;			
		case STMT_DEF:
			tk[0] = 0;
			tk[1] = 0;
			tk = restore_function(tk + 2);
			break;				
		}
		l = *tk;
	}	
}

void valderef(char at)
{
	char ei = esp - at - 1;
	switch (estack[ei].type)
	{
	case TYPE_GLOBAL_REF:
		estack[ei] = globals[(char)(estack[ei].value)].v;
		break;
	}
}

bool boolpop(void)
{
	valderef(0);
	esp--;
	return estack[esp].value != 0;
}

long valpop(void)
{
	esp--;
	return estack[esp].value;
}

long valget(char at)
{
	char ei = esp - at - 1;
	return estack[ei].value;
}

inline char typeget(char at)
{
	char ei = esp - at - 1;
	return estack[ei].type;
}

void valpush(char type, long value)
{
	estack[esp].type = type;
	estack[esp].value = value;
	esp++;
}

void valassign(void)
{
	esp--;
	switch (estack[esp - 1].type)
	{
	case TYPE_GLOBAL_REF:
		globals[(char)(estack[esp - 1].value)].v = estack[esp];
		break;
	}
}

void valswap(void)
{
	Value	es = estack[esp - 1];
	estack[esp - 1] = estack[esp - 2];
	estack[esp - 2] = es;
}

const char * exectk;

void interpret_builtin(char n)
{
	long lf = valget(n);
	switch (lf)
	{
	case RTSYM_ABS:
		{
			valderef(0);
			long li = valpop();
			esp -= n;
			valpush(TYPE_NUMBER, labs(li));
		} break;
	case RTSYM_CHROUT:
		{
			valderef(0);
			long li = valpop();
			esp -= n;
			putch(li >> 16);
			valpush(TYPE_NULL, 0);
		} break;		
	case RTSYM_RAND:
		esp -= n + 1;
		valpush(TYPE_NUMBER, rand());
		break;
	case RTSYM_PRINT:
		{
			for(char i=0; i<n; i++)
			{
				char k = n - i - 1;
				valderef(k);
				switch (typeget(k))
				{
				case TYPE_NUMBER:
					{
						char 	str[20];
						long	s = valget(k);
						if (s < 0)
						{
							putch('-');
							s = -s;
						}
						unsigned	hi = (unsigned long)s >> 16;
						if (hi == 0)
							putch('0');
						else
						{
							char i = 0;
							while (hi)
							{
								str[i] = hi % 10 + '0';
								hi /= 10;
								i++;
							}
							while (i > 0)
							{
								i--;
								putch(str[i]);
							}
						}
						unsigned long l = s & 0xffff;
						if (l)
						{
							str[0] = '.';
							char j = 1;
							for(char i=0; i<4; i++)
							{
								l *= 10;
								str[j] = char(l >> 16) + '0';
								j++;
								l &= 0xfffful;
							}
							while (str[j-1] == '0')
								j--;
							if (str[j-1] == '.')
								j--;
							char i = 0;
							while (i < j)
							{
								putch(str[i]);
								i++;
							}
						}

					} break;
				case TYPE_STRING:
					{
						const char * str = (const char *)valget(k);
						char n = str[0];
						for(char i=0; i<n; i++)
							putch(str[i + 1]);
					} break;
				}
			}
			esp -= n + 1;
			valpush(TYPE_NULL, 0);
		} break;
	}
}

bool interpret_expression(void)
{
	const char * tk = exectk;

	char	ti = 0;
	for(;;)
	{
		char 	t = tk[ti++];
		switch (t & 0xf0)
		{
		case TK_TINY_INT:
			valpush(TYPE_NUMBER, (unsigned long)(t & 0x0f) << 16);
			break;
		case TK_SMALL_INT:
			valpush(TYPE_NUMBER, (unsigned long)(((t & 0x0f) << 8) | tk[ti++]) << 16);
			break;
		case TK_NUMBER:
		{
			unsigned long l = tk[ti++];
			l <<= 8;
			l |= tk[ti++];
			l <<= 8;
			l |= tk[ti++];
			l <<= 8;
			l |= tk[ti++];
			valpush(TYPE_NUMBER, l);
		}	break;

		case TK_IDENT:
			{
				unsigned tv = ((t & 0x0f) << 8) | tk[ti++];
				valpush(TYPE_SYMBOL, tv);
			} break;
		case TK_CONST:	
		case TK_LOCAL:
			break;

		case TK_GLOBAL:	
			{
				unsigned tv = ((t & 0x0f) << 8) | tk[ti++];
				valpush(TYPE_GLOBAL_REF, tv);
			} break;

		case TK_BINARY:
			{
				valderef(0); 
				valderef(1);

				long l2 = valpop();
				long l1 = valpop();

				switch (t)
				{
				case TK_ADD:
					l1 += l2;
					break;
				case TK_SUB:
					l1 -= l2;
					break;
				case TK_MUL:
					l1 *= l2;
					break;
				case TK_DIV:
					l1 /= l2;
					break;
				case TK_MOD:
					l1 = (unsigned long)((unsigned)(l1 >> 16) % (unsigned)(l2 >> 16)) << 16;
					break;
				case TK_SHL:
					l1 <<= unsigned(l2 >> 16);
					break;
				case TK_SHR:
					l1 >>= unsigned(l2 >> 16);
					break;
				case TK_AND:
					l1 = (unsigned long)((unsigned)(l1 >> 16) & (unsigned)(l2 >> 16)) << 16;
					break;
				case TK_OR:
					l1 = (unsigned long)((unsigned)(l1 >> 16) | (unsigned)(l2 >> 16)) << 16;
					break;
				}

				valpush(TYPE_NUMBER, l1);
			}	break;
		case TK_RELATIONAL:
			{
				valderef(0); 
				valderef(1);

				long l2 = valpop();
				long l1 = valpop();

				bool	b = false;

				switch (t)
				{
				case TK_EQUAL:
					if (l1 == l2) b = true;
					break;
				case TK_NOT_EQUAL:
					if (l1 != l2) b = true;
					break;
				case TK_LESS_THAN:
					if (l1 < l2) b = true;
					break;
				case TK_LESS_EQUAL:
					if (l1 <= l2) b = true;
					break;
				case TK_GREATER_THAN:
					if (l1 > l2) b = true;
					break;
				case TK_GREATER_EQUAL:
					if (l1 >= l2) b = true;
					break;	
				}

				valpush(TYPE_NUMBER, b ? 0x10000ul : 0ul);
			} break;
		case TK_PREFIX:
			switch (t)
			{
			case TK_NEGATE:
				valderef(0); 
				valpush(TYPE_NUMBER, -valpop());
				break;
			case TK_NOT:
				valderef(0); 
				valpush(TYPE_NUMBER, (unsigned long)(~(unsigned)(valpop() >> 16)) << 16);
				break;
			}
			break;

		case TK_POSTFIX:
			switch (t)
			{
			case TK_INDEX:
				break;
			case TK_DOT:
				break;
			case TK_INVOKE:
				{
					char n = tk[ti++];

					valderef(n); 
					switch (typeget(n))
					{
					case TYPE_SYMBOL:
						interpret_builtin(n);
						break;
					}
				}	break;
			}
			break;

		case TK_ASSIGN:
			{
				valderef(0);
				valassign();
			}	break;

		case TK_STRUCTURE:
			{
				char n = tk[ti++];
				switch (t)
				{
				case TK_LIST:
					if (n > 0)
					{
						estack[esp - n] = estack[esp - 1];
						esp -= n - 1;
					}
					break;
				case TK_ARRAY:
					break;
				case TK_STRUCT:
					break;
				}
			}	break;

		case TK_CONTROL:
			{
				switch (t)
				{
				case TK_END:
					exectk += ti;
					return true;
				case TK_COMMA:
					esp--;
					break;
				case TK_STRING:
					valpush(TYPE_STRING, (unsigned)(tk + ti));
					ti += tk[ti] + 1;
					break;
				}
			}
		}
	}
}

void interpret_statement(void)
{
	const char * tk = exectk;

	char 	l = tk[0] - 1;
	while (l < ssp)
	{
		tk = stmstack[--ssp].tk;
		l = tk[0] - 1;
	}

	const char * etk = tk;

	char 	t = tk[1];

	switch (t)
	{
	case STMT_NONE:
		exectk = tk + 2;
		return;
	case STMT_ELSE:
	case STMT_ELSIF:
	case STMT_DEF:
		exectk = (char *)(etk[2] + (etk[3] << 8));
		return;
	case STMT_WHILE:
	case STMT_IF:
		tk += 4;
		break;
	default:
		tk += 2;
	}

	for(;;)
	{
		exectk = tk;

		interpret_expression();

		switch (t)
		{
		case STMT_EXPRESSION:
		case STMT_VAR:
			esp--;
			return;

		case STMT_WHILE:
			if (boolpop())
			{
				stmstack[ssp].tk = etk;
				ssp++;
			}
			else
				exectk = (char *)(etk[2] + (etk[3] << 8));
			return;

		case STMT_IF:
		case STMT_ELSIF:
			if (boolpop())
				return;

			tk = (char *)(etk[2] + (etk[3] << 8));
			if (tk[0] != etk[0])
				return;

			if (tk[1] == STMT_ELSE)
			{
				tk += 4;
				return;
			}
			else if (tk[1] == STMT_ELSIF)
			{
				etk = tk;
				tk += 4;
			}
			else
				return;
		}
	}
}
	
