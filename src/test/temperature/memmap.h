
/* file name : memmap.h                                     */
/* linux /dev/mem mmap support head file					*/
/* 															*/
/* 															*/
/* Copyright 2005 huawei com.                               */
/* Author :zhouaidi(42136)									*/
/* Create date: 2005-04-07									*/
/* Modify history											*/
/*                                                          */

#ifndef __MEM_MAP_H__
#define __MEM_MAP_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

void * memmap(unsigned int phy_addr, unsigned int size);
int memunmap(void * addr_mapped);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif

