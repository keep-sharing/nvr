#include"fisheye_mode.h"

enum StretchMode g_stretch_mode = StretchMode_Original; 
int FisheyeMode_UnInit(FisheyeMode* fdm) 
{
	FisheyeUnit** it = fdm->units; 
	while (*it)
	{
		FisheyeUnit_Delete(*it); 
		it++; 
	}
	if (fdm->units) { free(fdm->units); fdm->units = NULL; }
	fdm->cur_unit = NULL; 
	fdm->mode_internal->uninit(fdm); 
	return 0; 
}

//*********************************************************************************************************************************
static int FisheyeMode_AddUnit(FisheyeMode* fm, FisheyeUnit* fu)
{
	FisheyeUnit** ret = (FisheyeUnit**)AllocElement(&fm->units, sizeof(FisheyeUnit*)); 
	if (NULL == ret)
		return -1; 
	*ret = fu; 
	return 0; 
}
static int FisheyeMode_AddUnitByCJSON(FisheyeMode* fm, cJSON* json, FisheyeUnitContext* fu_ctx)
{
	FisheyeUnit* fu = FisheyeUnit_NewByCJSON(json, fu_ctx, fm->mode_internal->unit_internal, fm->priv_data);
	if (NULL == fu)
		return -1; 
	if (FisheyeMode_AddUnit(fm, fu))
	{
		FisheyeUnit_Delete(fu); 
		return -1; 
	}
	return 0;
}
static FisheyeUnit* FisheyeMode_GetUnit(FisheyeMode* fm, const char* name)
{
	FisheyeUnit** it = fm->units; 
	const char* tmp_name = NULL; 
	while (*it)
	{
		tmp_name = FisheyeUnit_GetName((*it));
		if (tmp_name && !strncmp(tmp_name, name, sizeof(fm->mode_name)))
			break; 
		it++; 
	}
	return *it; 
}
static int FisheyeMode_Init(FisheyeMode* fm, const char* mode_name)
{
	// 数据初始化
	memset(fm, 0, sizeof(FisheyeMode)); 
	fm->aspect = 1.0f; 
	fm->pstretch_mode = &g_stretch_mode; 
	if (mode_name)
		strncpy(fm->mode_name, mode_name, sizeof(fm->mode_name) - 1); 
	else
		return -1; 
	printf(" ==================== mode_name: %s\n", fm->mode_name); 
	return 0; 
}
static int FisheyeMode_InitByCJSON(FisheyeMode* fm, const cJSON* json, FisheyeUnitContext* fu_ctx, const struct FisheyeMode_Internal* mode_internal, void* arg)
{
	// TODO 后续的 return -1 应该对应更改， 改为goto error
	cJSON* tmp = cJSON_GetObjectItem(json, "name"); 
	if (NULL == tmp || cJSON_String != tmp->type)
		return -1; 
	if (FisheyeMode_Init(fm, tmp->valuestring))
		return -1; 
	
	fm->mode_internal = mode_internal;
	if (mode_internal->init(fm, arg))
		return -1;

	tmp = cJSON_GetObjectItem(json, "aspect"); 
	if(tmp && cJSON_Number == tmp->type)
		fm->aspect = (float)tmp->valuedouble; 
	fm->fu_ctx = *fu_ctx; 
	fm->fu_ctx.aspect_mode = fm->aspect; 
	tmp = cJSON_GetObjectItem(json, "units"); 
	if (NULL == tmp || cJSON_Array != tmp->type)
		return -1; 
	int size = cJSON_GetArraySize(tmp); 
	cJSON* item = NULL; 
	int i;
	for (i = 0; i < size; i++)
	{
		item = cJSON_GetArrayItem(tmp, i); 
		if (item && cJSON_Object == item->type)
			FisheyeMode_AddUnitByCJSON(fm, item, &fm->fu_ctx); 
	}
	fm->aspect_r = fu_ctx->aspect_r; 
	for (i = 0; i < size; i++)
	{
		item = cJSON_GetArrayItem(tmp, i); 
		cJSON* wire_frame = cJSON_GetObjectItem(item, "wireFrame"); 
		if(NULL==wire_frame || cJSON_Array != wire_frame->type)
			continue; 
		
		cJSON* tmp_name = cJSON_GetObjectItem(item, "name"); 
		if (NULL == tmp_name || cJSON_String != tmp_name->type)
		{
			DEBUG_WARNING("set wireFrame failed, request an unit name"); 
			continue; 
		}
		FisheyeUnit* unit_dst =	FisheyeMode_GetUnit(fm, tmp_name->valuestring); 
		if (NULL == unit_dst)
		{
			DEBUG_WARNING("set wireFrame failed, unknown unit name"); 
			continue; 
		}
		int item_count = cJSON_GetArraySize(wire_frame); 
		while (item_count--)
		{
			cJSON* tmp_name = cJSON_GetArrayItem(wire_frame, item_count); 
			if (NULL == tmp_name || cJSON_String != tmp_name->type)
				continue; 
			FisheyeUnit* unit_r = FisheyeMode_GetUnit(fm, tmp_name->valuestring); 
			if (NULL == unit_r || !strcmp(unit_r->type, "UnitR"))
			{
				DEBUG_WARNING("set wireFrame failed, unknown unit name"); 
				continue; 
			}
			FisheyeUnit_AddWireFrame(unit_dst, (FisheyeUnitR*)unit_r); 
		}
	}
	return 0; 
}
FisheyeMode* FisheyeMode_New(const char* name, struct FisheyeMode_Internal* mode_internal, void* arg)
{
	// 分配内存
	FisheyeMode* fm = (FisheyeMode*)malloc(sizeof(FisheyeMode)); 
	if (NULL == fm)
		return NULL; 
	// TODO 数据初始化
	if (FisheyeMode_Init(fm, name))
	{
		free(fm); 
		return NULL; 
	}
	return fm; 
}
FisheyeMode* FisheyeMode_NewByCJSON(const cJSON* json, FisheyeUnitContext* fu_ctx, const struct FisheyeMode_Internal* mode_internal, void* arg)
{
	FisheyeMode* fm = (FisheyeMode*)malloc(sizeof(FisheyeMode));
	if (NULL == fm)
		return NULL; 
	// 根据json 初始化数据
	if (FisheyeMode_InitByCJSON(fm, json, fu_ctx, mode_internal, arg))
		goto error; 

	return fm; 

error:
	free(fm);
	return NULL;
}
int FisheyeMode_Delete(FisheyeMode* fm)
{
	// 释放内部数据
	FisheyeMode_UnInit(fm); 
	// 释放内存
	free(fm); 
	return 0; 
}
//const char* FisheyeMode_GetName(FisheyeMode* fm)
//{
//	return fm->mode_name; 
//}
int FisheyeMode_OnDraw(FisheyeMode* fdm)
{
	FisheyeUnit** it = fdm->units; 
	while (*it)
	{
		FisheyeUnit_OnDraw(*it); 
		it++; 
	}
	return 0; 
}
int FisheyeMode_OnGrab(FisheyeMode* fm, void* data, int width, int height, int pixel_size)
{
	// TODO 截图
//	if (NULL != fm->cur_unit)
//		return FisheyeUnit_OnGrab(fm->cur_unit, data, width, height, pixel_size);
//	else
		return fm->mode_internal->on_grab(fm, data, width, height, pixel_size); 
}
int FisheyeMode_OnReshape(FisheyeMode* fm, MSRecti rt)
{
	DEBUG_PRINTF(" ########################## mode_name:%s\n", fm->mode_name);
	float aspect = 1.0f; 
	switch (*fm->pstretch_mode)
	{
	case StretchMode_Resize: 
		aspect = fm->aspect; 
		break; 
	case StretchMode_Original: 
		aspect = fm->aspect; 
		break; 
	case StretchMode_16_9: 
		aspect = 16.0f/9.0f; 
		break; 
	case StretchMode_4_3: 
		aspect = 4.0f/3.0f; 
		break; 
	default:
		aspect = 1.0f; 
	}
	float judge = aspect*rt.h - rt.w; 
	if(StretchMode_Resize == *fm->pstretch_mode)
	{
		fm->render_target = rt; 
		fm->rela_rt = (MSRectf){0.0f, 0.0f, 1.0f, 1.0f}; 
	}
	else if(judge <= 0)
	{
		fm->render_target.h = rt.h; 
		fm->render_target.w = (int)(aspect * rt.h); 
		fm->render_target.x = rt.x + (rt.w - fm->render_target.w)/2; 
		fm->render_target.y = rt.y; 
		fm->rela_rt.h = 1.0f; 
		fm->rela_rt.w = aspect*rt.h/rt.w; 
		fm->rela_rt.x = 0.5f - fm->rela_rt.w/2; 
		fm->rela_rt.y = 0.0f; 
	}
	else
	{
		fm->render_target.w = rt.w; 
		fm->render_target.h = (int)(rt.w/aspect); 
		fm->render_target.y = rt.y + (rt.h - fm->render_target.h)/2; 
		fm->render_target.x = rt.x; 
		fm->rela_rt.w = 1.0f; 
		fm->rela_rt.h = rt.w/(aspect*rt.h); 
		fm->rela_rt.y = 0.5f - fm->rela_rt.h/2; 
		fm->rela_rt.x = 0.0f; 
	}
	FisheyeUnit** it = fm->units; 
	while(it && *it)
	{
		FisheyeUnit_Reshape(*it, &fm->render_target); 
		it++; 
	}
	return 0; 
}
int FisheyeMode_OnLButtonDown(FisheyeMode* fdm, unsigned int nFlags, MSPointf point)
{
	MSPointf point_t = (MSPointf){(point.x-fdm->rela_rt.x)/fdm->rela_rt.w, (point.y-fdm->rela_rt.y)/fdm->rela_rt.h}; 
	if (NULL == fdm->units)
		return -1; 
	int i = 0; 
	fdm->cur_unit = NULL; 
	for (; fdm->units[i]; i++)
	{
		FisheyeUnit**it = fdm->units; 
		while(it && *it){
            (*it)->isActive = 0;
            if((*it)->curFrame)
                WireFrame_SetStatus((*it)->curFrame, WireFrame_normol);
			it++; 
		}
		if (FisheyeUnit_Contains(fdm->units[i], point_t.x, point_t.y))
		{
			fdm->cur_unit = fdm->units[i]; 
			fdm->cur_unit->isActive = 1; 
			fdm->cur_unit->mouseEvent->OnLButtonDown(fdm->cur_unit, nFlags, point_t); 
			break; 
		}
	}
	return 0; 
}
int FisheyeMode_OnLButtonUp(FisheyeMode* fdm, unsigned int nFlags, MSPointf point)
{
	MSPointf point_t = (MSPointf){(point.x-fdm->rela_rt.x)/fdm->rela_rt.w, (point.y-fdm->rela_rt.y)/fdm->rela_rt.h}; 
	if (fdm->cur_unit) { fdm->cur_unit->mouseEvent->OnLButtonUp(fdm->cur_unit, nFlags, point_t); }
	return 0; 
}
int FisheyeMode_OnMouseMove(FisheyeMode* fdm, unsigned int nFlags, MSPointf point)
{
	MSPointf point_t = (MSPointf){(point.x-fdm->rela_rt.x)/fdm->rela_rt.w, (point.y-fdm->rela_rt.y)/fdm->rela_rt.h}; 
	if (fdm->cur_unit) { fdm->cur_unit->mouseEvent->OnMouseMove(fdm->cur_unit, nFlags, point_t); }
	return 0;
}
int FisheyeMode_OnMouseWheel(FisheyeMode* fdm, unsigned int nFlags, short zDelta, MSPointf point)
{
	MSPointf point_t = (MSPointf){(point.x-fdm->rela_rt.x)/fdm->rela_rt.w, (point.y-fdm->rela_rt.y)/fdm->rela_rt.h}; 
	if (fdm->cur_unit) { fdm->cur_unit->mouseEvent->OnMouseWheel(fdm->cur_unit, nFlags, zDelta, point_t); }
	return 0;
}

int	FisheyeMode_ExecCmd(FisheyeMode* fm, enum Fisheye_CMD cmd, float speed)
{
	if(NULL == fm->cur_unit)
		return 0; 
	FisheyeUnit_SetCmd(fm->cur_unit, cmd, speed); 
	return 0; 
}
int	FisheyeMode_SetPreset(FisheyeMode* fm, float pan, float tile, float zoom)
{
	if(fm->cur_unit)
		FisheyeUnit_SetPreset(fm->cur_unit, pan, tile, zoom); 
	return 0; 
}


enum Fisheye_CMD FisheyeMode_GetCurCmd(FisheyeMode* fm)
{
	if (NULL == fm->cur_unit)
		return fcmd_stop; 
	return FisheyeUnit_GetCmd(fm->cur_unit); 

}

int FisheyeMode_GetUnitParamLength(FisheyeMode* fdm)
{
	int ret = 0; 
	FisheyeUnit**it = fdm->units;
	while (it && *it) {
		ret++; 
		it++; 
	}
	return ret;
}
int	FisheyeMode_SetParam(FisheyeMode* fdm, const FisheyePlayerParam* params)
{
	FisheyeUnit* unit = FisheyeMode_GetUnit(fdm, params->unitName);
	if (NULL == unit)
	{
		DEBUG_PRINTF("WARNNING: no unit named[%s], on model[%s]", params->modeName, fdm->mode_name);
		return -1;
	}
	FisheyeUnit_SetParam(unit, params);
	return 0; 
}
int FisheyeMode_GetParam(FisheyeMode* fdm, FisheyePlayerParam* params, int size, int* totalNum)
{
	int paramOffset = 0; 
	FisheyeUnit** it = fdm->units;
	while (it && *it) {
		memcpy(params + paramOffset, fdm->mode_name, MAX_NAME_LENGTH);
		FisheyeUnit_GetParam(*it, params + paramOffset); 
		paramOffset++; 
		it++;
	}
	*totalNum = paramOffset;
	return 0; 
}
