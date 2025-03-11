#ifndef FISHEYE_PLAYER_H
#define FISHEYE_PLAYER_H

#ifdef __linux__
#include"fisheye_EGL.h"
#endif // __linux__


#include"common.h"
#include"fisheye_mode.h"
#ifdef __cplusplus
extern "C" {
#endif

/*
*	参数说明:	renderTarget : 指定显示窗口
*				width, height: 输入图片的宽高，（视频流的分辨率）
*				correct_region: O视图在原图上的具体区域
*				fisheye_region: 鱼眼图像在O视图上的区域
*				aspect_r: 鱼眼R视图的标准比例
*				aspect_o: 鱼眼O视图的标准比例
*				modes 显示模式数组 用于存储 1O/1P/2P/1P4R 等
*				stretch_mode 图像缩放模式 StretchMode_Resize: 填充模式, StretchMode_Original: 保持原图比例
*				cmd_speed: 设置调用FisheyePlayer_ExecCmd时的速度
*				priv_data 内部数据(reserved)
*				
*/
typedef struct FisheyePlayer {
	MSRecti renderTarget; 
	int width, height; 
	MSRectf correct_region; 
	MSRectf fisheye_region; 
	float aspect_r; 
	float aspect_o; 
	int enable_wireframe; 
	FisheyeMode** modes; 
	FisheyeMode* cur_mode; 
	int cur_cnt; 
	enum StretchMode stretch_mode; 
	float cmd_speed; 
	FisheyeParamInstallModel install_model; 
	void(* onWireFrameTouched)(void*, int, char*); 
	void* onWireFrameTouched_arg; 
	enum FisheyeOSDStatus zoom_status; 
	enum FisheyeOSDStatus preset_status; 
	enum FisheyeOSDStatus patrol_status; 
	enum FisheyeOSDStatus autoScan_status; 
	const struct FisheyePlayerContextType* context_type; 
	void* priv_data; 
	unsigned int seq; 
}FisheyePlayer; 
int FisheyePlayer_InitByCJSON(FisheyePlayer* fp, MSRecti* rT, MSRectf* fish_region, int w, int h, const cJSON* json, const struct FisheyePlayerContextType* context_type, void* arg);
#ifdef __linux__
FisheyePlayer*	FisheyePlayer_New(int win_width, int win_height, int pix_width, int pix_height);
#else
FisheyePlayer*	FisheyePlayer_New(MSRecti* rT, MSRectf* fish_region, int w, int h, const char* type, void* arg);
#endif // __linux__

FisheyePlayer*	FisheyePlayer_NewByJSON			(MSRecti* rT, MSRectf* fish_region, int w, int h, const char* json, const char* type, void* arg); 
void			FisheyePlayer_Delete			(FisheyePlayer* fp); 

MSPointf		FisheyePlayer_AbsCoorToRelaCoor	(FisheyePlayer* fp, MSPointi point); 

int				FisheyePlayer_ResetDataSize			(FisheyePlayer* fp, int width, int height); 
int				FisheyePlayer_ResetFisheyeRegion	(FisheyePlayer* fp, MSRectf* fish_region); 
int				FisheyePlayer_Reshape			(FisheyePlayer* fp, int x, int y, int w, int h); 
int				FisheyePlayer_EnableWireFrame(FisheyePlayer* fp, int flag); 

#define FisheyePlayer_InitData(fp, buf)							(fp)->context_type->InitData(fp, buf)
#define FisheyePlayer_OnDraw(fp)								FisheyeMode_OnDraw((fp)->cur_mode)
#define FisheyePlayer_OnSnapshot(fp)							FisheyeMode_OnSnapshot((fp)->cur_mode)  // TODO
#define FisheyePlayer_OnGrab(fp, buf, w, h, pixel_size)			FisheyeMode_OnGrab((fp)->cur_mode, buf, w, h, pixel_size)
#define FisheyePlayer_OnLButtonDown(fp, nFlags, point)			FisheyeMode_OnLButtonDown((fp)->cur_mode, nFlags, FisheyePlayer_AbsCoorToRelaCoor(fp, point))
#define FisheyePlayer_OnLButtonUp(fp, nFlags, point)			FisheyeMode_OnLButtonUp((fp)->cur_mode, nFlags, FisheyePlayer_AbsCoorToRelaCoor(fp, point))
#define FisheyePlayer_OnMouseMove(fp, nFlags, point)			FisheyeMode_OnMouseMove((fp)->cur_mode, nFlags, FisheyePlayer_AbsCoorToRelaCoor(fp, point))
#define FisheyePlayer_OnMouseWheel(fp, nFlags, zDelta, point)	FisheyeMode_OnMouseWheel((fp)->cur_mode, nFlags, zDelta, FisheyePlayer_AbsCoorToRelaCoor(fp, point))

#define FisheyePlayer_SetInstallModel(fp, InstallModel) FisheyeParamInstallModel_SetModel(&(fp)->install_model, InstallModel)
int	FisheyePlayer_SetDisplayMode(FisheyePlayer* fp, const char* mode); 
int	FisheyePlayer_SetStretchMode(FisheyePlayer* fp, enum StretchMode stretch_mode); 

#define FisheyePlayer_ExecCmd(fp, cmd) FisheyeMode_ExecCmd((fp)->cur_mode, cmd, (fp)->cmd_speed)
#define FisheyePlayer_SetCmdSpeed(fp, speed) (fp)->cmd_speed = speed
#define FisheyePlayer_GetCmdSpeed(fp) (fp)->cmd_speed
#define FisheyePlayer_GetCurCmd(fp)	FisheyeMode_GetCurCmd((fp)->cur_mode)
int	FisheyePlayer_SetParams(FisheyePlayer* fp, const FisheyePlayerParam params[], int size);  
int	FisheyePlayer_GetParams(FisheyePlayer* fp, FisheyePlayerParam params[], int size, int* totalNum);  

int	FisheyePlayer_SetParam(FisheyePlayer* fp, const char* key, const char* value);  // TODO
int	FisheyePlayer_SetParamByJSON(FisheyePlayer* fp, const char* json);  // TODO
int	FisheyePlayer_SetPreset(FisheyePlayer* fp, float pan, float tile, float zoom); // TODO
int	FisheyePlayer_SetRAspect(FisheyePlayer* fp, float aspect_f); 
FisheyeParamR FisheyePlayer_GetPreset(FisheyePlayer* fp); 

int FisheyePlayer_GetCurModeUnitsNum(FisheyePlayer* fp); // 获取当前模式的总视图数量 若无当前模式设置，则返回 0

int FisheyePlayer_SetCurModeActUnit(FisheyePlayer* fp, int num); //设置当前模式下的受激活的视图

int FisheyePlayer_WireFrameTouchedFunc(FisheyePlayer* fp, void(* func)(void*, int, char*), void* arg); //设置当线框被选中时的回调函数

// TODO int FisheyePlayer_Get(fp, buf); 
typedef struct FisheyePlayerContextType {
	int(*Init)(FisheyePlayer*, void*); 
	int(*UnInit)(FisheyePlayer*); 
	int(*InitData)(FisheyePlayer* fp, void* data); 
	void(*DataChg)(FisheyePlayer* fp); 
	const FisheyeMode_Internal* mode_internal; 
}FisheyePlayerContextType; 
#ifdef __linux__
// RGB TO YUV
typedef struct FisheyeTSdata {
	void *pVirAddr;
	int phyAddr;
	int size;
	int width;
	int height;
}FisheyeTSdata;

int FisheyePlayer_TransformBegin(FisheyePlayer* fp, int OutputWidth, int OutputHeight);
int FisheyePlayer_TransformEnd(FisheyePlayer* fp);
void FisheyePlayer_TransformGetBuf(FisheyePlayer* fp, void **ppVirAddr, int *pPhyAddr);
int FisheyePlayer_Transforming(FisheyePlayer* fp, FisheyeTSdata *pstSrc, FisheyeTSdata *pstDst);
#endif
/*********** 模仿 ffmpeg 的方式 *******************/
typedef struct FisheyeContextType {
	const char* type; 
	const FisheyePlayerContextType* player_internal;
}FisheyeContextType; 

#ifdef __cplusplus
}
#endif

#endif // !FISHEYE_PLAYER_H