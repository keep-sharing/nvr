#ifndef FISHEYE_UNITS_H
#define FISHEYE_UNITS_H

#include"common.h"
#include"fisheye_param.h"

#ifdef __cplusplus
extern "C"{
#endif

enum Fisheye_CMD{
	fcmd_stop, 
	fcmd_focusin, 
	fcmd_focusout, 
	fcmd_zoomin, 
	fcmd_zoomout, 
	fcmd_up, 
	fcmd_down, 
	fcmd_left, 
	fcmd_upleft, 
	fcmd_upright, 
	fcmd_downleft, 
	fcmd_downright, 
	fcmd_right, 
	fcmd_auto, 
	fcmd_irisin, 
	fcmd_irisout, 
	fcmd_autofocus, 
	fcmd_resetfocus, 
	fcmd_zoompos, 
	fcmd_focuspos, 
	fcmd_irispos, 
}; 
typedef struct MouseEvent {
	void(*OnLButtonDown)(void*, unsigned int, MSPointf); 
	void(*OnLButtonUp)(void*, unsigned int, MSPointf); 
	void(*OnMouseMove)(void*, unsigned int, MSPointf); 
	void(*OnMouseWheel)(void*, unsigned int, short, MSPointf); 
	void(*ExecCMD)(void*, enum Fisheye_CMD); 
	void(*SetPreset)(void*, float, float, float); 
}MouseEvent; 
enum FisheyeOSDStatus{
	f_osd_2s = 2,
	f_osd_5s = 5, 
	f_osd_10s = 10, 
	f_osd_open = 0, 
	f_osd_close = -1
}; 
struct FisheyeUnit; 
struct FisheyeUnitR; 

typedef struct FisheyePlayerParam {
	char modeName[MAX_NAME_LENGTH];
	char unitName[MAX_NAME_LENGTH];
	float params[3];
} FisheyePlayerParam;

typedef struct FisheyeUnitContext {
	FisheyeParamInstallModel* install_model; 
	float* aspect_r; // R视图的宽高比
	float* aspect_o; // O视图的原始宽高比
	int* p_enable_wireframe; 
	MSRectf correct_region;  // 鱼眼视图区域在O视图中的相对位置
	float aspect_mode; // 视图的实际宽高比
	void(* onWireFrameTouched)(void*, int, char*); 
	void* onWireFrameTouched_arg; 
	enum FisheyeOSDStatus* zoom_status; 
	enum FisheyeOSDStatus* preset_status; 
	enum FisheyeOSDStatus* patrol_status; 
	enum FisheyeOSDStatus* autoScan_status; 
}FisheyeUnitContext; 

typedef struct FisheyeUnit_ExtraPrimitive{
	void(* OnDraw)(struct FisheyeUnit_ExtraPrimitive*); 
}FisheyeUnit_ExtraPrimitive; 
typedef struct FisheyeOSDZoom{
	FisheyeUnit_ExtraPrimitive primitive; 
	enum FisheyeOSDStatus* zoom_status; 
	float* zoom_dst; 
	float zoom_local; 
	int isActive ; 
	MSRectf relaRect; 
	clock_t ck_start;  
	int (*DrawText)(void*, const char*, MSRectf*); 
	void* drawtext_arg; 
	void* priv_data; 
}FisheyeOSDZoom; 
FisheyeOSDZoom* FisheyeOSDZoom_New(struct FisheyeUnitR* unit); 
void FisheyeOSDZoom_Delete(FisheyeOSDZoom*); 


#define WireFrame_WIDTH 1
#define UnitBorder_WIDTH 1
enum WireFrame_Status
{
	WireFrame_normol = 1, 
	WireFrame_touched = 2,
	WireFrame_pressed = 3
}; 
/*
 * 视图的外边框
 */
typedef struct UnitBorder{
	FisheyeUnit_ExtraPrimitive primitive; 
	FisheyeParam param; 
	int* isActive; 
	MSRectf relaRect; 
	unsigned int color; 
	int width; 
	void* priv_data; 
	void(*uninit)(void*); 
}UnitBorder; 
UnitBorder* UnitBorder_New(struct FisheyeUnit* unit); 
void UnitBorder_Delete(UnitBorder*); 

/* 
 * WireFrame R视图对应的矫正区域线框
 * 继承FisheyeUnit_ExtraPrimitive 实现绘制特定线框
 * relaRect: 线框的显示区域
 * correct_region: 鱼眼视图区域在显示区域中的相对位置
 * r_param：R视图参数
 * aspect_r 平面矫正视图(R视图)的宽高比
 * status 标注线框状态
 */
typedef struct WireFrame{
	FisheyeUnit_ExtraPrimitive primitive; 
	FisheyeParam param;
	unsigned int installmodel_seq; 
	MSRectf relaRect; 
	FisheyeParamR* r_param; 
	MSRectf* correct_region; 
	float* aspect_r; 
	int* p_enable_wireframe; 
	enum WireFrame_Status status; 
	unsigned int color; 
	int width; 
	// 函数——进行线框更新
	void(*update)(struct WireFrame*); 
	struct FisheyeUnitR* target_r_unit;
	// 函数——进行坐标系转换：将输入点转换为 坐标为中心点的右手坐标系
	MSPointf (*transform_point)(struct WireFrame*, MSPointf); 
	void* priv_data; 
	void(*uninit)(void*); 
}WireFrame; 
WireFrame* WireFrame_New(struct FisheyeUnit* unit, struct FisheyeUnitR* unit_r); 
void WireFrame_Delete(WireFrame*); 
#define WireFrame_ChgPTZByXY(wireframe, point) FisheyeParamR_setParamByXY((wireframe)->r_param, (wireframe)->transform_point(wireframe, point) )
#define WireFrame_Contains(wireframe, point) FisheyeParamR_Contains((wireframe)->r_param, (wireframe)->transform_point(wireframe, point))
int WireFrame_SetStatus(WireFrame* wireframe, enum WireFrame_Status status); 


typedef struct {
	InternalFunc inter_func; 
	int (*OnDraw)(struct FisheyeUnit*); 
	int (*InitWireFrame)(struct FisheyeUnit*, WireFrame*); 
	int (*InitUnitBorder)(struct FisheyeUnit*, UnitBorder*); 
	int (*OnReshape)(struct FisheyeUnit*, MSRecti*); 
	int (*DrawText)(struct FisheyeUnit*, const char*, MSRectf*); 
	int(*OnGrab)(struct FisheyeUnit*, void*, int, int, int); 
}FisheyeUnit_Function; 
/*
 *  公共的视图参数
 *	参数说明:	relaRect 在渲染区域中的相对显示位置；如{0.0,0.5,0.5,0.5}表明显示在左下角
 *				isActive 标注该视图是否被选中 0 未被选中， 1 被选中
 *				mouseEvent 鼠标事件
 *				extra_draws 额外绘制，用于绘制线框、字符等。
 *				f_cmd 命令参数
 *				cmd_speed 命令参数的执行速度
 *				aspect_r 平面矫正视图(R视图)的宽高比
 */
typedef struct FisheyeUnit {
	const MouseEvent* mouseEvent; 
	char name[MAX_NAME_LENGTH]; 
	enum Fisheye_CMD f_cmd; 
	float cmd_speed; 
	float* aspect_r; 
	char type[8]; 
	MSRectf relaRect; 
	int isActive; 
	unsigned int color; 
	WireFrame** R_Frames; 
	WireFrame* curFrame; 
	UnitBorder* unit_border; 
	FisheyeUnitContext* fu_ctx; 
	FisheyeUnit_ExtraPrimitive** extra_draws; 
	const FisheyeUnit_Function* inter_func;
	int(*SetParam)(struct FisheyeUnit*, const FisheyePlayerParam*);
	int(*GetParam)(struct FisheyeUnit*, FisheyePlayerParam*);
	void* priv_data; 
}FisheyeUnit; 
extern const MouseEvent fisheye_unit_mouse_event; 
INLINE int FisheyeUnit_Init(FisheyeUnit* unit, float x, float y, float w, float h, const MouseEvent* mouse_event); 
void FisheyeUnit_Delete(FisheyeUnit*); 
void FisheyeUnit_OnDraw(FisheyeUnit*); 
#define FisheyeUnit_Contains(unit, x, y)  MSRectf_Contains(&(unit)->relaRect, (x), (y))
#define FisheyeUnit_GetName(unit)  (unit)->name
int  FisheyeUnit_AddWireFrame(FisheyeUnit* unit, struct FisheyeUnitR* unit_r); 
#define FisheyeUnit_Reshape(unit, render_target) (unit)->inter_func->OnReshape(unit, render_target)
#define FisheyeUnit_SetCmd(unit, cmd, speed) {(unit)->f_cmd = cmd; (unit)->cmd_speed = speed; } 
#define FisheyeUnit_GetCmd(unit) (unit)->f_cmd
#define	FisheyeUnit_SetPreset(unit, pan, tile, zoom) (unit)->mouseEvent->SetPreset(unit, pan, tile, zoom)
#define FisheyeUnit_OnGrab(unit, buf, width, height, pixel_size) (unit)->inter_func->OnGrab(unit, buf, width, height, pixel_size)

typedef struct {
	FisheyeUnit unit; 
	FisheyeParamO orig; 
}FisheyeUnitO; 
extern const MouseEvent fisheye_unit_o_mouse_event; 
FisheyeUnitO* FisheyeUnitO_NewByCJSON(const cJSON* json, FisheyeUnitContext* fu_ctx, const FisheyeUnit_Function* unit_o_func, void* arg); 
INLINE int FisheyeUnitO_Init(FisheyeUnitO* unit, float x, float y, float w, float h, FisheyeParamO* param);
void FisheyeUnitO_Delete(FisheyeUnitO* fu); 

typedef struct FisheyeUnitP {
	FisheyeUnit unit; 
	FisheyeParamP pano; 
	MSPointf startP; 
}FisheyeUnitP; 
extern const MouseEvent fisheye_unit_p_mouse_event; 
FisheyeUnitP* FisheyeUnitP_NewByCJSON(const cJSON* json, FisheyeUnitContext* fu_ctx, const FisheyeUnit_Function* unit_p_func, void* arg); 
INLINE int FisheyeUnitP_Init(FisheyeUnitP* unit, float x, float y, float w, float h, FisheyeParamP* param);
int FisheyeUnitP_Delete(FisheyeUnitP* fu); 

typedef struct FisheyeUnit2P {
	FisheyeUnit unit; 
	FisheyeParamP pano; 
	MSPointf startP; 
}FisheyeUnit2P; 
extern const MouseEvent fisheye_unit_2p_mouse_event; 
FisheyeUnit2P* FisheyeUnit2P_NewByCJSON(const cJSON* json, FisheyeUnitContext* fu_ctx, const FisheyeUnit_Function* unit_2p_func, void* arg); 
INLINE int FisheyeUnit2P_Init(FisheyeUnit2P* unit, float x, float y, float w, float h, FisheyeParamP* param);
int FisheyeUnit2P_Delete(FisheyeUnit2P* fu); 


typedef struct FisheyeUnitW {
	FisheyeUnit unit; 
	FisheyeParamW wparam; 
}FisheyeUnitW; 
extern const MouseEvent fisheye_unit_w_mouse_event; 
FisheyeUnitW* FisheyeUnitW_NewByCJSON(const cJSON* json, FisheyeUnitContext* fu_ctx, const FisheyeUnit_Function* unit_w_func, void* arg);
INLINE int FisheyeUnitW_Init(FisheyeUnitW* unit, float x, float y, float w, float h, FisheyeParamW* param);
int FisheyeUnitW_Delete(FisheyeUnitW *fu); 


typedef struct FisheyeUnitR {
	FisheyeUnit unit; 
	FisheyeParamR ptz; 
	MSPointf start; 
	MSPointf pre; 
//	FisheyeOSDZoom* f_osd_zoom; // 注意： 未释放
}FisheyeUnitR; 
extern const MouseEvent fisheye_unit_r_mouse_event; 
FisheyeUnitR* FisheyeUnitR_NewByCJSON(const cJSON* json, FisheyeUnitContext* fu_ctx, const FisheyeUnit_Function* unit_r_func, void* arg);
INLINE int FisheyeUnitR_Init(FisheyeUnitR* unit, float x, float y, float w, float h, FisheyeParamR* param);
int FisheyeUnitR_Delete(FisheyeUnitR *fu); 

typedef struct FisheyeUnit_Internal {
	const FisheyeUnit_Function* unit_o_func; 
	const FisheyeUnit_Function* unit_p_func; 
	const FisheyeUnit_Function* unit_2p_func; 
	const FisheyeUnit_Function* unit_w_func; 
	const FisheyeUnit_Function* unit_r_func; 
}FisheyeUnit_Internal; 

FisheyeUnit* FisheyeUnit_NewByCJSON(const cJSON* json, FisheyeUnitContext* fu_ctx, const FisheyeUnit_Internal* unit_internal, void* arg); 
int	FisheyeUnit_SetParam(FisheyeUnit* unit, const FisheyePlayerParam* param);
int	FisheyeUnit_GetParam(FisheyeUnit* unit, FisheyePlayerParam* params);

#ifdef __cplusplus
}
#endif

#endif // !FISHEYE_UNITS_H
