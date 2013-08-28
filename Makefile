# Copyright 2013, Jernej Kovacic
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Version 2013-05.23 of the Sourcery toolchain is used as a build tool.
# See comments in "setenv.sh" for more details about downloading it
# and setting the appropriate environment variables.

TOOLCHAIN = arm-none-eabi-
CC = $(TOOLCHAIN)gcc
CXX = $(TOOLCHAIN)g++
AS = $(TOOLCHAIN)as
LD = $(TOOLCHAIN)ld
OBJCPY = $(TOOLCHAIN)objcopy
AR = $(TOOLCHAIN)ar

CPUFLAG = -mcpu=arm926ej-s

all : image.bin

rebuild : clean all

image.bin : image.elf
	$(OBJCPY) -O binary image.elf image.bin

image.elf : vectors.o exception.o interrupt.o uart.o timer.o rtc.o main.o linker.ld
	$(LD) -T linker.ld exception.o interrupt.o timer.o uart.o rtc.o main.o -o image.elf

interrupt.o : interrupt.c
	$(CC) -c $(CPUFLAG) interrupt.c -o interrupt.o

uart.o : uart.c
	$(CC) -c $(CPUFLAG) uart.c -o uart.o

timer.o : timer.c
	$(CC) -c $(CPUFLAG) timer.c -o timer.o

rtc.o : rtc.c
	$(CC) -c $(CPUFLAG) rtc.c -o rtc.o

main.o : main.c
	$(CC) -c $(CPUFLAG) main.c -o main.o

exception.o : exception.c
	$(CC) -c $(CPUFLAG) exception.c -o exception.o

vectors.o : vectors.s
	$(AS) $(CPUFLAG) vectors.s -o vectors.o

clean :
	rm -f *.o
	rm -f *.elf
	rm -f *.img
	rm -f *.bin

.PHONY : all clean rebuild
