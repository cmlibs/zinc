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

#include "time/time_keeper.h"
#include "api/cmiss_field_group.h"
#include "api/cmiss_graphics_module.h"
#include "command/cmiss.h"
#include "context/context.h"
#include "context/context_app.h"
#include "curve/curve.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/object.h"
#include "graphics/graphics_module.h"
#include "graphics/rendition.h"
#include "selection/any_object_selection.h"
#include "region/cmiss_region.h"
/* following is temporary until circular references are removed for Cmiss_region  */
#include "region/cmiss_region_private.h"
#include <set>

struct Cmiss_context_app *Cmiss_context_app_access(Cmiss_context_app *context);
int Cmiss_context_app_destroy(struct Cmiss_context_app **context_address);

struct Cmiss_context_app
{
	int access_count;
	struct Context *context;
	struct Cmiss_command_data  *default_command_data;
	struct User_interface_module *UI_module;
	struct Event_dispatcher *event_dispatcher;
};

struct Context *Cmiss_context_app_get_core_context(struct Cmiss_context_app *context)
{
	return context->context;
}

class Context_holder
{
private:
	std::set<Cmiss_context_app *> contextsList;

	Context_holder()
	{
	}

	~Context_holder()
	{
		destroy();
	}

public:

	static Context_holder *getInstance()
	{
		static Context_holder contextHolder;
		return &contextHolder;
	}

	void destroy()
	{
		std::set<Cmiss_context_app *>::iterator pos = contextsList.begin();
		while (pos != contextsList.end())
		{
			Cmiss_context_app * temp_pointer = *pos;
			Cmiss_context_app_destroy(&temp_pointer);
			++pos;
		}
		contextsList.clear();
	}

	void insert(Cmiss_context_app * context_in)
	{
		if (Cmiss_context_app_access(context_in))
		{
			contextsList.insert(context_in);
		}
	}

	int hasEntry(Cmiss_context_app * context_in)
	{
		if (contextsList.find(context_in) != contextsList.end())
		{
			return true;
		}
		return false;
	}

};

struct Cmiss_context_app *Cmiss_context_app_create(const char *id)
{
	struct Cmiss_context_app *context = NULL;
	if (ALLOCATE(context, struct Cmiss_context_app, 1))
	{
		context->access_count = 1;
		context->context = Cmiss_context_create(id);
		context->default_command_data = NULL;
		context->UI_module = NULL;
		context->event_dispatcher = NULL;
		Context_holder::getInstance()->insert(context);
	}

	return context;
}

int Cmiss_context_app_destroy(struct Cmiss_context_app **context_address)
{
	int return_code = 0;
	struct Cmiss_context_app *context = NULL;

	if (context_address && NULL != (context = *context_address))
	{
		context->access_count--;
		if (0 == context->access_count)
		{
			if (context->context)
				Cmiss_context_destroy(&context->context);
			if (context->default_command_data)
				Cmiss_command_data_destroy(&context->default_command_data);
			if (context->UI_module)
				User_interface_module_destroy(&context->UI_module);
			if (context->event_dispatcher)
			{
				DESTROY(Event_dispatcher)(&context->event_dispatcher);
			}
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

	return return_code;
}

struct Cmiss_context_app *Cmiss_context_app_access(struct Cmiss_context_app *context)
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
struct Cmiss_graphics_module *Cmiss_context_app_create_graphics_module(struct Cmiss_context_app *context)
{
	struct Cmiss_graphics_module *graphics_module = NULL;

	if (context)
	{
		graphics_module = Cmiss_graphics_module_create(context->context);
		if (graphics_module && context->UI_module &&
			context->UI_module->default_time_keeper)
		{
			Cmiss_graphics_module_set_time_keeper_internal(graphics_module,
				context->UI_module->default_time_keeper );
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_context_create_graphics_module.  Missing context");
	}

	return graphics_module;
}

struct Cmiss_command_data *Cmiss_context_get_default_command_interpreter(struct Cmiss_context_app *context)
{
	struct Cmiss_command_data *command_data = NULL;

	if (context && context->UI_module)
	{
		if (!context->default_command_data)
		{
			context->default_command_data = CREATE(Cmiss_command_data)(context, context->UI_module);
		}
		if (context->default_command_data)
		{
			command_data = Cmiss_command_data_access(context->default_command_data);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_context_get_default_command_interpreter.  Missing context "
			"or user interface has not been enable yet.");
	}

	return command_data;
}
#if defined (WX_USER_INTERFACE) || (!defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER))
struct User_interface_module *Cmiss_context_create_user_interface(
	struct Cmiss_context_app *context, int in_argc, const char *in_argv[],
	void *user_interface_instance)
#else
struct User_interface_module *Cmiss_context_create_user_interface(
	struct Context *context, int in_argc, const char *in_argv[],
	HINSTANCE current_instance, HINSTANCE previous_instance,
	LPSTR command_line,int initial_main_window_state,
	void *user_interface_instance)
#endif
{
	USE_PARAMETER(user_interface_instance);
	struct User_interface_module *UI_module = NULL;

	if (context)
	{
		if (!context->UI_module)
		{
#if defined (WX_USER_INTERFACE)
			if (user_interface_instance)
				Event_dispatcher_set_wx_instance(Cmiss_context_app_get_default_event_dispatcher(context), user_interface_instance);
#endif
#if defined (WX_USER_INTERFACE) || (!defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER))
			context->UI_module = User_interface_module_create(
				context, in_argc, in_argv);
#else
			context->UI_module = User_interface_module_create(
				context, in_argc, in_argv, current_instance,
				previous_instance, command_line, initial_main_window_state);
#endif
			struct Cmiss_graphics_module *graphics_module = Cmiss_context_get_default_graphics_module(context->context);
			if (context->UI_module && context->UI_module->default_time_keeper &&
				graphics_module)
			{
				Cmiss_graphics_module_set_time_keeper_internal(graphics_module,
					context->UI_module->default_time_keeper );
			}
		}
		if (context->UI_module)
		{
			UI_module = User_interface_module_access(context->UI_module);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_context_create_user_interface.  Missing context.");
	}

	return UI_module;
}

struct Event_dispatcher *Cmiss_context_app_get_default_event_dispatcher(struct Cmiss_context_app *context)
{
	struct Event_dispatcher *event_dispatcher = NULL;
	if (context)
	{
		if (!context->event_dispatcher)
		{
			context->event_dispatcher = CREATE(Event_dispatcher)();
		}
		event_dispatcher = context->event_dispatcher;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_context_app_get_default_event_dispatcher.  Missing context.");
	}
	return event_dispatcher;
}

int Cmiss_context_app_execute_command(Cmiss_context_app *context,
	const char *command)
{
	int return_code = 0;
	if (context && context->UI_module && command)
	{
		struct Cmiss_command_data *command_data =
			Cmiss_context_get_default_command_interpreter(context);
		return_code = cmiss_execute_command(command, (void *)command_data);
		Cmiss_command_data_destroy(&command_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_context_execute_command.  Missing context or user interface or"
			"command is empty.");
	}

	return return_code;
}

int Cmiss_context_run_main_loop(Cmiss_context_app *context)
{
	int return_code = 0;
	if (context && context->UI_module)
	{
		struct Cmiss_command_data *command_data =
			Cmiss_context_get_default_command_interpreter(context);
		return_code = Cmiss_command_data_main_loop(command_data);
		Cmiss_command_data_destroy(&command_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_context_start_main_loop.  Missing context or user interface");
	}

	return return_code;
}

Cmiss_time_keeper_id Cmiss_context_app_get_default_time_keeper(
	Cmiss_context_app *context)
{
	Cmiss_time_keeper *time_keeper = NULL;
	if (context && context->UI_module && context->UI_module->default_time_keeper)
	{
		time_keeper = Cmiss_time_keeper_access(context->UI_module->default_time_keeper);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_context_get_default_time_keeper.  Missing context or user interface");
	}
	return time_keeper;
}

//-- Cmiss_scene_viewer_package_id Cmiss_context_app_get_default_scene_viewer_package(
//-- 	Cmiss_context_app *context)
//-- {
//-- 	Cmiss_scene_viewer_package *scene_viewer_package = NULL;
//-- 	if (context && context->UI_module && context->UI_module->scene_viewer_package)
//-- 	{
//-- 		scene_viewer_package = context->UI_module->scene_viewer_package;
//-- 	}
//-- 	else
//-- 	{
//-- 		display_message(ERROR_MESSAGE,
//-- 			"Cmiss_context_get_default_scene_viewer_package.  "
//-- 			"Missing context or user interface");
//-- 	}
//-- 	return scene_viewer_package;
//-- }

#if defined (WX_USER_INTERFACE) || (!defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER))
int Cmiss_context_app_enable_user_interface(Cmiss_context_app *context, void *user_interface_instance)
#else
int Cmiss_context_enable_user_interface(
	Cmiss_context_id context, HINSTANCE current_instance, HINSTANCE previous_instance,
	LPSTR command_line,int initial_main_window_state, void *user_interface_instance)
#endif
{
	int return_code = 0;
#if defined (WX_USER_INTERFACE) || (!defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER))
	struct User_interface_module *UI_module = Cmiss_context_create_user_interface(
		context, 0, 0, user_interface_instance);
#else
	struct User_interface_module *UI_module=  Cmiss_context_create_user_interface(
		context, 0, 0, current_instance, previous_instance,
		command_line, initial_main_window_state, user_interface_instance);
#endif
	if (UI_module)
	{
		UI_module->external = 1;
		return_code = 1;
		User_interface_module_destroy(&UI_module);
	}

	return return_code;
}

int Cmiss_context_app_process_idle_event(Cmiss_context_app *context)
{
	if (context && context->event_dispatcher)
	{
		return Event_dispatcher_process_idle_event(context->event_dispatcher);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_context_do_idle_event.  Missing context or event dispatcher.");
	}

	return 0;
}

void Context_internal_cleanup()
{
	Context_holder::getInstance()->destroy();

	/* Write out any memory blocks still ALLOCATED when MEMORY_CHECKING is
		on.  When MEMORY_CHECKING is off this function does nothing but only list
		when context holder does not have an entry of this context */
	list_memory(/*count_number*/0, /*show_pointers*/0, /*increment_counter*/0,
		/*show_structures*/1);
}
