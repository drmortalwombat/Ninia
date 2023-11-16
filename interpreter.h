#pragma once


void prepare_statements(char * tk);

void restore_statements(char * tk);

extern const char * exectk;

bool interpret_statement(void);

#pragma compile("interpreter.cpp")

