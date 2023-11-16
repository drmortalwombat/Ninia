#include "editor.h"
#include "formatter.h"
#include <c64/vic.h>
#include <stdio.h>
#include <conio.h>
#include "tokens.h"

char	buffer[200], cbuffer[200];

//static char p2smap[] = {0x00, 0x20, 0x00, 0x40, 0x00, 0x60, 0x40, 0x60};
static char p2smap[] = {0x00, 0x00, 0x40, 0x20, 0x80, 0xc0, 0x80, 0x80};
//static char s2pmap[] = {0x40, 0x20, 0x60, 0xa0, 0x40, 0x20, 0x60, 0xa0};
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
char * screentk, * cursortk, * endtk;

const char * edit_display_line(char y, const char * tk)
{
	tk = format_statement(tk, buffer, cbuffer);

	char i = 0;
	while (i < screenx && buffer[i])
		i++;

	char * dp = Screen + 40 * y;
	char * cp = Color + 40 * y;
	char j = 0;
	while (j < 40 && buffer[i])
	{
		dp[j] = p2s(buffer[i]);
		cp[j] = cbuffer[i];
		j++; i++;
	}
	while (j < 40)
	{
		dp[j] = 0x20;
		cp[j] = VCOL_WHITE;
		j++;
	}

	return tk;
}

void edit_refresh_screen(void)
{
	const char * ctk = screentk;
	char y = 0;
	while (*ctk)
	{
		ctk = edit_display_line(y, ctk);
		y++;
	}	
}

char	ebuffer[200], ecbuffer[200];

void scroll_left(void)
{
	char * dp = Screen;
	char * cp = Color;

	for(char y=0; y<25; y++)
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

	for(char y=0; y<25; y++)
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
	while (i < 200)
	{
		ebuffer[i] = ' ';
		ecbuffer[i] = VCOL_LT_BLUE;
		i++;
	}

	char * dp = Screen + 40 * cursory;
	char * cp = Color + 40 * cursory;

	char upy = 25;
	const char * uptk = screentk;
	for(;;)
	{
		dp[cursorx - screenx] |= 0x80;

		char ch;
		if (upy != 25)
			ch = getchx();
		else
			ch = getch();

		if (ch)
		{
			dp[cursorx - screenx] &= ~0x80;

			bool	redraw = false;

			switch (ch)
			{
			case PETSCII_CURSOR_LEFT:
				if (cursorx > 0)
					cursorx--;
				break;
			case PETSCII_CURSOR_RIGHT:
				if (ebuffer[cursorx])
					cursorx++;
				break;
			case 95:
				i = 199;
				while (i > 0 && ebuffer[i - 1] == ' ')
					i--;
				cursorx = i;
				break;
			case PETSCII_HOME:
				i = 0;
				while (i < 200 && ebuffer[i] == ' ')
					i++;
				if (i >= cursorx)
					cursorx = 0;
				else
					cursorx = i;
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
				}
				else
				{
					if (edit_length() == 0)
					{
						memmove(cursortk, cursortk + psz, endtk - (cursortk + psz));
						endtk -= psz;
						if (upy >= cursory)
						{
							upy = cursory;
							uptk = cursortk;
						}

						while (upy < 25)
						{
							uptk = edit_display_line(upy, uptk);
							upy++;
						}
						return ch;						
					}
				}
				break;
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
						if (upy < 25 && upy > cursory)
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

					while (upy < 25)
					{
						uptk = edit_display_line(upy, uptk);
						upy++;
					}
					return ch;
				} break;
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
			}

			if (screenx > 0 && cursorx < screenx + 5)
			{
				scroll_right();
				screenx--;
				dp[0] = p2s(ebuffer[screenx]);
				cp[0] = ecbuffer[screenx];
				upy = 0;
				uptk = screentk;
			}
			else if (cursorx > screenx + 35)
			{
				scroll_left();
				screenx++;
				dp[39] = p2s(ebuffer[screenx + 39]);
				cp[39] = ecbuffer[screenx + 39];
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
		else if (upy < 25)
		{
			if (upy != cursory)
				uptk = edit_display_line(upy, uptk);
			else
				uptk += token_skip_statement(uptk);
			upy++;
		}
	}
}

