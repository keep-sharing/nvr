#!/bin/bash

if [ $# != 4 ]; then
	echo "$#";
        echo "usage:";
        echo "     mkboot.sh <loader file> <fdt file> <uboot file> <output file>";
        echo "     e.g." 
	echo "     mkboot.sh loader.bin fdt.bin u-boot.lz.bin output.bin";
	echo "";
	exit 1;
fi

dd if=/dev/zero bs=65536 count=1 | tr '\000' '\377' > loader.mk
dd if=$1 of=loader.mk bs=65536 count=1 conv=notrunc seek=0
dd if=/dev/zero bs=65536 count=1 | tr '\000' '\377' > fdt.mk
dd if=$2 of=fdt.mk bs=65536 count=1 conv=notrunc seek=0
cat loader.mk fdt.mk $3 > $4
rm -rf loader.mk fdt.mk

