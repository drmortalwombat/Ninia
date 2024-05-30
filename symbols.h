#pragma once

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

#define RTSYM_VSYNC		0x06
#define RTSYM_CAT		0x07
#define RTSYM_CHR		0x08
#define RTSYM_SHIFT		0x09
#define RTSYM_ARRAY		0x0a
#define RTSYM_PUSH		0x0b
#define RTSYM_POP		0x0c
#define RTSYM_ASC		0x0d
#define RTSYM_VAL		0x0e
#define RTSYM_STR		0x0f
#define RTSYM_FLOOR		0x10
#define RTSYM_CEIL		0x11
#define RTSYM_FIND		0x12
#define RTSYM_TIME		0x13
#define RTSYM_CHRIN		0x14
#define RTSYM_INPUT		0x15

#define RTSYM_FOPEN		0x16
#define RTSYM_FCLOSE	0x17
#define RTSYM_FGET		0x18
#define RTSYM_FPUT		0x19
#define RTSYM_FEOF		0x1a

#define RTSYM_CPUT		0x1b
#define RTSYM_CGET		0x1c
#define RTSYM_CFILL		0x1d
#define RTSYM_CMOVE		0x1e

#pragma code(tcode)

__noinline void symbols_init(void);

unsigned symbol_add(const char * s);

const char * symbol_string(unsigned id);

#pragma code(code)

#pragma compile("symbols.cpp")
