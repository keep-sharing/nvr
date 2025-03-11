/*
 *  linux/init/ms_kernel_versioon.h
 *
 *  Copyright (C) 2014  Milesight,Ltd
 *
 *  May be freely distributed as part of Linux.
 */
 
#ifndef __MS_KERNEL_VERSION_H__
#define __MS_KERNEL_VERSION_H__


// transfer to string
#ifndef __STR__ 
#define __STR__(str)			#str
#endif

#ifndef VERSION_MAKE_STR
#define VERSION_MAKE_STR(str)	__STR__(str)
#endif

#ifndef KERNEL_SUB_VERSION
#define KERNEL_SUB_VERSION		5
#else
#warning *** BUG ON *** KERNEL_SUB_VERSION has defined
#endif

#ifndef MS_COMPILE_VERSION
#define MS_COMPILE_VERSION		73.1.0.5
#else
#warning *** BUG ON *** MS_COMPILE_VERSION has defined
#endif

#define MS_KERNEL_VERSION		VERSION_MAKE_STR(MS_COMPILE_VERSION)

const char* version_des[] = 
{
};

#endif /* __MS_KERNEL_VERSION_H__ */
