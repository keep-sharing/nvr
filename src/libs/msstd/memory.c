#include "msstd.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <malloc.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/sendfile.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <arpa/inet.h>

typedef struct tag_MMAP_Node {
    unsigned int Start_P;
    unsigned int Start_V;
    unsigned int length;
    unsigned int refcount;  /* map后的空间段的引用计数 */
    struct tag_MMAP_Node *next;
} TMMAP_Node_t;

TMMAP_Node_t *pTMMAPNode = NULL;

#define PAGE_SIZE 0x1000
#define PAGE_SIZE_MASK 0xfffff000

static int fd = -1;
static const char dev[] = "/dev/mem";

/* no need considering page_size of 4K */
void *__ms_map(unsigned int phy_addr, unsigned int size)
{
    unsigned int phy_addr_in_page;
    unsigned int page_diff;

    unsigned int size_in_page;

    TMMAP_Node_t *pTmp;
    TMMAP_Node_t *pNew;

    void *addr = NULL;

    if (size == 0) {
        printf("memmap():size can't be zero!\n");
        return NULL;
    }

    /* check if the physical memory space have been mmaped */
    pTmp = pTMMAPNode;
    while (pTmp != NULL) {
        if ((phy_addr >= pTmp->Start_P) &&
            ((phy_addr + size) <= (pTmp->Start_P + pTmp->length))) {
            pTmp->refcount++;   /* referrence count increase by 1  */
            return (void *)(pTmp->Start_V + phy_addr - pTmp->Start_P);
        }

        pTmp = pTmp->next;
    }

    /* not mmaped yet */
    if (fd < 0) {
        /* dev not opened yet, so open it */
        fd = open(dev, O_RDWR | O_SYNC);
        if (fd < 0) {
            printf("memmap():open %s error!\n", dev);
            return NULL;
        }
    }

    /* addr align in page_size(4K) */
    phy_addr_in_page = phy_addr & PAGE_SIZE_MASK;
    page_diff = phy_addr - phy_addr_in_page;

    /* size in page_size */
    size_in_page = ((size + page_diff - 1) & PAGE_SIZE_MASK) + PAGE_SIZE;

    addr = mmap((void *)0, size_in_page, PROT_READ | PROT_WRITE, MAP_SHARED, fd, phy_addr_in_page);
    if (addr == MAP_FAILED) {
        printf("memmap():mmap @ 0x%x error!\n", phy_addr_in_page);
        return NULL;
    }

    /* add this mmap to MMAP Node */
    pNew = (TMMAP_Node_t *)malloc(sizeof(TMMAP_Node_t));
    if (NULL == pNew) {
        printf("memmap():malloc new node failed!\n");
        return NULL;
    }
    pNew->Start_P = phy_addr_in_page;
    pNew->Start_V = (unsigned int)addr;
    pNew->length = size_in_page;
    pNew->refcount = 1;
    pNew->next = NULL;

    if (pTMMAPNode == NULL) {
        pTMMAPNode = pNew;
    } else {
        pTmp = pTMMAPNode;
        while (pTmp->next != NULL) {
            pTmp = pTmp->next;
        }

        pTmp->next = pNew;
    }

    return (void *)(addr + page_diff);
}

int __ms_unmap(void *addr_mapped)
{
    TMMAP_Node_t *pPre;
    TMMAP_Node_t *pTmp;

    if (pTMMAPNode == NULL) {
        printf("memunmap(): address have not been mmaped!\n");
        return -1;
    }

    /* check if the physical memory space have been mmaped */
    pTmp = pTMMAPNode;
    pPre = pTMMAPNode;

    do {
        if (((unsigned int)addr_mapped >= pTmp->Start_V) &&
            ((unsigned int)addr_mapped <= (pTmp->Start_V + pTmp->length))) {
            pTmp->refcount--;   /* referrence count decrease by 1  */
            if (0 == pTmp->refcount) {
                /* 引用计数变为0, 被map的内存空间不再使用,此时需要进行munmap回收 */

                //        printf("memunmap(): map node will be remove!\n");

                /* delete this map node from pMMAPNode */
                if (pTmp == pTMMAPNode) {
                    pTMMAPNode = NULL;
                } else {
                    pPre->next = pTmp->next;
                }

                /* munmap */
                if (munmap((void *)pTmp->Start_V, pTmp->length) != 0) {
                    printf("memunmap(): munmap failed!\n");
                }

                free(pTmp);
            }

            return 0;
        }

        pPre = pTmp;
        pTmp = pTmp->next;
    } while (pTmp != NULL);

    printf("memunmap(): address have not been mmaped!\n");
    return -1;
}

QT_CALLBACK_MEM qt_callback_mem = NULL;

void set_qt_callback_mem(QT_CALLBACK_MEM callback)
{
    qt_callback_mem = callback;
}

#ifdef MS_LEAK_DETECT

#define MAX_LEAK_FILE_SIZE (2*1024*1024)
struct mem_info {
    void *address;
    unsigned int size;
    unsigned int line;
    char file[32];
    char time[32];
};

struct mem_leak {
    struct mem_info info;
    struct mem_leak *next;
};

static struct mem_leak *ptr_start = 0;
static struct mem_leak *ptr_next = 0;
static MUTEX_OBJECT global_mutex;

static void add(struct mem_info *info)
{
    struct mem_leak *leak = (struct mem_leak *)malloc(sizeof(struct mem_leak));
    if (!leak) {
        return;
    }
    leak->info.address = info->address;
    leak->info.size = info->size;
    leak->info.line = info->line;
    snprintf(leak->info.file, sizeof(leak->info.file), "%s", info->file);
    time_to_string_ms(leak->info.time);
    leak->next = 0;

    if (ptr_start == NULL) {
        ptr_start = leak;
        ptr_next = ptr_start;
    } else {
        ptr_next->next = leak;
        ptr_next = ptr_next->next;
    }
}

static void erase(unsigned int pos)
{
    unsigned int i = 0;
    struct mem_leak *leak;
    if (pos == 0) {
        leak = ptr_start;
        ptr_start = ptr_start->next;
        free(leak);
        return;
    }
    for (i = 0, leak = ptr_start; i < pos; leak = leak->next, ++i) {
        if (pos == i + 1) {
            struct mem_leak *temp = leak->next;
            leak->next = temp->next;
            free(temp);
            if (leak->next == 0) {
                ptr_next = leak;
            }
            break;
        }
    }
}

static void __ms_clear()
{
    struct mem_leak *temp = ptr_start;
    struct mem_leak *leak = ptr_start;

    while (leak != NULL) {
        leak = leak->next;
        free(temp);
        temp = leak;
    }
    ptr_start = 0;
}

static void add_mem_info(void *mem_ref, unsigned int size,  const char *file, unsigned int line)
{
    if (qt_callback_mem) {
        qt_callback_mem(QT_MEM_ADD, NULL, mem_ref, size, file, line);
    } else {
        struct mem_info info;
        memset(&info, 0, sizeof(info));
        info.address = mem_ref;
        info.size = size;
        snprintf(info.file, sizeof(info.file), "%s", file);
        info.line = line;
        ms_mutex_lock(&global_mutex);
        add(&info);
        ms_mutex_unlock(&global_mutex);
    }
}

static void remove_mem_info(void *mem_ref, const char *file, int lineno)
{
    if (qt_callback_mem) {
        qt_callback_mem(QT_MEM_DEL, NULL, mem_ref, 0, file, lineno);
    } else {
        unsigned int index;
        ms_mutex_lock(&global_mutex);
        struct mem_leak *leak_info = ptr_start;
        //printf("=====remove_mem_info=========0000=====%p====\n", mem_ref);
        for (index = 0; leak_info != NULL; ++index, leak_info = leak_info->next) {
            if (leak_info->info.address == mem_ref) {
                erase(index);
                break;
            }
        }
        ms_mutex_unlock(&global_mutex);
    }
}

static void replace_mem_info(void *mem_old, void *mem_new, int size, const char *file, int lineno)
{
    if (qt_callback_mem) {
        qt_callback_mem(QT_MEM_REP, mem_old, mem_new, size, file, lineno);
    } else {
        unsigned int index;
        ms_mutex_lock(&global_mutex);
        struct mem_leak *leak_info = ptr_start;
        for (index = 0; leak_info != NULL; ++index, leak_info = leak_info->next) {
            if (leak_info->info.address == mem_old) {
                leak_info->info.address = mem_new;
                snprintf(leak_info->info.file, sizeof(leak_info->info.file), "%s", file);
                leak_info->info.line = lineno;
                break;
            }
        }
        ms_mutex_unlock(&global_mutex);
    }
}
void *__ms_realloc(void *ptr, unsigned int size, const char *file, int lineno)
{
    void *p = realloc(ptr, size);
    if (!p) {
        system("cat /proc/buddyinfo");
        printf("[ERROR]file:%s line:%d realloc size:%d fail.\n", file, lineno, size);
        system("cat /proc/buddyinfo");
        sleep(2);
        assert(p);
    } else {
        if (p != ptr) {
            replace_mem_info(ptr, p, size, file, lineno);
        }
    }
    //memset(p, 0, size);
    return p;
}
void *__ms_malloc(unsigned int size, const char *file, int lineno)
{
    if (size <= 0) {
        return NULL;
    }
    //msprintf("[david debug] file:%s line:%d malloc size:%d fail.\n", file, lineno, size);
    void *p = malloc(size);
    if (!p) {
        system("cat /proc/buddyinfo");
        printf("[ERROR]file:%s line:%d malloc size:%d fail.\n", file, lineno, size);
        system("cat /proc/buddyinfo");
        sleep(2);
        assert(p);
    } else {
        //memset(p, 0, size);
        add_mem_info(p, size, file, lineno);
    }
    memset(p, 0, size);
    return p;
}

void  __ms_free(void *p, const char *file, int lineno)
{
    if (!p) {
        return;
    }

    remove_mem_info(p, file, lineno);
    free(p);
}

void *__ms_calloc(unsigned int n, unsigned int size, const char *file, int lineno)
{
    if (n <= 0 || size <= 0) {
        return NULL;
    }
    void *p = calloc(n, size);
    if (!p) {
        printf("[ERROR]file:%s line:%d calloc (%d)size:%d fail.\n", file, lineno, n, size);
        sleep(2);
        assert(p);
    }

    add_mem_info(p, size * n, file, lineno);
    memset(p, 0, size * n);
    return p;
}

void *__ms_valloc(unsigned int size, const char *file, int lineno)
{
    if (size <= 0) {
        return NULL;
    }
    void *p = valloc(size);
    if (!p) {
        system("cat /proc/buddyinfo");
        printf("[ERROR]file:%s line:%d valloc size:%d fail.\n", file, lineno, size);
        system("cat /proc/buddyinfo");
        sleep(2);
        assert(p);
    } else {
        add_mem_info(p, size, file, lineno);
    }
    memset(p, 0, size);
    return p;
}

void mem_leak_init()
{
    ms_mutex_init(&global_mutex);
}

unsigned int mem_get_cur_list()
{
    unsigned int count = 0;
    ms_mutex_lock(&global_mutex);
    struct mem_leak *leak_info = ptr_start;
    while (leak_info) {
        count++;
        leak_info = leak_info->next;
    }
    ms_mutex_unlock(&global_mutex);
    return count;
}

void mem_leak_report()
{
    struct mem_leak *leak_info;
    unsigned int total_size = 0, count = 0;

    FILE *fp_write = fopen(MEM_LEAK_OUTPUT_FILE, "wt");

    if (fp_write != NULL) {
        char *info = (char *)malloc(MAX_LEAK_FILE_SIZE);
        memset(info, 0, MAX_LEAK_FILE_SIZE);

        sprintf(info, "%s\n", "Memory Leak Summary");
        fwrite(info, (strlen(info) + 1), 1, fp_write);
        sprintf(info, "%s\n", "-----------------------------------");
        fwrite(info, (strlen(info) + 1), 1, fp_write);

        ms_mutex_lock(&global_mutex);
        for (leak_info = ptr_start; leak_info != NULL; leak_info = leak_info->next) {
            sprintf(info, "address : %p\n", (unsigned int *)leak_info->info.address);
            fwrite(info, (strlen(info) + 1), 1, fp_write);
            sprintf(info, "size    : %u bytes\n", leak_info->info.size);
            total_size += leak_info->info.size;
            fwrite(info, (strlen(info) + 1), 1, fp_write);
            sprintf(info, "file    : %s\n", leak_info->info.file);
            fwrite(info, (strlen(info) + 1), 1, fp_write);
            sprintf(info, "line    : %u\n", leak_info->info.line);
            fwrite(info, (strlen(info) + 1), 1, fp_write);
            sprintf(info, "%s==index:%d\n", "-----------------------------------", count);
            fwrite(info, (strlen(info) + 1), 1, fp_write);
            count++;
        }
        ms_mutex_unlock(&global_mutex);

        sprintf(info, "\ntotal count : %u\n", count);
        fwrite(info, (strlen(info) + 1), 1, fp_write);
        sprintf(info, "\ntotal size : %u\n", total_size);
        fwrite(info, (strlen(info) + 1), 1, fp_write);

        free(info);
        fclose(fp_write);
    }

    ms_mutex_lock(&global_mutex);
    __ms_clear();
    ms_mutex_unlock(&global_mutex);
    ms_mutex_uninit(&global_mutex);
}

void mem_leak_show()
{
    struct mem_leak *leak_info;
    unsigned int total_size = 0, count = 0;
    char fileName[64];
    char time[32];

    time_to_string_ms(time);
    sprintf(fileName, "/mnt/nand3/mem_%s", time);

    FILE *fp_write = fopen(fileName, "wt");

    if (fp_write != NULL) {
        char *info = (char *)malloc(MAX_LEAK_FILE_SIZE);
        memset(info, 0, MAX_LEAK_FILE_SIZE);

        sprintf(info, "%s\n", "Memory Leak Summary");
        fwrite(info, (strlen(info) + 1), 1, fp_write);
        sprintf(info, "%s\n", "-----------------------------------");
        fwrite(info, (strlen(info) + 1), 1, fp_write);

        ms_mutex_lock(&global_mutex);
        for (leak_info = ptr_start; leak_info != NULL; leak_info = leak_info->next) {
            sprintf(info, "address : %p\n", (unsigned int *)leak_info->info.address);
            fwrite(info, (strlen(info) + 1), 1, fp_write);
            sprintf(info, "size    : %u bytes\n", leak_info->info.size);
            total_size += leak_info->info.size;
            fwrite(info, (strlen(info) + 1), 1, fp_write);
            sprintf(info, "file    : %s\n", leak_info->info.file);
            fwrite(info, (strlen(info) + 1), 1, fp_write);
            sprintf(info, "line    : %u\n", leak_info->info.line);
            fwrite(info, (strlen(info) + 1), 1, fp_write);
            sprintf(info, "time    : %s\n", leak_info->info.time);
            fwrite(info, (strlen(info) + 1), 1, fp_write);
            sprintf(info, "%s==index:%d\n", "-----------------------------------", count);
            fwrite(info, (strlen(info) + 1), 1, fp_write);
            count++;
        }
        ms_mutex_unlock(&global_mutex);

        sprintf(info, "\ntotal count : %u\n", count);
        fwrite(info, (strlen(info) + 1), 1, fp_write);
        sprintf(info, "\ntotal size : %u\n", total_size);
        fwrite(info, (strlen(info) + 1), 1, fp_write);

        free(info);
        fclose(fp_write);
    }
}

#else
void *__ms_realloc(void *ptr, unsigned int size, const char *file, int lineno)
{
    int count = 0;
    void *p;
    do {
        p = realloc(ptr, size);
        if (p) {
            break;
        } else {
            count++;
            printf("[ERROR]file:%s line:%d realloc size:%d fail, count:%d.\n", file, lineno, size, count);
        }
    } while (count < 100);
    if (!p) {
        system("cat /proc/buddyinfo");
        printf("[ERROR]file:%s line:%d realloc size:%d fail.\n", file, lineno, size);
        system("cat /proc/buddyinfo");
		system("touch /mnt/nand3/out_of_memory.txt");	//goli add, 2022.12.16
        printf("[%s]out_of_memory\n", __func__);
        sleep(2);
        assert(p);
    }
    //memset(p, 0, size);

    return p;
}

void *__ms_malloc(unsigned int size, const char *file, int lineno)
{
    if (size <= 0) {
        return NULL;
    }

    int count = 0;
    void *p;
    do {
        p = malloc(size);
        if (p) {
            break;
        } else {
            count++;
            printf("[ERROR]file:%s line:%d malloc size:%d fail, count:%d.\n", file, lineno, size, count);
        }
    } while (count < 100);
    if (!p) {
        system("cat /proc/buddyinfo");
        printf("[ERROR]file:%s line:%d malloc size:%d fail.\n", file, lineno, size);
        system("cat /proc/buddyinfo");
        system("touch /mnt/nand3/out_of_memory.txt");	//goli add, 2022.12.16
        printf("[%s]out_of_memory\n", __func__);
        sleep(2);
        assert(p);
    }
    memset(p, 0, size);

    return p;
}

void  __ms_free(void *p, const char *file, int lineno)
{
    if (!p) {
        return;
    }
    free(p);
}

void __ms_scope_free(void **p) 
{
    if (!p || !*p) {
        return;
    }
    
    free(*p);
    p = NULL;  
}

void *__ms_calloc(unsigned int n, unsigned int size, const char *file, int lineno)
{
    if (n <= 0 || size <= 0) {
        return NULL;
    }

    int count = 0;
    void *p;
    do {
        p = calloc(n, size);
        if (p) {
            break;
        } else {
            count++;
            printf("[ERROR]file:%s line:%d calloc (%d)size:%d fail, count:%d.\n", file, lineno, n, size, count);
        }
    } while (count < 100);
    if (!p) {
        printf("[ERROR]file:%s line:%d calloc (%d)size:%d fail.\n", file, lineno, n, size);
        system("touch /mnt/nand3/out_of_memory.txt"); //goli add, 2022.12.16
        printf("[%s]out_of_memory\n", __func__); 
        sleep(2);
        assert(p);
    }
    memset(p, 0, n * size);

    return p;
}

void *__ms_valloc(unsigned int size, const char *file, int lineno)
{
    if (size <= 0) {
        return NULL;
    }

    int count = 0;
    void *p;
    do {
        p = valloc(size);
        if (p) {
            break;
        } else {
            count++;
            printf("[ERROR]file:%s line:%d valloc size:%d fail, count:%d.\n", file, lineno, size, count);
        }
    } while (count < 100);
    if (!p) {
        system("cat /proc/buddyinfo");
        printf("[ERROR]file:%s line:%d valloc size:%d fail.\n", file, lineno, size);
        system("cat /proc/buddyinfo");
		system("touch /mnt/nand3/out_of_memory.txt");	//goli add, 2022.12.16
        printf("[%s]out_of_memory\n", __func__);
        sleep(2);
        assert(p);
    }
    memset(p, 0, size);

    return p;
}

unsigned int mem_get_cur_list()
{
    return 0;
}
void mem_leak_init()
{
}
void mem_leak_report()
{
}
void mem_leak_show()
{
}

void ms_memcpy(void *dst, void *src, unsigned int size)
{
    if (!dst || !src || size <= 0) {
        return ;
    }

    int i, iCnt = 0, iRemainder = 0;
    const int pageSize = 4096;
    char *pdst = dst;
    char *psrc = src;

    if (size <= pageSize) {
        memcpy(pdst, psrc, size);
        return ;
    } else {
        iCnt = size / pageSize;
        iRemainder = size % pageSize;
    }

    for (i = 0; i < iCnt; i++) {
        memcpy(pdst, psrc, pageSize);
        if (size > pageSize) {
            pdst = pdst + pageSize;
            psrc = psrc + pageSize;
        }
    }

    if (iRemainder) {
        memcpy(pdst, psrc, iRemainder);
    }

    return ;
}

#endif

int ms_spliced_copy(int in_fd, int out_fd)
{
    loff_t in_off = 0;
    loff_t out_off = 0;
    static int buf_size = __IO_BUFSIZE;
    off_t len;
    int filedes[2];
    int err = -1;
    struct stat stbuf;

    if (pipe(filedes) < 0) {
        perror("pipe:");
        goto out;
    }
    if (fstat(in_fd, &stbuf) < 0) {
        perror("fstat:");
        goto out_close;
    }

    len = stbuf.st_size;
    while (len > 0) {
        if (buf_size > len) {
            buf_size = len;
        }
        /*
             * move to pipe buffer.
             */
        err = splice(in_fd, &in_off, filedes[1], NULL, buf_size, SPLICE_F_MOVE | SPLICE_F_MORE);
        if (err < 0) {
            perror("splice:");
            goto out_close;
        }
        /*
             * move from pipe buffer to out_fd
             */
        err = splice(filedes[0], NULL, out_fd, &out_off, buf_size, SPLICE_F_MOVE | SPLICE_F_MORE);
        if (err < 0) {
            perror("splice2:");
            goto out_close;
        }
        len -= buf_size;
    }
    err = 0;

out_close:
    close(filedes[0]);
    close(filedes[1]);

out:
    return err;

}

int ms_spliced_copy_ex(int in_fd, int pipe_out_fd)
{
    loff_t in_off = 0;
    static int buf_size = __IO_BUFSIZE;
    off_t len;
    int filedes[2];
    int err = -1;
    struct stat stbuf;

    if (pipe(filedes) < 0) {
        perror("pipe:");
        goto out;
    }
    if (fstat(in_fd, &stbuf) < 0) {
        perror("fstat:");
        goto out_close;
    }

    len = stbuf.st_size;
    while (len > 0) {
        if (buf_size > len) {
            buf_size = len;
        }
        /*
             * move to pipe buffer.
             */
        err = splice(in_fd, &in_off, filedes[1], NULL, buf_size, SPLICE_F_MOVE | SPLICE_F_MORE);
        if (err < 0) {
            perror("splice:");
            goto out_close;
        }
        /*
             * move from pipe buffer to out_fd
             */
        err = splice(filedes[0], NULL, pipe_out_fd, NULL, buf_size, SPLICE_F_MOVE | SPLICE_F_MORE);
        if (err < 0) {
            perror("splice2:");
            goto out_close;
        }
        len -= buf_size;
    }
    err = 0;

out_close:
    close(filedes[0]);
    close(filedes[1]);

out:
    return err;

}

int ms_sendfile_copy(int in_fd, int out_fd)
{
    struct stat buf;
    fstat(in_fd, &buf);
    int err = sendfile(out_fd, in_fd, NULL, buf.st_size);
    if (err < 0) {
        return -1;
    }

    return buf.st_size;
}

int ms_std_copy(int in_fd, int out_fd)
{
    char buf[__IO_BUFSIZE];
    int err = -1;
    int bytes;
    while ((bytes = read(in_fd, buf, sizeof(buf))) > 0) {
        if (write(out_fd, buf, bytes) != bytes) {
            perror("write:");
            goto out;
        }
    }
    err = 0;

out:
    return err;
}

int ms_file_copy(char *fileIn, char *fileOut)
{
    int ret = -1;
    int fdIn = -1;
    int fdOut = -1;
    FILE *fpIn = NULL;
    FILE *fpOut = NULL;
    struct stat filestat;

    fpIn = fopen(fileIn, "rb");
    fpOut = fopen(fileOut, "wb");
    if (!fpIn || !fpOut) {
        ret = -1;
    } else {
        stat(fileIn, &filestat);
        fdIn = (int)fileno(fpIn);
        fdOut = (int)fileno(fpOut);
//        ret = ms_spliced_copy(fdIn, fdOut);
        ret = ms_std_copy(fdIn, fdOut);
    }

    if (fpIn) {
        fclose(fpIn);
    }
    if (fpOut) {
        fclose(fpOut);
    }

    return ret;
}

int ms_file_move(char *srcPath, char *dstPath)
{
    int ret = -1;
    if (!ms_file_copy(srcPath, dstPath)) {
        ret = remove(srcPath);
    }

    return ret;
}

