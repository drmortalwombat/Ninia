#include "interpreter.h"
#include "variables.h"
#include "tokens.h"
#include "symbols.h"
__striped struct EvalStack
{
	bool	lvalue;
	union {
		long		value;
		long	*	addr;
	} v;
}	estack[32];
char		esp;

struct StmtStack
{
	const char * tk;
}	stmstack[16];

char ssp;


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

		case TK_COMMA:
			tk++;
			left = true;
			break;

		default:
			tk++;
			left = false;
		}
		t = *tk;
	}
	return tk + 1;
}

void prepare_statements(char * tk)
{
	ssp = 0;
	esp = 0;
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
			tk = prepare_expression(tk, false);
			break;
		case STMT_VAR:
			tk = prepare_expression(tk, true);
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

		case TK_COMMA:
			tk++;
			break;

		default:
			tk++;
		}
		t = *tk;
	}
	return tk + 1;
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
		}
		l = *tk;
	}	
}

long rvalpop(void)
{
	esp--;
	if (estack[esp].lvalue)
		return *(estack[esp].v.addr);
	else
		return estack[esp].v.value;
}

long rvalget(char at)
{
	char ei = esp - at - 1;
	if (estack[ei].lvalue)
		return *(estack[ei].v.addr);
	else
		return estack[ei].v.value;
}

void rvalpush(long value)
{
	estack[esp].lvalue = false;
	estack[esp].v.value = value;
	esp++;
}

void valswap(void)
{
	EvalStack	es = estack[esp - 1];
	estack[esp - 1] = estack[esp - 2];
	estack[esp - 2] = es;
}

const char * interpret_expression(const char * tk)
{
	esp = 0;

	char	ti = 0;
	for(;;)
	{
		char 	t = tk[ti++];
		switch (t & 0xf0)
		{
		case TK_TINY_INT:
			rvalpush((unsigned long)(t & 0x0f) << 16);
			break;
		case TK_SMALL_INT:
			rvalpush((unsigned long)(((t & 0x0f) << 8) | tk[ti++]) << 16);
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
			rvalpush(l);
		}	break;

		case TK_IDENT:
			{
				unsigned tv = ((t & 0x0f) << 8) | tk[ti++];
				rvalpush(tv);
			} break;
		case TK_CONST:	
		case TK_LOCAL:
			break;

		case TK_GLOBAL:	
			{
				unsigned tv = ((t & 0x0f) << 8) | tk[ti++];
				estack[esp].lvalue = true;
				estack[esp].v.addr = &(globals[tv].value);
				esp++;
			} break;

		case TK_BINARY:
			{
				long l2 = rvalpop();
				long l1 = rvalpop();

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
				rvalpush(l1);
			}	break;
		case TK_RELATIONAL:
			{
				long 	l2 = rvalpop();
				long 	l1 = rvalpop();
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

				rvalpush(b ? 0x10000ul : 0ul);
			} break;
		case TK_PREFIX:
			switch (t)
			{
			case TK_NEGATE:
				rvalpush(-rvalpop());
				break;
			case TK_NOT:
				rvalpush((unsigned long)(~(unsigned)(rvalpop() >> 16)) << 16);
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

					long lf = rvalget(n);
					if (n == 1)
					{
						long	li = rvalpop();
						esp--;

						switch (lf)
						{
						case RTSYM_ABS:
							rvalpush(labs(li));
							break;
						case RTSYM_CHROUT:
							putch(li >> 16);
							rvalpush(0);
							break;
						}
					}
					else if (n == 0)
					{
						esp--;
						switch (lf)
						{
						case RTSYM_RAND:
							rvalpush(rand());
							break;
						}
					}

				}	break;
			}
			break;

		case TK_ASSIGN:
			{
				long l = rvalpop();
				*(estack[esp-1].v.addr) = l;
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
					return tk + ti;
				case TK_COMMA:
					esp--;
					break;
				}
			}
		}
	}
}

const char * interpret_statement(const char * etk)
{
	const char * tk = etk;

	char 	l = *tk - 1;
	while (l < ssp)
	{
		tk = stmstack[--ssp].tk;
		l = *tk - 1;
	}

	etk = tk;

	tk++;
	char 	t = *tk++;

	switch (t)
	{
	case STMT_EXPRESSION:
		tk = interpret_expression(tk);
		esp--;
		break;
	case STMT_VAR:
		tk = interpret_expression(tk);
		break;
	case STMT_NONE:
		break;
	case STMT_WHILE:
		tk = interpret_expression(tk + 2);
		if (rvalpop())
		{
			stmstack[ssp].tk = etk;
			ssp++;
		}
		else
		{
			tk = (char *)(etk[2] + (etk[3] << 8));
		}
		break;		
	case STMT_IF:
		tk = interpret_expression(tk + 2);
		while (!rvalpop())
		{
			tk = (char *)(etk[2] + (etk[3] << 8));
			if (tk[0] == etk[0])
			{
				if (tk[1] == STMT_ELSE)
				{
					tk += 4;
					break;
				}
				else if (tk[1] == STMT_ELSIF)
				{
					etk = tk;					
					tk++;
					tk = interpret_expression(tk + 2);
				}
			}
			else
				break;
		}
		break;
	case STMT_ELSIF:
	case STMT_ELSE:
		tk = (char *)(etk[2] + (etk[3] << 8));
		break;

	}

	return tk;
}
	
