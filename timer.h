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
 * the board's timer controller.
 * 
 * @author Jernej Kovacic
 */


#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdint.h>


void initTimer(uint8_t nr);

void startTimer(uint8_t nr);

void stopTimer(uint8_t nr);

int8_t isTimerEnabled(uint8_t nr);

void enableTimerInterrupt(uint8_t nr);

void disableTimerInterrupt(uint8_t nr);

void clearTimerInterrupt(uint8_t nr);

void setTimerLoad(uint8_t nr, uint32_t value);

uint32_t getTimerValue(uint8_t nr);

const volatile uint32_t* getTimerValueAddr(uint8_t nr);

int8_t getTimerIRQ(uint8_t nr);

#endif  /* _TIMER_H_*/
