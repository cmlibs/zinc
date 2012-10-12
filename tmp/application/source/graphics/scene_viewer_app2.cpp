
#include "general/debug.h"
#include "general/message.h"
#include "graphics/scene_viewer.h"
#include "graphics/scene_viewer_app.h"
#include "three_d_drawing/graphics_buffer.h"
#include "three_d_drawing/graphics_buffer_app.h"

int Scene_viewer_get_opengl_information(struct Scene_viewer *scene_viewer,
	char **opengl_version, char **opengl_vendor, char **opengl_extensions,
	int *visual_id, int *colour_buffer_depth, int *depth_buffer_depth,
	int *accumulation_buffer_depth)
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the OpenGL state information.  The <opengl_version>, <opengl_vendor> and
<opengl_extensions> strings are static pointers supplied from the driver and
so should not be modified or deallocated.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_opengl_information);
	if (scene_viewer)
	{
		//-- scene_viewer->graphics_buffer->make_current();//-- Graphics_buffer_make_current(scene_viewer->graphics_buffer);
		*opengl_version=(char *)glGetString(GL_VERSION);
		*opengl_vendor=(char *)glGetString(GL_VENDOR);
		*opengl_extensions=(char *)glGetString(GL_EXTENSIONS);

#if defined (DEBUG_CODE)
		printf("%s\n", *opengl_extensions);
#endif /* defined (DEBUG_CODE) */

		*visual_id = 0;
		*colour_buffer_depth = 0;
		*depth_buffer_depth = 0;
		*accumulation_buffer_depth = 0;
		//-- Graphics_buffer_get_visual_id(Cmiss_scene_viewer_get_graphics_buffer(scene_viewer), visual_id);
		//-- Graphics_buffer_get_colour_buffer_depth(scene_viewer->graphics_buffer,
		//-- 	colour_buffer_depth);
		//-- Graphics_buffer_get_depth_buffer_depth(scene_viewer->graphics_buffer,
		//-- 	depth_buffer_depth);
		//-- Graphics_buffer_get_accumulation_buffer_depth(scene_viewer->graphics_buffer,
		//-- 	accumulation_buffer_depth);

		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_viewport_info.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_viewport_info */

