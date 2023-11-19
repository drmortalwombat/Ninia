#include "parser.h"
#include "errors.h"

bool is_letter(char c)
{
	return c >= p'a' && c <= p'z' || c >= p'A' && c <= p'Z';
}

bool is_exletter(char c)
{
	return c >= p'a' && c <= p'z' || c >= p'A' && c <= p'Z' || c >= '0' && c <= '9' || c == '_';
}

bool is_digit(char c)
{
	return c >= '0' && c <= '9';
}

char	ostack[32];
char	osp;

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
		case TK_OR:
		case TK_AND:
			return 25;
		}
		return 0;

	case TK_RELATIONAL:
		return 20;
	case TK_ASSIGN:
		return 30;
	case TK_PREFIX:
		return 5;
	case TK_POSTFIX:
		return 0x80;

	case TK_STRUCTURE:
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
		case TK_COMMA:
			return 0x7f;
		default:
			return 0xff;
		}
		break;
	default:
		return 0xff;
	}
}

char * parse_op(char * tk, char op)
{
	char pri = oppri(op);
	while (osp && oppri(ostack[osp-1]) <= pri)
	{
		*tk++ = ostack[osp-1];
		osp--;
	}
	
	ostack[osp++] = op;
	return tk;
}

char * parse_comma(char * tk)
{
	while (osp && oppri(ostack[osp-1]) < 0x40)
	{
		*tk++ = ostack[osp-1];
		osp--;
	}
	if (osp)
		ostack[osp++] = TK_COMMA;
	else
		*tk++ = TK_COMMA;
	return tk;
}

char * close_op(char * tk, bool prefix)
{
	int cnt = 0;
	if (!prefix)
		cnt++;

	while (osp && oppri(ostack[osp-1]) != 0x80)
	{
		if (ostack[osp-1] == TK_COMMA)
			cnt++;
		else
			*tk++ = ostack[osp-1];
		osp--;
	}

	*tk++ = ostack[osp-1];
	*tk++ = cnt;
	osp--;

	return tk;
}

char * parse_expression(const char * str, char * tk)
{
	osp = 0;

	char 	c = *str++;
	bool	prefix = true;
	for(;;)
	{
		switch (c)
		{
		case '\0':
			tk = parse_op(tk, TK_END);
			if (osp != 1)
				return nullptr;
			*tk++ = TK_END;
			return tk;

		case ' ':
			c = *str++;
			break;			
		case '(':
			if (prefix)
				ostack[osp++] = TK_LIST;
			else
				ostack[osp++] = TK_INVOKE;
			c = *str++;
			prefix = true;
			break;
		case '[':
			if (prefix)
				ostack[osp++] = TK_ARRAY;
			else
				ostack[osp++] = TK_INDEX;
			c = *str++;
			prefix = true;
			break;
		case '{':
		case 219:
			ostack[osp++] = TK_STRUCT;
			c = *str++;
			prefix = true;
			break;
		case ')':
		case ']':
		case '}':
		case 221:
			tk = close_op(tk, prefix);
			c = *str++;
			prefix = false;
			break;
		case ',':
			tk = parse_comma(tk);
			c = *str++;
			prefix = true;
			break;
		case ':':
			tk = parse_op(tk, TK_COLON);
			c = *str++;
			prefix = true;
			break;
		case '+':
			if (prefix)
				return nullptr;

			tk = parse_op(tk, TK_ADD);
			c = *str++;
			prefix = true;
			break;
		case '-':
			if (prefix)
				tk = parse_op(tk, TK_NEGATE);
			else
				tk = parse_op(tk, TK_SUB);

			c = *str++;
			prefix = true;
			break;
		case '*':
			tk = parse_op(tk, TK_MUL);
			c = *str++;
			prefix = true;
			break;
		case '/':
			tk = parse_op(tk, TK_DIV);
			c = *str++;
			prefix = true;
			break;
		case '%':
			tk = parse_op(tk, TK_MOD);
			c = *str++;
			prefix = true;
			break;
		case '.':
			tk = parse_op(tk, TK_DOT);
			c = *str++;
			prefix = true;
			break;
		case '!':
			c = *str++;
			if (c == '=')
			{
				tk = parse_op(tk, TK_NOT_EQUAL);
				c = *str++;
			}
			else
				tk = parse_op(tk, TK_NOT);
			prefix = true;
			break;
		case '=':
			c = *str++;
			if (c == '=')
			{
				tk = parse_op(tk, TK_EQUAL);
				c = *str++;
			}
			else
				tk = parse_op(tk, TK_ASSIGN);
			prefix = true;
			break;
		case '<':
			c = *str++;
			if (c == '=')
			{
				tk = parse_op(tk, TK_LESS_EQUAL);
				c = *str++;
			}
			else if (c == '<')
			{
				tk = parse_op(tk, TK_SHL);
				c = *str++;
			}
			else
				tk = parse_op(tk, TK_LESS_THAN);
			prefix = true;
			break;
		case '>':
			c = *str++;
			if (c == '=')
			{
				tk = parse_op(tk, TK_GREATER_EQUAL);
				c = *str++;
			}
			else if (c == '>')
			{
				tk = parse_op(tk, TK_SHR);
				c = *str++;
			}
			else
				tk = parse_op(tk, TK_GREATER_THAN);
			prefix = true;
			break;
		case '"':
			{
				if (!prefix)
					return nullptr;

				c = *str++;
				char i = 0;
				while (c != '"')
				{
					if (!c)
						return nullptr;

					if (c == '\\')
					{
						c = *str++;
						if (c == p'n' || c == p'N')
							c = 13;
						else if (c == p't' || c == p'T')
							c = 8;
					}

					tk[i + 2] = c;
					i++;
					c = *str++;
				}
				c = *str++;

				tk[0] = TK_STRING;
				tk[1] = i;
				tk += i + 2;
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
					c = *str++;
				} while (is_exletter(c));
				idbuf[j] = 0;

				unsigned	id = symbol_add(idbuf);
				*tk++ = (id >> 8) | TK_IDENT;
				*tk++ = id & 0xff;
				prefix = false;
			}
			else if (is_digit(c))
			{
				if (!prefix)
					return nullptr;

				unsigned long num = 0;
				do {
					num = num * 10 + (c - '0');
					c = *str++;
				} while (is_digit(c));
				if (c == '.')
				{
					*tk++ = TK_NUMBER;
					*tk++ = num >> 8;
					*tk++ = num & 0xff;
					num = 0;
					unsigned long rem = 1;
					c = *str++;
					while (is_digit(c))
					{
						rem *= 10;
						num = num * 10 + (c - '0');
						c = *str++;
					} 
					num = ((num << 16) + (rem - 1)) / rem;
					*tk++ = num >> 8;
					*tk++ = num & 0xff;
				}
				else if (num < 16)
				{
					*tk++ = TK_TINY_INT | num;
				}
				else if (num < 4096)
				{
					*tk++ = TK_SMALL_INT | (num >> 8);
					*tk++ = num & 0xff;
				}
				else
				{
					*tk++ = TK_NUMBER;
					*tk++ = num >> 8;
					*tk++ = num & 0xff;
					*tk++ = 0;
					*tk++ = 0;
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
	while (str[l] == ' ')
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
			lmin = lmax = lmax + 1;
			break;
		}
		tk += token_skip_statement(tk);		
	}
}
