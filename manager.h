#pragma once

#include "system.h"
#include "editor.h"

extern edit_cmd_t	filename;
extern char			sysdrive;

__noinline void manager_init(void);

__noinline void manager_invoke(void);

#pragma compile("manager.cpp")
