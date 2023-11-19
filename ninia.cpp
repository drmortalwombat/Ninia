#include <stdio.h>
#include <string.h>

#include <c64/vic.h>
#include "parser.h"
#include "symbols.h"
#include "formatter.h"
#include "runtime.h"
#include "interpreter.h"
#include "editor.h"
#include "system.h"
#include "errors.h"
#include <conio.h>
#include <c64/kernalio.h>

int main(void)
{
	system_init();

	symbols_init();

	edit_init();
	char * tk = starttk;

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
#elif 0
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
	tk = parse_statement(p"var a={x:1,y:2}", tk);
	tk = parse_statement(p"", tk);
	tk = parse_statement(p"print(a.x, \" \", a.y)", tk);
#elif 1
	tk = parse_statement(p"def pm(m)", tk);
	tk = parse_statement(p" var i=0", tk);
	tk = parse_statement(p" while i<len(m)", tk);
	tk = parse_statement(p"  var j=0,v=m[i]", tk);
	tk = parse_statement(p"  while j<len(v)", tk);
	tk = parse_statement(p"   if j>0", tk);
	tk = parse_statement(p"    print(\",\")", tk);
	tk = parse_statement(p"   print(v[j])", tk);
	tk = parse_statement(p"   j=j+1", tk);
	tk = parse_statement(p"  print(\"\\n\")", tk);
	tk = parse_statement(p"  i=i+1", tk);
	tk = parse_statement(p"", tk);
	tk = parse_statement(p"pm([[1,2],[3,4],[5,6,7]])", tk);


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
	*tk++ = STMT_END;
	endtk = tk;

	system_show_editor();
	edit_refresh_screen();

	// "LOAD SAVE FIND ---- RUN- ---- ---- ---- ----";

	for(;;)
	{
		bool	redraw = false;

		edit_show_status();
		char ch = edit_line();
		switch (ch)
		{
		case PETSCII_CURSOR_DOWN:
			if (*cursortk)
				cursory++;
			break;
		case PETSCII_CURSOR_UP:
			if (cursortk != starttk)
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
				system_show_runtime();
				clrscr();
				parse_pretty(starttk);
				prepare_statements(starttk);
				while (interpret_statement() && !runtime_error && *(volatile char *)0x91 != 0x7f)
					;
				restore_statements(starttk);
				if (!runtime_error)
					getch();
				system_show_editor();
				redraw = true;
			} break;
		case PETSCII_F6:
			parse_pretty(starttk);
			redraw = true;
			break;
		case PETSCII_F1:
			{
				krnio_setnam("@0:TEST.NIN,P,W");
				if (krnio_open(2, 9, 2))
				{				
					krnio_chkout(2);
					const char * tk = starttk;
					while (*tk)
					{
						tk = format_statement(tk, buffer, cbuffer);
						char i =0 ;
						while (char c = buffer[i])
						{
							if (c >= p'A' && c <= p'Z')
								c -= p'A' - 'A';
							else if (c >= p'a' && c <= p'z')
								c += 'a' - p'a';
							krnio_chrout(c);
							i++;
						}
						krnio_chrout(10);
					}
					krnio_clrchn();
					krnio_close(2);
				}
			} break;
		case PETSCII_F2:
			{
				krnio_setnam("@0:TEST.NIN,P,R");
				if (krnio_open(2, 9, 2))
				{				
					edit_init();
					krnio_chkin(2);

					char * tk = starttk;
					char status = krnio_status();
					while (!status)
					{
						vic.color_back++;
						char i = 0;
						while ((char c = krnio_chrin()) != 10 && !(status = krnio_status()))
						{
							vic.color_border++;
							if (c >= 'A' && c <= 'Z')
								c += p'A' - 'A';
							else if (c >= 'a' && c <= 'z')
								c -= 'a' - p'a';
							buffer[i++] = c;
						}
						buffer[i] = 0;
						tk = parse_statement(buffer, tk);
					}
					krnio_clrchn();
					krnio_close(2);

					*tk++ = STMT_END;
					endtk = tk;
					redraw = true;
				}
			} break;
		}

		if (cursory < 3 && screeny > 0)
		{
			screeny--;
			cursory++;
			screentk = starttk;
			for(unsigned i=0; i<screeny; i++)
				screentk += token_skip_statement(screentk);
			redraw = true;
		}
		else if (cursory > 20)
		{
			screeny++;
			cursory--;
			screentk = starttk;
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

	return 0;
}