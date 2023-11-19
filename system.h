#pragma once

static char * const Screen = (char *)0xcc00;
static char * const Color = (char *)0xd800;

#pragma section( editbss, 0, , , bss)

#pragma region( editbss , 0xc000, 0xcc00, , , {editbss} )

#pragma section( rtbss, 0, , , bss)

#pragma region( rtbss , 0xc000, 0xd000, , , {rtbss} )


void system_init(void);

void system_show_editor(void);

void system_show_runtime(void);

void system_putch(char ch);

char system_getch(void);

char system_getchx(void);

#pragma compile("system.cpp")
