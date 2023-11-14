#include <stdio.h>

#include <c64/vic.h>
#include "parser.h"
#include "dump.h"
#include "symbols.h"
#include "formatter.h"
#include "variables.h"
#include "interpreter.h"
#include "editor.h"
#include <conio.h>

char tokens[1024];

int main(void)
{
	clrscr();

	vic.color_border = VCOL_BLACK;
	vic.color_back = VCOL_BLACK;

	symbols_init();

	char * tk = tokens;

#if 0
	tk = parse_statement(p"var x = 0", tk);
	tk = parse_statement(p"var x + + 0", tk);
	tk = parse_statement(p"while (x < 10)", tk);
	tk = parse_statement(p" if (x % 4 == 1)", tk);
	tk = parse_statement(p"  chrout(65)", tk);
	tk = parse_statement(p" elsif (x % 4 == 2)", tk);
	tk = parse_statement(p"  chrout(70)", tk);
	tk = parse_statement(p" else", tk);
	tk = parse_statement(p"  chrout(66)", tk);
	tk = parse_statement(p" x = x + 1", tk);
	tk = parse_statement(p"chrout(10)", tk);

#elif 0
	tk = parse_statement("var x = 0", tk);
	tk = parse_statement("while (x < 1000)", tk);
	tk = parse_statement(" chrout(rand(2) + 205)", tk);
	tk = parse_statement(" x = x + 1", tk);
	tk = parse_statement("chrout(10)", tk);
#endif
	*tk++ = 0;

	screentk = cursortk = tokens;
	endtk = tk;
	cursorx = cursory = screenx = 0;

	putch(147); putch(14);
	edit_refresh_screen();

	for(;;)
	{
		char ch = edit_line();
		switch (ch)
		{
		case PETSCII_CURSOR_DOWN:
			if (*cursortk)
			{
				cursory++;
				cursortk = cursortk + (format_skip_statement(cursortk) - cursortk);
			}
			break;
		case PETSCII_CURSOR_UP:
			if (cursory > 0)
			{
				cursory--;
				cursortk = screentk;
				char i = 0;
				while (i < cursory)
				{
					cursortk = cursortk + (format_skip_statement(cursortk) - cursortk);
					i++;
				}
			}
			break;
		case '\n':
			if (*cursortk)
			{
				cursory++;
				cursortk = cursortk + (format_skip_statement(cursortk) - cursortk);
				cursorx = *cursortk - 1;
			}
			break;
		case PETSCII_F5:
			{
				putch(147);
				prepare_statements(tokens);
				while (*exectk && *(volatile char *)0x91 != 0x7f)
					interpret_statement();
				restore_statements(tokens);
				getch();
				putch(147); putch(14);
				edit_refresh_screen();
			} break;
		}
	}

#if 0

	prepare_statements(tokens);

	char str[100];

	const char * ctk = tokens;

	while (*ctk)
	{
		ctk = format_statement(ctk, str);
		printf(":%s:\n", str);
	}

	ctk = tokens;
	while (*ctk)
		ctk = dump_statement(ctk);

	ctk = tokens;
	while (*ctk)
		ctk = interpret_statement(ctk);
#endif
	return 0;

}