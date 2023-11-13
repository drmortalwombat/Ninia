#include "variables.h"


Variable	globals[256];
unsigned	num_globals;

unsigned global_find(unsigned symbol)
{
	unsigned	vi = 0;
	while (vi < num_globals && globals[vi].symbol != symbol)
		vi++;
	return vi;
}

unsigned global_add(unsigned symbol)
{
	globals[num_globals].symbol = symbol;
	globals[num_globals].value = 0;
	return num_globals++;
}

