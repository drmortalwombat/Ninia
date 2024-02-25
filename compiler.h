#pragma once

#include "system.h"

#pragma code(tcode)

__noinline void prepare_statements(char * tk);

__noinline void restore_statements(char * tk);

__noinline const char * token_skip_statement(const char * tk);

__noinline char * token_skip_statement(char * tk);

__noinline unsigned token_statement_size(const char * tk);

__noinline char * edit_screen_to_token(char y);

__noinline unsigned edit_token_to_line(const char * c);

__noinline char * edit_line_to_token(unsigned y);

__noinline bool tokens_import(const char * name);

__noinline bool tokens_load(const char * name);

__noinline bool tokens_save(const char * name);


#pragma code(code)

#pragma compile("compiler.cpp")
