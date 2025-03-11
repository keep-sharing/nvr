#!/bin/bash

if [ $# != 5 ]; then
	echo "$#";
        echo "usage:";
        echo "     mkboot.sh <loader file> <fdt file> <atf file> <uboot file> <output file>";
        echo "     e.g." 
	echo "     mkboot.sh loader.bin fdt.bin u-boot.lz.bin output.bin";
	echo "";
	exit 1;
fi

dd if=/dev/zero bs=65536 count=4 | tr '\000' '\377' > loader.mk
dd if=$1 of=loader.mk bs=65536 count=4 conv=notrunc seek=0
dd if=/dev/zero bs=65536 count=4 | tr '\000' '\377' > fdt.mk
dd if=$2 of=fdt.mk bs=65536 count=4 conv=notrunc seek=0
dd if=/dev/zero bs=65536 count=4 | tr '\000' '\377' > atf.mk
dd if=$3 of=atf.mk bs=65536 count=4 conv=notrunc seek=0
cat loader.mk fdt.mk atf.mk $4 > $5
rm -rf loader.mk fdt.mk atf.mk

