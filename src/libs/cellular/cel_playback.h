/**
 * @file cel_playback.h
 * @date Created on: 2013-7-10
 * @author chimmu
 */

#ifndef CEL_PLAYBACK_H_
#define CEL_PLAYBACK_H_

#include <pthread.h>
#include "cel_muldec.h"
#include "list.h"
/// 回放状态
enum pb_state {
	PB_INVALID = 0,
	PB_START,
	PB_PAUSE,
	PB_SPEED,
	PB_STEP,
	PB_STOP,
	PB_SINGLE_FRAME,
	PB_PAUSE_SNAPSHOT,//bruce.milesight add
	PB_MAX_STATE
};

typedef struct i_ts { // iframe interval
	unsigned long long first;
	unsigned long long second;
}I_TS;

struct pb_session {
	T_CELMD_CTRL *celmd_ctrl; ///< cellular 回放控制参数
	CALLBACK_DEC cb_dec; ///< 解码回调函数
	void *dec_arg; ///< 解码回调函数私有参数
	CALLBACK_EXCEPTION cb_exc; ///<  异常处理函数
	void *exc_arg; ///< 异常处理参数
	struct ms_exception except;
	pthread_mutex_t lock;
	pthread_t tid; ///<　线程id
	int stop; ///< 是否退出线程
	int sid; ///< 会话id
	CH_MASK ch_mask; ///< 要打开的通道掩码，通道号从１开始
	int chncnt; ///< opened channel counts
	int state; ///< 当前回放状态
	int prev_state; ///< 上次状态
	int speed; ///< 回放速度
	int direction; ///< 回放方向
#if 0
	int init_pb_time[CEL_MAX_CHANNEL]; ///< 开始回放时间
	unsigned long long start_time[CEL_MAX_CHANNEL];
	unsigned long long prv_time_sec[CEL_MAX_CHANNEL];
	unsigned long long prv_time_usec[CEL_MAX_CHANNEL];
	unsigned long long start_play_time[CEL_MAX_CHANNEL];
	unsigned long long start_play_time_msec[CEL_MAX_CHANNEL];
	unsigned long long start_ref_time_msec[CEL_MAX_CHANNEL];
	unsigned long long now_play_time[CEL_MAX_CHANNEL];
#endif

	unsigned long long *init_pb_time;
	unsigned long long *start_time;
	unsigned long long *prv_time_sec;
	unsigned long long *prv_time_usec;
	unsigned long long *start_play_time;
	unsigned long long *start_play_time_msec;
	unsigned long long *start_ref_time_msec;
//	unsigned long long *now_play_time;

	int *frame_cnt;
	I_TS *its; ///< time stamp  of two neighbor iframes

//	struct pb_session *next;
//	struct pb_session *prev;
	struct list_head list;
};

struct pb_ctrl {
//	struct pb_session *head; ///< session head
//	struct pb_session *tail; ///< session tail
	struct list_head head;
	int ss_cnt; ///< session counts
	int chn_cnt; ///< max channel
	CALLBACK_EXCEPTION cb_exc; ///<  异常处理函数
	void *exc_arg; ///< 异常处理参数
	pthread_mutex_t mutex;
	int maxpb; // max playback session
};

int celpb_init(int chn_cnt, CALLBACK_EXCEPTION fn, void *arg, int maxpb);

int celpb_deinit(void);

#endif /* CEL_PLAYBACK_H_ */
