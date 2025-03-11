#!/bin/bash

start_time=$(date +%s)

ROOT="/share"
VERSION=$(cat src/msgui/mscore/mssysconf.h | sed -n '/MS_HI_NVR_VERSION/s#.*"\(.*\)"#\1#p')
TIME=$(date "+%Y%m%d_%H%M%S")
PLATFORM=$1
TARGET="$ROOT/${TIME}_${VERSION}"

#平台切换
if [ ! -n "$PLATFORM" ]; then
    echo "Error Platform: cannot be empty" 
    exit
fi
make $PLATFORM
if [  $? -ne 0 ]; then
    echo "Error Platform: $PLATFORM"
    exit
fi

#-a版本默认打开tsar
if [[ $VERSION =~ -a[0-9]+ ]]; then
    sed -i 's/\<debug_level ERROR\>/debug_level DEBUG/g' ./platform/$PLATFORM/rootfs/etc/tsar.conf
    sed -i 's/\<tsar_log_stdout off\>/tsar_log_stdout on/g' ./platform/$PLATFORM/rootfs/etc/tsar.conf
    sed -i 's/\<tsar_log_fileout off\>/tsar_log_fileout on/g' ./platform/$PLATFORM/rootfs/etc/tsar.conf
    sed -i 's/\<tsar_data_fileout off\>/tsar_data_fileout on/g' ./platform/$PLATFORM/rootfs/etc/tsar.conf
fi

#编译
make distclean && make gui-clean 
make all
if [  $? -ne 0 ]; then
    exit
fi

#创建保存路径
mkdir -p $TARGET/$PLATFORM/symbols

#复制镜像和uboot
cp ./targets/$PLATFORM/firmware/MSFImage_*$VERSION* $TARGET/$PLATFORM
cp ./targets/$PLATFORM/firmware/UBOOT_*.bin $TARGET/$PLATFORM

#备份符号文件
echo "wait for backup symbols..."
find temp/$PLATFORM -name "*.so" | xargs -n1 -I {} cp {} $TARGET/$PLATFORM/symbols
cp temp/$PLATFORM/gui/mscore $TARGET/$PLATFORM/symbols
cd $TARGET/$PLATFORM
tar -zcf symbols.tar.gz symbols
rm -r $TARGET/$PLATFORM/symbols

#
end_time=$(date +%s)
cost_time=$[ $end_time - $start_time ]
echo "build $1 took $(($cost_time/60))min $(($cost_time%60))s"
echo "build finished: $TARGET"
