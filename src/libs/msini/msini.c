/* 
 * ***************************************************************
 * Filename:      	msini.c
 * Created at:    	2019.05.23
 * Description:       ms iniparser 
 * Author:        	zbing
 * Copyright (C)  	milesight
* ***************************************************************
*/

#include "msini.h"
#include "list.h"
#include "iniparser.h"

typedef struct iniHDL{
    MS_S32  busy;
    MS_S8 iniFile[MAX_INI_NAME_LEN];
	dictionary * dict;
    RWLOCK_BOJECT rwLock;
    struct list_head node;
}iniHDL;

typedef struct msini{
    MS_BOOL bInit;
	dictionary * dict;
    MUTEX_OBJECT mutex;
	struct list_head hdlList;
}msini;

#define HDL_FIND_BY_NAME(devname, pstHDL) \
    { \
        struct iniHDL* pos = NULL; \
        struct iniHDL* n = NULL; \
 	    list_for_each_entry_safe(pos, n, &g_ini.hdlList, node) \
        { \
            if (!strcmp(pos->iniFile, devname)) \
            { \
                pstHDL = pos; \
                break; \
            } \
        } \
    }

#define HDL_FIND_BY_HDL(HDL, pstHDL) \
    { \
        struct iniHDL* pos = NULL; \
        struct iniHDL* n = NULL; \
 	    list_for_each_entry_safe(pos, n, &g_ini.hdlList, node) \
        { \
            if (pos == HDL) \
            { \
                pstHDL = pos; \
                break; \
            } \
        } \
    }

static struct msini g_ini = {0};

static void
ini_handle_add(struct iniHDL *pstHDL)
{
    if (!pstHDL->busy)
    {
        ms_rwlock_init(&pstHDL->rwLock);
        list_add(&pstHDL->node, &g_ini.hdlList);
    }
    pstHDL->busy++;
}

static void
ini_handle_del(struct iniHDL *pstHDL)
{
    pstHDL->busy--;
    if (!pstHDL->busy)
    {
        list_del(&pstHDL->node);
        iniparser_freedict(pstHDL->dict);
        ms_rwlock_uninit(&pstHDL->rwLock);
        ms_free(pstHDL);
    }
}

static void
ini_handle_clear()
{
    struct iniHDL *pstHDL;
    struct iniHDL *n;
    
    ms_mutex_lock(&g_ini.mutex);
    list_for_each_entry_safe(pstHDL, n, &g_ini.hdlList, node)
    {
        list_del(&pstHDL->node);
        iniparser_freedict(pstHDL->dict);
        ms_rwlock_uninit(&pstHDL->rwLock);
        ms_free(pstHDL);
    }
    ms_mutex_unlock(&g_ini.mutex);
}

MS_S8 *
ms_ini_getstr(void *ini, MS_S8 * key)
{
    struct iniHDL * pstHDL = ini;
	MS_S8 * ret = NULL;
	
	if (pstHDL->dict && key)
	{
		ms_rwlock_rdlock(&pstHDL->rwLock);
		ret = iniparser_getstring(pstHDL->dict, key, NULL);
		ms_rwlock_unlock(&pstHDL->rwLock);
	}
	
	return ret;
}

MS_S32
ms_ini_getint(void *ini, MS_S8 * key, MS_S32 def)
{
    struct iniHDL * pstHDL = ini;
	MS_S32 ret = def;
	
	if (pstHDL->dict && key)
	{
		ms_rwlock_rdlock(&pstHDL->rwLock);
		ret = iniparser_getint(pstHDL->dict, key, def);
		ms_rwlock_unlock(&pstHDL->rwLock);
	}
	
	return ret;
}

MS_S32
ms_ini_getbool(void *ini, MS_S8 * key, MS_S32 def)
{
    struct iniHDL * pstHDL = ini;
	MS_S32 ret = def;
	
	if (pstHDL->dict && key)
	{
		ms_rwlock_rdlock(&pstHDL->rwLock);
		ret = iniparser_getboolean(pstHDL->dict, key, def);
		ms_rwlock_unlock(&pstHDL->rwLock);
	}
	
	return ret;
}

MS_S32
ms_ini_setstr(void *ini, MS_S8 * entry, MS_S8 * val)
{
    struct iniHDL * pstHDL = ini;
	MS_S32 ret = MS_FAILURE;
	if (pstHDL->dict)
	{
		ms_rwlock_wrlock(&pstHDL->rwLock);
		ret = iniparser_set(pstHDL->dict, entry, val);
		if(ret == MS_SUCCESS)
			iniparser_dump_ini(pstHDL->dict, pstHDL->iniFile);
		ms_rwlock_unlock(&pstHDL->rwLock);
	}
	return ret;
}

MS_S32
ms_ini_setint(void *ini, MS_S8 * entry, MS_S32 in)
{
    struct iniHDL * pstHDL = ini;
	MS_S32 ret = MS_FAILURE;
	MS_S8 val[16] = {0};
	
	if (pstHDL->dict)
	{
		ms_rwlock_wrlock(&pstHDL->rwLock);
		snprintf(val, sizeof(val), "%d", in);
		ret = iniparser_set(pstHDL->dict, entry, val);
		if(ret == MS_SUCCESS)
			iniparser_dump_ini(pstHDL->dict, pstHDL->iniFile);
		ms_rwlock_unlock(&pstHDL->rwLock);
	}
	
	return ret;
}

void
ms_ini_unset(void *ini, MS_S8 * entry)
{
    struct iniHDL * pstHDL = ini;
    
	if (pstHDL->dict)
	{
		ms_rwlock_wrlock(&pstHDL->rwLock);
		iniparser_unset(pstHDL->dict, entry);
		iniparser_dump_ini(pstHDL->dict, pstHDL->iniFile);
		ms_rwlock_unlock(&pstHDL->rwLock);
	}
}

void
ms_ini_dump(void *ini)
{
    struct iniHDL * pstHDL = ini;
    
    ms_rwlock_rdlock(&pstHDL->rwLock);
	if (pstHDL->dict)
		iniparser_dump(pstHDL->dict, stderr);
    ms_rwlock_unlock(&pstHDL->rwLock);
}

void
ms_ini_save(void *ini)
{
    struct iniHDL * pstHDL = ini;
    
	if (pstHDL->dict)
	{
		ms_rwlock_wrlock(&pstHDL->rwLock);
		iniparser_dump_ini(pstHDL->dict, pstHDL->iniFile);
		ms_rwlock_unlock(&pstHDL->rwLock);
	}
}

void *
ms_ini_open(MS_S8 *iniFile)
{
    if (strlen(iniFile) >= MAX_INI_NAME_LEN)
    {
        msprintf("file name over than %d", MAX_INI_NAME_LEN);
        return NULL;
    }
    
    struct iniHDL * pstHDL = NULL;
    FILE *pFd = NULL;
    
    ms_mutex_lock(&g_ini.mutex);
    HDL_FIND_BY_NAME(iniFile, pstHDL);

    if (!pstHDL)
    {
        pstHDL = ms_calloc(1, sizeof(struct iniHDL));
        pstHDL->dict = iniparser_load(iniFile);
        if(!pstHDL->dict)
        {
            pFd = fopen(iniFile, "w");
            if (!pFd)
            {
                msprintf("load %s fail !!!", iniFile);
            }
            else
            {
                //reload new file
                pstHDL->dict = iniparser_load(iniFile);
                fclose(pFd);
            }
            
            if (!pstHDL->dict)
            {
                ms_free(pstHDL);
                ms_mutex_unlock(&g_ini.mutex);
                return NULL;
            }
        }
        strncpy(pstHDL->iniFile, iniFile, sizeof(pstHDL->iniFile));
    }
    
    ini_handle_add(pstHDL);
    ms_mutex_unlock(&g_ini.mutex);
    
    return pstHDL;
}


MS_S32
ms_ini_close(void *ini)
{
    struct iniHDL *pstHDL = NULL;
	MS_S32 ret = MS_FAILURE;

    ms_mutex_lock(&g_ini.mutex);

    HDL_FIND_BY_HDL(ini, pstHDL);
	if (pstHDL)
	{
        ms_rwlock_wrlock(&pstHDL->rwLock);
        iniparser_dump_ini(pstHDL->dict, pstHDL->iniFile);
        ms_rwlock_unlock(&pstHDL->rwLock);
		ini_handle_del(pstHDL);
        ret = MS_SUCCESS;
	}
	
    ms_mutex_unlock(&g_ini.mutex);

	return ret;
}

MS_S32
ms_ini_init()
{
    if (g_ini.bInit == MS_YES)
        return MS_SUCCESS;
        
    memset(&g_ini, 0, sizeof(struct msini));
	INIT_LIST_HEAD(&g_ini.hdlList);
	ms_mutex_init(&g_ini.mutex);
    g_ini.bInit = MS_YES;

    return MS_SUCCESS;
}

MS_S32
ms_ini_deinit()
{
    if (g_ini.bInit == MS_NO)
        return MS_SUCCESS;

    ini_handle_clear();
	ms_mutex_uninit(&g_ini.mutex);
    g_ini.bInit = MS_NO;
	
    return MS_SUCCESS;
}


