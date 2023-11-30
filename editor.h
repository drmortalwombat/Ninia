#pragma once

#include "system.h"

extern char * starttk, * limittk;

extern char cursorx, cursory, screenx;
extern char * screentk, * cursortk, * endtk, * marktk, * blocktk;
extern unsigned screeny;

extern char	buffer[200], cbuffer[200];

void edit_init(void);

const char * edit_display_line(char y, const char * tk);

void edit_refresh_screen(void);

void edit_show_status(void);

void edit_scroll_up(void);

void edit_scroll_down(void);

char * edit_screen_to_token(char y);

unsigned edit_token_to_line(const char * c);

char * edit_line_to_token(unsigned y);


char edit_line(void);

__noinline char edit_text(void);

struct edit_cmd_t
{
	char	name[4];
	char	cmd[16];
};

__noinline bool edit_cmd(edit_cmd_t & ec);

__noinline bool tokens_load(const char * name);

__noinline bool tokens_save(const char * name);

#pragma compile("editor.cpp")
