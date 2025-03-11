#include"fisheye_player.h"
#ifdef __linux__
#include"mode_json.h"
#endif // __linux__

#include<string.h>

extern const FisheyeContextType* const context_type_list[]; 

void onWireFrameTouched_Default(void* arg, int state, char* unit_name)
{
	return; 
}

static MSRectf set_correct_region(int w, int h, float aspect_o, MSRectf fish_region)
{
	MSRectf correct_region = { 0 }; 
	float judge = aspect_o - 1.0f;
	if (judge > 0)
	{
		correct_region.h = fish_region.h;
		correct_region.w = (fish_region.h*h) / w;
		correct_region.x = fish_region.x + (fish_region.w - correct_region.w) / 2.0f;
		correct_region.y = fish_region.y;
	}
	else if (judge < 0)
	{
		correct_region.w = fish_region.w;
		correct_region.h = (fish_region.w*w) / h;
		correct_region.x = fish_region.x;
		correct_region.y = fish_region.y + (fish_region.h - correct_region.h) / 2.0f; ;
	}
	else
	{
		correct_region = fish_region;
	}
	return correct_region;
}

int FisheyePlayer_Init(FisheyePlayer* fp, MSRecti* rT, MSRectf* fish_region, int w, int h, const FisheyePlayerContextType* context_type, void* arg)
{
	fp->renderTarget = *rT; 
	fp->fisheye_region = *fish_region; 
	fp->aspect_r = 1.0f; 
	fp->width = w; 
	fp->height = h; 
	fp->aspect_o = (fish_region->w*w) / (fish_region->h*h); 

	fp->correct_region = set_correct_region(w, h, fp->aspect_o, fp->fisheye_region); 

	fp->context_type = context_type; 

	if (fp->context_type->Init(fp, arg)<0) {
        printf("FisheyePlayer_Init context_type init failed.\n");
		return -1; 
	}
	if( NULL == ( fp->modes = (FisheyeMode**)malloc(sizeof(FisheyeMode*)) ) )
		return -1; 
	memset(fp->modes, 0, sizeof(FisheyeMode*)); 
	fp->cur_cnt = 0; 
	fp->cmd_speed = 1.0f; 
	fp->stretch_mode = StretchMode_Original; 
	fp->install_model.install_model = flat; 
	fp->onWireFrameTouched = onWireFrameTouched_Default; 
	fp->onWireFrameTouched_arg = NULL; 
	fp->zoom_status = f_osd_close; 
	fp->preset_status = f_osd_5s; 
	fp->patrol_status = f_osd_5s; 
	fp->autoScan_status = f_osd_5s; 
	fp->enable_wireframe = 1; 
	return 0; 
}
int FisheyePlayer_UnInit(FisheyePlayer* fp)
{
	FisheyeMode** it = fp->modes; 
	while (*it)
	{
		FisheyeMode_Delete(*it); 
		it++; 
	}
	if(fp->modes)
		free(fp->modes); 
	fp->modes = NULL; 
	fp->context_type->UnInit(fp); 
	return 0; 
}
int FisheyePlayer_addMode(FisheyePlayer* fplayer, FisheyeMode* mode)
{
	return addElement(&fplayer->modes, mode, sizeof(FisheyeMode)); 
}
struct FisheyeMode*  FisheyePlayer_getMode(FisheyePlayer* fplayer, const char* mode)
{
	FisheyeMode** it = fplayer->modes; 
	if (NULL == it)
		return NULL; 
	while (*it){
		if (0 == strcmp((*it)->mode_name, mode))
			return *it; 
		it++; 
	}
	return NULL; 
}
static int FisheyePlayer_CreateMode(FisheyePlayer* fp, char* name)
{
	FisheyeMode** ret = AllocElement(&fp->modes, sizeof(FisheyeMode*)); 
	if (NULL == ret)
		return -1; 
	FisheyeMode* mode = (FisheyeMode*)malloc(sizeof(FisheyeMode)); 
	if (NULL == mode)
		return -1; 
	*ret = mode; 
	return 0; 
}
int FisheyePlayer_AddModeByJSON(FisheyePlayer* fplayer, char* json)
{
	return -1; 
}
MSPointf FisheyePlayer_AbsCoorToRelaCoor(FisheyePlayer* fp, MSPointi point)
{
	float x = (float)(point.x - fp->renderTarget.x); 
	float y = (float)(point.y - fp->renderTarget.y); 
	return (MSPointf){ x / fp->renderTarget.w, y / fp->renderTarget.h }; 
}

//***********************************************************************************************************************

static int FisheyePlayer_AddMode(FisheyePlayer* fplayer, FisheyeMode* mode)
{
	FisheyeMode** ret = AllocElement(&fplayer->modes, sizeof(FisheyeMode*)); 
	if (NULL == ret)
		return -1; 
	*ret = mode; 
	return 0; 
}

static int FisheyePlayer_AddModeByCJSON(FisheyePlayer* fp, const cJSON* json)
{
	FisheyeUnitContext fu_ctx = {
		.install_model = &fp->install_model, 
		.aspect_r = &fp->aspect_r, 
		.aspect_o = &fp->aspect_o, 
		.p_enable_wireframe = &fp->enable_wireframe, 
		.correct_region = (MSRectf){
			.x = (fp->correct_region.x - fp->fisheye_region.x)/fp->fisheye_region.w, 
			.y = (fp->correct_region.y - fp->fisheye_region.y)/fp->fisheye_region.h, 
			.w = fp->correct_region.w/fp->fisheye_region.w, 
			.h = fp->correct_region.h/fp->fisheye_region.h
		}, 
		.aspect_mode = 1.0f, 
		.onWireFrameTouched = fp->onWireFrameTouched, 
		.onWireFrameTouched_arg = fp->onWireFrameTouched_arg,
		.zoom_status = &fp->zoom_status, 
		.preset_status = &fp->preset_status, 
		.patrol_status = &fp->patrol_status, 
		.autoScan_status = &fp->autoScan_status
		}; 
	FisheyeMode* mode = FisheyeMode_NewByCJSON(json, &fu_ctx, fp->context_type->mode_internal, fp->priv_data); 
	if (NULL == mode)
		return -1; 
	FisheyeMode_OnReshape(mode, fp->renderTarget); 
	FisheyeMode_SetStretchMode(mode, &fp->stretch_mode); 
	if (FisheyePlayer_AddMode(fp, mode) < 0)
	{
		FisheyeMode_Delete(mode); 
		return -1; 
	}
	return 0; 
}

int FisheyePlayer_InitByCJSON(FisheyePlayer* fp, MSRecti* rT, MSRectf* cr, int w, int h, const cJSON* json, const FisheyePlayerContextType* context_type, void* arg)
{
	if (FisheyePlayer_Init(fp, rT, cr, w, h, context_type, arg))
		return -1; 

	cJSON* tmp = cJSON_GetObjectItem(json, "cur_cnt"); 
	if (tmp && cJSON_Number == tmp->type)
		fp->cur_cnt = tmp->valueint; 

	tmp = cJSON_GetObjectItem(json, "StretchModel"); 
	if (tmp && cJSON_Number == tmp->type)
		fp->stretch_mode = tmp->valueint; 

	tmp = cJSON_GetObjectItem(json, "InstallModel"); 
	if (tmp && cJSON_Number == tmp->type)
		FisheyePlayer_SetInstallModel(fp, tmp->valueint); 
	tmp = cJSON_GetObjectItem(json, "aspect_r"); 
	if (tmp && cJSON_Number == tmp->type)
		fp->aspect_r = (float)tmp->valuedouble;

	tmp = cJSON_GetObjectItem(json, "DisplayModes"); 
	if (tmp && cJSON_Array == tmp->type)
	{
		int size = cJSON_GetArraySize(tmp); 
		cJSON* item = NULL; 
        int i = 0;
        for (; i < size; i++)
		{
			item = cJSON_GetArrayItem(tmp, i); 
			if (item && cJSON_Object == item->type)
				FisheyePlayer_AddModeByCJSON(fp, item); 
		}
	}
	fp->cur_mode = fp->modes[0]; 
	return 0; 
}
#ifdef __linux__
FisheyePlayer *FisheyePlayer_New(int InputWidth, int InputHeight, int OutputWidth, int OutputHeight)
{
	if (!InputWidth || !InputHeight || !OutputWidth || !OutputHeight)
		return NULL;
	MSRecti renderTarget;
	MSRectf correctRgion;
	FisheyePlayer *player = NULL;
	renderTarget.x = 0;
	renderTarget.y = 0;
	renderTarget.w = OutputWidth;
	renderTarget.h = OutputHeight;
	correctRgion.x = 0.0f;
	correctRgion.y = 0.0f;
	correctRgion.w = 1.0f;
	correctRgion.h = 1.0f;

	player = FisheyePlayer_NewByJSON(&renderTarget, &correctRgion, InputWidth, InputHeight, default_json, "OpenGL", NULL);
	if (player)
		FisheyePlayer_TransformBegin(player, OutputWidth, OutputHeight);
	return player;
}

#else
FisheyePlayer*	FisheyePlayer_New(MSRecti* rT, MSRectf* fish_region, int w, int h, const char* type, void* arg)
{
	const FisheyeContextType* const* it = context_type_list;
	if (NULL == *it) return NULL;

	while (strcmp((*it)->type, type) && *++it);
	if (NULL == *it) return NULL;

	FisheyePlayer* ret = (FisheyePlayer*)malloc(sizeof(FisheyePlayer));
	if (NULL == ret) return NULL;

	if (FisheyePlayer_Init(ret, rT, fish_region, w, h, (*it)->player_internal, arg))
		goto error;

	return ret;
error:
	free(ret);
	return NULL;
}
#endif // __linux__


FisheyePlayer* FisheyePlayer_NewByJSON(MSRecti* rT, MSRectf* fish_region, int w, int h, const char* json, const char* type, void* arg)
{
	const FisheyeContextType* const * it = context_type_list; 
	if (NULL == *it) return NULL; 

	while (strcmp((*it)->type, type) && *++it); 
	if (NULL == *it) return NULL; 

	FisheyePlayer* ret = (FisheyePlayer*)malloc(sizeof(FisheyePlayer)); 
	if (NULL == ret) return NULL; 
    memset(ret, 0, sizeof(FisheyePlayer));

	cJSON* json_player = cJSON_Parse(json); 
	if (NULL == json_player)
	{
		free(ret); 
		return NULL;
	}

	if (FisheyePlayer_InitByCJSON(ret, rT, fish_region, w, h, json_player, (*it)->player_internal, arg))
	{
		cJSON_Delete(json_player); 
		free(ret); 
		return NULL; 
	}
	cJSON_Delete(json_player); 
	return ret; 
}

void FisheyePlayer_Delete(FisheyePlayer* fp)
{
	FisheyePlayer_UnInit(fp); 
	free(fp); 
}
static int FisheyePlayer_OnChanged_Data(FisheyePlayer* fp)
{
	int w = fp->width; 
	int h = fp->height; 
	fp->aspect_o = (fp->fisheye_region.w*w) / (fp->fisheye_region.h*h); 

	fp->correct_region = set_correct_region(w, h, fp->aspect_o, fp->fisheye_region); 

	FisheyeUnitContext fu_ctx = {
		.install_model = &fp->install_model,
		.aspect_r = &fp->aspect_r,
		.aspect_o = &fp->aspect_o,
		.p_enable_wireframe = &fp->enable_wireframe,
		.correct_region = (MSRectf) {
		.x = (fp->correct_region.x - fp->fisheye_region.x) / fp->fisheye_region.w,
			.y = (fp->correct_region.y - fp->fisheye_region.y) / fp->fisheye_region.h,
			.w = fp->correct_region.w / fp->fisheye_region.w,
			.h = fp->correct_region.h / fp->fisheye_region.h
	},
		.aspect_mode = 1.0f,
			.onWireFrameTouched = fp->onWireFrameTouched,
			.onWireFrameTouched_arg = fp->onWireFrameTouched_arg,
			.zoom_status = &fp->zoom_status,
			.preset_status = &fp->preset_status,
			.patrol_status = &fp->patrol_status,
			.autoScan_status = &fp->autoScan_status
	}; 
	FisheyeMode** it = fp->modes;
	while (it && *it)
	{
		(*it)->fu_ctx = fu_ctx; 
		it++;
	}
	fp->context_type->DataChg(fp); 
	fp->install_model.param.seq++; 
	fp->seq++; 
	return 0; 
}
int FisheyePlayer_ResetDataSize(FisheyePlayer* fp, int w, int h)
{
	if (w == fp->width && h == fp->height)
		return 0; 
	fp->width = w; 
	fp->height = h; 
	return FisheyePlayer_OnChanged_Data(fp); 
}

int	FisheyePlayer_ResetFisheyeRegion(FisheyePlayer* fp, MSRectf* fish_region)
{
	if(0 == memcmp(&fp->fisheye_region, fish_region, sizeof(MSRectf)))
		return 0; 
	fp->fisheye_region = *fish_region; 
	return FisheyePlayer_OnChanged_Data(fp); 
}

int FisheyePlayer_Reshape(FisheyePlayer* fp, int x, int y, int w, int h)
{
	fp->renderTarget.x = x; 
	fp->renderTarget.y = y; 
	fp->renderTarget.w = w; 
	fp->renderTarget.h = h; 
	fp->seq++; 
	FisheyeMode** it = fp->modes; 
	while(it && *it)
	{
		FisheyeMode_OnReshape(*it, fp->renderTarget); 
		it++; 
	}
	// TODO 问题来了，要不要通知底层，自己被更改了，感觉还是使用另一个方式会容易一些啊，或者说使用回调函数的形式？？？
	
	return 0; 
}
int	FisheyePlayer_EnableWireFrame(FisheyePlayer* fp, int flag)
{
	if (FALSE != flag || TRUE != flag)
		return -1; 
	fp->enable_wireframe = flag; 
	return 0; 
}

int	FisheyePlayer_SetRAspect(FisheyePlayer* fp, float aspect_f)
{
	fp->aspect_r = aspect_f; 
	FisheyePlayer_SetStretchMode(fp, fp->stretch_mode); 
	return 0; 
}

int	FisheyePlayer_SetStretchMode(FisheyePlayer* fp, enum StretchMode stretch_mode)
{
	fp->stretch_mode = stretch_mode; 
	FisheyeMode** it = fp->modes; 
	while(it && *it)
	{
		// TODO resize
		 FisheyeMode_OnReshape(*it, fp->renderTarget); 
		it++; 
	}
	fp->seq++; 
	return 0; 
}

int	FisheyePlayer_SetPreset(FisheyePlayer* fp, float pan, float tile, float zoom)
{
	FisheyeMode_SetPreset(fp->cur_mode, pan, tile, zoom); 
	return 0; 
}

int	FisheyePlayer_SetDisplayMode(FisheyePlayer* fp, const char* mode)
{
	FisheyeMode** it = fp->modes; 
	while (*it)
	{
		if (0 == strncmp(mode, FisheyeMode_GetName(*it), MAX_NAME_LENGTH))
		{
			fp->cur_mode = *it; 
			return 0; 
		}
		it++; 
	}
	return -1; 
}

int FisheyePlayer_GetCurModeUnitsNum(FisheyePlayer* fp)
{
	if (NULL == fp->cur_mode)
		return 0; 
	FisheyeUnit** it = fp->cur_mode->units; 
	int ret = 0; 
	while (*it++)
		ret++; 
	return ret; 
}

int FisheyePlayer_SetCurModeActUnit(FisheyePlayer* fp, int num)
{
	if (num < 0)
		return -1; 
	if (FisheyePlayer_GetCurModeUnitsNum(fp) <= num)
		return -1; 
	if (NULL == fp->cur_mode)
		return -1; 
	FisheyeUnit** it = fp->cur_mode->units;
	while (*it && num)
	{
		it++; 
		num--;
	}
	if (0 == num)
	{
		if (fp->cur_mode->cur_unit)
			fp->cur_mode->cur_unit->isActive = 0;
		fp->cur_mode->cur_unit = *it;
		fp->cur_mode->cur_unit->isActive = 1;
		return 0;
	}
	else
		return -1;
}

int FisheyePlayer_WireFrameTouchedFunc(FisheyePlayer* fp, void(* func)(void*, int, char*), void* arg)
{
	fp->onWireFrameTouched = func; 
	fp->onWireFrameTouched_arg = arg; 
	FisheyeMode** it = fp->modes;
	while (it && *it)
	{
		(*it)->fu_ctx.onWireFrameTouched = func;
		(*it)->fu_ctx.onWireFrameTouched_arg = arg; 
		it++;
	}
	return 0; 
}

int	FisheyePlayer_SetParams(FisheyePlayer* fp, const FisheyePlayerParam params[], int size)
{
	printf("=====%s, %d\n", __func__, __LINE__);
	int i; 
	for(i=0; i<size; i++)
	{
		FisheyeMode* mode = FisheyePlayer_getMode(fp, params[i].modeName);
		if (NULL == mode) 
		{
			DEBUG_PRINTF("WARNNING: no model named[%s]", params[i].modeName);
			continue; 
		}
		FisheyeMode_SetParam(mode, params + i); 
		
	}
	return 0; 
}
int	FisheyePlayer_GetParams(FisheyePlayer* fp, FisheyePlayerParam params[], int size, int* totalNum)
{
	int paraOffset = 0; 
	int tmpCount = 0; 
	int totalParams = 0; 
	FisheyeMode** it = fp->modes;
	while (it && *it)
	{
		totalParams += FisheyeMode_GetUnitParamLength(*it); 
		it++; 
	}
	if (totalParams > size)
		return -1; 

	it = fp->modes;;
	while (it && *it)
	{
		FisheyeMode_GetParam(*it, params + paraOffset, size - tmpCount, &tmpCount); 
		paraOffset += tmpCount;
		it++;
	}
	*totalNum = paraOffset; 
	return 0; 
}
