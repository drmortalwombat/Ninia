#pragma once

const char * format_statement(const char * tk, char * str, char * col);

const char * format_skip_statement(const char * tk);

#pragma compile("formatter.cpp")
