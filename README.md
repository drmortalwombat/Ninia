# Ninia
*A simplified Python/JS/BASIC hybrid for the C64.*

## Targets and Limits

This project is the result or rather an experiment, trying to find an answer to the question:

What would we have used instead of 80's BASIC in the home computers of the 8 bit era, if we would have known and loved all the languages of today.

The following targets and limitations, guided the design and implementation:

* Structured programming (no line numbers)
* Compact style, due to limited screen real estate
* Recursive functions with arguments and function variables
* Responsive full screen editor
* Fast turn around times, execution out of RAM, no compilation
* Tape as a potential storage medium
* Compact code storage, tokenized source
* Execution time at least twice as fast as BASIC
* Dynamic size array, strings and structs
* Garbage collection
* Dynamic typing
* Fractional numbers (need not be floating point)
* Infix notation for common arithmetic operators

## Language

The language uses a Python style syntax with indentation for block structures.  This is a very compact way of representing a block structured language, no need for curly braces or end.  This is not my favorite style but it has grown on me while implementing the interpreter.

	var i = 0
	while i<10
	 print(i, "\n")
	 i+=1

The number model is taken from Pico-8.  Numbers are represented using a 16.16 fractional encoding.  This gives a numeric range from -32768 to 32767.9999, with four full decimal fractions, and is faster than actual floating point.

Most common operators are available and use the following precedence

	* / %	: Multiply, divide, modulus
	<< >>	: Left shift, right shift
	+ -		: Addition subtraction
	== != < <= > >=	: Comparison
	&		: And
	|		: Or
	= += -=	: Assignment
	,		: Sequence

Strings have a maximum size of 255 characters and grow or shrink as required. Arrays can grow up to the maximum memory size.  Each array element requires five bytes, so a maximum of about 4000 elements is possible.  Structures use a fixed set of elements once declared, but no actual typing.  Strings and arrays can be concatenated using the + operator.

	var a="Hello"
	var b="World"
	print(a + " " + b + "\n");

Elements of arrays and strings can be extracted using square bracket postfix operator a[n].  A sub range can also be taken using a two dot ellipsis range a[1..2].  Indices start at zero.  The size of arrays and strings can be computed using the hash prefix operator.

	var a=[1,2,3,4]
	var b=a[2..3]
	print(#b)

Structs are declared when created.  Members can be changed, but not added or removed once created.

	var p={x:1,y:2}
	p.x += 10
	print(p.x, " ", p.y, "\n")

Functions can be declared with the def statement.  A function may exit once its block scope ends or a return statement is reached.  Parameters are always passed by value.  A function may return a struct if multiple return values are needed.

	def sum(a)
	 var i=0,s=0
	 while i<#a
	  s+=a[i]
	  i+=1
	 return s

## Tokenization

The source code is tokenized on a by line method.  The first byte of a line is the depth of indentation plus one, or zero for the end of the program.  The second byte is the statement.  Some statements are followed by two additional bytes that point to the end of the block scope controlled by this statement (if, else, elsif, while, def) to allow for fast skip.

Most statements are followed by an expression.  The expressions are tokenized using a stack based operator order (similar to FORTH) to allow sequential interpretation without the need to check for operator precedence while executing.

Numbers are parsed into a binary form of one, two or five bytes depending on their size and complexity.  Variables are stored as references to their name in a global symbol store during editing and as indices into global and local variable arrays during execution.



