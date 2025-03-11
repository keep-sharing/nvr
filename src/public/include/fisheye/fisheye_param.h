#ifndef FISHEYE_PARAM_H
#define FISHEYE_PARAM_H

#include"common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PTZ_ALPHA_MIN	0.0f
#define PTZ_ALPHA_MAX	MS_PI/2
#define PTZ_THETA_MIN	MS_PI/12
#define PTZ_THETA_MAX	2*MS_PI/3
typedef struct FisheyeParam {
	unsigned int seq; 
}FisheyeParam; 
#define FisheyeParam_Init(para)  (para)->seq = 0
#define FisheyeParam_Update(para)  (para)->seq++
enum Fisheye_InstallModel{
	Ceilling, 
	flat,
	wall
}; 
typedef struct FisheyeParamInstallModel {
	FisheyeParam param; 
	enum Fisheye_InstallModel install_model; 
}FisheyeParamInstallModel; 
void FisheyeParamInstallModel_SetModel(FisheyeParamInstallModel* fisheye_install_model, enum Fisheye_InstallModel install_model); 
enum Fisheye_InstallModel FisheyeParamInstallModel_GetModel(FisheyeParamInstallModel* fisheye_install_model); 


/*
*	O视图参数：暂为空
*/
typedef struct FisheyeParamO {
	FisheyeParam param; 
}FisheyeParamO; 
void FisheyeParamO_Init(FisheyeParamO* p); 
int FisheyeParamO_InitByCJSON(FisheyeParamO* p, const cJSON* json); 

/*
*  P视图参数
*	参数说明:	offset 对应于 全景视图的偏移量
*				range 显示的半径 在2P模式下应为π(180°)，其他默认为 2π(360°)
*/
typedef struct{
	FisheyeParam param; 
	FisheyeParamInstallModel* install_model; 
	float offset; 
	float range; 
}FisheyeParamP; 
void FisheyeParamP_Init(FisheyeParamP* p); 
int FisheyeParamP_InitByCJSON(FisheyeParamP* p, const cJSON* json); 
INLINE void FisheyeParamP_SetOffset(FisheyeParamP* param, float offset);
void FisheyeParamP_IncOffset(FisheyeParamP* param, float bias); 
float FisheyeParamP_GetRange(const FisheyeParamP* param); 


/*
*	W视图 经纬度法校正图 适用于壁挂模式 参数 暂为空
*/
typedef struct {
	FisheyeParam param; 
}FisheyeParamW; 
int FisheyeParamW_InitByCJSON(FisheyeParamW* p, const cJSON* json); 
void FisheyeParamW_Init(FisheyeParamW* p); 


/*
*	R视图参数
*	参数说明:	Alpha,Beta,Theta 对应PTZ操作的 Tilt、Pan、和Zoom
*/
typedef struct {
	FisheyeParam param; 
	FisheyeParamInstallModel* install_model; 
	float alpha; 
	float beta; 
	float theta; 
	float up; 
}FisheyeParamR; 
int FisheyeParamR_InitByCJSON(FisheyeParamR* r, const cJSON* json); 
void FisheyeParamR_Init(FisheyeParamR* r); 
//#define FisheyeParamR_SetAlpha(param_r, _alpha) {(param_r)->alpha = _alpha; FisheyeParam_Update(&(param_r)->param); }
//#define FisheyeParamR_SetBeta(param_r, _beta) {(param_r)->beta = _beta; FisheyeParam_Update(&(param_r)->param); }
//#define FisheyeParamR_SetTheta(param_r, _theta) {(param_r)->theta = _theta; FisheyeParam_Update(&(param_r)->param); }
void FisheyeParamR_SetAlpha(FisheyeParamR*, float alpha); 
void FisheyeParamR_SetBeta(FisheyeParamR*, float beta); 
void FisheyeParamR_SetTheta(FisheyeParamR*, float theta); 
void FisheyeParamR_IncAlpha(FisheyeParamR* p, float bias); 
void FisheyeParamR_IncBeta(FisheyeParamR* p, float bias); 
void FisheyeParamR_IncTheta(FisheyeParamR* p, float bias); 
int FisheyeParamR_Contains(FisheyeParamR* p, MSPointf point); 
int FisheyeParamR_setParamByXY(FisheyeParamR* p, MSPointf point); //将归一化圆（圆点为坐标轴中心，圆半径为1.0）的点point转化为ptz值
int FisheyeParamR_GotoPtz(FisheyeParamR* p, FisheyeParamR* src, float speed); 
MSPointf PTZ_AB2XY(float alpha, float beta); 
MSPointf PTZ_XY2AB(float x, float y); 

#ifdef __cplusplus
}
#endif

#endif // !FISHEYE_PARAM_H
