#ifndef _ACODEC_H_
#define _ACODEC_H_

#define IOC_TYPE_ACODEC 'A'

typedef enum hiACODEC_FS_E {
	ACODEC_FS_48000 =   0x1a,
	ACODEC_FS_24000 =   0x19,
	ACODEC_FS_12000 =   0x18,

	ACODEC_FS_44100 =   0x1a,
	ACODEC_FS_22050 =   0x19,
	ACODEC_FS_11025 =   0x18,

	ACODEC_FS_32000 =   0x1a,
	ACODEC_FS_16000 =   0x19,
	ACODEC_FS_8000  =   0x18,

	ACODEC_FS_64000 =   0x1b,

	ACODEC_FS_96000 =   0x1b,

	ACODEC_FS_BUTT = 0x1c,
} ACODEC_FS_E;

typedef enum hiACODEC_MIXER_E {
	
    ACODEC_MIXER_MICIN_D   = 0x0,
    ACODEC_MIXER_MICIN     = 0x1,
    ACODEC_MIXER_MICLINE_D = 0x2,
    ACODEC_MIXER_LINEIN    = 0x3,
    ACODEC_MIXER_LINEIN_D  = 0x4,

	ACODEC_MIXER_BUTT,
} ACODEC_MIXER_E;

typedef enum hiACODEC_LINTOUTD_E {
	ACODEC_LINEOUTD_LEFT     = 0x0,
	ACODEC_LINEOUTD_RIGHT    = 0x1,
	ACODEC_LINEOUTD_NONE     = 0x2,

	ACODEC_LINEOUTD_BUTT,
} ACODEC_LINEOUTD_E;


typedef struct {
	/*volume control, 0x00~0x7e, 0x7F:mute*/
	unsigned int vol_ctrl;
	/*adc/dac mute control, 1:mute, 0:unmute*/
	unsigned int vol_ctrl_mute;
} ACODEC_VOL_CTRL;

typedef enum hiACODEC_IOCTL_E {
	IOC_NR_SOFT_RESET_CTRL = 0x0,
	IOC_NR_POWER_DOWN_CTRL,	
	/******************************************************************************************
	The input volume range is [-113, +86]. Both the analog gain and digital gain are adjusted.
	A larger value indicates higher volume. 
	For example, the value 86 indicates the maximum volume of 86 dB, 
	and the value -113 indicates the minimum volume (muted status). 
	The volume adjustment takes effect simultaneously in the audio-left and audio-right channels. 
	The recommended volume range is [-16, +56]. 
	Within this range, the noises are lowest because only the analog gain is adjusted, 
	and the voice quality can be guaranteed.
	*******************************************************************************************/
	IOC_NR_SET_INPUT_VOL ,
	/*******************************************************************************************
	The output volume range is [-121, +6]. A larger value indicates higher volume. 
	For example, the value 6 indicates the maximum volume of 6 dB, 
	and the value -121 indicates the minimum volume (muted status). 
	The volume adjustment takes effect simultaneously in the audio-left and audio-right channels. 
	The digital gain is adjusted by calling this interface. 
	It is recommended that a small value is assigned to avoid noises.
	*******************************************************************************************/
	IOC_NR_SET_OUTPUT_VOL,
	IOC_NR_GET_INPUT_VOL,
	IOC_NR_GET_OUTPUT_VOL,
	/***********************/
	IOC_NR_SET_I2S1_FS,
	IOC_NR_SET_DAC_LINEOUTD,
	IOC_NR_SET_MIXER_MIC,
	IOC_NR_SEL_DAC_CLK,
	IOC_NR_SEL_ADC_CLK,
	IOC_NR_SEL_ANA_MCLK,
	IOC_NR_SET_GAIN_MICL,
	IOC_NR_SET_GAIN_MICR,
	IOC_NR_SET_GAIN_LINEOUTD,
	IOC_NR_SET_DACL_VOL,
	IOC_NR_SET_DACR_VOL,
	IOC_NR_SET_ADCL_VOL,
	IOC_NR_SET_ADCR_VOL,
	IOC_NR_SET_MICL_MUTE,
	IOC_NR_SET_MICR_MUTE,
	IOC_NR_SET_DACD_MUTE,
	IOC_NR_SET_DACL_MUTE,
	IOC_NR_SET_DACR_MUTE,
	IOC_NR_DAC_SOFT_MUTE,
	IOC_NR_DAC_SOFT_UNMUTE,
	IOC_NR_BOOSTL_ENABLE,
	IOC_NR_BOOSTR_ENABLE,

	IOC_NR_GET_GAIN_MICL,
	IOC_NR_GET_GAIN_MICR,
	IOC_NR_GET_DACL_VOL,
	IOC_NR_GET_DACR_VOL,
	IOC_NR_GET_ADCL_VOL,
	IOC_NR_GET_ADCR_VOL,

	IOC_NR_SET_PD_DACL,
	IOC_NR_SET_PD_DACR,
	IOC_NR_SET_PD_ADCL,
	IOC_NR_SET_PD_ADCR,
	IOC_NR_SET_PD_LINEINL,
	IOC_NR_SET_PD_LINEINR,
	IOC_NR_SET_PD_LINEOUTD,

	IOC_NR_DACL_SEL_TRACK,
	IOC_NR_DACR_SEL_TRACK,
	IOC_NR_ADCL_SEL_TRACK,
	IOC_NR_ADCR_SEL_TRACK,
	IOC_NR_SET_DAC_DE_EMPHASIS,
	IOC_NR_SET_ADC_HP_FILTER,  
	IOC_NR_DAC_POP_FREE,
	IOC_NR_DAC_SOFT_MUTE_RATE,

	IOC_NR_DAC_SEL_I2S,
	IOC_NR_ADC_SEL_I2S,
	IOC_NR_SET_I2S1_DATAWIDTH,
	IOC_NR_SET_I2S2_DATAWIDTH,
	IOC_NR_SET_I2S2_FS,
	IOC_NR_SET_DACR2DACL_VOL,
	IOC_NR_SET_DACL2DACR_VOL,
	IOC_NR_SET_ADCR2DACL_VOL,
	IOC_NR_SET_ADCL2DACR_VOL,
	IOC_NR_SET_ADCR2DACR_VOL,
	IOC_NR_SET_ADCL2DACL_VOL,
} ACODEC_IOCTL_E;

/*reset the audio code to the default config*/
#define ACODEC_SOFT_RESET_CTRL \
	_IO(IOC_TYPE_ACODEC, IOC_NR_SOFT_RESET_CTRL)
#define ACODEC_POWER_DOWN_CTRL \
	_IO(IOC_TYPE_ACODEC, IOC_NR_POWER_DOWN_CTRL)	
/*ACODEC_FS_E*/
#define ACODEC_SET_I2S1_FS \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_I2S1_FS, unsigned int)
/*select the differential output source, left channel or right channel*/
#define ACODEC_SET_DAC_LINEOUTD \
    _IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_DAC_LINEOUTD, unsigned int)
/*select the micpga's input, micin linein, or differential input(ACODEC_MIXER_E)*/
#define ACODEC_SET_MIXER_MIC \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_MIXER_MIC, unsigned int)
/*analog part input volume control(left channel 0~0x1f)*/
#define ACODEC_SET_GAIN_MICL \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_GAIN_MICL, unsigned int)
/*analog part input volume control(right channel 0~0x1f)*/
#define ACODEC_SET_GAIN_MICR \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_GAIN_MICR, unsigned int)
/*lineout gain sel: 0: 0db, 1: -6db*/
#define ACODEC_SET_GAIN_LINEOUTD \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_GAIN_LINEOUTD, unsigned int)
/*Output volume control(left channel) ACODEC_VOL_CTRL*/
#define ACODEC_SET_DACL_VOL \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_DACL_VOL, ACODEC_VOL_CTRL)
/*Output volume control(right channel) ACODEC_VOL_CTRL*/
#define ACODEC_SET_DACR_VOL \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_DACR_VOL, ACODEC_VOL_CTRL)
/*Input volume control(left channel) ACODEC_VOL_CTRL*/
#define ACODEC_SET_ADCL_VOL \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_ADCL_VOL, ACODEC_VOL_CTRL)
/*Input volume control(right channel) ACODEC_VOL_CTRL*/
#define ACODEC_SET_ADCR_VOL \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_ADCR_VOL, ACODEC_VOL_CTRL)
/*Input mute control(left channel), 1:mute, 0:unmute*/
#define ACODEC_SET_MICL_MUTE \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_MICL_MUTE, unsigned int)
/*Input mute control(right channel), 1:mute, 0:unmute*/
#define ACODEC_SET_MICR_MUTE \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_MICR_MUTE, unsigned int)
/*Output mute control(differential output), 1:mute, 0:unmute*/
#define ACODEC_SET_DACD_MUTE \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_DACD_MUTE, unsigned int)
/*Output mute control(left channel), 1:mute, 0:unmute*/
#define ACODEC_SET_DACL_MUTE \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_DACL_MUTE, unsigned int)
/*Output mute control(right channel), 1:mute, 0:unmute*/
#define ACODEC_SET_DACR_MUTE \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_DACR_MUTE, unsigned int)
/*Audio Fade Out Control, 1:on, 0:off*/
#define ACODEC_DAC_SOFT_MUTE \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_DAC_SOFT_MUTE, unsigned int)
/*Audio Fade In Control, 1:on, 0:off*/
#define ACODEC_DAC_SOFT_UNMUTE \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_DAC_SOFT_UNMUTE, unsigned int)
/*Audio AD BOOST Control, 1:on, 0:off*/
#define ACODEC_ENABLE_BOOSTL \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_BOOSTL_ENABLE, unsigned int)
#define ACODEC_ENABLE_BOOSTR \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_BOOSTR_ENABLE, unsigned int)

#define ACODEC_GET_GAIN_MICL \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_GET_GAIN_MICL, unsigned int)
#define ACODEC_GET_GAIN_MICR \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_GET_GAIN_MICR, unsigned int)
#define ACODEC_GET_DACL_VOL \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_GET_DACL_VOL, ACODEC_VOL_CTRL)
#define ACODEC_GET_DACR_VOL \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_GET_DACR_VOL, ACODEC_VOL_CTRL)
#define ACODEC_GET_ADCL_VOL \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_GET_ADCL_VOL, ACODEC_VOL_CTRL)
#define ACODEC_GET_ADCR_VOL \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_GET_ADCR_VOL, ACODEC_VOL_CTRL)

/*set adcl power, 0: power up, 1: power down*/
#define  ACODEC_SET_PD_DACL \
        _IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_PD_DACL, unsigned int)
/*set adcr power, 0: power up, 1: power down*/
#define  ACODEC_SET_PD_DACR \
        _IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_PD_DACR, unsigned int)
/*set adcl power, 0: power up, 1: power down*/
#define  ACODEC_SET_PD_ADCL \
        _IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_PD_ADCL, unsigned int)
/*set adcr power, 0: power up, 1: power down*/
#define  ACODEC_SET_PD_ADCR \
        _IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_PD_ADCR, unsigned int)
/*set adcl power, 0: power up, 1: power down*/
#define  ACODEC_SET_PD_LINEINL \
        _IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_PD_LINEINL, unsigned int)
/*set adcr power, 0: power up, 1: power down*/
#define  ACODEC_SET_PD_LINEINR \
        _IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_PD_LINEINR, unsigned int)
/*set adcr power, 0: power up, 1: power down*/
#define  ACODEC_SET_PD_LINEOUTD \
        _IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_PD_LINEOUTD, unsigned int)
	
/* Don't need to set, the driver will set a default value */
/*clock of dac and adc is reverse or obverse*/
#define ACODEC_SEL_DAC_CLK \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SEL_DAC_CLK, unsigned int)
#define ACODEC_SEL_ADC_CLK \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SEL_ADC_CLK, unsigned int)
/*clock of analog part and digital part is reverse or obverse*/
#define ACODEC_SEL_ANA_MCLK \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SEL_ANA_MCLK, unsigned int)
#define ACODEC_DACL_SEL_TRACK \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_DACL_SEL_TRACK, unsigned int)
#define ACODEC_DACR_SEL_TRACK \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_DACR_SEL_TRACK, unsigned int)
#define ACODEC_ADCL_SEL_TRACK \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_ADCL_SEL_TRACK, unsigned int)
#define ACODEC_ADCR_SEL_TRACK \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_ADCR_SEL_TRACK, unsigned int) 
#define ACODEC_SET_DAC_DE_EMPHASIS \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_DAC_DE_EMPHASIS, unsigned int)
#define ACODEC_SET_ADC_HP_FILTER \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_ADC_HP_FILTER, unsigned int)
#define ACODEC_DAC_POP_FREE \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_DAC_POP_FREE, unsigned int)
#define ACODEC_DAC_SOFT_MUTE_RATE \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_DAC_SOFT_MUTE_RATE, unsigned int)

#define ACODEC_SET_INPUT_VOL \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_INPUT_VOL, unsigned int)
#define ACODEC_SET_OUTPUT_VOL \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_OUTPUT_VOL, unsigned int)
#define ACODEC_GET_INPUT_VOL \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_GET_INPUT_VOL, unsigned int)
#define ACODEC_GET_OUTPUT_VOL \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_GET_OUTPUT_VOL, unsigned int)

/* Reserved ioctl cmd */
#define ACODEC_DAC_SEL_I2S \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_DAC_SEL_I2S, unsigned int)
#define ACODEC_ADC_SEL_I2S \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_ADC_SEL_I2S, unsigned int)
#define ACODEC_SET_I2S1_DATAWIDTH \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_I2S1_DATAWIDTH, unsigned int)
#define ACODEC_SET_I2S2_DATAWIDTH \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_I2S2_DATAWIDTH, unsigned int)
#define ACODEC_SET_I2S2_FS \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_I2S2_FS, unsigned int)
#define ACODEC_SET_DACR2DACL_VOL \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_DACR2DACL_VOL, ACODEC_VOL_CTRL)
#define ACODEC_SET_DACL2DACR_VOL \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_DACL2DACR_VOL, ACODEC_VOL_CTRL)
#define ACODEC_SET_ADCL2DACL_VOL \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_ADCL2DACL_VOL, ACODEC_VOL_CTRL)
#define ACODEC_SET_ADCR2DACL_VOL \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_ADCR2DACL_VOL, ACODEC_VOL_CTRL)
#define ACODEC_SET_ADCL2DACR_VOL \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_ADCL2DACR_VOL, ACODEC_VOL_CTRL)
#define ACODEC_SET_ADCR2DACR_VOL \
	_IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_ADCR2DACR_VOL, ACODEC_VOL_CTRL)

#endif /* End of #ifndef _ACODEC_H_ */
