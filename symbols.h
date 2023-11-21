#pragma once

void symbols_init(void);

// Symbol range 
//		0.. 255 single character symbol
// 	  256.. 511 system defined symbols
//    512..1023 user defined symbols

#define RTSYM_ABS		0x00
#define	RTSYM_RAND		0x01
#define RTSYM_CHROUT	0x02
#define RTSYM_PRINT		0x03
#define RTSYM_POKE		0x04
#define RTSYM_PEEK		0x05

#define RTSYM_LEN		0x06
#define RTSYM_CAT		0x07
#define RTSYM_CHR		0x08
#define RTSYM_SEG		0x09
#define RTSYM_ARRAY		0x0a
#define RTSYM_PUSH		0x0b
#define RTSYM_POP		0x0c

unsigned symbol_add(const char * s);

const char * symbol_string(unsigned id);

#pragma compile("symbols.cpp")
