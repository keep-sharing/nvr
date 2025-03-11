1.编译linux内核，使其支持pppoe，并将产生的文件拷进NVR芯片
2.编译ppp-2.4.5源码，并将产生的文件拷进NVR芯片
3.编译rp-pppoe-3.8源码
4.将第三步生成的配置脚本修改后（修改文件路径），拷进开发板
5.启动连接



（1）
首先得让内核支持PPP，进入Linux 内核目录，执行 #make menuconfig
添加如下内核选项：
Device Drivers ---> Network device support --->
<*> PPP (point-to-point protocol) support
[*]   PPP multilink support
<*> PPP support for async serial ports
<*> PPP support for sync tty ports
<*> SLIP (serial line) support
[*]   CSLIP compressed headers
添加完成后保存并退出，执行# make zImage
编译完成以后，在内核目录的arch/arm/boot 目录下便会生成一个 zImage 内核文件，烧到开发板中，这样开发板内核就支持了PPP了。

将生成的uImage覆盖\branches-x.9.0.4\targets\hi3536\firmware下的uImage    mv ./uImage ./branches-x.9.0.4/targets/hi3536/firmware/
编译镜像文件   make apps gui all    
将编译好的镜像文件拷进tftpboot文件夹    cp ./targets/hi3536/firmware/MSFImage_71.9.0.4-r1 /tftpboot/
在NVR中进入uboot模式，然后烧录  download


（2）
解压源码包  tar zxvf ppp-2.4.5.tar.gz
进入源码文件夹  cd ppp-2.4.5
1、配置文件  ./configure --prefix=
2、编译  make CC=arm-hisiv300-linux-gcc  （设置了交叉编译器，若命令未找到则执行   export PATH=$PATH:/opt/hisi-linux/x86-arm/arm-hisiv300-linux/target/bin）
3、更改Makefile文件，install默认用pc的strip，需要删除裁剪的步骤，install 后的-s需要去掉。
4、make install 
5、arm-hisiv300-linux-strip 
6、把在pppd文件夹下生成的pppd可执行文件拷贝到目标板（NVR芯片）的/opt/app/bin/文件夹下  scp ./pppd/pppd 192.168.2.131:/opt/app/bin/
在NVR的/opt/app/bin/目录下输入pppd，若有乱码则说明这一步成功了，若内核不支持会有提示


（3）
解压源码包  tar zxvf rp-pppoe-3.8.tar.gz
进入源码的src文件夹  cd rp-pppoe-3.8/src
配置文件 ./configure --host=arm-linux CC=arm-hisiv300-linux-gcc   （设置了编译平台和交叉编译器）
修改makefile文件（共两个：src下和libevent下）：gcc->arm-hisiv300-linux-gcc    ；   ar->arm-hisiv300-linux-ar
编译 make all
安装 make install


（4）
将rp-pppoe-3.8/configs下的所有脚本文件拷贝到NVR上的/etc/ppp/目录下，并将此目录加上可写权限
将/usr/sbin下产生的所有pppoe文件（pppoe，pppoe-server，pppoe-sniff，pppoe-relay，pppoe-setup， pppoe-start，pppoe-stop，pppoe-status，pppoe-connect）拷贝到NVR上的/opt/app/bin/目录下，并修改其中的相关文件路径，使得其能在NVR上运行


（5）
执行pppoe-setup   输入用户名等信息
执行pppoe-start



