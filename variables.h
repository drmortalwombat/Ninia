#pragma once

#include "tokens.h"


struct Variable
{
	unsigned	symbol;
	Value		v;
};

extern __striped Variable	globals[256];

extern char	num_globals;

unsigned global_find(unsigned symbol);

unsigned global_add(unsigned symbol);

#pragma compile("variables.cpp")
