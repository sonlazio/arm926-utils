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

#include "peripheral_irqs.h"
#include "interrupt.h"
#include "uart.h"
#include "timer.h"
#include "rtc.h"

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
    for ( i=0; i<8; ++i )
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
    for ( i=0; i<19; ++i )
    {
        buf[i] = ' ';
    }
    /* and set the zero terminator: */
    buf[19] = '\0';
    
    /* No more than 19 digits will be appended to buffer */
    for ( i=18; i>0; --i )
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


extern void _pic_set_irq_vector_mode(int8_t mode);


/*
 * A test function for testing vectored IRQ handling from the Timer 0.
 * The timer is prepared, IRQ4 is enabled, when 10 ticks occur, everything
 * is cleaned up.
 */
static void timerVectIrqTest(void)
{

    uart_print("\r\n=Timer vectored IRQ test:=\r\n\r\n");
    
    /* Initialize the PIC */
    pic_init();
    
    _pic_set_irq_vector_mode(1);
    
    /* Assign the ISR routine for the IRQ 4, triggered by timers 0 and 1 */
    pic_registerVectorIrq(IRQ_TIMER0, &timer0ISR);
    
    /* Enable IRQ mode */
    irq_enableIrqMode();
    
    /* Enable IRQ4 */
    pic_enableInterrupt(IRQ_TIMER0);
    
    /* Initialize the timer 0 to triggger IRQ 4 every 1000000 micro seconds, i.e. every 1 s */
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
    pic_disableInterrupt(IRQ_TIMER0);
    
    /* Disable IRQ mode */
    irq_disableIrqMode();
    
    /* Set IRQ handling mode back to non-vectored: */
    _pic_set_irq_vector_mode(0);
    
    uart_print("\r\n=Timer vectored IRQ test completed=\r\n");
}


/*
 * An ISR routine, invoked when the RTC triggers the IRQ 10
 * 
 * @param param - a void* casted pointer to a uint32_t counter that will be incremented 
 */
static void rtcISR(void* param)
{
    uint32_t* pCntr = (uint32_t*) param;

    /* If 'param' is specified, increase the counter of "ticks" */
    if ( NULL != pCntr )
    {
        ++(*pCntr); 
    }
    
    /* And acknowledge the interrupt, i.e. clear it in the RTC */
    rtc_clearInterrupt();
}


/*
 * A test function for testing nonvectored IRQ handling from the RTC.
 * The RTC and a timer are prepared, IRQ10 is enabled, when a "tick" occurs
 * adter 7 seconds and is verified by the timer, everything is cleaned up.
 */
static void rtcTest(void)
{
    const uint32_t period = 7;   /* in seconds */
    const uint32_t initTimerVal = 100000000; /* 100,000,000 us = 100 sec. */
    
    uart_print("\r\n=RTC test:=\r\n\r\n");
    
    /* Init all necessary peripherals */
    rtc_init();
    timer_init(3);
    pic_init();
    
    uart_print("Expecting a RTC interrupt in 7 seconds...\r\n");
    
    /* Enable necessary controllers: */
    pic_registerNonVectoredIrq(IRQ_RTC, &rtcISR, (void*) &__tick_cntr);
    irq_enableIrqMode();    
    pic_enableInterrupt(IRQ_RTC);
    rtc_enableInterrupt();
    
    /* Start the RTC */
    rtc_start();
    /* Load the timer with an initial value */
    timer_setLoad(3, initTimerVal);
    /* Set an "alarm" at "now()" + 7 seconds:*/
    rtc_setMatch(rtc_getValue()+period);
    /* Start a timer for verification of the RTC: */
    timer_start(3);
    
    /* Reset the counter of "ticks" */
    __tick_cntr = 0;
    
    /* Wait for the ISR to finish (i.e. update of a counter) */
    while ( 0 == __tick_cntr );
    
    /* Stop the timer (and preserve its value) immediately after the interrupt */
    timer_stop(3);
    
    /* Clean up, disable controllers, etc. */
    rtc_disableInterrupt();
    pic_disableInterrupt(IRQ_RTC);
    irq_disableIrqMode();
    
    /* Finally verify that the RTC indeed triggered an IRQ after approx. 7 seconds */
    ul2dec(strbuf, initTimerVal - timer_getValue(3) );
    uart_print("RTC interrupt triggered after: ");
    uart_print(strbuf);
    uart_print(" micro seconds.\r\n");
    
    uart_print("\r\n=RTC test completed=\r\n");
}

/*
 * Starting point of the application.
 * 
 * Currently it only executes a few simple test tasks that output
 * something to the serial port.
 */ 
void start(void)
{
    uart_init();
    
    uart_print("* * * T E S T   S T A R T * * *\r\n");
    
    timersEnabledTest();
    timerPollingTest();
    
    /*
     * W A R N I N G :
     * Early versions of Qemu implement vectored IRQ handling improperly. In such a case,
     * this function MUST NOT BE RUN as wrong ISR routines may be run due ti this bug!
     *
     * This defect was fixed at Qemu 1.3 (released in September 2012). More details:
     * - https://lists.gnu.org/archive/html/qemu-devel/2012-08/msg03354.html
     * - https://github.com/qemu/qemu/commit/14c126baf1c38607c5bd988878de85a06cefd8cf
     */
    timerVectIrqTest();
    
    rtcTest();
    
    uart_print("\r\n* * * T E S T   C O M P L E T E D * * *\r\n");
    
    /* End in an infinite loop */
    for ( ; ; );
} 
