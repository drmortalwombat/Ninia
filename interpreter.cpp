#include "interpreter.h"
#include "variables.h"
#include "tokens.h"
#include "symbols.h"
#include <fixmath.h>

__striped Value		estack[VALUE_STACK_SIZE];
char				esp, efp;

const char * loopstart[16];
const char * exectk;
char exect;
char execl;


struct CallStack
{
	const char *	loopstart[16];
	char			efp, esp;
	const char	*	tk;
	char			t, l;
}	callstack[16];

char csp;

unsigned		localvars[32];
char			num_locals;
char	*		functk;

unsigned local_find(unsigned symbol)
{
	char i = 0;
	for(char i=0; i<num_locals; i++)
		if (localvars[i] == symbol)
			return i;
	return num_locals;
}

unsigned local_add(unsigned symbol)
{
	local_symbols[num_local_symbols++] = symbol;
	localvars[num_locals] = symbol;
	return num_locals++;
}


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
					{
						if (functk)
							local_add(ti);
						else
							global_add(ti);
					}

					unsigned	vi = local_find(ti);
					if (vi < num_locals)
					{
						tk[0] = (vi >> 8) | TK_LOCAL;
						tk[1] = vi & 0xff;
					}
					else
					{
						vi = global_find(ti);
						if (vi < num_globals)
						{
							tk[0] = (vi >> 8) | TK_GLOBAL;
							tk[1] = vi & 0xff;
						}
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
	char vi = global_add(ti);

	tk += 2;

	while ((tk[0] & 0xf0) == TK_IDENT)
	{
		local_add(((tk[0] & 0x0f) << 8) | tk[1]);
		tk += 2;
	}

	globals[vi].type = TYPE_FUNCTION;
	globals[vi].value = unsigned(tk);
	functk = tk;

	return tk + 3;
}

void prepare_statements(char * tk)
{
	exectk = tk;
	execl = 0;
	esp = VALUE_STACK_SIZE;
	csp = 0;
	num_globals = 0;
	num_locals = 0;
	num_local_symbols = 0;
	functk = nullptr;
	runtime_error = 0;
	mem_init();

	char 		pl = 1;
	char 		l = *tk;
	char 	*	pt = nullptr;

	while (l > 0)
	{	
		if (tk[1] == STMT_NONE)		
			tk += 2;
		else
		{
			while (l < pl)
			{
				char * ppt = (char *)(pt[0] + (pt[1] << 8));
				pt[0] = (unsigned)tk & 0xff;
				pt[1] = (unsigned)tk >> 8;
				pt = ppt;
				pl--;
			}

			if (l == 1 && functk)
			{
				functk[1] = num_locals;
				num_locals = 0;
				functk = nullptr;
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
			case STMT_RETURN_NULL:
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
		}
		l = *tk;
	}

	while (pl > 1)
	{
		char * ppt = (char *)(pt[0] + (pt[1] << 8));
		pt[0] = (unsigned)tk & 0xff;
		pt[1] = (unsigned)tk >> 8;
		pt = ppt;
		pl--;
	}
}

char * restore_expression(char * tk, bool var)
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
				if (tt == TK_GLOBAL)
				{
					unsigned vi = global_symbols[ti];
					tk[0] = (vi >> 8) | TK_IDENT;
					tk[1] = vi & 0xff;
				}
				else if (tt == TK_LOCAL)
				{
					unsigned	vi;
					if (left && var)
					{
						vi = local_symbols[num_local_symbols++];
						localvars[ti] = vi;
					}
					else
						vi = localvars[ti];

					tk[0] = (vi >> 8) | TK_IDENT;
					tk[1] = vi & 0xff;					
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

char * restore_function(char * tk)
{
	functk = tk;

	tk += 2;

	char n = 0;
	while ((tk[0] & 0xf0) == TK_IDENT)
	{		
		local_add(((tk[0] & 0x0f) << 8) | tk[1]);
		tk += 2;
		n++;
	}

	tk[1] = n;

	return tk + 3;
}


void restore_statements(char * tk)
{
	num_locals = 0;
	functk = nullptr;	
	num_local_symbols = 0;

	char	l = *tk;
	while (l > 0)
	{		
		tk++;
		char 	t = *tk++;

		if (t != STMT_NONE)
		{
			if (l == 1 && functk)
			{
				num_locals = 0;
				functk = nullptr;
			}

			switch (t)
			{
			case STMT_EXPRESSION:
			case STMT_RETURN:
				tk = restore_expression(tk, false);
				break;
			case STMT_VAR:
				tk = restore_expression(tk, true);
				break;
			case STMT_RETURN_NULL:
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
		}
		l = *tk;
	}	
}

void valderef(char at)
{
	char ei = esp + at;
	switch (estack[ei].type)
	{
	case TYPE_GLOBAL_REF:
		estack[ei] = globals[(char)(estack[ei].value)];
		break;
	case TYPE_LOCAL_REF:
		estack[ei] = estack[(char)(efp - estack[ei].value)];
		break;
	case TYPE_ARRAY_REF:
		{
			MemArray	*	ma = (MemArray *)(unsigned)estack[ei].value;
			MemValues	*	mv = (MemValues *)ma->mh;
			unsigned		mi = estack[ei].value >> 16;
			estack[ei] = mv->values[mi];
		} break;
	}
}

bool boolpop(void)
{
	valderef(0);
	return estack[esp++].value != 0;
}

long valpop(void)
{
	return estack[esp++].value;
}

long valget(char at)
{
	char ei = esp + at;
	return estack[ei].value;
}

inline char typeget(char at)
{
	char ei = esp + at;
	return estack[ei].type;
}

void valpush(char type, long value)
{
	if (esp == 0)
		runtime_error = RERR_STACK_UNDERFLOW;
	else
	{
		esp--;
		estack[esp].type = type;
		estack[esp].value = value;
	}
}

void valassign(void)
{
	switch (estack[esp + 1].type)
	{
	case TYPE_GLOBAL_REF:
		globals[(char)(estack[esp + 1].value)] = estack[esp];
		break;
	case TYPE_LOCAL_REF:
		estack[(char)(efp - estack[esp + 1].value)] = estack[esp];
		break;
	case TYPE_ARRAY_REF:
		{
			MemArray	*	ma = (MemArray *)(unsigned)estack[esp + 1].value;
			MemValues	*	mv = (MemValues *)ma->mh;
			unsigned		mi = estack[esp + 1].value >> 16;
			mv->values[mi] = estack[esp];
		} break;
	default:
		runtime_error = RERR_INVALID_ASSIGN;
	}
	esp++;
}

void valswap(void)
{
	Value	es = estack[esp];
	estack[esp] = estack[esp + 1];
	estack[esp + 1] = es;
}

const char * valstring(char at, char * buffer)
{
	char ei = esp + at;
	switch (estack[ei].type)
	{
	case TYPE_STRING_LITERAL:
		return (const char *)estack[ei].value;
	case TYPE_STRING_SHORT:
		buffer[0] = 1;
		buffer[1] = (char)((unsigned long)(estack[ei].value) >> 16);
		return buffer;
	case TYPE_STRING_HEAP:
		return ((MemString *)estack[ei].value)->data;
	}
	buffer[0] = 0;
	return buffer;
}

MemHead * valmem(char at)
{
	char ei = esp + at;
	return (MemHead *)estack[ei].value;	
}

MemValues * values_allocate(unsigned size, unsigned capacity)
{
	MemValues	*	v = (MemValues *)mem_allocate(MEM_VALUES, sizeof(MemValues) + capacity * sizeof(Value));
	if (v)
	{
		v->capacity = capacity;

		for(unsigned i=size; i<capacity; i++)
			v->values[i].type = TYPE_NULL;
	}

	return v;
}

MemArray * array_allocate(unsigned size, unsigned capacity)
{
	MemArray	*	a = (MemArray *)mem_allocate(MEM_ARRAY, sizeof(MemArray) + sizeof(MemValues) + capacity * sizeof(Value));
	if (a)
	{
		MemValues	*	v = (MemValues *)(a + 1);

		a->mh = v;
		a->size = size;

		v->type = MEM_VALUES;
		v->capacity = capacity;

		for(unsigned i=size; i<capacity; i++)
			v->values[i].type = TYPE_NULL;
	}

	return a;
}

MemArray * array_expand(char at, unsigned by)
{
	MemArray	*	ma = (MemArray *)valmem(at);
	MemValues	*	mv = (MemValues *)ma->mh;

	unsigned	size = ma->size;
	if (size + by > mv->capacity)
	{
		MemValues * mnv = values_allocate(size, size + by + (mv->capacity >> 1));
		if (!mnv)
			return nullptr;
		ma = (MemArray *)valmem(at);
		mv = (MemValues *)ma->mh;
		for(unsigned i=0; i<size; i++)
			mnv->values[i] = mv->values[i];
		ma->mh = mnv;
	}

	return ma;
}

MemString * string_allocate(char size)
{
	MemString	*	s = (MemString *)mem_allocate(MEM_STRING, sizeof(MemString) + size);
	if (s)
		s->data[0] = size;
	return s;
}


void interpret_builtin(char n)
{
	char tmp[2];
	unsigned lf = valget(n);
	switch (lf)
	{
	case RTSYM_ABS:
		{
			valderef(0);
			long li = valpop();
			esp += n;
			valpush(TYPE_NUMBER, labs(li));
		} break;
	case RTSYM_CHROUT:
		{
			valderef(0);
			long li = valpop();
			esp += n;
			putch(li >> 16);
			valpush(TYPE_NULL, 0);
		} break;		
	case RTSYM_RAND:
		esp += n + 1;
		valpush(TYPE_NUMBER, rand());
		break;
	case RTSYM_PRINT:
		{
			for(char i=0; i<n; i++)
			{
				char k = n - i - 1;
				valderef(k);
				switch (typeget(k) & TYPE_MASK)
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
						const char * str = valstring(k, tmp);
						char n = str[0];
						for(char i=0; i<n; i++)
							putch(str[i + 1]);
					} break;
				}
			}
			esp += n + 1;
			valpush(TYPE_NULL, 0);
		} break;
	case RTSYM_POKE:
		{
			valderef(0);
			char v = (unsigned long)valpop() >> 16;
			valderef(0);			
			volatile char * lp = (char *)((unsigned long)valpop() >> 16);
			*lp = v;
			esp += n - 1;
			valpush(TYPE_NULL, 0);
		} break;
	case RTSYM_PEEK:
		{
			valderef(0);
			volatile char * lp = (char *)((unsigned long)valpop() >> 16);
			esp += n;
			valpush(TYPE_NUMBER, (long)*lp << 16);
		} break;
	case RTSYM_LEN:
		{
			valderef(0);
			char t = typeget(0);
			if ((t & TYPE_MASK) == TYPE_STRING)
			{
				const char * str = valstring(0, tmp);
				esp += n + 1;
				valpush(TYPE_NUMBER, (unsigned long)str[0] << 16);
			}
			else if (t == TYPE_ARRAY)
			{
				MemArray * ma = (MemArray *)valmem(0);
				esp += n + 1;
				valpush(TYPE_NUMBER, (unsigned long)ma->size << 16);
			}
			else
			{
				esp += n + 1;
				valpush(TYPE_NUMBER, 0);
			}

		} break;
	case RTSYM_CHR:
		{
			valderef(0);
			long li = valpop() & 0x00ff0000ul;
			esp += n;
			valpush(TYPE_STRING_SHORT, li);
		} break;
	case RTSYM_ARRAY:
		{
			valderef(0);
			int m = valpop() >> 16;
			MemArray	*	ma = array_allocate(0, m);
			if (ma)
			{
				ma->size = m;
				esp += n;
				valpush(TYPE_ARRAY, (unsigned)ma);
			}
			else
				valpush(TYPE_NULL, 0);
		} break;
	case RTSYM_PUSH:
		{
			valderef(0);
			valderef(1);

			MemArray	*	ma;
			char t = typeget(1);
			if (t == TYPE_ARRAY)
				ma = array_expand(1, 1);
			else
				ma = array_allocate(1, 1);

			if (ma)
			{
				MemValues	*	mv = (MemValues *)ma->mh;
				mv->values[ma->size] = estack[esp];
				ma->size++;
				esp += n + 1;
				valpush(TYPE_ARRAY, (unsigned)ma);
			}
			else
				valpush(TYPE_NULL, 0);
		} break;
	case RTSYM_POP:
		{
			valderef(0);

			MemArray * ma = (MemArray *)valmem(0);
			MemValues * mv = (MemValues *)ma->mh;

			esp += n;

			ma->size--;

			estack[esp] = mv->values[ma->size];
			mv->values[ma->size].type = TYPE_NULL;
		} break;

	case RTSYM_CAT:
		{
			char t = 0;
			for(char i=0; i<n; i++)
			{
				char k = n - i - 1;
				valderef(k);
				const char * str = valstring(k, tmp);
				t += str[0];
			}
			MemString	*	ms = string_allocate(t);
			if (ms)
			{
				char * dstr = ms->data;

				t = 0;
				for(char i=0; i<n; i++)
				{
					char k = n - i - 1;
					const char * str = valstring(k, tmp);
					char s = str[0];
					for(char j=0; j<s; j++)
						dstr[++t] = str[j + 1];
				}
				esp += n + 1;
				valpush(TYPE_STRING_HEAP, (unsigned)ms);
			}
			else
				valpush(TYPE_NULL, 0);
		} break;

	default:
		runtime_error = RERR_UNDEFINED_SYMBOL;
	}
}

bool interpret_expression(void)
{
	const char * tk = exectk;

	char	ti = 0;
	for(;;)
	{
		if (runtime_error)
			return false;

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
			break;
		case TK_LOCAL:
			{
				unsigned tv = ((t & 0x0f) << 8) | tk[ti++];
				valpush(TYPE_LOCAL_REF, tv);
			} break;

		case TK_GLOBAL:	
			{
				unsigned tv = ((t & 0x0f) << 8) | tk[ti++];
				valpush(TYPE_GLOBAL_REF, tv);
			} break;

		case TK_BINARY:
			{
				valderef(0); 
				valderef(1);

				char tm = typeget(0) | typeget(1);
				if (tm == TYPE_NUMBER)
				{
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
						l1 = lmul16f16s(l1, l2);
						break;
					case TK_DIV:
						l1 = ldiv16f16s(l1, l2);
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
				}
				else
					runtime_error = RERR_INVALID_TYPES;
			}	break;
		case TK_RELATIONAL:
			{
				valderef(0); 
				valderef(1);

				// GT EQ LT
				char cmp = 1;

				char tm = typeget(0) | typeget(1);
				if (tm == TYPE_NUMBER)
				{
					long l2 = valpop();
					long l1 = valpop();

					if (l1 == l2)
						cmp = 2;
					else if (l1 > l2)
						cmp = 4;
				}
				else if ((tm & TYPE_MASK) == TYPE_STRING)
				{
					char sb0[2], sb1[2];

					const char * s2 = valstring(0, sb0);
					const char * s1 = valstring(1, sb1);
					esp += 2;

					char i = 0;
					while (i < s1[0] && i < s2[0] && s1[i+1] == s2[i+1])
						i++;

					if (i < s1[0])
					{
						if (i == s2[0] || s1[i+1] > s2[i+1])
							cmp = 4;
					}
					else if (i == s2[0])
						cmp = 2;

				}
				else
				{
					esp += 2;
					cmp = 0;
				}

				static const char cmpmask[6] = {2, 5, 1, 3, 4, 6};

				valpush(TYPE_NUMBER, (cmp & cmpmask[t & 0x0f]) ? 0x10000ul : 0ul);
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
				{
					char n = tk[ti++];

					valderef(0);
					long ei = valpop();
					valderef(0);
					estack[esp].type = TYPE_ARRAY_REF;
					estack[esp].value |= ei & 0xffff0000ul;
					break;
				}
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
					case TYPE_FUNCTION:
						{
							callstack[csp].efp = efp;
							callstack[csp].esp = esp + n;
							callstack[csp].tk = tk + ti;
							callstack[csp].t = exect;
							callstack[csp].l = execl;
							for(char i=0; i<execl; i++)
								callstack[csp].loopstart[i] = loopstart[i];
							csp++;

							exectk = (char *)valget(n);
							execl = 0;
							loopstart[0] = nullptr;
							char k = exectk[1];
							
							for(char i=0; i<n; i++)
								valderef(i);
							while (n < k)
							{
								valpush(TYPE_NULL, 0);
								n++;
							}
							efp = esp + n - 1;

							exectk += 3;
							return false;
						}
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
						estack[esp + n - 1] = estack[esp];
						esp += n - 1;
					}
					break;
				case TK_ARRAY:
					{
						MemArray	*	ma = array_allocate(n, n);
						MemValues	*	mv = (MemValues *)(ma + 1);
						for(char i=0; i<n; i++)
						{
							char ei = n - i - 1;
							valderef(ei);
							mv->values[i] = estack[esp + ei];
						}
						esp += n;
						valpush(TYPE_ARRAY, (unsigned)ma);
					} break;
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
					esp++;
					break;
				case TK_STRING:
					valpush(TYPE_STRING_LITERAL, (unsigned)(tk + ti));
					ti += tk[ti] + 1;
					break;
				}
			}
		}
	}
}

bool interpret_statement(void)
{
	const char * tk = exectk;

	while (tk[0] && tk[1] == STMT_NONE)
		tk += 2;

	char 	l = tk[0];
	if (l > 0) l--;

	if (l > execl)
		execl = l;
	else
	{
		while (l < execl)
		{
			execl--;
			if (loopstart[execl])
			{
				tk = loopstart[execl];
				break;
			}
		}
	}

	loopstart[execl] = nullptr;

	if (!tk[0])
		return false;

	const char * etk = tk;

	char 	t = tk[1];

	if (execl == 0 && csp > 0)
		t = STMT_RETURN_NULL;

	switch (t)
	{
	case STMT_ELSE:
	case STMT_ELSIF:
	case STMT_DEF:
		exectk = (char *)(etk[2] + (etk[3] << 8));
		return true;
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
		exect = t;

		if (t == STMT_RETURN_NULL)
			valpush(TYPE_NULL, 0);
		else
		{
			if (!interpret_expression())
				return true;
		}

		switch (t)
		{
		case STMT_EXPRESSION:
		case STMT_VAR:
			esp++;
			return true;

		case STMT_RETURN_NULL:
		case STMT_RETURN:
			{
				valderef(0);
				csp--;
				char rsp = callstack[csp].esp;
				estack[rsp] = estack[esp];
				esp = rsp;

				efp = callstack[csp].efp;
				tk = callstack[csp].tk;
				t =  callstack[csp].t;
				execl = callstack[csp].l;
				for(char i=0; i<execl; i++)
					loopstart[i] = callstack[csp].loopstart[i];
			} break;

		case STMT_WHILE:
			if (boolpop())
				loopstart[execl] = etk;
			else
				exectk = (char *)(etk[2] + (etk[3] << 8));
			return true;

		case STMT_IF:
		case STMT_ELSIF:
			if (boolpop())
				return true;

			tk = (char *)(etk[2] + (etk[3] << 8));
			if (tk[0] != etk[0])
				return true;

			if (tk[1] == STMT_ELSE)
			{
				exectk = tk + 4;
				return true;
			}
			else if (tk[1] == STMT_ELSIF)
			{
				etk = tk;
				tk += 4;
			}
			else
			{
				exectk = tk;
				return true;
			}
		}
	}
}
	
