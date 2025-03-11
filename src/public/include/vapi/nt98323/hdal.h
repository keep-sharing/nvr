/**
	@mainpage hdal

	@section Description
	This document explains common API in Novatek's Device Abstraction Layer which is independent of the specific platform.
*/

/**
	@brief Collection of HDAL API header file.

	@file hdal.h

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef	_HDAL_H_
#define	_HDAL_H_

/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_common.h"
#include "hd_audiocapture.h"
#include "hd_audioout.h"
#include "hd_audioenc.h"
#include "hd_audiodec.h"
#include "hd_videocapture.h"
#include "hd_videoout.h"
#include "hd_videoenc.h"
#include "hd_videodec.h"
#include "hd_videoprocess.h"
#include "hd_gfx.h"
#include "hd_logger.h"

/********************************************************************
	MACRO CONSTANT DEFINITIONS
********************************************************************/
#ifndef HDAL_VERSION
#define HDAL_VERSION    0x020000 ///< Ver 2.0
#endif

#endif

