/*
Copyright 2013, Jernej Kovacic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/


/**
 * @file
 *
 * A linker script that defines the application's memory map, starting point
 * and stack memory sizes.
 *
 * This script is tailored for startup in Qemu.
 *
 * Based on these articles:
 * http://balau82.wordpress.com/2012/04/15/arm926-interrupts-in-qemu/
 * http://www.embedded.com/design/mcus-processors-and-socs/4026080/Building-Bare-Metal-ARM-Systems-with-GNU-Part-3
 */

 
/* 
 * Memory region between 0x00000000 and 0x03FFFFFF is reserved for max. 128MB of RAM.
 * See page 4-3 of the DUI0225D.
 */
ENTRY(vectors_start)
SECTIONS
{
    . = 0x10000;    /* Qemu will boot from this address */
    .text :
    {
        vectors.o  /* Exception vectors, specified in vectors.o, must be placed to the startup address! */
        /* followed by the rest of the code... */
        *(.text)
    }

    /* followed by other sections... */
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : { *(.bss) }
    . = ALIGN(8);  /* The section size is aligned to the 8-byte boundary */
    
    /* Finally allocate enough memory for supported operating modes' stacks */
    . = . + 0x500; /* reserve approx. 1kB for the stack memory */
    stack_top = .;
    . = . + 0x500; /* reserve approx. 1kB for the IRQ handler's stack memory */
    irq_stack_top = .;
    . = . + 0x40; /* a small stack is essential for the Supervisor mode */
    svc_stack_top = .;
}
