#include "editor.h"
#include "formatter.h"
#include "system.h"
#include <c64/vic.h>
#include <stdio.h>
#include <conio.h>
#include "tokens.h"
#include "errors.h"

#pragma bss( editbss )

char	buffer[200], cbuffer[200];
char	ebuffer[200], ecbuffer[200];

#pragma bss( bss )

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

char cursorx, cursory, screenx;
char * screentk, * cursortk, * endtk, * marktk, *blocktk;
char * starttk, * limittk;
unsigned screeny;

void edit_init(void)
{
	starttk = (char *)0xe000;
	endtk = starttk;
	screentk = cursortk = starttk;
	cursorx = 0;
	cursory = 0;
	screenx = 0;
	screeny = 0;
	*endtk++ = 1;
	*endtk++ = STMT_NONE;
	*endtk++ = STMT_END;
	limittk = (char *)0xfff0;
}

char * edit_screen_to_token(char y)
{
	char * tk = screentk;
	for(char i=0; i<y; i++)
		tk += token_skip_statement(tk);
	return tk;
}

unsigned edit_token_to_line(const char * ct)
{
	char * tk = starttk;
	unsigned line = 0;
	while (*tk && tk <= ct)
	{
		tk += token_skip_statement(tk);
		line++;
	}

	return line - 1;
}

char * edit_line_to_token(unsigned y)
{
	char * tk = starttk;
	for(unsigned i=0; i<y; i++)
		tk += token_skip_statement(tk);
	return tk;
}

const char * edit_display_line(char y, const char * tk)
{
	char mark = (marktk && tk >= marktk && tk < cursortk) ? 0x80 : 0x00;

	tk = format_statement(tk, buffer, cbuffer);

	char i = 0;
	while (i < screenx && buffer[i])
		i++;

	char * dp = Screen + 40 * y;
	char * cp = Color + 40 * y;

	char j = 0;
	while (j < 40 && buffer[i])
	{
		dp[j] = p2s(buffer[i]) | mark;
		cp[j] = cbuffer[i];
		j++; i++;
	}
	while (j < 40)
	{
		dp[j] = 0x20 | mark;
		cp[j] = VCOL_WHITE;
		j++;
	}

	return tk;
}

void edit_refresh_screen(void)
{
	const char * ctk = screentk;
	for(char y = 0; y<24; y++)
		ctk = edit_display_line(y, ctk);
}

void scroll_left(void)
{
	char * dp = Screen;
	char * cp = Color;

	for(char y=0; y<24; y++)
	{
		for(char x=0; x<39; x++)
		{
			dp[x] = dp[x + 1];
			cp[x] = cp[x + 1];
		}
		dp[39] = ' ';
		dp += 40;
		cp += 40;
	}
}

void scroll_right(void)
{
	char * dp = Screen;
	char * cp = Color;

	for(char y=0; y<24; y++)
	{
		for(signed char x=38; x>=0; x--)
		{
			dp[x + 1] = dp[x];
			cp[x + 1] = cp[x];
		}
		dp[0] = ' ';
		dp += 40;
		cp += 40;
	}
}

void edit_scroll_up(void)
{
	screeny++;
	screentk += token_skip_statement(screentk);

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

	edit_display_line(23, edit_screen_to_token(23));
}

void edit_scroll_down(void)
{
	screeny--;
	screentk = edit_line_to_token(screeny);

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

	edit_display_line(0, screentk);	
}

char edit_length(void)
{
	char i = 200;
	while (i > 0 && ebuffer[i - 1] == ' ')
		i--;
	return i;
}

char edit_line(void)
{
	int psz = format_statement(cursortk, ebuffer, ecbuffer) - cursortk;

	char i = 0;
	while (ebuffer[i])
		i++;

	if (cursorx > i)
		cursorx = i;

	while (i < 200)
	{
		ebuffer[i] = ' ';
		ecbuffer[i] = VCOL_LT_BLUE;
		i++;
	}

	char * dp = Screen + 40 * cursory;
	char * cp = Color + 40 * cursory;

	char upy = 24;
	const char * uptk = screentk;
	for(;;)
	{
		dp[cursorx - screenx] |= 0x80;

		char ch;
		if (upy != 24)
			ch = system_getchx();
		else
			ch = system_getch();

		if (ch)
		{
			dp[cursorx - screenx] &= ~0x80;

			bool	redraw = false;

			switch (ch)
			{
			case PETSCII_CURSOR_LEFT:
				if (cursorx > 0)
				{
					cursorx--;
					ch = 0;
				}
				break;
			case PETSCII_CURSOR_RIGHT:
				if (ebuffer[cursorx])
				{
					cursorx++;
					ch = 0;
				}
				break;
			case 95:
				i = 199;
				while (i > 0 && ebuffer[i - 1] == ' ')
					i--;
				cursorx = i;
				ch = 0;
				break;
			case PETSCII_HOME:
				i = 0;
				while (i < 200 && ebuffer[i] == ' ')
					i++;
				if (i >= cursorx)
					cursorx = 0;
				else
					cursorx = i;
				ch = 0;
				break;

			case PETSCII_DEL:
				if (cursorx > 0)
				{
					cursorx--;
					i = cursorx;
					while (i < 199)
					{
						ebuffer[i] = ebuffer[i + 1];
						ecbuffer[i] = ecbuffer[i + 1];
						i++;
					}
					redraw = true;
					ch = 0;
				}
				break;


			case S'Y':
			case S'X':
			case S'V':
			case S'C':
			case S'B':
			case PETSCII_CURSOR_UP:
			case PETSCII_CURSOR_DOWN:
			case '\n':
			case PETSCII_F1:
			case PETSCII_F2:
			case PETSCII_F3:
			case PETSCII_F4:
			case PETSCII_F5:
			case PETSCII_F6:
			case PETSCII_F7:
			case PETSCII_F8:
				break;
			default:
				if (ch >= ' ' && ch <= 127 || ch >= 160)
				{
					i = 199;
					while (i > cursorx)
					{
						i--;
						ebuffer[i + 1] = ebuffer[i];
						ecbuffer[i + 1] = ecbuffer[i];					
					}
					ebuffer[i] = ch;
					ecbuffer[i] = VCOL_WHITE;
					cursorx++;
					redraw = true;
				}
				ch = 0;
				break;
			}

			if (ch)
			{
				i = edit_length();
				ebuffer[i] = 0;

				int nsz = parse_statement(ebuffer, buffer) - buffer;

				if (ch == '\n')
				{
					if (upy > cursory)
					{
						upy = cursory;
						uptk = cursortk;
					}

					if (cursorx < buffer[0])
					{
						cursortk[0] = buffer[0];
						cursortk[1] = STMT_NONE;
						cursortk += 2;
						cursory++;
						psz-=2;
						ch = ' ';
					}
					else
					{
						buffer[nsz++] = buffer[0];
						buffer[nsz++] = STMT_NONE;							
					}
				}
				else
				{
					if (upy < 24 && upy > cursory)
					{
						upy = cursory;
						uptk = cursortk;
					}
				}

				memmove(cursortk + nsz, cursortk + psz, endtk - (cursortk + psz));
				memcpy(cursortk, buffer, nsz);
				endtk += nsz - psz;

				if (upy > cursory)
					edit_display_line(cursory, cursortk);

				while (upy < 24)
				{
					uptk = edit_display_line(upy, uptk);
					upy++;
				}
				return ch;
			}

			if (screenx > 0 && cursorx < screenx + 5)
			{
				if (cursorx == screenx + 4)
				{
					scroll_right();
					screenx--;
					dp[0] = p2s(ebuffer[screenx]);
					cp[0] = ecbuffer[screenx];
				}
				else
				{
					if (cursorx > 5)
						screenx = cursorx - 5;
					else
						screenx = 0;
					redraw = true;
				}
				upy = 0;
				uptk = screentk;
			}
			else if (cursorx > screenx + 35)
			{
				if (cursorx == screenx + 36)
				{
					scroll_left();
					screenx++;
					dp[39] = p2s(ebuffer[screenx + 39]);
					cp[39] = ecbuffer[screenx + 39];
				}
				else
				{
					screenx = cursorx - 35;
					redraw = true;
				}
				upy = 0;
				uptk = screentk;
			}

			if (redraw)
			{
				for(char i=0; i<40; i++)
				{
					dp[i] = p2s(ebuffer[screenx + i]);
					cp[i] = ecbuffer[screenx + i];
				}
			}
		}
		else if (upy < 24)
		{
			if (upy != cursory)
				uptk = edit_display_line(upy, uptk);
			else
				uptk += token_skip_statement(uptk);
			upy++;
		}
	}
}


void edit_show_status(void)
{
	const char * err = runtime_error_names[runtime_error];

	char * dp = Screen + 24 * 40;
	char * cp = Color + 24 * 40;

	char color = VCOL_LT_BLUE;
	if (runtime_error)
		color = VCOL_ORANGE;

	char i = 0;
	while (err[i])
	{
		dp[i] = p2s(err[i]) | 0x80;
		cp[i] = color;
		i++;
	}
	while (i < 30)
	{
		dp[i] = 160;
		cp[i] = color;
		i++;		
	}

	char str[6];
	utoa(cursory + screeny + 1, str, 10);
	char j = 0;
	while (str[j])
	{
		dp[i] = str[j] | 0x80;
		cp[i] = color;
		i++;
		j++;
	}

	while (i < 35)
	{
		dp[i] = 160;
		cp[i] = color;
		i++;		
	}

	utoa(limittk - endtk, str, 10);
	j = 0;
	while (str[j])
	{
		dp[i] = str[j] | 0x80;
		cp[i] = color;
		i++;
		j++;
	}

	while (i < 40)
	{
		dp[i] = 160;
		cp[i] = color;
		i++;		
	}
}
