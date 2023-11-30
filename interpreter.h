#pragma once

#include "runtime.h"

#define VALUE_STACK_SIZE	64
#define CALL_STACK_SIZE		32

__noinline void prepare_statements(char * tk);

__noinline void restore_statements(char * tk);

extern __striped Value		estack[VALUE_STACK_SIZE];
extern char				esp, efp;

extern const char * exectk;

__noinline bool interpret_statement(void);

#pragma compile("interpreter.cpp")

