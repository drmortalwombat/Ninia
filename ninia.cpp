#include <stdio.h>
#include <string.h>

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
#elif 1
	tk = parse_statement(p"var a=[]", tk);
	tk = parse_statement(p"while 1", tk);
	tk = parse_statement(p" push(a, 1)", tk);
	tk = parse_statement(p" print(len(a))", tk);
#elif 1
	tk = parse_statement(p"def par(a)", tk);
	tk = parse_statement(p" var i=0", tk);
	tk = parse_statement(p" while i<len(a)", tk);
	tk = parse_statement(p"  print(a[i], \", \")", tk);
	tk = parse_statement(p"  i=i+1", tk);
	tk = parse_statement(p" print(\"\\n\")", tk);
	tk = parse_statement(p"", tk);
	tk = parse_statement(p"def fill(n)", tk);
	tk = parse_statement(p" var i=0,a=[]", tk);
	tk = parse_statement(p" while i<n", tk);
	tk = parse_statement(p"  push(a,i*i)", tk);
	tk = parse_statement(p"  i=i+1", tk);
	tk = parse_statement(p" return a", tk);
	tk = parse_statement(p"", tk);
	tk = parse_statement(p"while 1", tk);
	tk = parse_statement(p" par(fill(9))", tk);	
#elif 1
	tk = parse_statement(p"var i=0, a=array(0)", tk);
	tk = parse_statement(p"while i<10", tk);
	tk = parse_statement(p" push(a,i*i)", tk);
	tk = parse_statement(p" i=i+1", tk);
	tk = parse_statement(p"print(a[4], \" \", a[9])", tk);
#elif 1
	tk = parse_statement(p"var i=0, a=array(10)", tk);
	tk = parse_statement(p"while i<10", tk);
	tk = parse_statement(p" a[i]=i*i", tk);
	tk = parse_statement(p" i=i+1", tk);	
	tk = parse_statement(p"print(a[4], \" \", a[9])", tk);
#elif 1
	tk = parse_statement(p"var i=0", tk);
	tk = parse_statement(p"var a=[1,3,5,7]", tk);	
	tk = parse_statement(p"print(a[2])", tk);
#elif 1
	tk = parse_statement(p"var i=0", tk);
	tk = parse_statement(p"while i<1000", tk);
	tk = parse_statement(p" var s=\"+\",j=0", tk);
	tk = parse_statement(p" while j<26", tk);
	tk = parse_statement(p"  s=cat(s,chr(j+65))", tk);
	tk = parse_statement(p"  j=j+1", tk);
	tk = parse_statement(p" print(\"<\",s,\">\\n\")", tk);
	tk = parse_statement(p" i=i+1", tk);
	tk = parse_statement(p"", tk);
#elif 1
	tk = parse_statement(p"var i=0", tk);
	tk = parse_statement(p"var hello=\"Hello\"", tk);
	tk = parse_statement(p"var world=\"World\"", tk);
	tk = parse_statement(p"while i<1000", tk);
	tk = parse_statement(p" print(cat(hello, \" \", world),\"\\n\")", tk);
	tk = parse_statement(p" i=i+1", tk);
	tk = parse_statement(p"", tk);
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
		bool	redraw = false;

		char ch = edit_line();
		switch (ch)
		{
		case PETSCII_CURSOR_DOWN:
			if (*cursortk)
				cursory++;
			break;
		case PETSCII_CURSOR_UP:
			if (cursortk != tokens)
				cursory--;
			break;
		case PETSCII_DEL:
			if (cursory > 0)
			{
				cursory--;
				char * prevtk = screentk;
				char i = 0;
				while (i < cursory)
				{
					prevtk += token_skip_statement(prevtk);
					i++;
				}

				if (token_skip_statement(prevtk) == 2)
				{
					// delete previous line
					memmove(prevtk, cursortk, endtk - cursortk);
					endtk -= 2;
					cursortk = prevtk;
					edit_refresh_screen();									
				}
				else if (token_skip_statement(cursortk) == 2)
				{
					// delete current line
					memmove(cursortk, cursortk + 2, endtk - cursortk - 2);
					endtk -= 2;
					edit_refresh_screen();
					cursortk = prevtk;
					cursorx = 255;
				}
				else
					cursory++;
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
		case 25:
			if (*cursortk)
			{
				char len = token_skip_statement(cursortk);
				char * nexttk = cursortk + len;
				memmove(cursortk, nexttk, endtk - nexttk);
				endtk -= len;
				redraw = true;
			} 
			break;
		case PETSCII_F5:
			{
				putch(147);
				parse_pretty(tokens);
				prepare_statements(tokens);
				while (interpret_statement() && !runtime_error && *(volatile char *)0x91 != 0x7f)
					;
				restore_statements(tokens);
				if (!runtime_error)
					getch();
				putch(147); putch(14);
				vic.color_border = VCOL_BLACK;
				vic.color_back = VCOL_BLACK;
				redraw = true;
			} break;
		case PETSCII_F6:
			parse_pretty(tokens);
			redraw = true;
			break;
		}

		if (cursory < 3 && screeny > 0)
		{
			screeny--;
			cursory++;
			screentk = tokens;
			for(unsigned i=0; i<screeny; i++)
				screentk += token_skip_statement(screentk);
			redraw = true;
		}
		else if (cursory > 20)
		{
			screeny++;
			cursory--;
			screentk = tokens;
			for(unsigned i=0; i<screeny; i++)
				screentk += token_skip_statement(screentk);
			redraw = true;			
		}

		if (redraw)
			edit_refresh_screen();

		cursortk = screentk;
		for(char i=0; i<cursory; i++)
			cursortk += token_skip_statement(cursortk);
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