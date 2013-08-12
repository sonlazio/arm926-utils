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
 * Implementation of the board's timer functionality.
 * All 4 available timers are supported.
 * 
 * More info about the board and the timer controller:
 * - Versatile Application Baseboard for ARM926EJ-S, HBI 0118 (DUI0225D):
 *   http://infocenter.arm.com/help/topic/com.arm.doc.dui0225d/DUI0225D_versatile_application_baseboard_arm926ej_s_ug.pdf
 * - ARM Dual-Timer Module (SP804) Technical Reference Manual (DDI0271):
 *   http://infocenter.arm.com/help/topic/com.arm.doc.ddi0271d/DDI0271.pdf
 * 
 * @author Jernej Kovacic
 */

#include <stdlib.h>

/* Number of timers: */
#define N_TIMERS        4

/*
 * Base addresses for all 4 timers,
 * (see page 4-21 of DUI0225D): 
 */
#define TIMER0_BASE     0x101E2000
#define TIMER1_BASE     0x101E2020
#define TIMER2_BASE     0x101E3000
#define TIMER3_BASE     0x101E3020


/*
 * Bit masks for the Control Registers (TimerXControl).
 * 
 * For description of each control register's bit, see page 3-2 of DDI0271:
 * 
 *  31:8 reserved
 *   7: enable bit (1: enabled, 0: disabled)
 *   6: timer mode (0: free running, 1: periodic)
 *   5: interrupt enable bit (0: disabled, 1: enabled)
 *   4: reserved
 *   3:2 prescale (00: 1, other combinations are not supported)
 *   1: counter length (0: 16 bit, 1: 32 bit)
 *   0: one shot enable bit (0: wrapping, 1: one shot)
 */

#define CTL_ENABLE      0x00000080
#define CTL_MODE        0x00000040
#define CTL_INTR        0x00000020
#define CTL_PRESCALE    0x0000000C
#define CTL_CTRLEN      0x00000002
#define CTL_ONESHOT     0x00000001


/*
 * 32-bit Registers of individual timer controllers,
 * relative to the controller's base address:
 * See page 3-2 of DDI0271:
 */
typedef struct _ARM926EJS_TIMER_REGS 
{
	unsigned long LOD;    /* Load Register, TimerXLoad */
	unsigned long VAL;    /* Current Value Register, TimerXValue */
	unsigned long CTL;    /* Control Register, TimerXControl */
	unsigned long CLI;    /* InterruptClear Register, TimerXIntClr */
	unsigned long RIS;    /* Raw Interrupt Status Register, TimerXRIS */
	unsigned long MIS;    /* Masked interrupt Status Register, TimerXMIS */
	unsigned long RLD;    /* Background Load Register, TimerXBGLoad */
} ARM926EJS_TIMER_REGS;

/*
 * Pointers to each timer register's base addresses:
 */
static volatile ARM926EJS_TIMER_REGS* const    timerReg[N_TIMERS] = 
                            { 
                               (ARM926EJS_TIMER_REGS*) (TIMER0_BASE),
                               (ARM926EJS_TIMER_REGS*) (TIMER1_BASE),
                               (ARM926EJS_TIMER_REGS*) (TIMER2_BASE),
                               (ARM926EJS_TIMER_REGS*) (TIMER3_BASE)
                             };


/**
 * Set up the specified timer controller.
 * The following parameters are set:
 * - periodic mode (when the counter reaches 0, it is wrapped to the value of the Load Register)
 * - 32-bit counter length
 * - prescale = 1
 * 
 * This function does not enable interrupt triggering and does not start the timer!
 * 
 * Nothing is done if 'nr' is invalid (4 or greater).
 * 
 * @param nr - number of the timer (between 0 and 3)
 */
void setupTimer(unsigned nr)
{
    volatile ARM926EJS_TIMER_REGS* pReg;
    
    /* sanity check: */
    if ( nr>N_TIMERS )
    {
        return;
    }
    
    pReg = timerReg[nr];
    
    /*
     * DDI0271 does not recommend modifyng reserved bits of the Control Register (see page 3-5).
     * For that reason, the register is set in two steps:
     * - the appropriate bit masks of 1-bits are bitwise or'ed to the CTL
     * - zero complements of the appropriate bit masks of 0-bits are bitwise and'ed to the CTL
     */
    
    
    /* 
     * The following bits will be set to 1:
     * - timer mode (periodic)
     * - counter length (32-bit) 
     */
    
    pReg->CTL |= ( CTL_MODE | CTL_CTRLEN );
    
    /*
     * The following bits are set to 0:
     * - enable bit (disabled, i.e. timer not running)
     * - interrupt bit (disabled)
     * - prescale bits (00 = 1)
     * - oneshot bit (wrapping mode)
     */
    
    pReg->CTL &= ( ~CTL_ENABLE & ~CTL_INTR & ~CTL_PRESCALE & ~CTL_ONESHOT );
    
    /* reserved bits remained unmodifed */
}


/**
 * Starts the specified timer.
 * 
 * Nothing is done if 'nr' is invalid (4 or greater).
 * 
 * @param nr - number of the timer (between 0 and 3)
 */
void startTimer(unsigned nr)
{
    volatile ARM926EJS_TIMER_REGS* pReg;
 
    /* sanity check: */
    if ( nr>N_TIMERS )
    {
        return;
    }
    
    pReg = timerReg[nr];

    /* Set bit 7 of the Control Register to 1, do not modify other bits */
    pReg->CTL |= CTL_ENABLE;
}


/**
 * Stops the specified timer.
 * 
 * Nothing is done if 'nr' is invalid (4 or greater).
 * 
 * @param nr - number of the timer (between 0 and 3)
 */
void stopTimer(unsigned nr)
{
    volatile ARM926EJS_TIMER_REGS* pReg;

    /* sanity check: */
    if ( nr>N_TIMERS )
    {
        return;
    }
    
    pReg = timerReg[nr];
    
    /* Set bit 7 of the Control Register to 0, do not modify other bits */
    pReg->CTL &= ~CTL_ENABLE;
}


/**
 * Checks whether the specified timer is enabled, i.e. running.
 * 
 * If it is enabled, a nonzero value, typically 1, is returned,
 * otherwise a zero value is returned.
 * 
 * If 'nr' is invalid, a zero is returned (as an invalid timer cannot be enabled).
 * 
 * @param nr - number of the timer (between 0 and 3)
 * 
 * @return a zero value if the timer is disabled, a nonzero if it is enabled
 */ 
int isTimerEnabled(unsigned nr)
{
    volatile ARM926EJS_TIMER_REGS* pReg;

    /* sanity check: */
    if ( nr>N_TIMERS )
    {
        return 0;
    }
    
    pReg = timerReg[nr];
    
    /* just check the enable bit of the timer's Control Register */
    return ( 0==(pReg->CTL & CTL_ENABLE) ? 0 : 1 );
}


/**
 * Enables the timer's interrupt triggering (when the counter reaches 0).
 * 
 * Nothing is done if 'nr' is invalid (4 or greater).
 * 
 * @param nr - number of the timer (between 0 and 3)
 */
void enableTimerInterrupt(unsigned nr)
{
    volatile ARM926EJS_TIMER_REGS* pReg;
 
    /* sanity check: */
    if ( nr>N_TIMERS )
    {
        return;
    }
    
    pReg = timerReg[nr];
    
    /* Set bit 5 of the Control Register to 1, do not modify other bits */
    pReg->CTL |= CTL_INTR;
}


/**
 * Disables the timer's interrupt triggering (when the counter reaches 0).
 * 
 * Nothing is done if 'nr' is invalid (4 or greater).
 * 
 * @param nr - number of the timer (between 0 and 3)
 */
void disableTimerInterrupt(unsigned nr)
{
    volatile ARM926EJS_TIMER_REGS* pReg;
 
    /* sanity check: */
    if ( nr>N_TIMERS )
    {
        return;
    }
    
    pReg = timerReg[nr];
    
    /* Set bit 5 of the Control Register to 0, do not modify other bits */
    pReg->CTL &= ~CTL_INTR;
}


/**
 * Clears the interrupt output from the specified timer's counter
 * 
 * Nothing is done if 'nr' is invalid (4 or greater).
 * 
 * @param nr - number of the timer (between 0 and 3)
 */
void clearTimerInterrupt(unsigned nr)
{
    volatile ARM926EJS_TIMER_REGS* pReg;

    /* sanity check: */
    if ( nr>N_TIMERS )
    {
        return;
    }
    
    pReg = timerReg[nr];
    
    /*
     * Writing anything ( e.g. 0xFFFFFFFF, i.e. all ones )into the 
     * Interrupt Clear Register (CLI) clears the timer's interrupt output.
     * See page 3-6 of DDI0271.
     */
    pReg->CLI = 0xFFFFFFFF;
}


/**
 * Sets the value of the specified timer's Load Register.
 * 
 * When the timer runs in periodic mode and its counter reaches 0,
 * the counter is reloaded to this value.
 * 
 * For more details, see page 3-4 of DDI0271.
 * 
 * Nothing is done if 'nr' is invalid (4 or greater).
 * 
 * @param nr - number of the timer (between 0 and 3)
 * @param value - value to be loaded int the Load Register
 */
void setTimerLoad(unsigned nr, unsigned long value)
{
    volatile ARM926EJS_TIMER_REGS* pReg;

    /* sanity check: */
    if ( nr>N_TIMERS )
    {
        return;
    }
    
    pReg = timerReg[nr];
    
    pReg->LOD = value;
}


/**
 * Returns the value of the specified timer's Value Register, 
 * i.e. the value of the counter at the moment of reading.
 * 
 * Zero is returned if 'nr' is invalid (4 or greater).
 * 
 * @param nr - number of the timer (between 0 and 3)
 * 
 * @return value of the timer's counter at the moment of reading
 */
unsigned long getTimerValue(unsigned nr)
{
    volatile ARM926EJS_TIMER_REGS* pReg;

    /* sanity check: */
    if ( nr>N_TIMERS )
    {
        return 0UL;
    }
    
    pReg = timerReg[nr];
    
    return pReg->VAL;
}


/**
 * Address of the specified timer's Value Register. It might be suitable
 * for applications that poll this register frequently and wish to avoid 
 * the overhead due to calling getTimerValue() each time.
 * 
 * NULL is returned if 'nr' is invalid (4 or greater).
 * 
 * @note Contents at this address are read only and should not be modified.
 * 
 * @param nr - number of the timer (between 0 and 3)
 * 
 * @return read-only address of the timer's counter (i.e. the Value Register)
 */ 
const unsigned long* getTimerValueAddr(unsigned nr)
{
    volatile ARM926EJS_TIMER_REGS* pReg;
    
    /* sanity check: */
    if ( nr>N_TIMERS )
    {
        return NULL;
    }
    
    pReg = timerReg[nr];
    
    return (const unsigned long*) &(pReg->VAL);
}


/**
 * A convenience function that returns the primary controller's IRQ number of
 * the specified timer when its counter reaches 0 (provided that
 * triggering of interrupts is enabled).
 * 
 * According to page 4-67 of DUI0225D, timers 0 and 1 trigger IRQ4,
 * timers 2 and 3 trigger IRQ5.
 * 
 * If 'nr' is invalid (4 or greater), -1 is returned.
 * 
 * @param nr - number of the timer (between 0 and 3)
 * 
 * @return IRQ number on the primary interrupt controller for the specified timer
 */
int getTimerIRQ(unsigned nr)
{
    int retVal;
    
    switch (nr)
    {
        case 0:
        case 1:
            retVal = 4;
            break;

        case 2:
        case 3:
            retVal = 5;
            break;

        default:
            retVal = -1;
    }
    
    return retVal;
} 
