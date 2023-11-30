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
#include "compiler.h"
#include <conio.h>
#include <c64/kernalio.h>
#include <c64/memmap.h>

edit_cmd_t	filename;

#pragma code( mcode )

void ninia_main(void)
{
	symbols_init();

	SYS_VCALL(edit_init);

	system_show_editor();

	// "LOAD SAVE FIND ---- RUN- ---- ---- ---- ----";

	for(;;)
	{
		char ch = SYS_RCALL(edit_text);
	
		switch (ch)
		{
		case PETSCII_F5:
			{
				system_show_runtime();
				SYS_VPCALL(parse_pretty, char *, starttk);
				SYS_VPCALL(prepare_statements, char *, starttk);

				SYS_VPCALL(interpreter_init, char *, starttk);

				while (SYS_RCALL(interpret_statement) && !runtime_error && *(volatile char *)0x91 != 0x7f)
					;
				SYS_VPCALL(restore_statements, char *, starttk);

				if (runtime_error)
				{
					unsigned line = SYS_RPCALL(edit_token_to_line, const char *, exectk);
					cursortk = SYS_RPCALL(edit_line_to_token, unsigned, line);
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
			if (SYS_RPCALL(edit_cmd, edit_cmd_t &, filename))
			{
				SYS_RPCALL(tokens_save, const char *, filename.cmd);
			} break;
		case PETSCII_F2:
			memcpy(filename.name, p"LOAD", 4);
			if (SYS_RPCALL(edit_cmd, edit_cmd_t &, filename))
			{
				SYS_RPCALL(tokens_load, const char *, filename.cmd);
			} break;
		}
	}
}