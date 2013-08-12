#!/bin/bash
#
# A convenience script that sets environment variables, necessary
# to use the ARM GCC toolchain.
#
# This script sets PATH and C_INCLUDE_PATH. If you intend to link
# the toolchain's libraries (e.g. libgcc.a), other variables
# (e.g. LIBRARY_PATH) must be set a s well.
#
# IMPORTANT: this script must be run as 
#     . ./setenv.sh
# or its longer equivalent:
#     source ./setenv.sh
#
# otherwise the variables will be discarded immediately after the script completes!


# NOTE: on x64 systems, make sure that 'ia32-libs' is also installed!

# I am using a Linux version of the toolchain, downloaded from:
# http://www.cl.cam.ac.uk/projects/raspberrypi/tutorials/os/downloads.html
# I have just unpacked the archive into my home directory. All toolchain's 
# directories are relative to this path:
TOOLCHAIN=~/arm-2008q3

# Add a path to gnu-none-eabi-* executables:
export PATH=$PATH:$TOOLCHAIN/bin

# After the script completes, you may check that output of
#    echo $PATH
# includes the desired path. Additionally you may check if arm-none-eabi-gcc
# is found if you attempt to run this executable.


# If you have gcc installed, C_INCLUDE_PATH might be set to its include paths.
# This may be confusing when you build ARM applications, therefore this variable
# (if it exists) will be overwritten:
export C_INCLUDE_PATH=$TOOLCHAIN/arm-none-eabi/include

# After the script completes, check the effect of this variable by executing:
# `arm-none-eabi-gcc -print-prog-name=cc1` -v
#
# More info about this at:
# http://stackoverflow.com/questions/344317/where-does-gcc-look-for-c-and-c-header-files

# Export other environment variables (e.g. LIBRARY_PATH) if necessary.
