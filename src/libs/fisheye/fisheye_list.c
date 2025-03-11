#include<stdlib.h>
#include"fisheye_player.h"
extern const FisheyeContextType fisheye_context_opengl;
const FisheyeContextType* const context_type_list[] = {
	&fisheye_context_opengl,
	NULL
}; 