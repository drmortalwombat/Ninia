#pragma once

#include "system.h"
#include "runtime.h"

#define VALUE_STACK_SIZE	64
#define CALL_STACK_SIZE		32

#define CSTACK_NONE		0
#define CSTACK_WHILE	1
#define CSTACK_FOR		2
#define CSTACK_CALL		3


extern __striped Value		estack[VALUE_STACK_SIZE];
extern char				esp, efp;

extern const char * exectk;

#pragma code(rcode)

__noinline void interpreter_init(char * tk);

__noinline void interpret_program(void);

#pragma code(code)

#pragma compile("interpreter.cpp")

