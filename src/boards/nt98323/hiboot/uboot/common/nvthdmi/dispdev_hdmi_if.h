#ifndef __DISPDEV_HDMI_IF_H__
#define __DISPDEV_HDMI_IF_H__

typedef void (*FP)(UINT32 data);

typedef struct {
	UINT32 	vid;		/* HDMI_VIDEOID */
	UINT32	output_mode;/* HDMI_MODE */
	UINT32	audio_fmt; 	/* HDMI_AUDIOFMT */
	UINT32	source; 	/* HDMI_source_selset*/
	UINT32	pclk;       /* HDMI_pclk*/
} DISP_HDMI_CFG, *PDISP_HDMI_CFG;

/* callback data */
enum {
	DISPDEV_TYPE_HDMI_16BIT = 0x100000,
	DISPDEV_TYPE_EMBD_HDMI,
};


/**
    Display Device Object Structure

    This field is the display device object structure which is used to open/close display device
    and also provides the interface to access the display device registers.
    This structure is used by the DISPLAY Object driver in the driver design. Normal user should not access the
    display device by the device object directly.
*/
typedef struct {
	ER (*open)(void);                             	///< Open the display device
	ER (*close)(void);                            	///< Close the display device
	UINT32 (*edidRead)(void);               		///< Edid table
	ER (*preSetConfig)(PDISP_HDMI_CFG config);     	///< Set HDMI config before open
	void (*setDevIOCtrl)(FP);             			///< Set Display Device IOCTRL API.
	void (*isr_routine)(void);						///< HDMI ISR
} DISPDEV_OBJ, *PDISPDEV_OBJ;

/**
    Get HDMI Display Device Object
    Get HDMI Display Device Object. This Object is used to control the HDMI Transmitter.
    @return The pointer of HDMI Display Device object.
*/
PDISPDEV_OBJ dispdev_get_hdmi_dev_obj(void);

/* HDMI ISR
*/
extern void  hdmi_isr(void);

#endif /* __DISPDEV_HDMI_IF_H__ */