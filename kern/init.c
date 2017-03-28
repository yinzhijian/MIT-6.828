/* See COPYRIGHT for copyright information. */

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/monitor.h>
#include <kern/console.h>
#include <kern/pmap.h>
#include <kern/kclock.h>

void 
print_cpu_info()
{
	uint32_t eaxp,ebxp,ecxp,edxp;
	cpuid(0,&eaxp,&ebxp,&ecxp,&edxp);
	cprintf("cpu is %d ,%c%c%c%c %c%c%c%c %c%c%c%c\n", eaxp,ebxp,ebxp>>8,ebxp>>16,ebxp>>24,ecxp,ecxp>>8,ecxp>>16,ecxp>>24,edxp,edxp>>8,edxp>>16,edxp>>24);
}
bool PSE_SUPPORT;
void
cpu_pse_check()
{
	uint32_t eaxp,ebxp,ecxp,edxp;
	cpuid(1,&eaxp,&ebxp,&ecxp,&edxp);
	if(edxp & 8){
		PSE_SUPPORT = true;
		cprintf("cpu support pse!\n");
	}
}
void
i386_init(void)
{
	extern char edata[], end[];

	// Before doing anything else, complete the ELF loading process.
	// Clear the uninitialized global data (BSS) section of our program.
	// This ensures that all static/global variables start out zero.
	memset(edata, 0, end - edata);

	// Initialize the console.
	// Can't call cprintf until after we do this!
	cons_init();

	cprintf("6828 decimal is %o octal!\n", 6828);
	print_cpu_info();
	cpu_pse_check();

	// Lab 2 memory management initialization functions
	mem_init();

	// Drop into the kernel monitor.
	while (1)
		monitor(NULL);
}


/*
 * Variable panicstr contains argument to first call to panic; used as flag
 * to indicate that the kernel has already called panic.
 */
const char *panicstr;

/*
 * Panic is called on unresolvable fatal errors.
 * It prints "panic: mesg", and then enters the kernel monitor.
 */
void
_panic(const char *file, int line, const char *fmt,...)
{
	va_list ap;

	if (panicstr)
		goto dead;
	panicstr = fmt;

	// Be extra sure that the machine is in as reasonable state
	asm volatile("cli; cld");

	va_start(ap, fmt);
	cprintf("kernel panic at %s:%d: ", file, line);
	vcprintf(fmt, ap);
	cprintf("\n");
	va_end(ap);

dead:
	/* break into the kernel monitor */
	while (1)
		monitor(NULL);
}

/* like panic, but don't */
void
_warn(const char *file, int line, const char *fmt,...)
{
	va_list ap;

	va_start(ap, fmt);
	cprintf("kernel warning at %s:%d: ", file, line);
	vcprintf(fmt, ap);
	cprintf("\n");
	va_end(ap);
}
