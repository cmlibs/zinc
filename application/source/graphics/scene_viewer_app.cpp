
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

int Scene_viewer_update_Interactive_tool(
	struct Scene_viewer *scene_viewer, void *interactive_tool_void)
/*******************************************************************************
LAST MODIFIED : 26 April 2007

DESCRIPTION :
Updates the interactive_tool that matches the type of <interactive_tool_void>
to have the same settings as <interactive_tool_void> overwriting the
settings the individual tool has.  Used to provide compatibility with the old
global tools.  The scene_viewers in a graphics_window are updated separately
from this.
==============================================================================*/
{
	char *tool_name;
	int return_code;
	struct Interactive_tool *global_interactive_tool;
	struct Interactive_tool *scene_viewer_interactive_tool;
	global_interactive_tool = (struct Interactive_tool *)interactive_tool_void;

	if (GET_NAME(Interactive_tool)(global_interactive_tool,&tool_name)
		&& (scene_viewer_interactive_tool= 0))
		//-- FIND_BY_IDENTIFIER_IN_MANAGER(Interactive_tool,name)(
		//-- (char *)tool_name,scene_viewer->interactive_tool_manager)))
	{
		Interactive_tool_copy(scene_viewer_interactive_tool,
			global_interactive_tool, (struct MANAGER(Interactive_tool) *)NULL);
	}
	return_code = 1;
	DEALLOCATE(tool_name);
	return (return_code);
}

int Cmiss_scene_viewer_package_update_Interactive_tool(
	struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package,
	struct Interactive_tool *interactive_tool)
/*******************************************************************************
LAST MODIFIED : 26 April 2007

DESCRIPTION :
Updates the interactive tools in each of the scene_viewers created with the
<cmiss_scene_viewer_package> to have the same settings as the <interactive_tool>.
This enables the old global commands to continue to work for all scene_viewers,
however new code should probably modify the particular tools for the
particular scene_viewer intended.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_scene_viewer_package_update_Interactive_tool);
	if (cmiss_scene_viewer_package)
	{
		return_code = 0;//-- FOR_EACH_OBJECT_IN_LIST(Scene_viewer)(
			//-- Scene_viewer_update_Interactive_tool, (void *)interactive_tool,
			//-- cmiss_scene_viewer_package->scene_viewer_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_scene_viewer_package_update_Interactive_tool.  Missing scene_viewer");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_package_update_Interactive_tool */

