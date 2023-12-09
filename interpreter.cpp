#include "interpreter.h"
#include "tokens.h"
#include "symbols.h"
#include "errors.h"
#include <fixmath.h>
#include <time.h>
#include <conio.h>

__zeropage char		esp;

#pragma code( rcode )
#pragma data( rdata )
#pragma bss( rtbss )

__striped Value		estack[VALUE_STACK_SIZE];
char efp;

__striped struct CallStack
{
	char			efp, esp, cfp;
	char			token, type;
	const char	*	tk;
}	callstack[CALL_STACK_SIZE];

char csp, cfp;

__striped unsigned		localvars[32];

#pragma bss( bss )

const char * exectk;
char exect;

void interpreter_init(char * tk)
{
	exectk = tk;
	esp = VALUE_STACK_SIZE;
	csp = 0;
	cfp = 0;
	runtime_error = 0;
	callstack[0].type = CSTACK_NONE;
	mem_init();
}


char struct_index(MemDict * md, unsigned sym)
{
	char sz = md->symbols[0];
	unsigned * sp = (unsigned *)(md->symbols + 1);

	__assume(sz < 64);
	while (sz > 0)
	{
		sz--;
		if (sp[sz] == sym)
			return sz;
	}
	return 0xff;
}

void valderef(char at)
{
	char ei = esp + at;
	__assume(ei < VALUE_STACK_SIZE);
	if (estack[ei].type & TYPE_REF)
	{
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
		case TYPE_STRUCT_REF:
			{
				MemDict		*	md = (MemDict *)(unsigned)estack[ei].value;
				MemValues	*	mv = (MemValues *)md->mh;
				unsigned		ms = estack[ei].value >> 16;
				char			mi = struct_index(md, ms);
				if (mi != 0xff)
					estack[ei] = mv->values[mi];
				else
					estack[ei].type = TYPE_NULL;
			} break;
		}
	}
}

bool boolpop(void)
{
	valderef(0);
	return estack[esp++].value != 0;
}

inline auto & valtop(void)
{
	return estack[esp].value;
}

inline long valpop(void)
{
	return estack[esp++].value;
}

inline long valget(char at)
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
	case TYPE_STRUCT_REF:
		{
			MemDict		*	md = (MemDict *)(unsigned)estack[esp + 1].value;
			MemValues	*	mv = (MemValues *)md->mh;
			unsigned		ms = estack[esp + 1].value >> 16;
			char			mi = struct_index(md, ms);
			if (mi != 0xff)
				mv->values[mi] = estack[esp];
			else
				runtime_error = RERR_UNDEFINED_SYMBOL;
		} break;
	default:
		runtime_error = RERR_INVALID_ASSIGN;
	}
	esp++;
}

void valinc(long by)
{
	switch (estack[esp].type)
	{
	case TYPE_GLOBAL_REF:
		globals[(char)(estack[esp].value)].value += by;
		break;
	case TYPE_LOCAL_REF:
		estack[(char)(efp - estack[esp].value)].value += by;
		break;
	case TYPE_ARRAY_REF:
		{
			MemArray	*	ma = (MemArray *)(unsigned)estack[esp].value;
			MemValues	*	mv = (MemValues *)ma->mh;
			unsigned		mi = estack[esp].value >> 16;
			mv->values[mi].value += by;
		} break;
	case TYPE_STRUCT_REF:
		{
			MemDict		*	md = (MemDict *)(unsigned)estack[esp].value;
			MemValues	*	mv = (MemValues *)md->mh;
			unsigned		ms = estack[esp].value >> 16;
			char			mi = struct_index(md, ms);
			if (mi != 0xff)
				mv->values[mi].value += by;
			else
				runtime_error = RERR_UNDEFINED_SYMBOL;
		} break;
	default:
		runtime_error = RERR_INVALID_ASSIGN;
	}
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

inline MemHead * valmem(char at)
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

MemDict * dict_allocate(const char * dict)
{
	char size = dict[0];
	MemDict	*	d = (MemDict *)mem_allocate(MEM_DICT, sizeof(MemDict) + sizeof(MemValues) + size * sizeof(MemValues));
	if (d)
	{
		MemValues	*	v = (MemValues *)(d + 1);

		d->mh = v;
		d->symbols = dict;

		v->type = MEM_VALUES;
		v->capacity = size;
	}

	return d;
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
		memcpy(mnv->values, mv->values, size * sizeof(Value));
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

MemString * string_allocate(const char * str)
{
	char i = 0;
	while (str[i])
		i++;

	MemString	*	s = (MemString *)mem_allocate(MEM_STRING, sizeof(MemString) + i);
	if (s)
	{
		s->data[0] = i;
		i = 0;
		while (char c = str[i])
		{
			i++;
			s->data[i] = c;
		}
	}
	return s;
}


void interpret_builtin(char n)
{
	char tmp[80], tmp2[2];
	unsigned lf = valget(n);

	if ((lf & 0xff00) != 0x0100)
	{
		runtime_error = RERR_UNDEFINED_SYMBOL;
		return;
	}

	switch ((char)lf)
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
			system_putch(li >> 16);
			valpush(TYPE_NULL, 0);
		} break;		
	case RTSYM_RAND:
		esp += n + 1;
		valpush(TYPE_NUMBER, rand());
		break;
	case RTSYM_TIME:
		{
			long l = clock();
			esp += n + 1;
			valpush(TYPE_NUMBER, lmul16f16s(l, 0x04444444l));
		}	break;
	case RTSYM_CHRIN:
		{
			char ch = system_getchx();
			esp += n + 1;
			valpush(TYPE_NUMBER, (long)ch << 16);
		} break;
	case RTSYM_INPUT:
		{
			char i = 0;
			while ((char c = system_readch()) != PETSCII_RETURN)
				tmp[i++] = c;
			tmp[i] = 0;
			MemString	*	ms = string_allocate(tmp);

			esp += n + 1;
			valpush(TYPE_STRING_HEAP, (unsigned)ms);
		} break;
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
						long	s = valget(k);					
						const char * str = number_format(s, true);
						char i = 0;
						while (str[i])
							system_putch(str[i++]);

					} break;
				case TYPE_STRING:
					{
						const char * str = valstring(k, tmp);
						char n = str[0];
						for(char i=0; i<n; i++)
							system_putch(str[i + 1]);
					} break;
				}
			}
			esp += n + 1;
			valpush(TYPE_NULL, 0);
		} break;
	case RTSYM_POKE:
		{
			for(char i=0; i<n; i+=2)
			{
				char k = n - i - 1;
				valderef(k);
				valderef(k-1);	

				volatile char * lp = (char *)((unsigned long)valget(k) >> 16);
				char v = (unsigned long)valget(k-1) >> 16;
				*lp = v;
			}
			esp += n + 1;
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

	case RTSYM_SEG:
		{
			int num = 250;
			if (n > 2)
			{
				valderef(n - 3);
				num = valget(n - 3) >> 16;
			}

			valderef(n - 1);
			valderef(n - 2);
			char t = typeget(n - 1);
			if ((t & TYPE_MASK) == TYPE_STRING)
			{
				const char * str = valstring(n - 1, tmp);

				int start = valget(n - 2) >> 16;
				if (start < 0)
					start == 0;
	
				esp += n + 1;

				if (start >= str[0] || num <= 0)
					valpush(TYPE_STRING_SHORT, 0);
				else if (num == 1)
					valpush(TYPE_STRING_SHORT, (unsigned long)str[start] << 16);
				else
				{
					if (start + num > str[0])
						num = str[0] - start;

					MemString	*	ms = string_allocate(num);
					if (ms)
					{
						char * dstr = ms->data;

						t = 0;
						for(char i=0; i<num; i++)
							dstr[++t] = str[i + start + 1];
						valpush(TYPE_STRING_HEAP, (unsigned)ms);
					}
					else
						valpush(TYPE_NULL, 0);					
				}
			}
			else
			{
				esp += n + 1;
				valpush(TYPE_NULL, 0);					
			}
		} break;

	case RTSYM_ASC:
		{
			valderef(0);
			char t = typeget(0);
			if ((t & TYPE_MASK) == TYPE_STRING)
			{
				const char * str = valstring(0, tmp);
				esp += n + 1;
				valpush(TYPE_NUMBER, str[0] > 0 ? (unsigned long)str[1] << 16 : 0);
			}
			else
			{
				esp += n + 1;
				valpush(TYPE_NUMBER, 0);
			}

		} break;
	case RTSYM_VAL:
		{
			valderef(0);
			const char * str = valstring(0, tmp);

			long l;
			number_parse(str + 1, str[0], l);

			esp += n + 1;
			valpush(TYPE_NUMBER, l);		
		} break;		
	case RTSYM_STR:
		{
			valderef(0);
			char t = typeget(0);
			if ((t & TYPE_MASK) == TYPE_STRING)
			{
				estack[esp + n] = estack[esp];
				esp += n;
			}
			else
			{
				long	s = valget(0);
				
				const char * str = number_format(s, true);

				MemString	*	ms = string_allocate(str);

				esp += n + 1;
				valpush(TYPE_STRING_HEAP, (unsigned)ms);
			}
		} break;
	case RTSYM_FLOOR:
		{
			valderef(0);
			long li = valpop() & 0xffff0000ul;
			esp += n;
			valpush(TYPE_NUMBER, li);
		} break;
	case RTSYM_CEIL:
		{
			valderef(0);
			long li = (valpop() + 0xffff) & 0xffff0000ul;
			esp += n;
			valpush(TYPE_NUMBER, li);
		} break;

	case RTSYM_FIND:
		{
			int start = 0;
			if (n > 2)
			{
				valderef(n - 3);
				start = valget(n - 3) >> 16;
			}

			valderef(n - 1);
			valderef(n - 2);
			char t = typeget(n - 1);
			if ((t & TYPE_MASK) == TYPE_STRING)
			{
				const char * str = valstring(n - 1, tmp);
				const char * find = valstring(n - 2, tmp2);

				esp += n + 1;

				while (start + find[0] <= str[0])
				{
					char i = 0;
					while (i < find[0] && find[i + 1] == str[start + i + 1])
						i++;
					if (i == find[0])
					{
						valpush(TYPE_NUMBER, (unsigned long)start << 16);
						return;
					}
					start++;
				}

				valpush(TYPE_NUMBER, 0xffff0000ul);
			}
			else
			{
				esp += n + 1;
				valpush(TYPE_NUMBER, 0xffff0000ul);
			}
		} break;

	default:
		runtime_error = RERR_UNDEFINED_SYMBOL;
	}
}

void interpret_binop(char t)
{
	long l2 = valpop();
	auto & l1 = valtop();

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
	default:
		__assume(false);
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
					interpret_binop(t);
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

				valpush(TYPE_NUMBER, (cmp & cmpmask[t & 0x0f]) ? 0xffff0000ul : 0ul);
			} break;
		case TK_PREFIX:
			switch (t)
			{
			case TK_NEGATE:
				valderef(0); 
				valpush(TYPE_NUMBER, -valpop());
				break;
			case TK_NOT:
				{
					valderef(0); 
					char t = typeget(0);
					if (t == TYPE_NUMBER)
						valpush(TYPE_NUMBER, valpop() & 0xffff0000ul ^ 0xffff0000ul);
					else if (t == TYPE_NULL)
					{
						esp++;
						valpush(TYPE_NUMBER, -1l << 16);
					}
					else
					{
						esp++;
						valpush(TYPE_NUMBER, 0);
					}
				}	break;
			default:
				__assume(false);
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
					char t = typeget(0);
					if (t == TYPE_ARRAY)
					{
						estack[esp].type = TYPE_ARRAY_REF;
						estack[esp].value |= ei & 0xffff0000ul;
					}
					else if ((t & TYPE_MASK) == TYPE_STRING)
					{
						char tmp[2];
						const char * s = valstring(0, tmp);
						int si = ei >> 16;
						if (si >= 0 && si < s[0])
						{
							esp++;
							valpush(TYPE_STRING_SHORT, (unsigned long)s[si + 1] << 16);
						}
						else
						{
							esp++;
							valpush(TYPE_STRING_SHORT, 0);
						}
					}
					else
						runtime_error = RERR_INVALID_TYPES;					
				}	break;
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
							callstack[csp].cfp = cfp;
							callstack[csp].tk = tk + ti;
							callstack[csp].token = exect;
							callstack[csp].type = CSTACK_CALL;
							cfp = csp;

							exectk = (char *)valget(n);
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
			}	break;

		case TK_ASSIGN:
			{
				valderef(0);
				switch (t)
				{
				case TK_ASSIGN:
					valassign();
					break;
				case TK_ASSIGN_ADD:
					valinc(valpop());
					break;
				case TK_ASSIGN_SUB:
					valinc(-valpop());
					break;
				default:
					__assume(false);
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
				case TK_COLON:
					break;
				case TK_DOT:
					valderef(0);
					unsigned long ei = valpop();
					valderef(0);
					estack[esp].type = TYPE_STRUCT_REF;
					estack[esp].value |= ei << 16;
					break;
				case TK_STRING:
					valpush(TYPE_STRING_LITERAL, (unsigned)(tk + ti));
					ti += tk[ti] + 1;
					break;
				case TK_LIST:
					{
						char n = tk[ti++];
						__assume(n < VALUE_STACK_SIZE);
						if (n > 0)
						{
							char ei = esp + n - 1;
							__assume(ei < VALUE_STACK_SIZE);
							estack[ei] = estack[esp];
							esp = ei;
						}
					} 	break;
				case TK_ARRAY:
					{
						char n = tk[ti++];
						__assume(n < VALUE_STACK_SIZE);
						MemArray	*	ma = array_allocate(n, n);
						MemValues	*	mv = (MemValues *)(ma + 1);
						for(char i=0; i<n; i++)
						{
							char ei = n - i - 1;
							valderef(ei);
							ei += esp;
							__assume(ei < VALUE_STACK_SIZE);
							mv->values[i] = estack[ei];
						}
						esp += n;
						valpush(TYPE_ARRAY, (unsigned)ma);
					} break;
				case TK_STRUCT:
					{
						char n = tk[ti];
						__assume(n < VALUE_STACK_SIZE);
						MemDict		*	md = dict_allocate(tk + ti);
						MemValues	*	mv = (MemValues *)(md->mh);

						for(char i=0; i<n; i++)
						{
							char ei = n - i - 1;
							valderef(ei);
							ei += esp;
							__assume(ei < VALUE_STACK_SIZE);
							mv->values[i] = estack[ei];
						}
						esp += n;
						ti += 2 * n + 1;
						valpush(TYPE_STRUCT, (unsigned)md);
					} break;
				default:
					__assume(false);
				}
			} break;
		default:
			__assume(false);
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

	l += cfp;

	if (l > csp)
	{
		csp = l;
		callstack[csp].type = CSTACK_NONE;
	}
	else
	{
		while (l < csp)
		{
			csp--;
			if (callstack[csp].type)
			{
				tk = callstack[csp].tk;
				break;
			}
		}
	}

	if (!tk[0])
		return false;

	const char * etk = tk;
	char 	t = tk[1];

	if (callstack[csp].type == CSTACK_CALL)
		t = STMT_RETURN_NULL;
	callstack[csp].type = CSTACK_NONE;

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
	case STMT_COMMENT:
		exectk = tk + tk[2] + 3;
		return true;
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
				csp = cfp;

				char rsp = callstack[csp].esp;
				estack[rsp] = estack[esp];
				esp = rsp;
				efp = callstack[csp].efp;
				cfp = callstack[csp].cfp;

				tk = callstack[csp].tk;
				t =  callstack[csp].token;
				callstack[csp].type = CSTACK_NONE;
			} break;

		case STMT_WHILE:
			if (boolpop())
			{
				callstack[csp].tk = etk;
				callstack[csp].type = CSTACK_WHILE;
			}
			else
				exectk = (char *)(etk[2] + (etk[3] << 8));
			return true;

		case STMT_IF:
		case STMT_ELSIF:
			if (boolpop())
				return true;

			tk = (char *)(etk[2] + (etk[3] << 8));
			if (tk[0] != etk[0])
			{
				exectk = tk;
				return true;
			}

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

		default:
			__assume(false);
		}
	}
}
	
void interpret_program(void)
{
	while (interpret_statement() && !runtime_error && *(volatile char *)0x91 != 0x7f)
		;
}

