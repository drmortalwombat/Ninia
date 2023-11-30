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
#include <c64/memmap.h>

edit_cmd_t	filename;

#pragma code( mcode )

void ninia_main(void)
{
	symbols_init();

	system_call(edit_init, BANK_EDITOR, BANK_MAIN);

#if 0
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
#endif

//	tokens_load("SPLIT.NIN");

	system_show_editor();

	// "LOAD SAVE FIND ---- RUN- ---- ---- ---- ----";

	for(;;)
	{
		char ch = system_call(edit_text, BANK_EDITOR, BANK_MAIN);
	
		switch (ch)
		{
		case PETSCII_F5:
			{
				system_show_runtime();
				system_fcall<BANK_EDITOR, BANK_MAIN, parse_pretty, char *>(starttk);
				system_fcall<BANK_RUNTIME, BANK_MAIN, prepare_statements, char *>(starttk);

				while (system_vcall<BANK_RUNTIME, BANK_MAIN, interpret_statement>() && !runtime_error && *(volatile char *)0x91 != 0x7f)
					;
				system_fcall<BANK_RUNTIME, BANK_MAIN, restore_statements, char *>(starttk);

				if (runtime_error)
				{
					unsigned line = system_tcall<BANK_EDITOR, BANK_MAIN, edit_token_to_line, const char *>(exectk);
					cursortk = system_tcall<BANK_EDITOR, BANK_MAIN, edit_line_to_token, unsigned>(line);
					if (line < screeny || line > screeny + 24)
					{
						screeny = line;
						cursory = 0;
					}
					else
						cursory = line - screeny;
				}
				else
					system_getch();
				system_show_editor();
			} break;
		case PETSCII_F1:
			memcpy(filename.name, p"SAVE", 4);
			if (system_tcall<BANK_EDITOR, BANK_MAIN, edit_cmd, edit_cmd_t &>(filename))
			{
				system_tcall<BANK_EDITOR, BANK_MAIN, tokens_save, const char *>(filename.cmd);
			} break;
		case PETSCII_F2:
			memcpy(filename.name, p"LOAD", 4);
			if (system_tcall<BANK_EDITOR, BANK_MAIN, edit_cmd, edit_cmd_t &>(filename))
			{
				system_tcall<BANK_EDITOR, BANK_MAIN, tokens_load, const char *>(filename.cmd);
			} break;
		}
	}
}