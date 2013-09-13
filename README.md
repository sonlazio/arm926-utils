##About
A simple collection of drivers and test cases for the 
[ARM Versatile Platform Baseboard](http://infocenter.arm.com/help/topic/com.arm.doc.dui0225d/DUI0225D_versatile_application_baseboard_arm926ej_s_ug.pdf), 
based on the ARM926EJ-S CPU.
The design is modular enough (e.g. some board specific data are separated from drivers and
test cases) to simplify porting to other boards with similar hardware.
The project is experimental and its main purpose is to provide necessary code that would facilitate 
porting [FreeRTOS](http://www.freertos.org/) to this board and possibly other similar 
ARM based boards.

Only drivers for Qemu supported peripherals are available at the moment, e.g. a vector 
interrupt controller, timers, UARTs and a real time clock. Additional drivers may be 
implemented in the future.

Provided build scripts will prepare an image, suitable to run in Qemu. In order to run it 
anywhere else or even on a real board, _qemu.ld_ must be edited appropriately.

##Prerequisites
* _Sourcery CodeBench Lite Edition for ARM EABI_ toolchain (now owned by Mentor Graphics), 
based on GCC. See comments in _setenv.sh_ for more details about download and installation.
* _GNU Make_
* _Qemu_ (version 1.3 or newer, older versions do not emulate the interrupt controller properly!)

##Build
A convenience Bash script _setenv.sh_ is provided to set paths to toolchain's commands 
and libraries. You may edit it and adjust the paths according to your setup. To set up 
the necessary paths, simply type:

`. ./setenv.sh`

If you wish to run the image anywhere else except in Qemu, you will probably have to 
edit the linker script _qemu.ld_ and adjust the startup address properly.

To build the image with the test application, just run _make_ or _make rebuild_. 
If the build process is successful, the image file _image.bin_ will be ready to boot.

##Run
To run the target image in Qemu, enter the following command:

`qemu-system-arm -M versatilepb -nographic -m 128 -kernel image.bin`

A convenience Bash script _start\_qemu.sh_ is provided. If necessary, you may 
edit it and adjust paths to Qemu and/or target image.

When the test application completes, it ends running in an infinite loop. 
The instance of Qemu must be "killed" manually. A convenience Bash script 
_stop\_qemu.sh_ (it must be run in another shell) is provided to automate 
the process. Note that it may not work properly if several instances of 
_qemu-system-arm_ are running.

For more details, see extensive comments in both scripts.

##License
The source code is licenced under the Apache 2.0 license. See LICENSE.txt and 
[http://www.apache.org/licenses/LICENSE-2.0](http://www.apache.org/licenses/LICENSE-2.0) 
for more details.

##Author
The author of the application is Jernej Kova&#x010d;i&#x010d;.
 
