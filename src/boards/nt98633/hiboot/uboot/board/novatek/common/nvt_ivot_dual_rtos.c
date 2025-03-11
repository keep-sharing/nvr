
#include <common.h>
#include <fs.h>
#include <asm/io.h>
#include <nand.h>
#include <mmc.h>
#include <spi_flash.h>
#include <lzma/LzmaTools.h>
#include <linux/libfdt.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/modelext/emb_partition_info.h>
#include <asm/nvt-common/modelext/bin_info.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/nvt-common/shm_info.h>
#include <asm/nvt-common/nvt_ivot_optee.h>
#include <asm/nvt-common/nvt_ivot_efuse_smc.h>
#include <asm/arch/crypto.h>
#include <asm/arch/IOAddress.h>
#include <compiler.h>
#include <u-boot/zlib.h>
#include <stdlib.h>
#include "nvt_ivot_soc_utils.h"
#include "nvt_ivot_pack.h"
#include <asm/nvt-common/nvt_ivot_optee.h>
#include <asm/nvt-common/nvt_ivot_aes_smc.h>
#include <malloc.h>

#define CFG_FULLY_CHECK 1 //0: only check bank info valid, 1: check fw checksum and bank info valid

//sync from rtos's flow_updfw.h
typedef struct _VERSION_CTRL {
	unsigned int fourcc;        // MAKEFOURCC 'V','E','R','C'
	unsigned int bank_id;       // 0 or 1 indicate uitron[0] or uitron[1]
	unsigned int fw_type;       // 0: debug, 1: release
	unsigned int fw_version;    // major: 1byte, minor 2byte
	unsigned int fw_date;       // date version
	unsigned int update_cnt;    // this device total upgrade counts
	unsigned char hash[1024];   // must 4 bytes alignment
	unsigned char reserved[64]; // must 4 bytes alignment
} VERSION_CTRL;

// index for BININFO:: Resv[64], //sync from rtos's prj_cfg.h
#define BININFO_RESV_IDX_TAG            0x0
#define BININFO_RESV_IDX_VER            0x1
#define BININFO_RESV_IDX_RELEASE_BUILD  0x2
#define BININFO_RESV_IDX_BOOT_BANK_FROM 0x3
#define BININFO_RESV_IDX_DATE_SEC       0x4


static UINT32 MemCheck_CalcCheckSum16Bit(UINT32 uiAddr, UINT32 uiLen)
{
	UINT32 i, uiSum = 0;
	UINT16 *puiValue = (UINT16 *)uiAddr;

	for (i = 0; i < (uiLen >> 1); i++)  {
		uiSum += (*(puiValue + i) + i);
	}
	uiSum &= 0x0000FFFF;
	return uiSum;
}

static int nvt_getfdt_rtos_addr_size(ulong addr, ulong *fdt_rtos_addr, ulong *fdt_rtos_len)
{
	int len;
	int nodeoffset; /* next node offset from libfdt */
	const u32 *nodep; /* property node pointer */


	nodeoffset = fdt_path_offset((const void *)addr, "/nvt_memory_cfg/rtos");
	if (nodeoffset < 0) {
		*fdt_rtos_addr = 0;
		*fdt_rtos_len = 0;
		return -1;
	}

	nodep = fdt_getprop((const void *)addr, nodeoffset, "reg", &len);
	if (len == 0) {
		*fdt_rtos_addr = 0;
		*fdt_rtos_len = 0;
		return -1;
	}

	*fdt_rtos_addr = be32_to_cpu(nodep[0]);
	*fdt_rtos_len = be32_to_cpu(nodep[1]);
	return 0;
}

int nvt_decide_dual_rtos_idx(void)
{
	//use static for big structure memory
	static VERSION_CTRL ver_ctrl[2] = {0};

	int i, j;
	unsigned int tmp_addr = 0;
	unsigned int rtos_ver[2] = {0};
	unsigned int rtos_date[2] = {0};
	unsigned int map_index[2] = {0, 1}; //search
	const char *part_names[2] = {"rtos", "rtos1"};
	const char *bank_name[2] = {"bank", "bank1"};

	char cmd[128] = {0};

	int len = 0;
	int nodeoffset; /* next node offset from libfdt */
	const char *nodep; /* property node pointer */

	nodeoffset = fdt_path_offset((const void *)nvt_fdt_buffer, "/nvt_info");
	if (nodeoffset < 0) {
		// unsupported dual rtos
		return 0;
	}

	nodep = fdt_getprop((const void *)nvt_fdt_buffer, nodeoffset, "NVT_DUAL_RTOS", &len);
	if (nodep == NULL || len == 0) {
		// unsupported dual rtos
		return 0;
	}

	if (strcmp(nodep, "NVT_DUAL_RTOS_ON") != 0) {
		return 0;
	}

	printf("dual rtos supported.\n");


	SHMINFO *p_shminfo = (SHMINFO *)CONFIG_SMEM_SDRAM_BASE;
	if (p_shminfo->boot.LdCtrl2 & LDCF_UPDATE_FW) {
		printf("fw updated from u-boot, force rtos boot from bank[0].\r\n");
		return 0;
	}

	ulong rtos_addr = 0, rtos_size = 0;
	nvt_getfdt_rtos_addr_size((ulong)nvt_fdt_buffer, &rtos_addr, &rtos_size);
	//read uenv for VERSION_CTRL blocks
	loff_t part_off = 0, part_size = 0;
#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT)
	for (i = 0; i < 2; i++) {
		sprintf(cmd, "nand read 0x%x %s 0x%x ", rtos_addr, bank_name[i], _EMBMEM_BLK_SIZE_);
		run_command(cmd, 0);
		memcpy(&ver_ctrl[i], (unsigned char *)rtos_addr, sizeof(VERSION_CTRL));
	}
#elif defined(CONFIG_NVT_LINUX_SPINOR_BOOT)
	for (i = 0; i < 2; i++) {
		get_part(bank_name[i], &part_off, &part_size);
		sprintf(cmd, "sf read 0x%x 0x%llx 0x%x", rtos_addr, part_off, part_size);
		run_command(cmd, 0);
		memcpy(&ver_ctrl[i], (unsigned char *)rtos_addr, sizeof(VERSION_CTRL));
	}
#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT)
	for (i = 0; i < 2; i++) {
		get_part(bank_name[i], &part_off, &part_size);
		sprintf(cmd, "mmc read 0x%x 0x%llx 0x%x", rtos_addr, part_off, part_size);
		run_command(cmd, 0);
		memcpy(&ver_ctrl[i], (unsigned char *)rtos_addr, sizeof(VERSION_CTRL));
	}
#endif /* CONFIG_NVT_LINUX_SPINAND_BOOT */

	if (ver_ctrl[0].fourcc != MAKEFOURCC('V', 'E', 'R', 'C') && ver_ctrl[1].fourcc != MAKEFOURCC('V', 'E', 'R', 'C')) {
		printf("factory version, force rtos boot from bank[0].\r\n");
		return 0;
	} else if (ver_ctrl[0].fourcc == MAKEFOURCC('V', 'E', 'R', 'C') && ver_ctrl[1].fourcc == MAKEFOURCC('V', 'E', 'R', 'C')) {
		if (ver_ctrl[1].update_cnt > ver_ctrl[0].update_cnt) {
			map_index[0] = 1;
			map_index[1] = 0;
		} else {
			map_index[0] = 0;
			map_index[1] = 1;
		}
	} else if (ver_ctrl[0].fourcc == MAKEFOURCC('V', 'E', 'R', 'C') && ver_ctrl[1].fourcc != MAKEFOURCC('V', 'E', 'R', 'C')) {
		map_index[0] = 0;
		map_index[1] = 1;
	} else if (ver_ctrl[0].fourcc != MAKEFOURCC('V', 'E', 'R', 'C') && ver_ctrl[1].fourcc == MAKEFOURCC('V', 'E', 'R', 'C')) {
		map_index[0] = 1;
		map_index[1] = 0;
	}

	printf("boot check sequence = {%d, %d}.\r\n", map_index[0], map_index[1]);

	for (j = 0; j < 2; j++) {
		i = map_index[j];
		u32 align_size = 0;
		get_part(part_names[i], &part_off, &part_size);

		unsigned int test_preload_size = 0x200;
#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT)
		sprintf(cmd, "nand read 0x%x %s 0x%x ", rtos_addr, part_names[i], test_preload_size);
#elif defined(CONFIG_NVT_LINUX_SPINOR_BOOT)
		sprintf(cmd, "sf read 0x%x 0x%llx 0x%x ", rtos_addr, part_off, test_preload_size);
#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT)
		sprintf(cmd, "mmc read 0x%x 0x%llx 0x4", rtos_addr, part_off);
#endif /* CONFIG_NVT_LINUX_SPINAND_BOOT */
		printf("=>run cmd : %s\n", cmd);
		run_command(cmd, 0);

		NVTPACK_BFC_HDR *pbfc = (NVTPACK_BFC_HDR *)rtos_addr;
		/* Boot compressed rtos or non-compressed rtos */
		if (pbfc->uiFourCC == MAKEFOURCC('B', 'C', 'L', '1')) {
			size_t firmware_size = cpu_to_be32(pbfc->uiSizeComp) + sizeof(NVTPACK_BFC_HDR);
			tmp_addr = ALIGN_FLOOR(rtos_addr + rtos_size - firmware_size, 4);

			if (firmware_size > rtos_size) {
				printf("rtos[%d] size larger than memory partition%d, skip it\n", i, firmware_size);
				continue;
			}
#if (CFG_FULLY_CHECK)
#if defined(CONFIG_NVT_LINUX_SPINAND_BOOT)
			sprintf(cmd, "nand read 0x%x %s 0x%x ", tmp_addr, part_names[i], firmware_size);
#elif defined(CONFIG_NVT_LINUX_SPINOR_BOOT)
			align_size = ALIGN_CEIL(firmware_size, ARCH_DMA_MINALIGN);
			sprintf(cmd, "sf read 0x%x 0x%llx 0x%x", tmp_addr, part_off, align_size);
#elif defined(CONFIG_NVT_LINUX_EMMC_BOOT)
			align_size = ALIGN_CEIL(firmware_size, MMC_MAX_BLOCK_LEN) / MMC_MAX_BLOCK_LEN;
			sprintf(cmd, "mmc read 0x%x 0x%llx 0x%x", tmp_addr, part_off, align_size);
#endif /* CONFIG_NVT_LINUX_SPINAND_BOOT */
			run_command(cmd, 0);

			if (MemCheck_CalcCheckSum16Bit((UINT32) tmp_addr, firmware_size) != 0) {
				printf("rtos[%d] chksum failed, skip it\n", i);
				memset(&ver_ctrl[i], 0, sizeof(VERSION_CTRL));
				continue;
			}

			//unlzma some to get version
			size_t Compsz = cpu_to_be32(pbfc->uiSizeComp);
			size_t Uncompsz = cpu_to_be32(pbfc->uiSizeUnComp);
			if ((cpu_to_be32(pbfc->uiAlgorithm) & 0xFF) == 11) {
				lzmaBuffToBuffDecompress((unsigned char *)rtos_addr, &Uncompsz, (unsigned char *)(tmp_addr + sizeof(NVTPACK_BFC_HDR)), Compsz);
			} else {
				const int unlz_preload_size = ALIGN_CEIL_4(BIN_INFO_OFFSET_RTOS + sizeof(BININFO));
				const int unlz_tmp_size = ALIGN_CEIL_4(128 * 1024); //128KB enough
				lz_uncompress((unsigned char *)(tmp_addr + sizeof(NVTPACK_BFC_HDR)), (unsigned char *)rtos_addr, unlz_tmp_size);
			}
			BININFO *p_bininfo = (BININFO *)(rtos_addr + BIN_INFO_OFFSET_RTOS);
			rtos_ver[i] = p_bininfo->Resv[BININFO_RESV_IDX_VER];
			printf("rtos_ver[%d] %08X\n", i, rtos_ver[i]);
			rtos_date[i] = p_bininfo->Resv[BININFO_RESV_IDX_DATE_SEC];
			printf("rtos_date[%d] %u\n", i, rtos_date[i]);
#else //!(CFG_FULLY_CHECK)
			rtos_ver[i] = ver_ctrl[i].fw_version;
			rtos_date[i] = ver_ctrl[i].fw_date;
#endif
			printf("rtos_cnt[%d] %u\n", i, ver_ctrl[i].update_cnt);
			if (ver_ctrl[i].fourcc != MAKEFOURCC('V', 'E', 'R', 'C')) {
				printf("rtos[%d] bank info is empty or invalid fourcc.\n", i);
				memset(&ver_ctrl[i], 0, sizeof(VERSION_CTRL));
				continue;
			} else if (ver_ctrl[i].fw_version != rtos_ver[i]) {
				printf("rtos[%d] version not match env(%08X) v.s. rtos(%08X), skip it\n", i, ver_ctrl[i].fw_version, rtos_ver[i]);
				memset(&ver_ctrl[i], 0, sizeof(VERSION_CTRL));
				continue;
			} else if (ver_ctrl[i].fw_date != rtos_date[i]) {
				printf("rtos[%d] dat_ver not match env(%08X) v.s. rtos(%08X), skip it\n", i, ver_ctrl[i].fw_date, rtos_date[i]);
				memset(&ver_ctrl[i], 0, sizeof(VERSION_CTRL));
				continue;
			}
			break; // if 1st FW (high priority sorted) is ok, no need to check 2nd FW
		} else {
			printf("rtos[%d] not bfc, skip it\n", i);
			memset(&ver_ctrl[i], 0, sizeof(VERSION_CTRL));
		}
	}

	// check if boot from bank[1]
	if (ver_ctrl[1].fourcc == MAKEFOURCC('V', 'E', 'R', 'C') &&
		ver_ctrl[1].update_cnt > ver_ctrl[0].update_cnt) {
		printf("boot from bank[%d]\r\n", 1);
		return 1;
	}

	if (ver_ctrl[0].fourcc != MAKEFOURCC('V', 'E', 'R', 'C')) {
		printf("boot from bank[%d], but bank info is empty or invalid\r\n", 0);
		return 0;
	}

	printf("boot from bank[0]\n");
	return 0;
}

void nvt_update_dual_rtos_info(void *p_bininfo, int boot_bank)
{
	BININFO *pbininfo = (BININFO *)p_bininfo;
	// BININFO_RESV_IDX_TAG 0
	if (pbininfo->Resv[BININFO_RESV_IDX_TAG] == MAKEFOURCC('D', 'R', 'T', 'S')) {
		pbininfo->Resv[BININFO_RESV_IDX_BOOT_BANK_FROM] = boot_bank;
		flush_dcache_range((ulong)pbininfo, (ulong)pbininfo + sizeof(BININFO));
	}
}
