#ifndef __HDMI_IF_H__
#define __HDMI_IF_H__

typedef enum {
	HDMI_VIDEO_720P = 0,
	HDMI_VIDEO_1080P,
	HDMI_VIDEO_1024x768,
} hdmi_video_t;

int hdmi_if_init(hdmi_video_t video);

#endif /* hdmi_video_t */