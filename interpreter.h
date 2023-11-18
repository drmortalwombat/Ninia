#pragma once

#include "variables.h"

#define VALUE_STACK_SIZE	64

void prepare_statements(char * tk);

void restore_statements(char * tk);

extern __striped Value		estack[VALUE_STACK_SIZE];
extern char				esp, efp;

extern const char * exectk;

bool interpret_statement(void);

#pragma compile("interpreter.cpp")

