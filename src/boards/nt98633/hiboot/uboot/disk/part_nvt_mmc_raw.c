// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Wayne Lin (Wayne_lin@novatek.com.tw)
 */

#include <common.h>
#include <command.h>
#include <asm/unaligned.h>
#include "part_nvt_mmc_raw.h"
#include <asm/nvt-common/nvt_common.h>
#include <memalign.h>


#ifdef CONFIG_HAVE_BLOCK_DEVICE

static int nvt_mmc_raw_emmc_part_parsing(uchar *name, unsigned long *start, unsigned long *size, int part_num)
{
	int len, len_s, i;
	char *emmcpart = env_get("nvtemmcpart");
	char *s;
	char buf[40];

	if (emmcpart == NULL) {
		return -1;
	}

	/* Find the head of the partition string (ex. 0x40000@0x80200(atf)ro) */
	for (i = 0; i < part_num; i++) {
		s = strstr(emmcpart,",");
		if(s == NULL) {
			return -1;
		}
		emmcpart = s + 1;
	}

	/* Get partition size (ex. 0x40000) */
	s = strstr(emmcpart,"@");
	if(s == NULL) {
		return -1;
	}

	len = strlen(emmcpart);
	len_s = strlen(s);
	strncpy(buf, emmcpart, len - len_s);
	*size = simple_strtoull(buf, NULL, 16);
	emmcpart = s + 1;
	memset(buf, 0, sizeof(buf));

	/* Get partition start address (ex. 0x80200) */
	s = strstr(emmcpart,"(");
	if(s == NULL) {
		return -1;
	}

	len = strlen(emmcpart);
	len_s = strlen(s);
	strncpy(buf, emmcpart, len - len_s);
	*start = simple_strtoull(buf, NULL, 16);
	emmcpart = s + 1;
	memset(buf, 0, sizeof(buf));

	/* Get partition name (ex. atf) */
	s = strstr(emmcpart,")");
	if(s == NULL) {
		return -1;
	}

	memset(name, 0, sizeof(name));
	len = strlen(emmcpart);
	len_s = strlen(s);
	strncpy((char *)name, emmcpart, len - len_s);

	return 0;
}

/* only boot records will be listed as valid partitions */
static int nvt_mmc_raw_part_get_info(struct blk_desc *dev_desc, int part_num,
			   disk_partition_t *info)
{
#ifdef CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT
	EMB_PARTITION *pEmb = emb_partition_info_data_curr;
	char buf[40];
#endif

	if (dev_desc->devnum != CONFIG_NVT_IVOT_EMMC)
		return -1;

	info->blksz = dev_desc->blksz;
	strcpy((char *)info->type, "NVT_MMC_RAW");
	if (env_get("nvtemmcpart")) {

		if (nvt_mmc_raw_emmc_part_parsing(info->name, &info->start, &info->size, part_num) < 0)
			return -1;

		info->size = info->size / info->blksz;
		info->start = info->start / info->blksz;

#ifdef CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT
	} else if (pEmb != NULL) {
		const EMB_PARTITION *p = pEmb + part_num;
		if (part_num >= EMB_PARTITION_INFO_COUNT)
			return -1;
		switch (p->EmbType) {
			case EMBTYPE_LOADER:
				strcpy((char *)info->name, "loader");
				break;
			case EMBTYPE_FDT:
				strcpy((char *)info->name, "fdt");
				break;
			case EMBTYPE_ATF:
				strcpy((char *)info->name, "atf");
				break;
			case EMBTYPE_TEEOS:
				strcpy((char *)info->name, "teeos");
				break;
			case EMBTYPE_UBOOT:
				strcpy((char *)info->name, "uboot");
				break;
			case EMBTYPE_UENV:
				strcpy((char *)info->name, "uenv");
				break;
			case EMBTYPE_LINUX:
				strcpy((char *)info->name, "linux");
				break;
			case EMBTYPE_ROOTFS:
				sprintf(buf, "rootfs%d", p->OrderIdx);
				strcpy((char *)info->name, buf);
				break;
			default:
				strcpy((char *)info->name, "unknow");
				break;

		}	
		info->size = p->PartitionSize / info->blksz;
		info->start = p->PartitionOffset / info->blksz;
#endif
	} else {
		printf("Partition table is empty\n");
		return -1;
	}
	return 0;
}

static void nvt_mmc_raw_part_print(struct blk_desc *dev_desc)
{
	int part_num = 0;
	disk_partition_t info;
	while (nvt_mmc_raw_part_get_info(dev_desc, part_num, &info) == 0)
	{
		if (strncmp((const char *)info.name, "unknow", 6))
			printf("partition name[%d]:%-15s sector:%ld@%-10ld\n", part_num, info.name, info.start, info.size);
		part_num++;
	}
}

static int nvt_mmc_raw_part_test(struct blk_desc *dev_desc)
{
	disk_partition_t info;

	return nvt_mmc_raw_part_get_info(dev_desc, 1, &info);
}

U_BOOT_PART_TYPE(nvt_mmc_raw) = {
	.name		= "NVT_MMC_RAW",
	.part_type	= PART_TYPE_NVT_MMC_RAW,
	.max_entries	= MAX_ENTRY_NUMBERS,
	.get_info	= nvt_mmc_raw_part_get_info,
	.print		= nvt_mmc_raw_part_print,
	.test		= nvt_mmc_raw_part_test,
};
#endif
