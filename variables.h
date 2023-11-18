#pragma once

#include "tokens.h"

enum RuntimeError
{
	RERR_OK,
	RERR_INVALID_TYPES,
	RERR_STACK_UNDERFLOW,
	RERR_UNDEFINED_SYMBOL,
	RERR_OUT_OF_MEMORY,
	RERR_INVALID_ASSIGN
};

extern RuntimeError	runtime_error;


extern __striped Value	globals[256];

extern unsigned			global_symbols[256];
extern unsigned			local_symbols[256];

extern char	num_globals, num_local_symbols;

unsigned global_find(unsigned symbol);

unsigned global_add(unsigned symbol);

struct MemHead
{
	char		type;
	unsigned	moved;
};

struct MemString : MemHead
{
	char		data[1];
};

struct MemArray : MemHead
{
	MemHead	*	mh;
	unsigned	size;
};

struct MemValues : MemHead
{
	unsigned	capacity;
	Value		values[0];
};

void mem_init(void);

void mem_collect(void);

#define MEM_STRING		0x01
#define MEM_ARRAY		0x02
#define MEM_VALUES		0x03

#define MEM_TYPE		0x0f

#define MEM_FLAGS		0xc0
#define MEM_CHECKED		0x40
#define MEM_REFERENCED	0x80

MemHead * mem_allocate(char type, unsigned size);

#pragma compile("variables.cpp")
