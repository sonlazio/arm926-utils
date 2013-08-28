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
 * the board's real time clock controller.
 * 
 * @author Jernej Kovacic
 */


#ifndef _RTC_H_
#define _RTC_H_

#include <stdint.h>


void rtc_init(void);

void rtc_start(void);

int8_t rtc_isRunning(void);

void rtc_enableInterrupt(void);

void rtc_disableInterrupt(void);

void rtc_clearInterrupt(void);

void rtc_setLoad(uint32_t value);

void rtc_setMatch(uint32_t value);

uint32_t rtc_getMatch(void);

uint32_t rtc_getValue(void);

const volatile uint32_t* rtc_getValueAddr(void);


#endif  /* _RTC_H_*/
