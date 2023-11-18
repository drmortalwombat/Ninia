#include "symbols.h"

#define NUM_SYMBOLS		256

struct Symbol
{
	char		s[8];	
}	symbols[NUM_SYMBOLS];

const char *	rtsymbols[]  =
{
	p"abs",
	p"rand",
	p"chrout",
	p"print",
	p"poke",
	p"peek",

	p"len",
	p"cat",
	p"chr",
	p"seg",
	p"array",
	p"push",
	p"pop",

};

#define NUM_RTSYMBOLS	(sizeof(rtsymbols) / sizeof(char *))

void symbols_init(void)
{
	for(int i=0; i<NUM_SYMBOLS; i++)
		symbols[i].s[0] = 0;
}

unsigned symbol_add(const char * s)
{
	if (!s[1])
		return s[0];

	for (char i = 0; i<NUM_RTSYMBOLS; i++)
	{
		if (!strcmp(rtsymbols[i], s))
			return i  + 256;
	}

	for(int i=0; i<NUM_SYMBOLS; i++)
	{
		char	*	sp = symbols[i].s;
		char j = 0;
		for(;;)
		{
			if (s[j] != sp[j])
				break;
			if (s[j] == 0)
				return i + 512;
			j++;
			if (j == 8)
				return i + 512;
		}
	}

	for(int i=0; i<NUM_SYMBOLS; i++)
	{
		char	*	sp = symbols[i].s;
		if (!sp[0])
		{
			char j = 0;
			while (j < 7 && s[j])
			{
				sp[j] = s[j];
				j++;
			}
			sp[j] = s[j];
			return i + 512;
		}
	}

	return NUM_SYMBOLS;
}

const char * symbol_string(unsigned id)
{
	static char buf[2];
	if (id < 256)
	{
		buf[0] = id;
		buf[1] = 0;
		return buf;
	}
	else if (id < 512)
		return rtsymbols[id & 0xff];
	else
		return symbols[id - 512].s;
}
