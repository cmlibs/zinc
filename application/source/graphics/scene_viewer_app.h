
#include "computed_field/computed_field.h"

#include "interaction/interactive_tool.h"

#define Scene_viewer_set_interactive_tool_by_name Cmiss_scene_viewer_set_interactive_tool_by_name


int Cmiss_scene_viewer_package_update_Interactive_tool(
	struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package,
	struct Interactive_tool *interactive_tool);
/*******************************************************************************
LAST MODIFIED : 26 April 2007

DESCRIPTION :
Updates the interactive tools in each of the scene_viewers created with the
<cmiss_scene_viewer_package> to have the same settings as the <interactive_tool>.
This enables the old global commands to continue to work for all scene_viewers,
however new code should probably modify the particular tools for the
particular scene_viewer intended.
==============================================================================*/


Render_graphics_opengl *Scene_viewer_rendering_data_get_renderer(
	Scene_viewer_rendering_data *rendering_data);


/***************************************************************************//**
 * Gets matrix transforming coordinate system to
 * CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FILL
 * Note this is a right-handed coordinate system with each coordinate on [-1,+1]
 * and farthest z = -1, nearest at z = +1. Compare with OpenGL normalised device
 * coordinates which reverse z so are left-handed.
 */
int Scene_viewer_get_transformation_to_window(struct Scene_viewer *scene_viewer,
	enum Cmiss_graphics_coordinate_system coordinate_system,
	gtMatrix *local_transformation_matrix, double *projection);
