#!/bin/bash
# build.sh -- a shell version of build.c for the new bootsect.s & setup.s
# author: falcon <wuzhangjin@gmail.com>
# update: 2008-10-10

bootsect=$1
setup=$2
system=$3
IMAGE=$4
root_dev=$5

# Set the biggest sys_size
# Changes from 0x20000 to 0x30000 by tigercn to avoid oversized code.
SYS_SIZE=$((0x3000*16))

# set the default "device" file for root image file
if [ -z "$root_dev" ]; then
	DEFAULT_MAJOR_ROOT=3
	DEFAULT_MINOR_ROOT=1
else
	DEFAULT_MAJOR_ROOT=${root_dev:0:2}
	DEFAULT_MINOR_ROOT=${root_dev:2:3}
fi

# Write bootsect (512 bytes, one sector) to stdout
[ ! -f "$bootsect" ] && echo "there is no bootsect binary file there" && exit -1
dd if=$bootsect bs=512 count=1 of=$IMAGE 2>&1 >/dev/null

# Write setup(4 * 512bytes, four sectors) to stdout
[ ! -f "$setup" ] && echo "there is no setup binary file there" && exit -1
dd if=$setup seek=1 bs=512 count=4 of=$IMAGE 2>&1 >/dev/null

# Write system(< SYS_SIZE) to stdout
[ ! -f "$system" ] && echo "there is no system binary file there" && exit -1
system_size=`wc -c $system |cut -d" " -f1`
[ $system_size -gt $SYS_SIZE ] && echo "the system binary is too big" && exit -1
dd if=$system seek=5 bs=512 count=$((2888-1-4)) of=$IMAGE 2>&1 >/dev/null

# Set "device" for the root image file
echo -ne "\x$DEFAULT_MINOR_ROOT\x$DEFAULT_MAJOR_ROOT" | dd ibs=1 obs=1 count=2 seek=508 of=$IMAGE conv=notrunc  2>&1 >/dev/null
