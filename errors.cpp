#include "errors.h"

#pragma code( ecode )

const char * const runtime_error_names[NUM_ERRORS] =
{
	p"Ok",
	p"Syntax error",
	p"Invalid types",
	p"Stack underflow",
	p"Undefined symbol",
	p"Out of memory",
	p"Invalid assign",
	p"Invalid break",
	p"Invalid arguments"
};

RuntimeError	runtime_error;
