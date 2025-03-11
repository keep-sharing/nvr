/**
 * system/include/hiboot_version.h
 *
 * History:
 *    2015.05.20 - [zbing] created file 
 *
 * Copyright (C) 2012-2015, Milesight, Inc.
 *
 * All rights reserved. No Part of this file may be reproduced, stored
 * in a retrieval system, or transmitted, in any form, or by any means,
 * electronic, mechanical, photocopying, recording, or otherwise,
 * without the prior consent of Milesight, Inc.
 */
 
#ifndef __HIBOOT_VERSION_H__
#define __HIBOOT_VERSION_H__

#define MS_VER_NUM(main,sub0,sub1,sub2) \
	((main << 24 ) | (sub0 << 16) | (sub1 << 8) | (sub2))

#define MS_VER_DATE(year,month,day) \
	((year << 16) | (month << 8) | day)	

// try to link version number
#define LINK_STR(a,b,c,d)	a##.##b##.##c##.##d
#define LINK_VER(a,b,c,d)	LINK_STR(a,b,c,d)

//ms_config define
#ifndef VER_CPU
#error "No define macro VER_CPU"
#endif

#ifndef VER_FILESYS
#error "No define macro VER_FILESYS"
#endif

#ifndef VER_DDR
#define VER_DDR	0
#endif

#ifndef VER_DDR3_512M
#error "No define macro VER_DDR3_512M"
#endif

#ifndef VER_DDR3_1G
#error "No define macro VER_DDR3_1G"
#endif

#ifndef VER_FLASH
#error "No define macro VER_FLASH"
#endif

// define ms boot logo.jpg version
#define VER_JPG		2

// define ms hiboot version
#define VER_HIBOOT	10
#define VER_BLD_NUM	MS_VER_NUM(VER_CPU,VER_FLASH,VER_DDR3_512M,VER_HIBOOT)
#define VER_BLD2_NUM	MS_VER_NUM(VER_CPU,VER_FLASH,VER_DDR3_1G,VER_HIBOOT)
#define VER_JPG_NUM	MS_VER_NUM(VER_CPU,VER_FLASH,0,VER_JPG)

#ifndef VER_LINUX
#error "No define macro VER_LINUX"
#endif

#ifndef VER_FS_MAIN
#error "No define macro VER_FS_MAIN"
#endif
#ifndef VER_FS_SUB	
#error "No define macro VER_FS_SUB"
#endif
#ifndef VER_FS_THIRD	
#error "No define macro VER_FS_THIRD"
#endif

#define VER_KER_NUM MS_VER_NUM(VER_CPU,VER_FLASH,VER_DDR,VER_LINUX)
#define VER_FS_NUM	MS_VER_NUM(VER_FILESYS,VER_FS_MAIN,VER_FS_SUB,VER_FS_THIRD)


#endif /* __HIBOOT_VERSION_H__ */
