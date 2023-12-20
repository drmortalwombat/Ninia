#pragma once

// 4 bit integer
// 0000 xxxx 
// 12 bit integer
// 0001 xxxx xxxx xxxx
// 32 bit integer
// 0010 0000 xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx

// ident
// 0011 iiii iiii iiii

// const
// 0100 iiii iiii iiii
// global
// 0101 iiii iiii iiii
// local
// 0110 iiii iiii iiii

// 1000 iiii binary operator
// 1001 iiii relational operator
// 1010 iiii unary operator
// 1011 iiii assignment operator
// 1100 iiii index operators

// 1101 iiii structural operators

#define TK_TINY_INT			0x00
#define TK_SMALL_INT		0x10
#define TK_NUMBER			0x20

#define TK_IDENT			0x30
#define TK_CONST			0x40
#define TK_GLOBAL			0x50
#define TK_LOCAL			0x60

#define TK_BINARY			0x80
#define TK_RELATIONAL		0x90
#define TK_PREFIX			0xa0
#define TK_POSTFIX			0xb0
#define TK_ASSIGN			0xc0

#define TK_CONTROL			0xf0

#define TK_ADD				(TK_BINARY +  0)
#define TK_SUB				(TK_BINARY +  1)
#define TK_MUL				(TK_BINARY +  2)
#define TK_DIV				(TK_BINARY +  3)
#define TK_MOD				(TK_BINARY +  4)
#define TK_SHL				(TK_BINARY +  5)
#define TK_SHR				(TK_BINARY +  6)
#define TK_AND				(TK_BINARY +  7)
#define TK_OR				(TK_BINARY +  8)

#define TK_EQUAL			(TK_RELATIONAL + 0)
#define TK_NOT_EQUAL		(TK_RELATIONAL + 1)
#define TK_LESS_THAN		(TK_RELATIONAL + 2)
#define TK_LESS_EQUAL		(TK_RELATIONAL + 3)
#define TK_GREATER_THAN		(TK_RELATIONAL + 4)
#define TK_GREATER_EQUAL	(TK_RELATIONAL + 5)

#define TK_NEGATE			(TK_PREFIX + 0)
#define TK_NOT				(TK_PREFIX + 1)
#define TK_LENGTH			(TK_PREFIX + 2)

#define TK_ASSIGN_ADD		(TK_ASSIGN + 1)
#define TK_ASSIGN_SUB		(TK_ASSIGN + 2)

#define TK_INDEX			(TK_POSTFIX + 0)
#define TK_INVOKE			(TK_POSTFIX + 1)


#define TK_END				(TK_CONTROL + 0)
#define TK_COMMA			(TK_CONTROL + 1)
#define TK_STRING			(TK_CONTROL + 2)
#define TK_DOT				(TK_CONTROL + 3)
#define TK_COLON			(TK_CONTROL + 4)
#define TK_LIST				(TK_CONTROL + 5)
#define TK_ARRAY			(TK_CONTROL + 6)
#define TK_STRUCT			(TK_CONTROL + 7)
#define TK_BYTES			(TK_CONTROL + 8)
#define TK_DOTDOT			(TK_CONTROL + 9)

#define STMT_END			0x00
#define STMT_ERROR			0x01
#define STMT_NONE			0x02
#define STMT_COMMENT		0x03

#define STMT_EXPRESSION		0x10
#define STMT_VAR			0x11
#define STMT_DEF			0x12

#define STMT_WHILE			0x20
#define STMT_IF				0x21
#define STMT_ELSIF			0x22
#define STMT_ELSE			0x23
#define STMT_RETURN			0x24
#define STMT_RETURN_NULL	0x25
#define STMT_BREAK			0x26
#define STMT_EXIT			0x27
#define STMT_FOR			0x28
#define STMT_NEXT			0x29


#define TYPE_NUMBER			0x08
#define TYPE_STRING			0x10
#define TYPE_COMPLEX		0x20

#define TYPE_MASK			0x38

#define TYPE_HEAP			0x40
#define TYPE_REF 			0x80

#define TYPE_STRING_SHORT	(TYPE_STRING | 0)
#define TYPE_STRING_LITERAL	(TYPE_STRING | 1)
#define TYPE_STRING_HEAP	(TYPE_STRING | TYPE_HEAP)

#define TYPE_NULL			(TYPE_COMPLEX | 0)
#define TYPE_SYMBOL			(TYPE_COMPLEX | 1)
#define TYPE_FUNCTION		(TYPE_COMPLEX | 2)
#define TYPE_RANGE			(TYPE_COMPLEX | 3)

#define TYPE_ARRAY			(TYPE_COMPLEX | 0 | TYPE_HEAP)
#define TYPE_STRUCT			(TYPE_COMPLEX | 1 | TYPE_HEAP)

#define TYPE_GLOBAL_REF		(TYPE_REF | 0)
#define TYPE_LOCAL_REF		(TYPE_REF | 1)
#define TYPE_ARRAY_REF		(TYPE_REF | 2 | TYPE_HEAP)
#define TYPE_STRUCT_REF		(TYPE_REF | 3 | TYPE_HEAP)

struct Value
{
	char	type;
	long	value;
};

bool is_letter(char c);

bool is_exletter(char c);

bool is_digit(char c);

bool is_hex(char c);


const char * number_format(unsigned long l, bool sign);

char number_parse(const char * str, char n, long & lr);


#pragma compile("tokens.cpp")
