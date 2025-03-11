#include <common.h>
#include <command.h>
#include <net.h>
#include <linux/ctype.h>    /* isalpha, isdigit */
#include <mmc.h>
#include <sparse_format.h>
#include <image-sparse.h>


#define SIZE_REMAINING      0xFFFFFFFF
#define OFFSET_NOT_SPECIFIED    0xFFFFFFFF
#define MIN_PART_SIZE       128
#define MAX_BLK_PART    16

static int curr_device = 1;


struct blk_ctrl {
    struct blkpart_info part[MAX_BLK_PART];
    u32 nums;               //实际分区个数，小于等于MAX_BLK_PART
    u32 totalSize;          //count的个数
};

static struct blk_ctrl g_stblk = {0};

static u64 memsize_parse(const char *const ptr, const char **retptr)
{
    u64 ret = simple_strtoull(ptr, (char **)retptr, 0);

    switch (**retptr) {
        case 'G':
        case 'g':
            ret <<= 10;
        case 'M':
        case 'm':
            ret <<= 10;
        case 'K':
        case 'k':
            ret <<= 10;
            ret >>= 9;      //emmc block is 512byte
            (*retptr)++;
        default:
            break;
    }

    return ret;
}

static int part_parse(const char *const partdef, const char **ret, struct blkpart_info *blkpart)
{
    u64 size;
    u64 offset;
    const char *name;
    int name_len;
    const char *p;

    p = partdef;
    *ret = NULL;

    /* fetch the partition size */
    if (*p == '-') {
        /* assign all remaining space to this partition */
        debug("'-': remaining size assigned\n");
        size = SIZE_REMAINING;
        p++;
    } else {
        size = memsize_parse(p, &p);
        if (size < MIN_PART_SIZE) {
            printf("partition size too small (%llx)\n", size);
            return 1;
        }
    }

    /* check for offset */
    offset = OFFSET_NOT_SPECIFIED;
    if (*p == '@') {
        p++;
        offset = memsize_parse(p, &p);
    }

    /* now look for the name */
    if (*p == '(') {
        name = ++p;
        if ((p = strchr(name, ')')) == NULL) {
            printf("no closing ) found in partition name\n");
            return 1;
        }
        name_len = p - name + 1;
        if ((name_len - 1) == 0) {
            printf("empty partition name\n");
            return 1;
        }
        p++;
    } else {
        /* 0x00000000@0x00000000 */
        name_len = 22;
        name = NULL;
    }

    /* check for next partition definition */
    if (*p == ',') {
        if (size == SIZE_REMAINING) {
            *ret = NULL;
            printf("no partitions allowed after a fill-up partition\n");
            return 1;
        }
        *ret = ++p;
    } else if ((*p == ';') || (*p == '\0')) {
        *ret = p;
    } else {
        printf("unexpected character '%c' at the end of partition\n", *p);
        *ret = NULL;
        return 1;
    }

    /*  allocate memory */
    memset(blkpart, 0, sizeof(struct blkpart_info));
    blkpart->size = size;
    blkpart->offset = offset;
    if (name) {
        /* copy user provided name */
        strncpy(blkpart->name, name, name_len - 1);
    } else {
        /* auto generated name in form of size@offset */
        printf("unexpected name\n");
        return 1;
    }
    blkpart->name[name_len - 1] = '\0';

    return 0;
}

int find_blk_part(const char *id, struct blkpart_info *part)
{
    struct blk_ctrl *pstBlk = &g_stblk;
    int i = 0;
    char flag = 0;

    for (i = 0; i < pstBlk->nums; i++) {
        if (!strcmp(id, pstBlk->part[i].name)) {
            flag = 1;
            break;
        }
    }

    if (!flag) {
        printf("not find partname=%s\n", id);
        return 1;
    }

    memcpy(part, &pstBlk->part[i], sizeof(struct blkpart_info));

    return 0;
}

static struct mmc *init_mmc_device(int dev, bool force_init)
{
    struct mmc *mmc;
    mmc = find_mmc_device(dev);
    if (!mmc) {
        printf("no mmc device at slot %x\n", dev);
        return NULL;
    }

    if (!mmc_getcd(mmc)) {
        force_init = true;
    }

    if (force_init) {
        mmc->has_init = 0;
    }
    if (mmc_init(mmc)) {
        return NULL;
    }

#ifdef CONFIG_BLOCK_CACHE
    struct blk_desc *bd = mmc_get_blk_desc(mmc);
    blkcache_invalidate(bd->if_type, bd->devnum);
#endif

    return mmc;
}

static int mmc_blk_init(struct blk_ctrl *pstBlk)
{
    struct mmc *mmc;

    if (curr_device < 0) {
        if (get_mmc_num() > 0) {
            curr_device = 0;
        } else {
            puts("No MMC device available\n");
            return 1;
        }
    }

    mmc = init_mmc_device(curr_device, false);
    if (!mmc) {
	printf("init mmc dev %d fail!\n", curr_device);
        return CMD_RET_FAILURE;
    }

    pstBlk->totalSize = mmc->capacity >> 9;

    printf("Capacity: %lld (%dMB), count=%08x\r\n", mmc->capacity, mmc->capacity >> 20, pstBlk->totalSize);

    return CMD_RET_SUCCESS;
}


int blkparts_init(void)
{
    struct blk_ctrl *pstBlk = &g_stblk;
    const char *parts, *p;
    const char *mtd_id;
    int err = 1;
    int offset = 0;
    int i = 0;

    memset(pstBlk, 0, sizeof(struct blk_ctrl));

    mmc_blk_init(pstBlk);

    parts = env_get("blkdevparts");

    p = parts;
    if (strncmp(p, "blkdevparts=", 12) != 0) {
        printf("blkparts variable doesn't start with 'blkparts='\n");
        return err;
    }
    p += 12;

    /* fetch <mtd-id> */
    mtd_id = p;
    if (!(p = strchr(mtd_id, ':'))) {
        printf("no <mtd-id> identifier\n");
        return 1;
    }
    p++;

    //初始化

    i = 0;
    offset = 0;
    //解析blkparts信息
    while (p && (*p != '\0') && (*p != ';')) {
        err = 1;

        if ((part_parse(p, &p, &pstBlk->part[i]) != 0)) {
            break;
        }

        pstBlk->part[i].offset = offset;
        if (pstBlk->part[i].size == SIZE_REMAINING) {
            pstBlk->part[i].size = pstBlk->totalSize - pstBlk->part[i].offset;
        }
        offset += pstBlk->part[i].size;

        i++;

        err = 0;
    }

    if (err != 0) {
        return err;
    }

    if (i > 0) {
        pstBlk->nums = i;
    } else {
        printf("no partitions for device\n");
        return 1;
    }

    return 0;
}

static void list_partitions(void)
{
    int i = 0;
    struct blk_ctrl *pstBlk = &g_stblk;

    printf("\n---list_partitions---\n");
    printf(" #: name\t\toffset\t\tsize\n");

    for (i = 0; i < pstBlk->nums; i++) {
        printf("%2d: %-20s0x%08x\t0x%08x\n",
               i, pstBlk->part[i].name, pstBlk->part[i].offset, pstBlk->part[i].size);
    }
}

int do_blkparts(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
    if (blkparts_init() != 0) {
        printf("blkparts_init failure\n");
        return 1;
    }

    list_partitions();

    return 0;
}

U_BOOT_CMD(
    blkparts, 1, 0, do_blkparts,
    "show emmc partions table",
    ""
);

