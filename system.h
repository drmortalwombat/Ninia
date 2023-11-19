#pragma once

static char * const Screen = (char *)0xcc00;
static char * const Color = (char *)0xd800;


void system_init(void);

void system_show_editor(void);

void system_show_runtime(void);



#pragma compile("system.cpp")
