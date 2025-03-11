重新编译dhcpv6步骤：
1、
../build/src/dhcpv6/dhcpv6.mk打开configure命令(./configure 前面的#去掉)

2、
每次configure之后，需要重新修改config.h文件(../src/dhcpv6/config.h)
    
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

3、
第一次解压源码时，需要修改dhcp6c.c文件(../src/dhcpv6/src/dhcp6c.c)
	
	在main()函数中。

	#ifndef __DARWIN_DEPRECATED_ATTRIBUTE

	    if (foreground == 0) {

#if 0		//David          //注释这部分代码

        if (daemon(0, 0) < 0) {

            err(1, "daemon");

        }
#endif
	
	
4、
编译dhcpv6需要依赖libnl库