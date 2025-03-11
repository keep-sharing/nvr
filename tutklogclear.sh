#!/bin/bash
sed -i 's/#if 1 \/\/ enable tutk log/#if 0 \/\/ enable tutk log/g' ./src/msgui/mscore/mod_p2p.c

sed -i 's/\<lIOTCAPIsT\>/lIOTCAPIs/g' ./src/test/hardware/Makefile
sed -i 's/\<lIOTCAPIsT\>/lIOTCAPIs/g' ./src/qlibs/qlibs.pro
sed -i 's/\<lIOTCAPIsT\>/lIOTCAPIs/g' ./src/libs/mshw/Makefile
sed -i 's/\<lIOTCAPIsT\>/lIOTCAPIs/g' ./src/demo/Makefile
sed -i 's/\<lIOTCAPIsT\>/lIOTCAPIs/g' ./src/msgui/Makefile
sed -i 's/\<lIOTCAPIsT\>/lIOTCAPIs/g' ./src/msgui/mscore/Makefile
sed -i 's/\<lIOTCAPIsT\>/lIOTCAPIs/g' ./src/msgui/mscore/mscore.pri

sed -i 's/\<lAVAPIsT\>/lAVAPIs/g'  ./src/test/hardware/Makefile
sed -i 's/\<lAVAPIsT\>/lAVAPIs/g' ./src/qlibs/qlibs.pro
sed -i 's/\<lAVAPIsT\>/lAVAPIs/g' ./src/libs/mshw/Makefile
sed -i 's/\<lAVAPIsT\>/lAVAPIs/g' ./src/demo/Makefile
sed -i 's/\<lAVAPIsT\>/lAVAPIs/g' ./src/msgui/Makefile
sed -i 's/\<lAVAPIsT\>/lAVAPIs/g' ./src/msgui/mscore/Makefile
sed -i 's/\<lAVAPIsT\>/lAVAPIs/g' ./src/msgui/mscore/mscore.pri

sed -i 's/\<lTUTKGlobalAPIsT\>/lTUTKGlobalAPIs/g' ./src/test/hardware/Makefile
sed -i 's/\<lTUTKGlobalAPIsT\>/lTUTKGlobalAPIs/g' ./src/qlibs/qlibs.pro
sed -i 's/\<lTUTKGlobalAPIsT\>/lTUTKGlobalAPIs/g' ./src/libs/mshw/Makefile
sed -i 's/\<lTUTKGlobalAPIsT\>/lTUTKGlobalAPIs/g' ./src/demo/Makefile
sed -i 's/\<lTUTKGlobalAPIsT\>/lTUTKGlobalAPIs/g' ./src/msgui/Makefile
sed -i 's/\<lTUTKGlobalAPIsT\>/lTUTKGlobalAPIs/g' ./src/msgui/mscore/Makefile
sed -i 's/\<lTUTKGlobalAPIsT\>/lTUTKGlobalAPIs/g' ./src/msgui/mscore/mscore.pri
