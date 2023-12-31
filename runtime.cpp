#include "runtime.h"
#include "editor.h"
#include "interpreter.h"
#include "errors.h"

#pragma code( rcode )
#pragma data( rdata )
#pragma bss( rtbss )

__striped Value			globals[256];
__striped unsigned		global_symbols[256];
__striped unsigned		local_symbols[256];

#pragma bss( bss )

char					num_globals, num_local_symbols;

unsigned	mem_end;
unsigned	mem_start;

static const unsigned mem_final = 0x7000;

void mem_init(void)
{
	mem_start = (unsigned)endtk;
	mem_end = mem_start;
}

unsigned mh_size(MemHead * mh)
{
	switch (mh->type & MEM_TYPE)
	{
	case MEM_STRING:
		return ((MemString *)mh)->data[0] + sizeof(MemString);
	case MEM_ARRAY:
		return sizeof(MemArray);
	case MEM_VALUES:
		return ((MemValues *)mh)->capacity * sizeof(Value) + sizeof(MemValues);
	case MEM_DICT:
		return sizeof(MemDict);
	default:
		return sizeof(MemHead);
	}
}
inline MemHead * mh_next(MemHead * mh)
{
	return (MemHead *)((char *)mh + mh_size(mh));
}

void mem_collect(void)
{
	// Reset mark

	MemHead	* mh = (MemHead *)mem_start;
	while (mh != (MemHead *)mem_end)
	{
		mh->type &= MEM_TYPE;
		mh = mh_next(mh);
	}

	// Mark globals

	for(char i=0; i<num_globals; i++)
	{
		if (globals[i].type & TYPE_HEAP)
			((MemHead *)globals[i].value)->type |= MEM_REFERENCED;
	}

	// Mark locals

	for(char i=esp; i<VALUE_STACK_SIZE; i++)
	{
		if (estack[i].type & TYPE_HEAP)
			((MemHead *)estack[i].value)->type |= MEM_REFERENCED;
	}

	// Recursive mark elements

	bool	changed;
	do {
		changed = false;
		mh = (MemHead *)mem_start;
		while (mh != (MemHead *)mem_end)
		{
			if ((mh->type & MEM_FLAGS) == MEM_REFERENCED)
			{
				mh->type |= MEM_CHECKED;
				switch (mh->type & MEM_TYPE)
				{
				case MEM_STRING:
					break;
				case MEM_DICT:
					((MemDict *)mh)->mh->type |= MEM_REFERENCED;
					changed = true;
					break;
				case MEM_ARRAY:
					((MemArray *)mh)->mh->type |= MEM_REFERENCED;
					changed = true;
					break;
				case MEM_VALUES:
					{
						MemValues	*	mv = (MemValues *)mh;
						unsigned s = mv->capacity;
						for(unsigned i=0; i<s; i++)
						{
							if (mv->values[i].type & TYPE_HEAP)
								((MemHead *)mv->values[i].value)->type |= MEM_REFERENCED;
						}
						changed = true;
					}	break;
				}
			}
			mh = mh_next(mh);
		}
	} while (changed);

	// Calculate new position

	unsigned	mem = mem_start;
	mh = (MemHead *)mem_start;
	while (mh != (MemHead *)mem_end)
	{
		if (mh->type & MEM_REFERENCED)
		{
			mh->moved = mem;
			mem += mh_size(mh);
		}
		mh = mh_next(mh);
	}

	// Update globals

	for(char i=0; i<num_globals; i++)
	{
		if (globals[i].type == TYPE_HEAP)
			globals[i].value = ((MemHead *)globals[i].value)->moved;
	}

	// Update locals

	for(char i=esp; i<VALUE_STACK_SIZE; i++)
	{
		if (estack[i].type == TYPE_HEAP)
			estack[i].value = (estack[i].value & 0xffff0000ul) | ((MemHead *)estack[i].value)->moved;
	}

	// Update heap
	mh = (MemHead *)mem_start;
	while (mh != (MemHead *)mem_end)
	{
		if (mh->type & MEM_REFERENCED)
		{
			switch (mh->type & MEM_TYPE)
			{
			case MEM_STRING:
				break;
			case MEM_DICT:
				{
					MemDict	*	ma = (MemDict *)mh;
					ma->mh = (MemHead *)(ma->mh->moved);
				}
				changed = true;
				break;
			case MEM_ARRAY:
				{
					MemArray	*	ma = (MemArray *)mh;
					ma->mh = (MemHead *)(ma->mh->moved);
				}
				changed = true;
				break;
			case MEM_VALUES:
				{
					MemValues	*	mv = (MemValues *)mh;
					unsigned s = mv->capacity;
					for(unsigned i=0; i<s; i++)
					{
						if (mv->values[i].type & TYPE_HEAP)
							mv->values[i].value = ((MemHead *)mv->values[i].value)->moved;
					}
					changed = true;
				}	break;
			}
		}
		mh = mh_next(mh);
	}


	// Compress heap

	mh = (MemHead *)mem_start;
	while (mh != (MemHead *)mem_end)
	{
		MemHead * nh = mh_next(mh);
		if (mh->type & MEM_REFERENCED)
			memcpy((char *)mh->moved, mh, mh_size(mh));
		mh = nh;
	}

	mem_end = mem;
}

MemHead * mem_allocate(char type, unsigned size)
{
	if (mem_end + size > mem_final)
		mem_collect();

	if (mem_end + size > mem_final)
	{
		runtime_error = RERR_OUT_OF_MEMORY;
		return nullptr;
	}

	MemHead * mh = (MemHead *)mem_end;
	mem_end += size;
	mh->type = type;
	return mh;
}
