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

#include "uart.h"
#include "timer.h"

/* A convenience buffer for strings */
#define BUFLEN       25
char strbuf[BUFLEN];


/*
 * Converts an unsigned long value into a string with its HEX representation.
 * 
 * @param buf - pointer to a string buffer (at least 11 characters)
 * 
 * @param val - unsigned long (4 bytes) value to be converted
 */
void ul2hex(char* buf, uint32_t val)
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
void ul2dec(char* buf, uint32_t val)
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
void unsignedLongSize(void)
{
    strbuf[0] = '0' + sizeof(uint8_t);
    strbuf[1] = '\0';
    print(strbuf);
    print("\r\n");
}
#endif


/*
 * Checks control registers of all timers and display
 * whether they are enabled
 */
void timersEnabledTest(void)
{
    uint8_t i;
    char* pstr;
    
    print("\r\n=Timer enabled test:=\r\n\r\n");
    /* Initialize all 4 timers: */
    for ( i=0; i<4; ++i )
    {
        initTimer(i);
    }
    
    /* Start the 2nd timer (it is running only, no interrupt is triggered): */
    setTimerLoad(1, 5000UL);
    startTimer(1);
    
     /* For each available timer... */
    for ( i=0; i<4; ++i )
    {

        /* Print the timer's number */
        print("Timer ");
        strbuf[0] = '0' + i;
        strbuf[1] = ':';
        strbuf[2] = ' ';
        strbuf[3] = '\0';
        print(strbuf);

        /* then call the appropriate timer function */
        pstr = (0!=isTimerEnabled(i) ? "enabled" : "disabled");
        print(pstr);
        print("\r\n");
    }
    
    /* The test is completed, stop the 2nd timer */
    stopTimer(1);
    
    print("\r\n=Timer enabled test completed=\r\n");
}


/*
 * Polls the selected timer's Value Register until a zero is detected.
 * Then prints a message about the "tick detected".
 * The sequence is repeated 10 times.
 * By default, the timer is loaded to 1,000,000.
 * At timer's frequency of 1 MHz, this means that 1 "tick" lasts 1 second.
 */
void timerPollingTest(void)
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
    
    print("\r\n=Timer polling test:=\r\n\r\n");
    
    initTimer(nr);
    
    /* 
     * To reduce overhead, obtain direct address of the timer's Value Register.
     * Note that the register should not be modified!
     */
    pVal = getTimerValueAddr(nr);
    /* sanity check */
    if ( NULL==pVal )
    {
        return;
    }
    
    setTimerLoad(nr, million);
    startTimer(nr);
    
    /* repeat the sequence for the specified number of ticks... */
    while (tick--)
    {
        /* poll the Value Register until it reaches 0 */
        while (*pVal);
        /* and output a "tick" */
	printChar('0' - 1 + nrTicks - tick);
        print(": polling tick detected\r\n");
    }

    /* When the test is complete, the timer can be disabled/stopped */
    stopTimer(nr);
    
    print("\r\n=Timer polling test completed=\r\n");
}


/*
 * Start point of the application.
 * 
 * Currently it only executes a few simple test tasks that output
 * something to the serial port.
 */ 
void start(void)
{
    print("* * * T E S T   S T A R T * * *\r\n");
    
    timersEnabledTest();
    timerPollingTest();
    
    print("\r\n* * * T E S T   C O M P L E T E D * * *\r\n");
} 
