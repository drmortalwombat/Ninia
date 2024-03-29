#pragma once

#include "system.h"

extern char * starttk, * limittk;

extern char cursorx, cursory, screenx;
extern char * screentk, * cursortk, * endtk, * marktk, * blocktk;
extern unsigned screeny;
extern bool tkmodified;

extern char	buffer[200], cbuffer[200];

#pragma code(ecode)

const char * edit_display_line(char y, const char * tk);

void edit_refresh_screen(void);

__noinline void edit_show_status(void);

void edit_scroll_up(void);

void edit_scroll_down(void);

char edit_line(void);

struct edit_cmd_t
{
	char	name[4];
	char	cmd[17];
};

__noinline void edit_restart(void);

__noinline void edit_init(void);

__noinline char edit_text(void);

__noinline bool edit_cmd(edit_cmd_t & ec);

#pragma code(code)

#pragma compile("editor.cpp")
