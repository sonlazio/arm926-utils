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

OBJS = vectors.o exception.o interrupt.o uart.o timer.o rtc.o main.o
BSP_DEP = bsp.h
LINKER_SCRIPT = qemu.ld
ELF_IMAGE = image.elf
IMAGE = image.bin

all : $(IMAGE)

rebuild : clean all

$(IMAGE) : $(ELF_IMAGE)
	$(OBJCPY) -O binary $< $@

$(ELF_IMAGE) : $(OBJS) $(LINKER_SCRIPT)
	$(LD) -T $(LINKER_SCRIPT) $(OBJS) -o $@

interrupt.o : interrupt.c $(BSP_DEP)
	$(CC) -c $(CPUFLAG) $< -o $@

uart.o : uart.c $(BSP_DEP)
	$(CC) -c $(CPUFLAG) $< -o $@

timer.o : timer.c $(BSP_DEP)
	$(CC) -c $(CPUFLAG) $< -o $@

rtc.o : rtc.c $(BSP_DEP)
	$(CC) -c $(CPUFLAG) $< -o $@

main.o : main.c $(BSP_DEP)
	$(CC) -c $(CPUFLAG) $< -o $@

exception.o : exception.c
	$(CC) -c $(CPUFLAG) $< -o $@

vectors.o : vectors.s
	$(AS) $(CPUFLAG) $< -o $@

clean_intermediate :
	rm -f *.o
	rm -f *.elf
	rm -f *.img

clean : clean_intermediate
	rm -f *.bin

.PHONY : all rebuild clean clean_intermediate
