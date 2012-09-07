
#include "api/cmiss_context.h"
#include "api/cmiss_graphics_module.h"

#include "general/message.h"
#include "general/debug.h"
#include "user_interface/user_interface.h"
#include "time/time_keeper.h"
#include "context/user_interface_module.h"
#include "user_interface/event_dispatcher.h"
#include "context/context.h"
#include "graphics/graphics_module.h"

struct Cmiss_graphics_module *Cmiss_context_get_default_graphics_module(struct Context *context)
{
	struct Cmiss_graphics_module *graphics_module = 0;

	if (context)
	{
		if (!context->graphics_module)
		{
			context->graphics_module = Cmiss_context_create_graphics_module(context);
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

struct Event_dispatcher *Cmiss_context_get_default_event_dispatcher(Cmiss_context_id context)
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
			"Cmiss_context_get_default_event_dispatcher.  Missing context.");
	}
	return event_dispatcher;
}

struct User_interface_module *Cmiss_context_create_user_interface(
	Cmiss_context_id context, int in_argc, const char *in_argv[],
	void *user_interface_instance)
{
	USE_PARAMETER(user_interface_instance);
	struct User_interface_module *UI_module = 0;

	if (context)
	{
		if (!context->UI_module)
		{
			if (user_interface_instance)
				Event_dispatcher_set_wx_instance(Cmiss_context_get_default_event_dispatcher(context), user_interface_instance);
			context->UI_module = User_interface_module_create(
				context, in_argc, in_argv);
			if (context->UI_module && context->UI_module->default_time_keeper &&
				context->graphics_module)
			{
				Cmiss_graphics_module_set_time_keeper_internal(context->graphics_module,
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


int Cmiss_context_enable_user_interface(Cmiss_context_id context, void *user_interface_instance)
{
	int return_code = 0;
	struct User_interface_module *UI_module = Cmiss_context_create_user_interface(
		context, 0, 0, user_interface_instance);
	if (UI_module)
	{
		return_code = 1;
		User_interface_module_destroy(&UI_module);
	}

	return return_code;
}

Cmiss_graphics_module_id Cmiss_context_create_graphics_module(struct Context *context)
{
	struct Cmiss_graphics_module *graphics_module = NULL;

	if (context)
	{
		graphics_module = Cmiss_graphics_module_create(context);
		if (graphics_module && context->UI_module &&
			context->UI_module->default_time_keeper)
		{
			Cmiss_graphics_module_set_time_keeper_internal(context->graphics_module,
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

Cmiss_time_keeper_id Cmiss_context_get_default_time_keeper(
	Cmiss_context_id context)
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

Cmiss_scene_viewer_package_id Cmiss_context_get_default_scene_viewer_package(
	Cmiss_context_id context)
{
	Cmiss_scene_viewer_package *scene_viewer_package = NULL;
	if (context && context->UI_module && context->UI_module->scene_viewer_package)
	{
		scene_viewer_package = context->UI_module->scene_viewer_package;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_context_get_default_scene_viewer_package.  "
			"Missing context or user interface");
	}
	return scene_viewer_package;
}

int Cmiss_context_process_idle_event(Cmiss_context_id context)
{
	if (context && context->event_dispatcher)
	{
		return Event_dispatcher_process_idle_event(context->event_dispatcher);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_context_do_idle_event.  Missing context or event dispatcher.");
		return 0;
	}
}

