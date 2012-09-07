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

#include "general/debug.h"
#include "general/mystring.h"
//#include "command/cmiss.h"
#include "graphics/colour.h"
#include "graphics/scene.h"
#include "graphics/graphics_module.h"
#include "three_d_drawing/graphics_buffer.h"
#include "user_interface/user_interface.h"
#include "user_interface/event_dispatcher.h"
#include "context/user_interface_module.h"
#include "time/time_keeper.h"
#include "graphics/scene_viewer.h"

#if defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) || defined (WX_USER_INTERFACE) || defined (CARBON_USER_INTERFACE)
#define USE_CMGUI_GRAPHICS_WINDOW
#endif /* defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) || defined (WX_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) */

struct User_interface_module *User_interface_module_create(
	Cmiss_context_id context, int in_argc, const char *in_argv[])
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
		UI_module->default_time_keeper = NULL;
		UI_module->user_interface = NULL;
		UI_module->scene_viewer_package = NULL;
		UI_module->graphics_buffer_package = NULL;
		UI_module->background_colour.red=(float)0;
		UI_module->background_colour.green=(float)0;
		UI_module->background_colour.blue=(float)0;
		UI_module->foreground_colour.red=(float)1;
		UI_module->foreground_colour.green=(float)1;
		UI_module->foreground_colour.blue=(float)1;
		UI_module->access_count = 1;
		UI_module->argc = in_argc;
		UI_module->argv = NULL;
		UI_module->unmodified_argv = NULL;
		UI_module->cleanup_argc = in_argc;
		UI_module->cleanup_argv = NULL;
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

		UI_module->event_dispatcher = Cmiss_context_get_default_event_dispatcher(context);
		if (NULL == (UI_module->user_interface = CREATE(User_interface)
				(&(UI_module->argc), UI_module->argv, UI_module->event_dispatcher, "Cmgui",
					"cmgui")))
		{
			display_message(ERROR_MESSAGE,"Could not create User interface");
		}
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
		if (UI_module->user_interface)
		{
			UI_module->graphics_buffer_package = CREATE(Graphics_buffer_package)(
				UI_module->user_interface);
			Graphics_buffer_package_set_override_visual_id(UI_module->graphics_buffer_package,
				visual_id);
		}
		/* graphics window manager.  Note there is no default window. */
		//-- UI_module->graphics_window_manager=CREATE(MANAGER(Graphics_window))();
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */

		UI_module->default_time_keeper=Cmiss_time_keeper_access(
			CREATE(Time_keeper)("default", UI_module->event_dispatcher,
				UI_module->user_interface));
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
						0,
						Cmiss_graphics_module_get_light_manager(graphics_module), default_light,
						Cmiss_graphics_module_get_light_model_manager(graphics_module), default_light_model,
						Cmiss_graphics_module_get_scene_manager(graphics_module), default_scene,
						UI_module->user_interface);
				DEACCESS(Light_model)(&default_light_model);
				DEACCESS(Light)(&default_light);
				Cmiss_scene_destroy(&default_scene);
			}
		}
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
			/* Must destroy the graphics_buffer_package after the windows which use it */
			if (UI_module->graphics_buffer_package)
			{
				DESTROY(Graphics_buffer_package)(&UI_module->graphics_buffer_package);
			}

			if (UI_module->default_time_keeper)
				Cmiss_time_keeper_destroy(&UI_module->default_time_keeper);
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
