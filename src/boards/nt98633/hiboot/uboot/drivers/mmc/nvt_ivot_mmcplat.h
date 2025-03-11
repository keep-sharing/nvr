#ifndef __NVT_IVOT_MMCPLAT_H__
#define __NVT_IVOT_MMCPLAT_H__

#if (defined(CONFIG_TARGET_NA51090) || defined(CONFIG_TARGET_NA51090_A64))
#include "nvt_ivot_mmc_na51090.h"
#endif

#if (defined(CONFIG_TARGET_NA51102) || defined(CONFIG_TARGET_NA51102_A64))
#include "nvt_ivot_mmc_na51102.h"
#endif

#if (defined(CONFIG_TARGET_NA51103) || defined(CONFIG_TARGET_NA51103_A64))
#include "nvt_ivot_mmc_na51103.h"
#endif

#endif
