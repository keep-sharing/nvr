#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#ifdef __APPLE__
#include"cJSON/cJSON.h"
#elif defined(__linux__)
#include"cJSON.h"
#else
#include "cJSON\cJSON.h"
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef SUCCEEDED
#define SUCCEEDED(hr) (((int)(hr)) >= 0)
#define FAILED(hr) (((int)(hr)) < 0) 
#endif
#define DEBUG_PRINTF printf 

#define FISHEYE_DEBUG(string) DEBUG_PRINTF("####LINE: %d == FUNC: %s == %s \n", __LINE__, __func__, string)
#define DEBUG_WENGX() DEBUG_PRINTF("####LINE: %d == FUNC: %s\n", __LINE__, __func__)
#define DEBUG_WARNING(string) DEBUG_PRINTF("####LINE: %d == FUNC: %s == WARNING: [%s] \n", __LINE__, __func__, string)
#ifndef  NULL
#define NULL 0
#endif // ! NULL

#define INLINE	
#define MS_PI	(3.14159265358979323846f)
#define AngleToRadian( degree )		((degree) * (MS_PI / 180.0f))
#define RadianToAngle( radian )		((radian) * (180.0f / MS_PI))
#define FISHEYE_LBUTTON	1
#define MAX_NAME_LENGTH	16

#define TOUCH_WIDTH		2.0f
#define PRESS_WIDTH		8.0f
#define NORMAL_WIDTH	1.0f
/*
 *	逻辑坐标系说明： 以左上角为原点(0, 0)
 *	x, y 逻辑矩形的起始点
 *	w, h 逻辑矩形的宽高
 */
typedef struct MSRectf {
	float x, y;
	float w, h;
}MSRectf;
typedef struct MSPointf {
	float x, y;
}MSPointf;
#define MSRectf_DefaultValue (MSRectf){\
.x = 0.0f, \
.y = 0.0f, \
.w = 1.0f, \
.h = 1.0f \
}
int MSRectf_InitByCJSON(MSRectf* rectf, const cJSON* json); 
int MSRectf_Contains(const MSRectf* rect_f, float x, float y); 

/*
 *	物理坐标系说明： 以左上角为原点(0, 0)
 *	x, y 逻辑矩形的起始点
 *	w, h 逻辑矩形的宽高
 */
typedef struct {
	int x, y;
	int w, h;
}MSRecti;
typedef struct {
	int x, y;
}MSPointi; 

/*
 *	功能:	获取 矩形src内 比例为retio的最大矩形，位置居中
 *	参数:	src: 输入的参考矩形
 *			ratio: 输入的矩形宽高比, ratio = 宽度/高度; 
 */
MSRecti GetRectByRatio(MSRecti src, float ratio); 

int addElement(void* pList, void* data, int size); 
/*
 *	功能:	从 *pList 指向的内存中返回一块大小为 size 的内存
				如果 *pList 为 NULL 则自动申请一块内存指向pList， 大小为2*size
				如果 *pList 的地址到达2的n次方， 则重新分配大小为size*pow(2, n+1)的内存给*pList
 *	参数:	pList: 目标区域
 *			size: 返回内存的大小
 *	返回值:	NULL, 内存申请失败, 否则返回对应内存地址
 *	注意: 	1. {0}作为结束符使用, *pList的合法元素不能包含{0}
 *			2. 由于*pList内存可能发生改变，故不能在多线程中使用
 */
void* AllocElement(void* pList, int size); 

typedef struct FisheyeTimer{
	void* data; 
	void(* timer_func)(void*, int); 
	void* arg; 
	int val; 
}FisheyeTimer; 
FisheyeTimer* FisheyeTimer_New(void(* timer_func)(void*, int), void* arg, int val); 
void FisheyeTimer_Delete(FisheyeTimer* timer); 
void FisheyeTimer_SetTimerEvent(FisheyeTimer* timer, void(* timer_func)(void*, int), void* arg, int val); 
void FisheyeTimer_Start(FisheyeTimer* timer, int val); 
void FisheyeTimer_Stop(FisheyeTimer* timer); 
typedef struct FisheyeTimerFunc{
	void* (* Timer_New)(void(* timer_func)(void*, int), void* arg, int val); 
	void (* Timer_Delete)(void* timer); 
	void (* Timer_SetTimerEvent)(void* timer, void(* timer_func)(void*, int), void* arg, int val); 
	void (* Timer_Start)(void* Timer, int val); 
	void (* Timer_Stop)(void* Timer); 
}FisheyeTimerFunc; 
void FisheyeTimer_SetDefaultFunc(FisheyeTimerFunc*); 

typedef struct InternalFunc {
	int(*Internal_Init)(void*, void*);
	void(*Internal_Release)(void*);
}InternalFunc; 
typedef struct Object {
	void** vtable; 
}Object; 
Object* Object_New(void* , ...);
void Object_Delete(Object*); 

#ifdef __cplusplus
}
#endif

#endif // !COMMON_H



