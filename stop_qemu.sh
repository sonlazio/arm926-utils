#!/bin/bash
#
# A convenient script to terminate a running instance of "system-arm-qemu",
# an equivalent to power off a real board.
# It may be useful when the application stops and the emulated processor
# keeps running in an infinite loop.
#
# NOTE: the script might not work properly if more than one instance
# of "system-arm-qemu" is running!


# Obtain the PID of (presumably) the only running instance of "system-arm-qemu"...

# The 11th element of the appropriate output of 'ps' must end with "qemu-system-arm":

# PID=`ps aux | grep qemu-system-arm | awk '$11=="qemu-system-arm" {print $2}'`
PID=`ps aux | grep qemu-system-arm | awk '$11~/qemu-system-arm$/ {print $2}'`

# ... and kill the process with the PID
kill $PID
