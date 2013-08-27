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

#include <stdint.h>
#include <stddef.h>

#include "base_address.h"


/* Number of timers: */
#define N_TIMERS        4


/*
 * Bit masks for the Control Register (TimerXControl).
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

#define CTL_ENABLE          0x00000080
#define CTL_MODE            0x00000040
#define CTL_INTR            0x00000020
#define CTL_PRESCALE_1      0x00000008
#define CTL_PRESCALE_2      0x00000004
#define CTL_CTRLEN          0x00000002
#define CTL_ONESHOT         0x00000001


/*
 * 32-bit registers of individual timer controllers,
 * relative to the controllers' base address:
 * See page 3-2 of DDI0271:
 */
typedef struct _ARM926EJS_TIMER_REGS 
{
	uint32_t LOAD;           /* Load Register, TimerXLoad */
	const uint32_t VALUE;    /* Current Value Register, TimerXValue, read only */
	uint32_t CONTROL;        /* Control Register, TimerXControl */
	uint32_t INTCLR;         /* Interrupt Clear Register, TimerXIntClr */
	uint32_t RIS;            /* Raw Interrupt Status Register, TimerXRIS, read only */
	uint32_t MIS;            /* Masked Interrupt Status Register, TimerXMIS, read only */
	uint32_t BGLOAD;         /* Background Load Register, TimerXBGLoad */
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
 * Initializes the specified timer controller.
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
void timer_init(uint8_t nr)
{

    /* sanity check: */
    if ( nr >= N_TIMERS )
    {
        return;
    }
    
    
    /*
     * DDI0271 does not recommend modifying reserved bits of the Control Register (see page 3-5).
     * For that reason, the register is set in two steps:
     * - the appropriate bit masks of 1-bits are bitwise or'ed to the CTL
     * - zero complements of the appropriate bit masks of 0-bits are bitwise and'ed to the CTL
     */
    
    
    /* 
     * The following bits will be set to 1:
     * - timer mode (periodic)
     * - counter length (32-bit) 
     */
    
    timerReg[nr]->CONTROL |= ( CTL_MODE | CTL_CTRLEN );
    
    /*
     * The following bits are will be to 0:
     * - enable bit (disabled, i.e. timer not running)
     * - interrupt bit (disabled)
     * - both prescale bits (00 = 1)
     * - oneshot bit (wrapping mode)
     */
    
    timerReg[nr]->CONTROL &= ( ~CTL_ENABLE & ~CTL_INTR & ~CTL_PRESCALE_1 & ~CTL_PRESCALE_2 & ~CTL_ONESHOT );
    
    /* reserved bits remained unmodifed */
}


/**
 * Starts the specified timer.
 * 
 * Nothing is done if 'nr' is invalid (4 or greater).
 * 
 * @param nr - number of the timer (between 0 and 3)
 */
void timer_start(uint8_t nr)
{

    /* sanity check: */
    if ( nr >= N_TIMERS )
    {
        return;
    }

    /* Set bit 7 of the Control Register to 1, do not modify other bits */
    timerReg[nr]->CONTROL |= CTL_ENABLE;
}


/**
 * Stops the specified timer.
 * 
 * Nothing is done if 'nr' is invalid (4 or greater).
 * 
 * @param nr - number of the timer (between 0 and 3)
 */
void timer_stop(uint8_t nr)
{

    /* sanity check: */
    if ( nr >= N_TIMERS )
    {
        return;
    }
    
    /* Set bit 7 of the Control Register to 0, do not modify other bits */
    timerReg[nr]->CONTROL &= ~CTL_ENABLE;
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
int8_t timer_isEnabled(uint8_t nr)
{

    /* sanity check: */
    if ( nr >= N_TIMERS )
    {
        return 0;
    }
    
    /* just check the enable bit of the timer's Control Register */
    return ( 0==(timerReg[nr]->CONTROL & CTL_ENABLE) ? 0 : 1 );
}


/**
 * Enables the timer's interrupt triggering (when the counter reaches 0).
 * 
 * Nothing is done if 'nr' is invalid (4 or greater).
 * 
 * @param nr - number of the timer (between 0 and 3)
 */
void timer_enableInterrupt(uint8_t nr)
{
 
    /* sanity check: */
    if ( nr >= N_TIMERS )
    {
        return;
    }
    
    /* Set bit 5 of the Control Register to 1, do not modify other bits */
    timerReg[nr]->CONTROL |= CTL_INTR;
}


/**
 * Disables the timer's interrupt triggering (when the counter reaches 0).
 * 
 * Nothing is done if 'nr' is invalid (4 or greater).
 * 
 * @param nr - number of the timer (between 0 and 3)
 */
void timer_disableInterrupt(uint8_t nr)
{

    /* sanity check: */
    if ( nr >= N_TIMERS )
    {
        return;
    }
    
    /* Set bit 5 of the Control Register to 0, do not modify other bits */
    timerReg[nr]->CONTROL &= ~CTL_INTR;
}


/**
 * Clears the interrupt output from the specified timer's counter
 * 
 * Nothing is done if 'nr' is invalid (4 or greater).
 * 
 * @param nr - number of the timer (between 0 and 3)
 */
void timer_clearInterrupt(uint8_t nr)
{

    /* sanity check: */
    if ( nr >= N_TIMERS )
    {
        return;
    }
    
    /*
     * Writing anything (e.g. 0xFFFFFFFF, i.e. all ones) into the 
     * Interrupt Clear Register clears the timer's interrupt output.
     * See page 3-6 of DDI0271.
     */
    timerReg[nr]->INTCLR = 0xFFFFFFFF;
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
void timer_setLoad(uint8_t nr, uint32_t value)
{

    /* sanity check: */
    if ( nr >= N_TIMERS )
    {
        return;
    }
    
    timerReg[nr]->LOAD = value;
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
uint32_t timer_getValue(uint8_t nr)
{

    /* sanity check: */
    if ( nr >= N_TIMERS )
    {
        return 0UL;
    }
    
    return timerReg[nr]->VALUE;
}


/**
 * Address of the specified timer's Value Register. It might be suitable
 * for applications that poll this register frequently and wish to avoid 
 * the overhead due to calling timer_getValue() each time.
 * 
 * NULL is returned if 'nr' is invalid (4 or greater).
 * 
 * @note Contents at this address are read only and should not be modified.
 * 
 * @param nr - number of the timer (between 0 and 3)
 * 
 * @return read-only address of the timer's counter (i.e. the Value Register)
 */ 
const volatile uint32_t* timer_getValueAddr(uint8_t nr)
{

    /* sanity check: */
    if ( nr >= N_TIMERS )
    {
        return NULL;
    }
    
    return (const volatile uint32_t*) &(timerReg[nr]->VALUE);
}
