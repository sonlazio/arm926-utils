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


/*
 * @file
 * 
 * Collection of some convenience functions and peripherals' drivers'
 * test tasks.
 */


#include <stddef.h>
#include <stdint.h>

#include "interrupt.h"
#include "uart.h"
#include "timer.h"

/* A convenience buffer for strings */
#define BUFLEN       25
static char strbuf[BUFLEN];


/*
 * Converts an unsigned long value into a string with its HEX representation.
 * 
 * @param buf - pointer to a string buffer (at least 11 characters)
 * 
 * @param val - unsigned long (4 bytes) value to be converted
 */
static void ul2hex(char* buf, uint32_t val)
{
    uint8_t i;
    uint8_t digit;
    
    /* Start the string with '0x' */
    buf[0] = '0';
    buf[1] = 'x';
    
    /* the output will not be longer than 8 hex "digits" */
    for ( i=0; i<8; i++ )
    {
        /* The last 4 bytes of val, i.e. val % 16 */
        digit = (uint8_t) (val & 0x0F);

        /* convert it to a hex digit char: 0..9A..F */
        if ( digit<10 )
        {
            buf[9-i] = '0' + digit;
        }
        else
        {
            buf[9-i] = 'A' + digit - 10;
        }
        
        /* Shift 'val' 4 bits to the right, i.e integer divide it by 16 */
        val >>= 4;
    }
    
    /* Do not forget the string's zero terminator! */
    buf[10] = '\0';
}


/*
 * Converts an unsigned long value into a string with its decimal representation.
 * 
 * The output will be prepended by spaces, so its entire length will be 20 characters.
 * 
 * @param buf - pointer to a string buffer (at least 20 characters)
 * 
 * @param val - unsigned long (4 bytes) value to be converted
 */
static void ul2dec(char* buf, uint32_t val)
{
    int8_t i;
    uint8_t digit;
    
    /* fill the buffer with spaces*/
    for ( i=0; i<19; i++ )
    {
        buf[i] = ' ';
    }
    /* and set the zero terminator: */
    buf[19] = '\0';
    
    /* No more than 19 digits will be appended to buffer */
    for ( i=18; i>0; i-- )
    {
        /* Ekstract the least significant digit of val */
        digit = val % 10;
        /* write it into the buffer */
        buf[i] = '0' + digit;
        
        /* integer division of val by 10, i.e. shift it one digit to the right */
        val /= 10;
        
        /* No need to continue the loop if no digits remain */
        if ( 0 == val )
        {
            break;
        }
    }
    
    /*
     * TODO: move digits to start of the buffer??
     */
}


#if 0
/*
 * At the very early stage I was curious about integer sizes.
 * Anyway, this might still be useful, hence I have preserved the function.
 */
static void unsignedLongSize(void)
{
    strbuf[0] = '0' + sizeof(unsigned long);
    strbuf[1] = '\0';
    uart_print(strbuf);
    uart_print("\r\n");
}
#endif


/*
 * Checks control registers of all timers and display
 * whether they are enabled
 */
static void timersEnabledTest(void)
{
    uint8_t i;
    char* pstr;
    
    uart_print("\r\n=Timer enabled test:=\r\n\r\n");
    /* Initialize all 4 timers: */
    for ( i=0; i<4; ++i )
    {
        timer_init(i);
    }
    
    /* Start the 2nd timer (it is running only, no interrupt is triggered): */
    timer_setLoad(1, 5000UL);
    timer_start(1);
    
     /* For each available timer... */
    for ( i=0; i<4; ++i )
    {

        /* Print the timer's number */
        uart_print("Timer ");
        strbuf[0] = '0' + i;
        strbuf[1] = ':';
        strbuf[2] = ' ';
        strbuf[3] = '\0';
        uart_print(strbuf);

        /* then call the appropriate timer function */
        pstr = (0!=timer_isEnabled(i) ? "enabled" : "disabled");
        uart_print(pstr);
        uart_print("\r\n");
    }
    
    /* The test is completed, stop the 2nd timer */
    timer_stop(1);
    
    uart_print("\r\n=Timer enabled test completed=\r\n");
}


/*
 * Polls the selected timer's Value Register until a zero is detected.
 * Then prints a message about the "tick detected".
 * The sequence is repeated 10 times.
 * By default, the timer is loaded to 1,000,000.
 * At timer's frequency of 1 MHz, this means that 1 "tick" lasts 1 second.
 */
static void timerPollingTest(void)
{
    const uint8_t nr = 2;
    /*
     * Hex representation of 1,000,000. Since the timers' counter frequency is
     * (supposed to be) 1 MHz, it is a convenient factor to specify the tick length
     * in seconds.
     */
    const uint32_t million = 0x000F4240;
    const uint8_t nrTicks = 10;
    uint8_t tick = nrTicks;
    
    const volatile uint32_t* pVal;
    
    uart_print("\r\n=Timer polling test:=\r\n\r\n");
    
    timer_init(nr);
    
    /* 
     * To reduce overhead, obtain direct address of the timer's Value Register.
     * Note that the register should not be modified!
     */
    pVal = timer_getValueAddr(nr);
    /* sanity check */
    if ( NULL==pVal )
    {
        return;
    }
    
    timer_setLoad(nr, million);
    timer_start(nr);
    
    /* repeat the sequence for the specified number of ticks... */
    while (tick--)
    {
        /* poll the Value Register until it reaches 0 */
        while (*pVal);
        /* and output a "tick" */
        uart_printChar('0' - 1 + nrTicks - tick);
        uart_print(": polling tick detected\r\n");
    }

    /* When the test is complete, the timer can be disabled/stopped */
    timer_stop(nr);
    
    uart_print("\r\n=Timer polling test completed=\r\n");
}


/* 
 * Counter of ticks, used by IRQ servicing routines. It is used by
 *several functions simultaneously, so it should be volatile.  
 */
static volatile uint32_t __tick_cntr = 0;

/*
 * An ISR routine, invoked whenever the Timer 0 (or 1) triggers the IRQ 4
 */
static void timer0ISR(void)
{
    /*
     * When this ISR routine is invoked, a message is sent to the UART:
     */
    uart_printChar('0' + __tick_cntr);
    uart_print(": IRQ tick detected\r\n");
    
    /* Increase the number of ticks */
    __tick_cntr++;
    
    /* And finally, acknowledge the interrupt, i.e. clear it in the timer */
    timer_clearInterrupt(0);
}

/*
 * A test function for testing nonvectored IRQ handling from the Timer 0.
 * The timer is prepared, IRQ4 is enabled, when 10 ticks occur, everything
 * is cleaned up.
 */
static void timerNvIrqTest(void)
{
    uart_print("\r\n=Timer IRQ test:=\r\n\r\n");
    
    /* Initialize the PIC */
    pic_init();
    
    /* Assign the ISR routine for the IRQ4, triggered by timers 0 and 1 */
    pic_registerNonVectoredIrq(4, timer0ISR);
    
    /* Enable IRQ mode */
    irq_enableIrqMode();
    
    /* Enable IRQ4 */
    pic_enableInterrupt(4);
    
    /* Initialize the timer 0 to triggger IRQ4 every 1000000 micro seconds, i.e. every 1 s */
    timer_init(0);
    timer_setLoad(0, 1000000);
    timer_enableInterrupt(0);
    
    /* Reset the tick counter and start the timer */
    __tick_cntr = 0;
    timer_start(0);
    
    /* just wait until IRQ4 is triggered 10 times */
    while ( __tick_cntr<10 );
    
    /* Cleanup. Reset the counter, disable interrupts, stop the Timer... */
    __tick_cntr = 0;
    timer_disableInterrupt(0);
    timer_stop(0);
    pic_disableInterrupt(4);
    
    /* Disable IRQ mode */
    irq_disableIrqMode();
    
    uart_print("\r\n=Timer IRQ test completed=\r\n");
}

/*
 * Starting point of the application.
 * 
 * Currently it only executes a few simple test tasks that output
 * something to the serial port.
 */ 
void start(void)
{
    uart_print("* * * T E S T   S T A R T * * *\r\n");
    
    timersEnabledTest();
    timerPollingTest();
    timerNvIrqTest();
    
    uart_print("\r\n* * * T E S T   C O M P L E T E D * * *\r\n");
} 
