#include "interpreter.h"
#include "tokens.h"
#include "symbols.h"
#include "errors.h"
#include <fixmath.h>
#include <time.h>
#include <conio.h>
#include <c64/kernalio.h>

__zeropage char		esp;

#pragma code( rcode )
#pragma data( rdata )
#pragma bss( rtbss )

__striped Value		estack[VALUE_STACK_SIZE];
char efp;

__striped struct CallStack
{
	char			efp, esp, cfp;
	char			type;
	const char	*	tk, * etk;
}	callstack[CALL_STACK_SIZE];

char csp, cfp;

__striped unsigned		localvars[32];

char	filemask;

#pragma bss( bss )

const char * exectk, * execetk;

unsigned useed;

unsigned int urand(void)
{
    useed ^= useed << 7;
    useed ^= useed >> 9;
    useed ^= useed << 8;
	return useed;
}

void interpret_init(char * tk)
{
	exectk = tk;
	esp = VALUE_STACK_SIZE;
	csp = 0;
	cfp = 0;
	runtime_error = 0;
	callstack[0].type = CSTACK_NONE;
	useed = clock() ^ 0x3417;
	mem_init();
	filemask = 0;
}

void interpret_reset(void)
{
	for(char i=0; i<8; i++)
		if (filemask & (1 << i))
			krnio_close(i + 1);
	filemask = 0;
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

void valpushchar(char ch)
{
	valpush(TYPE_STRING_SHORT, 1 | ((unsigned)ch << 8));
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

const char emptystr[1] = {0};

const char * valstring(char at, char * tmp)
{
	char ei = esp + at;
	switch (estack[ei].type)
	{
	case TYPE_STRING_LITERAL:
		return (const char *)estack[ei].value;
	case TYPE_STRING_SHORT:
		tmp[0] = estack[ei].value;
		tmp[1] = estack[ei].value >> 8;
		tmp[2] = estack[ei].value >> 16;
		tmp[3] = estack[ei].value >> 24;
		return tmp;
	case TYPE_STRING_HEAP:
		return ((MemString *)estack[ei].value)->data;
	default:
		return emptystr;
	}
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

MemString * string_allocate(const char * str, char size)
{
	MemString	*	s = (MemString *)mem_allocate(MEM_STRING, sizeof(MemString) + size);
	if (s)
	{
		s->data[0] = size;
		for(char i=0; i<size; i++)
			s->data[i + 1] = str[i];
	}
	return s;
}

MemString * string_allocate(const char * str)
{
	char i = 0;
	while (str[i])
		i++;
	return string_allocate(str, i);
}

void valpushstring(const char * str, char size)
{
	switch (size)
	{
	case 0:
		valpush(TYPE_STRING_SHORT, 0);
		break;
	case 1:
		valpush(TYPE_STRING_SHORT, 1 | ((unsigned)str[0] << 8));
		break;
	case 2:
		valpush(TYPE_STRING_SHORT, 2 | ((unsigned)str[0] << 8) | ((unsigned long)str[1] << 16));
		break;
	case 3:
		valpush(TYPE_STRING_SHORT, 3 | ((unsigned)str[0] << 8) | ((unsigned long)str[1] << 16) | ((unsigned long)str[2] << 24));
		break;
	default:
		valpush(TYPE_STRING_HEAP, (unsigned)string_allocate(str, size));
		break;
	}
}

void valpushstring(const char * str)
{
	char i = 0;
	while (str[i])
		i++;
	valpushstring(str, i);
}

void interpret_builtin(char n)
{
	char tmp[80], tmp1[20], tmp2[20];
	unsigned lf = valget(n);

	if ((lf & 0xff00) != 0x0100)
	{
		runtime_error = RERR_UNDEFINED_SYMBOL;
		return;
	}

	switch ((char)lf)
	{
	case RTSYM_ABS:
		if (n == 1)
		{
			valderef(0);
			long li = valpop();
			esp += 1;
			valpush(TYPE_NUMBER, labs(li));
		} 
		else
			runtime_error = RERR_INVALID_ARGUMENTS;
		break;
	case RTSYM_CHROUT:
		for(char i=0; i<n; i++)
		{
			valderef(n - i - 1);
			system_putch(valget(n - i - 1) >> 16);
		} 
		esp += n + 1;
		valpush(TYPE_NULL, 0);
		break;
	case RTSYM_RAND:
		if (n == 0)
		{
			esp += 1;
			valpush(TYPE_NUMBER, urand());
		}
		else
			runtime_error = RERR_INVALID_ARGUMENTS;
		break;
	case RTSYM_TIME:
		if (n == 0)
		{
			long l = clock();
			esp += 1;
			valpush(TYPE_NUMBER, lmul16f16s(l, 0x04444444l));
		}
		else
			runtime_error = RERR_INVALID_ARGUMENTS;
		break;
	case RTSYM_CHRIN:
		if (n == 0)
		{
			char ch = system_getchx();
			esp += 1;
			valpush(TYPE_NUMBER, (long)ch << 16);
		}
		else
			runtime_error = RERR_INVALID_ARGUMENTS;
		break;
	case RTSYM_INPUT:
		if (n == 0)
		{
			char i = 0;
			while ((char c = system_readch()) != PETSCII_RETURN)
				tmp[i++] = c;
			esp += 1;
			valpushstring(tmp, i);
		}
		else
			runtime_error = RERR_INVALID_ARGUMENTS;
		break;
	case RTSYM_PRINT:
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
					const char * str = valstring(k, tmp1);
					char n = str[0];
					for(char i=0; i<n; i++)
						system_putch(str[i + 1]);
				} break;
			}
		}
		esp += n + 1;
		valpush(TYPE_NULL, 0);
		break;

	case RTSYM_POKE:
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
		break;

	case RTSYM_PEEK:
		if (n == 1)
		{
			valderef(0);
			volatile char * lp = (char *)((unsigned long)valpop() >> 16);
			esp += 1;
			valpush(TYPE_NUMBER, (long)*lp << 16);
		}
		else
			runtime_error = RERR_INVALID_ARGUMENTS;
		break;

	case RTSYM_CHR:
		for(char i=0; i<n; i++)
		{
			char k = n - i - 1;
			valderef(k);
			tmp[i] = valget(n - i - 1) >> 16;
		}
		esp += n + 1;
		valpushstring(tmp, n);
		break;

	case RTSYM_ARRAY:
		if (n == 1)
		{
			valderef(0);
			int m = valpop() >> 16;
			MemArray	*	ma = array_allocate(0, m);
			if (ma)
			{
				ma->size = m;
				esp += 1;
				valpush(TYPE_ARRAY, (unsigned)ma);
			}
		}
		else
			runtime_error = RERR_INVALID_ARGUMENTS;
		break;

	case RTSYM_PUSH:
		if (n > 1)
		{
			char k = n - 1;
			valderef(k);
			MemArray	*	ma;
			char t = typeget(k);
			if (t == TYPE_ARRAY)
				ma = array_expand(k, k);
			else
				ma = array_allocate(k, k);

			if (ma)
			{
				MemValues	*	mv = (MemValues *)ma->mh;
				Value 		*	vp = mv->values + ma->size;
				char			ei = esp + k - 1;

				for(char i=0; i<k; i++)
				{
					valderef(k - i - 1);
					*vp++ = estack[ei--];
				}
				ma->size += k;
				esp += n + 1;
				valpush(TYPE_ARRAY, (unsigned)ma);
			}
		}
		else
			runtime_error = RERR_INVALID_ARGUMENTS;
		break;

	case RTSYM_SHIFT:
		if (n == 1)
		{
			valderef(0);

			MemArray * ma = (MemArray *)valmem(0);
			MemValues * mv = (MemValues *)ma->mh;

			esp += 1;
			estack[esp] = mv->values[0];

			ma->size--;

			int sz = ma->size;
			for(int i=0; i<sz; i++)
				mv->values[i] = mv->values[i + 1];
			mv->values[sz].type = TYPE_NULL;
		}
		else
			runtime_error = RERR_INVALID_ARGUMENTS;
		break;

	case RTSYM_POP:
		if (n == 1)
		{
			valderef(0);

			MemArray * ma = (MemArray *)valmem(0);
			MemValues * mv = (MemValues *)ma->mh;

			esp += 1;

			ma->size--;

			estack[esp] = mv->values[ma->size];
			mv->values[ma->size].type = TYPE_NULL;
		}
		else
			runtime_error = RERR_INVALID_ARGUMENTS;
		break;

	case RTSYM_CAT:
		{
			char t = 0;
			for(char i=0; i<n; i++)
			{
				char k = n - i - 1;
				valderef(k);
				const char * str = valstring(k, tmp1);
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
					const char * str = valstring(k, tmp1);
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

	case RTSYM_ASC:
		if (n == 1)
		{
			valderef(0);
			const char * str = valstring(0, tmp1);
			esp += 2;
			valpush(TYPE_NUMBER, str[0] > 0 ? (unsigned long)str[1] << 16 : 0);
		}
		else
			runtime_error = RERR_INVALID_ARGUMENTS;
		break;

	case RTSYM_VAL:
		if (n == 1)
		{
			valderef(0);
			const char * str = valstring(0, tmp1);

			long l;
			number_parse(str + 1, str[0], l);

			esp += n + 1;
			valpush(TYPE_NUMBER, l);		
		}
		else
			runtime_error = RERR_INVALID_ARGUMENTS;
		break;

	case RTSYM_STR:
		if (n == 1)
		{
			valderef(0);
			char t = typeget(0);
			if ((t & TYPE_MASK) == TYPE_STRING)
			{
				estack[esp + 1] = estack[esp];
				esp += 1;
			}
			else
			{
				long	s = valget(0);				
				const char * str = number_format(s, true);
				esp += 2;
				valpushstring(str);
			}
		}
		else
			runtime_error = RERR_INVALID_ARGUMENTS;
		break;

	case RTSYM_FLOOR:
		if (n == 1)
		{
			valderef(0);
			long li = valpop() & 0xffff0000ul;
			esp += 1;
			valpush(TYPE_NUMBER, li);
		}
		else
			runtime_error = RERR_INVALID_ARGUMENTS;
		break;

	case RTSYM_CEIL:
		if (n == 1)
		{
			valderef(0);
			long li = (valpop() + 0xffff) & 0xffff0000ul;
			esp += 1;
			valpush(TYPE_NUMBER, li);
		}
		else
			runtime_error = RERR_INVALID_ARGUMENTS;
		break;

	case RTSYM_FIND:
		if (n >= 2)
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
				const char * str = valstring(n - 1, tmp1);
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
		}
		else
			runtime_error = RERR_INVALID_ARGUMENTS;
		break;

	case RTSYM_FOPEN:
		if (n == 3)
		{
			valderef(2);
			valderef(1);
			valderef(0);
			if (typeget(2) == TYPE_NUMBER && typeget(1) == TYPE_NUMBER && (typeget(0) & TYPE_MASK) == TYPE_STRING)
			{

				char fid = 0;
				while (fid < 8 && (filemask & (1 << fid)))
					fid++;
				if (fid < 8)
				{
					const char * fname = valstring(0, tmp1);
					krnio_setnam_n(fname + 1, fname[0]);

					if (krnio_open(fid + 1, valget(2) >> 16, valget(1) >> 16))
					{
						filemask |= 1 << fid;

						esp += 4;
						valpush(TYPE_NUMBER, long(fid) << 16);
					}
					else
					{
						esp += 4;
						valpush(TYPE_NUMBER, 0xffff0000ul);
					}
				}
				else
				{
					esp += 4;
					valpush(TYPE_NUMBER, 0xffff0000ul);
				}
			}
			else
				runtime_error = RERR_INVALID_ARGUMENTS;
		}
		else
			runtime_error = RERR_INVALID_ARGUMENTS;
		break;

	case RTSYM_FCLOSE:
		if (n == 1)
		{
			valderef(0);
			if (typeget(0) == TYPE_NUMBER)
			{
				char fid = valget(0) >> 16;
				if (filemask & (1 << fid))
				{
					krnio_close(fid + 1);
					filemask &= ~(1 << fid);
				}

				esp += 2;
			}
			else
				runtime_error = RERR_INVALID_ARGUMENTS;
		}
		else
			runtime_error = RERR_INVALID_ARGUMENTS;
		break;

	case RTSYM_FGET:
		if (n >= 1)
		{
			valderef(n - 1);
			if (typeget(n - 1) == TYPE_NUMBER)
			{
				char fid = valget(n - 1) >> 16;

				const char * guard = tmp2;
				char limit = 1;
				tmp2[0] = 0;
				if (n >= 2)
				{
					valderef(n - 2);
					if (typeget(n - 2) == TYPE_NUMBER)
					{
						limit  = valget(n - 2) >> 16;
						if (limit > 255)
							limit = 255;							
					}
					else if ((typeget(n - 2) & TYPE_MASK) == TYPE_STRING)
					{
						limit = 255;
						guard = valstring(n - 2, tmp1);
					}
				}
				esp += n + 1;

				if (limit > 0 && (filemask & (1 << fid)))
				{
					krnio_chkin(fid + 1);
					char i = 0;
					while (i < limit)
					{
						char ch = krnio_chrin();
						krnioerr err = krnio_status();
						krnio_pstatus[fid + 1] = err;
						if (err && err != KRNIO_EOF)
							break;
						tmp[i++] = (char)ch;

						char j = 0;
						while (j < guard[0] && ch != guard[j + 1])
							j++;
						if (j < guard[0])
							break;
						if (err)
							break;
					}							
					krnio_clrchn();

					valpushstring(tmp, i);
				}
				else
					valpush(TYPE_NULL, 0);
			}
			else
				runtime_error = RERR_INVALID_ARGUMENTS;
		}
		else
			runtime_error = RERR_INVALID_ARGUMENTS;

		break;

	case RTSYM_FPUT:
		if (n == 2)
		{
			valderef(n - 1);
			valderef(n - 2);
			if (typeget(n - 1) == TYPE_NUMBER && (typeget(n - 2) & TYPE_MASK) == TYPE_STRING)
			{
				char fid = valget(n - 1) >> 16;

				if (filemask & (1 << fid))
				{						
					const char * str = valstring(n - 2, tmp1);
					krnio_chkout(fid + 1);
					char n = str[0];
					for(char i=0; i<n; i++)
						krnio_chrout(str[i + 1]);
					krnio_clrchn();
				}

				esp += n + 1;
				valpush(TYPE_NULL, 0);
			}
			else
				runtime_error = RERR_INVALID_ARGUMENTS;
		}
		else
			runtime_error = RERR_INVALID_ARGUMENTS;
		break;

	case RTSYM_FEOF:
		if (n == 1)
		{
			valderef(0);
			if (typeget(0) == TYPE_NUMBER)
			{
				char fid = valget(0) >> 16;
				char status = KRNIO_NODEVICE;
				if (filemask & (1 << fid))
					status = krnio_pstatus[fid + 1];
				
				esp += 2;
				valpush(TYPE_NUMBER, long(status) << 16);
			}
			else
				runtime_error = RERR_INVALID_ARGUMENTS;
		}
		else
			runtime_error = RERR_INVALID_ARGUMENTS;
		break;

	case RTSYM_CPUT:
		if (n == 4)
		{
			valderef(0);
			valderef(1);
			valderef(2);
			valderef(3);

			int x = valget(3) >> 16, y = valget(2) >> 16;
			char ch = valget(1) >> 16, co = valget(0) >> 16;

			if (x >= 0 && x < 40 && y >= 0 && y < 25)
			{
				unsigned	o = y * 40 + x;
				*(char *)(0x0400 + o) = ch;
				*(char *)(0xd800 + o) = co;
			}

			esp += 5;
			valpush(TYPE_NULL, 0);
		}
		else
			runtime_error = RERR_INVALID_ARGUMENTS;
		break;
	case RTSYM_CGET:
		if (n == 2)
		{
			valderef(0);
			valderef(1);

			int x = valget(1) >> 16, y = valget(0) >> 16;
			char ch = 0;

			if (x >= 0 && x < 40 && y >= 0 && y < 25)
			{
				unsigned	o = y * 40 + x;
				ch = *(char *)(0x0400 + o);
			}

			esp += 3;
			valpush(TYPE_NUMBER, long(ch) << 16);
		}
		else
			runtime_error = RERR_INVALID_ARGUMENTS;
		break;
	case RTSYM_CFILL:
		if (n == 6)
		{
			for(char i=0; i<6; i++)
				valderef(i);

			int x0 = valget(5) >> 16, y0 = valget(4) >> 16;
			int x1 = x0 + (valget(3) >> 16), y1 = y0 + (valget(2) >> 16);
			char ch = valget(1) >> 16, co = valget(0) >> 16;

			if (x0 < 0) x0 = 0;
			if (y0 < 0) y0 = 0;
			if (x1 > 40) x1 = 40;
			if (y1 > 25) y1 = 25;

			if (x1 > x0 && y1 > y0)
			{
				unsigned	o = y0 * 40 + x0;

				char * chp = (char *)(0x0400 + o), * cop = (char *)(0xd800 + o);

				char w = x1 - x0, h = y1 - y0;
				for(char y=0; y<h; y++)
				{
					for(char x=0; x<w; x++)
					{
						chp[x] = ch;
						cop[x] = co;
					}
					chp += 40;
					cop += 40;
				}
			}

			esp += 7;
			valpush(TYPE_NULL, 0);
		}
		else
			runtime_error = RERR_INVALID_ARGUMENTS;
		break;
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
	char tmp1[4], tmp2[4];

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
				else if (t == TK_ADD)
				{
					if ((tm & TYPE_MASK) == TYPE_STRING)
					{
						const char * s2 = valstring(0, tmp1);
						const char * s1 = valstring(1, tmp2);

						MemString	*	ms = string_allocate(s1[0] + s2[0]);
						if (ms)
						{
							char * dstr = ms->data;

							const char * s2 = valstring(0, tmp1);
							const char * s1 = valstring(1, tmp2);

							memcpy(dstr + 1, s1 + 1, s1[0]);
							memcpy(dstr + 1 + s1[0], s2 + 1, s2[0]);
							dstr[0] = s1[0] + s2[0];

							esp += 2;
							valpush(TYPE_STRING_HEAP, (unsigned)ms);
						}
					}
					else if (typeget(0) == TYPE_ARRAY && typeget(1) == TYPE_ARRAY)
					{
						MemArray * ma = (MemArray *)valmem(1);
						MemArray * mb = (MemArray *)valmem(0);

						int size = ma->size + mb->size;
						MemArray	*	mn = array_allocate(size, size);
						if (mn)
						{
							ma = (MemArray *)valmem(1);
							mb = (MemArray *)valmem(0);

							MemValues	*	mnv = (MemValues *)(mn + 1);
							MemValues	*	mav = (MemValues *)ma->mh;
							MemValues	*	mbv = (MemValues *)mb->mh;

							memcpy(mnv->values, mav->values, sizeof(Value) * ma->size);
							memcpy(mnv->values + ma->size, mbv->values, sizeof(Value) * mb->size);

							esp+=2;
							valpush(TYPE_ARRAY, (unsigned)mn);
						}
					}
					else
						runtime_error = RERR_INVALID_TYPES;
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
					const char * s2 = valstring(0, tmp1);
					const char * s1 = valstring(1, tmp2);
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
			case TK_LENGTH:
				{
					valderef(0);
					char t = typeget(0);
					if ((t & TYPE_MASK) == TYPE_STRING)
					{
						const char * str = valstring(0, tmp1);
						esp ++;
						valpush(TYPE_NUMBER, (unsigned long)str[0] << 16);
					}
					else if (t == TYPE_ARRAY)
					{
						MemArray * ma = (MemArray *)valmem(0);
						esp ++;
						valpush(TYPE_NUMBER, (unsigned long)ma->size << 16);
					}
					else
					{
						esp ++;
						valpush(TYPE_NUMBER, 0);
					}

				} break;
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
					char et = typeget(0);
					long ei = valpop();
					valderef(0);
					char t = typeget(0);
					if (t == TYPE_ARRAY)
					{
						if (et == TYPE_NUMBER)
						{
							estack[esp].type = TYPE_ARRAY_REF;
							estack[esp].value |= ei & 0xffff0000ul;
						}
						else if (et == TYPE_RANGE)
						{
							int start = ei >> 16;
							int end = ei + 1;

							MemArray * ma = (MemArray *)valmem(0);

							if (start < 0)
								start = 0;
							if (end > ma->size)								
								end = ma->size;

							if (end > start)
							{
								MemArray	*	mn = array_allocate(end - start, end - start);
								ma = (MemArray *)valmem(0);

								MemValues	*	mv = (MemValues *)ma->mh;
								MemValues	*	mnv = (MemValues *)(mn + 1);

								memcpy(mnv->values, mv->values + start, sizeof(Value) * (end - start));

								esp++;
								valpush(TYPE_ARRAY, (unsigned)mn);
							}
							else
							{
								MemArray	*	mn = array_allocate(0, 0);
								esp++;
								valpush(TYPE_ARRAY, (unsigned)mn);
							}
						}
						else
							runtime_error = RERR_INVALID_TYPES;					
					}
					else if ((t & TYPE_MASK) == TYPE_STRING)
					{						
						const char * s = valstring(0, tmp1);
						esp++;

						if (et == TYPE_NUMBER)
						{
							int si = ei >> 16;
							if (si >= 0 && si < s[0])
								valpush(TYPE_STRING_SHORT, 1 | ((unsigned)s[si + 1] << 8));
							else
								valpush(TYPE_STRING_SHORT, 0);
						}
						else if (et == TYPE_RANGE)
						{
							int start = ei >> 16;
							int end = ei + 1;

							if (start < 0)
								start = 0;
							if (end > s[0])
								end = s[0];
							if (end > start)
								valpushstring(s + 1 + start, end - start);
							else
								valpush(TYPE_STRING_SHORT, 0);
						}
						else
							runtime_error = RERR_INVALID_TYPES;					
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
							callstack[csp].etk = execetk;
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
				case TK_DOTDOT:
					{
						unsigned long i1 = valpop() >> 16;
						unsigned long i0 = valpop() >> 16;

						valpush(TYPE_RANGE, (i0 << 16) | i1);
					}	break;
				case TK_STRING:
				case TK_BYTES:
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
	case STMT_BREAK:
		while (csp > 0)
		{
			csp--;
			switch (callstack[csp].type)
			{
			case CSTACK_WHILE:
				tk = callstack[csp].tk;
				exectk = (char *)(tk[2] + (tk[3] << 8));
				return true;
			case CSTACK_CALL:
				runtime_error = RERR_INVALID_BREAK;
				return true;
			} 
		}
		runtime_error = RERR_INVALID_BREAK;
		return true;
	case STMT_EXIT:
		return false;
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
		execetk = etk;

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
				etk =  callstack[csp].etk;
				callstack[csp].type = CSTACK_NONE;
				t = etk[1];
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
			break;

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

