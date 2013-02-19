
#include "general/debug.h"
#include "general/message.h"
#include "graphics/scene_viewer.h"
#include "graphics/scene_viewer_app.h"
#include "three_d_drawing/graphics_buffer.h"
#include "three_d_drawing/graphics_buffer_app.h"
#include "general/list_private.h"
#include "general/callback_private.h"
#include "graphics/transform_tool.h"
#include "user_interface/event_dispatcher.h"
#include "graphics/graphics_library.h"
#include "graphics/graphics_coordinate_system.hpp"

int Scene_viewer_app_input_select(struct Scene_viewer_app *scene_viewer,
	struct Graphics_buffer_input *input);

int Scene_viewer_app_default_input_callback(struct Scene_viewer_app *scene_viewer,
	struct Graphics_buffer_input *input, void *dummy_void);

void Scene_viewer_app_initialise_callback(struct Graphics_buffer_app *graphics_buffer,
	void *dummy_void, void *scene_viewer_void);

void Scene_viewer_app_resize_callback(struct Graphics_buffer_app *graphics_buffer,
	void *dummy_void, void *scene_viewer_void);

void Scene_viewer_app_expose_callback(struct Graphics_buffer_app *graphics_buffer,
	void *expose_data_void, void *scene_viewer_void);

void Scene_viewer_app_graphics_buffer_input_callback(
	struct Graphics_buffer_app *graphics_buffer,
	struct Graphics_buffer_input *input, void *scene_viewer_void);

FULL_DECLARE_LIST_TYPE(Scene_viewer_app);

// Callback functions:
FULL_DECLARE_CMISS_CALLBACK_TYPES(Cmiss_scene_viewer_app_package_callback, \
	struct Cmiss_scene_viewer_app_package *, void *);

FULL_DECLARE_CMISS_CALLBACK_TYPES(Scene_viewer_app_callback, \
	struct Scene_viewer_app *, void *);

FULL_DECLARE_CMISS_CALLBACK_TYPES(Scene_viewer_app_input_callback,
	struct Scene_viewer_app *, struct Graphics_buffer_input *);

DEFINE_CMISS_CALLBACK_MODULE_FUNCTIONS(Cmiss_scene_viewer_app_package_callback, void);

DEFINE_CMISS_CALLBACK_MODULE_FUNCTIONS(Scene_viewer_app_callback, void);

DEFINE_CMISS_CALLBACK_MODULE_FUNCTIONS(Scene_viewer_app_input_callback, int);

DEFINE_CMISS_CALLBACK_FUNCTIONS(Cmiss_scene_viewer_app_package_callback, \
	struct Cmiss_scene_viewer_app_package *,void *);

DEFINE_CMISS_CALLBACK_FUNCTIONS(Scene_viewer_app_callback, \
	struct Scene_viewer_app *,void *)

DEFINE_CMISS_CALLBACK_FUNCTIONS(Scene_viewer_app_input_callback,
	struct Scene_viewer_app *,struct Graphics_buffer_input *);

struct Cmiss_scene_viewer_app_package *CREATE(Cmiss_scene_viewer_app_package)(
	struct Graphics_buffer_app_package *graphics_buffer_package,
	struct Colour *background_colour,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct MANAGER(Light) *light_manager,struct Light *default_light,
	struct MANAGER(Light_model) *light_model_manager,
	struct Light_model *default_light_model,
	struct MANAGER(Scene) *scene_manager,struct Scene *scene,
	struct User_interface *user_interface)
{
	struct Cmiss_scene_viewer_app_package *scene_viewer_package;
	if (graphics_buffer_package&&background_colour&&default_light_model&&scene&&
		user_interface&&graphics_buffer_package&&interactive_tool_manager)
	{
		/* allocate memory for the scene_viewer structure */
		if (ALLOCATE(scene_viewer_package,struct Cmiss_scene_viewer_app_package,1))
		{
			scene_viewer_package->access_count = 1;
			scene_viewer_package->graphics_buffer_package = graphics_buffer_package;
			scene_viewer_package->core_scene_viewer_package = CREATE(Cmiss_scene_viewer_package)(background_colour,
				interactive_tool_manager, light_manager, default_light, light_model_manager, default_light_model, scene_manager, scene);
			scene_viewer_package->user_interface = user_interface;
			scene_viewer_package->scene_viewer_app_list = CREATE(LIST(Scene_viewer_app))();
			scene_viewer_package->destroy_callback_list=
					CREATE(LIST(CMISS_CALLBACK_ITEM(Cmiss_scene_viewer_app_package_callback)))();
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Scene_viewer_package).  Not enough memory for scene_viewer");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Scene_viewer_package).  Invalid argument(s)");
		scene_viewer_package = 0;
	}
	LEAVE;

	return (scene_viewer_package);
} /* CREATE(Cmiss_scene_viewer_package) */

int DESTROY(Cmiss_scene_viewer_app_package)(
	struct Cmiss_scene_viewer_app_package **scene_viewer_app_package_address)
{
	int return_code = 0;
	struct Cmiss_scene_viewer_app_package *scene_viewer_app_package = 0;
	if (scene_viewer_app_package_address &&
		( 0 != (scene_viewer_app_package = *scene_viewer_app_package_address)))
	{
		return_code = 1;
		/* send the destroy callbacks */
		if (scene_viewer_app_package->destroy_callback_list)
		{
			CMISS_CALLBACK_LIST_CALL(Cmiss_scene_viewer_app_package_callback)(
				scene_viewer_app_package->destroy_callback_list,scene_viewer_app_package,NULL);
			DESTROY( LIST(CMISS_CALLBACK_ITEM(Cmiss_scene_viewer_app_package_callback)))(
				&scene_viewer_app_package->destroy_callback_list);
		}
		if (scene_viewer_app_package->core_scene_viewer_package)
		{
			Cmiss_scene_viewer_package_destroy(&scene_viewer_app_package->core_scene_viewer_package);
		}
		if (scene_viewer_app_package->scene_viewer_app_list)
		{
			DESTROY(LIST(Scene_viewer_app))(
				&scene_viewer_app_package->scene_viewer_app_list);
		}
		DEALLOCATE(*scene_viewer_app_package_address);
	}

	return return_code;
}

void My_Cmiss_scene_viewer_callback(Cmiss_scene_viewer_id /* scene_viewer */,
		void * /*callback_data*/, void *user_data)
{
	Scene_viewer_app_redraw((struct Scene_viewer_app *)user_data);
}

struct Scene_viewer_app *CREATE(Scene_viewer_app)(struct Graphics_buffer_app *graphics_buffer,
	struct Colour *background_colour,
	struct MANAGER(Light) *light_manager,struct Light *default_light,
	struct MANAGER(Light_model) *light_model_manager,
	struct Light_model *default_light_model,
	struct MANAGER(Scene) *scene_manager, struct Scene *scene,
	struct User_interface *user_interface)
{
	struct Scene_viewer_app *scene_viewer = 0;
	if (graphics_buffer)
	{
		if (ALLOCATE(scene_viewer, Scene_viewer_app, 1))
		{
			scene_viewer->access_count = 1;
			scene_viewer->graphics_buffer = ACCESS(Graphics_buffer_app)(graphics_buffer);
			scene_viewer->core_scene_viewer = CREATE(Scene_viewer)(Graphics_buffer_app_get_core_buffer(graphics_buffer),
				background_colour,
				light_manager,
				default_light,
				light_model_manager,
				default_light_model,
				scene_manager, scene);
			scene_viewer->user_interface = user_interface;
			scene_viewer->idle_update_callback_id = (struct Event_dispatcher_idle_callback *)NULL;
			/* no current interactive_tool */
			scene_viewer->interactive_tool=(struct Interactive_tool *)NULL;
			/* Currently only set when created from a Cmiss_scene_viewer_package
				to avoid changing the interface */
			scene_viewer->interactive_tool_manager = 0;
			scene_viewer->input_callback_list=
				CREATE(LIST(CMISS_CALLBACK_ITEM(Scene_viewer_app_input_callback)))();
			/* Add the default callback */
			CMISS_CALLBACK_LIST_ADD_CALLBACK(Scene_viewer_app_input_callback)(
				scene_viewer->input_callback_list,
				Scene_viewer_app_default_input_callback,NULL);
			scene_viewer->sync_callback_list=
				CREATE(LIST(CMISS_CALLBACK_ITEM(Scene_viewer_app_callback)))();
			scene_viewer->transform_callback_list=
				CREATE(LIST(CMISS_CALLBACK_ITEM(Scene_viewer_app_callback)))();
			scene_viewer->destroy_callback_list=
				CREATE(LIST(CMISS_CALLBACK_ITEM(Scene_viewer_app_callback)))();
			Cmiss_scene_viewer_add_repaint_required_callback(scene_viewer->core_scene_viewer, My_Cmiss_scene_viewer_callback, scene_viewer);
//			scene_viewer->repaint_required_callback_list=
//				CREATE(LIST(CMISS_CALLBACK_ITEM(Scene_viewer_callback)))();
			Graphics_buffer_app_add_initialise_callback(graphics_buffer,
				Scene_viewer_app_initialise_callback, scene_viewer);
			Graphics_buffer_app_add_resize_callback(graphics_buffer,
				Scene_viewer_app_resize_callback, scene_viewer);
			Graphics_buffer_app_add_expose_callback(graphics_buffer,
				Scene_viewer_app_expose_callback, scene_viewer);
			Graphics_buffer_app_add_input_callback(graphics_buffer,
				Scene_viewer_app_graphics_buffer_input_callback, scene_viewer);
		}
	}

	return (scene_viewer);
}

int DESTROY(Scene_viewer_app)(struct Scene_viewer_app **scene_viewer_app_address)
{
	int return_code = 0;
	struct Scene_viewer_app *scene_viewer = 0;
	if (scene_viewer_app_address && (scene_viewer = *scene_viewer_app_address))
	{
		return_code = 1;
		DESTROY(Scene_viewer)(&(scene_viewer->core_scene_viewer));
		/* send the destroy callbacks */
		if (scene_viewer->destroy_callback_list)
		{
			CMISS_CALLBACK_LIST_CALL(Scene_viewer_app_callback)(
				scene_viewer->destroy_callback_list,scene_viewer,NULL);
			DESTROY( LIST(CMISS_CALLBACK_ITEM(Scene_viewer_app_callback)))(
				&scene_viewer->destroy_callback_list);
		}
		if (scene_viewer->sync_callback_list)
		{
			DESTROY(LIST(CMISS_CALLBACK_ITEM(Scene_viewer_app_callback)))(
				&scene_viewer->sync_callback_list);
		}
		if (scene_viewer->transform_callback_list)
		{
			DESTROY(LIST(CMISS_CALLBACK_ITEM(Scene_viewer_app_callback)))(
				&scene_viewer->transform_callback_list);
		}
		if (scene_viewer->input_callback_list)
		{
			DESTROY(LIST(CMISS_CALLBACK_ITEM(Scene_viewer_app_input_callback)))(
				&scene_viewer->input_callback_list);
		}
		if (scene_viewer->graphics_buffer)
		{
			DEACCESS(Graphics_buffer_app)(&scene_viewer->graphics_buffer);
		}
		if (scene_viewer->interactive_tool_manager)
		{
			DESTROY(MANAGER(Interactive_tool))(&scene_viewer->interactive_tool_manager);
		}
		DEALLOCATE(*scene_viewer_app_address);
	}

	return return_code;
}

struct Scene_viewer_app *ACCESS(Scene_viewer_app)(struct Scene_viewer_app *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 19 January 2007

DESCRIPTION :
==============================================================================*/
{
	//Do nothing as the scene viewer removes itself from the package list
	return(scene_viewer);
}

int DEACCESS(Scene_viewer_app)(struct Scene_viewer_app **scene_viewer_address)
/*******************************************************************************
LAST MODIFIED : 19 January 2007

DESCRIPTION :
==============================================================================*/
{
	//Do nothing as the scene viewer removes itself from the package list
	*scene_viewer_address = (struct Scene_viewer_app *)NULL;
	return(1);
}

DECLARE_LIST_FUNCTIONS(Scene_viewer_app)

void Scene_viewer_app_destroy_remove_from_package(
	struct Scene_viewer_app *scene_viewer,
	void *dummy_void, void *package_void)
/*******************************************************************************
LAST MODIFIED : 19 April 2007

DESCRIPTION :
==============================================================================*/
{
	struct Cmiss_scene_viewer_app_package *package = (struct Cmiss_scene_viewer_app_package *)package_void;

	USE_PARAMETER(dummy_void);
	if (scene_viewer && package)
	{
		REMOVE_OBJECT_FROM_LIST(Scene_viewer_app)(scene_viewer,
			package->scene_viewer_app_list);
	}
}

int Scene_viewer_app_remove_destroy_callback(struct Scene_viewer_app *scene_viewer,
	CMISS_CALLBACK_FUNCTION(Scene_viewer_app_callback) *function,void *user_data);


int Scene_viewer_app_add_destroy_callback(struct Scene_viewer_app *scene_viewer,
	CMISS_CALLBACK_FUNCTION(Scene_viewer_app_callback) *function,void *user_data);

int Scene_viewer_app_destroy_from_package(
	struct Scene_viewer_app *scene_viewer, void *package_void)
/*******************************************************************************
LAST MODIFIED : 19 April 2007

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Cmiss_scene_viewer_app_package *package = (struct Cmiss_scene_viewer_app_package *)package_void;

	if (scene_viewer && package)
	{
		Scene_viewer_app_remove_destroy_callback(scene_viewer,
			Scene_viewer_app_destroy_remove_from_package, package);
		printf("scene viewer app destroying with the package.\n");
		DESTROY(Scene_viewer_app)(&scene_viewer);
	}
	return_code = 1;

	return (return_code);
}

struct Scene_viewer_app *create_Scene_viewer_app_from_package(
	struct Graphics_buffer_app *graphics_buffer,
	struct Cmiss_scene_viewer_app_package *cmiss_scene_viewer_package,
	struct Scene *scene)
{
	struct MANAGER(Interactive_tool) *new_interactive_tool_manager;
	struct Scene_viewer_app *scene_viewer = 0;

	if (graphics_buffer && cmiss_scene_viewer_package && scene)
	{
		scene_viewer = CREATE(Scene_viewer_app)(graphics_buffer,
			cmiss_scene_viewer_package->core_scene_viewer_package->background_colour,
			cmiss_scene_viewer_package->core_scene_viewer_package->light_manager,
			cmiss_scene_viewer_package->core_scene_viewer_package->default_light,
			cmiss_scene_viewer_package->core_scene_viewer_package->light_model_manager,
			cmiss_scene_viewer_package->core_scene_viewer_package->default_light_model,
			cmiss_scene_viewer_package->core_scene_viewer_package->scene_manager,
			scene, cmiss_scene_viewer_package->user_interface);

		new_interactive_tool_manager = 0;//--CREATE(MANAGER(Interactive_tool))();
		//--FOR_EACH_OBJECT_IN_MANAGER(Interactive_tool)(
		//--	Interactive_tool_create_copy_iterator, new_interactive_tool_manager,
		//--	cmiss_scene_viewer_package->interactive_tool_manager);

		//--Scene_viewer_set_interactive_tool(scene_viewer,
		//--	FIND_BY_IDENTIFIER_IN_MANAGER(Interactive_tool,name)
		//--	("transform_tool", new_interactive_tool_manager));
		scene_viewer->interactive_tool_manager = new_interactive_tool_manager;

		/* Add this scene_viewer to the package list */
		ADD_OBJECT_TO_LIST(Scene_viewer_app)(scene_viewer,
			cmiss_scene_viewer_package->scene_viewer_app_list);

		/* Register a callback so that if the scene_viewer is destroyed
			then it is removed from the list */
		Scene_viewer_app_add_destroy_callback(scene_viewer,
			Scene_viewer_app_destroy_remove_from_package,
			cmiss_scene_viewer_package);
	}

	return scene_viewer;
}

int Scene_viewer_app_start_freespin(struct Scene_viewer_app *scene_viewer,
	double *tumble_axis, double tumble_angle)
/*******************************************************************************
LAST MODIFIED : 10 September 2003

DESCRIPTION :
Sets the <scene_viewer> spinning in idle time.  The <tumble_axis> is the vector
about which the scene is turning relative to its lookat point and the
<tumble_angle> controls how much it turns on each redraw.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_start_freespin);
	if (scene_viewer && tumble_axis)
	{
		scene_viewer->core_scene_viewer->tumble_active = 1;
		scene_viewer->core_scene_viewer->tumble_axis[0] = tumble_axis[0];
		scene_viewer->core_scene_viewer->tumble_axis[1] = tumble_axis[1];
		scene_viewer->core_scene_viewer->tumble_axis[2] = tumble_axis[2];
		scene_viewer->core_scene_viewer->tumble_angle = tumble_angle;
		/* Repost the idle callback */
		if(!scene_viewer->idle_update_callback_id)
		{
			scene_viewer->idle_update_callback_id = Event_dispatcher_add_idle_callback(
				User_interface_get_event_dispatcher(scene_viewer->user_interface),
				Scene_viewer_app_idle_update_callback, (void *)scene_viewer,
				EVENT_DISPATCHER_IDLE_UPDATE_SCENE_VIEWER_PRIORITY);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_start_freespin.  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_start_freespin */

int Scene_viewer_app_sleep(struct Scene_viewer_app *scene_viewer)
{
	int return_code = 1;
	if (scene_viewer)
	{
		if (scene_viewer->idle_update_callback_id)
		{
			Event_dispatcher_remove_idle_callback(
				User_interface_get_event_dispatcher(scene_viewer->user_interface),
				scene_viewer->idle_update_callback_id);
			scene_viewer->idle_update_callback_id=(struct Event_dispatcher_idle_callback *)NULL;
		}
		return_code = Scene_viewer_sleep(scene_viewer->core_scene_viewer);
	}

	return return_code;
}

struct Interactive_tool *Scene_viewer_app_get_interactive_tool(
	struct Scene_viewer_app *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 11 April 2000

DESCRIPTION :
Returns the interactive_tool used by the Scene_viewer.
The interactive_tool may be NULL, indicating that no tool is in use.
==============================================================================*/
{
	struct Interactive_tool *interactive_tool;

	ENTER(Scene_viewer_get_interactive_tool);
	if (scene_viewer)
	{
		interactive_tool=scene_viewer->interactive_tool;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_interactive_tool.  Invalid argument(s)");
		interactive_tool=(struct Interactive_tool *)NULL;
	}
	LEAVE;

	return (interactive_tool);
} /* Scene_viewer_get_interactive_tool */

int Scene_viewer_app_set_interactive_tool(struct Scene_viewer_app *scene_viewer,
	struct Interactive_tool *interactive_tool)
/*******************************************************************************
LAST MODIFIED : 25 February 2008

DESCRIPTION :
Sets the interactive tool that will receive input if the Scene_viewer is in
SCENE_VIEWER_SELECT mode. A NULL value indicates no tool.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_interactive_tool);
	if (scene_viewer)
	{
		if (scene_viewer->interactive_tool &&
			(scene_viewer->interactive_tool != interactive_tool))
		{
			Interactive_tool_reset(scene_viewer->interactive_tool);
		}
		scene_viewer->interactive_tool=interactive_tool;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_interactive_tool.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_interactive_tool */

int Scene_viewer_get_opengl_information(struct Scene_viewer_app *scene_viewer,
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
		Graphics_buffer_app_make_current(scene_viewer->graphics_buffer);
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
		Graphics_buffer_get_visual_id(scene_viewer->graphics_buffer, visual_id);
		Graphics_buffer_get_colour_buffer_depth(scene_viewer->graphics_buffer,
			colour_buffer_depth);
		Graphics_buffer_get_depth_buffer_depth(scene_viewer->graphics_buffer,
			depth_buffer_depth);
		Graphics_buffer_get_accumulation_buffer_depth(scene_viewer->graphics_buffer,
			accumulation_buffer_depth);

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

int Scene_viewer_app_update_Interactive_tool(
	struct Scene_viewer_app *scene_viewer, void *interactive_tool_void)
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
		&& (scene_viewer_interactive_tool= //0))
			FIND_BY_IDENTIFIER_IN_MANAGER(Interactive_tool,name)(
			(char *)tool_name,scene_viewer->interactive_tool_manager)))
	{
		Interactive_tool_copy(scene_viewer_interactive_tool,
			global_interactive_tool, (struct MANAGER(Interactive_tool) *)NULL);
	}
	return_code = 1;
	DEALLOCATE(tool_name);
	return (return_code);
}

int Cmiss_scene_viewer_package_update_Interactive_tool(
	struct Cmiss_scene_viewer_app_package *cmiss_scene_viewer_package,
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
		return_code = FOR_EACH_OBJECT_IN_LIST(Scene_viewer_app)(
			Scene_viewer_app_update_Interactive_tool, (void *)interactive_tool,
			cmiss_scene_viewer_package->scene_viewer_app_list);
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

int Scene_viewer_automatic_tumble(struct Scene_viewer_app *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 28 September 2000

DESCRIPTION :
Rotates the scene_viewer when the tumble is active.
==============================================================================*/
{
	double centre_x,centre_y,size_x,size_y,viewport_bottom,viewport_height,
		viewport_left,viewport_width;
	enum Interactive_event_type interactive_event_type;
	int i,j,return_code;
	GLdouble temp_modelview_matrix[16], temp_projection_matrix[16];
	GLint viewport[4];
	struct Interactive_event *interactive_event;
	struct Interaction_volume *interaction_volume;

	ENTER(Scene_viewer_automatic_tumble);
	if (scene_viewer)
	{
		if (scene_viewer->core_scene_viewer->tumble_active)
		{
			Scene_viewer_rotate_about_lookat_point(scene_viewer->core_scene_viewer,
				scene_viewer->core_scene_viewer->tumble_axis,
				scene_viewer->core_scene_viewer->tumble_angle);
			CMISS_CALLBACK_LIST_CALL(Scene_viewer_app_callback)(
				scene_viewer->sync_callback_list,scene_viewer,NULL);

			if (scene_viewer->interactive_tool)
			{
				glGetIntegerv(GL_VIEWPORT,viewport);
				viewport_left   = (double)(viewport[0]);
				viewport_bottom = (double)(viewport[1]);
				viewport_width  = (double)(viewport[2]);
				viewport_height = (double)(viewport[3]);

				/*???RC*//*Scene_viewer_calculate_transformation(scene_viewer,
					viewport_width,viewport_height);*/

				size_x = SCENE_VIEWER_PICK_SIZE;
				size_y = SCENE_VIEWER_PICK_SIZE;

				centre_x=(double)(scene_viewer->core_scene_viewer->previous_pointer_x);
				/* flip y as x event has y=0 at top of window, increasing down */
				centre_y=viewport_height-(double)(scene_viewer->core_scene_viewer->previous_pointer_y)-1.0;

				/* Update the interaction volume */
				interactive_event_type=INTERACTIVE_EVENT_MOTION_NOTIFY;
				for (i=0;i<4;i++)
				{
					for (j=0;j<4;j++)
					{
						temp_modelview_matrix[i*4+j] =
							scene_viewer->core_scene_viewer->modelview_matrix[j*4+i];
						temp_projection_matrix[i*4+j] =
							scene_viewer->core_scene_viewer->window_projection_matrix[j*4+i];
					}
				}
				interaction_volume=create_Interaction_volume_ray_frustum(
					temp_modelview_matrix,temp_projection_matrix,
					viewport_left,viewport_bottom,viewport_width,viewport_height,
					centre_x,centre_y,size_x,size_y);
				ACCESS(Interaction_volume)(interaction_volume);
				interactive_event=CREATE(Interactive_event)(interactive_event_type,
					/*button_number*/-1,/*input_modifier*/0,interaction_volume,
					scene_viewer->core_scene_viewer->scene);
				ACCESS(Interactive_event)(interactive_event);
				//--Interactive_tool_handle_interactive_event(
				//--	scene_viewer->interactive_tool,(void *)scene_viewer,
				//--	interactive_event, scene_viewer->graphics_buffer);
				DEACCESS(Interactive_event)(&interactive_event);
				DEACCESS(Interaction_volume)(&interaction_volume);
			}
		}
		else
		{
			scene_viewer->core_scene_viewer->tumble_angle = 0.0;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_automatic_tumble.  Missing scene_viewer");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_automatic_tumble */

void Scene_viewer_app_set_transform_flag(struct Scene_viewer_app *scene_viewer)
{
	if (scene_viewer)
	{
		scene_viewer->core_scene_viewer->transform_flag = 1;
		CMISS_CALLBACK_LIST_CALL(Scene_viewer_app_callback)(scene_viewer->transform_callback_list,
			scene_viewer, NULL);
	}
}

int Scene_viewer_app_input_transform(struct Scene_viewer_app *scene_viewer_app,
	struct Graphics_buffer_input *input)
{
	//ALAN: This can be done properly with proper APIs registering and getting the state
	// of tumble.
	if (scene_viewer_app && scene_viewer_app->core_scene_viewer && input)
	{
		switch (input->type)
		{
			case CMISS_SCENE_VIEWER_INPUT_BUTTON_RELEASE:
			{
				if ((scene_viewer_app->core_scene_viewer->drag_mode == SV_DRAG_TUMBLE) &&
					scene_viewer_app->core_scene_viewer->tumble_angle)
				{
					scene_viewer_app->core_scene_viewer->tumble_active = 1;
					if (!scene_viewer_app->idle_update_callback_id)
					 {
						scene_viewer_app->idle_update_callback_id = Event_dispatcher_add_idle_callback(
								User_interface_get_event_dispatcher(scene_viewer_app->user_interface),
								Scene_viewer_app_idle_update_callback, (void *)scene_viewer_app,
								EVENT_DISPATCHER_IDLE_UPDATE_SCENE_VIEWER_PRIORITY);
					 }
				}
			} break;
			default:
			{
			} break;
		}
		return 1;
	}

	return 0;
} /* Scene_viewer_app_input_transform */

int Scene_viewer_app_default_input_callback(struct Scene_viewer_app *scene_viewer,
	struct Graphics_buffer_input *input, void *dummy_void)
{
	int return_code = 0;

	USE_PARAMETER(dummy_void);

	if (scene_viewer)
	{
		return_code = 1;
		switch (scene_viewer->core_scene_viewer->input_mode)
		{
		case SCENE_VIEWER_NO_INPUT_OR_DRAW:
		case SCENE_VIEWER_NO_INPUT:
		{
			/* do nothing */
		} break;
		case SCENE_VIEWER_SELECT:
		{
			/* can override select mode by holding down control key */
			if (CMISS_SCENE_VIEWER_INPUT_BUTTON_PRESS==input->type)
			{
				if (((CMISS_SCENE_VIEWER_INPUT_MODIFIER_CONTROL & input->input_modifier)&&
					(SCENE_VIEWER_ABSOLUTE_VIEWPORT != scene_viewer->core_scene_viewer->viewport_mode))
					||((SCENE_VIEWER_ABSOLUTE_VIEWPORT == scene_viewer->core_scene_viewer->viewport_mode)&&
					!((1==input->button_number)||
					(CMISS_SCENE_VIEWER_INPUT_MODIFIER_BUTTON1 & input->input_modifier))))
				{
					scene_viewer->core_scene_viewer->temporary_transform_mode=1;
				}
				else
				{
					scene_viewer->core_scene_viewer->temporary_transform_mode=0;
				}
			}
			if (scene_viewer->core_scene_viewer->temporary_transform_mode)
			{
				if (SCENE_VIEWER_RELATIVE_VIEWPORT == scene_viewer->core_scene_viewer->viewport_mode ||
					SCENE_VIEWER_DISTORTING_RELATIVE_VIEWPORT == scene_viewer->core_scene_viewer->viewport_mode)
				{
					if (SCENE_VIEWER_CUSTOM != scene_viewer->core_scene_viewer->projection_mode)
					{
						Scene_viewer_app_input_transform(scene_viewer, input);
						Scene_viewer_input_transform(scene_viewer->core_scene_viewer, input);
					}
				}
				else
				{
					Cmiss_scene_viewer_input_viewport_transform(scene_viewer->core_scene_viewer, input);
				}
			}
			else
			{
				/*???RC temporary until all tools are Interactive_tools */
				if (scene_viewer->interactive_tool)
				{
					Scene_viewer_app_input_select(scene_viewer, input);
					//display_message(ERROR_MESSAGE,
					//	"Scene_viewer_default_input_callback.  Input selection not implemented.");
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Scene_viewer_default_input_callback.  Always need an interactive tool");
				}
			}
		} break;
		case SCENE_VIEWER_UPDATE_ON_CLICK:
		case SCENE_VIEWER_TRANSFORM:
		{
			if (SCENE_VIEWER_UPDATE_ON_CLICK==scene_viewer->core_scene_viewer->input_mode)
			{
				if (CMISS_SCENE_VIEWER_INPUT_BUTTON_PRESS==input->type)
				{
					if (input->input_modifier & CMISS_SCENE_VIEWER_INPUT_MODIFIER_CONTROL)
					{
						Scene_viewer_view_all(scene_viewer->core_scene_viewer);
					}
				}
				scene_viewer->core_scene_viewer->update_pixel_image=1;
				Scene_viewer_app_redraw(scene_viewer);
			}
			if (SCENE_VIEWER_RELATIVE_VIEWPORT==scene_viewer->core_scene_viewer->viewport_mode ||
				SCENE_VIEWER_DISTORTING_RELATIVE_VIEWPORT==scene_viewer->core_scene_viewer->viewport_mode)
			{
				if (SCENE_VIEWER_CUSTOM != scene_viewer->core_scene_viewer->projection_mode)
				{
					Scene_viewer_app_input_transform(scene_viewer, input);
					Scene_viewer_input_transform(scene_viewer->core_scene_viewer, input);
				}
			}
			else
			{
				Cmiss_scene_viewer_input_viewport_transform(scene_viewer->core_scene_viewer, input);
			}
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_default_input_callback.  Invalid input mode");
		} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_app_default_input_callback.  Invalid argument(s)");
	}

	return return_code;
}


int Scene_viewer_app_redraw(struct Scene_viewer_app *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Requests a full redraw in idle time.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_redraw);
	if (scene_viewer)
	{
		Scene_viewer_app_redraw_in_idle_time(scene_viewer);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_redraw.  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_redraw */

int Scene_viewer_app_redraw_now(struct Scene_viewer_app *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 29 September 2000

DESCRIPTION :
Requests a full redraw immediately.
==============================================================================*/
{
	int return_code = 0;
	struct Event_dispatcher *event_dispatcher = 0;

	ENTER(Scene_viewer_redraw_now);
	if (scene_viewer)
	{
		/* remove idle update workproc if pending */
		event_dispatcher = User_interface_get_event_dispatcher(
			scene_viewer->user_interface);
		if (scene_viewer->idle_update_callback_id)
		{
			Event_dispatcher_remove_idle_callback(
				event_dispatcher, scene_viewer->idle_update_callback_id);
			scene_viewer->idle_update_callback_id = (struct Event_dispatcher_idle_callback *)NULL;
		}
		if (scene_viewer->core_scene_viewer->tumble_active)
		{
			Scene_viewer_automatic_tumble(scene_viewer);
			if(!scene_viewer->idle_update_callback_id)
			{
				scene_viewer->idle_update_callback_id =  Event_dispatcher_add_idle_callback(
					event_dispatcher, Scene_viewer_app_idle_update_callback, scene_viewer,
					EVENT_DISPATCHER_IDLE_UPDATE_SCENE_VIEWER_PRIORITY);
			}
		}
		Graphics_buffer_app_make_current(scene_viewer->graphics_buffer);
		return_code = Cmiss_scene_viewer_render_scene(scene_viewer->core_scene_viewer);
		if (scene_viewer->core_scene_viewer->swap_buffers)
		{
			Graphics_buffer_app_swap_buffers(scene_viewer->graphics_buffer);
		}
		//-- CMISS_CALLBACK_LIST_CALL(Scene_viewer_callback)(
		//-- 	scene_viewer->repaint_required_callback_list, scene_viewer, NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_redraw_now.  Missing scene_viewer");
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_redraw_now */

int Scene_viewer_app_redraw_now_with_overrides(struct Scene_viewer_app *scene_viewer,
	int antialias, int transparency_layers)
/*******************************************************************************
LAST MODIFIED : 23 September 2002

DESCRIPTION :
Requests a full redraw immediately.  If non_zero then the supplied <antialias>
and <transparency_layers> are used for just this render.
==============================================================================*/
{
	int return_code = 0;
	struct Event_dispatcher *event_dispatcher = 0;

	ENTER(Scene_viewer_redraw_now);
	if (scene_viewer)
	{
		/* remove idle update workproc if pending */
		event_dispatcher = User_interface_get_event_dispatcher(
			scene_viewer->user_interface);
		if (scene_viewer->idle_update_callback_id)
		{
			Event_dispatcher_remove_idle_callback(
				event_dispatcher, scene_viewer->idle_update_callback_id);
			scene_viewer->idle_update_callback_id = (struct Event_dispatcher_idle_callback *)NULL;
		}
		if (scene_viewer->core_scene_viewer->tumble_active)
		{
			Scene_viewer_automatic_tumble(scene_viewer);
			if(!scene_viewer->idle_update_callback_id)
			{
				scene_viewer->idle_update_callback_id = 0;Event_dispatcher_add_idle_callback(
					event_dispatcher, Scene_viewer_app_idle_update_callback, (void *)scene_viewer,
					EVENT_DISPATCHER_IDLE_UPDATE_SCENE_VIEWER_PRIORITY);
			}
		}
		Graphics_buffer_app_make_current(scene_viewer->graphics_buffer);
		return_code = Scene_viewer_render_scene_in_viewport_with_overrides(
			scene_viewer->core_scene_viewer, /*left*/0, /*bottom*/0, /*right*/0, /*top*/0,
			antialias, transparency_layers, /*drawing_offscreen*/0);
		if (scene_viewer->core_scene_viewer->swap_buffers)
		{
			Graphics_buffer_app_swap_buffers(scene_viewer->graphics_buffer);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_redraw_now.  Missing scene_viewer");
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_redraw_now */

int Scene_viewer_app_redraw_now_without_swapbuffers(
	struct Scene_viewer_app *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 25 July 1998

DESCRIPTION :
Forces a redraw of the given scene viewer to take place immediately but does
not swap the back and front buffers so that utilities such as the movie
extensions can get the updated frame from the backbuffer.
==============================================================================*/
{
	int return_code = 0;

	ENTER(Scene_viewer_redraw_now_without_swapbuffers);
	if (scene_viewer)
	{
		Graphics_buffer_app_make_current(scene_viewer->graphics_buffer);
		/* always do a full redraw */
		return_code = Cmiss_scene_viewer_render_scene(scene_viewer->core_scene_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_redraw_now_without_swapbuffers.  Missing scene_viewer");
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_redraw_now_without_swapbuffers */

int Scene_viewer_app_idle_update_callback(void *scene_viewer_void)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Updates the scene_viewer.
==============================================================================*/
{
	int repeat_idle;
	struct Scene_viewer_app *scene_viewer=(struct Scene_viewer_app *)scene_viewer_void;

	ENTER(Scene_viewer_app_idle_update_callback);
	if (scene_viewer != 0)
	{
		/* set workproc no longer pending */
		scene_viewer->idle_update_callback_id = (struct Event_dispatcher_idle_callback *)NULL;
		if (scene_viewer->core_scene_viewer->tumble_active &&
				(!Interactive_tool_is_Transform_tool(scene_viewer->interactive_tool) ||
				Interactive_tool_transform_get_free_spin(scene_viewer->interactive_tool)))
		{
			Scene_viewer_automatic_tumble(scene_viewer);
			/* Repost the idle callback */
			if(!scene_viewer->idle_update_callback_id)
			{
				scene_viewer->idle_update_callback_id = Event_dispatcher_add_idle_callback(
					User_interface_get_event_dispatcher(scene_viewer->user_interface),
					Scene_viewer_app_idle_update_callback, (void *)scene_viewer,
					EVENT_DISPATCHER_IDLE_UPDATE_SCENE_VIEWER_PRIORITY);
			}
		}
		else
		{
			scene_viewer->core_scene_viewer->tumble_angle = 0.0;
		}
		Graphics_buffer_app_make_current(scene_viewer->graphics_buffer);
		Cmiss_scene_viewer_render_scene(scene_viewer->core_scene_viewer);
		if (scene_viewer->core_scene_viewer->swap_buffers)
		{
			Graphics_buffer_app_swap_buffers(scene_viewer->graphics_buffer);
		}
		//-- CMISS_CALLBACK_LIST_CALL(Scene_viewer_app_callback)(
		//-- 	scene_viewer->repaint_required_callback_list, scene_viewer, NULL);
		/* We don't want the idle callback to repeat so we return 0 */
		repeat_idle = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_app_idle_update_callback.  Missing scene_viewer");
		/* We don't want the idle callback to repeat so we return 0 */
		repeat_idle = 0;
	}
	LEAVE;

	return (repeat_idle);
} /* Scene_viewer_app_idle_update_callback */

int Scene_viewer_app_redraw_in_idle_time(struct Scene_viewer_app *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Sets up a callback to Scene_viewer_idle_update for an update in idle time.
Does this by putting a WorkProc on the queue - if not already done for this
Scene_viewer - which will force a redraw at the next idle moment. If the
scene_viewer is changed again before it is updated, a new WorkProc will not be
put in the queue, but the old one will update the window to the new state.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_redraw_in_idle_time);
	if (scene_viewer)
	{
		if (!scene_viewer->idle_update_callback_id)
		{
			scene_viewer->idle_update_callback_id = Event_dispatcher_add_idle_callback(
				User_interface_get_event_dispatcher(scene_viewer->user_interface),
				Scene_viewer_app_idle_update_callback, (void *)scene_viewer,
				EVENT_DISPATCHER_IDLE_UPDATE_SCENE_VIEWER_PRIORITY);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_redraw_in_idle_time.  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_redraw_in_idle_time */

int Scene_viewer_app_add_input_callback(struct Scene_viewer_app *scene_viewer,
	CMISS_CALLBACK_FUNCTION(Scene_viewer_app_input_callback) *function,
	void *user_data, int add_first)
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
Adds callback that will be activated each time input is received by the
scene_viewer.
If <add_first> is true (non zero) then this callback will be added to the
front of the list.
When a callback event is generated the list is processed as long as each
callback function returns true, so to stop processing and not call any more
of the callbacks registered after your handler then return false.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_add_input_callback);
	if (scene_viewer&&function)
	{
		if (add_first)
		{
			return_code =
				CMISS_CALLBACK_LIST_ADD_CALLBACK_TO_FRONT(Scene_viewer_app_input_callback)(
				scene_viewer->input_callback_list,function,user_data);
		}
		else
		{
			return_code =
				CMISS_CALLBACK_LIST_ADD_CALLBACK(Scene_viewer_app_input_callback)(
				scene_viewer->input_callback_list,function,user_data);
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_add_input_callback.  Could not add callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_add_input_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_add_input_callback */

int Scene_viewer_app_remove_input_callback(struct Scene_viewer_app *scene_viewer,
	CMISS_CALLBACK_FUNCTION(Scene_viewer_app_input_callback) *function,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 2 July 2002

DESCRIPTION :
Removes the callback calling <function> with <user_data> from
<scene_viewer>.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_remove_input_callback);
	if (scene_viewer&&function)
	{
		if (CMISS_CALLBACK_LIST_REMOVE_CALLBACK(Scene_viewer_app_input_callback)(
			scene_viewer->input_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_remove_input_callback.  Could not remove callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_remove_input_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_remove_input_callback */

int Scene_viewer_app_add_sync_callback(struct Scene_viewer_app *scene_viewer,
	CMISS_CALLBACK_FUNCTION(Scene_viewer_app_callback) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_add_sync_callback);
	if (scene_viewer&&function)
	{
		if (CMISS_CALLBACK_LIST_ADD_CALLBACK(Scene_viewer_app_callback)(
			scene_viewer->sync_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_add_sync_callback.  Could not add callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_add_sync_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_add_sync_callback */

int Scene_viewer_app_remove_sync_callback(struct Scene_viewer_app *scene_viewer,
	CMISS_CALLBACK_FUNCTION(Scene_viewer_app_callback) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Removes the callback calling <function> with <user_data> from
<scene_viewer>.
==============================================================================*/
{
	int return_code;

	if (scene_viewer&&function)
	{
		if (CMISS_CALLBACK_LIST_REMOVE_CALLBACK(Scene_viewer_app_callback)(
			scene_viewer->sync_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_app_remove_sync_callback.  Could not remove callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_app_remove_sync_callback.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Scene_viewer_app_remove_sync_callback */

struct Graphics_buffer_app *Scene_viewer_app_get_graphics_buffer(struct Scene_viewer_app *scene_viewer)
{
	return scene_viewer->graphics_buffer;
}

int Scene_viewer_app_add_transform_callback(struct Scene_viewer_app *scene_viewer,
	CMISS_CALLBACK_FUNCTION(Scene_viewer_app_callback) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_add_transform_callback);
	if (scene_viewer&&function)
	{
		if (CMISS_CALLBACK_LIST_ADD_CALLBACK(Scene_viewer_app_callback)(
			scene_viewer->transform_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_add_transform_callback.  Could not add callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_add_transform_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_add_transform_callback */

int Scene_viewer_app_remove_transform_callback(struct Scene_viewer_app *scene_viewer,
	CMISS_CALLBACK_FUNCTION(Scene_viewer_app_callback) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Removes the callback calling <function> with <user_data> from
<scene_viewer>.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_remove_transform_callback);
	if (scene_viewer&&function)
	{
		if (CMISS_CALLBACK_LIST_REMOVE_CALLBACK(Scene_viewer_app_callback)(
			scene_viewer->transform_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_remove_transform_callback.  Could not remove callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_remove_transform_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_remove_transform_callback */

int Scene_viewer_set_interactive_tool(struct Scene_viewer_app *scene_viewer,
	struct Interactive_tool *interactive_tool)
/*******************************************************************************
LAST MODIFIED : 25 February 2008

DESCRIPTION :
Sets the interactive tool that will receive input if the Scene_viewer is in
SCENE_VIEWER_SELECT mode. A NULL value indicates no tool.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_interactive_tool);
	if (scene_viewer)
	{
		if (scene_viewer->interactive_tool &&
			(scene_viewer->interactive_tool != interactive_tool))
		{
			Interactive_tool_reset(scene_viewer->interactive_tool);
		}
		scene_viewer->interactive_tool=interactive_tool;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_interactive_tool.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_interactive_tool */

int Scene_viewer_app_remove_destroy_callback(struct Scene_viewer_app *scene_viewer,
	CMISS_CALLBACK_FUNCTION(Scene_viewer_app_callback) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 19 February 2002

DESCRIPTION :
Removes the callback calling <function> with <user_data> from
<scene_viewer>.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_remove_destroy_callback);
	if (scene_viewer&&function)
	{
		if (CMISS_CALLBACK_LIST_REMOVE_CALLBACK(Scene_viewer_app_callback)(
			scene_viewer->destroy_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_remove_destroy_callback.  Could not remove callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_remove_destroy_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_remove_destroy_callback */

int Scene_viewer_app_add_destroy_callback(struct Scene_viewer_app *scene_viewer,
	CMISS_CALLBACK_FUNCTION(Scene_viewer_app_callback) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 19 February 2002

DESCRIPTION :
Adds a callback to the <scene_viewer> that is called back before the scene
viewer is destroyed.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_add_destroy_callback);
	if (scene_viewer&&function)
	{
		if (CMISS_CALLBACK_LIST_ADD_CALLBACK(Scene_viewer_app_callback)(
			scene_viewer->destroy_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_add_destroy_callback.  Could not add callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_add_destroy_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_add_destroy_callback */

void Scene_viewer_app_initialise_callback(struct Graphics_buffer_app *graphics_buffer,
	void *dummy_void, void *scene_viewer_void)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
==============================================================================*/
{
	struct Scene_viewer_app *scene_viewer=(struct Scene_viewer_app *)scene_viewer_void;

	ENTER(Scene_viewer_initialise_callback);
	USE_PARAMETER(dummy_void);
	if (scene_viewer != 0)
	{
		Graphics_buffer_app_make_current(graphics_buffer);
		/* initialise graphics library to load XFont */
		initialize_graphics_library();
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_initialise_callback.  Missing scene_viewer");
	}
	LEAVE;
} /* Scene_viewer_initialise_callback */

void Scene_viewer_app_resize_callback(struct Graphics_buffer_app *graphics_buffer,
	void *dummy_void, void *scene_viewer_void)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Called when part of the Scene_viewer window is resized. All it does is notify
callbacks interested in the scene_viewers transformations.
==============================================================================*/
{
	struct Scene_viewer_app *scene_viewer=(struct Scene_viewer_app *)scene_viewer_void;

	ENTER(Scene_viewer_resize_callback);
	USE_PARAMETER(graphics_buffer);
	USE_PARAMETER(dummy_void);
	if (scene_viewer != 0)
	{
		//-- Scene_viewer_app_set_transform_flag(scene_viewer->core_scene_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_resize_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_viewer_resize_callback */

void Scene_viewer_app_expose_callback(struct Graphics_buffer_app *graphics_buffer,
	void *expose_data_void, void *scene_viewer_void)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Called when part of the Scene_viewer window is exposed. Does not attempt to
redraw just the exposed area. Instead, it redraws the whole picture, but only
if there are no more expose events pending.
==============================================================================*/
{
	struct Graphics_buffer_expose_data *expose_data;
	struct Scene_viewer_app *scene_viewer=(struct Scene_viewer_app *)scene_viewer_void;

	ENTER(Scene_viewer_expose_callback);
	USE_PARAMETER(graphics_buffer);
	if (scene_viewer != 0)
	{
		if (!(expose_data = (struct Graphics_buffer_expose_data *)expose_data_void))
		{
			/* The redraw everything in idle time default */
			Scene_viewer_app_redraw(scene_viewer);
		}
		else
		{
			/* We are not currently using the fields of this data */
			USE_PARAMETER(expose_data);
			Scene_viewer_app_redraw_now(scene_viewer);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_expose_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_viewer_expose_callback */

void Scene_viewer_app_graphics_buffer_input_callback(
	struct Graphics_buffer_app *graphics_buffer,
	struct Graphics_buffer_input *input, void *scene_viewer_void)
/*******************************************************************************
LAST MODIFIED : 04 February 2005

DESCRIPTION :
The callback for mouse or keyboard input in the Scene_viewer window. The
resulting behaviour depends on the <scene_viewer> input_mode. In Transform mode
mouse clicks and drags are converted to transformation; in Select mode OpenGL
picking is performed with picked objects and mouse click and drag information
returned to the scene.
==============================================================================*/
{
	struct Scene_viewer_app *scene_viewer=(struct Scene_viewer_app *)scene_viewer_void;

	ENTER(Scene_viewer_graphics_buffer_input_callback);
	USE_PARAMETER(graphics_buffer);
	if (scene_viewer != 0)
	{
		CMISS_CALLBACK_LIST_CALL(Scene_viewer_app_input_callback)(
			scene_viewer->input_callback_list,scene_viewer,input);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_graphics_buffer_input_callback.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* Scene_viewer_input_callback */

int Scene_viewer_app_input_select(struct Scene_viewer_app *scene_viewer,
	struct Graphics_buffer_input *input)
/*******************************************************************************
LAST MODIFIED : 27 April 2000

DESCRIPTION :
Creates abstract interactive events relating to the mouse input to the
<scene_viewer> <event> and sends them to the current interactive_tool for the
scene_viewer.
==============================================================================*/
{
	double centre_x = 0.0,centre_y = 0.0,size_x,size_y,viewport_bottom,viewport_height,
		viewport_left,viewport_width;
	enum Interactive_event_type interactive_event_type = INTERACTIVE_EVENT_BUTTON_PRESS;
	int button_number = 0,i,input_modifier,j,modifier_state = 0,mouse_event,return_code;
	GLdouble temp_modelview_matrix[16], temp_projection_matrix[16];
	GLint viewport[4];
	struct Interactive_event *interactive_event;
	struct Interaction_volume *interaction_volume;

	ENTER(Scene_viewer_input_select);
	if (scene_viewer && scene_viewer->interactive_tool && input)
	{
		return_code=1;
		mouse_event=0;
		glGetIntegerv(GL_VIEWPORT,viewport);
		viewport_left   = (double)(viewport[0]);
		viewport_bottom = (double)(viewport[1]);
		viewport_width  = (double)(viewport[2]);
		viewport_height = (double)(viewport[3]);
		switch (input->type)
		{
			case CMISS_SCENE_VIEWER_INPUT_BUTTON_PRESS:
			{
				interactive_event_type=INTERACTIVE_EVENT_BUTTON_PRESS;
				centre_x=(double)(input->position_x);
				/* flip y as x event has y=0 at top of window, increasing down */
				centre_y=viewport_height-(double)(input->position_y)-1.0;
				button_number=input->button_number;
				/* Keep position for automatic tumbling update */
				scene_viewer->core_scene_viewer->previous_pointer_x = input->position_x;
				scene_viewer->core_scene_viewer->previous_pointer_y = input->position_y;
				modifier_state=input->input_modifier;
				mouse_event=1;
			} break;
			case CMISS_SCENE_VIEWER_INPUT_MOTION_NOTIFY:
			{
				interactive_event_type=INTERACTIVE_EVENT_MOTION_NOTIFY;
				centre_x=(double)(input->position_x);
				/* flip y as x event has y=0 at top of window, increasing down */
				centre_y=viewport_height-(double)(input->position_y)-1.0;
				button_number=-1;
				/* Keep position for automatic tumbling update */
				scene_viewer->core_scene_viewer->previous_pointer_x = input->position_x;
				scene_viewer->core_scene_viewer->previous_pointer_y = input->position_y;
				modifier_state=input->input_modifier;
				mouse_event=1;
			} break;
			case CMISS_SCENE_VIEWER_INPUT_BUTTON_RELEASE:
			{
				interactive_event_type=INTERACTIVE_EVENT_BUTTON_RELEASE;
				centre_x=(double)(input->position_x);
				/* flip y as x event has y=0 at top of window, increasing down */
				centre_y=viewport_height-(double)(input->position_y)-1.0;
				button_number=input->button_number;
				modifier_state=input->input_modifier;
				mouse_event=1;
			}
			case CMISS_SCENE_VIEWER_INPUT_KEY_PRESS:
			{
			} break;
			case CMISS_SCENE_VIEWER_INPUT_KEY_RELEASE:
			{
			} break;
			default:
			{
				printf("Scene_viewer_input_select.  Invalid X event");
				return_code=0;
			} break;
		}
		if (return_code&&mouse_event)
		{
			/*???RC Picking sensitivity should not be hardcoded - read from
				defaults file and/or set from text command */
			size_x = SCENE_VIEWER_PICK_SIZE;
			size_y = SCENE_VIEWER_PICK_SIZE;
			input_modifier=0;
			if (GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT&modifier_state)
			{
				input_modifier += INTERACTIVE_EVENT_MODIFIER_SHIFT;
			}
			/* note that control key currently overrides to transform mode */
			if (GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL&modifier_state)
			{
				input_modifier += INTERACTIVE_EVENT_MODIFIER_CONTROL;
			}
			if (GRAPHICS_BUFFER_INPUT_MODIFIER_ALT&modifier_state)
			{
				input_modifier += INTERACTIVE_EVENT_MODIFIER_ALT;
			}
			for (i=0;i<4;i++)
			{
				for (j=0;j<4;j++)
				{
					temp_modelview_matrix[i*4+j] =
						scene_viewer->core_scene_viewer->modelview_matrix[j*4+i];
					temp_projection_matrix[i*4+j] =
						scene_viewer->core_scene_viewer->window_projection_matrix[j*4+i];
				}
			}

			interaction_volume=create_Interaction_volume_ray_frustum(
				temp_modelview_matrix,temp_projection_matrix,
				viewport_left,viewport_bottom,viewport_width,viewport_height,
				centre_x,centre_y,size_x,size_y);
			ACCESS(Interaction_volume)(interaction_volume);
			interactive_event=CREATE(Interactive_event)(interactive_event_type,
				button_number,input_modifier,interaction_volume,scene_viewer->core_scene_viewer->scene);
			ACCESS(Interactive_event)(interactive_event);
			return_code=Interactive_tool_handle_interactive_event(
				scene_viewer->interactive_tool,(void *)scene_viewer,interactive_event,
				scene_viewer->core_scene_viewer->graphics_buffer);
			DEACCESS(Interactive_event)(&interactive_event);
			DEACCESS(Interaction_volume)(&interaction_volume);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_input_select.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_input_select */

