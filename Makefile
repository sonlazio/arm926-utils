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

CPUFLAG = -mcpu=arm926ej-s

rebuild : clean all

all : image.bin

image.bin : image.elf
	$(OBJCPY) -O binary image.elf image.bin

image.elf : vectors.o exception.o uart.o timer.o main.o linker.ld
	$(LD) -T linker.ld exception.o timer.o uart.o main.o -o image.elf

uart.o : uart.c
	$(CC) -c $(CPUFLAG) uart.c -o uart.o

timer.o : timer.c
	$(CC) -c $(CPUFLAG) timer.c -o timer.o

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
