一、交叉编译 （一般--host指定后，对应的编译器就不用指定了，会默认xxxx-gcc）
./configure --prefix=/home/user1/hong/smdd/arm_for_libuv/3536 --host=arm-hisiv300-linux
./configure --prefix=/home/user1/hong/smdd/arm_for_libuv/3798 --host=arm-histbv310-linux

二、裁剪
arm-hisiv300-linux-strip ./libuv.so.1.0.0
arm-histbv310-linux-strip libuv.so.1.0.0


三、复制到分支下对应平台的lib文件下
libuv.so 
libuv.so.1
libuv.so.1.0.0
复制到../branches-x.9.0.3/platform/hi3536/rootfs/usr/lib

