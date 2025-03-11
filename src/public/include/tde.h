#ifndef DVR_TDE_H
#define DVR_TDE_H

#ifdef __cplusplus
extern "C" {
#endif

int tde_fb_init(void);
int tde_fb_uninit(void);
int tde_fb_scale(int enScreen, int srcX, int srcY, int srcWidth, int srcHeight, int *isMinifying);


#ifdef __cplusplus
}
#endif

#endif // DVR_LOG_H
