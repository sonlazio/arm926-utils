/**
 * @file
 *
 * Definition of exception vectors, implementation of reset (and startup handler),
 * Preparation of the IRQ mode.
 *
 * For more details, see:
 * ARM9EJ-S Technical Reference Manual (DDI0222):
 * http://infocenter.arm.com/help/topic/com.arm.doc.ddi0222b/DDI0222.pdf
 *
 * These articles are also useful:
 * http://balau82.wordpress.com/2012/04/15/arm926-interrupts-in-qemu/
 * http://www.embedded.com/design/mcus-processors-and-socs/4026075/Building-Bare-Metal-ARM-Systems-with-GNU-Part-2
 */

.text
 .code 32                      @ 32-bit ARM instruction set
 
 @ Both directives are visible to the linker  
 .global vectors_start
 .global vectors_end
 
vectors_start:
 @ Exception vectors, relative to the base address, see page 2-26 of DDI0222 
 LDR PC, reset_handler_addr             @ Reset (and startup) vector
 LDR PC, undef_handler_addr             @ Undefined (unknown) instruction
 LDR PC, swi_handler_addr               @ Software interrupt 
 LDR PC, prefetch_abort_handler_addr    @ Prefetch abort
 LDR PC, data_abort_handler_addr        @ Data abort (system bus cannot access a peripheral)
 B .                                    @ Reserved
 LDR PC, irq_handler_addr               @ IRQ handler
 LDR PC, fiq_handler_addr               @ FIQ handler

@ Definitions of exception handler functions, the above declared 
@ handlers' addresses point  to:
reset_handler_addr: .word reset_handler
undef_handler_addr: .word undef_handler
swi_handler_addr: .word swi_handler
prefetch_abort_handler_addr: .word prefetch_abort_handler
data_abort_handler_addr: .word data_abort_handler
irq_handler_addr: .word irq_handler
fiq_handler_addr: .word fiq_handler
 
vectors_end:

/*
 * Implementation of reset handler, execruted also at startup.
 * A separate stack pointer for the IRQ mode is defined,
 * the original mode (Supervisor) is restored and the IRQ mode is enabled.
 * Finally the startup function is executed.
 */
reset_handler:
 LDR sp, =stack_top                     @ stack for the supervisor mode
 BL copy_vectors                        @ vector table is copied to the point where the code is actually loaded
 MRS r0, cpsr                           @ copy Program Status Register (CPSR) to r0
 
 BIC r1, r0, #0x1F                      @ clear least significant 5 bits...
 ORR r1, r1, #0x12                      @ and set them to b10010 (0x12), i.e set IRQ mode
 MSR cpsr, r1                           @ update CPSR (program status register) for IRQ mode
 
 LDR sp, =irq_stack_top                 @ stack for the IRQ mode
 /* Enable IRQs */
 BIC r0, r0, #0x80                      @ clear the 8th bit (enables IRQ mode) of r0
 
 MSR cpsr, r0                           @ restore the CSPR (to Supervisor mode) with IRQ mode enabled
 
 BL start                               @ Starting point (start() instead of main()!)
 B .                                    @ infinite loop (if start() terminates)
 
.end

@ Other handlers and auxiliary functions are impleneted in exception.c.
