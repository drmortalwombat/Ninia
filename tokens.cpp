#include "tokens.h"

char token_skip_expression(const char * tk)
{
	char	ti = 0;
	char 	t = tk[ti];

	while (t != TK_END)
	{
		switch (t & 0xf0)
		{
		case TK_SMALL_INT:
			ti += 2;
			break;
		case TK_NUMBER:
			ti += 5;
			break;

		case TK_IDENT:
		case TK_CONST:	
		case TK_GLOBAL:	
		case TK_LOCAL:
			ti += 2;
			break;

		case TK_POSTFIX:
		case TK_STRUCTURE:
			ti += 2;
			break;

		case TK_CONTROL:
			if (t == TK_STRING)
				ti += tk[ti + 1] + 2;
			else
				ti++;
			break;

		default:
			ti++;
		}

		t = tk[ti];
	}

	return ti + 1;
}

char token_skip_statement(const char * tk)
{
	char ti = 0;
	if (tk[0])
	{
		ti++;
		char t = tk[ti++];
		switch (t)
		{
		case STMT_DEF:
		case STMT_WHILE:
		case STMT_IF:
		case STMT_ELSIF:
			ti += 2;
			ti += token_skip_expression(tk + ti);
			break;
		case STMT_ELSE:
			ti += 2;
			break;
		case STMT_EXPRESSION:
		case STMT_VAR:
		case STMT_RETURN:
			ti += token_skip_expression(tk + ti);
			break;
		case STMT_NONE:
		case STMT_RETURN_NULL:
			break;
		case STMT_ERROR:
			ti += tk[ti] + 1;
			break;
		}
	}

	return ti;	
}

