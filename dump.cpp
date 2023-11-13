#include "dump.h"
#include "tokens.h"
#include "symbols.h"

const char * dump_expression(const char * tk)
{
	char t = *tk++;
	while (t != TK_END)
	{
		printf("[%02x]:", t);

		switch (t & 0xf0)
		{
		case TK_TINY_INT:
			printf("%u", t & 0x0f);
			break;
		case TK_SMALL_INT:
			printf("%u", ((t & 0x0f) << 8) | *tk++);
			break;
		case TK_NUMBER:
			break;

		case TK_IDENT:
			{
				unsigned ti = ((t & 0x0f) << 8) | *tk++;
				printf("%s", symbol_string(ti));
			}	break;

		case TK_CONST:	
		case TK_GLOBAL:	
		case TK_LOCAL:
			{
				unsigned ti = ((t & 0x0f) << 8) | *tk++;
				printf("%s", symbol_string(globals[ti].symbol));
			}	break;

		case TK_BINARY:
			switch (t)
			{
			case TK_ADD:
				printf("+");
				break;
			case TK_SUB:
				printf("-");
				break;
			case TK_MUL:
				printf("*");
				break;
			case TK_DIV:
				printf("/");
				break;
			case TK_MOD:
				printf("%%");
				break;
			case TK_SHL:
				printf("<<");
				break;
			case TK_SHR:
				printf(">>");
				break;
			case TK_AND:
				printf("&");
				break;
			case TK_OR:
				printf("|");
				break;
			}
			break;
		case TK_RELATIONAL:
			switch (t)
			{
			case TK_EQUAL:
				printf("==");
				break;
			case TK_NOT_EQUAL:
				printf("!=");
				break;
			case TK_LESS_THAN:
				printf("<");
				break;
			case TK_LESS_EQUAL:
				printf("<=");
				break;
			case TK_GREATER_THAN:
				printf(">");
				break;
			case TK_GREATER_EQUAL:
				printf(">=");
				break;					
			}
			break;
		case TK_PREFIX:
			switch (t)
			{
			case TK_NEGATE:
				printf("-");
				break;
			case TK_NOT:
				printf("!");
				break;
			}
			break;

		case TK_POSTFIX:
			switch (t)
			{
			case TK_INDEX:
				printf("a[%d]", *tk++);
				break;
			case TK_DOT:
				printf("a.%d", *tk++);
				break;
			case TK_INVOKE:
				printf("a(%d)", *tk++);
				break;
			}
			break;

		case TK_ASSIGN:
			printf("=");
			break;
		case TK_STRUCTURE:
			switch (t)
			{
			case TK_LIST:
				printf("(%d)", *tk++);
				break;
			case TK_ARRAY:
				printf("[%d]", *tk++);
				break;
			case TK_STRUCT:
				printf("{%d}", *tk++);
				break;
			}
			break;

		case TK_CONTROL:
			switch (t)
			{
			case TK_END:
				printf(";");
				break;
			case TK_COMMA:
				printf(",");
				break;
			}
			break;

		}
		printf("\n");
		t = *tk++;
	}
	return tk;
}

const char * dump_statement(const char * tk)
{
	char l = *tk++;
	printf("STMT(%d) ", l);
	char t = *tk++;
	switch (t)
	{
	case STMT_EXPRESSION:
		printf("EXPRESSION\n");
		return dump_expression(tk);
	case STMT_WHILE:
		printf("WHILE\n");
		return dump_expression(tk + 2);
	case STMT_IF:
		printf("IF\n");
		return dump_expression(tk + 2);
	case STMT_ELSIF:
		printf("ELSIF\n");
		return dump_expression(tk + 2);
	case STMT_ELSE:
		printf("ELSE\n");
		return tk + 2;
	case STMT_VAR:
		printf("VAR\n");
		return dump_expression(tk);
	case STMT_NONE:
		printf("NONE\n");
		return tk;
	case STMT_ERROR:
		{
			char n = *tk++;
			printf("ERROR %d [", n);
			while (n)
			{
				printf("%c", *tk++);
				n--;
			}
			printf("]\n");

			return tk;
		}
	}

	return tk;
}
