#include "errors.h"


const char * runtime_error_names[NUM_ERRORS] =
{
	p"Ok",
	p"Syntax error",
	p"Invalid types",
	p"Stack underflow",
	p"Undefined symbol",
	p"Out of memory",
	p"Invalid assign",
};

RuntimeError	runtime_error;
