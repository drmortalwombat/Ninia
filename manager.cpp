#include <c64/vic.h>
#include <c64/asm6502.h>
#include "manager.h"
#include <conio.h>
#include "editor.h"
#include "errors.h"

#pragma code( acode )
#pragma data( adata )

#pragma bss( managebss )

struct dir_entry
{
	char		fname[16];
	unsigned	fsize;
}	dir_entries[100];

char	num_dir_entries;

#pragma bss( bss )

edit_cmd_t	filename, impname;
char		sysdrive;


static char p2smap[] = {0x00, 0x00, 0x40, 0x20, 0x80, 0xc0, 0x80, 0x80};
static char s2pmap[] = {0x40, 0x00, 0x20, 0xc0, 0xc0, 0x80, 0xa0, 0x40};

static inline char p2s(char ch)
{
	return ch ^ p2smap[ch >> 5];
}

static inline char s2p(char ch)
{
	return ch ^ s2pmap[ch >> 5];
}

void manager_cls(void)
{
	memset(Screen, ' ', 24 * 40);
}

void manager_reset_dir(void)
{
	num_dir_entries = 0;
	memset(Screen, ' ', 16 * 40);
}

void manager_scroll_up(void)
{
	char * dp = Screen;
	char * cp = Color;

	for(char y=0; y<23; y++)
	{
		for(signed char x=39; x>=0; x--)
		{
			dp[x] = dp[x + 40];
			cp[x] = cp[x + 40];
		}
		dp += 40;
		cp += 40;
	}

	for(char x=0; x<40; x++)
		dp[x] = ' ';
}

void manager_scroll_down(void)
{
	char * dp = Screen + 23 * 40;
	char * cp = Color + 23 * 40;

	for(char y=0; y<23; y++)
	{
		dp -= 40;
		cp -= 40;
		for(signed char x=39; x>=0; x--)
		{
			dp[x + 40] = dp[x];
			cp[x + 40] = cp[x];
		}
	}

	for(char x=0; x<40; x++)
		dp[x + 40] = ' ';
}

char manager_print_at(char x, char y, char c, const char * msg)
{
	char * dp = Screen + 40 * y + x;
	char * cp = Color + 40 * y + x;

	char i = 0;
	while (msg[i])
	{
		dp[i] = p2s(msg[i]);
		cp[i] = c;
		i++;
	}

	return x + i;
}

const char manager_main_menu[] = 
	p"_Load\t"
	p"_Save\t"
	p"_Import\t"
	p"_Export\n"
	p"_New\t"
	p"_Build\n"
	p"Dri_ve\t"
	p"_Directory\t"
	p"De_lete\t"
	p"_Return\n"
	p"_Quit\n";

void manager_menu(char y, const char * msg)
{
	char * dp = Screen + 40 * y;
	char * cp = Color + 40 * y;

	char i = 0, j = 0;
	while (msg[i])
	{
		if (msg[i] == '\n')
		{
			msg += i + 1;
			i = 0;
			j = 0;
			dp += 40;
			cp += 40;
		}
		else if (msg[i] == '\t')
		{
			i++;
			j += 10 - j % 10;
		}
		else
		{
			if (msg[i] == '_')
			{
				i++;
				cp[j] = VCOL_WHITE;
			}
			else		
				cp[j] = VCOL_LT_BLUE;

			dp[j] = p2s(msg[i]);
			i++;
			j++;
		}
	}
}

void manager_show_status(const char * msg)
{
	char * dp = Screen + 24 * 40;
	char * cp = Color + 24 * 40;

	char color = VCOL_LT_BLUE;
	if (runtime_error)
		color = VCOL_ORANGE;

	char i = 0;
	while (msg[i])
	{
		dp[i] = p2s(msg[i]) | 0x80;
		cp[i] = color;
		i++;
	}
	while (i < 30)
	{
		dp[i] = 160;
		cp[i] = color;
		i++;		
	}	
}

void manager_directory(void)
{
	manager_show_status(p"Reading directory");
	manager_reset_dir();

	runtime_error = RERR_OK;

	// Set name for directory

	krnio_setnam("$");

	// Open #2 on drive 9 (or 8)

	if (krnio_open(2, sysdrive, 0))
	{
		// Switch input to file #2

		if (krnio_chkin(2))
		{
			// Skip BASIC load address
			krnio_chrin();
			krnio_chrin();

			// Loop while we have more lines

			int ch;
			while(num_dir_entries < 100 && !krnio_status() && (ch = krnio_chrin()) > 0)
			{
				unsigned line;
				char	buff[40];			
			
				// Skip second basic link byte	
				krnio_chrin();
				
				// Read line number (size in blocks)
				ch = krnio_chrin();
				line = ch;
				ch = krnio_chrin();
				line += 256 * ch;

				auto & e = dir_entries[num_dir_entries];

				e.fsize = line;
				char n = 0;				
				while ((ch = krnio_chrin()) > 0 && ch != '"')
					;
				if (ch > 0)
				{
					// Read file name, reading till end of basic line
					while ((ch = krnio_chrin()) > 0 && ch != '"')
						e.fname[n++] = ch;
					while ((ch = krnio_chrin()) > 0)
						;
					e.fname[n] = 0;

					if (line > 0)				
						num_dir_entries++;
				}
			}

			if (krnio_status())
				runtime_error = RERR_FILE_READ_ERROR;
			
			// Reset channels

			krnio_clrchn();
		}
		
		// Close file #2
		
		krnio_close(2);
	}

	for(char y = 0; y<32 && y < num_dir_entries; y++)
		manager_print_at(20 * (y & 1), y >> 1, VCOL_LT_GREY, dir_entries[y].fname);
}

bool manager_are_you_sure(void)
{
	manager_show_status(p"Are you sure? (y/n)");
	for(;;)
	{
		char ch = system_getch();
		switch (ch)
		{
		case PETSCII_RETURN:
		case PETSCII_STOP:
		case p'n':
			return false;
		case p'y':
			return true;
		}
	}
}

__asm basicstart
{
	byt 0x01
	byt 0x08
	byt	0x0b
	byt 0x08
	byt	0x0a
	byt	0x00
	byt	0x9e
	byt	'2'
	byt	'0'
	byt	'6'
	byt	'1'
	byt	0x00
	byt	0x00
	byt	0x00

	lda #$36
	sta $01

	lda #$00
	sta $0d
	lda #$10
	sta $0e
	jsr interpret_init
	jsr interpret_program
	jsr interpret_reset

	lda #$37
	sta $01
	rts
}

void manager_build(const char * name)
{
	manager_show_status(p"Building");

	SYS_VPCALL(parse_pretty, starttk);
	SYS_VPCALL(prepare_statements, starttk);

	char	xname[32];
	char i = strbld(xname, "@0:", 0);
	i = strbld(xname, name, i);
	i = strbld(xname, ",P,W", i);

	krnio_setnam(xname);
	if (krnio_open(2, sysdrive, 2))
	{				
		krnio_chkout(2);

		manager_show_status(p"Building Upstart");

		for(char i=0; i<0x28; i++)
			krnio_chrout(((char *)basicstart)[i]);

		for(unsigned i=0x0827; i<0x1000; i++)
			krnio_chrout(0x00);

		manager_show_status(p"Building Program");

		for(unsigned i=0x1000; i<0x8000; i++)
			krnio_chrout(*(char *)i);

		manager_show_status(p"Building Runtime");

		for(unsigned i=0x8000; i<0xc000; i++)
			krnio_chrout(SYS_BREAD(BANK_RUNTIME, (char *)i));

		manager_show_status(p"Building Globals");

		for(unsigned i=0xc000; i<0xd000; i++)
			krnio_chrout(*(char *)i);

		krnio_clrchn();
		krnio_close(2);
	}

	SYS_VPCALL(restore_statements, starttk);
}

void manager_invoke(void)
{
	manager_cls();
	manager_menu(16, manager_main_menu);

	runtime_error = RERR_OK;
	for(;;)
	{
		SYS_VCALL(edit_show_status);

		char ch = system_getch();
		switch (ch)
		{
		case PETSCII_RETURN:
		case PETSCII_STOP:
			return;
		case p'n':
			if (!tkmodified || manager_are_you_sure())
			{
				filename.cmd[0] = 0;
				SYS_VCALL(edit_init);				
				return;
			}
			break;
		case p'l':
			if (!tkmodified || manager_are_you_sure())
			{
				memcpy(filename.name, p"LOAD", 4);
				if (SYS_RRCALL(edit_cmd, filename))
				{
					SYS_RPCALL(tokens_load, filename.cmd);
					SYS_VCALL(edit_restart);
					return;
				} 
			}
			break;
		case p's':
			memcpy(filename.name, p"SAVE", 4);
			if (SYS_RRCALL(edit_cmd, filename))
			{
				SYS_RPCALL(tokens_save, filename.cmd);
				return;
			}
			break;
		case p'b':
			memcpy(impname.name, p"BULD", 4);
			if (SYS_RRCALL(edit_cmd, impname))
			{
				manager_build(impname.cmd);
				return;
			}
			break;
		case p'i':
			impname.cmd[0] = 0;
			memcpy(impname.name, p"IMPR", 4);
			if (SYS_RRCALL(edit_cmd, impname))
			{
				SYS_RPCALL(tokens_import, impname.cmd);
				return;
			}
			break;
		case p'd':
			manager_directory();
			break;
		case p'v':
			sysdrive = sysdrive ^ 1;
			manager_reset_dir();
			break;
		case p'r':
			return;
		case p'q':
			if (!tkmodified || manager_are_you_sure())
			{
				char * rb = (char *)0x0400;
				rb += asm_np(rb, ASM_SEI);
				rb += asm_im(rb, ASM_LDA, 0x04);
				rb += asm_ab(rb, ASM_STA, 0xde02);
				rb += asm_ab(rb, ASM_JMP, 0xfce2);
				__asm {
					jmp $0400
				}
			}
			break;
		}
	}
}

void manager_init(void)
{
	sysdrive = 8;	
}
