#pragma once

#include <c64/easyflash.h>

#define BANK_MAIN		0
#define	BANK_EDITOR		1
#define BANK_RUNTIME	2
#define BANK_TOKENS		3

// Common code bank for all elements, contains stuff named code
#pragma region( cbank, 0xb000, 0xbffc, , {0, 1, 2, 3, 4, 5, 6}, { code, data } )

// Heap stack etc
#pragma region(main, 0x7000, 0x8000, , , { bss, stack, heap })

#pragma section( mcode, 0 )
#pragma section( mdata, 0 )
#pragma region( mbank, 0x80a0, 0xb000, , BANK_MAIN, { mcode, mdata } )

#pragma section( ecode, 0 )
#pragma section( edata, 0 )
#pragma region( ebank, 0x8000, 0xb000, , BANK_EDITOR, { ecode, edata } )

#pragma section( rcode, 0 )
#pragma section( rdata, 0 )
#pragma region( rbank, 0x8000, 0xb000, , BANK_RUNTIME, { rcode, rdata } )

#pragma section( tcode, 0 )
#pragma section( tdata, 0 )
#pragma region( tbank, 0x8000, 0xb000, , BANK_TOKENS, { tcode, tdata } )


static char * const Screen = (char *)0xcc00;
static char * const Color = (char *)0xd800;

#pragma section( editbss, 0, , , bss)
#pragma region( editbss , 0xc000, 0xcc00, , , {editbss} )

#pragma section( rtbss, 0, , , bss)
#pragma region( rtbss , 0xc000, 0xd000, , , {rtbss} )

void system_show_editor(void);

void system_show_runtime(void);

void system_putch(char ch);

char system_getch(void);

char system_getchx(void);

char system_readch(void);

template<int back, class fn>
__noinline void system_vcall(void)
{
	eflash.bank = __bankof(fn);
	fn();
	eflash.bank = back;
}

template<int back, class fn>
__noinline auto system_rcall(void)
{
	eflash.bank = __bankof(fn);
	auto r = fn();
	eflash.bank = back;
	return r;
}

template<int back, class fn, class T>
__noinline auto system_rpcall(T t)
{
	eflash.bank = __bankof(fn);
	auto r = fn(t);
	eflash.bank = back;
	return r;
}

template<int back, class fn, class T>
__noinline void system_vpcall(T t)
{
	eflash.bank = __bankof(fn);
	fn(t);
	eflash.bank = back;
}

#define SYS_VCALL(fn) system_vcall<__bankof(""), fn>()

#define SYS_RCALL(fn) system_rcall<__bankof(""), fn>()

#define SYS_VPCALL(fn, T, t) system_vpcall<__bankof(""), fn, T>(t)

#define SYS_RPCALL(fn, T, t) system_rpcall<__bankof(""), fn, T>(t)


#pragma compile("system.cpp")
