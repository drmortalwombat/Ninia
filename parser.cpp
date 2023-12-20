#include "parser.h"
#include "errors.h"
#include "system.h"

#pragma code( tcode )
#pragma data( tdata )

char		ostack[32];
char		osp;
unsigned	symlist[32];

char oppri(char op)
{
	switch (op & 0xf0)
	{
	case TK_BINARY:
		switch (op)
		{
		case TK_MUL:
		case TK_DIV:
		case TK_MOD:
			return 10;
		case TK_SHL:
		case TK_SHR:
			return 11;			
		case TK_ADD:
		case TK_SUB:
			return 12;
		case TK_AND:
			return 25;
		case TK_OR:
			return 26;
		}
		return 0;

	case TK_RELATIONAL:
		return 20;
	case TK_ASSIGN:
		return 40;
	case TK_PREFIX:
		return 5;
	case TK_POSTFIX:
		return 0x80;

	case TK_CONTROL:
		switch (op)
		{
		case TK_END:
			return 0x7f;
		case TK_COLON:
			return 1;
		case TK_DOT:
			return 4;
		case TK_DOTDOT:
			return 30;
		case TK_COMMA:
			return 0x7f;
		case TK_LIST:
		case TK_ARRAY:
		case TK_STRUCT:
			return 0x80;
		default:
			return 0xff;
		}
		break;
	default:
		return 0xff;
	}
}

char parse_op(char * tk, char ni, char op)
{
	char pri = oppri(op);
	while (osp && oppri(ostack[osp-1]) <= pri)
	{
		tk[ni++] = ostack[osp-1];
		osp--;
	}
	
	ostack[osp++] = op;
	return ni;
}

char parse_comma(char * tk, char ni)
{
	while (osp && oppri(ostack[osp-1]) < 0x40)
	{
		tk[ni++] = ostack[osp-1];
		osp--;
	}
	if (osp)
		ostack[osp++] = TK_COMMA;
	else
		tk[ni++] = TK_COMMA;

	return ni;
}

char close_op(char * tk, char ni, bool prefix)
{
	int cnt = 0;
	if (!prefix)
		cnt++;

	while (osp && oppri(ostack[osp-1]) != 0x80)
	{
		if (ostack[osp-1] == TK_COMMA)
			cnt++;
		else
			tk[ni++] = ostack[osp-1];
		osp--;
	}

	osp--;
	tk[ni++] = ostack[osp];
	tk[ni++] = cnt;
	if (ostack[osp] == TK_STRUCT)
	{
		for(char i=0; i<cnt; i++)
		{
			tk[ni++] = symlist[osp + i] & 0xff;
			tk[ni++] = symlist[osp + i] >> 8;
		}
	}

	return ni;
}

char hex_value(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'a' && c <= 'f')
		return c + 10 - 'a';
	else if (c >= 'A' && c <= 'F')
		return c + 10 - 'A';
	else
		return 0;
}
char * parse_expression(const char * str, char * tk)
{
	osp = 0;

	char	si = 0;
	char 	c = str[si++];
	char	ni = 0;
	bool	prefix = true;
	for(;;)
	{
		switch (c)
		{
		case '\0':
			ni = parse_op(tk, ni, TK_END);
			if (osp != 1)
				return nullptr;
			tk[ni++] = TK_END;
			return tk + ni;

		case ' ':
		case 160:
			c = str[si++];
			break;			
		case '(':
			if (prefix)
				ostack[osp++] = TK_LIST;
			else
				ostack[osp++] = TK_INVOKE;
			c = str[si++];
			prefix = true;
			break;
		case '[':
			if (prefix)
				ostack[osp++] = TK_ARRAY;
			else
				ostack[osp++] = TK_INDEX;
			c = str[si++];
			prefix = true;
			break;
		case '{':
		case 219:
			ostack[osp++] = TK_STRUCT;
			c = str[si++];
			prefix = true;
			break;
		case ')':
		case ']':
		case '}':
		case 221:
			ni = close_op(tk, ni, prefix);
			c = str[si++];
			prefix = false;
			break;
		case ',':
			ni = parse_comma(tk, ni);
			c = str[si++];
			prefix = true;
			break;
		case ':':
			ni -= 2;
			symlist[osp-1] = ((tk[ni] & 0x0f) << 16) | tk[ni + 1];
			c = str[si++];
			prefix = true;
			break;
		case '+':
			if (prefix)
				return nullptr;

			c = str[si++];
			if (c == '=')
			{
				ni = parse_op(tk, ni, TK_ASSIGN_ADD);
				c = str[si++];
			}
			else
				ni = parse_op(tk, ni, TK_ADD);
			prefix = true;
			break;
		case '-':
			c = str[si++];
			if (c == '=')
			{
				ni = parse_op(tk, ni, TK_ASSIGN_SUB);
				c = str[si++];
			}
			else
			{
				if (prefix)
					ni = parse_op(tk, ni, TK_NEGATE);
				else
					ni = parse_op(tk, ni, TK_SUB);
			}

			prefix = true;
			break;
		case '*':
			ni = parse_op(tk, ni, TK_MUL);
			c = str[si++];
			prefix = true;
			break;
		case '/':
			ni = parse_op(tk, ni, TK_DIV);
			c = str[si++];
			prefix = true;
			break;
		case '&':
			ni = parse_op(tk, ni, TK_AND);
			c = str[si++];
			prefix = true;
			break;
		case '|':
		case 220:
			ni = parse_op(tk, ni, TK_OR);
			c = str[si++];
			prefix = true;
			break;
		case '%':
			ni = parse_op(tk, ni, TK_MOD);
			c = str[si++];
			prefix = true;
			break;
		case '.':
			c = str[si++];
			if (c == '.')
			{
				ni = parse_op(tk, ni, TK_DOTDOT);
				c = str[si++];
			}
			else
				ni = parse_op(tk, ni, TK_DOT);
			prefix = true;
			break;
		case '!':
			c = str[si++];
			if (c == '=')
			{
				ni = parse_op(tk, ni, TK_NOT_EQUAL);
				c = str[si++];
			}
			else
				ni = parse_op(tk, ni, TK_NOT);
			prefix = true;
			break;
		case '#':
			ni = parse_op(tk, ni, TK_LENGTH);
			c = str[si++];
			prefix = true;
			break;
		case '=':
			c = str[si++];
			if (c == '=')
			{
				ni = parse_op(tk, ni, TK_EQUAL);
				c = str[si++];
			}
			else
				ni = parse_op(tk, ni, TK_ASSIGN);
			prefix = true;
			break;
		case '<':
			c = str[si++];
			if (c == '=')
			{
				ni = parse_op(tk, ni, TK_LESS_EQUAL);
				c = str[si++];
			}
			else if (c == '<')
			{
				ni = parse_op(tk, ni, TK_SHL);
				c = str[si++];
			}
			else
				ni = parse_op(tk, ni, TK_LESS_THAN);
			prefix = true;
			break;
		case '>':
			c = str[si++];
			if (c == '=')
			{
				ni = parse_op(tk, ni, TK_GREATER_EQUAL);
				c = str[si++];
			}
			else if (c == '>')
			{
				ni = parse_op(tk, ni, TK_SHR);
				c = str[si++];
			}
			else
				ni = parse_op(tk, ni, TK_GREATER_THAN);
			prefix = true;
			break;
		case '"':
			{
				if (!prefix)
					return nullptr;

				c = str[si++];
				char i = 0;
				while (c != '"')
				{
					if (!c)
						return nullptr;

					if (c == '\\')
					{
						c = str[si++];
						if (c == p'n' || c == p'N')
							c = 13;
						else if (c == p't' || c == p'T')
							c = 8;
					}

					tk[ni + i + 2] = c;
					i++;
					c = str[si++];
				}
				c = str[si++];

				tk[ni + 0] = TK_STRING;
				tk[ni + 1] = i;
				ni += i + 2;
				prefix = false;
			} break;
		case '$':
			{
				if (!prefix)
					return nullptr;

				char i = 0;
				c = str[si++];
				while (is_hex(c) && is_hex(str[si]))
				{
					char d = str[si++];
					tk[ni + i + 2] = (hex_value(c) << 4) | hex_value(d);
					i++;
					c = str[si++];
				}

				tk[ni + 0] = TK_BYTES;
				tk[ni + 1] = i;
				ni += i + 2;
				prefix = false;
			} break;
		default:
			if (is_letter(c))
			{
				if (!prefix)
					return nullptr;

				char	idbuf[10];
				char 	j = 0;
				do 	{
					if (j < 8)
						idbuf[j++] = c;
					c = str[si++];
				} while (is_exletter(c));
				idbuf[j] = 0;

				unsigned	id = symbol_add(idbuf);
				tk[ni++] = (id >> 8) | TK_IDENT;
				tk[ni++] = id & 0xff;
				prefix = false;
			}
			else if (is_digit(c))
			{
				if (!prefix)
					return nullptr;

				long lr;
				si += number_parse(str + si - 1, 240, lr) - 1;
				c = str[si++];

				unsigned long num = lr;
				if (num & 0xffff)
				{
					tk[ni++] = TK_NUMBER;
					tk[ni++] = num >> 24;
					tk[ni++] = num >> 16;
					tk[ni++] = num >> 8;
					tk[ni++] = num & 0xff;
				}
				else if (num < 0x00100000ul)
				{
					tk[ni++] = TK_TINY_INT | (num >> 16);
				}
				else if (num < 0x10000000ul)
				{
					tk[ni++] = TK_SMALL_INT | (num >> 24);
					tk[ni++] = num >> 16;
				}
				else
				{
					tk[ni++] = TK_NUMBER;
					tk[ni++] = num >> 24;
					tk[ni++] = num >> 16;
					tk[ni++] = 0;
					tk[ni++] = 0;
				}
				prefix = false;
			}
			else
				return nullptr;
		}
	}
}

char * parse_params(const char * str, char * tk)
{
	char * etk = tk;
	tk = parse_expression(str, tk);
	if (tk)
	{
		char i = 0;
		while ((etk[2 * i] & 0xf0) == TK_IDENT)
			i++;
		if (etk[2 * i] == TK_INVOKE && etk[2 * i + 1] + 1 == i)
			return tk;
	}
	return nullptr;
}

char tolower(char ch)
{
	if (ch >= p'A' && ch <= p'Z')
		return ch + (p'a' - p'A');
	else
		return ch;
}

char * parse_statement(const char * str, char * tk)
{
	char l = 0;
	while (str[l] == ' ' || str[l] == 160)
		l++;

	char * etk = tk;
	*tk++ = l + 1;

	str += l;
	char i = 0;

	char	idbuf[10];
	while (i < 10 && is_letter(str[i]))
	{
		idbuf[i] = tolower(str[i]);
		i++;
	}
	idbuf[i] = 0;
	if (i == 0 && !str[i])
		*tk++ = STMT_NONE;
	else if (!strcmp(idbuf, p"while"))
	{
		*tk++ = STMT_WHILE;	
		*tk++ = 0;
		*tk++ = 0;
		tk = parse_expression(str + i, tk);
	}
	else if (!strcmp(idbuf, p"for"))
	{
		*tk++ = STMT_FOR;	
		tk = parse_expression(str + i, tk);
		if (tk)
		{
			*tk++ = l + 1;
			*tk++ = STMT_NEXT;
			*tk++ = 0;
			*tk++ = 0;
		}
	}
	else if (!strcmp(idbuf, p"if"))
	{
		*tk++ = STMT_IF;	
		*tk++ = 0;
		*tk++ = 0;
		tk = parse_expression(str + i, tk);
	}
	else if (!strcmp(idbuf, p"elsif"))
	{
		*tk++ = STMT_ELSIF;
		*tk++ = 0;
		*tk++ = 0;
		tk = parse_expression(str + i, tk);
	}
	else if (!strcmp(idbuf, p"else"))
	{
		*tk++ = STMT_ELSE;
		*tk++ = 0;
		*tk++ = 0;
	}
	else if (!strcmp(idbuf, p"break"))
	{
		*tk++ = STMT_BREAK;
	}
	else if (!strcmp(idbuf, p"exit"))
	{
		*tk++ = STMT_EXIT;
	}
	else if (!strcmp(idbuf, p"var"))
	{
		*tk++ = STMT_VAR;
		tk = parse_expression(str + i, tk);
	}
	else if (!strcmp(idbuf, p"return"))
	{
		if (str[i])
		{
			*tk++ = STMT_RETURN;
			tk = parse_expression(str + i, tk);
		}
		else
			*tk++ = STMT_RETURN_NULL;
	}
	else if (!strcmp(idbuf, p"def"))
	{
		etk[0] = 1;
		*tk++ = STMT_DEF;
		*tk++ = 0;
		*tk++ = 0;
		tk = parse_params(str + i, tk);
	}
	else if (str[i] == '#')
	{
		*tk++ = STMT_COMMENT;
		i++;
		while (str[i])
		{
			tk[i] = str[i];
			i++;
		}
		tk[0] = i - 1;
		return tk + i;
	}
	else
	{
		*tk++ = STMT_EXPRESSION;
		tk = parse_expression(str, tk);
	}

	if (tk)
		runtime_error = RERR_OK;
	else
	{
		etk[1] = STMT_ERROR;
		i = 0;
		while (str[i])
		{
			etk[3 + i] = str[i];
			i++;
		}
		etk[2] = i;
		tk = etk + 3 + i;
		runtime_error = RERR_SYNTAX;
	}

	return tk;
}

void parse_pretty(char * tk)
{
	char lmin = 1, lmax = 1;
	char dl = 0;

	while (tk[0])
	{
		char t = tk[1];

		if (t != STMT_NONE)
		{
			char l = tk[0];

			if (t == STMT_DEF)
				lmin = lmax = 1;
			else if (lmax > 1 && (t == STMT_ELSE || t == STMT_ELSIF))
				lmin = lmax = lmax - 1;
			
			if (l < lmin)
				l = lmin;
			else if (l > lmax)
				l = lmax;

			lmax = l;
			lmin = 1;

			tk[0] = l;
		}
		else
			tk[0] = lmin;

		switch (t)
		{
		case STMT_DEF:
		case STMT_IF:
		case STMT_ELSE:
		case STMT_ELSIF:
		case STMT_WHILE:
		case STMT_FOR:
			lmin = lmax = lmax + 1;
			break;
		}
		tk += token_skip_statement(tk);		
	}
}
