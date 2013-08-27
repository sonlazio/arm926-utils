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
 * Definitions of base addresses of ARM926EJ-S and Versatile
 * Application Baseboard peripherals.
 * 
 * More details at:
 * Versatile Application Baseboard for ARM926EJ-S, HBI 0118 (DUI0225D):
 * http://infocenter.arm.com/help/topic/com.arm.doc.dui0225d/DUI0225D_versatile_application_baseboard_arm926ej_s_ug.pdf
 *
 * The header should be included into each source file that implenets peripherals' drivers.
 * @author Jernej Kovacic
 */


/*
 * At the moment, this header file is maintained  manually.
 * Ideally, one day it will be generated automaticaly by scripts that
 * read data from BSP (board support package) databases. 
 */

#ifndef _BASE_ADDRESS_H_
#define _BASE_ADDRESS_H_

/* Base addresses of all 3 UARTs (see page 4-68 of the DUI0225D): */
#define UART0_BASE      0x101F1000
#define UART1_BASE      0x101F2000
#define UART2_BASE      0x101F3000


/* Base addresses of the Primary Interrupt Controller (see page 4-44 of the DUI0225D): */
#define PIC_BASE        0x10140000

/* Base addresses of the Secondary Interrupt Controller (see page 4-44 of the DUI0225D): */
#define SIC_BASE        0x10003000


/* Base addresses for all 4 timers (see page 4-21 of DUI0225D): */
#define TIMER0_BASE     0x101E2000
#define TIMER1_BASE     0x101E2020
#define TIMER2_BASE     0x101E3000
#define TIMER3_BASE     0x101E3020


/* Base address of the Real time Clock (see page 4-60 of the DUI0225D): */
#define RTC_BASE        0x101E8000


/* Base address of the Watchdog: */
#define WATCHDOG_BASE   0x101E1000

/* More base address will follow when peripherals are supported */

#endif  /* _BASE_ADDRESS_H_ */
