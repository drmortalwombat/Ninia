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
	tk = parse_statement(p"print(\"Hello World\")", tk);	
#elif 0
	tk = parse_statement(p"var x = 0", tk);
	tk = parse_statement(p"while (x < 1000)", tk);
	tk = parse_statement(p" chrout(rand(2) + 205)", tk);
	tk = parse_statement(p" x = x + 1", tk);
	tk = parse_statement(p"chrout(10)", tk);
#elif 0
	tk = parse_statement(p"def test(a, b)", tk);
	tk = parse_statement(p" print(\"Hello\")", tk);
	tk = parse_statement(p" return a + b", tk);
	tk = parse_statement(p"print(\"Before\")", tk);	
	tk = parse_statement(p"print(\"(\", test(4, 5), \")\")", tk);
	tk = parse_statement(p"print(\"After\")", tk);	
#elif 0
	tk = parse_statement(p"def brace(n)", tk);
	tk = parse_statement(p" print(\"<\", n, \">\")", tk);
	tk = parse_statement(p"", tk);
	tk = parse_statement(p"var i=0", tk);
	tk = parse_statement(p"while i<10", tk);
	tk = parse_statement(p" brace(i)", tk);
	tk = parse_statement(p" i=i+1", tk);
#elif 1
	tk = parse_statement(p"def fib(n)", tk);
	tk = parse_statement(p" if n < 2", tk);
	tk = parse_statement(p"  return 1", tk);
	tk = parse_statement(p" else", tk);
	tk = parse_statement(p"  return fib(n-1)+fib(n-2)", tk);
	tk = parse_statement(p"", tk);
	tk = parse_statement(p"var i=0", tk);
	tk = parse_statement(p"while i<10", tk);
	tk = parse_statement(p" print(i, \" \", fib(i), \"\\n\")", tk);
	tk = parse_statement(p" i=i+1", tk);
#elif 0
	tk = parse_statement(p"def abc(a, b, c)", tk);
	tk = parse_statement(p" var i=0", tk);
	tk = parse_statement(p" while i<10", tk);
	tk = parse_statement(p"  print(a, \",\", b, \",\", c)", tk);
	tk = parse_statement(p"  chrout(13)", tk);
	tk = parse_statement(p"  i=i+1", tk);
	tk = parse_statement(p" return 0", tk);
	tk = parse_statement(p"abc(1, 2, 3)", tk);
#endif
	*tk++ = 0;

	screentk = cursortk = tokens;
	endtk = tk;
	cursorx = cursory = screenx = 0;

	putch(147); putch(14);
	edit_refresh_screen();

	// "LOAD SAVE FIND ---- RUN- ---- ---- ---- ----";

	for(;;)
	{
		char ch = edit_line();
		switch (ch)
		{
		case PETSCII_CURSOR_DOWN:
			if (*cursortk)
			{
				cursory++;
				cursortk += token_skip_statement(cursortk);
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
					cursortk += token_skip_statement(cursortk);
					i++;
				}
			}
			break;
		case '\n':
			if (*cursortk)
			{
				cursory++;
				cursortk += token_skip_statement(cursortk);
				cursorx = *cursortk - 1;
			}
			break;
		case PETSCII_F5:
			{
				putch(147);
				parse_pretty(tokens);
				prepare_statements(tokens);
				while (interpret_statement() && *(volatile char *)0x91 != 0x7f)
					;
				restore_statements(tokens);
				getch();
				putch(147); putch(14);
				vic.color_border = VCOL_BLACK;
				vic.color_back = VCOL_BLACK;
				edit_refresh_screen();
			} break;
		case PETSCII_F6:
			parse_pretty(tokens);
			edit_refresh_screen();
			break;
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