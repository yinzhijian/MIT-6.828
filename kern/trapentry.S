/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>



###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(num)						\
	.globl vector##num;		/* define global symbol for 'name' */	\
	.type vector##num, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	vector##num:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(num)					\
	.globl vector##num;							\
	.type vector##num, @function;						\
	.align 2;							\
	vector##num:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */
TRAPHANDLER_NOEC(0)
TRAPHANDLER_NOEC(1)
TRAPHANDLER_NOEC(2)
TRAPHANDLER_NOEC(3)
TRAPHANDLER_NOEC(4)
TRAPHANDLER_NOEC(5)
TRAPHANDLER_NOEC(6)
TRAPHANDLER_NOEC(7)
TRAPHANDLER(8)
TRAPHANDLER_NOEC(9)
TRAPHANDLER(10)
TRAPHANDLER(11)
TRAPHANDLER(12)
TRAPHANDLER(13)
TRAPHANDLER(14)
TRAPHANDLER_NOEC(15)
TRAPHANDLER_NOEC(16)
TRAPHANDLER_NOEC(17)
TRAPHANDLER_NOEC(18)
TRAPHANDLER_NOEC(19)
TRAPHANDLER_NOEC(20)
TRAPHANDLER_NOEC(21)
TRAPHANDLER_NOEC(22)
TRAPHANDLER_NOEC(23)
TRAPHANDLER_NOEC(24)
TRAPHANDLER_NOEC(25)
TRAPHANDLER_NOEC(26)
TRAPHANDLER_NOEC(27)
TRAPHANDLER_NOEC(28)
TRAPHANDLER_NOEC(29)
TRAPHANDLER_NOEC(30)
TRAPHANDLER_NOEC(31)
TRAPHANDLER_NOEC(32)
TRAPHANDLER_NOEC(33)
TRAPHANDLER_NOEC(34)
TRAPHANDLER_NOEC(35)
TRAPHANDLER_NOEC(36)
TRAPHANDLER_NOEC(37)
TRAPHANDLER_NOEC(38)
TRAPHANDLER_NOEC(39)
TRAPHANDLER_NOEC(40)
TRAPHANDLER_NOEC(41)
TRAPHANDLER_NOEC(42)
TRAPHANDLER_NOEC(43)
TRAPHANDLER_NOEC(44)
TRAPHANDLER_NOEC(45)
TRAPHANDLER_NOEC(46)
TRAPHANDLER_NOEC(47)
TRAPHANDLER_NOEC(48)
.data
.globl vectors
vectors:
  .long vector0
  .long vector1
  .long vector2
  .long vector3
  .long vector4
  .long vector5
  .long vector6
  .long vector7
  .long vector8
  .long vector9
  .long vector10
  .long vector11
  .long vector12
  .long vector13
  .long vector14
  .long vector15
  .long vector16
  .long vector17
  .long vector18
  .long vector19
  .long vector20
  .long vector21
  .long vector22
  .long vector23
  .long vector24
  .long vector25
  .long vector26
  .long vector27
  .long vector28
  .long vector29
  .long vector30
  .long vector31
  .long vector32
  .long vector33
  .long vector34
  .long vector35
  .long vector36
  .long vector37
  .long vector38
  .long vector39
  .long vector40
  .long vector41
  .long vector42
  .long vector43
  .long vector44
  .long vector45
  .long vector46
  .long vector47
  .long vector48


/*
 * Lab 3: Your code here for _alltraps
 */
_alltraps:
	pushl %ds
	pushl %es
	pushal
	movw $(GD_KD),%ax
	movw %ax,%ds
	movw %ax,%es
	pushl %esp
	call trap
	addl $4,%esp

