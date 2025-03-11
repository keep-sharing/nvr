一、libnl 1.1
1.解压libnl-1.1.tar.gz 到指定目录下 ../libnl_pro
2.修改libnl_pro内的文件
    **** .../libnl_pro/lib/route/link/vlan.c ****

    #include <linux/if_vlan.h>
    //找到这里，新增以下3行代码
    #ifndef VLAN_FLAG_REORDER_HDR
    #define VLAN_FLAG_REORDER_HDR 0x01
    #endif
    
3.交叉编译
    在目录../libnl_pro下执行，#### /home/user1/hong/smdd/pro/libnl/hi3536/ 为对应存放库文件路径
    ./configure CC=arm-hisiv300-linux-gcc --host=arm --prefix=/home/user1/hong/smdd/pro/libnl/hi3536/
    ./configure CC=arm-histbv310-linux-gcc --host=arm --prefix=/home/user1/hong/smdd/pro/libnl/hi3798/
    make clean
    make
    make install

    

二、DHCPv6
1.解压dhcpv6-1.2.0.tar.gz 到指定目录下 ../dhcpv6_pro  【3536和3798需区分开交叉编译，因此可以复制一个文件如../dhcpv6_pro_hi3798】
2.交叉编译：在../dhcpv6_pro路径下执行, /home/user1/hong/smdd/pro/libnl/hi3798/lib 为上面获取libnl的路径
    ./configure LIBNL_CFLAGS=-I/home/user1/hong/smdd/pro/libnl/include LIBNL_LIBS="-L/home/user1/hong/smdd/pro/libnl/hi3798/lib -lnl" CC=arm-histbv310-linux-gcc --host=arm --with-gnu-ld
    ./configure LIBNL_CFLAGS=-I/home/user1/hong/smdd/pro/libnl/include LIBNL_LIBS="-L/home/user1/hong/smdd/pro/libnl/hi3536/lib -lnl" CC=arm-hisiv300-linux-gcc --host=arm --with-gnu-ld
    
3.对项目下的一些文件必须作出修改
    **** ../dhcpv6_pro/config.h ****
    
    /* Directory for lease databases and DUID files. */

	//#define DB_FILE_PATH "/var/lib/dhcpv6"		//David.Milesight modify 14.07.28

	#define DB_FILE_PATH "/tmp"



	/* Directory for PID files. */

	//#define PID_FILE_PATH "/var/run/dhcpv6"		//David.Milesight modify 14.07.28

	#define PID_FILE_PATH "/tmp"	



	/* Define to rpl_malloc if the replacement function should be used. */

	//#define malloc rpl_malloc						//David			



	/* Define to `int' if <sys/types.h> does not define. */

	/* #undef pid_t */					



	/* Define to rpl_realloc if the replacement function should be used. */

	//#define realloc rpl_realloc					//David
    
    
    
    ***** ../dhcpv6_pro/src/dhcp6c.c *******
    
    在main()函数中。

	#ifndef __DARWIN_DEPRECATED_ATTRIBUTE

	    if (foreground == 0) {

#if 0		//David          //注释这部分代码

        if (daemon(0, 0) < 0) {

            err(1, "daemon");

        }
#endif
    
    
4.编译获取可执行文件
在路径 ../dhcpv6_pro/src 执行make clean 和make就可以在./src下生成对应的可执行文件 dhcp6c


备注：libnl路径修改的话请修改makefile以下两行
      LIBNL_CFLAGS = -I/home/user1/hong/smdd/pro/libnl/hi3536/include
      LIBNL_LIBS = -L/home/user1/hong/smdd/pro/libnl/hi3536/lib -lnl
      
      

三、使用于NVR上

	首先将../libnl/lib下的libnl.so,libnl.so.1,libnl.so.1.1拷到对应的libs文件里，如./targets/hi3536/app/libs/

	然后将../dhcpv6_pro/src下的dhcp6c拷到对应bin文件里，如./targets/hi3536/app/bin/
