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

void setupTimer(unsigned nr);

void startTimer(unsigned nr);

void stopTimer(unsigned nr);

void enableTimerInterrupt(unsigned nr);

void disableTimerInterrupt(unsigned nr);

void clearTimerInterrupt(unsigned nr);

void setTimerLoad(unsigned nr, unsigned long value);

unsigned long getTimerValue(unsigned nr);

const unsigned long* getTimerValueAddr(unsigned nr);

unsigned long getTimerCtrl(unsigned nr);

int getTimerIRQ(unsigned nr);

#endif  /* _TIMER_H_*/
