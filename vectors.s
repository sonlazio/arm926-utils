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
.code 32                                   @ 32-bit ARM instruction set
 
@ Both labels are visible to the linker  
.global vectors_start
.global vectors_end
 
vectors_start:
    @ Exception vectors, relative to the base address, see page 2-26 of DDI0222 
    LDR pc, reset_handler_addr             @ Reset (and startup) vector
    LDR pc, undef_handler_addr             @ Undefined (unknown) instruction
    LDR pc, swi_handler_addr               @ Software interrupt 
    LDR pc, prefetch_abort_handler_addr    @ Prefetch abort
    LDR pc, data_abort_handler_addr        @ Data abort (system bus cannot access a peripheral)
    B .                                    @ Reserved
    LDR pc, irq_handler_addr               @ IRQ handler
    LDR pc, fiq_handler_addr               @ FIQ handler

@ Labels with addresses to exception handler routines, referenced above:
reset_handler_addr:
    .word reset_handler
undef_handler_addr:
    .word undef_handler
swi_handler_addr:
    .word swi_handler
prefetch_abort_handler_addr:
    .word prefetch_abort_handler
data_abort_handler_addr:
    .word data_abort_handler
irq_handler_addr:
    .word irq_handler
fiq_handler_addr:
    .word fiq_handler

vectors_end:

/*
 * Implementation of the reset handler, executed also at startup.
 * It sets stack pointers for all supported operating modes (Supervisor,
 * IRQ and User), Disables IRQ interrupts for all modes and finally it
 * switches into the User mode and jumps into the startup function.
 *
 * Note: 'stack_top', 'irq_stack_top' and 'svc_stack_top' are allocated in qemu.ld
 */
reset_handler:
    @ The handler is always entered in Supervisor mode
    LDR sp, =svc_stack_top                 @ stack for the supervisor mode
    BL copy_vectors                        @ copy exception vectors to 0x00000000
    MRS r0, cpsr                           @ copy Program Status Register (CPSR) to r0

    @ Disable IRQ interrupts for the Supervisor mode
    @ This should be disabled by default, but it doesn't hurt...
    ORR r1, r0, #80
    MSR cpsr, r1

    @ Set and switch into IRQ mode
    BIC r1, r0, #0x1F                      @ clear least significant 5 bits...
    ORR r1, r1, #0x12                      @ and set them to b10010 (0x12), i.e set IRQ mode
    ORR r1, r1, #80                        @ also disable IRQ triggering (a default setting, but...)
    MSR cpsr, r1                           @ update CPSR (program status register) for IRQ mode
 
    @ When in IRQ mode, set its stack pointer
    LDR sp, =irq_stack_top                 @ stack for the IRQ mode
 
    @ Prepare and enter into User mode. This mode is configured as the last as it does
    @ not permit (direkt) switching into other operating modes.
 
    BIC r1, r0, #0x1F                      @ clear loweest 5 bits
    ORR r1, r1, #0x10                      @ and set them to the User mode
 
    @ It is a good idea if IRQ interrupts are disabled by default until all ISR vectors
    @ are configured properly, and then enabled "manually".
    ORR r1, r1, #0x80                      @ set the 8th bit (disables IRQ mode) of r0
 
    MSR cpsr, r1                           @ update the CSPR (to User mode) with IRQ mode disabled
    LDR sp, =stack_top                     @ stack for the User Mode

    BL _init                               @ before the application is started, initialize all hardware

    BL main                                @ and finally start the application
    B .                                    @ infinite loop (if main() ever returns)
 
.end

@ Other handlers and auxiliary functions are implemented in exception.c.
