/*******************************************************************************
FILE : cmiss_scene_viewer.h

LAST MODIFIED : 9 September 2002

DESCRIPTION :
The public interface to the Cmiss_scene_viewer object for rendering cmiss
scenes.
==============================================================================*/
#ifndef __CMISS_SCENE_VIEWER_PRIVATE_H__
#define __CMISS_SCENE_VIEWER_PRIVATE_H__

#include "api/cmiss_scene_viewer.h"
#include "general/object.h"
#include "graphics/colour.h"
#include "graphics/scene.h"
#include "graphics/light.h"
#include "graphics/light_model.h"
#include "interaction/interactive_tool.h"

int Cmiss_scene_viewer_free_data(void);
/*******************************************************************************
LAST MODIFIED : 9 September 2002

DESCRIPTION:
Frees all the data used to scene viewer objects.
==============================================================================*/

int Cmiss_scene_viewer_set_data(struct Colour *background_colour,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct Interactive_tool *default_interactive_tool,
	struct MANAGER(Light) *light_manager,struct Light *default_light,
	struct MANAGER(Light_model) *light_model_manager,
	struct Light_model *default_light_model,
	struct MANAGER(Scene) *scene_manager,struct Scene *scene,
	struct MANAGER(Texture) *texture_manager,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 9 September 2002

DESCRIPTION:
Initialises all the data used to create scene viewer objects.
==============================================================================*/
#endif /* __CMISS_SCENE_VIEWER_PRIVATE_H__ */
