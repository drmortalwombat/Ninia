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

### Operators

Most common operators are available and use the following precedence

	* / %	: Multiply, divide, modulus
	<< >>	: Left shift, right shift
	+ -		: Addition subtraction
	== != < <= > >=	: Comparison
	&		: And
	|		: Or
	= += -=	: Assignment
	,		: Sequence


### Strings

Strings have a maximum size of 255 characters and grow or shrink as required. Arrays can grow up to the maximum memory size.  Each array element requires five bytes, so a maximum of about 4000 elements is possible.  Structures use a fixed set of elements once declared, but no actual typing.  Strings and arrays can be concatenated using the + operator.

	var a="Hello"
	var b="World"
	print(a + " " + b + "\n");
	
String literals can also be defined using the hex notation with a dollar sign:

	var ch=$183c66c3663c1800

### Arrays

Elements of arrays and strings can be extracted using square bracket postfix operator a[n].  A sub range can also be taken using a two dot ellipsis range a[1..2].  Indices start at zero.  The size of arrays and strings can be computed using the hash prefix operator.

	var a=[1,2,3,4]
	var b=a[2..3]
	print(#b)

### Structs

Structs are declared when created.  Members can be changed, but not added or removed once created.

	var p={x:1,y:2}
	p.x += 10
	print(p.x, " ", p.y, "\n")

### Functions

Functions can be declared with the def statement.  A function may exit once its block scope ends or a return statement is reached.  Parameters are always passed by value.  A function may return a struct if multiple return values are needed.

	def sum(a)
	 var i=0,s=0
	 while i<#a
	  s+=a[i]
	  i+=1
	 return s

### Loops

Two types of loops are available, while loops and for loops.  For loops may iterate over the elements of an array, or the numbers in a numeric range:

	for i=1..10
	  print(i,"\n")

### Conditionals
	  
Conditional statements are composed of "if", "elsif" and "else" sections

	for i=1..10
	  if i<3
	   print("a")
	  elsif i<7
	   print("b")
	  else
	   print("c")

## Built in functions

### Numeric builtins

#### abs

	var y = abs(x)

Returns the absolute value of a number.

#### rand

	var y = rand()
	
Returns a random fractional number in the range 0 to 1

#### floor

	var y = floor(x)
	
Returns the largest integer number less or equal the argument (rounding down).

#### ceil

	var y = ceil(x)
	
Returns the smallest integer number greater or equal the argument (rounding up).

### String and array builtins

#### cat

	var y = cat(x1,x2,x3,...)
	
Returns the concatenated string or array of the arguments list.

#### chr

	var y = chr(x1,x2,x3,...)

Returns a string built from the petcii code arguments.

#### shift
		
	var y = shift(a)
	
Removes and returns the first element of an array.

#### array

	var a = array(n)
	
Allocate an array with n elements.
	
#### push

	push(a,x1,x2,x3,..)
	
Append the arguments to the array.

#### pop

	var y = pop(a)
	
Removes and returns the last element of an array.

#### asc

	var y = asc(s)
	
Returns the petscii code of the first character of a string

#### val

	var y = val(s)
	
Converts a string into a number.

#### str

	var s = str(x)
	
Converts a number into a string.

#### find
	
	var n = find(s,t,[i])
	
Find the first (or next) occurence of a substring t in a string s.  Returns -1 if not found.
	
### Input/Output builtins

#### chrout

	chrout(x1,x2,x3,...)
	
Output the characters specified by the numeric petscii codes of the arguments.

#### chrin

	var y = chrin()
	
Read one character from the input.

#### print

	print(s1,s2,s3,...)
	
Print a sequence of values.

#### input

	var s = input()
	
Read a line from the input into a string.

### File builtins

#### fopen

	var f = fopen(d,i,s)
	
Open a file with name s on device d and return file handle.

#### fclose

	fclose(f)
	
Close a file handle.

#### fget

	var s = fget(f)
	var s = fget(f,n)
	var s = fget(t,g)

Read a string from file f upto a maximum of n characters or a guard character in the string g.
	
#### fput

	fput(f,s)
	
Write a string to a file.

#### feof

	var b = feof(f)
	
Returns true if at end of file f.

### Screen builtins

#### cput

	cput(x,y,a,c)
	
Put character a at location x, y with color c on screen.

#### cget

	var a = cget(x,y)
	
Get the character at location x,y from screen.

#### cfill

	cfill(x,y,w,h,a,c)
	
Fill a rectangle of characters a at location x, y with color c on screen.

#### cmove

	cfill(dx,dy,w,h,sx,sy)
	
Move a rectangle of characters from location sx, sy to location dx, dy on screen.

### System builtins

#### poke

	poke(a1,v1,a2,s2,...)
	
Write a sequence of values v1, s2,... into memory address a1, a2, ...  All characters of a string will be written into consecutive memory locations.

#### peek

	var y = peek(a)
	
Read a value from a memory location.

#### time

	var t = time()
	
Get the number of seconds since system startup.

#### vsync

	vsync()
	
Wait for the raster beam to reach the bottom of the visible screen area.


## Tokenization

The source code is tokenized on a by line method.  The first byte of a line is the depth of indentation plus one, or zero for the end of the program.  The second byte is the statement.  Some statements are followed by two additional bytes that point to the end of the block scope controlled by this statement (if, else, elsif, while, def) to allow for fast skip.

Most statements are followed by an expression.  The expressions are tokenized using a stack based operator order (similar to FORTH) to allow sequential interpretation without the need to check for operator precedence while executing.

Numbers are parsed into a binary form of one, two or five bytes depending on their size and complexity.  Variables are stored as references to their name in a global symbol store during editing and as indices into global and local variable arrays during execution.



