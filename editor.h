#pragma once

static char * const Screen = (char *)0x0400;
static char * const Color = (char *)0xd800;

extern char cursorx, cursory, screenx;
extern char * screentk, * cursortk, * endtk;

const char * edit_display_line(char y, const char * tk);

void edit_refresh_screen(void);

char edit_line(void);

#pragma compile("editor.cpp")
