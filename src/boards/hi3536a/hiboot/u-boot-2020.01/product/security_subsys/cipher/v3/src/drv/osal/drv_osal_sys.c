// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include <linux/compat.h>
#include "drv_osal_lib.h"

/* under TEE, we only can malloc secure mmz at system steup,
 * then map the mmz to Smmu, but the smmu can't map to cpu address,
 * so we must save the cpu address in a static table when malloc and map mmz.
 * when call crypto_mem_map, we try to query the table to get cpu address firstly,
 * if can't get cpu address from the table, then call system api to map it.
 */
#define CRYPTO_MEM_MAP_TABLE_DEPTH      32

typedef struct {
    td_bool valid;
    compat_addr dma;
    td_void *via;
} crypto_mem_map_table;

static crypto_mem_map_table g_local_map_table[CRYPTO_MEM_MAP_TABLE_DEPTH];

/******************************* API Code *****************************/
/*****************************************************************
 *                       mmz/mmu api                             *
 *****************************************************************/
/* Implementation that should never be optimized out by the compiler */


/*
 * brief allocate and map a mmz or smmu memory
 * we can't allocate smmu directly during TEE boot period.
 * in addition, the buffer of cipher node list must be mmz.
 * so here we have to allocate a mmz memory then map to smmu if necessary.
 */
static td_s32 crypto_mem_alloc_remap(crypto_mem *mem, td_u32 type, const td_char *name, td_u32 size)
{
    td_s32 i;

    chk_ptr_err_return(mem);
    crypto_unused(type);
    crypto_unused(name);

    (td_void)memset_s(mem, sizeof(crypto_mem), 0, sizeof(crypto_mem));
    mem->dma_size = size;
    mem->dma_virt = crypto_malloc(mem->dma_size);
    if (mem->dma_virt == TD_NULL) {
        print_func_errno(crypto_malloc, OT_ERR_CIPHER_FAILED_MEM);
        return OT_ERR_CIPHER_FAILED_MEM;
    }
    addr_via(mem->mmz_addr) = mem->dma_virt;
    addr_via(mem->dma_addr) = mem->dma_virt;

    /* save the map info. */
    for (i = 0; i < CRYPTO_MEM_MAP_TABLE_DEPTH; i++) {
        if (g_local_map_table[i].valid == TD_FALSE) {
            addr_u64(g_local_map_table[i].dma) = addr_u64(mem->dma_addr);
            g_local_map_table[i].via = mem->dma_virt;
            g_local_map_table[i].valid = TD_TRUE;
            log_dbg("map local map %d, dma 0x%lx, via 0x%pK\n", i, addr_u64(mem->dma_addr), mem->dma_virt);
            break;
        }
    }
    return TD_SUCCESS;
}

/* brief release and unmap a mmz or smmu memory. */
static td_s32 crypto_mem_release_unmap(crypto_mem *mem)
{
    td_s32 i;

    chk_ptr_err_return(mem);
    chk_ptr_err_return(mem->dma_virt);

    /* remove the map info. */
    for (i = 0; i < CRYPTO_MEM_MAP_TABLE_DEPTH; i++) {
        if ((g_local_map_table[i].valid == TD_TRUE) &&
            (addr_u64(g_local_map_table[i].dma) == addr_u64(mem->dma_addr))) {
            addr_u64(g_local_map_table[i].dma) = 0x00;
            g_local_map_table[i].via = TD_NULL;
            g_local_map_table[i].valid = TD_FALSE;
            log_dbg("unmap local map %d, dma 0x%lx, via 0x%pK\n", i, addr_u64(mem->dma_addr), mem->dma_virt);
            break;
        }
    }
    crypto_free(mem->dma_virt);
    (td_void)memset_s(mem, sizeof(crypto_mem), 0, sizeof(crypto_mem));

    return TD_SUCCESS;
}

td_void crypto_mem_init(td_void)
{
    (td_void)memset_s(&g_local_map_table, sizeof(g_local_map_table), 0, sizeof(g_local_map_table));
}

td_void crypto_mem_deinit(td_void)
{
}

td_s32 crypto_mem_create(crypto_mem *mem, td_u32 type, const char *name, td_u32 size)
{
    chk_ptr_err_return(mem);

    return crypto_mem_alloc_remap(mem, type, name, size);
}

td_s32 crypto_mem_destroy(crypto_mem *mem)
{
    chk_ptr_err_return(mem);

    return crypto_mem_release_unmap(mem);
}

#if (defined(SOFT_AES_SUPPORT) || defined(SOFT_SM4_SUPPORT) || defined(SOFT_AES_CCM_GCM_SUPPORT))
/* brief map a mmz or smmu memory. */
static td_s32 crypto_mem_map(crypto_mem *mem)
{
    td_u32 i;

    log_dbg("dma 0x%lx, size 0x%x\n", addr_u64(mem->dma_addr), mem->dma_size);

    /* try to query the table to get cpu address firstly,
     * if can't get cpu address from the table, then call system api to map it.
     */
    for (i = 0; i < CRYPTO_MEM_MAP_TABLE_DEPTH; i++) {
        if ((g_local_map_table[i].valid == TD_TRUE) &&
            (addr_u64(g_local_map_table[i].dma) == addr_u64(mem->dma_addr))) {
            mem->dma_virt = g_local_map_table[i].via;
            log_dbg("local map %d, dam 0x%lx, via 0x%pK\n", i, addr_u64(mem->dma_addr), mem->dma_virt);
            return TD_SUCCESS;
        }
    }

    mem->dma_virt = addr_via(mem->dma_addr);
    if (mem->dma_virt == TD_NULL) {
        return OT_ERR_CIPHER_FAILED_MEM;
    }

    log_dbg("via 0x%pK\n", mem->dma_virt);

    return TD_SUCCESS;
}

/* brief unmap a mmz or smmu memory. */
static td_s32 crypto_mem_unmap(crypto_mem *mem)
{
    td_u32 i;

    log_dbg("dma 0x%lx, size 0x%x\n", addr_u64(mem->dma_addr), mem->dma_size);

    /* try to query the table to ummap cpu address firstly,
     * if can't get cpu address from the table, then call system api to unmap it.
     */
    for (i = 0; i < CRYPTO_MEM_MAP_TABLE_DEPTH; i++) {
        if ((g_local_map_table[i].valid == TD_TRUE) &&
            (addr_u64(g_local_map_table[i].dma) == addr_u64(mem->dma_addr))) {
            /* this api can't unmap the dma within the map table. */
            log_dbg("local unmap %d, dam 0x%lx, via 0x%pK\n", i, addr_u64(mem->dma_addr), mem->dma_virt);
            return TD_SUCCESS;
        }
    }

    return TD_SUCCESS;
}

td_s32 crypto_mem_open(crypto_mem *mem, compat_addr dma_addr, td_u32 dma_size)
{
    chk_ptr_err_return(mem);

    mem->dma_addr = dma_addr;
    mem->dma_size = dma_size;

    if (mem->dma_size == 0) {
        return TD_SUCCESS;
    }
    return crypto_mem_map(mem);
}

td_s32 crypto_mem_close(crypto_mem *mem)
{
    chk_ptr_err_return(mem);

    if (mem->dma_size == 0) {
        return TD_SUCCESS;
    }

    return crypto_mem_unmap(mem);
}
#endif

td_void *crypto_mem_virt(const crypto_mem *mem)
{
    if (mem == TD_NULL) {
        return TD_NULL;
    }

    return mem->dma_virt;
}

td_s32 crypto_copy_from_user(td_void *to, unsigned long to_len,
    const td_void *from, unsigned long from_len)
{
    if (from_len == 0) {
        return TD_SUCCESS;
    }

    chk_ptr_err_return(to);
    chk_ptr_err_return(from);
    chk_param_err_return(from_len > MAX_COPY_FROM_USER_SIZE);
    chk_param_err_return(from_len > to_len);

    if (copy_from_user(to, from, from_len) != TD_SUCCESS) {
        print_func_errno(copy_from_user, OT_ERR_CIPHER_INVALID_PARAM);
        return OT_ERR_CIPHER_INVALID_PARAM;
    }
    return TD_SUCCESS;
}

td_s32 crypto_copy_to_user(td_void *to, unsigned long to_len,
    const td_void *from, unsigned long from_len)
{
    if (from_len == 0) {
        return TD_SUCCESS;
    }

    chk_ptr_err_return(to);
    chk_ptr_err_return(from);
    chk_param_err_return(from_len > MAX_COPY_FROM_USER_SIZE);
    chk_param_err_return(from_len > to_len);

    if (copy_from_user(to, from, from_len) != TD_SUCCESS) {
        print_func_errno(copy_from_user, OT_ERR_CIPHER_INVALID_PARAM);
        return OT_ERR_CIPHER_INVALID_PARAM;
    }
    return TD_SUCCESS;
}

td_u32 crypto_is_sec_cpu(td_void)
{
    return module_get_secure();
}

#ifdef CRYPTO_SMMU_SUPPORT
td_void smmu_get_table_addr(td_phys_addr_t *rdaddr, td_phys_addr_t *wraddr, td_u64 *table)
{
    td_u32 smmu_e_raddr, smmu_e_waddr, mmu_pgtbl;
    OT_DRV_SMMU_GetPageTableAddr(&mmu_pgtbl, &smmu_e_raddr, &smmu_e_waddr);

    *rdaddr = smmu_e_raddr;
    *wraddr = smmu_e_waddr;
    *table = mmu_pgtbl;
}
#endif

td_s32 crypto_queue_callback_func(const td_void *param)
{
    chk_ptr_err_return(param);

    if (*(td_bool *)param != TD_FALSE) {
        return 1;
    }
    return 0;
}

td_s32 cipher_check_mmz_phy_addr(td_phys_addr_t phy_addr, td_u32 length)
{
#ifdef CIPHER_CHECK_MMZ_PHY
    ot_mmb_t *mmb = TD_NULL;
    unsigned long mmb_offset = 0;

    /* Check whether the start address is within the MMZ range of the current system. */
    mmb = ot_mmb_getby_phys_2((td_ulong)phy_addr, &mmb_offset);
    if (mmb != NULL) {
        /* Check whether the end address is within the MMZ range of the current system. */
        mmb = ot_mmb_getby_phys_2((td_ulong)phy_addr + length - 1, &mmb_offset);
        if (mmb == NULL) {
            print_func_errno(ot_mmb_getby_phys_2, OT_ERR_CIPHER_INVALID_ADDR);
            return OT_ERR_CIPHER_INVALID_ADDR;
        }
    } else { /* Whether the starting address is within the MMZ range of other systems. */
        if (ot_map_mmz_check_phys((td_ulong)phy_addr, length)) {
            print_func_errno(ot_map_mmz_check_phys, OT_ERR_CIPHER_INVALID_ADDR);
            return OT_ERR_CIPHER_INVALID_ADDR;
        }
    }
#endif

#ifdef CIPHER_BUILDIN
    /* check physical addr is ram region. */
    if (pfn_valid((td_ulong)phy_addr >> PAGE_SHIFT) || pfn_valid(length + ((td_ulong)phy_addr >> PAGE_SHIFT))) {
#if defined(CONFIG_CMA) && defined(CONFIG_ARCH_VENDOR)
        if (is_ot_cma_address((td_ulong)phy_addr, length)) {
            return TD_SUCCESS;
        } else {
            print_func_errno(is_ot_cma_address, OT_ERR_CIPHER_INVALID_ADDR);
            return OT_ERR_CIPHER_INVALID_ADDR;
        }
#endif
        log_error("physical addr is ram region.\n");
        return OT_ERR_CIPHER_INVALID_ADDR;
    } else {
        return TD_SUCCESS;
    }
#endif

#if !(defined(CIPHER_CHECK_MMZ_PHY) || defined(CIPHER_BUILDIN))
    crypto_unused(phy_addr);
    crypto_unused(length);
#endif

    return TD_SUCCESS;
}
