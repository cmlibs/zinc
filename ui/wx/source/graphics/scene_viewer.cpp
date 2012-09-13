
#include <wx/wx.h>

#include "cmiss_zinc_configure.h"
#if defined (USE_GLEW)
#	include <GL/glew.h>
#endif
#include "wx/glcanvas.h"

#include "api/cmiss_scene_viewer.h"
#include "graphics/scene_viewer.h"
#include "general/message.h"
#define WX_USER_INTERFACE
#include "three_d_drawing/graphics_buffer.h"


Cmiss_scene_viewer_id Cmiss_scene_viewer_create(
	struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package,
	void *canvas_void)
{
	enum Graphics_buffer_buffering_mode graphics_buffer_buffering_mode;
	enum Graphics_buffer_stereo_mode graphics_buffer_stereo_mode;
	struct Graphics_buffer_wx *graphics_buffer;
	struct Cmiss_scene_viewer *scene_viewer;
	wxGLCanvas *canvas = (wxGLCanvas *)canvas_void;

	if (cmiss_scene_viewer_package)
	{
		graphics_buffer = create_Graphics_buffer_wx(
			Cmiss_scene_viewer_package_get_graphics_buffer_package(cmiss_scene_viewer_package),
			canvas,
			graphics_buffer_buffering_mode, graphics_buffer_stereo_mode,
			8, 8,
			8, 0);
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

