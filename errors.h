#pragma once


enum RuntimeError
{
	RERR_OK,
	RERR_SYNTAX,
	RERR_INVALID_TYPES,
	RERR_STACK_UNDERFLOW,
	RERR_UNDEFINED_SYMBOL,
	RERR_OUT_OF_MEMORY,
	RERR_INVALID_ASSIGN,
	RERR_INVALID_BREAK,

	NUM_ERRORS
};

extern const char * const runtime_error_names[NUM_ERRORS];

extern RuntimeError	runtime_error;



#pragma compile("errors.cpp")

