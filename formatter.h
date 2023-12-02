#pragma once

#pragma code(tcode)

const char * format_statement(const char * tk, char * str, char * col);

#pragma code(code)

#pragma compile("formatter.cpp")
