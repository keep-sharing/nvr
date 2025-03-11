/*
 * (C) Copyright 2005
 * 2N Telekomunikace, a.s. <www.2n.cz>
 * Ladislav Michl <michl@2n.cz>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <nand.h>
#include <linux/list.h>
#include <malloc.h>

#ifndef CONFIG_SYS_NAND_BASE_LIST
#define CONFIG_SYS_NAND_BASE_LIST { CONFIG_SYS_NAND_BASE }
#endif

DECLARE_GLOBAL_DATA_PTR;

int nand_curr_device = -1;
nand_info_t nand_info[CONFIG_SYS_MAX_NAND_DEVICE];

static struct nand_chip nand_chip[CONFIG_SYS_MAX_NAND_DEVICE];
static ulong base_address[CONFIG_SYS_MAX_NAND_DEVICE] = CONFIG_SYS_NAND_BASE_LIST;

static const char default_nand_name[] = "nand";
static __attribute__((unused)) char dev_name[CONFIG_SYS_MAX_NAND_DEVICE][8];

static void nand_init_chip(struct mtd_info *mtd, struct nand_chip *nand,
			   ulong base_addr, char *flash_type)
{
	int maxchips = CONFIG_SYS_NAND_MAX_CHIPS;
	int __attribute__((unused)) i = 0;

	if (maxchips < 1)
		maxchips = 1;
	mtd->priv = nand;

	nand->IO_ADDR_R = nand->IO_ADDR_W = (void  __iomem *)base_addr;
	if (board_nand_init(nand) == 0) {
		if (nand_scan(mtd, maxchips) == 0) {
			if (!mtd->name)
				mtd->name = (char *)default_nand_name;
#ifndef CONFIG_RELOC_FIXUP_WORKS
			else
				mtd->name += gd->reloc_off;
#endif

#ifdef CONFIG_MTD_DEVICE
			/*
			 * Add MTD device so that we can reference it later
			 * via the mtdcore infrastructure (e.g. ubi).
			 */
			strncpy(flash_type, mtd->name, strlen(mtd->name)+1);
			sprintf(dev_name[i], "nand%d", i);
			mtd->name = dev_name[i++];
			add_mtd_device(mtd);
#endif
		} else
			mtd->name = NULL;
	} else {
		mtd->name = NULL;
		mtd->size = 0;
	}

}

#define PARTITION_PARAM "baseparam"
#define PARTITION_LOGO  "logo"
#define PARTITION_KER1  "ker1"
#define PARTITION_KER2  "ker2"
#define PARTITION_FS1   "fs1"
#define PARTITION_FS2   "fs2"
#define PARTITION_CFG   "cfg"
#define PARTITION_OEM   "oem"
#define PARTITION_RSD   "rsd"

typedef struct {
    char name[20];      /* 分区名 */
    short startBlock;   /* 分区块起始序号 */
    short endBlock;     /* 分区块终止序号 */
    struct list_head list;
}MS_Partition;

MS_Partition partCtrl;
static unsigned int badnums;    /* 坏块数 */
static unsigned char partNumber;/* 分区数 */

static int atoi(char *string)
{
    int length;
    int retval = 0;
    int i;
    int sign = 1;

    length = strlen(string);
    for (i = 0; i < length; i++) {
        if (0 == i && string[0] == '-') {
            sign = -1;
            continue;
        }
        if (string[i] > '9' || string[i] < '0') {
            break;
        }
        retval *= 10;
        retval += string[i] - '0';
   }
   retval *= sign;
   return retval;
}

static int get_index_from_list(short dst, unsigned short *list)
{
    int indexLeft, indexRight, indexMid;

    for(indexLeft=0, indexRight=badnums;;) {
        indexMid = (indexLeft + indexRight) >> 1;
        if(dst < list[indexMid]) {
            indexRight = indexMid;
        }else {
            indexLeft = indexMid;
        }
        if(indexRight - indexLeft < 2) {
            break;
        }
    }
    return indexLeft;
}

/* 获取分区坏块数. */
static int get_bad_count_by_partition(unsigned short *badList, const char *partition)
{
   MS_Partition *pos;
   int startIndex, endIndex;
   int badCnt = 0;

    list_for_each_entry(pos, &(partCtrl.list), list) {
        if(strstr(pos->name, partition) != NULL) {    /* match the partition. */
            /* badList scan. */
            if((pos->endBlock < badList[0]) || (pos->startBlock > badList[badnums - 1])) {
                return 0;
            }
            startIndex = get_index_from_list(pos->startBlock, badList);
            startIndex = badList[startIndex] < pos->startBlock ? startIndex + 1 : startIndex;
            endIndex = get_index_from_list(pos->endBlock, badList);
            endIndex = badList[endIndex] > pos->endBlock ? endIndex - 1 : endIndex;
            //printf("name = %s[%d, %d] startIndex=%d endStart=%d\n", pos->name, pos->startBlock, pos->endBlock, startIndex, endIndex);
            badCnt = endIndex - startIndex + 1;
        }
    }
    return badCnt;
}

/* 获取分区总块数. */
static int get_block_count_by_partition(char *partition)
{
    MS_Partition *pos;

    list_for_each_entry(pos, &(partCtrl.list), list) {
        if(strstr(pos->name, partition) != NULL) {    /* match the partition. */
            return (pos->endBlock - pos->startBlock + 1);
        }
    }
    return 0;
}

static void free_partition_info(void)
{
    MS_Partition *pos;

    list_for_each_entry(pos, &(partCtrl.list), list) {
		free(pos);
    }
    return ;
}

/* 解析分区环境变量信息. */
static void parse_partition(const char *rowData)
{
    int i, j, partSize, blockNum;
    char tmp[20];
    char partList[20][20];
    int blockOff = 0;
    char *partInfo;
    char *partTmp;

    partInfo = malloc(strlen(rowData));
    if(!partInfo) {
        printf("malloc failed!\n");
        return ;
    }

    strcpy(partInfo, rowData);
    partTmp = strstr(partInfo, ":");
    char* token = strtok(partTmp + 1, ",");
    while(token != NULL) {
        strcpy(partList[partNumber++], token);
        token = strtok( NULL, ",");
    }

    for(i=0; i<partNumber; i++) {
        MS_Partition *newPart = malloc(sizeof(MS_Partition));
        strcpy(tmp, partList[i]);
        strcpy(newPart->name, tmp);
        for(j=0; j<strlen(tmp); j++) {
            if(tmp[j] == '(') {
                break;
            }
        }
        switch(tmp[j - 1]) {
        case 'G':
        case 'g': {
            tmp[j - 1] = '\0';
            partSize = atoi(tmp) << 20;
        }break;
        case 'M':
        case 'm': {
            tmp[j - 1] = '\0';
            partSize = atoi(tmp) << 10;
        }break;
        case 'K':
        case 'k':
        default:{
            tmp[j - 1] = '\0';
            partSize = atoi(tmp);
        }break;
        }

        blockNum = partSize >> 7;  /* 分区块数 */
        newPart->startBlock = blockOff;
        newPart->endBlock = newPart->startBlock + (blockNum - 1);
        blockOff += blockNum;   /* 记录块序号偏移 */
        list_add(&newPart->list, &partCtrl.list);
        //printf("parse partitions[%d]=> name:%s  start block:%d  end block:%d\n", i, newPart->name, newPart->startBlock, newPart->endBlock);
    }

    if(partInfo) {
        free(partInfo);
    }
}

/* 坏块分析. */
void ms_bad_block_analysis(short badNum, unsigned short *badList, short blockNum)
{
    char *partString;
    int badCntCfg, badCntOem, totalBlock, retInteger, retDecimal;

    if(badNum == 0 || !badList) {
		printf("Total bad blocks: %d\n", badNum);
        return ;
    }

    /* 1.获取分区表 */
    partString = getenv("mtdparts");
    if(!partString) {
        printf("getenv failed!\n");
        return ;
    }

    INIT_LIST_HEAD(&(partCtrl.list));
    parse_partition(partString);   /* 解析分区表 */

    /* 2.1: 坏块的总数不超过flash规格总块数的2% */
    retInteger = (badNum * 100) / blockNum;
    retDecimal = (badNum * 1000) / blockNum - retInteger * 10;
    printf("Total bad blocks: %d\n", badnums);
    printf("Flash bad blocks percentage: %d.%d%%\n", retInteger, retDecimal);

    /* 2.2: cfg&oem分区坏块总数不应超过总块数的1% */
    badCntCfg = get_bad_count_by_partition(badList, PARTITION_CFG);
    badCntOem = get_bad_count_by_partition(badList, PARTITION_OEM);
    totalBlock = get_block_count_by_partition(PARTITION_CFG);
    totalBlock += get_block_count_by_partition(PARTITION_OEM);
    if(totalBlock != 0) {
        retInteger = ((badCntCfg + badCntOem) * 100) / totalBlock;
        retDecimal = ((badCntCfg + badCntOem) * 1000) / totalBlock - retInteger * 10;
        printf("CFG & OEM partition bad blocks percentage: %d.%d%%\n", retInteger, retDecimal);
    }

    free_partition_info();

    return ;
}

void nand_init(char *flash_type)
{
	int i;
	unsigned int size = 0;
	loff_t off;
	nand_info_t *nand;
    unsigned short *badList;

	for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++) {
		nand_init_chip(&nand_info[i], &nand_chip[i], base_address[i], flash_type);
		size += nand_info[i].size >> 10;
		if (nand_curr_device == -1)
			nand_curr_device = i;
	}

	//---------------------- bad block statistic.solin 180720 ----------------
	/* the following commands operate on the current device */
	if (nand_curr_device < 0
		|| nand_curr_device >= CONFIG_SYS_MAX_NAND_DEVICE
		|| !nand_info[nand_curr_device].name) {
		printf("\nno devices available\n");
	}else
	{
		nand = &nand_info[nand_curr_device];
        badList = malloc(size >> 7);   /* 初始化坏块列表(保存坏块序号) */
		printf("\nNVR Device %d bad blocks start-\n", nand_curr_device);
        for (off = 0, i = 0; off < nand->size; off += nand->erasesize, i++) {
            if (nand_block_isbad(nand, off)) {
                printf("  %d-%08llx\n", i, off);
                badList[badnums++] = i;
            }
        }
        printf("NVR Bad blocks count:%d\nNVR Device %d bad blocks end-\n", badnums, nand_curr_device);
        ms_bad_block_analysis(badnums, badList, size >> 7);
        free(badList);
	}
	//--------------------------------------------------------

	if (size)
#ifdef CONFIG_HIFMC_SPI_NAND
		printf("SPI Nand total size: %uMB\n", size >> 10);
#else
		printf("Nand total size: %uMB\n", size >> 10);
#endif

#ifdef CONFIG_SYS_NAND_SELECT_DEVICE
	/*
	 * Select the chip in the board/cpu specific driver
	 */
	board_nand_select_device(nand_info[nand_curr_device].priv, nand_curr_device);
#endif
}
