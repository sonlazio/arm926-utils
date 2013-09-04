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
 * Definitions of peripherals' IRQs of the Versatile Application Baseboard.
 * 
 * More details at:
 * Versatile Application Baseboard for ARM926EJ-S, HBI 0118 (DUI0225D):
 * http://infocenter.arm.com/help/topic/com.arm.doc.dui0225d/DUI0225D_versatile_application_baseboard_arm926ej_s_ug.pdf
 *
 * The header should be included into each source file that implements IRQ setting and servicing.
 * 
 * @author Jernej Kovacic
 */


/*
 * At the moment, this header file is maintained  manually.
 * Ideally, one day it will be generated automaticaly by scripts that
 * read data from BSP (board support package) databases. 
 */

#ifndef _PERIPHERAL_IRQS_H_
#define _PERIPHERAL_IRQS_H_

/* 
 * IRQs of UARTS.
 * See page 4-68 of the DUI0225D.
 */
#define IRQ_UART0        12
#define IRQ_UART1        13
#define IRQ_UART2        14


/*
 * IRQs of timers.
 * See page 4-67 of the DUI0225D.
 */
#define IRQ_TIMER0        4
#define IRQ_TIMER1        5


/*
 * IRQ of the real-time clock (RTC).
 * See page 4-60 of the DUI0225D.
 */
#define IRQ_RTC          10


/*
 * IRQ of the watchdog.
 * See page 4-72 of the DUI0225D.
 */
#define IRQ_WATCHDOG      0


/*
 * IRQ, reserved for software generated interrupts.
 * See pp.4-46 to 4-48 of the DUI0225D.
 */
#define IRQ_SOFTWARE      1


#endif  /* _PERIPHERAL_IRQS_H_ */
