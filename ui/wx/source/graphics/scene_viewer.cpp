
#include <wx/wx.h>

#include "api/cmiss_scene_viewer_ui.h"
#include "graphics/scene_viewer.h"
#include "general/message.h"
#define WX_USER_INTERFACE
#include "three_d_drawing/graphics_buffer.h"


Cmiss_scene_viewer_id Cmiss_scene_viewer_create(
	struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package,
	void *parent_void,
	enum Cmiss_scene_viewer_buffering_mode buffer_mode,
	enum Cmiss_scene_viewer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth)
{
	enum Graphics_buffer_buffering_mode graphics_buffer_buffering_mode;
	enum Graphics_buffer_stereo_mode graphics_buffer_stereo_mode;
	struct Graphics_buffer_wx *graphics_buffer;
	struct Cmiss_scene_viewer *scene_viewer;
	wxPanel *parent;

	parent = (wxPanel *)parent_void;
	if (cmiss_scene_viewer_package)
	{
		if (CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE==buffer_mode)
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_ANY_BUFFERING_MODE;
		}
		else if (CMISS_SCENE_VIEWER_BUFFERING_SINGLE==buffer_mode)
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_SINGLE_BUFFERING;
		}
		else if (CMISS_SCENE_VIEWER_BUFFERING_RENDER_OFFSCREEN_AND_COPY==buffer_mode)
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_RENDER_OFFSCREEN_AND_COPY;
		}
		else if (CMISS_SCENE_VIEWER_BUFFERING_RENDER_OFFSCREEN_AND_BLEND==buffer_mode)
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_RENDER_OFFSCREEN_AND_BLEND;
		}
		else
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_DOUBLE_BUFFERING;
		}
		if (CMISS_SCENE_VIEWER_STEREO_ANY_MODE==stereo_mode)
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_ANY_STEREO_MODE;
		}
		else if (CMISS_SCENE_VIEWER_STEREO_STEREO==stereo_mode)
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_STEREO;
		}
		else
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_MONO;
		}
		graphics_buffer = create_Graphics_buffer_wx(
			Cmiss_scene_viewer_package_get_graphics_buffer_package(cmiss_scene_viewer_package),
			parent,
			graphics_buffer_buffering_mode, graphics_buffer_stereo_mode,
			minimum_colour_buffer_depth, minimum_depth_buffer_depth,
			minimum_accumulation_buffer_depth, 0);
		scene_viewer = create_Scene_viewer_from_package(reinterpret_cast<Graphics_buffer *>(graphics_buffer),
			cmiss_scene_viewer_package,
			Cmiss_scene_viewer_package_get_default_scene(cmiss_scene_viewer_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Cmiss_scene_viewer_wx.  "
			"The Cmiss_scene_viewer data must be initialised before any scene "
			"viewers can be created.");
		scene_viewer=(struct Cmiss_scene_viewer *)NULL;
	}

	return (scene_viewer);
} /* create_Cmiss_scene_viewer_wx */

