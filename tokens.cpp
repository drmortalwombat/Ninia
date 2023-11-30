#include "tokens.h"
#include <fixmath.h>


static inline unsigned long lmul10(unsigned long l)
{
	return (l + 4 * l) * 2;
}

const char * number_format(unsigned long l, bool sign)
{
	static char buffer[16];

	if (sign && (long)l < 0)
		l = -(long)l;
	else
		sign = false;
	
	// rounding 0.00005
	l += 3;

	char		si = 6;	
	unsigned	hi = l >> 16;
	if (hi == 0)
		buffer[--si] = '0';
	else
	{
		char i = 0;
		while (hi)
		{
			buffer[--si] = hi % 10 + '0';
			hi /= 10;
			i++;
		}		
	}
	if (sign)
		buffer[--si] = '-';

	char ei = 6;

	l &= 0xffff;
	if (l > 5)
	{
		buffer[ei++] = '.';
		for(char i=0; i<4; i++)
		{
			l = lmul10(l);
			//l *= 10;
			buffer[ei++] = char(l >> 16) + '0';
			l &= 0xfffful;
		}
		while (buffer[ei-1] == '0')
			ei--;
		if (buffer[ei-1] == '.')
			ei--;
	}
	buffer[ei] = 0;

	return buffer + si;
}

bool is_letter(char c)
{
	return c >= p'a' && c <= p'z' || c >= p'A' && c <= p'Z';
}

bool is_exletter(char c)
{
	return c >= p'a' && c <= p'z' || c >= p'A' && c <= p'Z' || c >= '0' && c <= '9' || c == '_';
}

bool is_digit(char c)
{
	return c >= '0' && c <= '9';
}

unsigned long fscale[8] = {
	858993459,
	85899345,
	8589934,
	858993,	
	85899,
	8589,
	858,
	85,
};

char number_parse(const char * str, char n, long & lr)
{
	char 	i = 0;
	bool	sign = false;

	while(i < n && str[i] == ' ')
		i++;

	if (i < n && str[i] == '-')
	{
		sign = true;
		i++;
	}

	unsigned num = 0;
	while (i < n && is_digit(str[i]))
	{
		num = num * 10 + (str[i] - '0');
		i++;
	}

	lr = (long)num << 16;
	if (i < n && str[i] == '.')
	{

		unsigned long fract = 0;
		i++;

		char k = 0;
		while (i < n && is_digit(str[i]))
		{
			k++;
			fract = lmul10(fract) + (str[i] - '0');
			i++;
		} 
		if (k > 0)
			lr += (unsigned long)(lmul16f16s(fract, fscale[k - 1]) + 1) >> 1;
	}

	if (sign)
		lr = -lr;

	return i;
}

