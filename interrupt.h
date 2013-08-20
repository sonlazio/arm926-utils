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
 * Declaration of public functions that handle
 * the board's primary interrupt controller (PIC).
 * 
 * @author Jernej Kovacic
 */

#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_


#include <stdint.h>


typedef void (*IsrPrototype)(void);

void pic_registerNonVectoredIrq(uint8_t irq, IsrPrototype addr);

void pic_init(void);

void pic_enableInterrupt(uint8_t irq);

void pic_disableInterrupt(uint8_t irq);

void pic_disableAllInterrupts(void);

int8_t pic_isInterruptEnabled(uint8_t irq);

int8_t pic_getInterruptType(uint8_t irq);

void pic_setInterruptType(uint8_t irq, int8_t toIrq);


#if 0
IsrPrototype pic_getVectorAddr(void);

void pic_writeVectAddr(void);
 
void pic_setDefaultVectorAddr(IsrPrototype addr);

int8_t pic_registerVectorIrq(uint8_t irq, IsrPrototype addr);

void pic_unregisterVectorIrq(uint8_t irq);

int8_t pic_enableVectorIrq(uint8_t irq);

void pic_disableVectorIrq(uint8_t irq);

void pic_unregisterAllVectorIrqs(void);
#endif


void pic_setSoftwareInterrupt(void);

void pic_clearSoftwareInterrupt(void);

#endif  /* _INTERRUPT_H_ */
 