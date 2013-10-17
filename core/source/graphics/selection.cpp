/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <cstdlib>
#include "zinc/fieldgroup.h"
#include "computed_field/computed_field.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/object.h"
#include "graphics/graphics_module.h"
#include "graphics/scene.h"
#include "region/cmiss_region.h"
#include "graphics/selection.hpp"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_group_base.hpp"
#include "general/message.h"

static void  cmzn_selectionnotifier_callback(
	struct MANAGER_MESSAGE(Computed_field) *message,void *selectionnotifier_void);

struct cmzn_selectionevent
{
	enum cmzn_selectionevent_change_type change_type;
	int owning_scene_destroyed, access_count;

	cmzn_selectionevent() :
		change_type(CMZN_SELECTIONEVENT_CHANGE_NONE),
		owning_scene_destroyed(0),
		access_count(1)
	{
	}

	~cmzn_selectionevent()
	{
	}
};

struct cmzn_selectionnotifier
{
	bool hierarchical_flag;
	cmzn_scene_id scene;
	cmzn_selectionnotifier_callback_function function;
	void *user_data;
	void *callback_id;
	int owning_scene_destroyed, access_count;

	cmzn_selectionnotifier() :
		hierarchical_flag(false),
		scene(NULL),
		function(NULL),
		user_data(NULL),
		callback_id(NULL),
		owning_scene_destroyed(0),
		access_count(1)
	{
	}

	~cmzn_selectionnotifier()
	{
		clear();
	}

	void set_scene(struct cmzn_scene *scene_in)
	{
		scene = scene_in;
	}

	int set_callback(cmzn_selectionnotifier_callback_function function_in,
		void *user_data_in)
	{
		remove_manager_callback();
		function = function_in;
		user_data = user_data_in;
		int return_code = 0;
		cmzn_region_id region = cmzn_scene_get_region(scene);
		if (region)
		{
			struct MANAGER(Computed_field) *field_manager = cmzn_region_get_Computed_field_manager(region);
			if (field_manager)
			{
				callback_id = MANAGER_REGISTER(Computed_field)(
					cmzn_selectionnotifier_callback, (void *)this,	field_manager);
				return_code = (callback_id != NULL);
			}
		}
		return return_code;
	}

	void set_hierarchical(bool hierarchical_flag_in)
	{
		 hierarchical_flag = hierarchical_flag_in;
	}

	bool is_hierarchical()
	{
		 return hierarchical_flag;
	}

	void remove_manager_callback()
	{
		if (callback_id && scene)
		{
			cmzn_region_id region = cmzn_scene_get_region(scene);
			if (region)
			{
				struct MANAGER(Computed_field) *field_manager = cmzn_region_get_Computed_field_manager(region);
				if (field_manager && callback_id)
				{
					MANAGER_DEREGISTER(Computed_field)(callback_id, field_manager);
					callback_id = NULL;
				}
			}
		}
	}

	int clear()
	{
		remove_manager_callback();
		function = NULL;
		user_data = NULL;
		return 1;
	}

};

namespace {

/**
 * Frees the memory for the selection notifier at <*selectionnotifier_address>.
 * Sets *selectionnotifier_address to NULL.
 */
int DESTROY(cmzn_selectionnotifier)(struct cmzn_selectionnotifier **selectionnotifier_address)
{
	int return_code;

	ENTER(DESTROY(cmzn_selectionnotifier));
	if (selectionnotifier_address && (*selectionnotifier_address))
	{
		delete *selectionnotifier_address;
		*selectionnotifier_address = NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(cmzn_selectionnotifier).  Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int DESTROY(cmzn_selectionevent)(struct cmzn_selectionevent **selection_event_address)
{
	int return_code;

	if (selection_event_address && (*selection_event_address))
	{
		delete *selection_event_address;
		*selection_event_address = NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(selection_event_address).  Invalid argument");
		return_code = 0;
	}

	return (return_code);
}

} /* anonymous namespace */

static void cmzn_selectionnotifier_callback(
	struct MANAGER_MESSAGE(Computed_field) *message,void *selectionnotifier_void)
{
	if (message)
	{
		int selection_changed = 0;
		struct cmzn_selectionnotifier *selectionnotifier = (struct cmzn_selectionnotifier *)selectionnotifier_void;
		const cmzn_field_change_detail *source_change_detail = NULL;
		if (selectionnotifier && selectionnotifier->scene)
		{
			cmzn_field_group_id group_field = cmzn_scene_get_selection_group(selectionnotifier->scene);
			if (group_field)
			{
				int change = Computed_field_manager_message_get_object_change_and_detail(
					message, cmzn_field_group_base_cast(group_field), &source_change_detail);
				cmzn_selectionevent_id event = new cmzn_selectionevent();
				event->change_type = CMZN_SELECTIONEVENT_CHANGE_NONE;
				if (change & MANAGER_CHANGE_RESULT(Computed_field))
				{
					selection_changed = 1;
				}
				else if (change & MANAGER_CHANGE_REMOVE(Computed_field))
				{
					selection_changed = 1;
					event->change_type = CMZN_SELECTIONEVENT_CHANGE_CLEAR;
				}
				else if (change & MANAGER_CHANGE_ADD(Computed_field))
				{
					selection_changed = 1;
					event->change_type = CMZN_SELECTIONEVENT_CHANGE_ADD;
				}
				if (selection_changed)
				{
					if (source_change_detail)
					{
						const cmzn_field_group_base_change_detail *group_change_detail =
							dynamic_cast<const cmzn_field_group_base_change_detail *>(source_change_detail);
						cmzn_field_group_change_type group_change = CMZN_FIELD_GROUP_NO_CHANGE;
						if (selectionnotifier->hierarchical_flag)
							group_change = group_change_detail->getChange();
						else
							group_change = group_change_detail->getLocalChange();
						switch (group_change)
						{
							case CMZN_FIELD_GROUP_CLEAR:
							{
								event->change_type = CMZN_SELECTIONEVENT_CHANGE_CLEAR;
							} break;
							case CMZN_FIELD_GROUP_ADD:
							{
								event->change_type = CMZN_SELECTIONEVENT_CHANGE_ADD;
							} break;
							case CMZN_FIELD_GROUP_REMOVE:
							{
								event->change_type = CMZN_SELECTIONEVENT_CHANGE_REMOVE;
							} break;
							case CMZN_FIELD_GROUP_REPLACE:
							{
								event->change_type = CMZN_SELECTIONEVENT_CHANGE_REPLACE;
							} break;
							default:
							{
								event->change_type = CMZN_SELECTIONEVENT_CHANGE_NONE;
							} break;
						}
					}
					event->owning_scene_destroyed = selectionnotifier->owning_scene_destroyed;
					if (event->change_type != CMZN_SELECTIONEVENT_CHANGE_NONE)
						(selectionnotifier->function)(event, selectionnotifier->user_data);
				}
				cmzn_selectionevent_destroy(&event);
				cmzn_field_group_destroy(&group_field);
			}
		}
	}
}

int cmzn_selectionnotifier_set_scene(cmzn_selectionnotifier_id selectionnotifier,
	struct cmzn_scene *scene_in)
{
	if (selectionnotifier && scene_in)
	{
		selectionnotifier->set_scene(scene_in);
		return 1;
	}
	return 0;
}

bool cmzn_selectionnotifier_is_hierarchical(cmzn_selectionnotifier_id selectionnotifier)
{
	if (selectionnotifier)
		return selectionnotifier->is_hierarchical();
	return false;
}

int cmzn_selectionnotifier_set_hierarchical(
	cmzn_selectionnotifier_id selectionnotifier, bool hierarchical)
{
	if (selectionnotifier)
	{
		selectionnotifier->set_hierarchical(hierarchical);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_selectionnotifier_set_callback(cmzn_selectionnotifier_id selectionnotifier,
	cmzn_selectionnotifier_callback_function function_in, void *user_data_in)
{
	if (selectionnotifier && function_in)
	{
		return selectionnotifier->set_callback(function_in, user_data_in);
	}

	return 0;
}

/***************************************************************************//**
 * Clear callback.
 */
int cmzn_selectionnotifier_clear_callback(cmzn_selectionnotifier_id selectionnotifier)
{
	if (selectionnotifier)
	{
		return selectionnotifier->clear();
	}
	return 0;
}

DECLARE_OBJECT_FUNCTIONS(cmzn_selectionnotifier);

cmzn_selectionnotifier_id cmzn_selectionnotifier_access(
	cmzn_selectionnotifier_id selectionnotifier)
{
	return ACCESS(cmzn_selectionnotifier)(selectionnotifier);
}

int cmzn_selectionnotifier_destroy(cmzn_selectionnotifier_id *selectionnotifier_address)
{
	return DEACCESS(cmzn_selectionnotifier)(selectionnotifier_address);
}

DECLARE_OBJECT_FUNCTIONS(cmzn_selectionevent);

cmzn_selectionevent_id cmzn_selectionevent_access(
	cmzn_selectionevent_id selection_event)
{
	return ACCESS(cmzn_selectionevent)(selection_event);
}

int cmzn_selectionevent_destroy(cmzn_selectionevent_id *selection_event_address)
{
	return DEACCESS(cmzn_selectionevent)(selection_event_address);
}

enum cmzn_selectionevent_change_type cmzn_selectionevent_get_change_type(
	cmzn_selectionevent_id selection_event)
{
	return selection_event->change_type;
}

int cmzn_selectionevent_owning_scene_is_destroyed(
		cmzn_selectionevent_id selection_event)
	{
		return selection_event->owning_scene_destroyed;
	}

cmzn_selectionnotifier_id cmzn_selectionnotifier_create_private()
{
	cmzn_selectionnotifier_id notifier = new cmzn_selectionnotifier();
	return notifier;
}

int cmzn_selectionnotifier_scene_destroyed(cmzn_selectionnotifier_id selectionnotifier)
{
	if (selectionnotifier && selectionnotifier->function)
	{
		selectionnotifier->owning_scene_destroyed = 1;
		selectionnotifier->scene = NULL;
		if (selectionnotifier->function)
		{
			cmzn_selectionevent_id event = new cmzn_selectionevent();
			event->change_type = CMZN_SELECTIONEVENT_CHANGE_NONE;
			event->owning_scene_destroyed = 1;
			(selectionnotifier->function)(event, selectionnotifier->user_data);
			cmzn_selectionevent_destroy(&event);
		}
		selectionnotifier->remove_manager_callback();
	}

	return 0;
}

char *cmzn_selectionevent_change_type_enum_to_string(enum cmzn_selectionevent_change_type type)
{
	char *string = NULL;
	if (0 <= type && type <= 4)
	{
		const char *str[] = {"CHANGE_NONE", "CHANGE_CLEAR", "CHANGE_ADD", "CHANGE_REMOVE", "CHANGE_REPLACE"};
		string = duplicate_string(str[type - 1]);
	}
	return string;
}
