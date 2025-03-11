#ifndef __FE_EGL__
#define __FE_EGL__

#ifdef __cplusplus
extern "C" {
#endif

#include <EGL/egl.h>
#include <EGL/eglext.h>

typedef struct {
    NativeDisplayType NativeDisplay;
    NativeWindowType NativeWindow;
    NativePixmapType InputPixmap ;
    NativePixmapType OutputPixmap;

    EGLDisplay EglDisplay;
    EGLContext EglContext;
    EGLSurface EglSurface;
    
    void *InputVirAddr;
    void *OutputVirAddr;
    EGLint InputPhyAddr;
    EGLint OutputPhyAddr;
    EGLImageKHR image;
    EGLint InputWidth;
    EGLint InputHeight;
    EGLint OutputWidth;
    EGLint OutputHeight;
    EGLint MmzFd;
}EGLContex; 

int  EGL_Init(EGLContex *pEGL, int InputWidth, int InputHeight, int OutputWidth, int OutputHeight);
void EGL_Uninit(EGLContex *pEGL);

#ifdef __cplusplus
}
#endif
#endif
