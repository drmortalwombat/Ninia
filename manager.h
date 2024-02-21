#pragma once

#include "system.h"
#include "editor.h"

extern edit_cmd_t	filename;
extern char			sysdrive;

void manager_init(void);

void manager_invoke(void);

#pragma compile("manager.cpp")
