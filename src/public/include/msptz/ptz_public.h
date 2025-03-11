/**
 *@file ptz_public.h
 *@date Created on: 2013-7-24
 *@author root
 */

#ifndef PTZ_PUBLIC_H_
#define PTZ_PUBLIC_H_

#ifndef PRESET_MAX
#define PRESET_MAX  300
#endif

#define PTZ_VERSION 63//63以上版本快捷预置点名称通过onvif获取


#ifndef TOUR_MAX
#define TOUR_MAX	8
#endif

#ifndef MS_KEY_MAX
#define MS_KEY_MAX  48
#endif

#ifndef STREAM_MAX
#define STREAM_MAX	5
#endif


#define	PRESET_ENABLE		(1)
#define	PRESET_CLR_ENABLE	(1 << 1)
#define	TRACK_ENABLE		(1 << 2)
#define	TOUR_ENABLE         (1 << 3)
#define TOUR_SPD_ENABLE     (1 << 4)
#define TOUR_TIME_ENABLE    (1 << 5)

#define PTZ_ALL_FUN_ENABLE  (PRESET_ENABLE | PRESET_CLR_ENABLE | TRACK_ENABLE | TOUR_ENABLE | TOUR_SPD_ENABLE | TOUR_TIME_ENABLE)

typedef enum {
    B_1200 = 0,
    B_1800,
    B_2400,
    B_4800,
    B_9600,
    B_19200,
    B_38400,
    B_57600,
    B_115200,
    B_MAX
}BAUDRATE;

typedef enum {
    D5 = 0,
    D6,
    D7,
    D8,
    D_MAX
}DATABIT;

typedef enum {
    PARITY_NONE = 0,
    PARITY_ODD,
    PARITY_EVEN,
    PARITY_MAX
}PARITYBIT;

typedef enum ptz_protocol{
    PTZ_PROTOCOL_1602 = 0,
    PTZ_PROTOCOL_A01,
    PTZ_PROTOCOL_AB_D,
    PTZ_PROTOCOL_AB_P,
    PTZ_PROTOCOL_ADV,
    PTZ_PROTOCOL_HIK,
    PTZ_PROTOCOL_LILIN,
    PTZ_PROTOCOL_PANASONIC_CS850,
    PTZ_PROTOCOL_PELCO_D,
    PTZ_PROTOCOL_PELCO_P,
    PTZ_PROTOCOL_SAMSUNG_E,
    PTZ_PROTOCOL_SAMSUNG_T,
    PTZ_PROTOCOL_SONY_D70,
    PTZ_PROTOCOL_YAAN,
    PTZ_PROTOCOL_MAX
} MS_PTZ_PROTOCOL;

typedef enum ptz_action{
/******basic action start***********/
    PTZ_UP = 0,
    PTZ_DOWN,
    PTZ_LEFT,
    PTZ_RIGHT,
    PTZ_LEFT_UP,
    PTZ_RIGHT_DOWN,
    PTZ_LEFT_DOWN,
    PTZ_RIGHT_UP,
    PTZ_IRIS_PLUS,
    PTZ_IRIS_MINUS,
    PTZ_ZOOM_PLUS,		//10
    PTZ_ZOOM_MINUS,
    PTZ_AUTO_FOCUS,
    PTZ_FOCUS_PLUS,
    PTZ_FOCUS_MINUS,
    PTZ_FOCUS_STOP,
    PTZ_AUTO_SCAN,
    PTZ_LIGHT_ON,
    PTZ_LIGHT_OFF,
    PTZ_BRUSH_ON,
    PTZ_BRUSH_OFF,	  //20	
    PTZ_STOP_ALL,
    /**!preset operation start*/
    PTZ_PRESET_SET,
    PTZ_PRESET_GOTO,
    PTZ_PRESET_CLEAR,
    /**!preset operation end*/
    /**!track operation start*/
    PTZ_PATTERN_START,
    PTZ_PATTERN_RUN,
    PTZ_PATTERN_STOP,
    /**!track operation end*/
/*****basic action end******************/
    PTZ_PRESET_STOP, ///< reserve
    PTZ_PRESET_AUTO_SCAN, ///< reserve
    /**********preset tour**********/
    PTZ_PRESET_TOUR_STOP, ///< reserve		//30
    PTZ_PRESET_TOUR_END, ///< reserve
    PTZ_PRESET_TOUR_ADD, ///< reserve
    PTZ_PRESET_TOUR_RUN, ///< reserve
    PTZ_PRESET_SCAN_ON, ///< reserve
    /********preset tour end*****/
    PTZ_CREATE_PRESET_TOUR, ///< reserve
    PTZ_CLEAR_PRESET_TOUR, ///< reserve
    PTZ_SET_SRL_PROTO, ///< reserve
    PTZ_SET_SRL_PORT, ///< reserve
    PTZ_SET_SPEED, ///< reserve
    
	PTZ_PATTERN_DEL
}PTZ_ACTION_E;

//speed: 1-8
struct speed {
	int pan;
	int tilt;
	int zoom;
	int focus;
	int timeout; ///< reserved
};

struct serial_port {
	int baud_rate;
	int parity_bit;
        int data_bit;
	int stop_bit;
	int proto;
	int addr;
};

struct tour_prm {
	int preno; ///< preset number
	int tour_spd; ///< tour speed
	int tour_time; ///< tour scan time
};

struct ptz_limit {
	int fun_avail; ///< function available
	int max_preset;
	int max_tour;
	int max_track;
	int max_addr;
	int max_tour_time;
	int max_tour_speed;
};


//hrz.milesight for 8.0.6
typedef enum fisheye_ptz_cmd{
	FISHEYE_PTZ_STOP_ALL = 0,
	FISHEYE_PTZ_UP,
    FISHEYE_PTZ_DOWN,
    FISHEYE_PTZ_LEFT,
    FISHEYE_PTZ_RIGHT,
    FISHEYE_PTZ_TOP_RIGHT,
    FISHEYE_PTZ_TOP_LEFT,
    FISHEYE_PTZ_BOTTOM_RIGHT,
    FISHEYE_PTZ_BOTTOM_LEFT,	//8

	
    FISHEYE_PTZ_ZOOM_IN = 32,
    FISHEYE_PTZ_ZOOM_OUT,
    FISHEYE_PTZ_FOCUS_NEAR,
    FISHEYE_PTZ_FOCUS_FAR,
    FISHEYE_PTZ_IRIS_CLOSE,
    FISHEYE_PTZ_IRIS_OPEN,	//37


    FISHEYE_PTZ_ZOOM_AUTO = 48,
    FISHEYE_PTZ_FOCUS_AUTO,
    FISHEYE_PTZ_IRIS_AUTO,
    

    FISHEYE_PTZ_PRESET_POINT_SET = 64,
    FISHEYE_PTZ_PRESET_POINT_CLEAR,
    FISHEYE_PTZ_PRESET_POINT_GOTO,
    
  
	FISHEYE_PTZ_PRESET_TOUR_RUN = 70,
    FISHEYE_PTZ_PRESET_TOUR_STOP, 
    FISHEYE_PTZ_PRESET_TOUR_SET, 
    FISHEYE_PTZ_PRESET_TOUR_ADD,
	FISHEYE_PTZ_PRESET_TOUR_CLEAR,
	FISHEYE_PTZ_PRESET_TOUR_GET,
	FISHEYE_PTZ_POSITION_SET,
	FISHEYE_PTZ_POSITION_GET,
	FISHEYE_PTZ_POSITION_3D_SET,		//78


	FISHEYE_PTZ_AUTOPAN_ON = 80,		//80
	FISHEYE_PTZ_AUTOPAN_OFF,
	FISHEYE_PTZ_ZONESCAN_ON,
	FISHEYE_PTZ_ZONESCAN_OFF,
	FISHEYE_PTZ_LIGHT_ON,
	FISHEYE_PTZ_LIGHT_OFF,				//85
	

	FISHEYE_PTZ_FLIP_ON = 128,
	FISHEYE_PTZ_FLIP_OFF,
	FISHEYE_PTZ_REMOTE_RESET,

}FISHEYE_PTZ_CMD;


int ptz_init(int chn_cnt, const char *dev);

int ptz_deinit(void);

int ptz_ctrl(int action, int chn);

int ptz_set_serial_port(int chn, const struct serial_port *srl_port);

int ptz_set_speed(int chn, const struct speed *spd);

int ptz_get_limit(int chn, struct ptz_limit *srl);

int ptz_op_preset(int chn, int preno, int act);

int ptz_add_key_point(int chn, int tour_id, const struct tour_prm *prm);

int ptz_del_key_point(int chn, int tour_id, int key_idx);

int ptz_key_point_up(int chn, int tour_id, int key_idx);

int ptz_key_point_down(int chn, int tour_id, int key_idx);

int ptz_tour_run(int chn, int tour_id);

int ptz_tour_stop(int chn, int tour_id);

int ptz_tour_clear(int chn, int tour_id);

int ptz_op_track(int chn, int track_id, int act);

#endif /* PTZ_PUBLIC_H_ */
