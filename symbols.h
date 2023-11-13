#pragma once

void symbols_init(void);

// Symbol range 
//		0.. 255 single character symbol
// 	  256.. 511 system defined symbols
//    512..1023 user defined symbols

#define RTSYM_ABS		0x100
#define	RTSYM_RAND		0x101
#define RTSYM_CHROUT	0x102

unsigned symbol_add(const char * s);

const char * symbol_string(unsigned id);

#pragma compile("symbols.cpp")
