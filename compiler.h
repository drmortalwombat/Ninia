#pragma once

#include "system.h"

#pragma code(tcode)

__noinline void prepare_statements(char * tk);

__noinline void restore_statements(char * tk);

#pragma code(code)

#pragma code(ecode)

char token_skip_expression(const char * tk);

char token_skip_statement(const char * tk);

#pragma code(code)

#pragma compile("compiler.cpp")
