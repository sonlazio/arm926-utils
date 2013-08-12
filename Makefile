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


TOOLCHAIN = arm-none-eabi-
CC = $(TOOLCHAIN)gcc
AS = $(TOOLCHAIN)as
LD = $(TOOLCHAIN)ld
OBJCPY = $(TOOLCHAIN)objcopy

CPUFLAG = -mcpu=arm926ej-s

all : image.bin

image.bin : image.elf
	$(OBJCPY) -O binary image.elf image.bin

image.elf : startup.o uart.o timer.o main.o linker.ld
	$(LD) -T linker.ld timer.o uart.o main.o -o image.elf

uart.o : uart.c
	$(CC) -c $(CPUFLAG) uart.c -o uart.o

timer.o : timer.c
	$(CC) -c $(CPUFLAG) timer.c -o timer.o

main.o : main.c
	$(CC) -c $(CPUFLAG) main.c -o main.o

startup.o : startup.s
	$(AS) $(CPUFLAG) startup.s -o startup.o

clean :
	rm -f *.o
	rm -f *.elf
	rm -f *.img
	rm -f *.bin

.PHONY : all clean
