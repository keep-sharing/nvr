#ifndef FISHEYE_MODE_H
#define FISHEYE_MODE_H

#include"fisheye_units.h"

#ifdef __cplusplus
extern "C" {
#endif

enum StretchMode{
	StretchMode_Resize = 0,
	StretchMode_Original = 1,
	StretchMode_16_9 = 2,
	StretchMode_4_3 = 3
}; 

/*
*	参数说明: units: 该模式下的具体视图
*				cur_unit: 标定当前显示视图
*				aspect: 该显示模式的实际宽高比，默认为1.0f
*				mode_internal,标定安装模式
*
*/
typedef struct FisheyeMode {
	char mode_name[MAX_NAME_LENGTH]; 
	FisheyeUnit** units; 
	FisheyeUnit* cur_unit; 
	float aspect; 
	MSRecti render_target; 
	enum StretchMode* pstretch_mode; 
	MSRectf rela_rt; 
	const struct FisheyeMode_Internal* mode_internal; 
	float* aspect_r; 
	FisheyeUnitContext fu_ctx; 
	void* priv_data; 
}FisheyeMode; 

FisheyeMode* FisheyeMode_New(const char* name, struct FisheyeMode_Internal* mode_internal, void* arg); 
FisheyeMode* FisheyeMode_NewByCJSON(const cJSON* json, FisheyeUnitContext* fu_ctx, const struct FisheyeMode_Internal* mode_internal, void* arg); 
int FisheyeMode_Delete(FisheyeMode* fm); 

int FisheyeMode_OnDraw(FisheyeMode* fdm); 
int FisheyeMode_OnGrab(FisheyeMode* fm, void* data, int width, int height, int pixel_size); // 抓取矫正后的图像， data: 数据存放处， width, height, 数据宽高， pixel_size 像素大小，必须为4， 格式为rgba
int FisheyeMode_OnReshape(FisheyeMode* fm, MSRecti rt); 
#define FisheyeMode_SetStretchMode(fm, p_stretch_mode) (fm)->pstretch_mode = p_stretch_mode

int FisheyeMode_OnLButtonDown(FisheyeMode* fdm, unsigned int nFlags, MSPointf point); 
int FisheyeMode_OnLButtonUp(FisheyeMode* fdm, unsigned int nFlags, MSPointf point); 
int FisheyeMode_OnMouseMove(FisheyeMode* fdm, unsigned int nFlags, MSPointf point); 
int FisheyeMode_OnMouseWheel(FisheyeMode* fdm, unsigned int nFlags, short zDelta, MSPointf point); 
//const char* FisheyeMode_GetName(FisheyeMode* fm); 

int FisheyeMode_GetUnitParamLength(FisheyeMode* fdm); 
int	FisheyeMode_SetParam(FisheyeMode* fdm, const FisheyePlayerParam* params);
int FisheyeMode_GetParam(FisheyeMode* fdm, FisheyePlayerParam* params, int size, int* totalNum);

#define FisheyeMode_GetName(fm) ((const char*)(fm)->mode_name)

int	FisheyeMode_ExecCmd(FisheyeMode* fm, enum Fisheye_CMD cmd, float speed); 
int	FisheyeMode_SetPreset(FisheyeMode* fm, float pan, float tile, float zoom); 
enum Fisheye_CMD FisheyeMode_GetCurCmd(FisheyeMode* fm); 

typedef struct FisheyeMode_Internal {
	int(*init)(FisheyeMode*, void*); 
	int(*uninit)(FisheyeMode*);	
	int(*on_grab)(FisheyeMode*, void*, int, int, int); 
	const FisheyeUnit_Internal* unit_internal; 
}FisheyeMode_Internal; 

#ifdef __cplusplus
}
#endif

#endif // !FISHEYE_MODE_H
