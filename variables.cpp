#include "variables.h"


__striped Variable		globals[256];
char					num_globals;

unsigned global_find(unsigned symbol)
{
	char	vi = 0;
	while (vi < num_globals && globals[vi].symbol != symbol)
		vi++;
	return vi;
}

unsigned global_add(unsigned symbol)
{
	globals[num_globals].symbol = symbol;
	globals[num_globals].v.value = 0;
	globals[num_globals].v.type = TYPE_NULL;
	return num_globals++;
}

