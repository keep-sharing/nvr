#ifdef __APPLE__
#include <OpenGLES/ES2/gl.h>
#elif __linux__
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#else
#include <GL/glew.h>
#endif


#include <stddef.h>
#include <memory.h>
#include <string.h>

#include "fisheye_player.h" 
#define MATH_3D_IMPLEMENTATION
#include "math_3d.h"

#define PRINT_GL_ERROR() printf("LINE: %d, FUNC: %s; glGetError: %d\n", __LINE__, __func__, glGetError())

// O视图着色器代码
static const char* o_vsrc =
" \
	attribute vec4 vPosition; \n \
	varying vec3 coord; \n \
	void main(void)\n \
	{\n \
	gl_Position =vPosition; \n \
	gl_Position.z = 0.0; \n\
	coord = mat3(0.5, 0.0, 0.0, \n\
				 0.0,-0.5, 0.0, \n\
				 0.5, 0.5, 1.0) * vPosition.xyz; \n \
	}\n"; 
static const char* o_fsrc =
"\
#extension GL_OES_EGL_image_external : require \n \
	varying highp vec3 coord; \n \
	uniform samplerExternalOES tex_yuv; \n \
	uniform lowp mat3 tsmat33; \n \
	void main(void) \n \
	{ \n \
	highp vec2 coor = (tsmat33 * coord).xy; \n \
	    coor-= vec2(1.0/2176.0, 1.0/2176.0); \n \
	lowp vec4 rgba = texture2D(tex_yuv, coor); \n \
	gl_FragColor = vec4(rgba.rgb, 1.0); \n \
	}"; 

// P视图着色器代码
static const char* p_vsrc =
"\
	attribute vec4 vPosition; \n \
	varying  lowp vec3 coord; \n \
	void main(void)\
	{\
	gl_Position =vPosition; \n \
	gl_Position.z = 0.0; \n\
	coord = mat3(0.5, 0.0, 0.0, \n\
				 0.0,-0.5, 0.0, \n\
				 0.5, 0.5, 1.0) * vPosition.xyz; \n \
	}"; 
	// TODO transform 应该写在顶点着色器上
static const char* p_fsrc =
"\
#extension GL_OES_EGL_image_external : require \n \
	varying highp vec3 coord; \n \
	uniform samplerExternalOES tex_yuv; \n \
	uniform highp mat3 tsmat33; \
	uniform highp mat3 transform; \
	uniform highp float offset; \
	uniform highp float range; \
	void main(void)\
	{\
	highp vec3 coord1 = transform * coord; \
	highp vec2  coor = vec2(coord1.y*cos(coord1.x*range + offset), \
	-coord1.y*sin(coord1.x*range + offset)); \
	coor = 0.5*coor + 0.5; \
	coor = (tsmat33*vec3(coor,1)).xy; \
	lowp vec4 rgba = texture2D(tex_yuv, coor); \n \
	gl_FragColor = vec4(rgba.rgb, 1.0); \n \
	}"; 

// W视图着色器代码
static const char* w_vsrc =
" \
	attribute vec4 vPosition; \n \
	varying vec3 coord; \n \
	void main(void)\n \
	{\n \
	gl_Position =vPosition; \n \
	gl_Position.z = 0.0; \n\
	coord = mat3(0.5, 0.0, 0.0, \n\
				 0.0,-0.5, 0.0, \n\
				 0.5, 0.5, 1.0) * vPosition.xyz; \n \
	}\n"; 
static const char* w_fsrc =
"\
#extension GL_OES_EGL_image_external : require \n \
	varying highp vec3 coord; \n \
	uniform samplerExternalOES tex_yuv; \n \
	uniform lowp mat3 tsmat33; \n \
	void main(void) \n \
	{ \n \
	highp vec2 coor = vec2(coord.x*1.8 - 0.9, coord.y*1.64 - 0.82); \
	coor = vec2(coor.x * sqrt(1.0 - pow(coor.y, 2.0) ), coor.y); \
	coor = 0.5 * coor + 0.5; \
	coor = (tsmat33 * vec3(coor, 1.0)).xy; \n \
	lowp vec4 rgba = texture2D(tex_yuv, coor); \n \
	gl_FragColor = vec4(rgba.rgb, 1.0); \n \
	}"; 

// R视图着色器代码
static const char* r_vsrc =
"\
	attribute vec4 vPosition; \n \
	varying  highp vec3 coord; \n \
	void main(void)\
	{\
	gl_Position =vPosition; \n \
	gl_Position.z = 0.0; \n\
	coord = mat3(0.5, 0.0, 0.0, \n\
				 0.0,-0.5, 0.0, \n\
				 0.5, 0.5, 1.0) * vPosition.xyz; \n \
	}"; 
static const char* r_fsrc =
"\
#extension GL_OES_EGL_image_external : require \n \
	varying highp vec3 coord; \n \
	uniform samplerExternalOES tex_yuv; \n \
	uniform lowp mat3 tsmat33; \
	\
	const lowp float k_gamma = 1.364; \n\
	\n\
	uniform highp mat4 transform; \n\
	\n\
	void main(void)\
	{\
	highp vec4 pos0_4 = vec4(coord,1); \n\
	highp vec4 sphere1 = transform * pos0_4; \
	highp float rho1 = length(vec3(0,0,1) - normalize(sphere1.xyz))/k_gamma; \
	\
	highp vec2 textureOut2 = rho1*normalize(sphere1.xy)/2.0 + 0.5; \
	textureOut2 = (tsmat33*vec3(textureOut2,1)).xy; \
	lowp vec4 rgba = texture2D(tex_yuv, textureOut2); \n \
	gl_FragColor = vec4(rgba.rgb, 1.0); \n \
	}"; 

// 线段着色器代码
static const char* l_vsrc =
" \
	attribute vec4 vPosition; \n \
	void main(void)\n \
	{\n \
		gl_Position =vPosition; \n \
		gl_Position.z = -1.0; \n\
	}\n"; 
static const char* l_fsrc =
"\
	uniform lowp vec4 color; \n \
	void main(void) \n \
	{ \n \
		gl_FragColor =color; \n \
	}"; 

// rgb 转 yuv 着色器代码
static const char* chg_vsrc =
" \
	attribute vec4 vPosition; \n \
	varying vec3 coord; \n \
	void main(void)\n \
	{\n \
	gl_Position =vPosition; \n \
	gl_Position.z = 0.0; \n\
	coord = mat3(0.5, 0.0, 0.0, \n\
				 0.0,-0.5, 0.0, \n\
				 0.5, 0.5, 1.0) * vPosition.xyz; \n \
	}\n"; 
#if 1
static const char* chg_fsrc =
"\
        varying highp vec3 coord; \n \
        uniform sampler2D tex_rgb; \n \
        uniform highp float offset; \
        lowp vec3 RGB_2_YUV(lowp vec4 color) \n \
        { \n \
                lowp vec3 yuv = mat3(  \n \
                        0.299002, -0.147139,  0.615002, \n \
                        0.586999, -0.288862, -0.514988, \n \
                        0.113999,  0.436000, -0.100014 ) * color.rgb; \n \
                return yuv + vec3(0, 0.5, 0.5); \n \
        } \n \
        void main(void) \n \
        { \n \
                highp vec2 coor = coord.xy; \n \
                if(coord.y > 1.0/3.0) \n \
                 { \n \
                        coor.t *= 1.5; \n \
                        coor.t -= 0.5; \n \
                        coor.s -= 1.5 * offset;  \n \
                        lowp vec3 yuv0 =  RGB_2_YUV(texture2D(tex_rgb, coor)); \n \
                        coor.s += offset;  \n \
                        lowp vec3 yuv1 =  RGB_2_YUV(texture2D(tex_rgb, coor)); \n \
                        coor.s += offset;  \n \
                        lowp vec3 yuv2 =  RGB_2_YUV(texture2D(tex_rgb, coor)); \n \
                        coor.s += offset;  \n \
                        lowp vec3 yuv3 =  RGB_2_YUV(texture2D(tex_rgb, coor)); \n \
                        gl_FragColor = vec4(yuv0.x, yuv1.x, yuv2.x, yuv3.x); \n \
                } \n \
                else \n \
                 { \n \
                        coor.t *= 3.0; \n \
                        coor.s -= offset;  \n \
                        lowp vec3 yuv0 =  RGB_2_YUV(texture2D(tex_rgb, coor)); \n \
                        coor.s += 2.0 * offset;  \n \
                        lowp vec3 yuv1 =  RGB_2_YUV(texture2D(tex_rgb, coor)); \n \
                        gl_FragColor = vec4(yuv0.z, yuv0.y, yuv1.z, yuv1.y); \n \
                } \n \
        }"; 
#else
static const char* chg_fsrc =
"\
	varying highp vec3 coord; \n \
	uniform sampler2D tex_rgb; \n \
	uniform highp float offset; \
	void main(void) \n \
	{ \n \
		highp vec2 coor = coord.xy; \n \
		lowp vec3 rgb = vec3( \n \
			texture2D(tex_rgb, coor).r + offset, \n \
			texture2D(tex_rgb, coor).g + offset, \n \
			texture2D(tex_rgb, coor).b + offset \n \
		); \n \
		gl_FragColor = vec4(rgb, 1.0); \n \
	}"; 
#endif



static void printShaderInfoLog(GLuint shaderObject)
{
	GLint logLen = 0; 
	GLint writtenLen = 0; 
	GLchar* info_log; 

	glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &logLen); 

	if (logLen > 1)
	{
		info_log = (GLchar*)malloc(logLen); 
		glGetShaderInfoLog(shaderObject, logLen, &writtenLen, info_log); 
		DEBUG_PRINTF("%s\n", info_log); 
		free(info_log); 
	}

}
static void printProgramInfoLog(GLuint programObject)
{
	GLint logLen = 0; 
	GLint writtenLen = 0; 
	GLchar* info_log; 

	glGetShaderiv(programObject, GL_INFO_LOG_LENGTH, &logLen); 

	if (logLen > 1)
	{
		info_log = (GLchar*)malloc(logLen); 
		glGetProgramInfoLog(programObject, logLen, &writtenLen, info_log); 
		DEBUG_PRINTF("%s\n", info_log); 
		free(info_log); 
	}
}

static int init_shader(GLuint* pObj, const char* vsrc, const char*fsrc, GLint vertex_location)
{
	GLint ret_param = 0; 
	int ret = 0; 

	// 创建 顶点着色器、 像素着色器、 着色器程序
	GLuint vObj = glCreateShader(GL_VERTEX_SHADER); 
	GLuint fObj = glCreateShader(GL_FRAGMENT_SHADER); 
	*pObj = glCreateProgram(); 

	//为 着色器 添加原始代码
	glShaderSource(vObj, 1, &vsrc, NULL); 
	glShaderSource(fObj, 1, &fsrc, NULL); 

	//编译着色器
	glCompileShader(vObj); 
	glGetShaderiv(vObj, GL_COMPILE_STATUS, &ret_param); 
	if (GL_FALSE == ret_param)
	{
		DEBUG_PRINTF("vertex shader compile error\n"); 
		printShaderInfoLog(vObj); 
		ret = -1; 
		goto end; 
	}
	glCompileShader(fObj); 
	glGetShaderiv(fObj, GL_COMPILE_STATUS, &ret_param); 
	if (GL_FALSE == ret_param)
	{
		DEBUG_PRINTF("flag shader compile error\n"); 
		printShaderInfoLog(fObj); 
		ret = -1; 
		goto end; 
	}

	//将编译好的着色器添加进 程序中
	glAttachShader(*pObj, vObj); 
	glAttachShader(*pObj, fObj); 

	glBindAttribLocation(*pObj, vertex_location, "vPosition"); 

	glLinkProgram(*pObj); 

	glGetProgramiv(*pObj, GL_LINK_STATUS, &ret_param); 
	if (GL_FALSE == ret_param)
	{
		DEBUG_PRINTF("shader link error\n"); 
		printProgramInfoLog(*pObj); 
		ret = -1; 
		goto end; 
	}
end:
	glDeleteShader(vObj); 
	glDeleteShader(fObj); 
	return ret; 
}
static int init_texture(GLuint* TBO_yuv)
{
	glGenTextures(1, TBO_yuv); 
#define F(id) {glBindTexture(GL_TEXTURE_EXTERNAL_OES, id); \
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES,GL_TEXTURE_MAG_FILTER,GL_LINEAR); \
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES,GL_TEXTURE_MIN_FILTER,GL_LINEAR); \
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); \
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); \
	}

	F(TBO_yuv[0]); 

#undef  F
	return 0; 
}
#define VERTEXT_INDEX 0
#define SIZE 1.0f
static GLfloat vertex[] = {
    -SIZE, -SIZE, 1.0f, 
	 SIZE, -SIZE, 1.0f, 
	-SIZE,  SIZE, 1.0f, 
	 SIZE,  SIZE, 1.0f
}; 
#define VERTEXT_BORDER_INDEX 1
static GLfloat vertex_border[] = {
    -SIZE*0.999f, -SIZE*0.999f, 1.0f, 
	 SIZE*0.999f, -SIZE*0.999f, 1.0f, 
	 SIZE*0.999f,  SIZE*0.999f, 1.0f, 
	-SIZE*0.999f,  SIZE*0.999f, 1.0f
}; 
#undef SIZE 

static int init_vertex(GLuint* VBO, GLuint v_index, void* data, GLint size) 
{
	glGenBuffers(1, VBO); 
	glBindBuffer(GL_ARRAY_BUFFER, *VBO); 
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW); 
	glVertexAttribPointer(v_index, 3, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(v_index); 
	glBindBuffer(GL_ARRAY_BUFFER, 0); 
	return 0; 
}
static const float wire_points[80 + 1][3] = {
	{-1.0f, 1.0f, 0.0f}, {-0.9f, 1.0f, 0.0f}, {-0.8f, 1.0f, 0.0f}, {-0.7f, 1.0f, 0.0f}, {-0.6f, 1.0f, 0.0f}, {-0.5f, 1.0f, 0.0f}, {-0.4f, 1.0f, 0.0f}, {-0.3f, 1.0f, 0.0f}, {-0.2f, 1.0f, 0.0f}, {-0.1f, 1.0f, 0.0f}, 
	{ 0.0f, 1.0f, 0.0f}, { 0.1f, 1.0f, 0.0f}, { 0.2f, 1.0f, 0.0f}, { 0.3f, 1.0f, 0.0f}, { 0.4f, 1.0f, 0.0f}, { 0.5f, 1.0f, 0.0f}, { 0.6f, 1.0f, 0.0f}, { 0.7f, 1.0f, 0.0f}, { 0.8f, 1.0f, 0.0f}, { 0.9f, 1.0f, 0.0f}, 
	{ 1.0f, 1.0f, 0.0f}, { 1.0f, 0.9f, 0.0f}, { 1.0f, 0.8f, 0.0f}, { 1.0f, 0.7f, 0.0f}, { 1.0f, 0.6f, 0.0f}, { 1.0f, 0.5f, 0.0f}, { 1.0f, 0.4f, 0.0f}, { 1.0f, 0.3f, 0.0f}, { 1.0f, 0.2f, 0.0f}, { 1.0f, 0.1f, 0.0f}, 
	{ 1.0f, 0.0f, 0.0f}, { 1.0f,-0.1f, 0.0f}, { 1.0f,-0.2f, 0.0f}, { 1.0f,-0.3f, 0.0f}, { 1.0f,-0.4f, 0.0f}, { 1.0f,-0.5f, 0.0f}, { 1.0f,-0.6f, 0.0f}, { 1.0f,-0.7f, 0.0f}, { 1.0f,-0.8f, 0.0f}, { 1.0f,-0.9f, 0.0f}, 
	{ 1.0f,-1.0f, 0.0f}, { 0.9f,-1.0f, 0.0f}, { 0.8f,-1.0f, 0.0f}, { 0.7f,-1.0f, 0.0f}, { 0.6f,-1.0f, 0.0f}, { 0.5f,-1.0f, 0.0f}, { 0.4f,-1.0f, 0.0f}, { 0.3f,-1.0f, 0.0f}, { 0.2f,-1.0f, 0.0f}, { 0.1f,-1.0f, 0.0f}, 
	{ 0.0f,-1.0f, 0.0f}, {-0.1f,-1.0f, 0.0f}, {-0.2f,-1.0f, 0.0f}, {-0.2f,-1.0f, 0.0f}, {-0.4f,-1.0f, 0.0f}, {-0.5f,-1.0f, 0.0f}, {-0.6f,-1.0f, 0.0f}, {-0.7f,-1.0f, 0.0f}, {-0.8f,-1.0f, 0.0f}, {-0.9f,-1.0f, 0.0f}, 
	{-1.0f,-1.0f, 0.0f}, {-1.0f,-0.9f, 0.0f}, {-1.0f,-0.8f, 0.0f}, {-1.0f,-0.7f, 0.0f}, {-1.0f,-0.6f, 0.0f}, {-1.0f,-0.5f, 0.0f}, {-1.0f,-0.4f, 0.0f}, {-1.0f,-0.3f, 0.0f}, {-1.0f,-0.2f, 0.0f}, {-1.0f,-0.1f, 0.0f}, 
	{-1.0f, 0.0f, 0.0f}, {-1.0f, 0.1f, 0.0f}, {-1.0f, 0.2f, 0.0f}, {-1.0f, 0.3f, 0.0f}, {-1.0f, 0.4f, 0.0f}, {-1.0f, 0.5f, 0.0f}, {-1.0f, 0.6f, 0.0f}, {-1.0f, 0.7f, 0.0f}, {-1.0f, 0.8f, 0.0f}, {-1.0f, 0.9f, 0.0f}, 
	{-1.0f, 1.0f, 0.0f}
}; 

struct LineShader; 
typedef struct{
	float output_points[80 + 1][3]; 
	mat4_t TransformMatrixs; // 最终的转换矩阵
	GLuint VBO_Line; 
	GLfloat color_rgba[4]; 
	struct LineShader* line_shader; 
}Unit_WireFrame; 
static INLINE int Unit_WireFrame_Init(Unit_WireFrame* u_wf, struct LineShader* line_shader, GLuint color)
{
	memset(u_wf, 0, sizeof(Unit_WireFrame)); 
	u_wf->line_shader = line_shader; 
	init_vertex(&u_wf->VBO_Line, VERTEXT_BORDER_INDEX, (void*)wire_points, sizeof(wire_points)); 
	
	u_wf->color_rgba[0] = ((color&0xff000000)>>24)/255.0f; 
	u_wf->color_rgba[1] = ((color&0x00ff0000)>>16)/255.0f; 
	u_wf->color_rgba[2] = ((color&0x0000ff00)>> 8)/255.0f; 
	u_wf->color_rgba[3] = ((color&0x000000ff)>> 0)/255.0f; 
	u_wf->TransformMatrixs = m4_identity(); 
	return 0; 
	
}
static Unit_WireFrame* Unit_WireFrame_New(struct LineShader* line_shader, GLuint color)
{
	Unit_WireFrame* ret = (Unit_WireFrame*)malloc(sizeof(Unit_WireFrame)); 
	if (NULL == ret)
		goto end; 
	Unit_WireFrame_Init(ret, line_shader, color); 
end:
	return ret; 
}
static INLINE void Unit_WireFrame_Uninit(Unit_WireFrame* u_wf)
{
	glDeleteBuffers(1, &u_wf->VBO_Line); 
}
static void Unit_WireFrame_Delete(Unit_WireFrame* u_wf)
{
	Unit_WireFrame_Uninit(u_wf); 
	free(u_wf); 
}


typedef struct{
	GLuint VBO_Line; 
	GLfloat color_rgba[4]; 
	struct LineShader* line_shader; 
	const int* absWidth; 
}Unit_Border; 
static INLINE int Unit_Border_Init(Unit_Border* ub, struct LineShader* line_shader, GLuint color, const int* abs_width)
{
	memset(ub, 0, sizeof(Unit_Border)); 
	ub->line_shader = line_shader; 
	init_vertex(&ub->VBO_Line, VERTEXT_BORDER_INDEX, vertex_border, sizeof(vertex_border)); 
	
	ub->color_rgba[0] = ((color&0xff000000)>>24)/255.0f; 
	ub->color_rgba[1] = ((color&0x00ff0000)>>16)/255.0f; 
	ub->color_rgba[2] = ((color&0x0000ff00)>> 8)/255.0f; 
	ub->color_rgba[3] = ((color&0x000000ff)>> 0)/255.0f; 
	ub->absWidth = abs_width; 
	return 0; 
}
static Unit_Border* Unit_Border_New(struct LineShader* line_shader, GLuint color, const int* abs_width)
{
	Unit_Border* ret = (Unit_Border*)malloc(sizeof(Unit_Border)); 
	if (NULL == ret)
		goto end; 
	Unit_Border_Init(ret, line_shader, color, abs_width);
end:
	return ret; 
}
static void Unit_Border_Uninit(Unit_Border* ub)
{
	glDeleteBuffers(1, &ub->VBO_Line); 
}
static void Unit_Border_Delete(Unit_Border* ub)
{
	Unit_Border_Uninit(ub); 
	free(ub); 
}

// DrawLines
// TODO 线段的宽度还没设定
//      待验证
typedef struct LineShader{
	GLuint PObj; 
	GLint color; 
}LineShader; 
static INLINE int LineShader_Init(LineShader* line_shader, const char* vsrc, const char*fsrc)
{
	init_shader(&line_shader->PObj, vsrc, fsrc, VERTEXT_BORDER_INDEX); 
	line_shader->color = glGetUniformLocation(line_shader->PObj, "color"); 
	return 0; 
}
static INLINE void LineShader_Uninit(LineShader* line_shader)
{
	glDeleteProgram(line_shader->PObj); 
	return; 
}
static INLINE void LineShader_UseShader(LineShader* line_shader)
{
	glUseProgram(line_shader->PObj); 
}
static INLINE int LineShader_SetParam(LineShader* line_shader, float red, float green, float blue, float alpha)
{
	glUniform4f(line_shader->color, red, green, blue, alpha); 
	return 0; 
}

typedef struct ChgShader {
	GLuint PObj; 
	GLint tex_rgb; 
	GLint offset; 
}ChgShader; 
static INLINE int ChgShader_Init(ChgShader* chg_shader, const char* vsrc, const char*fsrc)
{
	init_shader(&chg_shader->PObj, vsrc, fsrc, VERTEXT_INDEX); 
	chg_shader->tex_rgb = glGetUniformLocation(chg_shader->PObj, "tex_rgb"); 
	chg_shader->offset = glGetUniformLocation(chg_shader->PObj, "offset"); 
	return 0; 
}
static INLINE void ChgShader_UseShader(ChgShader* chg_shader)
{
	glUseProgram(chg_shader->PObj); 
}
static INLINE int ChgShader_SetParam(ChgShader* chg_shader, int value_rgb, float offset)
{
	glUniform1i(chg_shader->tex_rgb, value_rgb); 
	glUniform1f(chg_shader->offset, offset); 
	return 0; 
}

typedef struct {
	GLuint PObj; 
	GLint tex_yuv; 
	GLint tsmat33; 
}YUVShader; 
static INLINE int YUVShader_Init(YUVShader* yuv, const char* vsrc, const char*fsrc)
{
	init_shader(&(yuv->PObj), vsrc, fsrc, VERTEXT_INDEX); 
	yuv->tex_yuv = glGetUniformLocation(yuv->PObj, "tex_yuv"); 
	yuv->tsmat33 = glGetUniformLocation(yuv->PObj, "tsmat33"); 
	return 0; 
}
static INLINE void YUVShader_Uninit(YUVShader* yuv)
{
	glDeleteProgram(yuv->PObj); 
	return; 
}
// TODO  未完
static INLINE void YUVShader_UseShader(YUVShader* yuv)
{
	glUseProgram(yuv->PObj); 
}
static INLINE int YUVShader_SetParam(YUVShader* yuv, int value_yuv, GLfloat* TsMat)
{
	glUniform1i(yuv->tex_yuv, value_yuv); 
	glUniformMatrix3fv(yuv->tsmat33, 1, GL_FALSE, TsMat); 
	return 0; 
}
typedef struct {
	YUVShader yuv; 
} OShader; 
static INLINE int OShader_Init(OShader* osh)
{
	YUVShader_Init(&(osh->yuv), o_vsrc, o_fsrc); 
	return 0; 
}
static INLINE int OShader_Uninit(OShader* osh)
{
	YUVShader_Uninit(&(osh->yuv)); 
	return 0; 
}
static INLINE void OShader_UpdateYuvTex(OShader* osh, int value_yuv, GLfloat* _TsMat)
{
	//GLfloat TsMat[9] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f }; 
	YUVShader_SetParam(&(osh->yuv), value_yuv, _TsMat); 
}
static INLINE void OShader_UseShader(OShader* osh)
{
	YUVShader_UseShader(&(osh->yuv)); 
}
static INLINE int OShader_SetParam(OShader*psh, FisheyeParamO* param)
{
	return 0; 
}

typedef struct {
	YUVShader yuv; 
	GLint transform; 
	GLint offset; 
	GLint range; 
} PShader; 
static INLINE int PShader_Init(PShader* psh)
{
	YUVShader_Init(&(psh->yuv), p_vsrc, p_fsrc); 
	psh->transform = glGetUniformLocation(psh->yuv.PObj, "transform"); 
	psh->offset = glGetUniformLocation(psh->yuv.PObj, "offset"); 
	psh->range = glGetUniformLocation(psh->yuv.PObj, "range"); 
	return 0; 
}
static INLINE int PShader_Uninit(PShader* psh)
{
	YUVShader_Uninit(&(psh->yuv)); 
	return 0; 
}
static INLINE void PShader_UpdateYuvTex(PShader* psh, int value_yuv, GLfloat* TsMat)
{
	YUVShader_SetParam(&(psh->yuv), value_yuv, TsMat); 
}
static INLINE void PShader_UseShader(PShader* psh)
{
	YUVShader_UseShader(&(psh->yuv)); 
}

static INLINE int PShader_SetParam(PShader*psh, FisheyeParamP* param)
{
	GLfloat TsMat[9] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f }; 
	if(Ceilling == param->install_model->install_model)
	{
		TsMat[0] = -1.0f; TsMat[6] = 1.0f; 
		TsMat[4] = -1.0f; TsMat[7] = 1.0f; 
	}
	glUniformMatrix3fv(psh->transform, 1, GL_FALSE, TsMat); 
	glUniform1f(psh->offset, param->offset); 
	glUniform1f(psh->range, param->range); 
	return 0; 
}

typedef struct {
	YUVShader yuv; 
} WShader; 
static INLINE int WShader_Init(WShader* wsh)
{
	// TODO add w_vsrc w_fsrc
	YUVShader_Init(&(wsh->yuv), w_vsrc, w_fsrc); 
	return 0; 
}
static INLINE int WShader_Uninit(WShader* wsh)
{
	YUVShader_Uninit(&(wsh->yuv)); 
	return 0; 
}
static INLINE void WShader_UpdateYuvTex(WShader* wsh, int value_yuv, GLfloat* TsMat)
{
	YUVShader_SetParam(&(wsh->yuv), value_yuv, TsMat); 
}
static INLINE void WShader_UseShader(WShader* wsh)
{
	YUVShader_UseShader(&(wsh->yuv)); 
}
static INLINE int WShader_SetParam(WShader*wsh, FisheyeParamW* param)
{
	return 0; 
}

typedef struct {
	YUVShader yuv; 
	GLint transform; 
}RShader; 
static INLINE int RShader_Init(RShader* rsh)
{
	YUVShader_Init(&(rsh->yuv), r_vsrc, r_fsrc); 
	rsh->transform = glGetUniformLocation(rsh->yuv.PObj, "transform"); 
	return 0; 
}
static INLINE int RShader_Uninit(RShader* rsh)
{
	YUVShader_Uninit(&(rsh->yuv)); 
	return 0; 
}
static INLINE void RShader_UpdateYuvTex(RShader* rsh, int value_yuv, GLfloat* TsMat)
{
	YUVShader_SetParam(&(rsh->yuv), value_yuv, TsMat); 
}
static INLINE void RShader_UseShader(RShader* rsh)
{
	YUVShader_UseShader(&(rsh->yuv)); 
}
static INLINE int RShader_SetParam(RShader*rsh, mat4_t* tramform)//FisheyeParamR*param)
{
	glUniformMatrix4fv(rsh->transform, 1, GL_FALSE, (const GLfloat *)tramform); 
	return 0; 
}

typedef struct {
	OShader oshader; 
	PShader pshader; 
	WShader wshader; 
	RShader rshader; 
	LineShader lshader; 
	ChgShader chgshader; 
	GLuint TBO_yuv[1]; 
	GLuint VBO; 
	GLint* win_height; 
	MSRecti* renderTarget; 
	GLfloat TsMat[9]; 
	GLfloat TsMat_UnitO[9]; 
	GLuint transform_fbo; 
	GLuint transform_tbo; 
	EGLContex EGL;
	void *arg;
}Player_Context; 

static INLINE int Player_Context_Init(Player_Context* player_ctx, int InputWidth, int InputHeight, 
                                        MSRecti* renderTarget, MSRectf* correct_region, 
                                        MSRectf* fisheye_region, void* arg)
{
#ifndef __APPLE__
#ifdef __glew_h__
#ifdef __GLEW_H__
	glewInit(); 
#endif
#endif
#endif // !__APPLE__

    int ret = EGL_Init(&(player_ctx->EGL), InputWidth, InputHeight, renderTarget->w, renderTarget->h);
    if (ret < 0) {
        printf("### EGL_Init failed. ret:%d\n", ret);
        return -1;
    }
    
	glDisable(GL_BLEND); 
	OShader_Init(&(player_ctx->oshader)); 
	PShader_Init(&(player_ctx->pshader)); 
	WShader_Init(&(player_ctx->wshader)); 
	RShader_Init(&(player_ctx->rshader)); 
	LineShader_Init(&(player_ctx->lshader), l_vsrc, l_fsrc); 
	ChgShader_Init(&(player_ctx->chgshader), chg_vsrc, chg_fsrc); 
	init_texture(player_ctx->TBO_yuv); 
	player_ctx->transform_fbo = -1; 
	player_ctx->transform_tbo = -1; 
	// init VBO
	init_vertex(&player_ctx->VBO, VERTEXT_INDEX, vertex, sizeof(vertex)); 
	player_ctx->TsMat[0] = correct_region->w, player_ctx->TsMat[1] = 0.0f, player_ctx->TsMat[2] = 0.0f; 
	player_ctx->TsMat[3] = 0.0f; player_ctx->TsMat[4] = correct_region->h, player_ctx->TsMat[5] = 0.0f; 
	player_ctx->TsMat[6] = correct_region->x; player_ctx->TsMat[7] = correct_region->y, player_ctx->TsMat[8] = 1.0f; 
	player_ctx->TsMat_UnitO[0] = fisheye_region->w, player_ctx->TsMat_UnitO[1] = 0.0f, player_ctx->TsMat_UnitO[2] = 0.0f; 
	player_ctx->TsMat_UnitO[3] = 0.0f; player_ctx->TsMat_UnitO[4] = fisheye_region->h, player_ctx->TsMat_UnitO[5] = 0.0f; 
	player_ctx->TsMat_UnitO[6] = fisheye_region->x; player_ctx->TsMat_UnitO[7] = fisheye_region->y, player_ctx->TsMat_UnitO[8] = 1.0f; 
	player_ctx->renderTarget = renderTarget; 
	player_ctx->win_height = (GLint*)&renderTarget->h; 
	player_ctx->arg = arg;
	return 0; 
}
static INLINE int Player_Context_Uninit(Player_Context* player_ctx)
{
	OShader_Uninit(&(player_ctx->oshader)); 
	PShader_Uninit(&(player_ctx->pshader)); 
	WShader_Uninit(&(player_ctx->wshader)); 
	RShader_Uninit(&(player_ctx->rshader)); 
	LineShader_Uninit(&(player_ctx->lshader)); 
	// uninit TBO、VBO
	glDeleteBuffers(1, &player_ctx->VBO); 
	glDeleteTextures(3, player_ctx->TBO_yuv);
	
    EGL_Uninit(&(player_ctx->EGL));
	
	return 0; 
}

typedef struct {
	Player_Context* player_ctx; 
	MSRecti* render_target; 
}Mode_Context; 

static INLINE int Mode_Context_Init(Mode_Context* mode_ctx, Player_Context* player_ctx, MSRecti* rt)
{
	mode_ctx->player_ctx = player_ctx; 
	mode_ctx->render_target = rt; 
	return 0; 
}
static INLINE int Mode_Context_Uninit(Mode_Context* mode_ctx)
{
	mode_ctx->player_ctx = NULL; 
	return 0; 
}


typedef struct {
	GLint* win_height; 
	MSRecti view_port; 
	GLfloat* TsMat; 
	LineShader* lshader; 
}Unit_Context; 

static INLINE int Unit_Context_Init(Unit_Context* unit_ctx, Mode_Context* mode_ctx, MSRectf* relaRect) 
{
	MSRecti* rT = mode_ctx->player_ctx->renderTarget; 
	unit_ctx->win_height = mode_ctx->player_ctx->win_height; 
	unit_ctx->view_port = (MSRecti) {
		rT->x + (int)(0.5f + rT->w * relaRect->x), 
			*unit_ctx->win_height - rT->y - (int)(0.5f + rT->h * (relaRect->y + relaRect->h)), 
			(int)(1.0f + rT->w * relaRect->w), 
			(int)(1.0f + rT->h * relaRect->h)
	}; 
	unit_ctx->lshader = &mode_ctx->player_ctx->lshader; 
	unit_ctx->TsMat = mode_ctx->player_ctx->TsMat; 
	return 0; 
}
static INLINE int Unit_Context_Uninit(Unit_Context* unit_ctx)
{
	unit_ctx->view_port = (MSRecti){ 0 }; 
	return 0; 
}
static int Unit_Context_OnReshape(Unit_Context* unit_ctx, MSRecti* render_target, MSRectf* relaRect)
{
	MSRecti* rT = render_target; 
	unit_ctx->view_port = (MSRecti) {
			rT->x + (int)(0.5f + rT->w * relaRect->x), 
			*unit_ctx->win_height - rT->y - (int)(0.5f + rT->h * (relaRect->y + relaRect->h)), 
			(int)(1.0f + rT->w * relaRect->w), 
			(int)(1.0f + rT->h * relaRect->h)
	}; 
	return 0; 
}

typedef struct {
	Unit_Context unit_ctx; 
	OShader* osh; 
	GLfloat* TsMat_UnitO; 
}UnitO_Context; 

static INLINE int UnitO_Context_Init(UnitO_Context* unit_o_ctx, Mode_Context* mode_ctx, MSRectf* relaRect)
{
	unit_o_ctx->osh = &mode_ctx->player_ctx->oshader; 
	unit_o_ctx->TsMat_UnitO = mode_ctx->player_ctx->TsMat_UnitO; 
	return Unit_Context_Init(&unit_o_ctx->unit_ctx, mode_ctx, relaRect); 
}
static INLINE int UnitO_Context_Uninit(UnitO_Context* unit_o_ctx)
{
	unit_o_ctx->osh = NULL; 
	return Unit_Context_Uninit(&unit_o_ctx->unit_ctx); 
}


typedef struct {
	Unit_Context unit_ctx; 
	PShader* psh; 
}UnitP_Context; 

static INLINE int UnitP_Context_Init(UnitP_Context* unit_p_ctx, Mode_Context* mode_ctx, MSRectf* relaRect)
{
	unit_p_ctx->psh = &mode_ctx->player_ctx->pshader; 
	return Unit_Context_Init(&unit_p_ctx->unit_ctx, mode_ctx, relaRect); 
}
static INLINE int UnitP_Context_Uninit(UnitP_Context* unit_p_ctx)
{
	unit_p_ctx->psh = NULL; 
	return Unit_Context_Uninit(&unit_p_ctx->unit_ctx); 
}


typedef struct {
	Unit_Context unit_ctx; 
	WShader* wsh; 
}UnitW_Context; 

static INLINE int UnitW_Context_Init(UnitW_Context* unit_w_ctx, Mode_Context* mode_ctx, MSRectf* relaRect)
{
	unit_w_ctx->wsh = &mode_ctx->player_ctx->wshader; 
	return Unit_Context_Init(&unit_w_ctx->unit_ctx, mode_ctx, relaRect); 
}
static INLINE int UnitW_Context_Uninit(UnitW_Context* unit_w_ctx)
{
	unit_w_ctx->wsh = NULL; 
	return Unit_Context_Uninit(&unit_w_ctx->unit_ctx); 
}


typedef struct {
	Unit_Context unit_ctx; 
	mat4_t TransformMatrixs; // 最终的转换矩阵
	RShader* rsh; 
}UnitR_Context; 

static INLINE int UnitR_Context_Init(UnitR_Context* unit_r_ctx, Mode_Context* mode_ctx, MSRectf* relaRect)
{
	unit_r_ctx->rsh = &mode_ctx->player_ctx->rshader; 
	unit_r_ctx->TransformMatrixs = m4_identity(); 
	return Unit_Context_Init(&unit_r_ctx->unit_ctx, mode_ctx, relaRect); 
}
static INLINE int UnitR_Context_Uninit(UnitR_Context* unit_r_ctx)
{
	unit_r_ctx->rsh = NULL; 
	return Unit_Context_Uninit(&unit_r_ctx->unit_ctx); 
}
static INLINE int UnitR_Context_Update(UnitR_Context* unit_r_ctx, FisheyeParamR* param, float aspect_r)
{
	GLfloat mode_angle = 0.0f; 
	switch(param->install_model->install_model)
	{
	case flat: 
		mode_angle = -param->beta + MS_PI/2.0f; 
		break; 
	case Ceilling: 
		mode_angle = -param->beta - MS_PI/2.0f; 
		break; 
	case wall: 
		mode_angle = -MS_PI; 
		break; 
	default: 
		mode_angle = -param->beta + MS_PI/2.0f; 
	}
	mat4_t mat_0 = m4_identity(); mat_0.m22 = 0.0f; mat_0.m00 = -1.0f; 
	mat4_t mat_1 = m4_mul(m4_translation((vec3_t){0.5f, -0.5f, 1.0f/tanf(param->theta/2.0f)}), mat_0); 
	mat4_t mat_2 = m4_mul(m4_scaling((vec3_t){aspect_r*1.41421f, -1.41421f, 1.0f}), mat_1); 
	mat4_t mat_3 = m4_mul(m4_rotation_z(mode_angle), mat_2); 
	mat4_t mat_rota = m4_rotation(param->alpha, (vec3_t) { sinf(param->beta), cosf(param->beta), 0 }); 
	unit_r_ctx->TransformMatrixs = m4_mul(mat_rota, mat_3); 
	return 0; 
}

static void UnitBorder_OnDraw(UnitBorder* ub)
{
	if(0 == *ub->isActive)
		return; 
	Unit_Border* u_b = (Unit_Border*)ub->priv_data; 
	LineShader_UseShader(u_b->line_shader); 
	glBindBuffer(GL_ARRAY_BUFFER, u_b->VBO_Line); 
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_border), vertex_border, GL_STATIC_DRAW ); 
	glVertexAttribPointer(VERTEXT_BORDER_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0); 
	LineShader_SetParam(u_b->line_shader, u_b->color_rgba[0], u_b->color_rgba[1], u_b->color_rgba[2], u_b->color_rgba[3]); 
	glLineWidth((GLfloat)(*u_b->absWidth * (ub->width) >> 9) + 1);
	glDrawArrays(GL_LINE_LOOP, 0, sizeof(vertex_border)/(sizeof(vertex_border[0])*3)); 
	glBindBuffer(GL_ARRAY_BUFFER, 0); 
}

static int  FisheyeUnit_InitUnitBorder(FisheyeUnit* unit, UnitBorder* ub)
{
	Unit_Context* unit_ctx = (Unit_Context*)unit->priv_data; 
	ub->priv_data = Unit_Border_New(unit_ctx->lshader, ub->color, unit_ctx->win_height); 
	if(NULL == ub->priv_data)
		return -1; 
	ub->uninit = (void(*)(void*))&Unit_Border_Delete; 
	ub->primitive.OnDraw = (void(*)(struct FisheyeUnit_ExtraPrimitive*))&UnitBorder_OnDraw; 
	return 0; 
}

static int  FisheyeUnit_OnReshape(FisheyeUnit* unit, MSRecti* render_target)
{
	return Unit_Context_OnReshape((Unit_Context*)unit->priv_data, render_target, &unit->relaRect); 
}

static int FisheyeUnit__OnGrab(FisheyeUnit* unit, void* buf, int width, int height, int pixel_size)
{
	GLenum format = 0; 
	switch (pixel_size)
	{
	case 3: 
		format = GL_RGB; 
		break; 
	case 4:
		format = GL_RGBA; 
		break; 
	default: 
		return -1; 
	}
	GLuint fbo = 0, rbo_pre = 0, rbo = 0; 
	glGenFramebuffers(1, &fbo); 
	glGenRenderbuffers(1, &rbo); 
	//	glGetIntegerv(GL_RENDERBUFFER_BINDING, &rbo_pre); 
	glBindFramebuffer(GL_FRAMEBUFFER, fbo); 
	glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, width, height); 
	glGetIntegerv(GL_RENDERBUFFER_BINDING, (GLint *)&rbo_pre); 
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo); 
	unit->inter_func->OnDraw(unit); 
	glReadPixels(0, 0, width, height, format, GL_UNSIGNED_BYTE, buf); 
	glBindFramebuffer(GL_FRAMEBUFFER, 0); 
	glBindRenderbuffer(GL_RENDERBUFFER, rbo_pre); 
	glDeleteFramebuffers(1, &fbo); 
	glDeleteRenderbuffers(1, &rbo); 
	return 0; 
}

static int FisheyeUnitO_InitContext(FisheyeUnitO* uo, Mode_Context* mode_ctx) 
{
	if (NULL == (uo->unit.priv_data = malloc(sizeof(UnitO_Context))))
		return -1; 
	return UnitO_Context_Init((UnitO_Context*)uo->unit.priv_data, mode_ctx, &uo->unit.relaRect); 
}
static void FisheyeUnitO_UninitContext(FisheyeUnitO* uo)
{
	UnitO_Context_Uninit((UnitO_Context*)uo->unit.priv_data); 
	free(uo->unit.priv_data); 
	return ; 
}
static int FisheyeUnitO_OnDraw(FisheyeUnitO* uo) 
{
	UnitO_Context* u_ctx = (UnitO_Context*)(uo->unit.priv_data); 
	OShader_UseShader(u_ctx->osh); 
	OShader_UpdateYuvTex(u_ctx->osh, 0, u_ctx->TsMat_UnitO); 
	OShader_SetParam(u_ctx->osh, &uo->orig); 
	MSRecti* viewport = &u_ctx->unit_ctx.view_port; 
	glViewport(viewport->x,
		viewport->y,
		viewport->w,
		viewport->h
	); 
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); 
	return 0; 
}

static MSPointf WireFrame_O_transform_point(WireFrame* wf, MSPointf point)
{
	return (MSPointf){point.x*2.0f-1.0f, point.y*2.0f-1.0f}; 
}
static void WireFrame_O_OnDraw(WireFrame* wf)
{
	if (FALSE == *wf->p_enable_wireframe)
        return;
        
	Unit_WireFrame* u_wf = (Unit_WireFrame*)wf->priv_data; 
	wf->update(wf); 
	LineShader_UseShader(u_wf->line_shader); 
	glBindBuffer(GL_ARRAY_BUFFER, u_wf->VBO_Line); 
	glBufferData(GL_ARRAY_BUFFER, sizeof(u_wf->output_points), u_wf->output_points, GL_STATIC_DRAW ); 
	glVertexAttribPointer(VERTEXT_BORDER_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0); 
	LineShader_SetParam(u_wf->line_shader, u_wf->color_rgba[0], u_wf->color_rgba[1], u_wf->color_rgba[2], u_wf->color_rgba[3]); 
	glLineWidth((GLfloat)wf->width); 
	glDrawArrays(GL_LINE_STRIP, 0, sizeof(u_wf->output_points)/sizeof(u_wf->output_points[0])); 
	glBindBuffer(GL_ARRAY_BUFFER, 0); 
	
}
static void WireFrame_O_Update(WireFrame* wf)
{
	if( ( wf->param.seq == wf->r_param->param.seq ) && (wf->r_param->install_model->param.seq == wf->installmodel_seq))
	{
		return; 
	}
	wf->param.seq = wf->r_param->param.seq; 
	wf->installmodel_seq = wf->r_param->install_model->param.seq; 
	
	GLfloat mode_angle = 0.0f; 
	switch(wf->r_param->install_model->install_model)
	{
	case flat: 
		mode_angle = wf->r_param->beta + MS_PI/2.0f; 
		break; 
	case Ceilling: 
		mode_angle = wf->r_param->beta - MS_PI/2.0f; 
		break; 
	case wall: 
		mode_angle = -MS_PI; 
		break; 
	default: 
		mode_angle = -wf->r_param->beta + MS_PI/2.0f; 
	}
	Unit_WireFrame* u_wf = (Unit_WireFrame*)wf->priv_data; 
	mat4_t matrix_0 = m4_scaling((vec3_t){*wf->aspect_r, 1.0f, 1.0f}); 
	mat4_t matrix_1 = m4_mul(m4_translation((vec3_t){0.0f, 0.0f, 1.41421f/tanf(wf->r_param->theta/2.0f)}), matrix_0); 
	mat4_t matrix_2 = m4_mul(m4_rotation_z(mode_angle), matrix_1); 
	u_wf->TransformMatrixs = m4_mul(m4_rotation(wf->r_param->alpha, (vec3_t) { -sinf(wf->r_param->beta), cosf(wf->r_param->beta), 0 }), matrix_2); 
	int i = 0; 
	vec3_t tmp = {0}; 
	float zoom = 1.0f; 
	vec3_t normal = {0}; 
	for(i=0; i<sizeof(wire_points)/sizeof(wire_points[0]); i++)
	{
		tmp = m4_mul_pos(u_wf->TransformMatrixs, *((vec3_t*)wire_points[i])); 
		normal = v3_norm(tmp); 
		zoom = 1/(sqrtf(2*normal.z+2)*sinf(AngleToRadian(171.0f)/4)); 
		u_wf->output_points[i][0] = zoom * normal.x * wf->correct_region->w; 
		u_wf->output_points[i][1] = zoom * normal.y * wf->correct_region->h; 
		u_wf->output_points[i][2] = 1.0f; 
	}
}
static int FisheyeUnitO_InitWireFrame(FisheyeUnitO* unit, WireFrame* wire_frame)
{
	// 设置绘制函数
	wire_frame->primitive.OnDraw = (void(*)(struct FisheyeUnit_ExtraPrimitive*))&WireFrame_O_OnDraw; 
	// 设置更新函数 
	wire_frame->update = &WireFrame_O_Update; 
	// 设置坐标转换函数
	wire_frame->transform_point = &WireFrame_O_transform_point; 
	wire_frame->priv_data = Unit_WireFrame_New(((UnitO_Context*)unit->unit.priv_data)->unit_ctx.lshader, wire_frame->color); 
	if(NULL == wire_frame->priv_data)
	{
		return -1; 
	}
	wire_frame->uninit = (void(*)(void*))&Unit_WireFrame_Delete; 
	return 0; 
}
static int FisheyeUnitO_OnGrab(FisheyeUnitO* unit, void* buf, int width, int height, int pixel_size)
{
	UnitO_Context* u_ctx = (UnitO_Context*)(unit->unit.priv_data); 
	//  重设 viewport
	MSRecti view_port_pre = u_ctx->unit_ctx.view_port; 
	u_ctx->unit_ctx.view_port = (MSRecti){ .x = 0, .y = 0, .w = width, .h = height }; 
	FisheyeUnit__OnGrab((FisheyeUnit*)unit, buf, width, height, pixel_size); 
	//  将viewport复原
	u_ctx->unit_ctx.view_port = view_port_pre; 
	return 0; 
}

typedef struct UnitP_WireFrame{
	Unit_WireFrame u_wf; 
	int breakPoint[2]; 
	FisheyeParamP* pano; 
	GLint pano_seq; 
}UnitP_WireFrame; 

static UnitP_WireFrame*  UnitP_WireFrame_New(FisheyeParamP* pano, struct LineShader* line_shader, GLuint color)
{
	UnitP_WireFrame* u_wf = (UnitP_WireFrame*)malloc(sizeof(UnitP_WireFrame)); 
	if (NULL == u_wf)
		goto end; 
	Unit_WireFrame_Init(&u_wf->u_wf, line_shader, color); 
	u_wf->breakPoint[0] = u_wf->breakPoint[1] = 0; 
	u_wf->pano = pano; 
	u_wf->pano_seq = 0; 
end:
	return u_wf; 
	
}
static void UnitP_WireFrame_Delete(UnitP_WireFrame* u_wf)
{
	Unit_WireFrame_Uninit(&u_wf->u_wf); 
	free(u_wf); 
}
static MSPointf WireFrame_P_transform_point(WireFrame* wf, MSPointf point)
{
	UnitP_WireFrame* u_wf = (UnitP_WireFrame*)wf->priv_data; 
	float angle = 0.0f, pho = 0.0f; 
	if(Ceilling == wf->r_param->install_model->install_model)
	{
		angle = (1.0f - point.x) * MS_PI * 2 + u_wf->pano->offset; 
		pho = 1.0f - point.y; 
	}
	else
	{
		angle = point.x * MS_PI * 2 + u_wf->pano->offset; 
		pho = point.y; 
	}
	return (MSPointf){pho * cosf(-angle), pho*sinf(-angle)}; 
}
static void WireFrame_P_OnDraw(WireFrame* wf)
{
	if (FALSE == *wf->p_enable_wireframe)
		return; 

	Unit_WireFrame* u_wf = &((UnitP_WireFrame*)wf->priv_data)->u_wf; 
	wf->update(wf); 
	LineShader_UseShader(u_wf->line_shader); 
	glBindBuffer(GL_ARRAY_BUFFER, u_wf->VBO_Line); 
	glBufferData(GL_ARRAY_BUFFER, sizeof(u_wf->output_points), u_wf->output_points, GL_STATIC_DRAW ); 
	glVertexAttribPointer(VERTEXT_BORDER_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(VERTEXT_BORDER_INDEX); 
	LineShader_SetParam(u_wf->line_shader, u_wf->color_rgba[0], u_wf->color_rgba[1], u_wf->color_rgba[2], u_wf->color_rgba[3]); 
	glLineWidth((GLfloat)wf->width); 
	int* breakPoint = ((UnitP_WireFrame*)wf->priv_data)->breakPoint; 
	glDrawArrays(GL_LINE_STRIP, 				  0, breakPoint[0]); 
	glDrawArrays(GL_LINE_STRIP, breakPoint[0], breakPoint[1] - breakPoint[0]); 
	glDrawArrays(GL_LINE_STRIP, breakPoint[1], sizeof(u_wf->output_points)/sizeof(u_wf->output_points[0]) - breakPoint[1]); 
	glBindBuffer(GL_ARRAY_BUFFER, 0); 
}
static void WireFrame_P_Update(WireFrame* wf)
{
	UnitP_WireFrame* u_wf = (UnitP_WireFrame*)wf->priv_data; 
	if((wf->param.seq == wf->r_param->param.seq) && (wf->r_param->install_model->param.seq == wf->installmodel_seq) && (u_wf->pano_seq == u_wf->pano->param.seq))
	{
		return; 
	}
	wf->param.seq = wf->r_param->param.seq; 
	wf->installmodel_seq = wf->r_param->install_model->param.seq; 
	u_wf->pano_seq = u_wf->pano->param.seq; 
	GLfloat mode_angle = 0.0f; 
	switch(wf->r_param->install_model->install_model)
	{
	case flat: 
		mode_angle = wf->r_param->beta + MS_PI/2.0f; 
		break; 
	case Ceilling: 
		mode_angle = wf->r_param->beta - MS_PI/2.0f; 
		break; 
	case wall: 
		mode_angle = -MS_PI; 
		break; 
	default: 
		mode_angle = -wf->r_param->beta + MS_PI/2.0f; 
	}


	mat4_t matrix__ = m4_scaling((vec3_t){*wf->aspect_r, 1.0f, 1.0f}); 
	mat4_t matrix_0 = m4_mul(m4_translation((vec3_t){0.0f, 0.0f, 1.41421f/tanf(wf->r_param->theta/2.0f)}), matrix__); 
	mat4_t matrix_1 = m4_mul(m4_rotation_z(mode_angle), matrix_0); 
	mat4_t matrix_2 = m4_mul(m4_rotation(wf->r_param->alpha, (vec3_t) { -sinf(wf->r_param->beta), cosf(wf->r_param->beta), 0 }), matrix_1); 
	u_wf->u_wf.TransformMatrixs = m4_mul(m4_rotation_z(-u_wf->pano->offset), matrix_2); 
	int i = 0; 
	vec3_t tmp = {0}; 
	float zoom = 1.0f; 
	float angle = 0.0f, length = 0.0f; 
	vec3_t normal = {0}; 
	float x = 0.0f, y = 0.0f; 
	for(i=0; i<sizeof(wire_points)/sizeof(wire_points[0]); i++)
	{
		tmp = m4_mul_pos(u_wf->u_wf.TransformMatrixs, *((vec3_t*)wire_points[i])); 
		normal = v3_norm(tmp); 
		zoom = 1/(sqrtf(2*normal.z+2)*sinf(AngleToRadian(171.0f)/4)); 
		x = zoom * normal.x; 
		y = zoom * normal.y; 
		length = sqrtf(powf(x, 2) + powf(y, 2)); 
		if(0.0f == length)
			return ; 
		angle = y>0 ? acosf(x/length) : 2*MS_PI - acosf(x/length); // angle ∈ [-Π， Π]
		angle /= 2*MS_PI; 	// angle ∈ [  0，  1]
		u_wf->u_wf.output_points[i][0] = Ceilling != wf->r_param->install_model->install_model ? angle*2.0f - 1.0f : 1.0f - angle*2.0f; 
		u_wf->u_wf.output_points[i][1] = Ceilling != wf->r_param->install_model->install_model ? 1.0f - length*2.0f : length*2.0f - 1.0f; 
		u_wf->u_wf.output_points[i][2] = 1.0f; 
	}
	int j = 0; 
	u_wf->breakPoint[0] = 0; 
	u_wf->breakPoint[1] = sizeof(u_wf->u_wf.output_points)/sizeof(u_wf->u_wf.output_points[0]); 
	for(i=1; i<sizeof(u_wf->u_wf.output_points)/sizeof(u_wf->u_wf.output_points[0]); i++){
		if(fabs(u_wf->u_wf.output_points[i][0]-u_wf->u_wf.output_points[i-1][0]) > 1.4f)
		{
			u_wf->breakPoint[j] = i; j++; 
			if(j == 2)
				break; 
		}
	}
}
static int FisheyeUnitP_InitContext(FisheyeUnitP* up, Mode_Context* mode_ctx)
{
	if (NULL == (up->unit.priv_data = malloc(sizeof(UnitP_Context))))
		return -1; 
	return UnitP_Context_Init((UnitP_Context*)up->unit.priv_data, mode_ctx, &up->unit.relaRect); 
}
static int FisheyeUnitP_UninitContext(FisheyeUnitP* up)
{
	UnitP_Context_Uninit((UnitP_Context*)up->unit.priv_data); 
	free(up->unit.priv_data); 
	return 0; 
}
static int FisheyeUnitP_OnDraw(FisheyeUnitP* up)
{
	UnitP_Context* u_ctx = (UnitP_Context*)(up->unit.priv_data); 
	PShader_UseShader(u_ctx->psh); 
	PShader_UpdateYuvTex(u_ctx->psh, 0, u_ctx->unit_ctx.TsMat); 
	
	PShader_SetParam(u_ctx->psh, &up->pano); 
	MSRecti* viewport = &u_ctx->unit_ctx.view_port; 
	glViewport(viewport->x,
		viewport->y,
		viewport->w,
		viewport->h
	); 
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); 
	return 0; 
}
static int FisheyeUnitP_InitWireFrame(FisheyeUnitP* unit, WireFrame* wire_frame)
{
	// 设置绘制函数
	wire_frame->primitive.OnDraw = (void(*)(struct FisheyeUnit_ExtraPrimitive*))&WireFrame_P_OnDraw; 
	// 设置更新函数 
	wire_frame->update = WireFrame_P_Update; 
	// 设置坐标转换函数
	wire_frame->transform_point = WireFrame_P_transform_point; 
	wire_frame->priv_data = UnitP_WireFrame_New(&unit->pano, ((UnitP_Context*)unit->unit.priv_data)->unit_ctx.lshader, wire_frame->color); 
	if (NULL == wire_frame->priv_data)
	{
		return -1; 
	}
	wire_frame->uninit = (void(*)(void*))&UnitP_WireFrame_Delete; 
	
	return 0; 
}
static int FisheyeUnitP_OnGrab(FisheyeUnitP* unit, void* buf, int width, int height, int pixel_size)
{
	UnitP_Context* u_ctx = (UnitP_Context*)(unit->unit.priv_data); 
	//  重设 viewport
	MSRecti view_port_pre = u_ctx->unit_ctx.view_port; 
	u_ctx->unit_ctx.view_port = GetRectByRatio( (MSRecti){ .x = 0, .y = 0, .w = width, .h = height }, 1.0f/3.0f); 
	FisheyeUnit__OnGrab((FisheyeUnit*)unit, buf, width, height, pixel_size); 
	//  将viewport复原
	u_ctx->unit_ctx.view_port = view_port_pre; 
	return 0; 
}



static int FisheyeUnit2P_OnDraw(FisheyeUnitP* up)
{
	UnitP_Context* u_ctx = (UnitP_Context*)(up->unit.priv_data); 
	PShader_UseShader(u_ctx->psh); 
	PShader_UpdateYuvTex(u_ctx->psh, 0, u_ctx->unit_ctx.TsMat); 
	PShader_SetParam(u_ctx->psh, &up->pano); 
	MSRecti* viewport = &u_ctx->unit_ctx.view_port; 
	glViewport(viewport->x, 
		viewport->y, 
		viewport->w, 
		viewport->h/2
	); 
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); 
	FisheyeParamP pano = up->pano; 
	pano.offset += MS_PI; 
	PShader_SetParam(u_ctx->psh, &pano); 
	glViewport(viewport->x, 
		viewport->y + viewport->h/2, 
		viewport->w, 
		viewport->h/2
	); 
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); 
	glViewport(viewport->x, 
		viewport->y, 
		viewport->w, 
		viewport->h
	); 
	return 0; 
}

static int FisheyeUnit2P_OnGrab(FisheyeUnit2P* unit, void* buf, int width, int height, int pixel_size)
{
	UnitP_Context* u_ctx = (UnitP_Context*)(unit->unit.priv_data); 
	//  重设 viewport
	MSRecti view_port_pre = u_ctx->unit_ctx.view_port; 
	u_ctx->unit_ctx.view_port = GetRectByRatio((MSRecti) { .x = 0, .y = 0, .w = width, .h = height }, 1.0f); 
	FisheyeUnit__OnGrab((FisheyeUnit*)unit, buf, width, height, pixel_size); 
	//  将viewport复原
	u_ctx->unit_ctx.view_port = view_port_pre; 
	return 0; 
}


static int FisheyeUnitW_InitContext(FisheyeUnitW* uw, Mode_Context* mode_ctx)
{
	if (NULL == (uw->unit.priv_data = malloc(sizeof(UnitW_Context))))
		return -1; 
	return UnitW_Context_Init((UnitW_Context*)uw->unit.priv_data, mode_ctx, &uw->unit.relaRect); 
}
static int FisheyeUnitW_UninitContext(FisheyeUnitW* uw)
{
	UnitW_Context_Uninit((UnitW_Context*)uw->unit.priv_data); 
	free(uw->unit.priv_data); 
	return 0; 
}
static int FisheyeUnitW_OnDraw(FisheyeUnitW* uw)
{
	UnitW_Context* u_ctx = (UnitW_Context*)(uw->unit.priv_data); 
	WShader_UseShader(u_ctx->wsh); 
	WShader_UpdateYuvTex(u_ctx->wsh, 0, u_ctx->unit_ctx.TsMat); 
	WShader_SetParam(u_ctx->wsh, &uw->wparam); 
	MSRecti* viewport = &u_ctx->unit_ctx.view_port; 
	glViewport(viewport->x,
		viewport->y,
		viewport->w,
		viewport->h
	); 
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); 
	return 0; 

}

static MSPointf WireFrame_W_transform_point(WireFrame* wf, MSPointf point)
{
	MSPointf p = {point.x*1.8f-0.9f, point.y*1.64f-0.82f}; 
	p.x = fabs(p.y) > 1.0f ? 0.0f : p.x * sqrtf(1.0f - powf(p.y, 2));
	return p; 
}
static void WireFrame_W_OnDraw(WireFrame* wf)
{
	if (FALSE == *wf->p_enable_wireframe)
		return; 

	Unit_WireFrame* u_wf = (Unit_WireFrame*)wf->priv_data; 
	wf->update(wf); 
	LineShader_UseShader(u_wf->line_shader); 
	glBindBuffer(GL_ARRAY_BUFFER, u_wf->VBO_Line); 
	glBufferData(GL_ARRAY_BUFFER, sizeof(u_wf->output_points), u_wf->output_points, GL_STATIC_DRAW ); 
	glVertexAttribPointer(VERTEXT_BORDER_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0); 
	LineShader_SetParam(u_wf->line_shader, u_wf->color_rgba[0], u_wf->color_rgba[1], u_wf->color_rgba[2], u_wf->color_rgba[3]); 
	glLineWidth((GLfloat)wf->width); 
	glDrawArrays(GL_LINE_STRIP, 0, sizeof(u_wf->output_points)/sizeof(u_wf->output_points[0])); 
	glBindBuffer(GL_ARRAY_BUFFER, 0); 
	
}
static void WireFrame_W_Update(WireFrame* wf)
{
	if(wf->param.seq == wf->r_param->param.seq && wf->r_param->install_model->param.seq)
	{
		return; 
	}
	wf->param.seq = wf->r_param->param.seq; 
	wf->installmodel_seq = wf->param.seq == wf->r_param->param.seq; 
	Unit_WireFrame* u_wf = (Unit_WireFrame*)wf->priv_data; 
	mat4_t matrix_0 = m4_scaling((vec3_t){*wf->aspect_r, 1.0f, 1.0f}); 
	mat4_t matrix_1 = m4_mul(m4_translation((vec3_t){0.0f, 0.0f, 1.41421f/tanf(wf->r_param->theta/2.0f)}), matrix_0); 
	mat4_t matrix_2 = m4_mul(m4_rotation_z(wf->r_param->beta + MS_PI/2.0f), matrix_1); 
	u_wf->TransformMatrixs = m4_mul(m4_rotation(wf->r_param->alpha, (vec3_t) { -sinf(wf->r_param->beta), cosf(wf->r_param->beta), 0 }), matrix_2); 
	int i = 0; 
	vec3_t tmp = {0}; 
	float zoom = 1.0f; 
	vec3_t normal = {0}; 
	float x = 0.0f, y = 0.0f; 
	for(i=0; i<sizeof(wire_points)/sizeof(wire_points[0]); i++)
	{
		tmp = m4_mul_pos(u_wf->TransformMatrixs, *((vec3_t*)wire_points[i])); 
		normal = v3_norm(tmp); 
		zoom = 1/(sqrtf(2*normal.z+2)*sinf(AngleToRadian(171.0f)/4)); 
		x = zoom * normal.x; 
		y = zoom * normal.y; 
		u_wf->output_points[i][0] = (x/sqrtf(1.0f - powf(y, 2))) * 1.111f; 
		u_wf->output_points[i][1] = y * 1.220f; 
		u_wf->output_points[i][2] = 1.0f; 
	}
}
static int FisheyeUnitW_InitWireFrame(FisheyeUnitW* unit, WireFrame* wire_frame)
{
//	return -1; 
	// 设置绘制函数
	wire_frame->primitive.OnDraw = (void(*)(struct FisheyeUnit_ExtraPrimitive*))&WireFrame_W_OnDraw; 
	// 设置更新函数 
	wire_frame->update = WireFrame_W_Update; 
	// 设置坐标转换函数
	wire_frame->transform_point = WireFrame_W_transform_point; 
	wire_frame->priv_data = Unit_WireFrame_New(((UnitW_Context*)unit->unit.priv_data)->unit_ctx.lshader, wire_frame->color); 
	if (NULL == wire_frame->priv_data)
	{
		return -1; 
	}
	wire_frame->uninit = (void(*)(void*))&Unit_WireFrame_Delete; 
	return 0; 
}
static int FisheyeUnitW_OnGrab(FisheyeUnitW* unit, void* buf, int width, int height, int pixel_size)
{
	UnitW_Context* u_ctx = (UnitW_Context*)(unit->unit.priv_data); 
	//  重设 viewport
	MSRecti view_port_pre = u_ctx->unit_ctx.view_port; 
	u_ctx->unit_ctx.view_port = GetRectByRatio( (MSRecti){ .x = 0, .y = 0, .w = width, .h = height }, 1.0f); 
	FisheyeUnit__OnGrab((FisheyeUnit*)unit, buf, width, height, pixel_size); 
	//  将viewport复原
	u_ctx->unit_ctx.view_port = view_port_pre; 
	return 0; 
}


static int FisheyeUnitR_InitContext(FisheyeUnitR* ur, Mode_Context* mode_ctx)
{
	if (NULL == (ur->unit.priv_data = malloc(sizeof(UnitR_Context))))
		return -1; 
	return UnitR_Context_Init((UnitR_Context*)ur->unit.priv_data, mode_ctx, &ur->unit.relaRect); 
}
static int FisheyeUnitR_UninitContext(FisheyeUnitR* ur)
{
	UnitR_Context_Uninit((UnitR_Context*)ur->unit.priv_data); 
	free(ur->unit.priv_data); 
	return 0; 
}
static int FisheyeUnitR_OnDraw(FisheyeUnitR* ur)
{
	UnitR_Context* u_ctx = (UnitR_Context*)(ur->unit.priv_data); 
	UnitR_Context_Update(u_ctx, &ur->ptz, *ur->unit.aspect_r); 
	RShader_UseShader(u_ctx->rsh); 
	RShader_UpdateYuvTex(u_ctx->rsh, 0, u_ctx->unit_ctx.TsMat); 
	RShader_SetParam(u_ctx->rsh, &u_ctx->TransformMatrixs); 
	MSRecti* viewport = &u_ctx->unit_ctx.view_port; 
	glViewport(viewport->x,
		viewport->y,
		viewport->w,
		viewport->h
	); 
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); 
	return 0; 
}
static int FisheyeUnitR_InitWireFrame(FisheyeUnitR* ur, WireFrame* wire_frame)
{
	return -1; 
}
static int FisheyeUnitR_OnGrab(FisheyeUnitR* unit, void* buf, int width, int height, int pixel_size)
{
	UnitR_Context* u_ctx = (UnitR_Context*)(unit->unit.priv_data); 
	//  重设 viewport
	MSRecti view_port_pre = u_ctx->unit_ctx.view_port; 
	u_ctx->unit_ctx.view_port = GetRectByRatio((MSRecti) { .x = 0, .y = 0, .w = width, .h = height }, *unit->unit.aspect_r); 
	FisheyeUnit__OnGrab((FisheyeUnit*)unit, buf, width, height, pixel_size); 
	//  将viewport复原
	u_ctx->unit_ctx.view_port = view_port_pre; 
	return 0; 
}


static int FisheyeMode_InitContext(FisheyeMode* fm, Player_Context* player_ctx) 
{
	if (NULL == (fm->priv_data = malloc(sizeof(Mode_Context))))
		return -1; 
	return Mode_Context_Init((Mode_Context*)fm->priv_data, player_ctx, &fm->render_target); 
}
static int FisheyeMode_UninitContext(FisheyeMode* fm)
{
	Mode_Context_Uninit((Mode_Context*)fm->priv_data); 
	free(fm->priv_data); 
	return 0; 
}
int FisheyePlayer_TransformBegin(FisheyePlayer* fp, int OutputWidth, int OutputHeight)
{
	
	Player_Context* player_ctx = (Player_Context*)fp->priv_data; 
	if(-1 != player_ctx->transform_fbo)
		glDeleteFramebuffers(1, &player_ctx->transform_fbo); 
	
	if (-1 != player_ctx->transform_tbo)
		glDeleteTextures(1, &player_ctx->transform_tbo);  

	glDisable(GL_BLEND); 
	glDisable(GL_DEPTH_TEST); 
	// 由于之前使用了 PBO，状态未更改 调用 glTexImage2D时 根据之前的 PBO，而导致了失败
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0); 
	
	glGenTextures(1, &player_ctx->transform_tbo); 
	glActiveTexture(GL_TEXTURE3); 
	glBindTexture(GL_TEXTURE_2D, player_ctx->transform_tbo); 
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, OutputWidth, OutputHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
	glBindTexture(GL_TEXTURE_2D, 0); 

	glGenFramebuffers(1, &player_ctx->transform_fbo); 
	glBindFramebuffer(GL_FRAMEBUFFER, player_ctx->transform_fbo); 
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, player_ctx->transform_tbo, 0); 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	FisheyePlayer_Reshape(fp, 0, 0, OutputWidth, OutputHeight); 
	FisheyePlayer_SetStretchMode(fp, StretchMode_Resize); 
	
	
	return 0; 
}

void FisheyePlayer_TransformGetBuf(FisheyePlayer* fp, void **ppVirAddr, int *pPhyAddr)
{
	Player_Context* player_ctx = (Player_Context*)fp->priv_data;
	*ppVirAddr = player_ctx->EGL.InputVirAddr;
	*pPhyAddr  = player_ctx->EGL.InputPhyAddr;
}

int FisheyePlayer_Transforming(FisheyePlayer* fp, FisheyeTSdata *pstSrc, FisheyeTSdata *pstDst)
{
	Player_Context* player_ctx = (Player_Context*)fp->priv_data;

	FisheyePlayer_InitData(fp, pstSrc->pVirAddr); 
	glBindFramebuffer(GL_FRAMEBUFFER, player_ctx->transform_fbo); 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	FisheyePlayer_OnDraw(fp); 
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0); 
	glActiveTexture(GL_TEXTURE3); 
	glBindTexture(GL_TEXTURE_2D, player_ctx->transform_tbo); 
	
	ChgShader_UseShader(&player_ctx->chgshader); 
	ChgShader_SetParam(&player_ctx->chgshader, 3, 1.0f/player_ctx->EGL.OutputWidth); 
	
	glViewport(0, 0, player_ctx->EGL.OutputWidth/4, player_ctx->EGL.OutputHeight * 3/2); 
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); 
	
	glFinish(); 
    pstDst->pVirAddr = player_ctx->EGL.OutputVirAddr;
    pstDst->phyAddr  = player_ctx->EGL.OutputPhyAddr;
    pstDst->width  = player_ctx->EGL.OutputWidth;
    pstDst->height = player_ctx->EGL.OutputHeight;
    pstDst->size   = pstDst->width * pstDst->height * 3/2;
    
	return 0; 
}

int FisheyePlayer_TransformEnd(FisheyePlayer* fp)
{
	Player_Context* player_ctx = (Player_Context*)fp->priv_data; 
	if (-1 != player_ctx->transform_fbo)
		glDeleteFramebuffers(1, &player_ctx->transform_fbo); 
	player_ctx->transform_fbo = -1; 
	if (-1 != player_ctx->transform_tbo)
		glDeleteTextures(1, &player_ctx->transform_tbo); 
	player_ctx->transform_tbo = -1; 
	return 0; 
}
static int FisheyeMode__OnGrab(FisheyeMode* fm, void* buf, int width, int height, int pixel_size)
{
	GLenum format = 0; 
	switch (pixel_size)
	{
	case 3:
		format = GL_RGB; 
		break; 
	case 4:
		format = GL_RGBA; 
		break; 
	default:
		return -1; 
	}
	GLuint tbo; 
	glDisable(GL_BLEND); 
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0); // 由于之前使用了 PBO，状态未更改 调用 glTexImage2D时 根据之前的 PBO导致了失败
	glGenTextures(1, &tbo); 
	glActiveTexture(GL_TEXTURE3); 
	glBindTexture(GL_TEXTURE_2D, tbo); 
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, NULL); // 由于之前使用了 PBO，状态未更改 调用 glTexImage2D时 根据之前的 PBO导致了失败
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
	glBindTexture(GL_TEXTURE_2D, 0); 
	GLuint fbo = 0, rbo_pre = 0, rbo = 0; 
	glGenFramebuffers(1, &fbo); 
	glGenRenderbuffers(1, &rbo); 
	glGetIntegerv(GL_RENDERBUFFER_BINDING, (GLint *)&rbo_pre); 
	glBindFramebuffer(GL_FRAMEBUFFER, fbo); 
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tbo, 0); 
//	glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
//	glRenderbufferStorage(GL_RENDERBUFFER, format, width, height); 
	glGetIntegerv(GL_RENDERBUFFER_BINDING, (GLint *)&rbo_pre); 
//	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo); 
	// 重设 rendertarget， 拉伸模式改为 Original
	Mode_Context* m_c = (Mode_Context*)fm->priv_data; 
	int win_height_pre = *m_c->player_ctx->win_height; 
	*m_c->player_ctx->win_height = height; 
	MSRecti mode_rt_pre = fm->render_target; 
	enum StretchMode strench_mode_pre = *fm->pstretch_mode; 
	*fm->pstretch_mode = StretchMode_Resize; 
	FisheyeMode_OnReshape(fm, (MSRecti) { .x = 0, .y = 0, .w = width, .h = height }); 

	FisheyeMode_OnDraw(fm); 
	glReadPixels(0, 0, width, height, format, GL_UNSIGNED_BYTE, buf); 
	glBindFramebuffer(GL_FRAMEBUFFER, 0); 
	glBindRenderbuffer(GL_RENDERBUFFER, rbo_pre); 
	glDeleteRenderbuffers(1, &rbo); 
	glDeleteFramebuffers(1, &fbo); 
	glDeleteTextures(1, &tbo); 
	// 将render target 和 拉伸模式复原
	*m_c->player_ctx->win_height = win_height_pre; 
	*fm->pstretch_mode = strench_mode_pre; 
	FisheyeMode_OnReshape(fm, mode_rt_pre); 

	return 0; 
}
static int FisheyePlayer_InitContext(FisheyePlayer* fp, void* arg)
{
    int ret;
    
	if (NULL == (fp->priv_data = malloc(sizeof(Player_Context))))
		return -1; 

    memset(fp->priv_data, 0, sizeof(Player_Context));
    ret = Player_Context_Init((Player_Context*)fp->priv_data, fp->width, fp->height, &fp->renderTarget, &fp->correct_region, &fp->fisheye_region, arg); 
    if (ret == -1) {
        free(fp->priv_data);
        fp->priv_data = NULL;
    }

    return ret;
}
static int FisheyePlayer_UninitContext(FisheyePlayer* fp)
{
	Player_Context_Uninit((Player_Context*)fp->priv_data); 
	free(fp->priv_data); 
	return 0; 
}
static int FisheyePlayer_BindData(FisheyePlayer* fp, char* data)
{
	Player_Context* player_ctx = (Player_Context*)fp->priv_data; 
	glActiveTexture(GL_TEXTURE0); 
	glBindTexture(GL_TEXTURE_EXTERNAL_OES, player_ctx->TBO_yuv[0]); 
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, player_ctx->EGL.image); 
	return 0; 
}

static int FisheyePlayer_OnChanged_Data(FisheyePlayer* fp)
{
	Player_Context* player_ctx = (Player_Context*)fp->priv_data; 

	MSRectf* fisheye_region = &fp->fisheye_region; 
	
	player_ctx->TsMat[0] = fp->correct_region.w, player_ctx->TsMat[1] = 0.0f, player_ctx->TsMat[2] = 0.0f; 
	player_ctx->TsMat[3] = 0.0f; player_ctx->TsMat[4] = fp->correct_region.h, player_ctx->TsMat[5] = 0.0f; 
	player_ctx->TsMat[6] = fp->correct_region.x; player_ctx->TsMat[7] = fp->correct_region.y, player_ctx->TsMat[8] = 1.0f; 
	
	player_ctx->TsMat_UnitO[0] = fisheye_region->w, player_ctx->TsMat_UnitO[1] = 0.0f, player_ctx->TsMat_UnitO[2] = 0.0f; 
	player_ctx->TsMat_UnitO[3] = 0.0f; player_ctx->TsMat_UnitO[4] = fisheye_region->h, player_ctx->TsMat_UnitO[5] = 0.0f; 
	player_ctx->TsMat_UnitO[6] = fisheye_region->x; player_ctx->TsMat_UnitO[7] = fisheye_region->y, player_ctx->TsMat_UnitO[8] = 1.0f; 
	
	return 0; 
}

static const FisheyeUnit_Function fisheye_unit_o_function_opengl = {
	.inter_func = { (int(*)(void*, void*))&FisheyeUnitO_InitContext, (void(*)(void*))&FisheyeUnitO_UninitContext },
	.OnDraw = (int (*)(struct FisheyeUnit*))&FisheyeUnitO_OnDraw, 
	.InitWireFrame = (int (*)(struct FisheyeUnit*, WireFrame*))&FisheyeUnitO_InitWireFrame, 
	.InitUnitBorder = &FisheyeUnit_InitUnitBorder, 
	.OnReshape = &FisheyeUnit_OnReshape, 
	.OnGrab = (int(*)(struct FisheyeUnit*, void*, int, int, int))&FisheyeUnitO_OnGrab
}; 

static const FisheyeUnit_Function fisheye_unit_p_function_opengl = {
	.inter_func = { (int(*)(void*, void*))&FisheyeUnitP_InitContext, (void(*)(void*))&FisheyeUnitP_UninitContext },
	.OnDraw = (int (*)(struct FisheyeUnit*))&FisheyeUnitP_OnDraw, 
	.InitWireFrame = (int (*)(struct FisheyeUnit*, WireFrame*))&FisheyeUnitP_InitWireFrame, 
	.InitUnitBorder = FisheyeUnit_InitUnitBorder, 
	.OnReshape = &FisheyeUnit_OnReshape,
	.OnGrab = (int(*)(struct FisheyeUnit*, void*, int, int, int))&FisheyeUnitP_OnGrab
}; 
static const FisheyeUnit_Function fisheye_unit_2p_function_opengl = {
	.inter_func = { (int(*)(void*, void*))&FisheyeUnitP_InitContext, (void(*)(void*))&FisheyeUnitP_UninitContext },
	.OnDraw = (int (*)(struct FisheyeUnit*))&FisheyeUnit2P_OnDraw, 
	.InitWireFrame = (int (*)(struct FisheyeUnit*, WireFrame*))FisheyeUnitR_InitWireFrame,  // TODO 2P视图 线框是否需要实现？？？
	.InitUnitBorder = FisheyeUnit_InitUnitBorder, 
	.OnReshape = &FisheyeUnit_OnReshape,
	.OnGrab = (int(*)(struct FisheyeUnit*, void*, int, int, int))&FisheyeUnit2P_OnGrab
}; 

static const FisheyeUnit_Function fisheye_unit_w_function_opengl = {
	.inter_func = { (int(*)(void*, void*))&FisheyeUnitW_InitContext, (void(*)(void*))&FisheyeUnitW_UninitContext },
	.OnDraw = (int (*)(struct FisheyeUnit*))&FisheyeUnitW_OnDraw, 
	.InitWireFrame = (int (*)(struct FisheyeUnit*, WireFrame*))&FisheyeUnitW_InitWireFrame, 
	.InitUnitBorder = FisheyeUnit_InitUnitBorder, 
	.OnReshape = &FisheyeUnit_OnReshape,
	.OnGrab = (int(*)(struct FisheyeUnit*, void*, int, int, int))&FisheyeUnitW_OnGrab
}; 

static const FisheyeUnit_Function fisheye_unit_r_function_opengl = {
	.inter_func = { (int(*)(void*, void*))&FisheyeUnitR_InitContext, (void(*)(void*))&FisheyeUnitR_UninitContext },
	.OnDraw = (int (*)(struct FisheyeUnit*))&FisheyeUnitR_OnDraw, 
	.InitWireFrame = (int (*)(struct FisheyeUnit*, WireFrame*))&FisheyeUnitR_InitWireFrame, 
	.InitUnitBorder = &FisheyeUnit_InitUnitBorder, 
	.OnReshape = &FisheyeUnit_OnReshape,
	.OnGrab = (int(*)(struct FisheyeUnit*, void*, int, int, int))&FisheyeUnitR_OnGrab
}; 

static const FisheyeUnit_Internal fisheye_unit_internal_opengl = {
	.unit_o_func = &fisheye_unit_o_function_opengl, 
	.unit_p_func = &fisheye_unit_p_function_opengl, 
	.unit_2p_func = &fisheye_unit_2p_function_opengl, 
	.unit_w_func = &fisheye_unit_w_function_opengl, 
	.unit_r_func = &fisheye_unit_r_function_opengl
}; 

static const FisheyeMode_Internal fisheye_mode_internal_opengl = {
	.init = (int(*)(FisheyeMode*, void*))&FisheyeMode_InitContext, 
	.uninit = &FisheyeMode_UninitContext, 
	.unit_internal = &fisheye_unit_internal_opengl, 
	.on_grab = &FisheyeMode__OnGrab
}; 

static const FisheyePlayerContextType fisheye_player_context_opengl = {
	.Init = (int(*)(FisheyePlayer*, void*))&FisheyePlayer_InitContext, 
	.UnInit = &FisheyePlayer_UninitContext, 
	.InitData = (int(*)(FisheyePlayer*, void*))&FisheyePlayer_BindData, 
	.DataChg = &FisheyePlayer_OnChanged_Data, 
	.mode_internal = &fisheye_mode_internal_opengl
}; 

const FisheyeContextType fisheye_context_opengl = {
	.type = "OpenGL", 
	.player_internal = &fisheye_player_context_opengl
}; 
