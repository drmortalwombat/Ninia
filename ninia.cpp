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
#include "manager.h"
#include <conio.h>
#include <c64/kernalio.h>
#include <c64/memmap.h>

#pragma code( mcode )

void ninia_main(void)
{
	SYS_VCALL(symbols_init);
	SYS_VCALL(manager_init);

	SYS_VCALL(edit_init);

	system_show_editor();

	for(;;)
	{
		char ch = SYS_RCALL(edit_text);
	
		switch (ch)
		{
		case PETSCII_F5:
			{
				system_show_runtime();
				SYS_VPCALL(parse_pretty, starttk);
				SYS_VPCALL(prepare_statements, starttk);

				SYS_VPCALL(interpret_init, starttk);
				SYS_VCALL(interpret_program);
				SYS_VCALL(interpret_reset);

				SYS_VPCALL(restore_statements, starttk);

				if (runtime_error)
				{
					unsigned line = SYS_RPCALL(edit_token_to_line, exectk);
					cursortk = SYS_RPCALL(edit_line_to_token, line);
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
			if (SYS_RRCALL(edit_cmd, filename))
			{
				SYS_RPCALL(tokens_save, filename.cmd);
			} break;
		case PETSCII_F2:
			memcpy(filename.name, p"LOAD", 4);
			if (SYS_RRCALL(edit_cmd, filename))
			{
				SYS_RPCALL(tokens_load, filename.cmd);
				SYS_VCALL(edit_restart);
			} break;
		case PETSCII_F7:
			SYS_VCALL(manager_invoke);
			system_show_editor();
			break;
		}
	}
}