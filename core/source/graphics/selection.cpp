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
 * Portions created by the Initial Developer are Copyright (C) 2010
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

static void  cmzn_selection_handler_callback(
	struct MANAGER_MESSAGE(Computed_field) *message,void *selection_handler_void);

struct cmzn_selection_event
{
	enum cmzn_selection_change_type change_type;
	int owning_scene_destroyed, access_count;

	cmzn_selection_event() :
		change_type(CMZN_SELECTION_NO_CHANGE),
		owning_scene_destroyed(0),
		access_count(1)
	{
	}

	~cmzn_selection_event()
	{
	}
};

struct cmzn_selection_handler
{
	int hierarchical_flag;
	cmzn_scene_id scene;
	cmzn_selection_handler_callback_function function;
	void *user_data;
	void *callback_id;
	int owning_scene_destroyed, access_count;

	cmzn_selection_handler() :
		hierarchical_flag(0),
		scene(NULL),
		function(NULL),
		user_data(NULL),
		callback_id(NULL),
		owning_scene_destroyed(0),
		access_count(1)
	{
	}

	~cmzn_selection_handler()
	{
		clear();
	}

	void set_scene(struct cmzn_scene *scene_in)
	{
		scene = scene_in;
	}

	int set_callback(cmzn_selection_handler_callback_function function_in,
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
					cmzn_selection_handler_callback, (void *)this,	field_manager);
				return_code = (callback_id != NULL);
			}
		}
		return return_code;
	}

	void set_hierarchical(int hierarchical_flag_in)
	{
		 hierarchical_flag = hierarchical_flag_in;
	}

	int get_hierarchical()
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

/***************************************************************************//**
 * Frees the memory for the selection_handlers of <*selection_handler_address>.
 * Sets *selection_handler_address to NULL.
 */
int DESTROY(cmzn_selection_handler)(struct cmzn_selection_handler **selection_handler_address)
{
	int return_code;

	ENTER(DESTROY(cmzn_selection_handler));
	if (selection_handler_address && (*selection_handler_address))
	{
		delete *selection_handler_address;
		*selection_handler_address = NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(cmzn_selection_handler).  Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int DESTROY(cmzn_selection_event)(struct cmzn_selection_event **selection_event_address)
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

static void cmzn_selection_handler_callback(
	struct MANAGER_MESSAGE(Computed_field) *message,void *selection_handler_void)
{
	if (message)
	{
		int selection_changed = 0;
		struct cmzn_selection_handler *selection_handler = (struct cmzn_selection_handler *)selection_handler_void;
		const cmzn_field_change_detail *source_change_detail = NULL;
		if (selection_handler && selection_handler->scene)
		{
			cmzn_field_group_id group_field = cmzn_scene_get_selection_group(selection_handler->scene);
			if (group_field)
			{
				int change = Computed_field_manager_message_get_object_change_and_detail(
					message, cmzn_field_group_base_cast(group_field), &source_change_detail);
				cmzn_selection_event_id event = new cmzn_selection_event();
				event->change_type = CMZN_SELECTION_NO_CHANGE;
				if (change & MANAGER_CHANGE_RESULT(Computed_field))
				{
					selection_changed = 1;
				}
				else if (change & MANAGER_CHANGE_REMOVE(Computed_field))
				{
					selection_changed = 1;
					event->change_type = CMZN_SELECTION_CLEAR;
				}
				else if (change & MANAGER_CHANGE_ADD(Computed_field))
				{
					selection_changed = 1;
					event->change_type = CMZN_SELECTION_ADD;
				}
				if (selection_changed)
				{
					if (source_change_detail)
					{
						const cmzn_field_group_base_change_detail *group_change_detail =
							dynamic_cast<const cmzn_field_group_base_change_detail *>(source_change_detail);
						cmzn_field_group_change_type group_change = CMZN_FIELD_GROUP_NO_CHANGE;
						if (selection_handler->hierarchical_flag)
							group_change = group_change_detail->getChange();
						else
							group_change = group_change_detail->getLocalChange();
						switch (group_change)
						{
							case CMZN_FIELD_GROUP_CLEAR:
							{
								event->change_type = CMZN_SELECTION_CLEAR;
							} break;
							case CMZN_FIELD_GROUP_ADD:
							{
								event->change_type = CMZN_SELECTION_ADD;
							} break;
							case CMZN_FIELD_GROUP_REMOVE:
							{
								event->change_type = CMZN_SELECTION_REMOVE;
							} break;
							case CMZN_FIELD_GROUP_REPLACE:
							{
								event->change_type = CMZN_SELECTION_REPLACE;
							} break;
							default:
							{
								event->change_type = CMZN_SELECTION_NO_CHANGE;
							} break;
						}
					}
					event->owning_scene_destroyed = selection_handler->owning_scene_destroyed;
					if (event->change_type != CMZN_SELECTION_NO_CHANGE)
						(selection_handler->function)(event, selection_handler->user_data);
				}
				cmzn_selection_event_destroy(&event);
				cmzn_field_group_destroy(&group_field);
			}
		}
	}
}

int cmzn_selection_handler_set_scene(cmzn_selection_handler_id selection_handler,
	struct cmzn_scene *scene_in)
{
	if (selection_handler && scene_in)
	{
		selection_handler->set_scene(scene_in);
		return 1;
	}

	return 0;
}

int cmzn_selection_handler_get_hierarchical(cmzn_selection_handler_id selection_handler)
{
	if (selection_handler)
	{
		return selection_handler->get_hierarchical();
	}
	else
	{
		return 0;
	}
}

int cmzn_selection_handler_set_hierarchical(cmzn_selection_handler_id selection_handler,
	int hierarchical_flag)
{
	if (selection_handler)
	{
		selection_handler->set_hierarchical(hierarchical_flag);
		return 1;
	}

	return 0;
}

int cmzn_selection_handler_set_callback(cmzn_selection_handler_id selection_handler,
	cmzn_selection_handler_callback_function function_in, void *user_data_in)
{
	if (selection_handler && function_in)
	{
		return selection_handler->set_callback(function_in, user_data_in);
	}

	return 0;
}

/***************************************************************************//**
 * Clear callback.
 */
int cmzn_selection_handler_clear_callback(cmzn_selection_handler_id selection_handler)
{
	if (selection_handler)
	{
		return selection_handler->clear();
	}
	return 0;
}

DECLARE_OBJECT_FUNCTIONS(cmzn_selection_handler);

cmzn_selection_handler_id cmzn_selection_handler_access(
	cmzn_selection_handler_id selection_handler)
{
	return ACCESS(cmzn_selection_handler)(selection_handler);
}

int cmzn_selection_handler_destroy(cmzn_selection_handler_id *selection_handler_address)
{
	return DEACCESS(cmzn_selection_handler)(selection_handler_address);
}

DECLARE_OBJECT_FUNCTIONS(cmzn_selection_event);

cmzn_selection_event_id cmzn_selection_event_access(
	cmzn_selection_event_id selection_event)
{
	return ACCESS(cmzn_selection_event)(selection_event);
}

int cmzn_selection_event_destroy(cmzn_selection_event_id *selection_event_address)
{
	return DEACCESS(cmzn_selection_event)(selection_event_address);
}

enum cmzn_selection_change_type cmzn_selection_event_get_change_type(
	cmzn_selection_event_id selection_event)
{
	return selection_event->change_type;
}

int cmzn_selection_event_owning_scene_is_destroyed(
		cmzn_selection_event_id selection_event)
	{
		return selection_event->owning_scene_destroyed;
	}

cmzn_selection_handler_id cmzn_selection_handler_create_private()
{
	cmzn_selection_handler_id handler = new cmzn_selection_handler();
	return handler;
}

int cmzn_selection_handler_scene_destroyed(cmzn_selection_handler_id selection_handler)
{
	if (selection_handler && selection_handler->function)
	{
		selection_handler->owning_scene_destroyed = 1;
		selection_handler->scene = NULL;
		if (selection_handler->function)
		{
			cmzn_selection_event_id event = new cmzn_selection_event();
			event->change_type = CMZN_SELECTION_NO_CHANGE;
			event->owning_scene_destroyed = 1;
			(selection_handler->function)(event, selection_handler->user_data);
			cmzn_selection_event_destroy(&event);
		}
		selection_handler->remove_manager_callback();
	}

	return 0;
}

char *cmzn_selection_event_type_enum_to_string(enum cmzn_selection_change_type type)
{
	char *string = NULL;
	if (0 < type && type <= 4)
	{
		const char *str[] = {"CLEAR", "ADD", "REMOVE", "REPLACE"};
		string = duplicate_string(str[type - 1]);
	}
	return string;
}
