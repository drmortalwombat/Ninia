#include "tokens.h"


const char * number_format(unsigned long l, bool sign)
{
	static char buffer[16];

	if (sign && (long)l < 0)
		l = -(long)l;
	else
		sign = false;
	
	char		si = 6;	
	unsigned	hi = l >> 16;
	if (hi == 0)
		buffer[--si] = '0';
	else
	{
		char i = 0;
		while (hi)
		{
			buffer[--si] = hi % 10 + '0';
			hi /= 10;
			i++;
		}		
	}
	if (sign)
		buffer[--si] = '-';

	char ei = 6;

	l &= 0xffff;
	if (l)
	{
		buffer[ei++] = '.';
		for(char i=0; i<4; i++)
		{
			l *= 10;
			buffer[ei++] = char(l >> 16) + '0';
			l &= 0xfffful;
		}
		while (buffer[ei-1] == '0')
			ei--;
		if (buffer[ei-1] == '.')
			ei--;
	}
	buffer[ei] = 0;

	return buffer + si;
}

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


unsigned long number_parse(const char * str, char n)
{
	char 	i = 0;
	bool	sign = false;

	while(i < n && str[i] == ' ')
		i++;

	if (i < n && str[i] == '-')
	{
		sign = true;
		i++;
	}

	unsigned num = 0;
	while (i < n && is_digit(str[i]))
	{
		num = num * 10 + (str[i] - '0');
		i++;
	}

	unsigned long ul = (unsigned long)num << 16;
	if (i < n && str[i] == '.')
	{

		unsigned long fract = 0;
		unsigned long rem = 1;
		i++;

		while (i < n && is_digit(str[i]))
		{
			rem *= 10;
			fract = fract * 10 + (str[i] - '0');
			i++;
		} 
		ul += ((fract << 16) + (rem - 1)) / rem;
	}

	if (sign)
		return -(long)ul;
	else
		return ul;
}

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
			ti += 2;
			break;

		case TK_CONTROL:
			switch (t)
			{
			case TK_STRING:
				ti += tk[ti + 1] + 2;
				break;
			case TK_LIST:
			case TK_ARRAY:
				ti += 2;
				break;
			case TK_STRUCT:
				ti += 2 + 2 * tk[ti + 1];
				break;
			default:
				ti++;
			}
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

