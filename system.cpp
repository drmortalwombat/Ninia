#include "system.h"
#include <string.h>
#include <c64/memmap.h>
#include <c64/vic.h>
#include <c64/sid.h>
#include <c64/easyflash.h>

#pragma stacksize( 1024 )
#pragma heapsize( 0 )


#pragma section( bcode, 0 )
#pragma region( brom, 0x8080, 0x80a0, , 0, { bcode } )

#pragma section( bvector, 0 )
#pragma region( bvector, 0xbffc, 0xbffe, , 0, { bvector } )

#pragma data ( bvector )

#pragma data ( bvector )

__export const unsigned bvector[1] = {0x8080};

#pragma code ( bcode )

__export void bcode(void)
{
	__asm
	{
		lda #$87
		sta $de02
		jmp ($fffc)
	}
}

#pragma code( mcode )
#pragma data( mdata )

void kernal_init(void)
{
	__asm
	{
		jsr	$fda3
		jsr	$fd50
		jsr	$fd15
		jsr	$ff5b
		cli
	}
}

void ninia_main(void);


int main(void)
{
	mmap_set(MMAP_ROM);

	kernal_init();
	mmap_trampoline();

	ninia_main();

	return 0;
}

static char * const Font = (char *)0xd000;
static char * const Sprites = (char *)0xd800;

struct FontPatch
{
	char c;
	char m[8];
};

static FontPatch fontpatch[] = {
	{
		s'\\', {
		0b00000000,
		0b11000000,
		0b01100000,
		0b00110000,
		0b00011000,
		0b00001100,
		0b00000110,
		0b00000000}
	},{
		s'^', {
		0b00000000,
		0b00011000,
		0b00111100,
		0b01100110,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000}
	},{
		s'_', {
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b11111110,
		0b00000000}
	},{
		s'{', {
		0b00001100,
		0b00011000,
		0b00011000,
		0b01110000,
		0b00011000,
		0b00011000,
		0b00001100,
		0b00000000}
	},{
		s'|', {
		0b00110000,
		0b00110000,
		0b00110000,
		0b00000000,
		0b00110000,
		0b00110000,
		0b00110000,
		0b00000000}
	},{
		s'}', {
		0b01100000,
		0b00110000,
		0b00110000,
		0b00011100,
		0b00110000,
		0b00110000,
		0b01100000,
		0b00000000}
	},{
		96, {
		0b00010000,
		0b00000000,
		0b00010000,
		0b00000000,
		0b00010000,
		0b00000000,
		0b00010000,
		0b00000000}			
	},{
		107, {
		0b11000000,
		0b11100000,
		0b11110000,
		0b11111000,
		0b11110000,
		0b11100000,
		0b11000000,
		0b00000000}
	}
};

void system_show_editor(void)
{
	__asm
	{
		sei
	}
	mmap_set(MMAP_ALL_ROM);
	memcpy(Font, Font + 0x0800, 0x0800);
	for(char i=0; i<8; i++)
	{
		char * dp0 = Font + 8 * fontpatch[i].c;
		char * dp1 = dp0 + 0x0400;
		const char * sp = fontpatch[i].m;

		for(char j=0; j<8; j++)
		{
			dp0[j] = sp[j];
			dp1[j] = ~sp[j];
		}
	}
	memset(Sprites, 0, 64);
	for(char i=0; i<10; i++)
		Sprites[3 * i] = 0xe0;
	mmap_set(MMAP_ROM);
	__asm
	{
		cli
	}

	vic_setmode(VICM_TEXT, Screen, Font);

	Screen[0x3f8] = 96;
	vic.spr_enable = 0;
	vic.spr_multi = 0;
	vic.spr_expand_x = 0;
	vic.spr_expand_y = 0;
	vic.spr_color[0] = VCOL_WHITE;
	vic.spr_priority = 1;
	vic.spr_pos[0].y = 0;
	vic.spr_pos[0].x = 0;
	vic.spr_msbx = 0;

	vic.color_border = VCOL_BLACK;
	vic.color_back = VCOL_BLACK;	
	sid.fmodevol = 0;
}

void system_show_runtime(void)
{
	vic_setmode(VICM_TEXT, (char *)0x0400, (char *)0x1000);

	vic.color_border = VCOL_LT_BLUE;
	vic.color_back = VCOL_BLUE;

	clrscr();
}

#pragma code( code )
#pragma data( data )

char system_bank_read(char back, char bank, volatile const char * p)
{
	eflash.bank = bank;
	char c = *p;
	eflash.bank = back;	
	return c;
}

void system_putch(char ch)
{
	__asm
	{
		lda ch
		jsr $ffd2
	}
}

char system_readch(void)
{	
	__asm
	{
		jsr $ffcf
		sta accu
	}
}

char system_getch(void)
{	
	__asm
	{
	l1:
		jsr $ffe4
		beq l1
		sta accu
	}
}

char system_getchx(void)
{	
	__asm
	{
		jsr $ffe4
		sta accu
	}
}
