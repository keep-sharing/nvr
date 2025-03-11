/*
 * ***************************************************************
 * Filename:        reco_rtsp_buffer.h
 * Created at:      2021.03.18
 * Description:     reco rtsp buffer api.
 * Author:          david
 * Copyright (C)    milesight
 * ***************************************************************
 */

#ifndef __RECO_RTSP_BUFFER_H__
#define __RECO_RTSP_BUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

void rtsp_buffer_show();
void *rtsp_buffer_malloc(int size);
void *rtsp_buffer_calloc(int n, int size);
void rtsp_buffer_free(void *p, int size);
void rtsp_buffer_init(int bitrate);
void rtsp_buffer_uninit();

void msrtsp_buffer_malloc(void **dst, int size, const char *file, int line);
void msrtsp_buffer_calloc(void **dst, int n, int size, const char *file, int line);
void msrtsp_buffer_free(void *p, int size, const char *file, int line);


#ifdef __cplusplus
}
#endif

#endif
