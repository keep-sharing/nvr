/*
 *  include/asm/arch-nvt-na51xxx/hardware.h
 *
 *  Author:	Howard Chang
 *  Created:	Jan 21, 2016
 *  Copyright:	Novatek Inc.
 *
 */
#ifndef __ARCH_HARDWARE_H__
#define __ARCH_HARDWARE_H__

#define __REG(x) 	(*((volatile u32 *) (x)))

enum CHIP_ID {
	CHIP_NA51055 = 0x4821,
	CHIP_NA51084 = 0x5021,
	CHIP_NA51089 = 0x7021,
	CHIP_NA51090 = 0xBC21,
	CHIP_NA51102 = 0x5221,
	CHIP_NA51103 = 0x8B20
};

#define CHIPVER_A                       0x0
#define CHIPVER_B                       0x1
#define CHIPVER_C                       0x2


extern u32 nvt_get_chip_id(void);
extern u32 nvt_get_chip_ver(void);

#define NVT_TIMER0_CNT		0xF0040108

#endif /* __ARCH_HARDWARE_H__ */
