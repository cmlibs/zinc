/*******************************************************************************
FILE : cmiss_scene_viewer.c

LAST MODIFIED : 10 September 2002

DESCRIPTION :
The public interface to the Cmiss_scene_viewer object for rendering cmiss
scenes.
==============================================================================*/
#include <stdarg.h>
#if defined (GTK_USER_INTERFACE)
#include <gtk/gtk.h>
#endif /* defined (GTK_USER_INTERFACE) */
#include "api/cmiss_scene_viewer.h"
#include "api/cmiss_scene_viewer_private.h"
#include "general/debug.h"
#include "graphics/scene_viewer.h"
#include "graphics/transform_tool.h"
#include "three_d_drawing/graphics_buffer.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

struct Cmiss_scene_viewer_data
/*******************************************************************************
LAST MODIFIED : 9 September 2002

DESCRIPTION:
The default data used to create Cmiss_scene_viewers.
==============================================================================*/
{
	struct Colour *background_colour;
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	struct Interactive_tool *default_interactive_tool;
	struct MANAGER(Light) *light_manager;
	struct Light *default_light;
	struct MANAGER(Light_model) *light_model_manager;
	struct Light_model *default_light_model;
	struct MANAGER(Scene) *scene_manager;
	struct Scene *scene;
	struct MANAGER(Texture) *texture_manager;
	struct User_interface *user_interface;
};

/*
Module variables
----------------
*/

static struct Cmiss_scene_viewer_data *cmiss_scene_viewer_data = NULL;

/*
Global functions
----------------
*/

int Cmiss_scene_viewer_free_data(void)
/*******************************************************************************
LAST MODIFIED : 6 September 2002

DESCRIPTION:
Frees all the data used to scene viewer objects.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_scene_viewer_free_data);
	if (cmiss_scene_viewer_data)
	{
		/* Free the previous copy */
		DEACCESS(Interactive_tool)(&cmiss_scene_viewer_data->default_interactive_tool);
		DEACCESS(Light)(&cmiss_scene_viewer_data->default_light);
		DEACCESS(Light_model)(&cmiss_scene_viewer_data->default_light_model);
		DEACCESS(Scene)(&cmiss_scene_viewer_data->scene);
		DEALLOCATE(cmiss_scene_viewer_data);
		cmiss_scene_viewer_data = (struct Cmiss_scene_viewer_data *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_free_data.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_free_data */

int Cmiss_scene_viewer_set_data(struct Colour *background_colour,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct Interactive_tool *default_interactive_tool,
	struct MANAGER(Light) *light_manager,struct Light *default_light,
	struct MANAGER(Light_model) *light_model_manager,
	struct Light_model *default_light_model,
	struct MANAGER(Scene) *scene_manager,struct Scene *scene,
	struct MANAGER(Texture) *texture_manager,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 6 September 2002

DESCRIPTION:
Initialises all the data used to scene viewer objects.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_scene_viewer_set_data);
	if (background_colour && light_manager && default_light &&
		light_model_manager && default_light_model && scene_manager
		&& scene && texture_manager && user_interface)
	{
		if (cmiss_scene_viewer_data)
		{
			/* Free the previous copy */
			Cmiss_scene_viewer_free_data();
		}
		if (ALLOCATE(cmiss_scene_viewer_data, struct Cmiss_scene_viewer_data, 1))
		{
			cmiss_scene_viewer_data->background_colour = background_colour;
			cmiss_scene_viewer_data->interactive_tool_manager = interactive_tool_manager;
			cmiss_scene_viewer_data->default_interactive_tool = ACCESS(Interactive_tool)
				(default_interactive_tool);
			cmiss_scene_viewer_data->light_manager = light_manager;
			cmiss_scene_viewer_data->default_light = ACCESS(Light)(default_light);
			cmiss_scene_viewer_data->light_model_manager = light_model_manager;
			cmiss_scene_viewer_data->default_light_model = ACCESS(Light_model)(default_light_model);
			cmiss_scene_viewer_data->scene_manager = scene_manager;
			cmiss_scene_viewer_data->scene = ACCESS(Scene)(scene);
			cmiss_scene_viewer_data->texture_manager = texture_manager;
			cmiss_scene_viewer_data->user_interface = user_interface;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_set_data.  "
				"Unable to allocate data storage");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_set_data.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_set_data */

#if defined (GTK_USER_INTERFACE)
Cmiss_scene_viewer_id create_Cmiss_scene_viewer_gtk(
	GtkContainer *scene_viewer_widget,
	enum Cmiss_scene_viewer_buffering_mode buffer_mode,
	enum Cmiss_scene_viewer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth, int specified_visual_id)
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Creates a Cmiss_scene_viewer by creating a GtkGlArea inside the specified 
<scene_viewer_widget> container.
If <minimum_colour_buffer_depth>, <minimum_depth_buffer_depth> or 
<minimum_accumulation_buffer_depth> are not zero then they are used to filter
out the possible visuals selected for graphics_buffers.  If they are zero then 
the accumulation_buffer_depth are not tested and the maximum colour buffer depth is
chosen.
If <specified_visual_id> is nonzero then this overrides all other visual
selection mechanisms and this visual will be used if possible or the create will
fail.
==============================================================================*/
{
	enum Graphics_buffer_buffering_mode graphics_buffer_buffering_mode;
	enum Graphics_buffer_stereo_mode graphics_buffer_stereo_mode;
	struct Graphics_buffer *graphics_buffer;
	struct Cmiss_scene_viewer *scene_viewer;

	ENTER(create_Cmiss_scene_viewer_gtk);
	/* Not implemented yet */
	USE_PARAMETER(minimum_colour_buffer_depth);
	USE_PARAMETER(minimum_accumulation_buffer_depth);
	USE_PARAMETER(minimum_depth_buffer_depth);
	USE_PARAMETER(specified_visual_id);
	if (cmiss_scene_viewer_data)
	{
		if (CMISS_SCENE_VIEWER_ANY_BUFFERING_MODE==buffer_mode)
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_ANY_BUFFERING_MODE;
		}
		else if (CMISS_SCENE_VIEWER_SINGLE_BUFFERING==buffer_mode)
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_SINGLE_BUFFERING;
		}
		else
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_DOUBLE_BUFFERING;
		}
		if (CMISS_SCENE_VIEWER_ANY_STEREO_MODE==stereo_mode)
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_ANY_STEREO_MODE;
		}
		else if (CMISS_SCENE_VIEWER_STEREO==stereo_mode)
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_STEREO;
		}
		else
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_MONO;
		}
		graphics_buffer = create_Graphics_buffer_gtkgl(scene_viewer_widget,
			graphics_buffer_buffering_mode, graphics_buffer_stereo_mode,
			minimum_colour_buffer_depth, minimum_depth_buffer_depth,
			minimum_accumulation_buffer_depth, specified_visual_id);
		scene_viewer = CREATE(Scene_viewer)(graphics_buffer,
			cmiss_scene_viewer_data->background_colour,
			cmiss_scene_viewer_data->light_manager,
			cmiss_scene_viewer_data->default_light,
			cmiss_scene_viewer_data->light_model_manager,
			cmiss_scene_viewer_data->default_light_model,
			cmiss_scene_viewer_data->scene_manager, cmiss_scene_viewer_data->scene,
			cmiss_scene_viewer_data->texture_manager,
			cmiss_scene_viewer_data->user_interface);
		Scene_viewer_set_interactive_tool(scene_viewer,
			cmiss_scene_viewer_data->default_interactive_tool);
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Cmiss_scene_viewer_gtk.  "
			"The Cmiss_scene_viewer data must be initialised before any scene "
			"viewers can be created.");
		scene_viewer=(struct Cmiss_scene_viewer *)NULL;
	}
	LEAVE;

	return (scene_viewer);
} /* create_Cmiss_scene_viewer_gtk */
#endif /* create_Cmiss_scene_viewer_gtk */

int Cmiss_scene_viewer_get_near_and_far_plane(Cmiss_scene_viewer_id scene_viewer,
	double *near_plane, double *far_plane)
/*******************************************************************************
LAST MODIFIED : 13 September 2002

DESCRIPTION :
Gets the distance from the eye_point to the <near> clip plane and to the <far>
clip plane in the <scene_viewer>.
==============================================================================*/
{
	double left, right, bottom, top;
	int return_code;

	ENTER(Cmiss_scene_viewer_get_near_and_far_plane);
	if (scene_viewer)
	{
		return_code = Scene_viewer_get_viewing_volume(scene_viewer,
		  &left, &right, &bottom, &top, near_plane, far_plane);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_get_near_and_far_plane.  "
			"Missing scene_viewer parameter.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_get_near_and_far_plane */

int Cmiss_scene_viewer_set_near_and_far_plane(Cmiss_scene_viewer_id scene_viewer,
	double near_plane, double far_plane)
/*******************************************************************************
LAST MODIFIED : 13 September 2002

DESCRIPTION :
Sets the distance from the eye_point to the <near> clip plane and to the <far>
clip plane in the <scene_viewer>.
==============================================================================*/
{
	double left, right, bottom, top, old_near, old_far;
	int return_code;

	ENTER(Cmiss_scene_viewer_set_near_and_far_plane);
	if (scene_viewer)
	{
		if (Scene_viewer_get_viewing_volume(scene_viewer,
			&left, &right, &bottom, &top, &old_near, &old_far))
		{
			return_code = Scene_viewer_set_viewing_volume(scene_viewer,
				left, right, bottom, top, near_plane, far_plane);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_get_near_and_far_plane.  "
			"Missing scene_viewer parameter.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_get_near_and_far_plane */

int Cmiss_scene_viewer_get_projection_mode(Cmiss_scene_viewer_id scene_viewer,
	enum Cmiss_scene_viewer_projection_mode *projection_mode)
/*******************************************************************************
LAST MODIFIED : 17 September 2002

DESCRIPTION :
Returns the projection mode - parallel/perspective - of the Cmiss_scene_viewer.
==============================================================================*/
{
	int return_code;
	enum Scene_viewer_projection_mode scene_viewer_projection_mode;

	ENTER(Cmiss_scene_viewer_get_projection_mode);
	if (scene_viewer)
	{
		return_code = Scene_viewer_get_projection_mode(scene_viewer,
			&scene_viewer_projection_mode);
		if (return_code)
		{
			switch(scene_viewer_projection_mode)
			{
				case SCENE_VIEWER_PERSPECTIVE:
				{
					*projection_mode = CMISS_SCENE_VIEWER_PERSPECTIVE;
					return_code = 1;
				} break;
				case SCENE_VIEWER_PARALLEL:
				{
					*projection_mode = CMISS_SCENE_VIEWER_PARALLEL;
					return_code = 1;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_scene_viewer_set_projection_mode.  "
						"Projection mode not supported in public interface.");
					return_code = 0;
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_scene_viewer_get_projection_mode.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_get_projection_mode */

int Cmiss_scene_viewer_set_projection_mode(Cmiss_scene_viewer_id scene_viewer,
	enum Cmiss_scene_viewer_projection_mode projection_mode)
/*******************************************************************************
LAST MODIFIED : 17 September 2002

DESCRIPTION :
Sets the projection mode - parallel/perspective/custom - of the Cmiss_scene_viewer.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_scene_viewer_set_projection_mode);
	if (scene_viewer)
	{
		switch(projection_mode)
		{
			case CMISS_SCENE_VIEWER_PERSPECTIVE:
			{
				return_code = Scene_viewer_set_projection_mode(scene_viewer, 
					SCENE_VIEWER_PERSPECTIVE);
			} break;
			case CMISS_SCENE_VIEWER_PARALLEL:
			{
				return_code = Scene_viewer_set_projection_mode(scene_viewer, 
					SCENE_VIEWER_PARALLEL);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_scene_viewer_set_projection_mode.  "
					"Unknown projection mode.");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_scene_viewer_set_projection_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_set_projection_mode */

int Cmiss_scene_viewer_get_background_colour_rgb(
	Cmiss_scene_viewer_id scene_viewer, double *red, double *green, double *blue)
/*******************************************************************************
LAST MODIFIED : 11 September 2002

DESCRIPTION :
Returns the background_colour of the scene_viewer.
==============================================================================*/
{
	int return_code;
	struct Colour colour;

	ENTER(Cmiss_scene_viewer_set_background_colour_rgb);
	if (scene_viewer)
	{
		if (return_code = Scene_viewer_get_background_colour(scene_viewer, &colour))
		{
			*red = colour.red;
			*green = colour.green;
			*blue = colour.blue;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_get_background_colour_rgb.  "
			"Missing scene_viewer parameter.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_get_background_colour_rgb */

int Cmiss_scene_viewer_set_background_colour_rgb(
	Cmiss_scene_viewer_id scene_viewer, double red, double green, double blue)
/*******************************************************************************
LAST MODIFIED : 11 September 2002

DESCRIPTION :
Sets the background_colour of the scene_viewer.
==============================================================================*/
{
	int return_code;
	struct Colour colour;

	ENTER(Cmiss_scene_viewer_set_background_colour_rgb);
	if (scene_viewer)
	{
		colour.red = red;
		colour.green = green;
		colour.blue = blue;
		return_code = Scene_viewer_set_background_colour(scene_viewer, &colour);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_set_background_colour_rgb.  "
			"Missing scene_viewer parameter.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_set_background_colour_rgb */

int Cmiss_scene_viewer_get_interactive_tool_name(
	Cmiss_scene_viewer_id scene_viewer, char **tool_name)
/*******************************************************************************
LAST MODIFIED : 11 September 2002

DESCRIPTION :
Returns an ALLOCATED string which specifies the name of the current
interactive_tool.  You should call Cmiss_deallocate with the returned
pointer when it is no longer required.
==============================================================================*/
{
	struct Interactive_tool *interactive_tool;
	int return_code;

	ENTER(Cmiss_scene_viewer_get_interactive_tool_name);
	if (scene_viewer)
	{
		if ((interactive_tool = Scene_viewer_get_interactive_tool(scene_viewer))
			&&(GET_NAME(Interactive_tool)(interactive_tool, tool_name)))
		{
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_get_interactive_tool_name.  "
				"Failed to get the tool or tool name.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_get_background_colour_rgb.  "
			"Missing scene_viewer parameter.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_get_interactive_tool_name */

int Cmiss_scene_viewer_set_interactive_tool_by_name(
	Cmiss_scene_viewer_id scene_viewer, char *tool_name)
/*******************************************************************************
LAST MODIFIED : 11 September 2002

DESCRIPTION :
Sets the currently active interactive tool for the scene_viewer if one that
matches the <tool_name> exists.
==============================================================================*/
{
	struct Interactive_tool *interactive_tool;
	int return_code;

	ENTER(Cmiss_scene_viewer_set_interactive_tool_by_name);
	if (cmiss_scene_viewer_data)
	{
		if (interactive_tool=
			FIND_BY_IDENTIFIER_IN_MANAGER(Interactive_tool,name)(
				tool_name,cmiss_scene_viewer_data->interactive_tool_manager))
		{
			if (Interactive_tool_is_Transform_tool(interactive_tool))
			{
				Scene_viewer_set_input_mode(scene_viewer,SCENE_VIEWER_TRANSFORM);
			}
			else
			{
				Scene_viewer_set_input_mode(scene_viewer,SCENE_VIEWER_SELECT);
			}
			return_code = Scene_viewer_set_interactive_tool(scene_viewer,
				interactive_tool);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_set_interactive_tool_by_name.  "
				"Unable to find an interactive tool named %s.", tool_name);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_set_interactive_tool_by_name.  "
			"The Cmiss_scene_viewer data must be initialised before using "
			"the scene_viewer api.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_set_interactive_tool_by_name */

int Cmiss_scene_viewer_get_scene_name(
	Cmiss_scene_viewer_id scene_viewer, char **scene_name)
/*******************************************************************************
LAST MODIFIED : 13 September 2002

DESCRIPTION :
Returns an ALLOCATED string which identifies the scene currently rendered
by the <scene_viewer>.  You should call Cmiss_deallocate with the returned
pointer when it is no longer required.
==============================================================================*/
{
	struct Scene *scene;
	int return_code;

	ENTER(Cmiss_scene_viewer_get_scene_name);
	if (scene_viewer)
	{
		if ((scene = Scene_viewer_get_scene(scene_viewer))
			&&(GET_NAME(Scene)(scene, scene_name)))
		{
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_get_scene_name.  "
				"Failed to get the scene or scene name.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_get_background_colour_rgb.  "
			"Missing scene_viewer parameter.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_get_scene_name */

int Cmiss_scene_viewer_set_scene_by_name(Cmiss_scene_viewer_id scene_viewer,
	char *scene_name)
/*******************************************************************************
LAST MODIFIED : 13 September 2002

DESCRIPTION :
Sets the currently scene rendered in the scene_viewer if one that
matches the <scene_name> exists.
==============================================================================*/
{
	struct Scene *scene;
	int return_code;

	ENTER(Cmiss_scene_viewer_set_scene_by_name);
	if (cmiss_scene_viewer_data)
	{
		if (scene=FIND_BY_IDENTIFIER_IN_MANAGER(Scene,name)(
				scene_name,cmiss_scene_viewer_data->scene_manager))
		{
			return_code = Scene_viewer_set_scene(scene_viewer, scene);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_set_scene_by_name.  "
				"Unable to find a scene named %s.", scene_name);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_set_scene_by_name.  "
			"The Cmiss_scene_viewer data must be initialised before using "
			"the scene_viewer api.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_set_scene_by_name */

int Cmiss_scene_viewer_set_overlay_scene_by_name(Cmiss_scene_viewer_id scene_viewer,
	char *scene_name)
/*******************************************************************************
LAST MODIFIED : 10 September 2003

DESCRIPTION :
Sets the overlay scene rendered in the scene_viewer if one that
matches the <scene_name> exists.
==============================================================================*/
{
	struct Scene *scene;
	int return_code;

	ENTER(Cmiss_scene_viewer_set_scene_by_name);
	if (cmiss_scene_viewer_data)
	{
		if (scene=FIND_BY_IDENTIFIER_IN_MANAGER(Scene,name)(
				scene_name,cmiss_scene_viewer_data->scene_manager))
		{
			return_code = Scene_viewer_set_overlay_scene(scene_viewer, scene);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_set_scene_by_name.  "
				"Unable to find a scene named %s.", scene_name);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_set_scene_by_name.  "
			"The Cmiss_scene_viewer data must be initialised before using "
			"the scene_viewer api.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_set_scene_by_name */

int Cmiss_scene_viewer_set_background_texture_by_name(Cmiss_scene_viewer_id scene_viewer,
	char *texture_name)
/*******************************************************************************
LAST MODIFIED : 10 September 2003

DESCRIPTION :
Sets the background texture rendered in the scene_viewer if one that
matches the <texture_name> exists.
==============================================================================*/
{
	struct Texture *texture;
	int return_code;

	ENTER(Cmiss_scene_viewer_set_texture_by_name);
	if (cmiss_scene_viewer_data)
	{
		if (texture=FIND_BY_IDENTIFIER_IN_MANAGER(Texture,name)(
			texture_name,cmiss_scene_viewer_data->texture_manager))
		{
			return_code = Scene_viewer_set_background_texture(scene_viewer, texture);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_set_texture_by_name.  "
				"Unable to find a texture named %s.", texture_name);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_set_texture_by_name.  "
			"The Cmiss_scene_viewer data must be initialised before using "
			"the scene_viewer api.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_set_texture_by_name */

int Cmiss_scene_viewer_write_image_to_file(Cmiss_scene_viewer_id scene_viewer,
	char *file_name, int force_onscreen, int preferred_width, int preferred_height,
	int preferred_antialias, int preferred_transparency_layers)
/*******************************************************************************
LAST MODIFIED : 18 September 2002

DESCRIPTION :
Writes the view in the scene_viewer to the specified filename.
==============================================================================*/
{
	enum Texture_storage_type storage;
	int return_code;
	struct Cmgui_image *cmgui_image;
	struct Cmgui_image_information *cmgui_image_information;

	ENTER(Cmiss_scene_viewer_write_image_to_file);
	if (scene_viewer && file_name)
	{
		storage = TEXTURE_RGBA;
		if (cmgui_image = Scene_viewer_get_image(scene_viewer,
			force_onscreen, preferred_width, preferred_height, preferred_antialias,
			preferred_transparency_layers, storage))
		{
			cmgui_image_information = CREATE(Cmgui_image_information)();
			Cmgui_image_information_add_file_name(cmgui_image_information,
				file_name);
			Cmgui_image_write(cmgui_image, cmgui_image_information);
			DESTROY(Cmgui_image_information)(&cmgui_image_information);
			DESTROY(Cmgui_image)(&cmgui_image);
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_write_image_to_file.  "
			"Invalid scene_viewer or file name.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_write_image_to_file */
