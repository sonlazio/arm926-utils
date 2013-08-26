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
 * the board's UART controller.
 * 
 * @author Jernej Kovacic
 */

#ifndef _UART_H_
#define _UART_H_

void uart_init(void);

void uart_printChar(char ch);

void uart_print(const char*);

void uart_enableUart(void);

void uart_disableUart(void);

void uart_enableTx(void);

void uart_disableTx(void);

void uart_enableRx(void);

void uart_disableRx(void);

#endif  /* _UART_H_ */
