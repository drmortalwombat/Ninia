#pragma once


void prepare_statements(char * tk);

void restore_statements(char * tk);


const char * interpret_statement(const char * etk);

#pragma compile("interpreter.cpp")

