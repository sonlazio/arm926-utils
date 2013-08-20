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
 * Implementation of the board's UART functionality.
 * Currently only UART0 is supported.
 * 
 * More info about the board and the UART controller:
 * - Versatile Application Baseboard for ARM926EJ-S, HBI 0118 (DUI0225D):
 *   http://infocenter.arm.com/help/topic/com.arm.doc.dui0225d/DUI0225D_versatile_application_baseboard_arm926ej_s_ug.pdf
 * - PrimeCell UART (PL011) Technical Reference Manual (DDI0183):
 *   http://infocenter.arm.com/help/topic/com.arm.doc.ddi0183f/DDI0183.pdf
 * 
 * @author Jernej Kovacic
 */

#include <stdint.h>
#include <stddef.h>

/* Base addresses of UART registers (see page 4-68 of the DUI0225D): */
#define UART0_BASE      0x101F1000
#define UART1_BASE      0x101F2000
#define UART2_BASE      0x101F3000 


/*
 * 32-bit Registers of individual UART controllers,
 * relative to the controller's base address:
 * See page 3-3 of DDI0183.
 * 
 * Note: all registers are 32-bit words, however all registers only use the least
 * significant 16 bits (or even less). DDI0183 does not explicitly mention, how
 * the remaining bits should be handled, therefore they will be treated as
 * "should not be modified".
 */
typedef struct _ARM926EJS_UART_REGS
{
    uint32_t UARTDR;               /* UART Data Register, UARTDR */
    uint32_t UARTRSR;              /* Receive Status Register, Error Clear Register, UARTRSR/UARTECR */
    const uint32_t Reserved1[4];   /* reserved, should not be modified */
    const uint32_t UARTFR;         /* Flag Register, UARTFR, read only */
    const uint32_t Reserved2;      /* reserved, should not be modified */
    uint32_t UARTILPR;             /* IrDA Low-Power Counter Register, UARTILPR */
    uint32_t UARTIBRD;             /* Integer Baud Rate Register, UARTIBRD */
    uint32_t UARTFBRD;             /* Fractional Baud Rate Register, UARTFBRD */
    uint32_t UARTLC_H;             /* Line Control Register, UARTLC_H */
    uint32_t UARTCR;               /* Control Register, UARTCR */
    uint32_t UARTIFLS;             /* Interrupt FIFO Level Select Register, UARTIFLS */
    uint32_t UARTIMSC;             /* Interrupt Mask Set/Clear Register, UARTIMSC */
    const uint32_t UARTRIS;        /* Raw Interrupt Status Register, UARTRIS, read only */
    const uint32_t UARTMIS;        /* Mask Interrupt Status Register, UARTMIS, read only */
    uint32_t UARTICR;              /* Interrupt Clear Register */
    uint32_t UARTDMACR;            /* DMA Control Register, UARTDMACR */
} ARM926EJS_UART_REGS;

/* Shared UART register: */
#define UARTECR       UARTRSR

/* Bitmask for the Flag Register's TXFF bit (transmit FIFO full) */
#define FR_TXFF       0x00000020

static volatile ARM926EJS_UART_REGS* const pReg = (ARM926EJS_UART_REGS*) (UART0_BASE);

/*
 * Outputs a character to the UART0. This short function is used by other functions,
 * that is why it is implemented as an inline function.
 * 
 * @param ch - character to be sent to the UART
 */
static inline void __printCh(char ch)
{
   /*
    * Qemu ignores other UART's registers, anyway the Flag Register is checked 
    * to better emulate a "real" UART controller.
    * See description of the register on page 3-8 of DDI0183 for more details.
    */
   
   /* 
    * Poll the Flag Register's TXFF bit until the Transmit FIFO is not full.
    * When the TXFF bit is set to 1, the controller's internal Transmit FIFO is full.
    * In this case, wait until a some "waiting" characters have been transmitted and
    * the TXFF is set to 0, indicating the Transmit FIFO can accept additional characters.
    */
   while ( pReg->UARTFR & FR_TXFF );
   
   /*
    * The Data Register is a 32-bit word, however only the least significant 8 bits
    * can be assigned the character to be sent, while other bits represent various flags
    * and should not be set to 0. For that reason, the following trick is introduced:
    * 
    * Casting the Data Register's address to char* effectively turns the word into an array
    * of (four) 8-bit characters. Now, dereferencing the first character of this array affects
    * only the desired character itself, not the whole word.  
    */
   
    *( (char*) &(pReg->UARTDR) ) = ch;
}

/**
 * Outputs a character to UART0
 * 
 * @param ch - character to be sent to the UART
 */ 
void uart_printChar(char ch)
{
    /* just use the provided inline function: */
    __printCh(ch);
}

void uart_print(const char* str)
{
    /* 
      if NULL is passed, avoid possible problems with deferencing of NULL 
      and print this string: 
     */
    char* null_str = "<NULL>\r\n";
    char* cp;
    
    /* handle possible NULL value of str: */
    cp = ( NULL==str ? null_str : (char*) str );
    
    /*
     * Just print each character until a zero terminator is detected
     */
    for ( ; '\0' != *cp; cp++ )
    {
        __printCh(*cp);
    }
}
