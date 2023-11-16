#pragma once
#include "tokens.h"

char * parse_statement(const char * str, char * tk);

void parse_pretty(char * tk);

#pragma compile("parser.cpp")
