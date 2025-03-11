#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include "fisheye_EGL.h"
#include "linux/hi_dbe.h"

#if defined(_HI3536_)
#define HIL_MMZ_NAME_LEN 32
#define HIL_MMB_NAME_LEN 16

struct mmb_info {
        unsigned long phys_addr;        /* phys-memory address */
        unsigned long align;            /* if you need your phys-memory have special align size */
        unsigned long size;             /* length of memory you need, in bytes */
        unsigned int order;

        void *mapped;                   /* userspace mapped ptr */

        union {
                struct {
                        unsigned long prot  :8; /* PROT_READ or PROT_WRITE */
                        unsigned long flags :12;/* MAP_SHARED or MAP_PRIVATE */

#ifdef __KERNEL__
                        unsigned long reserved :8; /* reserved, do not use */
                        unsigned long delayed_free :1; 
                        unsigned long map_cached :1; 
#endif
                };
                unsigned long w32_stuf;
        };

        char mmb_name[HIL_MMB_NAME_LEN];
        char mmz_name[HIL_MMZ_NAME_LEN];
        unsigned long gfp;              /* reserved, do set to 0 */

#ifdef __KERNEL__
        int map_ref;
        int mmb_ref;

        struct list_head list;
        hil_mmb_t *mmb;
#endif
};

struct dirty_area {
        unsigned long dirty_phys_start; /* dirty physical address */
        unsigned long dirty_virt_start; /* dirty virtual  address,
                                           must be coherent with dirty_phys_addr */
        unsigned long dirty_size;
};
#define IOC_MMB_ALLOC           _IOWR('m', 10,  struct mmb_info)
#define IOC_MMB_ATTR            _IOR('m',  11,  struct mmb_info)
#define IOC_MMB_FREE            _IOW('m',  12,  struct mmb_info)
#define IOC_MMB_ALLOC_V2        _IOWR('m', 13,  struct mmb_info)

#define IOC_MMB_USER_REMAP      _IOWR('m', 20,  struct mmb_info)
#define IOC_MMB_USER_REMAP_CACHED _IOWR('m', 21,  struct mmb_info)
#define IOC_MMB_USER_UNMAP      _IOWR('m', 22,  struct mmb_info)

#define IOC_MMB_ADD_REF         _IO('r', 30)    /* ioctl(file, cmd, arg), arg is mmb_addr */
#define IOC_MMB_DEC_REF         _IO('r', 31)    /* ioctl(file, cmd, arg), arg is mmb_addr */

#define IOC_MMB_FLUSH_DCACHE    _IO('c', 40)

#define IOC_MMB_FLUSH_DCACHE_DIRTY              _IOW('d', 50, struct dirty_area)
#define IOC_MMB_TEST_CACHE      _IOW('t',  11,  struct mmb_info)

#elif defined(_HI3798_)

#define HIL_MAX_NAME_LEN 16
#define HIL_MMZ_NAME_LEN HIL_MAX_NAME_LEN
#define HIL_MMB_NAME_LEN HIL_MAX_NAME_LEN

/* remove pclint waring anonymous struct or union */

struct mmb_info {
    unsigned long phys_addr;   /* phys-memory address */
    unsigned long smmu_addr;
    unsigned long align;       /* phys-memory's special align size */
    unsigned long size;        /* length of memory, in bytes*/
    void *mapped;          /* userspace mapped ptr */

    struct {
        unsigned long prot:8;  /* PROT_READ or PROT_WRITE */
        unsigned long flags:12;/* MAP_SHARED or MAP_PRIVATE */
        unsigned long reserved:12; /* reserved, do not use */
    };
    char mmb_name[HIL_MAX_NAME_LEN];
    char mmz_name[HIL_MAX_NAME_LEN];
    unsigned long gfp;     /* reserved, do set to 0 */
};

struct dirty_area {
    unsigned long dirty_phys_start;    /* dirty physical address */
    void* dirty_virt_start; /* dirty virtual  address,
    must be coherent with dirty_phys_addr */
    unsigned long dirty_size;
};

struct sec_info {
    char mmb_name[HIL_MAX_NAME_LEN];
    char mmz_name[HIL_MAX_NAME_LEN];
    unsigned long sec_smmu;
    unsigned long nosec_smmu;
    unsigned long phys_addr;
    unsigned long size;
    unsigned long align;
};

#define IOC_MMB_ALLOC               _IOWR('m', 10,  struct mmb_info)
#define IOC_MMB_ATTR                _IOR ('m', 11,  struct mmb_info)
#define IOC_MMB_FREE                _IOW ('m', 12,  struct mmb_info)
#define IOC_MMB_ALLOC_V2            _IOWR('m', 13,  struct mmb_info)
#define IOC_MMB_ALLOC_SHARE         _IOWR('m', 14,  struct mmb_info)
#define IOC_MMB_ALLOC_SHM_COM       _IOWR('m', 15,  struct mmb_info)
#define IOC_MMB_GET_SHM_COM         _IOWR('m', 16,  struct mmb_info)
#define IOC_MMB_FORCE_FREE          _IOW ('m', 17,  struct mmb_info)

#define IOC_MMB_USER_REMAP          _IOWR('m', 20,  struct mmb_info)
#define IOC_MMB_USER_REMAP_CACHED   _IOWR('m', 21,  struct mmb_info)
#define IOC_MMB_USER_UNMAP          _IOWR('m', 22,  struct mmb_info)
#define IOC_MMB_USER_GETPHYADDR     _IOWR('m', 23,  struct mmb_info)
#define IOC_MMB_USER_GETPHYADDR_S   _IOWR('m', 24,  struct mmb_info)
#define IOC_MMB_USER_CMA_MAPTO_SMMU _IOWR('m', 25,  struct mmb_info)
#define IOC_MMB_USER_CMA_UNMAPTO_SMMU   _IOWR('m', 26,  struct mmb_info)


#define IOC_MMB_SECSMMU_ALLOC       _IOWR('s', 12, struct sec_info)
#define IOC_MMB_SECSMMU_FREE        _IOWR('s', 13, struct sec_info)
#define IOC_MMB_SECSMMU_MAPTOSECSMMU      _IOWR('s', 10, struct sec_info)
#define IOC_MMB_SECSMMU_UNMAPFROMSMMU     _IOWR('s', 11, struct sec_info)

#define IOC_MMB_ADD_REF             _IO('r', 30)    /* ioctl(file, cmd, arg), arg is mmb_addr */
#define IOC_MMB_DEC_REF             _IO('r', 31)    /* ioctl(file, cmd, arg), arg is mmb_addr */

#define IOC_MMB_FLUSH_DCACHE        _IOWR('c', 40, struct mmb_info)

#define IOC_MMB_FLUSH_DCACHE_DIRTY  _IOW('d', 50, struct dirty_area)

#define IOC_MMB_TEST_CACHE          _IOW('t',  11,  struct mmb_info)

#else
     #error YOU MUST DEFINE  CHIP_TYPE!
#endif

static inline int MmzNew(int fd, int size, int align, char *mmz_name, char *mmb_name)
{
	struct mmb_info mmi = {0};
	int ret;			
    if (fd < 0) {
        printf("MmzNew fd invalid. %d\n", fd);
        return 0;
    }
	if( size ==0 || size > 0x40000000) return 0;
	mmi.size = size;
	
	mmi.align = align;
	if( mmb_name != NULL ){
        strncpy(mmi.mmb_name, mmb_name, HIL_MMB_NAME_LEN);
        mmi.mmb_name[HIL_MMB_NAME_LEN-1]='\0';
	}
	if( mmz_name != NULL ){
        strncpy(mmi.mmz_name, mmz_name, HIL_MMZ_NAME_LEN);
        mmi.mmz_name[HIL_MMZ_NAME_LEN-1]='\0';
	}

	if((ret = ioctl(fd, IOC_MMB_ALLOC, &mmi)) !=0)
			return 0;
	else
			return mmi.phys_addr;

}

static inline void *MmzMap(int fd, int phyAddr, int cached)
{

	struct mmb_info mmi = {0};
	int ret;
    
    if (fd < 0 || !phyAddr) {
        printf("MmzMap fd invalid. fd:%d phyAddr:%d\n", fd, phyAddr);
        return NULL;
    }
	if(cached != 0 && cached != 1)  return NULL;
	
	mmi.prot = PROT_READ | PROT_WRITE;
	mmi.flags = MAP_SHARED;	
	mmi.phys_addr = phyAddr;
	
	if (cached)
		ret = ioctl(fd, IOC_MMB_USER_REMAP_CACHED, &mmi);
	else
		ret = ioctl(fd, IOC_MMB_USER_REMAP, &mmi);
			
	if( ret !=0 ) return NULL;

	return (void *)mmi.mapped;

}

static inline int MmzUnmap(int fd, int phyAddr, void *pVirAddr)
{

	struct mmb_info mmi = {0};

    if (fd < 0 || !pVirAddr) {
        printf("MmzUnmap fd invalid. fd:%d pVirAddr:%p\n", fd, pVirAddr);
        return -1;
    }
	
	mmi.phys_addr = phyAddr;
	mmi.mapped = pVirAddr;
	
	return ioctl(fd, IOC_MMB_USER_UNMAP, &mmi);

}
 
static inline int MmzDelete(int fd, int phyAddr)
{
	struct mmb_info mmi = {0};

    if (fd < 0 || phyAddr == 0) {
        printf("MmzDelete fd invalid. fd:%d phyAddr:%d\n", fd, phyAddr);
        return -1;
    }
	mmi.phys_addr = phyAddr;
	
	return ioctl(fd, IOC_MMB_FREE, &mmi);
}

static inline int MmzFlush(int fd)
{
    if (fd < 0) {
        printf("MmzFlush fd invalid. %d\n", fd);
        return -1;
    }
	return ioctl(fd, IOC_MMB_FLUSH_DCACHE, NULL);
}

static inline int MmzOpen()
{
    return open("/dev/mmz_userdev", O_RDWR);
}

static inline void MmzClose(int fd)
{
    close(fd);
}

static inline int MmzAlloc(int fd, void **ppVirAddr, int *pPhyaddr, int size, int cached)
{
    *pPhyaddr = MmzNew(fd, size, 0, NULL, "fisheyeBuf");
    if (*pPhyaddr == 0) {
        printf("MmzNew failed.\n");
        return -1;
    }
    *ppVirAddr = MmzMap(fd, *pPhyaddr, cached);
    if (*ppVirAddr == NULL) {
        printf("MmzMap failed. pPhyaddr:%d\n", *pPhyaddr);
        MmzDelete(fd, *pPhyaddr);
        *pPhyaddr = 0;
        return -1;
    }
    return 0;
}

static inline void MmzFree(int fd, int phyAddr, void *pVirAddr)
{
    MmzUnmap(fd, phyAddr, pVirAddr);
    MmzDelete(fd, phyAddr);
}

static int WrapDmaBufFD(int phyAddr, int size)
{
    struct hidbe_ioctl_wrap phywrap;
    int dmabuf_fd;

    int fd = open("/dev/hi_dbe", O_RDWR);
    if (fd < 0) {
        printf("open hi_dbe failed. fd:%d\n", fd);
        return -1;
    }

    phywrap.dbe_phyaddr = phyAddr;
    phywrap.dbe_size = size;

    dmabuf_fd = ioctl(fd, DBE_COMMAND_WRAP, &phywrap);
    if (dmabuf_fd < 0) {
        printf("WrapDmaBufFD DBE_COMMAND_WRAP failed. fd:%d\n", dmabuf_fd);
        close(fd);
        return -1;
    }
    close(fd);
    
    return dmabuf_fd;
}

static NativeWindowType CreateWindow(int width, int height)
{
    fbdev_window * fb_window = (fbdev_window *)malloc(sizeof(fbdev_window)) ;

    fb_window->width = width ;
    fb_window->height = height ;

    return (NativeWindowType)fb_window ;
}

static void DestroyWindow(NativeWindowType NativeWindow)
{
    fbdev_window *fb_window = (fbdev_window*)NativeWindow ;

    free(fb_window) ;
}

static NativeDisplayType CreateDisplay(void)
{
	return (NativeDisplayType)EGL_DEFAULT_DISPLAY ;
}

static void DestroyDisplay(NativeDisplayType NativeDisplay)
{
	return ;
}

static NativePixmapType CreateMMZPixmap(int fd, int width,int height, fbdev_pixmap_format enColorFormat, void **ppVirAddr, int *pPhyaddr)
{
    int size = 0;
    NativePixmapType ret;
    struct egl_linux_pixmap *pixmap_dma = (struct egl_linux_pixmap*)malloc(sizeof(struct egl_linux_pixmap));

    memset(pixmap_dma, 0, sizeof(*pixmap_dma));
    pixmap_dma->width = width;
    pixmap_dma->height = height;
    switch(enColorFormat)
    {
        case EGL_PIXMAP_FORMAT_ABGR8888:
            size = width * height * 4;
            if (MmzAlloc(fd, ppVirAddr, pPhyaddr, size, 0) != 0) {
                free(pixmap_dma);
                return -1;
            }
            pixmap_dma->planes[0].stride = width * 4;
            pixmap_dma->planes[0].size = size;
            pixmap_dma->planes[0].offset = 0;
            pixmap_dma->handles[0].fd = WrapDmaBufFD(*pPhyaddr, size);
            if (pixmap_dma->handles[0].fd == -1) {
                MmzFree(fd, *pPhyaddr, *ppVirAddr);
                free(pixmap_dma);
                return -1;
            }
            break;

        case EGL_PIXMAP_FORMAT_NV21_BT709_WIDE:
            size = width * height * 3 / 2;
            if (MmzAlloc(fd, ppVirAddr, pPhyaddr, size, 0) != 0) {
                free(pixmap_dma);
                return -1;
            }
            pixmap_dma->planes[0].stride = width;
            pixmap_dma->planes[0].size = width * height;
            pixmap_dma->planes[0].offset = 0;
            pixmap_dma->handles[0].fd = WrapDmaBufFD(*pPhyaddr, size);
            if (pixmap_dma->handles[0].fd == -1) {
                MmzFree(fd, *pPhyaddr, *ppVirAddr);
                free(pixmap_dma);
                return -1;
            }
            pixmap_dma->planes[1].stride = width;
            pixmap_dma->planes[1].size = pixmap_dma->planes[1].stride * height/2;
            pixmap_dma->planes[1].offset = pixmap_dma->planes[0].size;
            pixmap_dma->handles[1].fd = pixmap_dma->handles[0].fd;
            break;

        default:
            free(pixmap_dma);
            return -1;
    }

    pixmap_dma->pixmap_format = enColorFormat;

    ret = (NativePixmapType)egl_create_pixmap_ID_mapping(pixmap_dma);
    if (ret == -1) {
        MmzFree(fd, *pPhyaddr, *ppVirAddr);
        close(pixmap_dma->handles[0].fd); 
        free(pixmap_dma);
    }
    return ret;
}

static void DestroyMMZPixmap(int fd, NativePixmapType pixmap, int phyAddr, void *pVirAddr)
{
    struct egl_linux_pixmap *pixmap_dma = egl_lookup_pixmap_ID_mapping(pixmap) ;

    if (pixmap_dma) {
        egl_destroy_pixmap_ID_mapping(pixmap);
    }
    if (fd >= 0) {
        MmzFree(fd, phyAddr, pVirAddr);
    }
    if (pixmap_dma && pixmap_dma->handles[0].fd >= 0) {
        close(pixmap_dma->handles[0].fd); 
    }

    if (pixmap_dma) {
        free(pixmap_dma);
    }
}

int EGL_Init(EGLContex *pEGL, int InputWidth, int InputHeight, int OutputWidth, int OutputHeight)
{
    EGLConfig config;
    EGLint matchingConfigs;
    EGLint err;
    EGLint configAttribs[] =
    {
        EGL_SAMPLES,                   0,
        EGL_RED_SIZE,                  8,
        EGL_GREEN_SIZE,                8,
        EGL_BLUE_SIZE,                 8,
        EGL_ALPHA_SIZE,                8,
        EGL_DEPTH_SIZE,               16,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_NONE,
    };

    EGLint ctxAttribs[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    pEGL->InputPixmap = -1;
    pEGL->OutputPixmap = -1;
    
    pEGL->MmzFd = MmzOpen();
    if (pEGL->MmzFd < 0) {
        printf("MmzOpen failed. fd:%d\n", pEGL->MmzFd);
        return -1; 
    }
    pEGL->NativeDisplay = CreateDisplay();
    pEGL->NativeWindow = CreateWindow(OutputWidth/4, OutputHeight*3/2);
    if (!pEGL->NativeWindow) {
        printf("CreateWindow failed.\n");
        EGL_Uninit(pEGL);
        return -3;
    }
    
    pEGL->InputPixmap = CreateMMZPixmap(pEGL->MmzFd, InputWidth, InputHeight, 
                                        EGL_PIXMAP_FORMAT_NV21_BT709_WIDE,
                                        &pEGL->InputVirAddr, &pEGL->InputPhyAddr);
    if (pEGL->InputPixmap < 0) {
        printf("CreateMMZPixmap InputPixmap failed.\n");
        EGL_Uninit(pEGL);
        return -4;
    }
                        
    pEGL->OutputPixmap = CreateMMZPixmap(pEGL->MmzFd, OutputWidth/4, OutputHeight*3/2, 
                                        EGL_PIXMAP_FORMAT_ABGR8888,
                                        &pEGL->OutputVirAddr, &pEGL->OutputPhyAddr);
    if (pEGL->OutputPixmap < 0) {
        printf("CreateMMZPixmap OutputPixmap failed.\n");
        EGL_Uninit(pEGL);
        return -5;
    }
    
    eglBindAPI(EGL_OPENGL_ES_API);
    pEGL->EglDisplay = eglGetDisplay(pEGL->NativeDisplay);
    eglInitialize(pEGL->EglDisplay, NULL, NULL);
    eglChooseConfig(pEGL->EglDisplay, configAttribs, &config, 1, &matchingConfigs);
    pEGL->EglSurface = eglCreatePixmapSurface(pEGL->EglDisplay, config, pEGL->OutputPixmap, NULL);
    pEGL->EglContext = eglCreateContext(pEGL->EglDisplay, config, NULL, ctxAttribs);
    eglMakeCurrent(pEGL->EglDisplay, pEGL->EglSurface, pEGL->EglSurface, pEGL->EglContext);
    if (EGL_SUCCESS != (err = eglGetError()))
    {
        printf("eglGetError = 0x%x\n", err);
        EGL_Uninit(pEGL);
        return -6;
    }

    pEGL->image = eglCreateImageKHR(pEGL->EglDisplay, NULL, EGL_NATIVE_PIXMAP_KHR, (EGLClientBuffer)pEGL->InputPixmap, NULL);

    if (!pEGL->image) {
        printf("eglCreateImageKHR failed.\n");
        EGL_Uninit(pEGL);
        return -7;
    }
    pEGL->InputWidth  = InputWidth;
    pEGL->InputHeight = InputHeight;
    pEGL->OutputWidth  = OutputWidth;
    pEGL->OutputHeight = OutputHeight;
    
    return 0;
}

void EGL_Uninit(EGLContex *pEGL)
{
    if (pEGL->EglDisplay) {
        eglMakeCurrent(pEGL->EglDisplay, NULL, NULL, NULL);
    }

    if (pEGL->EglContext)
        eglDestroyContext(pEGL->EglDisplay, pEGL->EglContext);

    if (pEGL->EglSurface)
        eglDestroySurface(pEGL->EglDisplay, pEGL->EglSurface);

    eglTerminate(pEGL->EglDisplay);
    if (pEGL->image) {
        eglDestroyImageKHR(pEGL->NativeDisplay, pEGL->image);
    }

    if (pEGL->NativeWindow) {
        DestroyWindow(pEGL->NativeWindow);
    }

    if (pEGL->NativeDisplay) {
        DestroyDisplay(pEGL->NativeDisplay);
    }

    if (pEGL->InputPixmap != -1) {
        DestroyMMZPixmap(pEGL->MmzFd, pEGL->InputPixmap, pEGL->InputPhyAddr, pEGL->InputVirAddr);
    }

    if (pEGL->OutputPixmap != -1) {
        DestroyMMZPixmap(pEGL->MmzFd, pEGL->OutputPixmap, pEGL->OutputPhyAddr, pEGL->OutputVirAddr);
    }

    if (pEGL->MmzFd >= 0) {
        MmzClose(pEGL->MmzFd);
    }
}


