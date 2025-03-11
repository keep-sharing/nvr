#!/bin/bash
sed -i 's/#if 0 \/\/ enable tutk log/#if 1 \/\/ enable tutk log/g' ./src/msgui/mscore/mod_p2p.c

sed -i 's/\<lIOTCAPIs\>/lIOTCAPIsT/g' ./src/test/hardware/Makefile
sed -i 's/\<lIOTCAPIs\>/lIOTCAPIsT/g' ./src/qlibs/qlibs.pro
sed -i 's/\<lIOTCAPIs\>/lIOTCAPIsT/g' ./src/libs/mshw/Makefile
sed -i 's/\<lIOTCAPIs\>/lIOTCAPIsT/g' ./src/demo/Makefile
sed -i 's/\<lIOTCAPIs\>/lIOTCAPIsT/g' ./src/msgui/Makefile
sed -i 's/\<lIOTCAPIs\>/lIOTCAPIsT/g' ./src/msgui/mscore/Makefile
sed -i 's/\<lIOTCAPIs\>/lIOTCAPIsT/g' ./src/msgui/mscore/mscore.pri

sed -i 's/\<lAVAPIs\>/lAVAPIsT/g'  ./src/test/hardware/Makefile
sed -i 's/\<lAVAPIs\>/lAVAPIsT/g' ./src/qlibs/qlibs.pro
sed -i 's/\<lAVAPIs\>/lAVAPIsT/g' ./src/libs/mshw/Makefile
sed -i 's/\<lAVAPIs\>/lAVAPIsT/g' ./src/demo/Makefile
sed -i 's/\<lAVAPIs\>/lAVAPIsT/g' ./src/msgui/Makefile
sed -i 's/\<lAVAPIs\>/lAVAPIsT/g' ./src/msgui/mscore/Makefile
sed -i 's/\<lAVAPIs\>/lAVAPIsT/g' ./src/msgui/mscore/mscore.pri

sed -i 's/\<lTUTKGlobalAPIs\>/lTUTKGlobalAPIsT/g' ./src/test/hardware/Makefile
sed -i 's/\<lTUTKGlobalAPIs\>/lTUTKGlobalAPIsT/g' ./src/qlibs/qlibs.pro
sed -i 's/\<lTUTKGlobalAPIs\>/lTUTKGlobalAPIsT/g' ./src/libs/mshw/Makefile
sed -i 's/\<lTUTKGlobalAPIs\>/lTUTKGlobalAPIsT/g' ./src/demo/Makefile
sed -i 's/\<lTUTKGlobalAPIs\>/lTUTKGlobalAPIsT/g' ./src/msgui/Makefile
sed -i 's/\<lTUTKGlobalAPIs\>/lTUTKGlobalAPIsT/g' ./src/msgui/mscore/Makefile
sed -i 's/\<lTUTKGlobalAPIs\>/lTUTKGlobalAPIsT/g' ./src/msgui/mscore/mscore.pri
