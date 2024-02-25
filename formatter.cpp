#include "formatter.h"
#include "tokens.h"
#include "system.h"
#include "compiler.h"
#include <c64/vic.h>

#pragma code( tcode )
#pragma data( tdata )

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

char format_insert_sym(char * str, char * color, char si, char ei, char c, unsigned id)
{
	const char * s = symbol_string(id);
	char n = 0;
	while (n < 8 && s[n])
		n++;

	do {
		ei--;
		str[(char)(ei + n + 2)] = str[ei];
		color[(char)(ei + n + 2)] = color[ei];
	} while (si != ei);
	str[si] = c;
	color[si] = VCOL_MED_GREY;

	for(char i=0; i<n; i++)
	{
		str[(char)(si + i + 1)] = s[i];
		color[(char)(si + i + 1)] = VCOL_YELLOW;
	}

	str[(char)(si + n + 1)] = ':';
	color[(char)(si + n + 1)] = VCOL_MED_GREY;

	return n + 2;
}

char format_append(char * str, char * color, char si, char c, const char * src)
{
	char i = 0;
	while (src[i])
	{
		str[si] = src[i];
		color[si] = c;
		si++;
		i++;
	}
	return si;
}


const char binary_names[][2] = {
	"+", "-", "*", "/", "%", "<<", ">>", "&", "|"
};

const char relational_names[][2] = {
	"==", "!=", "<", "<=", ">", ">="
};

const char assign_names[][2] = {
	"=", "+=", "-="
};

const char hex_chars[16] = p"0123456789abcdef";

const char * format_expression(const char * tk, char * str, char * color, char si)
{
	char		ti = 0;
	char 		t = tk[ti++];
	char		stack[32];
	char		sp = 0;

	for(;;)
	{
		switch (t & 0xf0)
		{
		case TK_TINY_INT:
			stack[sp++] = si;
			si = format_append(str, color, si, VCOL_LT_GREY, number_format((unsigned long)(t & 0x0f) << 16, false));
			break;
		case TK_SMALL_INT:
			stack[sp++] = si;
			si = format_append(str, color, si, VCOL_LT_GREY, number_format((unsigned long)(((t & 0x0f) << 8) | tk[ti++]) << 16, false));
			break;
		case TK_NUMBER:
			{
				stack[sp++] = si;

				unsigned long l = tk[ti++];
				l <<= 8;
				l |= tk[ti++];
				l <<= 8;
				l |= tk[ti++];
				l <<= 8;
				l |= tk[ti++];

				si = format_append(str, color, si, VCOL_LT_GREY, number_format(l, false));
			}	break;

		case TK_IDENT:
		case TK_CONST:	
		case TK_GLOBAL:	
		case TK_LOCAL:
			{
				stack[sp++] = si;
				char tt = t & 0xf0;
				unsigned id = ((t & 0x0f) << 8) | tk[ti++];
				const char * ss;			
				if (tt == TK_IDENT)
					ss = symbol_string(id);
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
			if (binary_names[t & 0x0f][1])
			{
				format_insert2(str, color, stack[--sp], si, binary_names[t & 0x0f][0], binary_names[t & 0x0f][1]);
				si += 2;				
			}
			else
			{
				format_insert(str, color, stack[--sp], si, binary_names[t & 0x0f][0]);
				si++;				
			}
			break;
		case TK_RELATIONAL:
			if (relational_names[t & 0x0f][1])
			{
				format_insert2(str, color, stack[--sp], si, relational_names[t & 0x0f][0], relational_names[t & 0x0f][1]);
				si += 2;				
			}
			else
			{
				format_insert(str, color, stack[--sp], si, relational_names[t & 0x0f][0]);
				si++;				
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
			case TK_LENGTH:
				format_insert(str, color, stack[sp-1], si, '#');
				si++;
				break;
			}
			break;

		case TK_POSTFIX:
			{
				char n = tk[ti++];
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
			if (assign_names[t & 0x0f][1])
			{
				format_insert2(str, color, stack[--sp], si, assign_names[t & 0x0f][0], assign_names[t & 0x0f][1]);
				si += 2;				
			}
			else
			{
				format_insert(str, color, stack[--sp], si, assign_names[t & 0x0f][0]);
				si++;				
			}
			break;

		case TK_CONTROL:
			switch (t)
			{
			case TK_COMMA:
				str[si] = ',';
				color[si] = VCOL_MED_GREY;
				si++;
				break;
			case TK_DOT:
				format_insert(str, color, stack[--sp], si, '.');
				si++;
				break;
			case TK_COLON:
				format_insert(str, color, stack[--sp], si, ':');
				si++;
				break;
			case TK_DOTDOT:
				format_insert2(str, color, stack[--sp], si, '.', '.');
				si += 2;
				break;
			case TK_STRING:
				{
					stack[sp++] = si;
					str[si] = '"';
					color[si] = VCOL_MED_GREY;
					si++;
					char n = tk[ti++];
					for(char i=0; i<n; i++)
					{
						char c = tk[ti++];

						if (c == 13 || c == 8 || c == '\\')
						{
							str[si] = '\\';
							color[si] = VCOL_LT_BLUE;
							si++;
							if (c == 13)
								c = p'n';
							else if (c == 8)
								c = p't';
						}
						str[si] = c;
						color[si] = VCOL_LT_BLUE;
						si++;
					}
					str[si] = '"';
					color[si] = VCOL_MED_GREY;
					si++;
				} break;
			case TK_BYTES:
				{
					stack[sp++] = si;
					str[si] = '$';
					color[si] = VCOL_MED_GREY;
					si++;
					char n = tk[ti++];
					for(char i=0; i<n; i++)
					{
						char c = tk[ti++];
						str[si] = hex_chars[c >> 4];
						str[si+1] = hex_chars[c & 0x0f];
						color[si] = color[si+1] = VCOL_LT_BLUE ^ 8 * (i & 1);

						si += 2;
					}
				} break;
			case TK_LIST:
			case TK_ARRAY:
				{
					char n = tk[ti++];
					while (n > 1)
					{
						format_insert(str, color, stack[--sp], si, ',');
						si++;
						n--;
					}

					if (n > 0)
						format_insert(str, color, stack[sp-1], si, t == TK_LIST ? '(' : '[');
					else
					{
						stack[sp++] = si;
						str[si] = t == TK_LIST ? '(' : '[';
						color[si] = VCOL_MED_GREY;
					}
					str[si + 1] = t == TK_LIST ? ')' : ']';
					color[si + 1] = VCOL_MED_GREY;
					si += 2;
				}	break;

			case TK_STRUCT:
			{
				char n = tk[ti++];
				if (n)
				{
					char i = n;
					while (i > 0)
					{
						i--;
						si += format_insert_sym(str, color, stack[--sp], si, i == 0 ? '{' : ',', (tk[ti + 2 * i + 1] << 8) | tk[ti + 2 * i]);
					}
					sp++;
				}
				else
				{
					stack[sp++] = si;
					str[si] = '{';
					color[si] = VCOL_MED_GREY;
					si ++;
				}
				str[si] = '}';
				color[si] = VCOL_MED_GREY;
				si ++;
				ti += 2 * n;
			} break;

			case TK_END:
				str[si] = 0;
				return tk + ti;
			} 
			break;
		}
		t = tk[ti++];
	}

}

const char * format_statement(const char * tk, char * str, char * col)
{
	if (*tk)
	{
		char l = *tk++ - 1;
		for(char i=0; i<l; i++)
		{
			str[i] = 160;
			col[i] = VCOL_DARK_GREY;
		}

		const char *	ftk = nullptr;

		char t = *tk++;
		if (t == STMT_FOLD)
		{
			if (l == 0)
				l++;
			str[0] = 171 ;
			col[0] = VCOL_LT_GREY;

			ftk = tk + (tk[0] + 256 * tk[1]);

			// skip header byte of next command
			tk += 3;
			t = *tk++;			
		}

		switch (t)
		{
		case STMT_COMMENT:
			{
				str[l] = '#';
				col[l] = VCOL_PURPLE;
				l++;

				char n = *tk++;
				while (n)
				{
					str[l] = *tk++;
					col[l] = VCOL_PURPLE;
					l++;
					n--;
				}
				str[l] = 0;
			} break;			
		case STMT_EXPRESSION:
			tk = format_expression(tk, str, col, l);
			break;
		case STMT_WHILE:
			l = format_append(str, col, l, VCOL_WHITE, p"WHILE ");
			tk = format_expression(tk + 2, str, col, l);
			break;
		case STMT_FOR:
			l = format_append(str, col, l, VCOL_WHITE, p"FOR ");
			tk = format_expression(tk, str, col, l) + 4;
			break;
		case STMT_IF:
			l = format_append(str, col, l, VCOL_WHITE, p"IF ");
			tk = format_expression(tk + 2, str, col, l);
			break;
		case STMT_ELSIF:
			l = format_append(str, col, l, VCOL_WHITE, p"ELSIF ");
			tk = format_expression(tk + 2, str, col, l);
			break;
		case STMT_ELSE:
			l = format_append(str, col, l, VCOL_WHITE, p"ELSE");
			str[l] = 0;
			tk += 2;
			break;
		case STMT_VAR:
			l = format_append(str, col, l, VCOL_WHITE, p"VAR ");
			tk = format_expression(tk, str, col, l);
			break;
		case STMT_RETURN:
			l = format_append(str, col, l, VCOL_WHITE, p"RETURN ");
			tk = format_expression(tk, str, col, l);
			break;
		case STMT_BREAK:
			l = format_append(str, col, l, VCOL_WHITE, p"BREAK");
			str[l] = 0;
			break;
		case STMT_EXIT:
			l = format_append(str, col, l, VCOL_WHITE, p"EXIT");
			str[l] = 0;
			break;
		case STMT_RETURN_NULL:
			l = format_append(str, col, l, VCOL_WHITE, p"RETURN");
			str[l] = 0;
			break;
		case STMT_DEF:
			l = format_append(str, col, l, VCOL_WHITE, p"DEF ");
			tk = format_expression(tk + 2, str, col, l);
			break;
		case STMT_NONE:
			str[l] = 0;
			break;
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

		// now skip folded section
		if (ftk)
			tk = ftk;
	}
	else
	{
		str[0] = 180;
		col[0] = VCOL_DARK_GREY;
		str[1] = 0;
	}

	return tk;
}

