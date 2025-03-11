#include"fisheye_units.h"

#include <math.h> 
#include <stdio.h> 
float g_aspect_r = 1.0f; 
WireFrame* WireFrame_New(FisheyeUnit* unit, struct FisheyeUnitR* unit_r)
{
	WireFrame* wire_frame = (WireFrame*)malloc(sizeof(WireFrame)); 
	if(NULL == wire_frame)
		return NULL; 
	
	memset(wire_frame, 0, sizeof(WireFrame)); 
	
	FisheyeParam_Init(&wire_frame->param); 
	wire_frame->target_r_unit = unit_r;
	wire_frame->relaRect = unit->relaRect; 
	wire_frame->r_param = &unit_r->ptz; 
	wire_frame->param.seq = wire_frame->r_param->param.seq-1; 
	wire_frame->status = WireFrame_normol; 
	wire_frame->color = unit_r->unit.color; 
	wire_frame->aspect_r = unit_r->unit.aspect_r; 
	wire_frame->p_enable_wireframe = unit->fu_ctx->p_enable_wireframe; 
	wire_frame->correct_region = &unit->fu_ctx->correct_region; 
	wire_frame->width =    (int)WireFrame_WIDTH; 
	wire_frame->priv_data = NULL; 
	wire_frame->uninit = NULL; 
	return wire_frame; 
}
void WireFrame_Delete(WireFrame* wf)
{
	// release priv_data
	if (wf->uninit)
		wf->uninit(wf->priv_data); 
	wf->priv_data = NULL; 
	free(wf); 
	return; 
}

int WireFrame_SetStatus(WireFrame* wireframe, enum WireFrame_Status status)
{
	wireframe->status = status; 
	wireframe->target_r_unit->unit.isActive = (status == WireFrame_normol) ? 0 : 1;
	wireframe->width = wireframe->status * WireFrame_WIDTH; 
	return 0;
}

static void SetWireFramesStatus(WireFrame** frames, enum WireFrame_Status status)
{
	if (NULL == frames)
		return; 
	int i;
	for (i = 0; frames[i]; i++)
	{
		WireFrame_SetStatus(frames[i], status);
	}
}
static WireFrame* GetActiveWireFrame(WireFrame** frames, MSPointf point)
{
	if (NULL == frames)
		return NULL; 
	int i;
	for (i = 0; frames[i]; i++)
	{
		if (WireFrame_Contains(frames[i], point))
			return frames[i]; 
	}
	return NULL; 
}

static void FisheyeOSDZoom_OnDraw(FisheyeOSDZoom* fz)
{
	if(*fz->zoom_status == f_osd_close)
		return; 
	clock_t interval = clock() - fz->ck_start; 
	if( *fz->zoom_status == f_osd_open || interval < *fz->zoom_status * CLOCKS_PER_SEC )
	{
		char zoom_msg[32] = {0}; 
		float zoom_value = 9.0f - 7.0f/105.0f*RadianToAngle(*fz->zoom_dst); 
		sprintf(zoom_msg, "Zoom x%.1f \n", zoom_value); 
		fz->DrawText(fz->drawtext_arg, zoom_msg, &fz->relaRect); 
		DEBUG_PRINTF("OnZoomMsg -- interval： %ld ==== %s", interval/1000, zoom_msg); 
	}
	
	if(fz->zoom_local == *fz->zoom_dst)
		return; 
	else
	{
		fz->ck_start = clock(); 
		fz->zoom_local = *fz->zoom_dst; 
	}
	return; 
}
FisheyeOSDZoom* FisheyeOSDZoom_New(struct FisheyeUnitR* unit)
{
	FisheyeOSDZoom* ret = (FisheyeOSDZoom*)malloc(sizeof(FisheyeOSDZoom)); 
	if(NULL == ret)
		return NULL; 
	ret->primitive.OnDraw = (void(*)(FisheyeUnit_ExtraPrimitive*))&FisheyeOSDZoom_OnDraw; 
	ret->zoom_local = unit->ptz.theta; 
	ret->zoom_dst = &unit->ptz.theta; 
	ret->relaRect = unit->unit.relaRect; 
	ret->zoom_status = unit->unit.fu_ctx->zoom_status; 
	ret->drawtext_arg = unit; 
//	ret->DrawText = (int(*)(void*, const char*, MSRectf*))unit->unit.inter_func->DrawText; 
	ret->priv_data = NULL; // TODO
	ret->ck_start = clock(); 
	return ret; 
}
void FisheyeOSDZoom_Delete(FisheyeOSDZoom* fz)
{
	// TOOD 内部数据的删除，
    if (fz)
    	free(fz); 
}

UnitBorder* UnitBorder_New(struct FisheyeUnit* unit)
{
	UnitBorder* ret = (UnitBorder*)malloc(sizeof(UnitBorder)); 
	if(NULL == ret)
		return NULL; 
	memset(ret, 0, sizeof(UnitBorder)); 
	FisheyeParam_Init(&ret->param); 
	ret->isActive = &unit->isActive; 
	ret->color = unit->color; 
	ret->width = (int)UnitBorder_WIDTH;
	ret->relaRect = MSRectf_DefaultValue; 
	return ret; 
}
void UnitBorder_Delete(UnitBorder* ub)
{
	if(ub->uninit)
		ub->uninit(ub->priv_data); // release privdata
	ub->priv_data = NULL; 
	free(ub); 
}


#define  FisheyeUnit_DefaultValue (FisheyeUnit){\
.relaRect = MSRectf_DefaultValue, \
.f_cmd = fcmd_stop, \
.cmd_speed = 1.0f, \
.aspect_r = &g_aspect_r, \
.isActive = 0, \
.color = 0x00c0ffff, \
.R_Frames = NULL, \
.curFrame = NULL, \
.unit_border = NULL, \
.mouseEvent = &fisheye_unit_mouse_event,\
}
FisheyeUnit* FisheyeUnit_NewByCJSON(const cJSON* json, FisheyeUnitContext* fu_ctx, const FisheyeUnit_Internal* unit_internal, void* arg)
{
	cJSON* tmp = cJSON_GetObjectItem(json, "type");
	if (NULL == tmp || cJSON_String != tmp->type)
		return NULL; 
	if (!strcmp(tmp->valuestring, "UnitO"))
		return (FisheyeUnit*)FisheyeUnitO_NewByCJSON(json, fu_ctx, unit_internal->unit_o_func, arg); 
	else if (!strcmp(tmp->valuestring, "UnitP"))
		return (FisheyeUnit*)FisheyeUnitP_NewByCJSON(json, fu_ctx, unit_internal->unit_p_func, arg); 
	else if (!strcmp(tmp->valuestring, "Unit2P"))
		return (FisheyeUnit*)FisheyeUnit2P_NewByCJSON(json, fu_ctx, unit_internal->unit_2p_func, arg); 
	else if (!strcmp(tmp->valuestring, "UnitW"))
		return (FisheyeUnit*)FisheyeUnitW_NewByCJSON(json, fu_ctx, unit_internal->unit_w_func, arg); 
	else if (!strcmp(tmp->valuestring, "UnitR"))
		return (FisheyeUnit*)FisheyeUnitR_NewByCJSON(json, fu_ctx, unit_internal->unit_r_func, arg); 
	else
		return NULL; 
}
void FisheyeUnit_OnDraw(FisheyeUnit* unit)
{
	unit->mouseEvent->ExecCMD(unit, unit->f_cmd); //业务逻辑
	unit->inter_func->OnDraw((unit)); 			  // 功能实现
	FisheyeUnit_ExtraPrimitive** it = unit->extra_draws; //业务逻辑
	while(it && *it){
		(*it)->OnDraw(*it); // 功能实现
		it++; 
	}
}
int  FisheyeUnit_AddWireFrame(FisheyeUnit* unit, struct FisheyeUnitR* unit_r)
{	
	WireFrame* wire_frame = WireFrame_New(unit, unit_r); 
	if(NULL == wire_frame)
		return -1; 
	
	if(unit->inter_func->InitWireFrame(unit, wire_frame))
		goto ERROR; 
	
	WireFrame** wire_target = (WireFrame**)AllocElement(&unit->extra_draws, sizeof(WireFrame*)); 
	if(NULL == wire_target)
		goto ERROR; 
	
	WireFrame** target = (WireFrame**)AllocElement(&unit->R_Frames, sizeof(WireFrame**)); 
	if(NULL == target)
		goto ERROR; 
	
	*target = wire_frame; 
	*wire_target = wire_frame; 

	unit_r->unit.curFrame = wire_frame; 

	return 0; 
	
ERROR: 
	DEBUG_PRINTF("####LINE: %d == FUNC: %s ==  ERROR: init private data error unit name = %s \n", __LINE__, __func__, unit->name); 
	WireFrame_Delete(wire_frame); 
	return -1; 
}

INLINE int FisheyeUnit_Init(FisheyeUnit* unit, float x, float y, float w, float h, const MouseEvent* mouse_event)
{
	MSRectf rectf = { x, y, w, h };
	unit->relaRect = rectf;
	unit->isActive = 0;
	unit->mouseEvent = mouse_event ? mouse_event : &fisheye_unit_mouse_event;
	return 0;
}

void FisheyeUnit_Delete(FisheyeUnit* unit)
{
	WireFrame** it = unit->R_Frames; 
	while(it && *it){
		WireFrame_Delete(*it); 
		it++; 
	}
	if(unit->R_Frames)
		free(unit->R_Frames); 
	unit->R_Frames = NULL; 
	if (unit->extra_draws)
		free(unit->extra_draws); 
	unit->extra_draws = NULL; 
	if(unit->unit_border)
		UnitBorder_Delete(unit->unit_border); 
	unit->unit_border = NULL; 
	unit->inter_func->inter_func.Internal_Release(unit); 
	free(unit); 
}
static int FisheyeUnit_InitByCJSON(FisheyeUnit* fu, FisheyeUnitContext* fu_ctx, const cJSON* json, const MouseEvent* mouse_event, const FisheyeUnit_Function* unit_func, void* arg)
{
	*fu = FisheyeUnit_DefaultValue; 
	fu->aspect_r = fu_ctx->aspect_r; 
	fu->fu_ctx = fu_ctx; 
	fu->mouseEvent = mouse_event; 
	cJSON* tmp = cJSON_GetObjectItem(json, "name"); 
	if(tmp)
	{
		if (cJSON_String != tmp->type) return -1; 
		strncpy(fu->name, tmp->valuestring, sizeof(fu->name)-1); 
	}
	tmp = cJSON_GetObjectItem(json, "region"); 
	if (tmp)
	{
		if (cJSON_Object != tmp->type) return -1; 
		if (MSRectf_InitByCJSON(&fu->relaRect, tmp) < 0) return -1; 
	}
	tmp = cJSON_GetObjectItem(json, "isActive"); 
	if (tmp)
	{
		switch (tmp->type)
		{
		case cJSON_True:
			fu->isActive = 1; 
			break; 
		case cJSON_False:
			fu->isActive = 0; 
			break; 
		default:
			return -1; 
		}
	}
	tmp = cJSON_GetObjectItem(json, "color"); 
	if (tmp)
	{
		if (cJSON_String != tmp->type) 
			return -1; 
		if(1 != sscanf(tmp->valuestring, "%x", &fu->color))
		{
			return -1; 
		}
	}
	fu->inter_func = unit_func; 
	if (unit_func->inter_func.Internal_Init(fu, arg))
	{
		DEBUG_WENGX(); 
		return -1; 
	}
	
	fu->extra_draws = NULL; 
	fu->unit_border = NULL; 
	tmp = cJSON_GetObjectItem(json, "enableBorder"); 
	if (NULL == tmp || cJSON_False != tmp->type)
	{
		if(NULL == (fu->unit_border = UnitBorder_New(fu)) )
		{
			DEBUG_WENGX(); 
			unit_func->inter_func.Internal_Release(fu); 
			return -1; 
		}
		if(unit_func->InitUnitBorder(fu, fu->unit_border))
		{
			DEBUG_WENGX(); 
			UnitBorder_Delete(fu->unit_border); 
			unit_func->inter_func.Internal_Release(fu); 
			return -1; 
		}
		UnitBorder** ub_target = (UnitBorder**)AllocElement(&fu->extra_draws, sizeof(UnitBorder*)); 
		if(NULL == ub_target)
		{
			DEBUG_WENGX(); 
			UnitBorder_Delete(fu->unit_border); 
			unit_func->inter_func.Internal_Release(fu); 
			return -1; 
		}
		*ub_target = fu->unit_border; 
	}
	fu->R_Frames = (WireFrame**)malloc(sizeof(WireFrame*)); 
	if(NULL == fu->R_Frames)
	{
		DEBUG_WENGX(); 
		UnitBorder_Delete(fu->unit_border); 
		unit_func->inter_func.Internal_Release(fu); 
		free(fu->extra_draws); 
		return -1; 
	}
	fu->R_Frames[0] = NULL; 
	return 0; 
}
static void FisheyeUnit_OnLButtonDown(FisheyeUnit* unit, unsigned int nFlags, MSPointf point)
{
	return ; 
}
static void FisheyeUnit_OnLButtonUp(FisheyeUnit* unit, unsigned int nFlags, MSPointf point)
{
	return ; 
}
static void FisheyeUnit_OnMouseMove(FisheyeUnit* unit, unsigned int nFlags, MSPointf point)
{
	return ; 
}
static void FisheyeUnit_OnMouseWheel(FisheyeUnit*unit, unsigned int nFlags, short zDelta, MSPointf point)
{
	return ; 
}
static void FisheyeUnit_exec_cmd(FisheyeUnit*unit, enum Fisheye_CMD f_cmd)
{
	return ; 
}

static void FisheyeUnit_set_preset(FisheyeUnit*unit, float pan, float tile, float zoom)
{
	return ; 
}

#define FisheyeUnitO_DefaultValue (FisheyeUnitO){\
	.unit =  FisheyeUnit_DefaultValue,\
	.orig = {{0}},\
}
static int FisheyeUnitO_InitByCJSON(FisheyeUnitO* fu, FisheyeUnitContext* fu_ctx, const cJSON* json, const FisheyeUnit_Function* unit_o_func, void* arg)
{
	*fu = FisheyeUnitO_DefaultValue; 
	cJSON* tmp = cJSON_GetObjectItem(json, "type"); 
	if (tmp)
	{
		if (cJSON_String != tmp->type || strcmp(tmp->valuestring, "UnitO"))
			return -1; 
		strncpy(fu->unit.type, "UnitO", sizeof(fu->unit.type)-1); 
	}
	if(FisheyeUnit_InitByCJSON(&fu->unit, fu_ctx, json, &fisheye_unit_o_mouse_event, unit_o_func, arg)<0)
		return -1; 
	tmp = cJSON_GetObjectItem(json, "orgi"); 
	if (tmp)
	{
		if (cJSON_Object != tmp->type) return -1; 
		if (FisheyeParamO_InitByCJSON(&fu->orig, tmp) < 0)
			return -1; 
	}
	return 0; 
}

static int FisheyeUnitO_UnInit(FisheyeUnitO* fu)
{
	return 0; 
}

int FisheyeUnitO_SetParam(FisheyeUnitO* unit, const FisheyePlayerParam* para)
{
	return 0; 
}

int FisheyeUnitO_GetParam(FisheyeUnitO* unit, FisheyePlayerParam* para)
{
	para->params[0] = 0.0f;
	para->params[1] = 0.0f;
	para->params[2] = 0.0f;
	return 0;
}
FisheyeUnitO* FisheyeUnitO_NewByCJSON(const cJSON* json, FisheyeUnitContext* fu_ctx, const FisheyeUnit_Function* unit_o_func, void* arg)
{
	FisheyeUnitO* fu = (FisheyeUnitO*)malloc(sizeof(FisheyeUnitO)); 
	if (NULL == fu)
		return NULL; 
	if (FisheyeUnitO_InitByCJSON(fu, fu_ctx, json, unit_o_func, arg))
	{
		free(fu); 
		return NULL; 
	}
	fu->unit.SetParam = (int(*)(struct FisheyeUnit*, const FisheyePlayerParam*))FisheyeUnitO_SetParam;
	fu->unit.GetParam = (int(*)(struct FisheyeUnit*, FisheyePlayerParam*))FisheyeUnitO_GetParam;
	return fu; 
}

INLINE int FisheyeUnitO_Init(FisheyeUnitO* unit, float x, float y, float w, float h, FisheyeParamO* param)
{
	FisheyeUnit_Init(&unit->unit, x, y, w, h, &fisheye_unit_o_mouse_event);
	unit->orig = *param;
	return 0;
}

void FisheyeUnitO_Delete(FisheyeUnitO* fu)
{
	FisheyeUnitO_UnInit(fu);
	free(fu);
}

static void FisheyeUnitO_OnLButtonDown(FisheyeUnitO* unit, unsigned int nFlags, MSPointf point)
{
	MSPointf point_t = {(point.x - unit->unit.relaRect.x)/unit->unit.relaRect.w, (point.y - unit->unit.relaRect.y)/unit->unit.relaRect.h}; 
	MSRectf correct_region = unit->unit.fu_ctx->correct_region; 
	point_t = (MSPointf){(point_t.x - correct_region.x)/correct_region.w, 
							(point_t.y - correct_region.y)/correct_region.h}; 
	if (NULL == unit->unit.R_Frames)
		return; 
	//TODO 当鼠标按下时， 判断哪个线框被选中。
	SetWireFramesStatus(unit->unit.R_Frames, WireFrame_normol); 
	unit->unit.curFrame = GetActiveWireFrame(unit->unit.R_Frames, point_t); 
	if (unit->unit.curFrame)
	{
		WireFrame_SetStatus(unit->unit.curFrame, WireFrame_pressed);
        unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 2, unit->unit.name);
	}
	else
	{
		unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 0, unit->unit.name); 
	}
}

static void FisheyeUnitO_OnLButtonUp(FisheyeUnitO* unit, unsigned int nFlags, MSPointf point)
{
    MSPointf point_t = {(point.x - unit->unit.relaRect.x)/unit->unit.relaRect.w, (point.y - unit->unit.relaRect.y)/unit->unit.relaRect.h};
    MSRectf correct_region = unit->unit.fu_ctx->correct_region;
    point_t = (MSPointf){(point_t.x - correct_region.x)/correct_region.w,
                            (point_t.y - correct_region.y)/correct_region.h};
    if (NULL == unit->unit.R_Frames)
        return;
    //TODO 当鼠标按下时， 判断哪个线框被选中。
    SetWireFramesStatus(unit->unit.R_Frames, WireFrame_normol);
    unit->unit.curFrame = GetActiveWireFrame(unit->unit.R_Frames, point_t);
    if (unit->unit.curFrame)
    {
        WireFrame_SetStatus(unit->unit.curFrame, WireFrame_pressed);
        unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 1, unit->unit.name);
    }
    else
    {
        unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 0, unit->unit.name);
    }
}

static void FisheyeUnitO_OnMouseMove(FisheyeUnitO* unit, unsigned int nFlags, MSPointf point)
{
	MSPointf point_t = {(point.x - unit->unit.relaRect.x)/unit->unit.relaRect.w, (point.y - unit->unit.relaRect.y)/unit->unit.relaRect.h}; 
	MSRectf correct_region = unit->unit.fu_ctx->correct_region; 
	point_t = (MSPointf){(point_t.x - correct_region.x)/correct_region.w, 
							(point_t.y - correct_region.y)/correct_region.h}; 
	if (FISHEYE_LBUTTON == nFlags && unit->unit.curFrame)
	{
		WireFrame_ChgPTZByXY(unit->unit.curFrame, point_t);
	}
	else if (0 == nFlags)
	{
		SetWireFramesStatus(unit->unit.R_Frames, WireFrame_normol);
		unit->unit.curFrame = GetActiveWireFrame(unit->unit.R_Frames, point_t);
		if (unit->unit.curFrame)
		{
			WireFrame_SetStatus(unit->unit.curFrame, WireFrame_touched);
			unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 1, unit->unit.name);
		}
		else
		{
			unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 0, unit->unit.name);
		}
	}
}

static void FisheyeUnitO_OnMouseWheel(FisheyeUnitO*unit, unsigned int nFlags, short zDelta, MSPointf point)
{
	WireFrame* frame = unit->unit.curFrame;
	if (NULL == frame)
		return;
	FisheyeParamR_IncTheta(frame->r_param, AngleToRadian( zDelta>0 ? 2.0f : -2.0f )); 
}
static void FisheyeUnitO_ExecCmd(FisheyeUnitO*unit, enum Fisheye_CMD f_cmd)
{
	switch(f_cmd)
	{
	case fcmd_stop: 
		break; 
	case fcmd_focusin: 
		break; 
	case fcmd_focusout: 
		break; 
	case fcmd_zoomin: 
		break; 
	case fcmd_zoomout: 
		break; 
	case fcmd_up: 
		break; 
	case fcmd_down: 
		break; 
	case fcmd_left: 
		break; 
	case fcmd_upleft: 
		break; 
	case fcmd_upright: 
		break; 
	case fcmd_downleft: 
		break; 
	case fcmd_downright: 
		break; 
	case fcmd_right: 
		break; 
	case fcmd_auto: 
		break; 
	case fcmd_irisin: 
		break; 
	case fcmd_irisout: 
		break; 
	case fcmd_autofocus: 
		break; 
	case fcmd_resetfocus: 
		break; 
	case fcmd_zoompos: 
		break; 
	case fcmd_focuspos: 
		break; 
	case fcmd_irispos: 
		break; 
	default: 
		break;
	};
	return ; 
}



#define FisheyeUnitP_DefaultValue (FisheyeUnitP){\
	.unit =  FisheyeUnit_DefaultValue,\
	.pano = {{0}},\
}

int FisheyeUnitP_UnInit(FisheyeUnitP* fu)
{
	// TODO
	return 0; 
}

INLINE int FisheyeUnitP_Init(FisheyeUnitP* unit, float x, float y, float w, float h, FisheyeParamP* param)
{
	FisheyeUnit_Init(&unit->unit, x, y, w, h, &fisheye_unit_p_mouse_event);
	unit->pano = *param;
	return 0;
}

int FisheyeUnitP_Delete(FisheyeUnitP* fu)
{
	FisheyeUnitP_UnInit(fu);
	free(fu); 
	return 0; 
}

static int FisheyeUnitP_InitByCJSON(FisheyeUnitP* fu, FisheyeUnitContext* fu_ctx, const cJSON* json, const FisheyeUnit_Function* unit_p_func, void* arg)
{
	*fu = FisheyeUnitP_DefaultValue; 
	if (FisheyeUnit_InitByCJSON(&fu->unit, fu_ctx, json, &fisheye_unit_p_mouse_event, unit_p_func, arg)<0)
		return -1; 
	FisheyeParamP_Init(&fu->pano); 
	cJSON* tmp = cJSON_GetObjectItem(json, "type"); 
	if (tmp)
	{
		if (cJSON_String != tmp->type || strcmp(tmp->valuestring, "UnitP"))
			return -1; 
		strncpy(fu->unit.type, "UnitP", sizeof(fu->unit.type)-1); 
	}
	tmp = cJSON_GetObjectItem(json, "pano"); 
	if (tmp)
	{
		if (cJSON_Object != tmp->type) return -1; 
		if (FisheyeParamP_InitByCJSON(&fu->pano, tmp) < 0)
			return -1; 
	}
	return 0; 
}

int FisheyeUnitP_SetParam(FisheyeUnitP* unit, const FisheyePlayerParam* para)
{
	FisheyeParamP_SetOffset(&unit->pano, para->params[0]);
	return 0;
}

int FisheyeUnitP_GetParam(FisheyeUnitP* unit, FisheyePlayerParam* para)
{
	para->params[0] = unit->pano.offset;
	para->params[1] = 0.0f;
	para->params[2] = 0.0f;
	return 0;
}

FisheyeUnitP* FisheyeUnitP_NewByCJSON(const cJSON* json, FisheyeUnitContext* fu_ctx, const FisheyeUnit_Function* unit_p_func, void* arg)
{
	FisheyeUnitP* fu = (FisheyeUnitP*)malloc(sizeof(FisheyeUnitP)); 
	if (NULL == fu)
		return NULL; 
	if (FisheyeUnitP_InitByCJSON(fu, fu_ctx, json, unit_p_func, arg))
	{
		free(fu); 
		return NULL; 
	}
	fu->pano.install_model = fu_ctx->install_model; 
	fu->unit.SetParam = (int(*)(struct FisheyeUnit*, const FisheyePlayerParam*))FisheyeUnitP_SetParam;
	fu->unit.GetParam = (int(*)(struct FisheyeUnit*, FisheyePlayerParam*))FisheyeUnitP_GetParam;
	return fu; 
}

static void FisheyeUnitP_OnLButtonDown(FisheyeUnitP* unit, unsigned int nFlags, MSPointf point)
{
	MSPointf point_t = {(point.x - unit->unit.relaRect.x)/unit->unit.relaRect.w, (point.y - unit->unit.relaRect.y)/unit->unit.relaRect.h}; 
	SetWireFramesStatus(unit->unit.R_Frames, WireFrame_normol);
	unit->unit.curFrame = GetActiveWireFrame(unit->unit.R_Frames, point_t);
	if (unit->unit.curFrame)
	{
		WireFrame_SetStatus(unit->unit.curFrame, WireFrame_pressed);
		unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 2, unit->unit.name);
	}
	else
	{
		unit->startP = point_t;
		unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 2, unit->unit.name);
	}
}

static void FisheyeUnitP_OnLButtonUp(FisheyeUnitP* unit, unsigned int nFlags, MSPointf point)
{
    MSPointf point_t = {(point.x - unit->unit.relaRect.x)/unit->unit.relaRect.w, (point.y - unit->unit.relaRect.y)/unit->unit.relaRect.h};
    SetWireFramesStatus(unit->unit.R_Frames, WireFrame_normol);
    unit->unit.curFrame = GetActiveWireFrame(unit->unit.R_Frames, point_t);
    if (unit->unit.curFrame)
    {
        WireFrame_SetStatus(unit->unit.curFrame, WireFrame_pressed);
        unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 1, unit->unit.name);
    }
    else
    {
        unit->startP = point_t;
        unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 1, unit->unit.name);
    }
    //
	unit->startP.x = unit->startP.y = -1.0f;
}

static void FisheyeUnitP_OnMouseMove(FisheyeUnitP* unit, unsigned int nFlags, MSPointf point)
{
	MSPointf point_t = {(point.x - unit->unit.relaRect.x)/unit->unit.relaRect.w, (point.y - unit->unit.relaRect.y)/unit->unit.relaRect.h}; 
	WireFrame* frame = unit->unit.curFrame;
	if (FISHEYE_LBUTTON == nFlags)
	{
		if (frame)
		{
			WireFrame_ChgPTZByXY(frame, point_t);
		}
		else
		{
			// TODO
			float offset = FisheyeParamP_GetRange(&unit->pano) * (unit->startP.x - point_t.x);
			if(Ceilling == unit->pano.install_model->install_model) offset *= -1; 
			FisheyeParamP_IncOffset(&unit->pano, offset);
			unit->startP = point_t; 
		}
	}
	else if (0 == nFlags)
	{
		SetWireFramesStatus(unit->unit.R_Frames, WireFrame_normol);
		unit->unit.curFrame = GetActiveWireFrame(unit->unit.R_Frames, point_t);
		if (unit->unit.curFrame)
		{
			WireFrame_SetStatus(unit->unit.curFrame, WireFrame_touched); 
			unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 1, unit->unit.name); 
		}
		else
		{
			if(FisheyeUnit_Contains((FisheyeUnit*)unit, point.x, point.y))
				unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 1, unit->unit.name);
			else
				unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 0, unit->unit.name);
		}
	}
}

static void FisheyeUnitP_OnMouseWheel(FisheyeUnitP* unit, unsigned int nFlags, short zDelta, MSPointf point)
{
	WireFrame* frame = unit->unit.curFrame;
	if (frame)
		FisheyeParamR_IncTheta(frame->r_param, AngleToRadian( zDelta>0 ? 2.0f : -2.0f )); 
}
static void FisheyeUnitP_ExecCmd(FisheyeUnitP* unit, enum Fisheye_CMD f_cmd)
{
	switch(f_cmd)
	{
	case fcmd_stop: 
		break; 
	case fcmd_focusin: 
		break; 
	case fcmd_focusout: 
		break; 
	case fcmd_zoomin: 
		break; 
	case fcmd_zoomout: 
		break; 
	case fcmd_up: 
		break; 
	case fcmd_down: 
		break; 
	case fcmd_left: 
		FisheyeParamP_IncOffset(&unit->pano, unit->unit.cmd_speed * MS_PI/180.0f); 
		break; 
	case fcmd_upleft: 
		break; 
	case fcmd_upright: 
		break; 
	case fcmd_downleft: 
		break; 
	case fcmd_downright: 
		break; 
	case fcmd_right: 
		FisheyeParamP_IncOffset(&unit->pano, unit->unit.cmd_speed * (-MS_PI/180.0f)); 
		break; 
	case fcmd_auto: 
		break; 
	case fcmd_irisin: 
		break; 
	case fcmd_irisout: 
		break; 
	case fcmd_autofocus: 
		break; 
	case fcmd_resetfocus: 
		break; 
	case fcmd_zoompos: 
		break; 
	case fcmd_focuspos: 
		break; 
	case fcmd_irispos: 
		break; 
	default: 
		break;
	};
	return ; 
}

#define FisheyeUnit2P_DefaultValue (FisheyeUnit2P){\
	.unit =  FisheyeUnit_DefaultValue,\
	.pano = {0},\
}

int FisheyeUnit2P_UnInit(FisheyeUnit2P* fu)
{
	// TODO
	return 0; 
}

INLINE int FisheyeUnit2P_Init(FisheyeUnit2P* unit, float x, float y, float w, float h, FisheyeParamP* param)
{
	FisheyeUnit_Init(&unit->unit, x, y, w, h, &fisheye_unit_p_mouse_event);
	unit->pano = *param;
	return 0;
}

int FisheyeUnit2P_Delete(FisheyeUnit2P* fu)
{
	FisheyeUnit2P_UnInit(fu);
	free(fu); 
	return 0; 
}

static int FisheyeUnit2P_InitByCJSON(FisheyeUnit2P* fu, FisheyeUnitContext* fu_ctx, const cJSON* json, const FisheyeUnit_Function* unit_2p_func, void* arg)
{
//	*fu = FisheyeUnit2P_DefaultValue; 
	FisheyeParamP_Init(&fu->pano); 
	cJSON* tmp = cJSON_GetObjectItem(json, "type"); 
	if (tmp)
	{
		if (cJSON_String != tmp->type || strcmp(tmp->valuestring, "Unit2P"))
			return -1; 
		strncpy(fu->unit.type, "Unit2P", sizeof(fu->unit.type)-1); 
	}
	if (FisheyeUnit_InitByCJSON(&fu->unit, fu_ctx, json, &fisheye_unit_2p_mouse_event, unit_2p_func, arg)<0)
		return -1; 
	tmp = cJSON_GetObjectItem(json, "pano"); 
	if (tmp)
	{
		if (cJSON_Object != tmp->type) return -1; 
		if (FisheyeParamP_InitByCJSON(&fu->pano, tmp) < 0)
			return -1; 
	}
	fu->pano.range = MS_PI; 
	return 0; 
}

int FisheyeUnit2P_SetParam(FisheyeUnit2P* unit, const FisheyePlayerParam* para)
{
	FisheyeParamP_SetOffset(&unit->pano, para->params[0]); 
	return 0;
}

int FisheyeUnit2P_GetParam(FisheyeUnit2P* unit, FisheyePlayerParam* para)
{
	para->params[0] = unit->pano.offset;
	para->params[1] = 0.0f;
	para->params[2] = 0.0f;
	return 0;
}

FisheyeUnit2P* FisheyeUnit2P_NewByCJSON(const cJSON* json, FisheyeUnitContext* fu_ctx, const FisheyeUnit_Function* unit_2p_func, void* arg)
{
	FisheyeUnit2P* fu = (FisheyeUnit2P*)malloc(sizeof(FisheyeUnit2P)); 
	if (NULL == fu)
		return NULL; 
	if (FisheyeUnit2P_InitByCJSON(fu, fu_ctx, json, unit_2p_func, arg))
	{
		free(fu); 
		return NULL; 
	}
	fu->pano.install_model = fu_ctx->install_model;
	fu->unit.SetParam = (int(*)(struct FisheyeUnit*, const FisheyePlayerParam*))FisheyeUnit2P_SetParam;
	fu->unit.GetParam = (int(*)(struct FisheyeUnit*, FisheyePlayerParam*))FisheyeUnit2P_GetParam;
	return fu; 
}


static void FisheyeUnit2P_OnLButtonDown(FisheyeUnit2P* unit, unsigned int nFlags, MSPointf point)
{
	MSPointf point_t = {(point.x - unit->unit.relaRect.x)/unit->unit.relaRect.w, (point.y - unit->unit.relaRect.y)/unit->unit.relaRect.h}; 
	SetWireFramesStatus(unit->unit.R_Frames, WireFrame_normol); 
	unit->unit.curFrame = GetActiveWireFrame(unit->unit.R_Frames, point_t); 
    if (unit->unit.curFrame) {
		WireFrame_SetStatus(unit->unit.curFrame, WireFrame_pressed);
        unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 2, unit->unit.name);
    } else {
        unit->startP = point_t;
        unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 2, unit->unit.name);
    }
}

static void FisheyeUnit2P_OnLButtonUp(FisheyeUnit2P* unit, unsigned int nFlags, MSPointf point)
{
    MSPointf point_t = {(point.x - unit->unit.relaRect.x)/unit->unit.relaRect.w, (point.y - unit->unit.relaRect.y)/unit->unit.relaRect.h};
    SetWireFramesStatus(unit->unit.R_Frames, WireFrame_normol);
    unit->unit.curFrame = GetActiveWireFrame(unit->unit.R_Frames, point_t);
    if (unit->unit.curFrame) {
        WireFrame_SetStatus(unit->unit.curFrame, WireFrame_pressed);
        unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 1, unit->unit.name);
    } else {
        unit->startP = point_t;
        unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 1, unit->unit.name);
    }
    //
	unit->startP.x = unit->startP.y = -1.0f;
}

static void FisheyeUnit2P_OnMouseMove(FisheyeUnit2P* unit, unsigned int nFlags, MSPointf point)
{
	MSPointf point_t = {(point.x - unit->unit.relaRect.x)/unit->unit.relaRect.w, (point.y - unit->unit.relaRect.y)/unit->unit.relaRect.h}; 
	WireFrame* frame = unit->unit.curFrame;
	if (FISHEYE_LBUTTON == nFlags)
	{
		if (frame)
		{
			WireFrame_ChgPTZByXY(frame, point_t);
		}
		else
		{
			// TODO
			float offset = -FisheyeParamP_GetRange(&unit->pano) * (unit->startP.x - point_t.x);
			if(Ceilling == unit->pano.install_model->install_model) offset *= -1; 
			FisheyeParamP_IncOffset(&unit->pano, -offset);
			unit->startP = point_t; 
		}
	}
	else if (0 == nFlags)
	{
		SetWireFramesStatus(unit->unit.R_Frames, WireFrame_normol);
		unit->unit.curFrame = GetActiveWireFrame(unit->unit.R_Frames, point_t);
        if (unit->unit.curFrame) {
			WireFrame_SetStatus(unit->unit.curFrame, WireFrame_touched);
            unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 1, unit->unit.name);
        } else {
            if(FisheyeUnit_Contains((FisheyeUnit*)unit, point.x, point.y)) {
                unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 1, unit->unit.name);
            } else {
                unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 0, unit->unit.name);
            }
        }
	}
}

static void FisheyeUnit2P_OnMouseWheel(FisheyeUnit2P*unit, unsigned int nFlags, short zDelta, MSPointf point)
{
	WireFrame* frame = unit->unit.curFrame;
	if (frame)
		FisheyeParamR_IncTheta(frame->r_param, AngleToRadian( zDelta>0 ? 2.0f : -2.0f )); 
}


#define FisheyeUnitW_DefaultValue (FisheyeUnitW){\
	.unit =  FisheyeUnit_DefaultValue,\
	.wparam = {0},\
}

int FisheyeUnitW_UnInit(FisheyeUnitW* fu)
{
	// TODO
	return 0;
}

INLINE int FisheyeUnitW_Init(FisheyeUnitW* unit, float x, float y, float w, float h, FisheyeParamW* param)
{
	FisheyeUnit_Init(&unit->unit, x, y, w, h, &fisheye_unit_w_mouse_event);
	unit->wparam = *param;
	return 0;
}

int FisheyeUnitW_Delete(FisheyeUnitW* fu)
{
	FisheyeUnitW_UnInit(fu);
	free(fu);
	return 0;
}

static int FisheyeUnitW_InitByCJSON(FisheyeUnitW* fu, FisheyeUnitContext* fu_ctx, const cJSON* json, const FisheyeUnit_Function* unit_w_func, void* arg)
{
//	*fu = FisheyeUnitW_DefaultValue; 
	cJSON* tmp = cJSON_GetObjectItem(json, "type");
	if (tmp)
	{
		if (cJSON_String != tmp->type || strcmp(tmp->valuestring, "UnitW"))
			return -1;
		strncpy(fu->unit.type, "UnitW", sizeof(fu->unit.type) - 1);
	}
	if (FisheyeUnit_InitByCJSON(&fu->unit, fu_ctx, json, &fisheye_unit_w_mouse_event, unit_w_func, arg)<0)
		return -1;
	tmp = cJSON_GetObjectItem(json, "w_param");
	if (tmp)
	{
		if (cJSON_Object != tmp->type) return -1;
		if (FisheyeParamW_InitByCJSON(&fu->wparam, tmp) < 0)
			return -1;
	}
	return 0;
}


int FisheyeUnitW_SetParam(FisheyeUnitW* unit, const FisheyePlayerParam* para)
{
	return 0;
}

int FisheyeUnitW_GetParam(FisheyeUnitW* unit, FisheyePlayerParam* para)
{
	para->params[0] = 0.0f;
	para->params[1] = 0.0f;
	para->params[2] = 0.0f;
	return 0;
}

FisheyeUnitW* FisheyeUnitW_NewByCJSON(const cJSON* json, FisheyeUnitContext* fu_ctx, const FisheyeUnit_Function* unit_w_func, void* arg)
{
	FisheyeUnitW* fu = (FisheyeUnitW*)malloc(sizeof(FisheyeUnitW));
	if (NULL == fu)
		return NULL;
	if (FisheyeUnitW_InitByCJSON(fu, fu_ctx, json, unit_w_func, arg))
	{
		free(fu);
		return NULL;
	}
	fu->unit.SetParam = (int(*)(struct FisheyeUnit*, const FisheyePlayerParam*))FisheyeUnitW_SetParam;
	fu->unit.GetParam = (int(*)(struct FisheyeUnit*, FisheyePlayerParam*))FisheyeUnitW_GetParam;
	return fu;
}

static void FisheyeUnitW_OnLButtonDown(FisheyeUnitW* unit, unsigned int nFlags, MSPointf point)
{
	MSPointf point_t = {(point.x - unit->unit.relaRect.x)/unit->unit.relaRect.w, (point.y - unit->unit.relaRect.y)/unit->unit.relaRect.h}; 
	SetWireFramesStatus(unit->unit.R_Frames, WireFrame_normol);
	unit->unit.curFrame = GetActiveWireFrame(unit->unit.R_Frames, point_t);
	if (unit->unit.curFrame)
	{
		WireFrame_SetStatus(unit->unit.curFrame, WireFrame_pressed);
        unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 2, unit->unit.name);
	}
	else
	{
		unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 0, unit->unit.name);
	}
}

static void FisheyeUnitW_OnLButtonUp(FisheyeUnitW* unit, unsigned int nFlags, MSPointf point)
{
    MSPointf point_t = {(point.x - unit->unit.relaRect.x)/unit->unit.relaRect.w, (point.y - unit->unit.relaRect.y)/unit->unit.relaRect.h};
    SetWireFramesStatus(unit->unit.R_Frames, WireFrame_normol);
    unit->unit.curFrame = GetActiveWireFrame(unit->unit.R_Frames, point_t);
    if (unit->unit.curFrame)
    {
        WireFrame_SetStatus(unit->unit.curFrame, WireFrame_pressed);
        unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 1, unit->unit.name);
    }
    else
    {
        unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 0, unit->unit.name);
    }
}

static void FisheyeUnitW_OnMouseMove(FisheyeUnitW* unit, unsigned int nFlags, MSPointf point)
{
	MSPointf point_t = {(point.x - unit->unit.relaRect.x)/unit->unit.relaRect.w, (point.y - unit->unit.relaRect.y)/unit->unit.relaRect.h}; 
	if (FISHEYE_LBUTTON == nFlags && unit->unit.curFrame)
	{
		WireFrame_ChgPTZByXY(unit->unit.curFrame, point_t);
	}
	else if (0 == nFlags)
	{
		SetWireFramesStatus(unit->unit.R_Frames, WireFrame_normol);
		unit->unit.curFrame = GetActiveWireFrame(unit->unit.R_Frames, point_t);
		if (unit->unit.curFrame)
		{
			WireFrame_SetStatus(unit->unit.curFrame, WireFrame_touched);
			unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 1, unit->unit.name);
		}
		else
		{
			unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 0, unit->unit.name);
		}
	}
}

static void FisheyeUnitW_OnMouseWheel(FisheyeUnitW*unit, unsigned int nFlags, short zDelta, MSPointf point)
{
	WireFrame* frame = unit->unit.curFrame;
	if (frame)
		FisheyeParamR_IncTheta(frame->r_param, AngleToRadian( zDelta>0 ? 2.0f : -2.0f )); 
}

static void FisheyeUnitW_ExecCmd(FisheyeUnitW* unit, enum Fisheye_CMD f_cmd)
{
	switch(f_cmd)
	{
	case fcmd_stop: 
		break; 
	case fcmd_focusin: 
		break; 
	case fcmd_focusout: 
		break; 
	case fcmd_zoomin: 
		break; 
	case fcmd_zoomout: 
		break; 
	case fcmd_up: 
		break; 
	case fcmd_down: 
		break; 
	case fcmd_left: 
		break; 
	case fcmd_upleft: 
		break; 
	case fcmd_upright: 
		break; 
	case fcmd_downleft: 
		break; 
	case fcmd_downright: 
		break; 
	case fcmd_right: 
		break; 
	case fcmd_auto: 
		break; 
	case fcmd_irisin: 
		break; 
	case fcmd_irisout: 
		break; 
	case fcmd_autofocus: 
		break; 
	case fcmd_resetfocus: 
		break; 
	case fcmd_zoompos: 
		break; 
	case fcmd_focuspos: 
		break; 
	case fcmd_irispos: 
		break; 
	default: 
		break;
	};
	return ; 
}


#define FisheyeUnitR_DefaultValue (FisheyeUnitR){\
	.unit =  FisheyeUnit_DefaultValue,\
	.ptz = 0,\
	.start = {0, 0},\
	.pre = {0, 0}\
}

int FisheyeUnitR_UnInit(FisheyeUnitR* fu)
{
	// TODO
//	if(fu->f_osd_zoom)
//		FisheyeOSDZoom_Delete(fu->f_osd_zoom);
	return 0;
}

INLINE int FisheyeUnitR_Init(FisheyeUnitR* unit, float x, float y, float w, float h, FisheyeParamR* param)
{
	FisheyeUnit_Init(&unit->unit, x, y, w, h, &fisheye_unit_r_mouse_event);
	unit->ptz = *param;
	return 0;
}

int FisheyeUnitR_Delete(FisheyeUnitR* fu)
{
	FisheyeUnitR_UnInit(fu); 
	free(fu);
	return 0;
}

static int FisheyeUnitR_InitByCJSON(FisheyeUnitR* fu, FisheyeUnitContext* fu_ctx, const cJSON* json, const FisheyeUnit_Function* unit_r_func, void* arg)
{
//	*fu = FisheyeUnitR_DefaultValue;
	cJSON* tmp = cJSON_GetObjectItem(json, "type");
	if (tmp)
	{
		if (cJSON_String != tmp->type || strcmp(tmp->valuestring, "UnitR"))
			return -1;
		strncpy(fu->unit.type, "UnitR", sizeof(fu->unit.type) - 1);
	}
	if (FisheyeUnit_InitByCJSON(&fu->unit, fu_ctx, json, &fisheye_unit_r_mouse_event, unit_r_func, arg)<0)
		return -1;
	tmp = cJSON_GetObjectItem(json, "ptz");
	if (tmp)
	{
		if (cJSON_Object != tmp->type) return -1;
		if (FisheyeParamR_InitByCJSON(&fu->ptz, tmp) < 0)
			return -1;
	}
//	fu->f_osd_zoom = FisheyeOSDZoom_New(fu); 
//	if(NULL == fu->f_osd_zoom)
//		return 0; 
//	FisheyeOSDZoom** f_osd_zoom = (FisheyeOSDZoom**)AllocElement(&fu->unit.extra_draws, sizeof(FisheyeOSDZoom*)); 
//	if(NULL == f_osd_zoom)
//	{
//		DEBUG_WENGX(); 
//		FisheyeOSDZoom_Delete(fu->f_osd_zoom); 
//		// TODO
//		return -1; 
//	}
//	*f_osd_zoom = fu->f_osd_zoom; 
	
	return 0;
}

int FisheyeUnitR_SetParam(FisheyeUnitR* unit, const FisheyePlayerParam* para)
{
	FisheyeParamR_SetAlpha(&unit->ptz, para->params[0]);
	FisheyeParamR_SetBeta(&unit->ptz, para->params[1]);
	FisheyeParamR_SetTheta(&unit->ptz, para->params[2]);
	return 0;
}

int FisheyeUnitR_GetParam(FisheyeUnitR* unit, FisheyePlayerParam* para)
{
	para->params[0] = unit->ptz.alpha;
	para->params[1] = unit->ptz.beta;
	para->params[2] = unit->ptz.theta;
	return 0;
}

FisheyeUnitR* FisheyeUnitR_NewByCJSON(const cJSON* json, FisheyeUnitContext* fu_ctx, const FisheyeUnit_Function* unit_r_func, void* arg)
{
	FisheyeUnitR* fu = (FisheyeUnitR*)malloc(sizeof(FisheyeUnitR));
	if (NULL == fu)
		return NULL;
	if (FisheyeUnitR_InitByCJSON(fu, fu_ctx, json, unit_r_func, arg))
	{
		free(fu);
		return NULL;
	}
	fu->ptz.install_model = fu_ctx->install_model; 
	fu->unit.SetParam = (int(*)(struct FisheyeUnit*, const FisheyePlayerParam*))FisheyeUnitR_SetParam;
	fu->unit.GetParam = (int(*)(struct FisheyeUnit*, FisheyePlayerParam*))FisheyeUnitR_GetParam;
	return fu;
}

static void FisheyeUnitR_OnLButtonDown(FisheyeUnitR* unit, unsigned int nFlags, MSPointf point)
{
	unit->start = unit->pre = point; 
    unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 2, unit->unit.name);
    if (unit->unit.curFrame) {
        WireFrame_SetStatus(unit->unit.curFrame, WireFrame_pressed);
    }
}

static void FisheyeUnitR_OnLButtonUp(FisheyeUnitR* unit, unsigned int nFlags, MSPointf point)
{
	unit->start = unit->pre = (MSPointf){0.0f, 0.0f};
	unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 1, unit->unit.name); 
}

static void FisheyeUnitR_OnMouseMove(FisheyeUnitR* unit, unsigned int nFlags, MSPointf point)
{
	// TODO 待优化 1. 不要使用具体数据1.0f, 2.0f;  2.
		
	if (FISHEYE_LBUTTON == nFlags)
	{
		unit->unit.f_cmd = fcmd_stop; 
		if (fabs(point.x - unit->start.x)>fabs(point.y - unit->start.y))
			FisheyeParamR_IncBeta(&unit->ptz, point.x - unit->pre.x>0 ? AngleToRadian(-1.0f) : AngleToRadian(1.0f));
		else
			FisheyeParamR_IncAlpha(&unit->ptz, point.y - unit->pre.y>0 ? AngleToRadian(-2.0f) : AngleToRadian(2.0f)); 
		unit->start = unit->pre = point; 
	}
	else
	{
		if(FisheyeUnit_Contains((FisheyeUnit*)unit, point.x, point.y))
			unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 1, unit->unit.name);
		else
			unit->unit.fu_ctx->onWireFrameTouched(unit->unit.fu_ctx->onWireFrameTouched_arg, 0, unit->unit.name);
	}
}

static void FisheyeUnitR_OnMouseWheel(FisheyeUnitR*unit, unsigned int nFlags, short zDelta, MSPointf point)
{
	unit->unit.f_cmd = fcmd_stop; 
	FisheyeParamR_IncTheta(&unit->ptz, AngleToRadian(zDelta>0 ? 2.0f : -2.0f));
}

static void FisheyeUnitR_ExecCmd(FisheyeUnitR* unit, enum Fisheye_CMD f_cmd)
{
	float speed = unit->unit.cmd_speed; 
	switch(f_cmd)
	{
	case fcmd_stop: 
		break; 
	case fcmd_focusin: 
		break; 
	case fcmd_focusout: 
		break; 
	case fcmd_zoomin: 
		FisheyeParamR_IncTheta(&unit->ptz, speed * (-MS_PI/180.0f)); 
		break; 
	case fcmd_zoomout: 
		FisheyeParamR_IncTheta(&unit->ptz, speed * MS_PI/180.0f); 
		break; 
	case fcmd_up: 
		FisheyeParamR_IncAlpha(&unit->ptz, speed * (-MS_PI/180.0f)); 
		break; 
	case fcmd_down: 
		FisheyeParamR_IncAlpha(&unit->ptz, speed * MS_PI/180.0f); 
		break; 
	case fcmd_left: 
		FisheyeParamR_IncBeta(&unit->ptz, speed * (-MS_PI/180.0f)); 
		break; 
	case fcmd_upleft: 
		FisheyeParamR_IncAlpha(&unit->ptz, speed * (-MS_PI/180.0f)); 
		FisheyeParamR_IncBeta(&unit->ptz, speed * (-MS_PI/180.0f)); 
		break; 
	case fcmd_upright: 
		FisheyeParamR_IncAlpha(&unit->ptz, speed * (-MS_PI/180.0f)); 
		FisheyeParamR_IncBeta(&unit->ptz, speed * MS_PI/180.0f); 
		break; 
	case fcmd_downleft: 
		FisheyeParamR_IncAlpha(&unit->ptz, speed * MS_PI/180.0f); 
		FisheyeParamR_IncBeta(&unit->ptz, speed * (-MS_PI/180.0f)); 
		break; 
	case fcmd_downright: 
		FisheyeParamR_IncAlpha(&unit->ptz, speed * MS_PI/180.0f); 
		FisheyeParamR_IncBeta(&unit->ptz, speed * MS_PI/180.0f); 
		break; 
	case fcmd_right: 
		FisheyeParamR_IncBeta(&unit->ptz, speed * MS_PI/180.0f); 
		break; 
	case fcmd_auto: 
		FisheyeParamR_IncBeta(&unit->ptz, speed * (-MS_PI/180.0f)); 
		break; 
	case fcmd_irisin: 
		break; 
	case fcmd_irisout: 
		break; 
	case fcmd_autofocus: 
		break; 
	case fcmd_resetfocus: 
		break; 
	case fcmd_zoompos: 
		break; 
	case fcmd_focuspos: 
		break; 
	case fcmd_irispos: 
		break; 
	default: 
		break;
	};
	return ; 
}
static void FisheyeUnitR_SetPreset(FisheyeUnitR*unit, float pan, float tile, float zoom)
{	
	FisheyeParamR_SetTheta(&unit->ptz, AngleToRadian(120.0f - zoom*1.05f)); 
	FisheyeParamR_SetAlpha(&unit->ptz, AngleToRadian((3600 - (int)pan)%360)); 
	FisheyeParamR_SetAlpha(&unit->ptz, AngleToRadian(tile - 180)/2); 
	return ; 
}

int	FisheyeUnit_SetParam(FisheyeUnit* unit, const FisheyePlayerParam* param)
{
	unit->SetParam(unit, param); 
	return 0; 
}

int	FisheyeUnit_GetParam(FisheyeUnit* unit, FisheyePlayerParam* param)
{
	memcpy(param->unitName, unit->name, MAX_NAME_LENGTH);
	unit->GetParam(unit, param);
	return 0; 
}

const MouseEvent fisheye_unit_mouse_event = {
	.OnLButtonDown = (void(*)(void*, unsigned int, MSPointf))&FisheyeUnit_OnLButtonDown,
	.OnLButtonUp = (void(*)(void*, unsigned int, MSPointf))&FisheyeUnit_OnLButtonUp,
	.OnMouseMove = (void(*)(void*, unsigned int, MSPointf))&FisheyeUnit_OnMouseMove,
	.OnMouseWheel = (void(*)(void*, unsigned int, short, MSPointf))&FisheyeUnit_OnMouseWheel, 
	.ExecCMD = (void(*)(void*, enum Fisheye_CMD))&FisheyeUnit_exec_cmd, 
	.SetPreset = (void(*)(void*, float, float, float))&FisheyeUnit_set_preset
}; 

const MouseEvent fisheye_unit_o_mouse_event = {
	.OnLButtonDown = (void(*)(void*, unsigned int, MSPointf))&FisheyeUnitO_OnLButtonDown,
	.OnLButtonUp = (void(*)(void*, unsigned int, MSPointf))&FisheyeUnitO_OnLButtonUp,
	.OnMouseMove = (void(*)(void*, unsigned int, MSPointf))&FisheyeUnitO_OnMouseMove,
	.OnMouseWheel = (void(*)(void*, unsigned int, short, MSPointf))&FisheyeUnitO_OnMouseWheel, 
	.ExecCMD = (void(*)(void*, enum Fisheye_CMD))&FisheyeUnitO_ExecCmd, 
	.SetPreset = (void(*)(void*, float, float, float))&FisheyeUnit_set_preset
};

const MouseEvent fisheye_unit_p_mouse_event = {
	.OnLButtonDown = (void(*)(void*, unsigned int, MSPointf))&FisheyeUnitP_OnLButtonDown,
	.OnLButtonUp = (void(*)(void*, unsigned int, MSPointf))&FisheyeUnitP_OnLButtonUp,
	.OnMouseMove = (void(*)(void*, unsigned int, MSPointf))&FisheyeUnitP_OnMouseMove,
	.OnMouseWheel = (void(*)(void*, unsigned int, short, MSPointf))&FisheyeUnitP_OnMouseWheel, 
	.ExecCMD = (void(*)(void*, enum Fisheye_CMD))&FisheyeUnitP_ExecCmd, 
	.SetPreset = (void(*)(void*, float, float, float))&FisheyeUnit_set_preset
};
	
const MouseEvent fisheye_unit_2p_mouse_event = {
	.OnLButtonDown = (void(*)(void*, unsigned int, MSPointf))&FisheyeUnit2P_OnLButtonDown,
	.OnLButtonUp = (void(*)(void*, unsigned int, MSPointf))&FisheyeUnit2P_OnLButtonUp,
	.OnMouseMove = (void(*)(void*, unsigned int, MSPointf))&FisheyeUnit2P_OnMouseMove,
	.OnMouseWheel = (void(*)(void*, unsigned int, short, MSPointf))&FisheyeUnit2P_OnMouseWheel, 
	.ExecCMD = (void(*)(void*, enum Fisheye_CMD))&FisheyeUnitP_ExecCmd, 
	.SetPreset = (void(*)(void*, float, float, float))&FisheyeUnit_set_preset
};

const MouseEvent fisheye_unit_w_mouse_event = {
	.OnLButtonDown = (void(*)(void*, unsigned int, MSPointf))&FisheyeUnitW_OnLButtonDown,
	.OnLButtonUp = (void(*)(void*, unsigned int, MSPointf))&FisheyeUnitW_OnLButtonUp,
	.OnMouseMove = (void(*)(void*, unsigned int, MSPointf))&FisheyeUnitW_OnMouseMove,
	.OnMouseWheel = (void(*)(void*, unsigned int, short, MSPointf))&FisheyeUnitW_OnMouseWheel, 
	.ExecCMD = (void(*)(void*, enum Fisheye_CMD))&FisheyeUnitW_ExecCmd, 
	.SetPreset = (void(*)(void*, float, float, float))&FisheyeUnit_set_preset
};

const MouseEvent fisheye_unit_r_mouse_event = {
	.OnLButtonDown = (void(*)(void*, unsigned int, MSPointf))&FisheyeUnitR_OnLButtonDown,
	.OnLButtonUp = (void(*)(void*, unsigned int, MSPointf))&FisheyeUnitR_OnLButtonUp,
	.OnMouseMove = (void(*)(void*, unsigned int, MSPointf))&FisheyeUnitR_OnMouseMove,
	.OnMouseWheel = (void(*)(void*, unsigned int, short, MSPointf))&FisheyeUnitR_OnMouseWheel, 
	.ExecCMD = (void(*)(void*, enum Fisheye_CMD))&FisheyeUnitR_ExecCmd, 
	.SetPreset = (void(*)(void*, float, float, float))&FisheyeUnitR_SetPreset
};
