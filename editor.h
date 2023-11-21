#pragma once

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

char edit_line(void);

char * edit_screen_to_token(char y);

unsigned edit_token_to_line(const char * c);

char * edit_line_to_token(unsigned y);

#pragma compile("editor.cpp")
