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
 * Implementation of the board's Primary Interrupt Controller (PIC) functionality.
 * 
 * Secondary Interrupt Controller (SIC) is currently not supported.
 *
 * More info about the board and the PIC controller:
 * - Versatile Application Baseboard for ARM926EJ-S, HBI 0118 (DUI0225D):
 *   http://infocenter.arm.com/help/topic/com.arm.doc.dui0225d/DUI0225D_versatile_application_baseboard_arm926ej_s_ug.pdf
 * - PrimeCell Vectored Interrupt Controller (PL190) Technical Reference Manual (DDI0181):
 *   http://infocenter.arm.com/help/topic/com.arm.doc.ddi0181e/DDI0181.pdf
 * 
 * @author Jernej Kovacic
 */

#include <stdint.h>
#include <stddef.h>

/* For public definitions of types: */
#include "interrupt.h"


/* Base addresses of the PIC controller (see page 4-44 of the DUI0225D): */
#define PIC_BASE      0x10140000

/* Base addresses of the SIC controller (see page 4-44 of the DUI0225D): */
#define SIC_BASE      0x10003000

/*
 * 32-bit Registers of the Primary Interrupt Controller,
 * relative to the controller's base address:
 * See page 3-3 of DDI0181.
 * 
 * Although not explicitly mentioned by DDI0181, there are gaps
 * among certain groups of register. The gaps are filled by
 * Unused* "registers" and are treated as "should not be modified".
 */
typedef struct _ARM926EJS_PIC_REGS
{
    const uint32_t VICIRQSTATUS;      /* IRQ Status Register, read only */
    const uint32_t VICFIQSTATUS;      /* FIQ Status Register, read only */
    const uint32_t VICRAWINTR;        /* Raw Interrupt Status Register, read only */
    uint32_t VICINTSELECT;            /* Interrupt Select Register */
    uint32_t VICINTENABLE;            /* Interrupt Enable Register */
    uint32_t VICINTENCLEAR;           /* Interrupt Enable Clear Register */
    uint32_t VICSOFTINT;              /* Software Interrupt Register */
    uint32_t VICSOFTINTCLEAR;         /* Software Interrupt Clear Register */
    uint32_t VICPROTECTION;           /* Protection Enable Register */
    const uint32_t Unused1[3];        /* Unused, should not be modified*/
    uint32_t VICVECTADDR;             /* Vector Address Register */
    uint32_t VICDEFVECTADDR;          /* Default Vector Address Register */
    const uint32_t Unused2[50];       /* Unused, should not be modified */
    uint32_t VICVECTADDRn[16];        /* Vector Address Registers */
    const uint32_t Unused3[48];       /* Unused, should not be modified */
    uint32_t VICVECTCNTLn[16];        /* Vector Control Registers */
} ARM926EJS_PIC_REGS;

/* 
 * Register map of the SIC is provided, but it is commented out
 * until the SIC is actually supported.
 */
#if 0
/*
 * 32-bit Registers of the Secondary Interrupt Controller,
 * relative to the controller's base address:
 * See page 4-49 of DUI0225D
 *
 * Note that the SIC is implemented inside a FPGA and is
 * not equal to the PIC. It triggers interrupt 31 on the PIC.
 * 
 * Although not explicitly mentioned by DUI0225D, there is a gap
 * among registers. The gap is filled by Unused1 and is treated 
 * as "should not be modified".
 *
 * Note that some registers share their addresses. See #defines below.
 */
typedef struct _ARM926EJS_SIC_REGS
{
    const uint32_t SIC_STATUS;        /* Status of interrupt (after mask), read only */
    const uint32_t SIC_RAWSTAT;       /* Status of interrupt (before mask), read only */
    uint32_t SIC_ENABLE;              /* Interrupt mask; also SIC_ENSET */
    uint32_t SIC_ENCLR;               /* Clears bits in interrupt mask */
    uint32_t SIC_SOFTINTSET;          /* Set software interrupt */
    uint32_t SIC_SOFTINTCLR;          /* Clear software interrupt */
    const uint32_t Unused1[2];        /* Unused, should not be modified */
    uint32_t SIC_PICENABLE;           /* Read status of pass-through mask; also SIC_PICENSET */
    uint32_t SIC_PICENCLR;            /* Clear interrupt pass through bits */
} ARM926EJS_SIC_REGS;

/* SIC_ENSET (Set bits in interrupt mask) shares its address with SIC_ENABLE. */
#define SIC_ENSET       SIC_ENABLE
/* SIC_PICENSET (Set interrupt pass through bits) shares its address with SIC_PICENABLE. */
#define SIC_PICENSET    SIC_PICENABLE

#endif /* #if 0 */

#define UL1             0x00000001
#define BM_5BITS        0x0000001F
#define BM_6TH_BIT      0x00000020

#define NR_VECTORS      16
#define NR_INTERRUPTS   32

/* Software interrupt number, see pp. 4-47 and 4-48 of DUI0225D */
#define SW_PIC_IRQ     1

static volatile ARM926EJS_PIC_REGS* const pPicReg = (ARM926EJS_PIC_REGS*) (PIC_BASE);
/* static volatile ARM926EJS_SIC_REGS* const pSicReg = (ARM926EJS_SIC_REGS*) (SIC_BASE); */

/*
 * A table with addresses of ISR routines for each IRQ request between 0 and 31.
 * The table is used for non vectored interrupt handling only.
 * 
 * A routine address for the given IRQ request ('irq') is simply accessed as
 * __isrNV[irq]. In future, this may be redesigned to support prioritization, additional parameters, etc.
 * 
 * TODO  parameters of routines as void* param ? 
 * TODO  prioritization parameters for each IRQ?
 */
static IsrPrototype __isrNV[NR_INTERRUPTS];

/*
 * IRQ handling mode:
 * - 0: nonvectored mode
 * - any other value: vectored mode
 */ 
static int8_t __irq_vector_mode = 0;


#if 0
/*
 * A "hidden" function that determines IRQ handling policy (vectored vs nonvectored).
 * It is used in this testing application for testing purposes only, in real applications 
 * it should never be used, this is why it should not be exposed in a .h file.
 * 
 * Until vectored IRQ handling is not supported, the function's implementation is commented out.
 */
void __pic_set_irq_vector_mode(int8_t mode)
{
    /* just set a variable to the requested mode */
    __irq_vector_mode = mode;
}
#endif


/*
 * IRQ handler routine, called directly from the IRQ vector, implemented in exception.c
 * Prototype of this function is not public and should not be exposed in a .h file. Instead,
 * its prototype must be declared as 'extern' where required (typically in exception.c only).
 *
 * NOTE:
 * There is no check that provided addresses are correct! It is up to developers
 * that valid ISR addresses are assigned before the IRQ is enabled!
 * 
 * It supports two modes of IRQ handling, vectored and nonvectored mode. They are implemented 
 * for testing purposes only, in a real application only one mode should be selected and implemented.
 */
void _pic_IrqHandler(void)
{
    if ( !__irq_vector_mode )
    {
        /*
	 * Non vectored implementation, a.k.a. "Simple interrupt flow", described
	 * on page 2-9 of DDI0181.
	 * 
	 * At the moment the IRQs are "prioritarized" by their numbers,
	 * smaller number is a higher priority.
	 */
	
        uint8_t irq;
        
	/*
	 * Check each bit of VICIRQSTATUS to determine which sources triggered
	 * an interrupt.
	 */
        for ( irq=0; irq<NR_INTERRUPTS; ++irq )
        {
            if ( pPicReg->VICIRQSTATUS & (UL1<<irq) )
            {
	        /*
		 * The irq'th bit is set, call its service routine:
		 */ 
                (*__isrNV[irq])();
            }
        }
    }
#if 0
    /* vectored mode not supported yet, so it is commented out */
    else
    {
        /*
	 * Vectored implementation, a.k.a. "Vectored interrupt flow sequence", described
	 * on page 2-9 of DDI0181.
	 */
	
        IsrPrototype isrAddr;
        
	/* 
	 * Reads the Vector Address Register with the ISR address of the currently active interrupt.
         * Reading this register also indicates to the priority hardware that the interrupt 
         * is being serviced.
	 */
        isrAddr = (IsrPrototype) pPicReg->VICVECTADDR;
	
	/* Execute the routine at the vector address */
        (*isrAddr)();
        
        /* 
	 * Writes an arbitrary value to the Vector Address Register. This indicates to the
         * priority hardware that the interrupt has been serviced. 
	 */
        pPicReg->VICVECTADDR = 0xFFFFFFFF;
    }
#endif
}


/**
 * Register an interrupt routine service (ISR) for the specified IRQ request.
 * 
 * Nothing is done if either irq (equal or greater than 32) or addr (equal to NULL)
 * is invalid.
 * 
 * @param irq - IRQ request number (must be smaller than 32)
 * @param addr - address of the ISR that services the interrupt 'irq'
 */ 
void pic_registerNonVectoredIrq(uint8_t irq, IsrPrototype addr)
{
    if (irq<NR_INTERRUPTS && NULL!=addr)
    {
        /* at the current implementation just put addr to the appropriate field of the table */
        __isrNV[irq] = addr;
    }
}


/**
 * Initializes the primary interrupt controler to default settings.
 *
 * All interrupt request lines are set to generate IRQ interrupts and all
 * interrupt request lines are disabled by default. Additionally, all vector 
 * and other registers are cleared.
 */
void pic_init(void)
{
    uint8_t i;
    
    /* All interrupt request lines generate IRQ interrupts: */
    pPicReg->VICINTSELECT = 0x00000000;
    
    /* Disable all interrupt request lines: */
    pPicReg->VICINTENCLEAR = 0xFFFFFFFF;
    
    /* Clear all software generated interrupt: */
    pPicReg->VICSOFTINTCLEAR = 0xFFFFFFFF;
    
    /* Reset the default vector address: */
    pPicReg->VICDEFVECTADDR = 0x00000000;
    
    /* for each vector: */
    for ( i=0; i<NR_VECTORS; ++i )
    {
        /* clear its ISR address */
        pPicReg->VICVECTADDRn[i] = 0x00000000;
        /* and clear its control register */
        pPicReg->VICVECTCNTLn[i] = 0x00000000;
    }
    
    /* clear all nonvectored ISR addresses: */
    for ( i=0; i<NR_INTERRUPTS; ++i )
    {
        __isrNV[i] = (IsrPrototype) 0x00000000;
    }
    
    /* set to non vectored mode (currently the only supported one) */
    __irq_vector_mode = 0;
}


/**
 * Enable the the interrupt request line on the PIC for the specified interrupt number.
 *
 * Nothing is done if 'irq' is invalid, i.e. equal or greater than 32.
 *
 * @param irq - interrupt number (must be smaller than 32)
 */
void pic_enableInterrupt(uint8_t irq)
{
    /* TODO check for valid (unreserved) interrupt numbers? Applies also for other functions */
    
    if ( irq < NR_INTERRUPTS )
    {
        /* See description of VICINTENABLE, page 3-7 of DDI0181: */
        pPicReg->VICINTENABLE |= ( UL1 << irq );
        
        /* Only the bit for the requested interrupt source is modified. */
    }
}


/**
 * Disable the the interrupt request line on the PIC for the specified interrupt number.
 *
 * Nothing is done if 'irq' is invalid, i.e. equal or greater than 32.
 *
 * @param irq - interrupt number (must be smaller than 32)
 */
void pic_disableInterrupt(uint8_t irq)
{
    if ( irq < NR_INTERRUPTS )
    {
        /* 
	 * VICINTENCLEAR is a write only register and any ateempt of reading it
	 * will result in a crash. For that reason, operators as |=, &=, etc.
	 * are not permitted and only direct setting of it (using operator =)
	 * is possible. This is not a problem anyway as only 1-bits disable their
	 * correspondent IRQs while 0-bits do not affect their corresponding
	 * interrupt lines.
	 * 
	 * For more details, see description of VICINTENCLEAR on page 3-7 of DDI0181.
	 */
        pPicReg->VICINTENCLEAR = ( UL1 << irq );
    }
}


/**
 * Disable all interrupt request lines of the PIC.
 */
void pic_disableAllInterrupts(void)
{
    /* 
     * See description of VICINTENCLEAR, page 3-7 of DDI0181.
     * All 32 bits of this register are set to 1.     
     */
    pPicReg->VICINTENCLEAR = 0xFFFFFFFF;
}


/**
 * Checks whether the interrupt request line for the requested interrupt is enabled.
 *
 * 0 is returned if 'irq' is invalid, i.e. equal or greater than 32.
 *
 * @param irq - interrupt number (must be smaller than 32)
 *
 * @return 0 if disabled, a nonzero value (typically 1) if the interrupt request line is enabled
 */
int8_t pic_isInterruptEnabled(uint8_t irq)
{
   /* See description of VICINTENCLEAR, page 3-7 of DDI0181: */
   
   if ( irq<NR_INTERRUPTS && 
        ( pPicReg->VICINTENABLE & (UL1<<irq) ) 
      )
   {
       return 1;
   }
   
   return 0;
}


/**
 * What type (IRQ or FIQ) is the requested interrupt?
 *
 * 0 is returned if 'irq' is invalid, i.e. equal or greater than 32.
 *
 * @param irq - interrupt number (must be smaller than 32)
 *
 * @return 0 if irq's type is FIQ, a nonzero value (typically 1) if the irq's type is IRQ 
 */
int8_t pic_getInterruptType(uint8_t irq)
{
    /* 
     * See description of VICINTSELECT, page 3-7 of DDI0181.
     *
     * If the corresponding bit is set to 1, the interrupt's type is FIQ,
     * otherwise it is IRQ.
     */
     
    return ( irq<NR_INTERRUPTS && 0==(pPicReg->VICINTSELECT & UL1<<irq) ? 1 : 0 );
}


/**
 * Set the requested interrupt to the desired type (IRQ or FIQ).
 *
 * Nothing is done if 'irq' is invalid, i.e. equal or greater than 32.
 *
 * @param irq - interrupt number (must be smaller than 32)
 * @param toIrq - if 0, set the interrupt's type to FIQ, otherwise set it to IRQ
 */
void pic_setInterruptType(uint8_t irq, int8_t toIrq)
{
    if (irq<NR_INTERRUPTS)
    {
    
        /*
         * Only the corresponding bit must be modified, all other bits must remain unmodified.
         * For that reason, appropriate bitwise operators are applied.
         *
         * The interrupts type is set via VICINTSELECT. See description
         * of the register at page 3-7 of DDI0181.
         */
        if ( toIrq )
        {
            /* Set the corresponding bit to 0 by bitwise and'ing bitmask's zero complement */
            pPicReg->VICINTSELECT &= ~(UL1 << irq);
        }
        else
        {
            /* Set the corresponding bit to 1 by bitwise or'ing the bitmask */
            pPicReg->VICINTSELECT |= (UL1 << irq);
        }
    }
}

/* 
 * These functions do not make any sense until vectored mode is supported.
 * Till then they are commented out.
 */
#if 0
/**
 * Reads the Vector Address Register with the ISR address of the currently active interrupt.
 * Reading this register also indicates to the priority hardware that the interrupt 
 * is being serviced.
 *
 * @return ISR address od the currently active interrupt
 */
IsrPrototype pic_getVectorAddr(void)
{
    return (IsrPrototype) pPicReg->VICVECTADDR;
}


/**
 * Writes an arbitrary value to the Vector Address Register. This indicates to the
 * priority hardware that the interrupt has been serviced. 
 *
 * @return ISR address od the currently active interrupt
 */
void pic_writeVectAddr(void)
{
    pPicReg->VICVECTADDR = 0xFFFFFFFF;
}


/**
 * Assigns default vector address.
 *
 * @param addr - address od the default ISR
 */ 
void pic_setDefaultVectorAddr(IsrPrototype addr)
{
    pPicReg->VICDEFVECTADDR = (uint32_t) addr;
}


/**
 * Registers a vector interrupt ISR for the requested interrupt request line.
 * The vectored interrupt is enabled by default.
 *
 * If the irq has already been registered, it is enabled and its ISR address 
 * is overridden with the new one.
 *
 * Nothing is done and -1 is returned if either 'irq' is invalid (must be less than 32) 
 * or ISR's address is NULL.
 *
 * @param irq - interrupt number (must be smaller than 32)
 * @param addr - address of the ISR
 *
 * @return interrupt's vector slot (0 to 15) or -1 if no slot available
 */
int8_t pic_registerVectorIrq(uint8_t irq, IsrPrototype addr)
{
    uint8_t cntr;
    int8_t empty;
    
    /* sanity check: */
    if (irq>=NR_INTERRUPTS || NULL==addr )
    {
        return -1;
    }
    
    /* Find either a slot with the 'irq' or the first empty slot: */
    for ( empty=-1, cntr=0; cntr<NR_VECTORS; ++cntr )
    {
        /* strip the enable bit and reserved bits from the VICVECTCNTLn: */
        uint8_t currentIrq = (uint8_t) (pPicReg->VICVECTCNTLn[cntr] & BM_5BITS);
        if ( currentIrq==irq )
        {
            /* found an occurrence of irq, discontinue the loop: */
            break;  /* out of for */
        }
        if (empty<0 && 0==currentIrq)
        {
	    /* TODO can be a watcdog (IRQ0)!!! */
            /* mark only the first empty slot found */
            empty = cntr;
        }
    } /* for cntr */
    
    if ( cntr >= NR_VECTORS )
    {
        /* no occurrence of irq found, use an empty slot if it exists: */
        if (empty<0)
        {
            /* no available slots */
            return -1;
        }
        
        cntr = empty;
    }
    
    /* Assign the ISR's address: */
    pPicReg->VICVECTADDRn[cntr] = (uint32_t) addr;
    /* and assign the request line and enable the interrupt: */
    pPicReg->VICVECTCNTLn[cntr] = irq + BM_6TH_BIT;
    
    return cntr;
}


/**
 * Unregisters a vector interrupt ISR for the requested interrupt request line.
 *
 * Nothing is done if 'irq' is invalid, i.e. equal or greater than 32 or
 * no vector for the 'irq' exists.
 *
 * @param irq - interrupt number (must be smaller than 32)
 */
void pic_unregisterVectorIrq(uint8_t irq)
{
    uint8_t i;
    
    if (irq>=NR_INTERRUPTS)
    {
        return;
    }
    
    for ( i=0; i<NR_VECTORS; ++i )
    {
        if ( irq != (pPicReg->VICVECTCNTLn[i] & BM_5BITS) )
        {
            /* This slot not configured for the requested irq, skip it */
            continue;  /* for i */
        }
        
        /* 
         * If this point is reached, the slot is configured for the irq.
         * Set its VICVECTCNTLn and VICVECTADDRn to 0.         
         */
        pPicReg->VICVECTCNTLn[i] = 0x00000000;
        pPicReg->VICVECTADDRn[i] = 0x00000000;
    }  /* for i */
}


/**
 * Enables the vectored interrupt for the requested interrupt request line.
 *
 * Nothing is done and -1 is returned if irq is invalid (equal or greater than 32) or
 * no slot with the requested irq has been found.
 *
 * @param irq - interrupt number (must be smaller than 32)
 *
 * @return interrupt's vector slot (0 to 15) or -1 if no slot has been found
 */
int8_t pic_enableVectorIrq(uint8_t irq)
{
    int8_t cntr;
    
    if ( irq>=NR_INTERRUPTS )
    {
        return -1;
    }
    
    /* Find a slot with the requested irq: */
    for ( cntr=0; cntr<NR_VECTORS; ++cntr )
    {
        if ( irq == (uint8_t) (pPicReg->VICVECTCNTLn[cntr] & BM_5BITS) )
        {
            /* found, set its ENABLE bit in VICVECTCNTLn: */
            pPicReg->VICVECTCNTLn[cntr] |= BM_6TH_BIT;
            /* No need to search further: */
            break;  /* out of for cntr */
        }
    } /* for cntr */
    
    return ( cntr<NR_VECTORS ? cntr : -1 );
}


/**
 * Disables the vectored interrupt for the requested interrupt request line.
 *
 * Nothing is done if irq is invalid (equal or greater than 32) or
 * no slot with the requested irq has been found.
 *
 * @param irq - interrupt number (must be smaller than 32)
 */
void pic_disableVectorIrq(uint8_t irq)
{
    uint8_t cntr;
    
    if (irq>=NR_INTERRUPTS)
    {
        return;
    }
    
    for ( cntr=0; cntr<NR_VECTORS; ++cntr )
    {
        if ( irq == (pPicReg->VICVECTCNTLn[cntr] & BM_5BITS) )
        {
            /* found an occurrence of irq, unset its ENABLE bit in VICVECTCNTLn: */
            pPicReg->VICVECTCNTLn[cntr] &= ~BM_6TH_BIT;
            /* 
             * Continue the loop and disable potential other slots
             * with the same irq (should not occur in normal situatuons but
             * it does not hurt). 
             */
        }
    }
}


/**
 * Unregisters all vector interrupts.
 */
void pic_unregisterAllVectorIrqs(void)
{
    uint8_t i;
    
    /* Clear all vector's VICVECTCNTLn and VICVECTADDRn registers: */
    for ( i=0; i<NR_VECTORS; ++i )
    {
        pPicReg->VICVECTCNTLn[i] = 0x00000000;
        pPicReg->VICVECTADDRn[i] = 0x00000000;
    }
}
#endif

/**
 * Triggers the software generated interrupt (IRQ1).
 */
void pic_setSoftwareInterrupt(void)
{
    /* 
     * Software interrupts are generated via VICSOFTINT.
     * See description of the register on page 3-8 of DDI0181.
     *
     * IRQ1 is reserved for a pure software interrupt. See pp 4.47 and 4-48 of DUI0225D.
     */
    pPicReg->VICSOFTINT |= (UL1 << SW_PIC_IRQ);
}


/**
 * Clears the software generated interrupt (IRQ1).
 */
void pic_clearSoftwareInterrupt(void)
{
    /* 
     * Software interrupts are cleared via VICSOFTINTCLEAR.
     * See description of the register on page 3-8 of DDI0181.
     *
     * IRQ1 is reserved for a pure software interrupt. See pp 4.47 and 4-48 of DUI0225D.
     */
    pPicReg->VICSOFTINTCLEAR |= (UL1 << SW_PIC_IRQ);
}
