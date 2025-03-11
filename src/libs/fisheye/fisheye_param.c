#include"fisheye_param.h"
#include <math.h>
static FisheyeParamInstallModel g_installModel = {
	.param = {0},
	.install_model = Ceilling
}; 
	
void FisheyeParamInstallModel_SetModel(FisheyeParamInstallModel* fisheye_install_model, enum Fisheye_InstallModel install_model)
{
	fisheye_install_model->install_model = install_model; 
	FisheyeParam_Update(&fisheye_install_model->param); 
}

enum Fisheye_InstallModel FisheyeParamInstallModel_GetModel(FisheyeParamInstallModel* fisheye_install_model)
{
	return fisheye_install_model->install_model; 
}


void FisheyeParamO_Init(FisheyeParamO* p)
{
	FisheyeParam_Init(&p->param); 
}

int FisheyeParamO_InitByCJSON(FisheyeParamO* p, const cJSON* json)
{
	return 0; 
}

void FisheyeParamP_Init(FisheyeParamP* p)
{
	FisheyeParam_Init(&p->param); 
	p->offset = 0.0f; 
	p->range = 2*(float)MS_PI; 
	p->install_model = &g_installModel; 
}

INLINE void FisheyeParamP_SetOffset(FisheyeParamP* param, float offset)
{
	param->offset = offset; 
	FisheyeParam_Update(&param->param); 
}

void FisheyeParamP_IncOffset(FisheyeParamP* param, float bias)
{
	param->offset += bias; 
	FisheyeParam_Update(&param->param); 
}

float FisheyeParamP_GetRange(const FisheyeParamP* param)
{
	return param->range; 
}

int FisheyeParamP_InitByCJSON(FisheyeParamP* p, const cJSON* json)
{
	FisheyeParamP_Init(p); 
	cJSON* tmp = cJSON_GetObjectItem(json, "offset"); 
	if (tmp)
	{
		if (cJSON_Number != tmp->type) return -1; 
		p->offset = (float)tmp->valuedouble; 
	}
	tmp = cJSON_GetObjectItem(json, "range"); 
	if (tmp)
	{
		if (cJSON_Number != tmp->type) return -1; 
		p->range = (float)tmp->valuedouble;
	}
	return 0; 
}


int FisheyeParamW_InitByCJSON(FisheyeParamW* p, const cJSON* json)
{
	return 0; 
}
void FisheyeParamW_Init(FisheyeParamW* p)
{
	FisheyeParam_Init(&p->param); 
}


int FisheyeParamR_InitByCJSON(FisheyeParamR* r, const cJSON* json)
{
	FisheyeParamR_Init(r); 
	cJSON* tmp = cJSON_GetObjectItem(json, "alpha"); 
	if (tmp)
	{
		if (cJSON_Number != tmp->type) return -1; 
		r->alpha = AngleToRadian( (float)tmp->valuedouble ); 
	}
	tmp = cJSON_GetObjectItem(json, "beta"); 
	if (tmp)
	{
		if (cJSON_Number != tmp->type) return -1; 
		r->beta = AngleToRadian( (float)tmp->valuedouble ); 
	}
	tmp = cJSON_GetObjectItem(json, "theta"); 
	if (tmp)
	{
		if (cJSON_Number != tmp->type) return -1; 
		r->theta = AngleToRadian( (float)tmp->valuedouble ); 
	}
	return 0; 
}

void FisheyeParamR_Init(FisheyeParamR* r)
{
	FisheyeParam_Init(&r->param); 
	r->alpha = 0.0f; 
	r->beta = 0.0f; 
	r->theta = (float)(MS_PI / 2); 
	r->install_model = &g_installModel; 
}

void FisheyeParamR_SetAlpha(FisheyeParamR* p, float alpha)
{
	p->alpha = alpha; 
	float t = PTZ_ALPHA_MAX - atanf(0.707106781186547524401 * tanf(p->theta / 2)) - AngleToRadian(4.5f); 
	if (p->alpha > t)
		p->alpha = t; 
	if (p->alpha < PTZ_ALPHA_MIN)
		p->alpha = PTZ_ALPHA_MIN; 
	FisheyeParam_Update(&p->param); 
}
void FisheyeParamR_SetBeta(FisheyeParamR* p, float beta)
{
	p->beta = beta; 
	if(p->beta > 2*MS_PI)
		p->beta -= 2*MS_PI; 
	if(p->beta < 0)
		p->beta += 2*MS_PI; 
	FisheyeParam_Update(&p->param); 
}
void FisheyeParamR_SetTheta(FisheyeParamR* p, float theta)
{
	p->theta = theta; 
	if(p->theta < PTZ_THETA_MIN)
		p->theta = PTZ_THETA_MIN; 
	if(p->theta > PTZ_THETA_MAX)
		p->theta = PTZ_THETA_MAX; 
	float t = PTZ_ALPHA_MAX - atanf(0.707106781186547524401 * tanf(p->theta / 2)) - AngleToRadian(4.5f); 
	if (p->alpha > t)
		p->alpha = t; 
	FisheyeParam_Update(&p->param); 
}
void FisheyeParamR_IncAlpha(FisheyeParamR* p, float bias)
{
	switch(p->install_model->install_model)
	{
	case Ceilling:
		bias = -bias; 
		break; 
	case flat:
		break; 
	case wall:
#if 1
		{
			MSPointf xy = PTZ_AB2XY(p->alpha, p->beta); 
			xy.y -= bias; 
			float length = sqrtf(powf(xy.x, 2)+ powf(xy.y, 2)); 
			if(length>1.0f)
			{
				xy.y = sqrtf(1.0f - xy.y*xy.y); 
			}
			MSPointf ab = PTZ_XY2AB(xy.x, xy.y); 
			FisheyeParamR_SetAlpha(p, ab.x); 
			FisheyeParamR_SetBeta(p, ab.y); 
			return; 
		}
#endif
		break; 
	default:
		break; 
	}; 
	p->alpha += bias; 
	float t = PTZ_ALPHA_MAX - atanf(0.707106781186547524401 * tanf(p->theta / 2)) - AngleToRadian(4.5f); 
	if (p->alpha > t)
		p->alpha = t; 
	if (p->alpha < PTZ_ALPHA_MIN)
		p->alpha = PTZ_ALPHA_MIN; 
	FisheyeParam_Update(&p->param); 
}
void FisheyeParamR_IncBeta(FisheyeParamR* p, float bias)
{
	switch(p->install_model->install_model)
	{
	case Ceilling:
		bias = -bias; 
		break; 
	case flat:
		break; 
	case wall:
		{
			MSPointf xy = PTZ_AB2XY(p->alpha, p->beta); 
			xy.x += bias; 
			float length = sqrtf(powf(xy.x, 2)+ powf(xy.y, 2)); 
			if(length>1.0f)
				xy.x = sqrtf(1.0f - xy.y*xy.y);
			MSPointf ab = PTZ_XY2AB(xy.x, xy.y); 
			FisheyeParamR_SetAlpha(p, ab.x); 
			FisheyeParamR_SetBeta(p, ab.y); 
			return; 
		}
		break; 
	default:
		break; 
	}; 
	p->beta += bias; 
	if(p->beta > 2*MS_PI)
		p->beta -= 2*MS_PI; 
	if(p->beta < 0)
		p->beta += 2*MS_PI; 
	FisheyeParam_Update(&p->param); 
}
void FisheyeParamR_IncTheta(FisheyeParamR* p, float bias)
{
	p->theta += bias; 
	if(p->theta < PTZ_THETA_MIN)
		p->theta = PTZ_THETA_MIN; 
	if(p->theta > PTZ_THETA_MAX)
		p->theta = PTZ_THETA_MAX; 
	float t = PTZ_ALPHA_MAX - atanf(0.707106781186547524401 * tanf(p->theta / 2)) - AngleToRadian(4.5f); 
	if (p->alpha > t)
		p->alpha = t; 
	FisheyeParam_Update(&p->param); 
}
int FisheyeParamR_Contains(FisheyeParamR* p, MSPointf point)
{
	float length = sqrtf(powf(point.x, 2)+ powf(point.y, 2)); 
	if(length > 1)
		return 0; 
	float zoom = 1/sinf(AngleToRadian(171.0f/4))/sqrtf(2+2*cosf(p->alpha)); 
	float x = sinf(p->alpha)*cosf(p->beta)*zoom; 
	float y = -sinf(p->alpha)*sinf(p->beta)*zoom; 
	if(sqrtf(powf(point.x-x, 2)+ powf(point.y-y, 2)) < sin(p->theta/4))
		return 1; 

	return 0; 
}
int FisheyeParamR_setParamByXY(FisheyeParamR* p, MSPointf point)
{
	float length = sqrtf(powf(point.x, 2) + powf(point.y, 2)); 
	if(length > 1.0f)
		return -1; 
	if(0 == length)
	{
		FisheyeParamR_SetAlpha(p, 0.0f); 
		FisheyeParamR_SetBeta(p, 0.0f); 
		return 0; 
	}
	
	float alpha = 2 * asinf( length*sinf(AngleToRadian(171.0f/4) ) ); 
	float beta = point.x>0 ? -asinf(point.y/length) : MS_PI + asinf(point.y/length); 
	FisheyeParamR_SetAlpha(p, alpha); 
	FisheyeParamR_SetBeta(p, beta); 
	
	return 0; 
}
int FisheyeParamR_GotoPtz(FisheyeParamR* p, FisheyeParamR* src, float speed)
{
	float offset_alpha = p->alpha - src->alpha, offset_beta = p->beta - src->beta, offset_theta = p->theta - src->theta; 
	if((0.0f != offset_alpha) || (0.0f != offset_beta) || (0.0f != offset_theta))
	{
		if(offset_alpha >0)
		{
			p->alpha += AngleToRadian(speed); 
			if(p->alpha > src->alpha)
			{
				p->alpha = src->alpha; 
			}
		}
		else if(offset_alpha < 0)
		{
			p->alpha += AngleToRadian(-speed); 
			if(p->alpha < src->alpha)
			{
				p->alpha = src->alpha; 
			}
		}
		FisheyeParamR_SetAlpha(p, p->alpha); 
		if(offset_beta > 0)
		{
			p->beta += AngleToRadian(speed); 
			if(p->beta > src->beta)
			{
				p->beta = src->beta; 
			}
		}
		else if(offset_beta < 0)
		{
			p->beta += AngleToRadian(-speed); 
			if(p->beta < src->beta)
			{
				p->beta = src->beta; 
			}
		}
		FisheyeParamR_SetBeta(p, p->beta); 
		if(offset_theta >0)
		{
			FisheyeParamR_IncTheta(p, AngleToRadian(speed)); 
			if(p->theta > src->theta)
			{
				FisheyeParamR_SetTheta(p, p->theta); 
			}
		}
		else if(offset_theta < 0)
		{
			FisheyeParamR_IncTheta(p, AngleToRadian(-speed)); 
			if(p->theta < src->theta)
			{
				FisheyeParamR_SetTheta(p, p->theta); 
			}
		}
		return 1; 
	}
	else
	{
		return 0; 
	}
}

MSPointf PTZ_AB2XY(float alpha, float beta)
{
	float zoom = 1/sinf(AngleToRadian(171.0f/4))/sqrtf(2+2*cosf(alpha)); 
	float x = sinf(alpha)*cosf(beta)*zoom; 
	float y = sinf(alpha)*sinf(beta)*zoom; 
	return (MSPointf){x, y}; 
}

MSPointf PTZ_XY2AB(float x, float y)
{
	MSPointf ret = {.x = 0.0f, .y = 0.0f}; 
	float length = sqrtf(powf(x, 2) + powf(y, 2)); 
	if(length > 1.0f)
	{
		x /= length; 
		y /= length; 
		length = 1.0f; 
	}else if(0 == length)
	{
		return ret; 
	}
	
	ret.x = 2 * asinf( length*sinf(AngleToRadian(171.0f/4) ) ); 
	ret.y = x>0 ? asinf(y/length) : MS_PI - asinf(y/length); 
	return ret; 
}

