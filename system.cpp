#include "system.h"
#include <string.h>
#include <c64/memmap.h>
#include <c64/vic.h>

static char * const Font = (char *)0xd000;

void system_init(void)
{
	mmap_trampoline();
	mmap_set(MMAP_NO_BASIC);
}

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
		0b00110000,
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
	}
};

void system_show_editor(void)
{
	mmap_set(MMAP_CHAR_ROM);
	memcpy(Font, Font + 0x0800, 0x0800);
	for(char i=0; i<6; i++)
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
	mmap_set(MMAP_NO_ROM);

	vic_setmode(VICM_TEXT, Screen, Font);

	vic.color_border = VCOL_BLACK;
	vic.color_back = VCOL_BLACK;	
}

void system_show_runtime(void)
{
	vic_setmode(VICM_TEXT, (char *)0x0400, (char *)0x1000);

	vic.color_border = VCOL_LT_BLUE;
	vic.color_back = VCOL_BLUE;

	mmap_set(MMAP_NO_BASIC);
	clrscr();
	mmap_set(MMAP_NO_ROM);
}

void system_putch(char ch)
{
	mmap_set(MMAP_NO_BASIC);
	putch(ch);
	mmap_set(MMAP_NO_ROM);
}

char system_getch(void)
{	
	mmap_set(MMAP_NO_BASIC);
	char ch = getch();
	mmap_set(MMAP_NO_ROM);	
	return ch;
}

char system_getchx(void)
{	
	mmap_set(MMAP_NO_BASIC);
	char ch = getchx();
	mmap_set(MMAP_NO_ROM);	
	return ch;
}
