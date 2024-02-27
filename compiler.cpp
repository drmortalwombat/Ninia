#include "interpreter.h"
#include "compiler.h"
#include "tokens.h"
#include "runtime.h"
#include "manager.h"
#include "errors.h"

#pragma code(tcode)
#pragma data(tdata)

char			num_locals;
char	*		functk;

unsigned global_find(unsigned symbol)
{
	char	vi = 0;
	while (vi < num_globals && global_symbols[vi] != symbol)
		vi++;
	return vi;
}

unsigned global_add(unsigned symbol)
{
	global_symbols[num_globals] = symbol;

	globals[num_globals].value = 0;
	globals[num_globals].type = TYPE_NULL;
	return num_globals++;
}

unsigned local_find(unsigned symbol)
{
	char i = 0;
	for(char i=0; i<num_locals; i++)
		if (localvars[i] == symbol)
			return i;
	return num_locals;
}

unsigned local_add(unsigned symbol)
{
	local_symbols[num_local_symbols++] = symbol;
	localvars[num_locals] = symbol;
	return num_locals++;
}


char * prepare_expression(char * tk, bool var)
{
	char	tstack[32];
	char	tsp = 0;

	char	i = 0;
	char 	t = *tk;
	bool	left = true;
	while (t != TK_END)
	{
		switch (t & 0xf0)
		{
		case TK_TINY_INT:
			tstack[tsp++] = i;
			i ++;
			left = false;
			break;
		case TK_SMALL_INT:
			tstack[tsp++] = i;
			i += 2;
			left = false;
			break;
		case TK_NUMBER:
			tstack[tsp++] = i;
			i += 5;
			left = false;
			break;

		case TK_IDENT:
		case TK_CONST:	
		case TK_GLOBAL:	
		case TK_LOCAL:
			{
				tstack[tsp++] = i;

				char tt = t & 0xf0;
				unsigned ti = ((t & 0x03) << 8) | tk[i + 1];
				if (tt == TK_IDENT && !(t & 8))
				{
					if (left && var)
					{
						if (functk)
							local_add(ti);
						else
							global_add(ti);
					}

					unsigned	vi = local_find(ti);
					if (vi < num_locals)
					{
						tk[i + 0] = (vi >> 8) | TK_LOCAL;
						tk[i + 1] = vi & 0xff;
					}
					else
					{
						vi = global_find(ti);
						if (vi < num_globals)
						{
							tk[i + 0] = (vi >> 8) | TK_GLOBAL;
							tk[i + 1] = vi & 0xff;
						}
					}
				}
			i += 2;
			left = false;
			}	break;

		case TK_RELATIONAL:
		case TK_BINARY:
			tsp -= 2;
			tstack[tsp++] = i;
			i++;
			left = false;
			break;

		case TK_PREFIX:
			tsp --;
			tstack[tsp++] = i;
			i++;
			left = false;
			break;

		case TK_POSTFIX:
			switch (t)
			{
			case TK_INDEX:
				tsp -= 2;
				break;
			case TK_INVOKE:
				tsp -= tk[i + 1] + 1;
				break;
			}

			tstack[tsp++] = i;
			i += 2;
			left = false;
			break;

		case TK_ASSIGN:
			{
				tsp -= 2;
				char j = tstack[tsp];
				char tj = tk[j];
				if (tj == TK_INDEX)
					tk[j] = TK_LINDEX;
				else if (tj == TK_DOT)
					tk[j] = TK_LDOT;
				else if ((tj & 0xf0) == TK_GLOBAL || (tj & 0xf0) == TK_LOCAL)
					tk[j] |= 0x08;
				else
				{
					runtime_error = RERR_INVALID_ASSIGN;
					exectk = tk + i;
				}
				tstack[tsp++] = i;
				i++;
			}	break;

		case TK_CONTROL:
			{
				switch (t)
				{
				case TK_COMMA:
					tstack[tsp++] = i;
					i++;
					left = true;
					break;
				case TK_STRING:
				case TK_BYTES:
					tstack[tsp++] = i;
					i += tk[i + 1] + 2;
					left = false;
					break;
				case TK_DOTDOT:
				case TK_DOT:
					tsp -= 2;
					tstack[tsp++] = i;
					i++;
					left = false;
					break;
				case TK_COLON:
					i++;
					left = false;
					break;
				case TK_LIST:
				case TK_ARRAY:
					tsp -= tk[i + 1];
					tstack[tsp++] = i;
					i += 2;
					left = false;
					break;
				case TK_STRUCT:
					tsp -= tk[i + 1];
					tstack[tsp++] = i;
					i += 2 * tk[i + 1] + 2;
					left = false;
					break;
				case TK_NULL:
					tstack[tsp++] = i;
					i ++;
					left = false;
					break;

				default:
					i++;
				}
			} break;

		default:
			i++;
			left = false;
		}
		t = tk[i];
	}

	return tk + i + 1;
}

char * prepare_function(char * tk)
{
	unsigned ti = ((tk[0] & 0x03) << 8) | tk[1];
	char vi = global_add(ti);

	tk += 2;

	while ((tk[0] & 0xf0) == TK_IDENT)
	{
		local_add(((tk[0] & 0x03) << 8) | tk[1]);
		tk += 2;
	}

	globals[vi].type = TYPE_FUNCTION;
	globals[vi].value = unsigned(tk);
	functk = tk;

	return tk + 3;
}

void prepare_statements(char * tk)
{
	num_globals = 0;
	num_locals = 0;
	num_local_symbols = 0;
	runtime_error = 0;
	functk = nullptr;

	char 		pl = 1;
	char 		l = *tk;
	char 	*	pt = nullptr;

	while (l > 0)
	{	
		if (tk[1] == STMT_NONE)		
			tk += 2;
		else
		{
			while (l < pl)
			{
				char * ppt = (char *)(pt[0] + (pt[1] << 8));
				pt[0] = (unsigned)tk & 0xff;
				pt[1] = (unsigned)tk >> 8;
				pt = ppt;
				pl--;
			}

			if (l == 1 && functk)
			{
				functk[1] = num_locals;
				num_locals = 0;
				functk = nullptr;
			}

			tk++;
			char 	t = *tk++;
			switch (t)
			{
			case STMT_EXPRESSION:
			case STMT_RETURN:
				tk = prepare_expression(tk, false);
				break;
			case STMT_VAR:
				tk = prepare_expression(tk, true);
				break;
			case STMT_DEF:
				tk[0] = (unsigned)pt & 0xff;
				tk[1] = (unsigned)pt >> 8;
				pt = tk;
				tk = prepare_function(tk + 2);
				pl++;
				break;		
			case STMT_RETURN_NULL:
			case STMT_NONE:
			case STMT_BREAK:
			case STMT_EXIT:
				break;
			case STMT_WHILE:
			case STMT_IF:
			case STMT_ELSIF:
				tk[0] = (unsigned)pt & 0xff;
				tk[1] = (unsigned)pt >> 8;
				pt = tk;
				tk = prepare_expression(tk + 2, false);
				pl++;
				break;		
			case STMT_FOR:
				tk = prepare_expression(tk, true);
				break;						
			case STMT_NEXT:
			case STMT_ELSE:
				tk[0] = (unsigned)pt & 0xff;
				tk[1] = (unsigned)pt >> 8;
				pt = tk;
				tk += 2;
				pl++;
				break;		
			case STMT_COMMENT:
				tk += tk[0] + 1;
				break;
			case STMT_FOLD:
				tk += 2;
				break;
			}
		}
		l = *tk;
	}

	while (pl > 1)
	{
		char * ppt = (char *)(pt[0] + (pt[1] << 8));
		pt[0] = (unsigned)tk & 0xff;
		pt[1] = (unsigned)tk >> 8;
		pt = ppt;
		pl--;
	}
}

char * restore_expression(char * tk, bool var)
{
	char 	t = *tk;
	bool	left = true;

	while (t != TK_END)
	{
		switch (t & 0xf0)
		{
		case TK_SMALL_INT:
			tk += 2;
			left = false;
			break;
		case TK_NUMBER:
			tk += 5;
			left = false;
			break;

		case TK_IDENT:
		case TK_CONST:	
		case TK_GLOBAL:	
		case TK_LOCAL:
			{
				char tt = t & 0xf0;
				unsigned ti = ((t & 0x03) << 8) | tk[1];
				if (tt == TK_GLOBAL)
				{
					unsigned vi = global_symbols[ti];
					tk[0] = (vi >> 8) | TK_IDENT;
					tk[1] = vi & 0xff;
				}
				else if (tt == TK_LOCAL)
				{
					unsigned	vi;
					if (left && var)
					{
						vi = local_symbols[num_local_symbols++];
						localvars[ti] = vi;
					}
					else
						vi = localvars[ti];

					tk[0] = (vi >> 8) | TK_IDENT;
					tk[1] = vi & 0xff;					
				}
			tk += 2;
			left = false;
			}	break;			

		case TK_POSTFIX:
			if (t == TK_LINDEX)
				tk[0] = TK_INDEX;

			tk += 2;
			left = false;
			break;

		case TK_CONTROL:
			{
				switch (t)
				{
				case TK_COMMA:
					tk++;
					left = true;
					break;
				case TK_STRING:
				case TK_BYTES:
					tk += tk[1] + 2;
					left = false;
					break;
				case TK_LDOT:
					tk[0] = TK_DOT;
					tk++;
					left = false;
					break;
				case TK_COLON:
				case TK_DOT:
				case TK_DOTDOT:
					tk++;
					left = false;
					break;
				case TK_LIST:
				case TK_ARRAY:
					tk += 2;
					left = false;
					break;
				case TK_STRUCT:
					tk += 2 * tk[1] + 2;
					left = false;
					break;
				default:
					tk++;
				}
			} break;

		default:
			tk++;
			left = false;
		}
		t = *tk;
	}
	return tk + 1;
}

char * restore_function(char * tk)
{
	functk = tk;

	tk += 2;

	char n = 0;
	while ((tk[0] & 0xf0) == TK_IDENT)
	{		
		local_add(((tk[0] & 0x03) << 8) | tk[1]);
		tk += 2;
		n++;
	}

	tk[1] = n;

	return tk + 3;
}


void restore_statements(char * tk)
{
	num_locals = 0;
	functk = nullptr;	
	num_local_symbols = 0;

	char	l = *tk;
	while (l > 0)
	{		
		tk++;
		char 	t = *tk++;

		if (t != STMT_NONE)
		{
			if (l == 1 && functk)
			{
				num_locals = 0;
				functk = nullptr;
			}

			switch (t)
			{
			case STMT_EXPRESSION:
			case STMT_RETURN:
				tk = restore_expression(tk, false);
				break;
			case STMT_VAR:
				tk = restore_expression(tk, true);
				break;
			case STMT_RETURN_NULL:
			case STMT_NONE:
			case STMT_BREAK:
			case STMT_EXIT:
				break;
			case STMT_WHILE:
			case STMT_IF:
			case STMT_ELSIF:
				tk[0] = 0;
				tk[1] = 0;
				tk = restore_expression(tk + 2, false);
				break;		
			case STMT_FOR:
				tk = restore_expression(tk, true);
				break;						
			case STMT_NEXT:
			case STMT_ELSE:
				tk[0] = 0;
				tk[1] = 0;
				tk += 2;
				break;			
			case STMT_DEF:
				tk[0] = 0;
				tk[1] = 0;
				tk = restore_function(tk + 2);
				break;				
			case STMT_COMMENT:
				tk += tk[0] + 1;
				break;
			case STMT_FOLD:
				tk += 2;
				break;
			}
		}
		l = *tk;
	}	
}

char token_skip_expression(const char * tk)
{
	char	ti = 0;
	char 	t = tk[ti];

	while (t != TK_END)
	{
		switch (t & 0xf0)
		{
		case TK_SMALL_INT:
			ti += 2;
			break;
		case TK_NUMBER:
			ti += 5;
			break;

		case TK_IDENT:
		case TK_CONST:	
		case TK_GLOBAL:	
		case TK_LOCAL:
			ti += 2;
			break;

		case TK_POSTFIX:
			ti += 2;
			break;

		case TK_CONTROL:
			switch (t)
			{
			case TK_STRING:
			case TK_BYTES:
				ti += tk[ti + 1] + 2;
				break;
			case TK_LIST:
			case TK_ARRAY:
				ti += 2;
				break;
			case TK_STRUCT:
				ti += 2 + 2 * tk[ti + 1];
				break;
			default:
				ti++;
			}
			break;

		default:
			ti++;
		}

		t = tk[ti];
	}

	return ti + 1;
}

unsigned token_statement_size(const char * tk)
{
	unsigned ti = 0;
	if (tk[0])
	{
		ti++;
		char t = tk[ti++];
		switch (t)
		{
		case STMT_DEF:
		case STMT_WHILE:
		case STMT_IF:
		case STMT_ELSIF:
			ti += 2;
			ti += token_skip_expression(tk + ti);
			break;
		case STMT_ELSE:
			ti += 2;
			break;
		case STMT_EXPRESSION:
		case STMT_VAR:
		case STMT_RETURN:
			ti += token_skip_expression(tk + ti);
			break;
		case STMT_FOR:
			ti += token_skip_expression(tk + ti);
			ti += 4;
			break;
		case STMT_NONE:
		case STMT_RETURN_NULL:
		case STMT_BREAK:
		case STMT_EXIT:
			break;
		case STMT_ERROR:
		case STMT_COMMENT:
			ti += tk[ti] + 1;
			break;
		case STMT_FOLD:
			ti += tk[ti] + 256 * tk[ti + 1];
			break;
		}
	}

	return ti;	
}

const char * token_skip_statement(const char * tk)
{	
	return tk + token_statement_size(tk);
}

char * token_skip_statement(char * tk)
{
	return tk + token_statement_size(tk);
}


char * edit_screen_to_token(char y)
{
	char * tk = screentk;
	for(char i=0; i<y; i++)
		tk = token_skip_statement(tk);
	return tk;
}

unsigned edit_token_to_line(const char * ct)
{
	char * tk = starttk;
	unsigned line = 0;
	while (*tk && tk <= ct)
	{
		tk = token_skip_statement(tk);
		line++;
	}

	return line - 1;
}

char * edit_line_to_token(unsigned y)
{
	char * tk = starttk;
	for(unsigned i=0; i<y; i++)
		tk = token_skip_statement(tk);
	return tk;
}

char * tokens_file_load(char * tk, bool import)
{
	krnio_chkin(2);

	char * foldtk = nullptr;

	char status = krnio_status();
	while (!status)
	{
		char i = 0;
		bool fold = false;
		while ((char c = krnio_chrin()) != 10 && !(status = krnio_status()))
		{
			if (c == 187)
				fold = true;
			else if (c == 13)
				;
			else
			{
				if (c >= 'A' && c <= 'Z')
					c += p'A' - 'A';
				else if (c >= 'a' && c <= 'z')
					c -= 'a' - p'a';
				buffer[i++] = c;
			}
		}

		buffer[i] = 0;
		if (fold)
		{
			foldtk = tk;
			tk += 4;
		}
		char * ntk = parse_statement((char *)buffer, tk);			
		if (fold)
		{
			foldtk[0] = tk[0];
			foldtk[1] = STMT_FOLD;				
		}
		else if (foldtk && foldtk[0] >= tk[0])
		{
			unsigned d = ntk - foldtk - 2;
			foldtk[2] = d & 0xff;
			foldtk[3] = d >> 8;
			foldtk = nullptr;
		}
		if (import && tk[1] == STMT_COMMENT && tk[3] == '!')
			break;

		tk = ntk;

	}
	krnio_clrchn();
	krnio_close(2);

	return tk;
}


bool tokens_import(const char * name)
{
	char	xname[32];
	strcpy(xname, "0:");
	strcat(xname, name);
	strcat(xname, ",P,R");

	krnio_setnam(xname);
	if (krnio_open(2, sysdrive, 2))
	{				
		// Make space for new data
		unsigned	esize = endtk - cursortk;
		memmove(limittk - esize, cursortk, esize);

		char * tk = tokens_file_load(cursortk, true);

		memmove(tk, limittk - esize, esize);
		endtk = tk + esize;

		tkmodified = true;

		return true;
	}

	return false;
}

bool tokens_load(const char * name)
{
	char	xname[32];
	strcpy(xname, "0:");
	strcat(xname, name);
	strcat(xname, ",P,R");

	krnio_setnam(xname);
	if (krnio_open(2, sysdrive, 2))
	{				
		edit_init();

		char * tk = tokens_file_load(starttk, false);

		if (tk != starttk)
		{
			*tk++ = STMT_END;
			endtk = tk;

			tkmodified = false;

			return true;
		}
	}

	return false;
}

bool tokens_save(const char * name)
{
	char	xname[32];
	strcpy(xname, "@0:");
	strcat(xname, name);
	strcat(xname, ",P,W");

	krnio_setnam(xname);
	if (krnio_open(2, sysdrive, 2))
	{				
		krnio_chkout(2);

		const char * tk = starttk;
		while (*tk)
		{
			if (tk[1] == STMT_FOLD)
			{
				krnio_chrout(187);
				tk += 4;
			}

			tk = format_statement(tk, buffer, cbuffer);

			char i =0 ;
			while (char c = buffer[i])
			{
				if (c >= p'A' && c <= p'Z')
					c -= p'A' - 'A';
				else if (c >= p'a' && c <= p'z')
					c += 'a' - p'a';
				else if (c == 0xa0)
					c = ' ';
				krnio_chrout(c);
				i++;
			}
			krnio_chrout(10);
		}

		krnio_clrchn();
		krnio_close(2);

		tkmodified = false;

		return true;
	}

	return false;
}

