#pragma once

extern char * starttk, * limittk;

extern char cursorx, cursory, screenx;
extern char * screentk, * cursortk, * endtk;
extern unsigned screeny;

extern char	buffer[200], cbuffer[200];

void edit_init(void);

const char * edit_display_line(char y, const char * tk);

void edit_refresh_screen(void);

void edit_show_status(void);

char edit_line(void);

#pragma compile("editor.cpp")
