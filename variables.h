#pragma once

extern struct Variable
{
	unsigned	symbol;
	long		value;
}	globals[256];
extern unsigned	num_globals;

unsigned global_find(unsigned symbol);

unsigned global_add(unsigned symbol);

#pragma compile("variables.cpp")
