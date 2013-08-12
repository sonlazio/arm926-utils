#!/bin/bash
#
# Run an instance of "qemu-system-arm", emulating an ARM Versatile Application Baseboard with
# ARM926ES-J. Qemu is run in the "nographics mode", i.e. its display will not open and the
# board's UART0 serial port will be "connected" directly to the running shell's standard output
# and/or input.
#
# TODO: Currently the firmware's filename is hardcoded as "image.bin" (as built by "make all").
# It would be a nice idea if it could also be passed as a parameter to this script.

qemu-system-arm -M versatilepb -nographic -kernel image.bin
