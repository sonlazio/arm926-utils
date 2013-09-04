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
 * Implementation of the board's real time clock (RTC) functionality.
 * 
 * More info about the board and the timer controller:
 * - Versatile Application Baseboard for ARM926EJ-S, HBI 0118 (DUI0225D):
 *   http://infocenter.arm.com/help/topic/com.arm.doc.dui0225d/DUI0225D_versatile_application_baseboard_arm926ej_s_ug.pdf
 * - ARM PrimeCell Real Time Clock (PL031) Technical Reference Manual (DDI0224):
 *   http://infocenter.arm.com/help/topic/com.arm.doc.ddi0224b/DDI0224.pdf
 * 
 * @author Jernej Kovacic
 */

#include <stdint.h>

#include "bsp.h"


/* Bit mask of the RTCCR that starts the RTC: */
#define CTL_START          0x00000001

/* Bit mask of the RTCIMSC register that (un)masks interrupt triggering: */
#define INT_SC             0x00000001

/* Bit mask of the RTCICR register that clears interrupts: */
#define INTCLR             0x00000001

/*
 * 32-bit registers of the RTC controller,
 * relative to the controller's base address:
 * See page 3-3 of DDI0224 for more details:
 */
typedef struct _ARM926EJS_RTC_REGS 
{
    const uint32_t RTCDR;                /* Data register, read only */
    uint32_t RTCMR;                      /* Match register */
    uint32_t RTCLR;                      /* Load register */
    uint32_t RTCCR;                      /* Control register */
    uint32_t RTCIMSC;                    /* Interrupt mask set and clear register */
    const uint32_t RTCRIS;               /* Raw interrupt status register, read only */
    const uint32_t RTCMIS;               /* Masked interrupt status register, read only */
    uint32_t RTCICR;                     /* Interupt clear register, write only */
    const uint32_t Reserved1[24];        /* Reserved, should not be modified */
    const uint32_t ReservedTest[5];      /* Reserved for test purposes, should not be modified */
    const uint32_t Reserved2[975];       /* Reserved, should not be modified */
    const uint32_t ReservedIdExp[4];     /* Reserved for future ID expansion, should not be modified */
    const uint32_t RTCPERIPHID[4];       /* Peripheral ID register, read only */
    const uint32_t RTCCELLID[4];         /* PrimeCell ID register */
} ARM926EJS_RTC_REGS;

/*
 * Pointer to the RTC base address:
 */
static volatile ARM926EJS_RTC_REGS* const pReg = (ARM926EJS_RTC_REGS*) (BSP_RTC_BASE_ADDRESS);


/**
 * Initializes the real time clock controller.
 * 
 * @note This function does not enable interrupt triggering and does not start the real time clock!
 */
void rtc_init(void)
{
    /* interrupt triggering is disabled */
    pReg->RTCIMSC &= ~INT_SC;
}


/**
 * Starts the real time clock.
 *
 * @note The RTC cannot be stopped once it is started.
 */
void rtc_start(void)
{
    pReg->RTCCR |= CTL_START;
}


/**
 * Checks whether the real time clock is running.
 * 
 * @return  0 if RTC is not running, a nonzero value (typically 1) if it is running
 */ 
int8_t rtc_isRunning(void)
{
    return (int8_t) (pReg->RTCCR & CTL_START);
}


/**
 * Enables the real time clock's interrupt triggering.
 */
void rtc_enableInterrupt(void)
{
    pReg->RTCIMSC |= INT_SC;
}


/**
 * Disables the real time clock's interrupt triggering.
 */
void rtc_disableInterrupt(void)
{
    pReg->RTCIMSC &= ~INT_SC;
}


/**
 * Clears the real time clock's interrupt
 */
void rtc_clearInterrupt(void)
{    
    /*
     * To clear an interupt, only the LSB of the RTCICR should be set to 1 
     * (see page 3-7 of the DDI0224).
     * According to this, '|=' would be a more appropriate operation.
     * However, the RTCICR register is write only, so only the wholeword can be
     * written to the register.
     */
    pReg->RTCICR = INTCLR;
}


/**
 * Sets the value of the real time clock's Load Register.
 *
 * @param value - value to be loaded int the Load Register
 */
void rtc_setLoad(uint32_t value)
{    
    pReg->RTCLR = value;
}


/**
 * Loads the real time clock's Match Register.
 *
 * @param value - value to be loaded into the Match Register
 */
void rtc_setMatch(uint32_t value)
{
    pReg->RTCMR = value;
}


/**
 * @return value of the real time clock's Match Register
 */
uint32_t rtc_getMatch(void)
{
    return pReg->RTCMR;
}


/**
 * @return value of the real time clock
 */
uint32_t rtc_getValue(void)
{    
    return pReg->RTCDR;
}


/**
 * Address of the real time clock's Data Register. It might be suitable
 * for applications that poll this register frequently and wish to avoid 
 * the overhead due to calling rtc_getValue() each time.
 * 
 * @note Contents at this address are read only and should not be modified.
 * 
 * @return read-only address of the real time clock's counter (i.e. the Data Register)
 */ 
const volatile uint32_t* rtc_getValueAddr(void)
{    
    return (const volatile uint32_t*) &(pReg->RTCDR);
}
