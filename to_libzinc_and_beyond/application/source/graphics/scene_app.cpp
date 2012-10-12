
#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */



#include "command/parser.h"
#include "general/enumerator_private.hpp"
#include "graphics/scene.h"
#include "graphics/scene_app.h"
#include "general/message.h"
#include "graphics/light_app.h"
#include "graphics/graphics_filter.hpp"
#include "graphics/graphics_filter_app.hpp"
#include "region/cmiss_region_app.h"
#include "user_interface/user_interface.h"

#include "graphics/render_gl.h"

int set_Scene(struct Parse_state *state,
	void *scene_address_void,void *scene_manager_void)
/*******************************************************************************
LAST MODIFIED : 22 June 1999

DESCRIPTION :
Modifier function to set the scene from a command.
???RC set_Object routines could become a macro.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Scene *scene,**scene_address;
	struct MANAGER(Scene) *scene_manager;

	ENTER(set_Scene);
	if (state)
	{
		current_token=state->current_token;
		if (current_token != 0)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if ((scene_address=(struct Scene **)scene_address_void)&&
					(scene_manager=(struct MANAGER(Scene) *)scene_manager_void))
				{
					if (fuzzy_string_compare(current_token,"NONE"))
					{
						if (*scene_address)
						{
							DEACCESS(Scene)(scene_address);
							*scene_address=(struct Scene *)NULL;
						}
						return_code=1;
					}
					else
					{
						scene=FIND_BY_IDENTIFIER_IN_MANAGER(Scene,name)(current_token,
							scene_manager);
						if (scene != 0)
						{
							if (*scene_address != scene)
							{
								ACCESS(Scene)(scene);
								if (*scene_address)
								{
									DEACCESS(Scene)(scene_address);
								}
								*scene_address=scene;
							}
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown scene : %s",
								current_token);
							return_code=0;
						}
					}
					shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Scene.  Invalid argument(s)");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," SCENE_NAME|none");
				scene_address=(struct Scene **)scene_address_void;
				if (scene_address != 0)
				{
					scene= *scene_address;
					if (scene != 0)
					{
						display_message(INFORMATION_MESSAGE,"[%s]",scene->name);
					}
					else
					{
						display_message(INFORMATION_MESSAGE,"[none]");
					}
				}
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing scene name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Scene.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Scene */

int define_Scene_contents(struct Parse_state *state, void *scene_void,
	void *define_scene_data_void)
{
	int return_code;

	ENTER(define_Scene_contents);
	struct Define_scene_data *define_scene_data =
		(struct Define_scene_data *)define_scene_data_void;
	if (state && define_scene_data)
	{
		Cmiss_scene *scene = (Cmiss_scene *)scene_void; // can be NULL;
		struct Light *light_to_add = NULL;
		struct Light *light_to_remove = NULL;
		Cmiss_region *region = NULL;
		Cmiss_graphics_filter_id filter = NULL;
		if (scene && scene->region)
		{
			region = Cmiss_region_access(scene->region);
		}
		else
		{
			region = Cmiss_region_access(define_scene_data->root_region);
		}

		struct Option_table *option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table, "add_light", &light_to_add,
			define_scene_data->light_manager, set_Light);
		Option_table_add_set_Cmiss_region(option_table, "region",
			define_scene_data->root_region, &region);
		Option_table_add_entry(option_table, "remove_light", &light_to_remove,
			define_scene_data->light_manager, set_Light);
		Option_table_add_entry(option_table, "filter", &filter,
			define_scene_data->graphics_module, set_Cmiss_graphics_filter);

		return_code = Option_table_multi_parse(option_table,state);
		if (return_code)
		{
			if (scene)
			{
				Scene_begin_cache(scene);
				if (light_to_add)
				{
					Scene_add_light(scene,light_to_add);
				}
				if (light_to_remove)
				{
					Scene_remove_light(scene,light_to_remove);
				}
				Cmiss_scene_set_region(scene, region);
				if (filter)
					Cmiss_scene_set_filter(scene, filter);
				Scene_end_cache(scene);
			}
		}
		DESTROY(Option_table)(&option_table);
		if (light_to_add)
		{
			DEACCESS(Light)(&light_to_add);
		}
		if (light_to_remove)
		{
			DEACCESS(Light)(&light_to_remove);
		}
		if (filter)
			DEACCESS(Cmiss_graphics_filter)(&filter);
		Cmiss_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Scene_contents.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int define_Scene(struct Parse_state *state, void *dummy_to_be_modified,
	void *define_scene_data_void)
{
	int return_code = 1;
	const char *current_token;
	struct Option_table *option_table;
	struct Define_scene_data *define_scene_data;

	ENTER(define_Scene);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (define_scene_data =
		(struct Define_scene_data *)define_scene_data_void))
	{
		current_token=state->current_token;
		if (current_token != 0)
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				MANAGER(Scene) *scene_manager = define_scene_data->scene_manager;
				MANAGER_BEGIN_CACHE(Scene)(scene_manager);
				Cmiss_scene_id scene =
					Cmiss_graphics_module_find_scene_by_name(define_scene_data->graphics_module, current_token);
				if (!scene)
				{
					scene = Cmiss_graphics_module_create_scene(define_scene_data->graphics_module);
					Cmiss_scene_set_name(scene, current_token);
				}
				shift_Parse_state(state,1);
				if (scene)
				{
					// set managed state for all tessellations created or edited otherwise
					// cleaned up at end of command.
					Cmiss_scene_set_attribute_integer(scene, CMISS_SCENE_ATTRIBUTE_IS_MANAGED, 1);
					return_code = define_Scene_contents(state, (void *)scene, define_scene_data_void);
				}
				Cmiss_scene_destroy(&scene);
				MANAGER_END_CACHE(Scene)(scene_manager);
			}
			else
			{
				option_table = CREATE(Option_table)();
				Option_table_add_entry(option_table, "SCENE_NAME",
					/*scene*/(void *)NULL, define_scene_data_void,
					define_Scene_contents);
				return_code = Option_table_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing scene name");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "define_Scene.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}


