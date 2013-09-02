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

#include "zinc/fieldgroup.h"
#include "zinc/graphicsmodule.h"
#include "context/context.h"
#include "curve/curve.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/object.h"
#include "graphics/scene_viewer.h"
#include "graphics/graphics_module.h"
#include "graphics/scene.h"
#include "selection/any_object_selection.h"
#include "region/cmiss_region.h"
#include "time/time_keeper.hpp"
//-- #include "user_interface/event_dispatcher.h"
/* following is temporary until circular references are removed for cmzn_region  */
#include "region/cmiss_region_private.h"

struct Context *cmzn_context_create(const char *id)
{
	struct Context *context = NULL;
	if (ALLOCATE(context, struct Context, 1))
	{
		context->graphics_module = NULL;
		context->root_region = NULL;
		context->id = duplicate_string(id);
		context->any_object_selection = NULL;
		context->element_point_ranges_selection = NULL;
		context->io_stream_package = NULL;
		context->curve_manager = NULL;
		context->time_keeper = 0;
		context->access_count = 1;
	}

	return context;
}

int cmzn_context_destroy(struct Context **context_address)
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
			if (context->graphics_module)
			{
				cmzn_graphics_module_remove_external_callback_dependency(
					context->graphics_module);
				cmzn_graphics_module_destroy(&context->graphics_module);
			}
			if (context->root_region)
			{
				/* need the following due to circular references where field owned by region references region itself;
				 * when following removed also remove #include "region/cmiss_region_private.h". Also scene
				 * has a computed_field manager callback so it must be deleted before detaching fields hierarchical */
				cmzn_region_detach_fields_hierarchical(context->root_region);
				DEACCESS(cmzn_region)(&context->root_region);
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
			if (context->time_keeper)
			{
				cmzn_time_keeper_destroy(&(context->time_keeper));
			}
			DEALLOCATE(*context_address);
		}
		*context_address = NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_context_destroy.  Missing context address");
		return_code = 0;
	}

	/* Write out any memory blocks still ALLOCATED when MEMORY_CHECKING is
		on.  When MEMORY_CHECKING is off this function does nothing */
	list_memory(/*count_number*/0, /*show_pointers*/0, /*increment_counter*/0,
		/*show_structures*/1);

	return return_code;
}

struct Context *cmzn_context_access(struct Context *context)
{
	if (context)
	{
		context->access_count++;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_context_access.  Missing context");
	}
	return context;
}

struct cmzn_region *cmzn_context_get_default_region(struct Context *context)
{
	struct cmzn_region *root_region = 0;

	if (context)
	{
		if (!context->root_region)
		{
			context->root_region = cmzn_region_create_internal();
		}
		root_region = ACCESS(cmzn_region)(context->root_region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_context_get_default_region.  Missing context");
	}

	return root_region;
}

struct cmzn_graphics_module *cmzn_context_get_graphics_module(struct Context *context)
{
	struct cmzn_graphics_module *graphics_module = 0;

	if (context)
	{
		if (!context->graphics_module)
		{
			context->graphics_module = cmzn_graphics_module_create(context);
		}
		graphics_module = cmzn_graphics_module_access(context->graphics_module);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_context_get_default_graphics_module.  Missing context");
	}

	return graphics_module;
}

struct cmzn_region *cmzn_context_create_region(struct Context *context)
{
	cmzn_region *region = NULL;

	if (context)
	{
		// all regions share the element shapes and bases from the default_region
		if (!context->root_region)
		{
			cmzn_region *default_region = cmzn_context_get_default_region(context);
			cmzn_region_destroy(&default_region);
		}
		region = cmzn_region_create_region(context->root_region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_context_create_region.  Missing context");
	}
	return region;
}

struct Any_object_selection *cmzn_context_get_any_object_selection(
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
			"cmzn_context_get_any_object_selection.  Missing context.");
	}
	return any_object_selection;
}

struct Element_point_ranges_selection *cmzn_context_get_element_point_ranges_selection(
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
			"cmzn_context_get_element_point_ranges_selection.  Missing context.");
	}
	return element_point_ranges_selection;
}

struct IO_stream_package *cmzn_context_get_default_IO_stream_package(
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
			"cmzn_context_get_default_IO_stream_package.  Missing context.");
	}

	return io_stream_package;
}

struct cmzn_time_keeper *cmzn_context_get_default_time_keeper(struct Context *context)
{
	cmzn_time_keeper *time_keeper = 0;
	if (context)
	{
		if (!context->time_keeper)
		{
			context->time_keeper = new cmzn_time_keeper();
		}
		time_keeper = cmzn_time_keeper_access(context->time_keeper);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_context_get_default_time_keeper.  Missing context");
	}
	return time_keeper;
}

struct MANAGER(Curve) *cmzn_context_get_default_curve_manager(
	cmzn_context_id context)
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
			"cmzn_context_get_default_curve_manager.  "
			"Missing context");
	}
	return curve_manager;
}

