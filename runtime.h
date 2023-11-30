#pragma once

#include "tokens.h"

extern __striped Value	globals[256];

extern __striped unsigned			global_symbols[256];
extern __striped unsigned			local_symbols[256];

extern char	num_globals, num_local_symbols;

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

struct MemDict : MemHead
{
	MemHead		*	mh;
	const char	*	symbols;
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
#define MEM_DICT		0x04

#define MEM_TYPE		0x0f

#define MEM_FLAGS		0xc0
#define MEM_CHECKED		0x40
#define MEM_REFERENCED	0x80

MemHead * mem_allocate(char type, unsigned size);

#pragma compile("runtime.cpp")
