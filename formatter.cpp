#include "formatter.h"
#include "tokens.h"
#include <c64/vic.h>

void format_insert(char * str, char * color, char si, char ei, char c)
{
	do {
		ei--;
		str[ei + 1] = str[ei];
		color[ei + 1] = color[ei];
	} while (si != ei);
	str[si] = c;
	color[si] = VCOL_MED_GREY;
}

void format_insert2(char * str, char * color, char si, char ei, char c0, char c1)
{
	do {
		ei--;
		str[ei + 2] = str[ei];
		color[ei + 2] = color[ei];
	} while (si != ei);
	str[si + 0] = c0;
	color[si + 0] = VCOL_MED_GREY;
	str[si + 1] = c1;
	color[si + 1] = VCOL_MED_GREY;
}

const char * format_expression(const char * tk, char * str, char * color, char si)
{
	char 		t = *tk++;
	char		stack[32];
	char		sp = 0;

	while (t != TK_END)
	{

		switch (t & 0xf0)
		{
		case TK_TINY_INT:
			stack[sp++] = si;
			utoa(t & 0x0f, str + si, 10);
			while (str[si])
			{
				color[si] = VCOL_LT_GREY;
				si++;
			}
			break;
		case TK_SMALL_INT:
			stack[sp++] = si;
			utoa(((t & 0x0f) << 8) | *tk++, str + si, 10);
			while (str[si])
			{
				color[si] = VCOL_LT_GREY;
				si++;
			}
			break;
		case TK_NUMBER:
			{
				stack[sp++] = si;
				unsigned f = (*tk++ << 8);
				f |= *tk++;
				utoa(f, str + si, 10);
				while (str[si])
				{
					color[si] = VCOL_LT_GREY;
					si++;
				}
				unsigned long l = *tk++;
				l <<= 8;
				l |= *tk++; 
				if (l)
				{
					str[si] = '.';
					color[si] = VCOL_LT_GREY;
					si++;
					for(char i=0; i<4; i++)
					{
						l *= 10;
						str[si] = char(l >> 16) + '0';
						color[si] = VCOL_LT_GREY;
						si++;
						l &= 0xfffful;
					}
					while (str[si-1] == '0')
						si--;
					if (str[si-1] == '.')
						si--;
				}
			}	break;

		case TK_IDENT:
		case TK_CONST:	
		case TK_GLOBAL:	
		case TK_LOCAL:
			{
				stack[sp++] = si;
				char tt = t & 0xf0;
				unsigned ti = ((t & 0x0f) << 8) | *tk++;
				const char * ss;			
				if (tt == TK_IDENT)
					ss = symbol_string(ti);
				else if (tt == TK_GLOBAL)
					ss = symbol_string(globals[ti].symbol);
				else 
					ss = "";

				char i = 0;
				while (i < 8 && ss[i])
				{
					str[si] = ss[i++];
					color[si++] = VCOL_YELLOW;
				}
			}	break;

		case TK_BINARY:
			switch (t)
			{
			case TK_ADD:
				format_insert(str, color, stack[--sp], si, '+');
				si++;
				break;
			case TK_SUB:
				format_insert(str, color, stack[--sp], si, '-');
				si++;
				break;
			case TK_MUL:
				format_insert(str, color, stack[--sp], si, '*');
				si++;
				break;
			case TK_DIV:
				format_insert(str, color, stack[--sp], si, '/');
				si++;
				break;
			case TK_MOD:
				format_insert(str, color, stack[--sp], si, '%');
				si++;
				break;
			case TK_SHL:
				format_insert2(str, color, stack[--sp], si, '<', '<');
				si += 2;
				break;
			case TK_SHR:
				format_insert2(str, color, stack[--sp], si, '>', '>');
				si += 2;
				break;
			case TK_AND:
				format_insert(str, color, stack[--sp], si, '&');
				si++;
				break;
			case TK_OR:
				format_insert(str, color, stack[--sp], si, '|');
				si++;
				break;
			}
			break;
		case TK_RELATIONAL:
			switch (t)
			{
			case TK_EQUAL:
				format_insert2(str, color, stack[--sp], si, '=', '=');
				si += 2;
				break;
			case TK_NOT_EQUAL:
				format_insert2(str, color, stack[--sp], si, '!', '=');
				si += 2;
				break;
			case TK_LESS_THAN:
				format_insert(str, color, stack[--sp], si, '<');
				si++;
				break;
			case TK_LESS_EQUAL:
				format_insert2(str, color, stack[--sp], si, '<', '=');
				si += 2;
				break;
			case TK_GREATER_THAN:
				format_insert(str, color, stack[--sp], si, '>');
				si++;
				break;
			case TK_GREATER_EQUAL:
				format_insert2(str, color, stack[--sp], si, '>', '=');
				si += 2;
				break;
			}
			break;
		case TK_PREFIX:
			switch (t)
			{
			case TK_NEGATE:
				format_insert(str, color, stack[sp-1], si, '-');
				si++;
				break;
			case TK_NOT:
				format_insert(str, color, stack[sp-1], si, '!');
				si++;
				break;
			}
			break;

		case TK_POSTFIX:
			{
				char n = *tk++;
				while (n > 1)
				{
					format_insert(str, color, stack[--sp], si, ',');
					si++;
					n--;
				}

				switch (t)
				{
				case TK_INDEX:
					format_insert(str, color, stack[--sp], si, '[');
					str[si + 1] = ']';
					color[si + 1] = VCOL_MED_GREY;
					si += 2;
					break;
				case TK_DOT:
					format_insert(str, color, stack[--sp], si, '.');
					si++;
					break;
				case TK_INVOKE:
					if (n)
						format_insert(str, color, stack[--sp], si, '(');
					else
					{
						str[si] = '(';
						color[si] = VCOL_MED_GREY;
					}
					str[si + 1] = ')';
					color[si + 1] = VCOL_MED_GREY;
					si += 2;
					break;
				}
			}
			break;

		case TK_ASSIGN:
			format_insert(str, color, stack[--sp], si, '=');
			si++;
			break;
		case TK_STRUCTURE:
			{
				char n = *tk++;
				while (n > 1)
				{
					format_insert(str, color, stack[--sp], si, ',');
					si++;
					n--;
				}
				switch (t)
				{
				case TK_LIST:
					if (n)
						format_insert(str, color, stack[sp-1], si, '(');
					else
					{
						str[si] = '(';
						color[si] = VCOL_MED_GREY;
					}
					str[si + 1] = ')';
					color[si + 1] = VCOL_MED_GREY;
					si += 2;
					break;
				case TK_ARRAY:
					break;
				case TK_STRUCT:
					break;
				}
			}
			break;

		case TK_CONTROL:
			switch (t)
			{
			case TK_END:
				break;
			case TK_COMMA:
				str[si] = ',';
				color[si] = VCOL_MED_GREY;
				si++;
				break;
			case TK_STRING:
				{
					stack[sp++] = si;
					str[si] = '"';
					color[si] = VCOL_MED_GREY;
					si++;
					char i = 0;
					while (i < tk[0])
					{
						str[si] = tk[i + 1];
						color[si] = VCOL_LT_BLUE;
						si++;
						i++;
					}
					str[si] = '"';
					color[si] = VCOL_MED_GREY;
					si++;
					tk += i + 1;
				} break;
			}
			break;
		}
		t = *tk++;
	}

	str[si] = 0;
	return tk;
}

char format_append(char * str, char * col, char si, const char * src)
{
	char j = 0;
	while (src[j])
	{
		str[si] = src[j];
		col[si] = VCOL_WHITE;
		si++;
		j++;
	}
	return si;
}

const char * format_statement(const char * tk, char * str, char * col)
{
	if (*tk)
	{
		char l = *tk++ - 1;
		for(char i=0; i<l; i++)
		{
			str[i] = ' ';
			col[i] = VCOL_LT_BLUE;
		}

		char t = *tk++;
		switch (t)
		{
		case STMT_EXPRESSION:
			return format_expression(tk, str, col, l);
		case STMT_WHILE:
			l = format_append(str, col, l, p"WHILE ");
			return format_expression(tk + 2, str, col, l);
		case STMT_IF:
			l = format_append(str, col, l, p"IF ");
			return format_expression(tk + 2, str, col, l);
		case STMT_ELSIF:
			l = format_append(str, col, l, p"ELSIF ");
			return format_expression(tk + 2, str, col, l);
		case STMT_ELSE:
			l = format_append(str, col, l, p"ELSE");
			str[l] = 0;
			return tk + 2;
		case STMT_VAR:
			l = format_append(str, col, l, p"VAR ");
			return format_expression(tk, str, col, l);
		case STMT_RETURN:
			l = format_append(str, col, l, p"RETURN ");
			return format_expression(tk, str, col, l);
		case STMT_DEF:
			l = format_append(str, col, l, p"DEF ");
			return format_expression(tk + 2, str, col, l);
		case STMT_NONE:
			str[l] = 0;
			return tk;
		case STMT_ERROR:
			{
				char n = *tk++;
				while (n)
				{
					str[l] = *tk++;
					col[l] = VCOL_ORANGE;
					l++;
					n--;
				}
				str[l] = 0;
			} break;
		}
	}
	else
		str[0] = 0;

	return tk;
}

const char * format_skip_expression(const char * tk)
{
	char 	t = *tk;
	while (t != TK_END)
	{
		switch (t & 0xf0)
		{
		case TK_SMALL_INT:
			tk += 2;
			break;
		case TK_NUMBER:
			tk += 5;
			break;

		case TK_IDENT:
		case TK_CONST:	
		case TK_GLOBAL:	
		case TK_LOCAL:
			tk += 2;
			break;

		case TK_POSTFIX:
		case TK_STRUCTURE:
			tk += 2;
			break;

		case TK_CONTROL:
			if (t == TK_STRING)
				tk += tk[1] + 2;
			else
				tk++;
			break;

		default:
			tk++;
		}
		t = *tk;
	}
	return tk + 1;
}

const char * format_skip_statement(const char * tk)
{
	if (*tk)
	{
		tk++;
		char t = *tk++;
		switch (t)
		{
		case STMT_DEF:
		case STMT_WHILE:
		case STMT_IF:
		case STMT_ELSIF:
			return format_skip_expression(tk + 2);
		case STMT_ELSE:
			return tk + 2;
		case STMT_EXPRESSION:
		case STMT_VAR:
		case STMT_RETURN:
			return format_skip_expression(tk);
		case STMT_NONE:
			return tk;
		case STMT_ERROR:
			{
				char n = *tk++;
				return tk + n;
			} break;
		}
	}

	return tk;	
}

