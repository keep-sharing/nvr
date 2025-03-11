#include"common.h"


void* (* G_Timer_New)(void(* timer_func)(void*, int), void* arg, int val); 
void (* G_Timer_Delete)(void* timer); 
void (* G_Timer_SetTimerEvent)(void* timer, void(* timer_func)(void*, int), void* arg, int val); 
void (* G_Timer_Start)(void* Timer, int val); 
void (* G_Timer_Stop)(void* Timer); 

FisheyeTimer* FisheyeTimer_New(void(* timer_func)(void*, int), void* arg, int val)
{
	FisheyeTimer* ret = (FisheyeTimer*)malloc(sizeof(FisheyeTimer)); 
	ret->data = G_Timer_New(timer_func, arg, val); 
	if(NULL == ret->data)
	{
		free(ret); 
		return NULL; 
	}
	ret->arg = arg; 
	ret->timer_func = timer_func; 
	ret->val = val; 
	return ret; 
}
void FisheyeTimer_Delete(FisheyeTimer* timer)
{
	G_Timer_Delete(timer->data); 
	free(timer); 
}
void FisheyeTimer_SetTimerEvent(FisheyeTimer* timer, void(* timer_func)(void*, int), void* arg, int val)
{
	timer->timer_func = timer_func; 
	timer->arg = arg; 
	timer->val = val; 
	G_Timer_SetTimerEvent(timer->data, timer_func, arg, val); 
}
void FisheyeTimer_Start(FisheyeTimer* timer, int val)
{
	G_Timer_Start(timer->data, val); 
}
void FisheyeTimer_Stop(FisheyeTimer* timer)
{
	G_Timer_Stop(timer->data); 
}


void FisheyeTimer_SetDefaultFunc(FisheyeTimerFunc* func)
{
	G_Timer_New = func->Timer_New; 
	G_Timer_Delete = func->Timer_Delete; 
	G_Timer_SetTimerEvent = func->Timer_SetTimerEvent; 
	G_Timer_Start = func->Timer_Start; 
	G_Timer_Stop = func->Timer_Stop; 
}

// 注意 如果 data全0，则不应该可用， 此处应添加判断。
int MSRectf_InitByCJSON(MSRectf* rectf, const cJSON* json)
{
	*rectf = MSRectf_DefaultValue; 
	cJSON* tmp = cJSON_GetObjectItem(json, "x"); 
	if (tmp)
	{
		if (cJSON_Number != tmp->type) return -1; 
		rectf->x = (float)tmp->valuedouble; 
	}
	tmp = cJSON_GetObjectItem(json, "y"); 
	if (tmp)
	{
		if (cJSON_Number != tmp->type) return -1; 
		rectf->y = (float)tmp->valuedouble; 
	}
	tmp = cJSON_GetObjectItem(json, "w"); 
	if (tmp)
	{
		if (cJSON_Number != tmp->type) return -1; 
		rectf->w = (float)tmp->valuedouble; 
	}
	tmp = cJSON_GetObjectItem(json, "h"); 
	if (tmp)
	{
		if (cJSON_Number != tmp->type) return -1; 
		rectf->h = (float)tmp->valuedouble; 
	}
	return 0; 
}

int MSRectf_Contains(const MSRectf* rect_f, float x, float y)
{
	if (rect_f->x < x && rect_f->y < y && rect_f->x + rect_f->w > x && rect_f->y + rect_f->h > y)
		return 1;
	else
		return 0;
}

int addElement(void* pList, void* data, int size)
{
	void* tar = AllocElement(pList, size); 
	if (NULL == tar)
		return -1; 
	memcpy(tar, data, size); 
	return 0; 
}
// TODO 可能出现一种情况： alloc之后由于某种原因仍赋值为NULL，那么下次再alloc时就可能会重复申请内存 e.g. 计数达到<2^n - 1>时
void* AllocElement(void* pList, int size)
{
	char** addr = (char**)pList; 
	void* ret = NULL; 
	int* empty = (void*)malloc(size); 
	if (NULL == empty)
		return NULL; 
	memset(empty, 0, size); 
	if (NULL == *addr)
	{
		*addr = (char*)malloc(2 * size); 
		memset(*addr, 0, 2 * size); 
		if (NULL == *addr)
			goto end; 
		ret = *addr; 
	}
	else
	{
		char* tmp = *addr; 
		int i = 0; 
		for (; memcmp(tmp, empty, size); i++, tmp += size); 
		if (i & (i + 1))
		{
			ret = tmp; 
		}
		else
		{
			void* tar = malloc(2 * (i + 1)*size); 
			if (NULL == tar)
				goto end; 
			memset(tar, 0, 2 * (i + 1)*size); 
			memcpy(tar, *addr, (i + 1)*size); 
			free(*addr); 
			*addr = (char*)tar; 
			ret = *addr + i*size; 
		}
	}
end:
	if(*empty)
		printf("#### empty = %d\n", *empty); 
	free(empty); 
	return ret; 
}

MSRecti GetRectByRatio(MSRecti src, float ratio)
{
	MSRecti ret = { 0 }; 
	if ((int)(ratio*src.h) <= src.w)
	{
		ret.h = src.h; 
		ret.w = (int)(ratio * src.h); 
		ret.x = src.x + (src.w - ret.w) / 2; 
		ret.y = src.y; 
	}
	else
	{
		ret.w = src.w; 
		ret.h = (int)(src.w / ratio); 
		ret.y = src.y + (src.h - ret.h) / 2; 
		ret.x = src.x; 
	}
	return ret; 
}