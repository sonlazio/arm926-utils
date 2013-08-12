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

#include <stdlib.h>

/* Base address of the UART0 register (see page 4-68 of the DUI0225D): */
#define UART0_BASE      0x101F1000

static volatile char* const uart0 = (char*) (UART0_BASE);

/*
 * Outputs a character to the UART0. This short function is used by other functions,
 * that is why it is implemented as an inline function.
 * 
 * @param ch - character to be sent to the UART
 */
inline void __printCh(char ch)
{
   /*
    * TODO
    * Qemu ignores other UART's registers and to send a character, it is sufficient
    * to simply write its value to the UART's base address.
    * 
    * The future implementations should also handle other registers, even
    * qemu ignores them.
    */
    *uart0 = ch;
}

/**
 * Outputs a character to UART0
 * 
 * @param ch - character to be sent to the UART
 */ 
void printChar(char ch)
{
    /* just use the provided inline function: */
    __printCh(ch);
}

void print(const char* str)
{
    /* 
      if NULL is passed, avoid possible problems with deferencing of NULL 
      and print this string: 
     */
    char* null_str = "<NULL>\r\n";
    char* cp;
    
    /* handle possible NULL value of str: */
    if ( NULL == str )
    {
        cp = null_str;
    }
    else
    {
        /* Otherwise the actual passed string will be printed:*/
        cp = (char*) str;
    }
    
    /*
     * Just print each character until a zero terminator is detected
     */
    for ( ; '\0' != *cp; cp++ )
    {
        __printCh(*cp);
    }
}
