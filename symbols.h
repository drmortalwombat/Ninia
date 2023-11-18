#pragma once

void symbols_init(void);

// Symbol range 
//		0.. 255 single character symbol
// 	  256.. 511 system defined symbols
//    512..1023 user defined symbols

#define RTSYM_ABS		0x100
#define	RTSYM_RAND		0x101
#define RTSYM_CHROUT	0x102
#define RTSYM_PRINT		0x103
#define RTSYM_POKE		0x104
#define RTSYM_PEEK		0x105

#define RTSYM_LEN		0x106
#define RTSYM_CAT		0x107
#define RTSYM_CHR		0x108
#define RTSYM_SEG		0x109
#define RTSYM_ARRAY		0x10a
#define RTSYM_PUSH		0x10b
#define RTSYM_POP		0x10c

unsigned symbol_add(const char * s);

const char * symbol_string(unsigned id);

#pragma compile("symbols.cpp")
