#pragma once

#include "system.h"
#include "tokens.h"

#pragma code(ecode)

char * parse_statement(const char * str, char * tk);

void parse_pretty(char * tk);

#pragma code(code)

#pragma compile("parser.cpp")
