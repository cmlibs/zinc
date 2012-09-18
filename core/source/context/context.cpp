/***************************************************************************//**
 * context.cpp
 *
 * The main root structure of cmgui.
 */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
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

#include "api/cmiss_field_group.h"
#include "api/cmiss_graphics_module.h"
#include "context/context.h"
#include "curve/curve.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/object.h"
#include "graphics/scene_viewer.h"
#include "graphics/graphics_module.h"
#include "graphics/rendition.h"
#include "selection/any_object_selection.h"
#include "region/cmiss_region.h"
#include "time/time_keeper.h"
//-- #include "user_interface/event_dispatcher.h"
/* following is temporary until circular references are removed for Cmiss_region  */
#include "region/cmiss_region_private.h"

struct Context *Cmiss_context_create(const char *id)
{
	struct Context *context = NULL;
	if (ALLOCATE(context, struct Context, 1))
	{
		context->graphics_module = NULL;
		context->root_region = NULL;
		context->id = duplicate_string(id);
		//-- context->UI_module = NULL;
		context->any_object_selection = NULL;
		context->element_point_ranges_selection = NULL;
		context->scene_viewer_package = NULL;
		//-- context->event_dispatcher = NULL;
		context->io_stream_package = NULL;
		context->curve_manager = NULL;
		context->access_count = 1;
	}

	return context;
}

int Cmiss_context_destroy(struct Context **context_address)
{
	int return_code = 0;
	struct Context *context = NULL;

	if (context_address && NULL != (context = *context_address))
	{
		context->access_count--;
		if (0 == context->access_count)
		{
			if (context->id)
				DEALLOCATE(context->id);
			//-- if (context->UI_module)
			//-- {
			//-- 	User_interface_module_destroy(&context->UI_module);
			//-- }
			if (context->graphics_module)
				Cmiss_graphics_module_destroy(&context->graphics_module);
			if (context->root_region)
			{
				/* need the following due to circular references where field owned by region references region itself;
				 * when following removed also remove #include "region/cmiss_region_private.h". Also rendition
				 * has a computed_field manager callback so it must be deleted before detaching fields hierarchical */
				Cmiss_region_detach_fields_hierarchical(context->root_region);
				DEACCESS(Cmiss_region)(&context->root_region);
			}
			if (context->scene_viewer_package)
			{
				DESTROY(Cmiss_scene_viewer_package)(&context->scene_viewer_package);
			}
			if (context->any_object_selection)
			{
				DESTROY(Any_object_selection)(&context->any_object_selection);
			}
			if (context->element_point_ranges_selection)
			{
				DESTROY(Element_point_ranges_selection)(&context->element_point_ranges_selection);
			}
			if (context->curve_manager)
			{
				DESTROY(MANAGER(Curve))(&context->curve_manager);
			}
			if (context->io_stream_package)
			{
				DESTROY(IO_stream_package)(&context->io_stream_package);
			}
			//-- if (context->event_dispatcher)
			//-- {
			//-- 	DESTROY(Event_dispatcher)(&context->event_dispatcher);
			//-- }
			DEALLOCATE(*context_address);
		}
		*context_address = NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_context_destroy.  Missing context address");
		return_code = 0;
	}

	/* Write out any memory blocks still ALLOCATED when MEMORY_CHECKING is
		on.  When MEMORY_CHECKING is off this function does nothing */
	list_memory(/*count_number*/0, /*show_pointers*/0, /*increment_counter*/0,
		/*show_structures*/1);

	return return_code;
}

struct Context *Cmiss_context_access(struct Context *context)
{
	if (context)
	{
		context->access_count++;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_context_access.  Missing context");
	}
	return context;
}

struct Cmiss_region *Cmiss_context_get_default_region(struct Context *context)
{
	struct Cmiss_region *root_region = 0;

	if (context)
	{
		if (!context->root_region)
		{
			context->root_region = Cmiss_region_create_internal();
		}
		root_region = ACCESS(Cmiss_region)(context->root_region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_context_get_default_region.  Missing context");
	}

	return root_region;
}

struct Cmiss_graphics_module *Cmiss_context_get_default_graphics_module(struct Context *context)
{
	struct Cmiss_graphics_module *graphics_module = 0;

	if (context)
	{
		if (!context->graphics_module)
		{
			context->graphics_module = Cmiss_graphics_module_create(context);
		}
		graphics_module = Cmiss_graphics_module_access(context->graphics_module);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_context_get_default_graphics_module.  Missing context");
	}

	return graphics_module;
}

struct Cmiss_region *Cmiss_context_create_region(struct Context *context)
{
	Cmiss_region *region = NULL;

	if (context)
	{
		// all regions share the element shapes and bases from the default_region
		if (!context->root_region)
		{
			Cmiss_region *default_region = Cmiss_context_get_default_region(context);
			Cmiss_region_destroy(&default_region);
		}
		region = Cmiss_region_create_region(context->root_region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_context_create_region.  Missing context");
	}
	return region;
}

struct Any_object_selection *Cmiss_context_get_any_object_selection(
	struct Context *context)
{
	struct Any_object_selection *any_object_selection = NULL;
	if (context)
	{
		if (!context->any_object_selection)
		{
			context->any_object_selection = CREATE(Any_object_selection)();
		}
		any_object_selection = context->any_object_selection;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_context_get_any_object_selection.  Missing context.");
	}
	return any_object_selection;
}

struct Element_point_ranges_selection *Cmiss_context_get_element_point_ranges_selection(
	struct Context *context)
{
	struct Element_point_ranges_selection *element_point_ranges_selection = NULL;
	if (context)
	{
		if (!context->element_point_ranges_selection)
		{
			context->element_point_ranges_selection = CREATE(Element_point_ranges_selection)();
		}
		element_point_ranges_selection = context->element_point_ranges_selection;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_context_get_element_point_ranges_selection.  Missing context.");
	}
	return element_point_ranges_selection;
}

struct IO_stream_package *Cmiss_context_get_default_IO_stream_package(
	struct Context *context)
{
	struct IO_stream_package *io_stream_package = NULL;
	if (context)
	{
		if (!context->io_stream_package)
		{
			context->io_stream_package = CREATE(IO_stream_package)();
		}
		io_stream_package = context->io_stream_package;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_context_get_default_IO_stream_package.  Missing context.");
	}

	return io_stream_package;
}

Cmiss_scene_viewer_package_id Cmiss_context_get_default_scene_viewer_package(
	Cmiss_context_id context)
{
	Cmiss_scene_viewer_package *scene_viewer_package = NULL;
	if (context)
	{
		if (!context->scene_viewer_package)
		{
			Cmiss_graphics_module_id graphics_module = Cmiss_context_get_default_graphics_module(context);
			if (graphics_module)
			{
				struct Light *default_light =
					Cmiss_graphics_module_get_default_light(graphics_module);
				struct Light_model *default_light_model =
					Cmiss_graphics_module_get_default_light_model(graphics_module);
				struct Scene *default_scene =
					Cmiss_graphics_module_get_default_scene(graphics_module);
				Colour default_background_colour;
				default_background_colour.red = 0.0;
				default_background_colour.green = 0.0;
				default_background_colour.blue = 0.0;
				context->scene_viewer_package = CREATE(Cmiss_scene_viewer_package)
					(/* Graphics_buffer_package */0,
						&default_background_colour,
						/* interactive_tool_manager */0,
						Cmiss_graphics_module_get_light_manager(graphics_module), default_light,
						Cmiss_graphics_module_get_light_model_manager(graphics_module), default_light_model,
						Cmiss_graphics_module_get_scene_manager(graphics_module), default_scene,
						/* user_interface */0);
				DEACCESS(Light_model)(&default_light_model);
				DEACCESS(Light)(&default_light);
				Cmiss_scene_destroy(&default_scene);
			}
		}
		scene_viewer_package = ACCESS(Cmiss_scene_viewer_package)(context->scene_viewer_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_context_get_default_scene_viewer_package.  "
			"Missing context");
	}
	return scene_viewer_package;
}

struct MANAGER(Curve) *Cmiss_context_get_default_curve_manager(
	Cmiss_context_id context)
{
	MANAGER(Curve) *curve_manager = NULL;
	if (context)
	{
		if (!context->curve_manager)
		{
			context->curve_manager = CREATE(MANAGER(Curve))();
		}
		curve_manager = context->curve_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_context_get_default_curve_manager.  "
			"Missing context");
	}
	return curve_manager;
}

//#if defined (WX_USER_INTERFACE) || (!defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER))
//int Cmiss_context_enable_user_interface(Cmiss_context_id context, void *user_interface_instance)
//#else
//int Cmiss_context_enable_user_interface(
//	Cmiss_context_id context, HINSTANCE current_instance, HINSTANCE previous_instance,
//	LPSTR command_line,int initial_main_window_state, void *user_interface_instance)
//#endif
//{
//	int return_code = 0;
//#if defined (WX_USER_INTERFACE) || (!defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER))
//	struct User_interface_module *UI_module = Cmiss_context_create_user_interface(
//		context, 0, 0, user_interface_instance);
//#else
//	struct User_interface_module *UI_module=  Cmiss_context_create_user_interface(
//		context, 0, 0, current_instance, previous_instance,
//		command_line, initial_main_window_state, user_interface_instance);
//#endif
//	if (UI_module)
//	{
//		UI_module->external = 1;
//		return_code = 1;
//		User_interface_module_destroy(&UI_module);
//	}
//
//	return return_code;
//}

