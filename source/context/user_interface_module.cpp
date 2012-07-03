/*******************************************************************************
FILE : User_intrefac_module.c

LAST MODIFIED : 7 January 2003

DESCRIPTION :
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1 f
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

extern "C" {
#include "time/time_keeper.h"
#include "comfile/comfile.h"
#include "command/command_window.h"
#include "command/cmiss.h"
#include "element/element_point_tool.h"
#include "element/element_tool.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "graphics/colour.h"
#include "graphics/graphics_module.h"
#include "graphics/transform_tool.h"
#include "graphics/scene.h"
#include "interaction/interactive_tool.h"
#include "node/node_tool.h"
#include "three_d_drawing/graphics_buffer.h"
#include "user_interface/event_dispatcher.h"
#include "user_interface/message.h"
}
#if defined (USE_OPENCASCADE)
#include "cad/cad_tool.h"
#endif /* defined (USE_OPENCASCADE) */

#if defined (WX_USER_INTERFACE) || (!defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER))
struct User_interface_module *User_interface_module_create(
	struct Context *context, int in_argc, const char *in_argv[])
#else
struct User_interface_module *User_interface_module_create(
	struct Context *context, int in_argc, const char *in_argv[],
	HINSTANCE current_instance, HINSTANCE previous_instance, 
	LPSTR command_line,int initial_main_window_state)
#endif
{
	struct User_interface_module  *UI_module = NULL;
	Cmiss_region *root_region = NULL;;
	struct Cmiss_graphics_module *graphics_module = NULL;
	int visual_id = 0;

	if (context && ALLOCATE(UI_module, struct User_interface_module, 1))
	{
		root_region = Cmiss_context_get_default_region(context);
		graphics_module = Cmiss_context_get_default_graphics_module(context);
		UI_module->event_dispatcher = NULL;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
		UI_module->graphics_window_manager = NULL;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
		UI_module->default_time_keeper = NULL;
		UI_module->user_interface = NULL;
		UI_module->emoter_slider_dialog = NULL;
#if defined (WX_USER_INTERFACE)
		UI_module->comfile_window_manager = NULL;
		UI_module->data_viewer = NULL; 
		UI_module->node_viewer = NULL;
		UI_module->element_point_viewer = NULL;
		UI_module->material_editor_dialog = NULL;
		UI_module->region_tree_viewer = NULL;
		UI_module->spectrum_editor_dialog = NULL;
#endif /* defined (WX_USER_INTERFACE) */
		UI_module->scene_viewer_package = NULL;
		UI_module->graphics_buffer_package = NULL;
		UI_module->interactive_tool_manager = NULL;
		UI_module->background_colour.red=(float)0;
		UI_module->background_colour.green=(float)0;
		UI_module->background_colour.blue=(float)0;
		UI_module->foreground_colour.red=(float)1;
		UI_module->foreground_colour.green=(float)1;
		UI_module->foreground_colour.blue=(float)1;
		UI_module->access_count = 1;
		UI_module->argc = in_argc;
		UI_module->argv = NULL;
		UI_module->external = 0;
		UI_module->unmodified_argv = NULL;
		UI_module->cleanup_argc = in_argc;
		UI_module->cleanup_argv = NULL;
		struct Cmgui_command_line_options command_line_options;
		Cmiss_command_data_process_command_line(in_argc, in_argv, 
			&command_line_options);
		visual_id = command_line_options.visual_id_number;
		if (0 < in_argc)
		{
			ALLOCATE(UI_module->argv, char *, in_argc);
			ALLOCATE(UI_module->unmodified_argv, char *, in_argc);
			ALLOCATE(UI_module->cleanup_argv, char *, in_argc);
			for (int ai = 0; ai < in_argc; ai++)
			{
				UI_module->cleanup_argv[ai] = UI_module->argv[ai] = UI_module->unmodified_argv[ai] =
					duplicate_string(in_argv[ai]);
			}
		}

		if ((!command_line_options.command_list_flag) && (!command_line_options.write_help_flag))
		{
			if (NULL != (UI_module->event_dispatcher = Cmiss_context_get_default_event_dispatcher(
				context)))
			{
				if (!command_line_options.no_display_flag)
				{
#if defined (WX_USER_INTERFACE) || (!defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER))
					if (NULL == (UI_module->user_interface = CREATE(User_interface)
							(&(UI_module->argc), UI_module->argv, UI_module->event_dispatcher, "Cmgui",
								"cmgui")))
					{
						display_message(ERROR_MESSAGE,"Could not create User interface");
					}
#else /* !defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER) */
					if (NULL == (UI_module->user_interface = CREATE(User_interface)
							(current_instance, previous_instance, command_line,
								initial_main_window_state, &(UI_module->argc), UI_module->argv, UI_module->event_dispatcher)))
					{
						display_message(ERROR_MESSAGE,"Could not create User interface");
					}
#endif /* !defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER) */
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Could not create Event dispatcher.");
		}
		if (command_line_options.example_file_name)
		{
			DEALLOCATE(command_line_options.example_file_name);
		}
		if (command_line_options.id_name)
		{
			DEALLOCATE(command_line_options.id_name);
		}
		if (command_line_options.execute_string)
		{
			DEALLOCATE(command_line_options.execute_string);
		}
		if (command_line_options.command_file_name)
		{
			DEALLOCATE(command_line_options.command_file_name);
		}
		if (command_line_options.epath_directory_name)
		{
			DEALLOCATE(command_line_options.epath_directory_name);
		}
#if defined (WX_USER_INTERFACE)
		/* comfile window manager */
		UI_module->comfile_window_manager = CREATE(MANAGER(Comfile_window))();
#endif /* defined (WX_USER_INTERFACE) */

#if defined (USE_CMGUI_GRAPHICS_WINDOW)
		if (UI_module->user_interface)
		{
			UI_module->graphics_buffer_package = CREATE(Graphics_buffer_package)(
				UI_module->user_interface);
			Graphics_buffer_package_set_override_visual_id(UI_module->graphics_buffer_package,
				visual_id);
		}
		/* graphics window manager.  Note there is no default window. */
		UI_module->graphics_window_manager=CREATE(MANAGER(Graphics_window))();
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */

		UI_module->default_time_keeper=ACCESS(Time_keeper)(
			CREATE(Time_keeper)("default", UI_module->event_dispatcher,
			UI_module->user_interface));
		UI_module->interactive_tool_manager=CREATE(MANAGER(Interactive_tool))();
		if (UI_module->user_interface)
		{
			struct Material_package *material_package = 
				Cmiss_graphics_module_get_material_package(graphics_module);
			UI_module->transform_tool=create_Interactive_tool_transform(
				UI_module->user_interface);
			ADD_OBJECT_TO_MANAGER(Interactive_tool)(UI_module->transform_tool,
				UI_module->interactive_tool_manager);
			UI_module->node_tool=CREATE(Node_tool)(
				UI_module->interactive_tool_manager,
				root_region, /*use_data*/0,
				Material_package_get_default_material(material_package),
				UI_module->user_interface,
				UI_module->default_time_keeper);
			UI_module->data_tool=CREATE(Node_tool)(
				UI_module->interactive_tool_manager,
				root_region, /*use_data*/1,
				Material_package_get_default_material(material_package),
				UI_module->user_interface,
				UI_module->default_time_keeper);
			UI_module->element_tool=CREATE(Element_tool)(
				UI_module->interactive_tool_manager,
				root_region,
				Cmiss_context_get_element_point_ranges_selection(context),
				Material_package_get_default_material(material_package),
				UI_module->user_interface,
				UI_module->default_time_keeper);
#if defined (USE_OPENCASCADE)
			UI_module->cad_tool=CREATE(Cad_tool)(
				UI_module->interactive_tool_manager,
				root_region,
				Cmiss_context_get_element_point_ranges_selection(context),
				Material_package_get_default_material(material_package),
				UI_module->user_interface,
				UI_module->default_time_keeper);
#endif /* defined (USE_OPENCASCADE) */
			UI_module->element_point_tool=CREATE(Element_point_tool)(
				UI_module->interactive_tool_manager,
				root_region,
				Cmiss_context_get_element_point_ranges_selection(context),
				Material_package_get_default_material(material_package),
				UI_module->user_interface,
				UI_module->default_time_keeper);
			DEACCESS(Material_package)(&material_package);
		}
		if (UI_module->user_interface)
		{
			/* set up image library */
#if defined (UNIX) /* switch (Operating_System) */
			if (UI_module->argv != 0)
			{
				Open_image_environment(*(UI_module->argv));
			}
			else
			{
				Open_image_environment("cmgui");
			}
#elif defined (WIN32_USER_INTERFACE) /* switch (Operating_System) */
			/* SAB Passing a string to this function so that it
				starts up, should get the correct thing from
				the windows system */
			Open_image_environment("cmgui");
#endif /* switch (Operating_System) */
		}
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
		if (UI_module->user_interface)
		{
			if (graphics_module)
			{
				struct Light *default_light = 
					Cmiss_graphics_module_get_default_light(graphics_module);
				struct Light_model *default_light_model = 
					Cmiss_graphics_module_get_default_light_model(graphics_module);
				struct Scene *default_scene =
					Cmiss_graphics_module_get_default_scene(graphics_module);
				UI_module->scene_viewer_package = CREATE(Cmiss_scene_viewer_package)
					(UI_module->graphics_buffer_package,
						&UI_module->background_colour,
						UI_module->interactive_tool_manager,
						Cmiss_graphics_module_get_light_manager(graphics_module), default_light,
						Cmiss_graphics_module_get_light_model_manager(graphics_module), default_light_model,
						Cmiss_graphics_module_get_scene_manager(graphics_module), default_scene,
						UI_module->user_interface);
				DEACCESS(Light_model)(&default_light_model);
				DEACCESS(Light)(&default_light);
				DEACCESS(Scene)(&default_scene);
			}
		}
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
		Cmiss_region_destroy(&root_region);
		Cmiss_graphics_module_destroy(&graphics_module);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"User_interface_module_create.  Invalid argument(s)");
	}

	return UI_module;
}

struct User_interface_module *User_interface_module_access(
	struct User_interface_module *UI_module)
{
	if (UI_module)
	{
		UI_module->access_count++;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"User_interface_module_access.  Invalid argument(s)");
	}

	return UI_module;
}

int User_interface_module_destroy(
	struct User_interface_module **UI_module_address)
{
	int return_code = 0;
	struct User_interface_module *UI_module  = NULL;

	if (UI_module_address && NULL != (UI_module = *UI_module_address))
	{
		UI_module->access_count--;
		if (0 == UI_module->access_count)
		{
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
			if (UI_module->scene_viewer_package)
			{
				DESTROY(Cmiss_scene_viewer_package)(&UI_module->scene_viewer_package);		
			}
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
			DESTROY(MANAGER(Graphics_window))(&UI_module->graphics_window_manager);
			/* Must destroy the graphics_buffer_package after the windows which use it */
			if (UI_module->graphics_buffer_package)
			{
				DESTROY(Graphics_buffer_package)(&UI_module->graphics_buffer_package);
			}
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
			DESTROY(MANAGER(Interactive_tool))(&(UI_module->interactive_tool_manager));

#if defined (WX_USER_INTERFACE)
			DESTROY(MANAGER(Comfile_window))(&UI_module->comfile_window_manager);
#endif /* defined (WX_USER_INTERFACE) */
			if (UI_module->default_time_keeper)
				DEACCESS(Time_keeper)(&UI_module->default_time_keeper);
			if (UI_module->user_interface)
			{
				/* reset up messages */
				set_display_message_function(ERROR_MESSAGE,
					(Display_message_function *)NULL, NULL);
				set_display_message_function(INFORMATION_MESSAGE,
					(Display_message_function *)NULL, NULL);
				set_display_message_function(WARNING_MESSAGE,
					(Display_message_function *)NULL, NULL);
				/* close the user interface */
				DESTROY(User_interface)(&(UI_module->user_interface));
			}

			if (UI_module->cleanup_argv != NULL)
			{
				for (int ai = 0; ai < UI_module->cleanup_argc; ai++)
				{
					DEALLOCATE(UI_module->cleanup_argv[ai]);
				}
				DEALLOCATE(UI_module->cleanup_argv);
				DEALLOCATE(UI_module->unmodified_argv);
				DEALLOCATE(UI_module->argv);
			}
			DEALLOCATE(*UI_module_address);
		}
		*UI_module_address = NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
				"User_interface_module_destroy.  Missing user interface module address");
		return_code = 0;
	}
		
	return return_code;
}
