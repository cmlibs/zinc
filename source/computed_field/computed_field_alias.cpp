/*****************************************************************************//**
 * FILE : computed_field_alias.cpp
 * 
 * Implements a cmiss field which is an alias for another field, commonly from a
 * different region to make it available locally.
 *
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
extern "C" {
#include <stdlib.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_alias.h"
}
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_set.h"
extern "C" {
#include "general/debug.h"
#include "general/mystring.h"
#include "region/cmiss_region.h"
#include "user_interface/message.h"
}

/*
Module types
------------
*/

class Computed_field_alias_package : public Computed_field_type_package
{
public:
	Cmiss_region *root_region;

	Computed_field_alias_package(Cmiss_region *root_region)
	  : root_region(root_region)
	{
		ACCESS(Cmiss_region)(root_region);
	}
	
	~Computed_field_alias_package()
	{
		DEACCESS(Cmiss_region)(&root_region);
	}
};

namespace {

char computed_field_alias_type_string[] = "alias";

class Computed_field_alias : public Computed_field_core
{
public:
	void *other_field_manager_callback_id;

	Computed_field_alias(Computed_field* field) : Computed_field_core(field),
		other_field_manager_callback_id(NULL)
	{
	}

	~Computed_field_alias()
	{
		if (other_field_manager_callback_id)
		{
			if (field && (field->number_of_source_fields > 0) && field->source_fields && original_field())
			{
				MANAGER_DEREGISTER(Computed_field)(other_field_manager_callback_id,
					original_field()->manager);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"~Computed_field_alias.  Computed_field source_fields removed before core. Can't get manager of aliased field to end callbacks.");
			}
		}
	}

private:
	inline Computed_field *original_field(void) { return field->source_fields[0]; }

	static void other_field_manager_change(
		MANAGER_MESSAGE(Computed_field) *message, void *alias_field_core_void);

	void check_alias_from_other_manager(void);

	Computed_field_core* copy(Computed_field* new_parent);

	char* get_type_string()
	{
		return (computed_field_alias_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int set_values_at_location(Field_location* location, FE_value *values);
	
	int check_source_fields_manager(MANAGER(Computed_field) **manager_address)
	{
		// nothing to do as source field may be in a different manager
		return 1;
	}

	int manage_source_fields(MANAGER(Computed_field) *manager)
	{
		// nothing to do as source field may be in a different manager
		return 1;
	};

	void field_is_managed(void)
	{
		check_alias_from_other_manager();
	}
};

/***************************************************************************//**
 * Callback for changes in the field manager owning original_field.
 * If this field depends on the change, propagate to this manager as a change to
 * this field.
 */
void Computed_field_alias::other_field_manager_change(
	struct MANAGER_MESSAGE(Computed_field) *message, void *alias_field_core_void)
{
	Computed_field_alias *alias_field_core =
		reinterpret_cast<Computed_field_alias *>(alias_field_core_void);
	Computed_field *field;
	
	if (message && alias_field_core && (field = alias_field_core->field) &&
		(field->number_of_source_fields > 0) && field->source_fields)
	{
		if (Computed_field_depends_on_Computed_field_in_list(
			alias_field_core->original_field(), message->changed_object_list))
		{
			Computed_field_changed(field, field->manager);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_alias::other_field_manager_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Computed_field_alias::other_field_manager_change */

/***************************************************************************//**
 * If original_field is from a different manager to this field, request
 * manager messages to propagate changes to this manager.
 */
void Computed_field_alias::check_alias_from_other_manager(void)
{
	ENTER(Computed_field_alias::check_alias_from_other_manager);
	if (!other_field_manager_callback_id)
	{
		if (field && (field->number_of_source_fields > 0) && field->source_fields &&
			original_field() && original_field()->manager)
		{
			if (field->manager && (field->manager != original_field()->manager))
			{
				// alias from another region: set up manager callbacks
				other_field_manager_callback_id = MANAGER_REGISTER(Computed_field)(
					other_field_manager_change, (void *)this, original_field()->manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_alias::check_alias_from_other_manager.  Invalid source_fields array.");
		}
	}
	LEAVE;
} /* Computed_field_alias::check_alias_from_other_manager */

/***************************************************************************//**
 * Copy the type specific data used by this type.
 */
Computed_field_core* Computed_field_alias::copy(Computed_field *new_parent)
{
	Computed_field_alias* core;

	ENTER(Computed_field_alias::copy);
	if (new_parent)
	{
		core = new Computed_field_alias(new_parent);
		// this function is used to modify fields already in the manager
		// so must check if callbacks needed from another manager
		core->check_alias_from_other_manager();
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_alias::copy.  Invalid argument.");
		core = (Computed_field_alias*)NULL;
	}
	LEAVE;

	return (core);
} /* Computed_field_alias::copy */

/***************************************************************************//**
 * Compare the type specific data.
 */
int Computed_field_alias::compare(Computed_field_core *other_core)
{
	int return_code;

	ENTER(Computed_field_alias::compare);
	if (field && dynamic_cast<Computed_field_alias*>(other_core))
	{
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_alias::compare */

/***************************************************************************//**
 * Evaluate the values of the field at the supplied location.
 */
int Computed_field_alias::evaluate_cache_at_location(
	Field_location* location)
{
	int i, return_code;

	ENTER(Computed_field_alias::evaluate_cache_at_location);
	if (field && location)
	{
		if (return_code = Computed_field_evaluate_cache_at_location(
			original_field(), location))
		{
			return_code = Computed_field_evaluate_cache_at_location(
				original_field(), location);
			for (i = 0; i < field->number_of_components; i++)
			{
				field->values[i] = original_field()->values[i];
			}
			int number_of_derivatives = location->get_number_of_derivatives();
			if ((0 < number_of_derivatives) && original_field()->derivatives_valid)
			{
				for (i = 0; i < field->number_of_components*number_of_derivatives; i++)
				{
					field->derivatives[i] = original_field()->derivatives[i];
				}
				field->derivatives_valid = 1;
			}
			else
			{
				field->derivatives_valid = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_alias::evaluate_cache_at_location.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_alias::evaluate_cache_at_location */

/***************************************************************************//**
 * Sets values of the original field at the supplied location. 
 */
int Computed_field_alias::set_values_at_location(
	Field_location* location, FE_value *values)
{
	int return_code;
	
	ENTER(Computed_field_alias::set_values_at_location);
	if (field && location && values)
	{
		return_code = Computed_field_set_values_at_location(
			original_field(), location, values);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_alias::set_values_at_location.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_alias::set_values_at_location */

/***************************************************************************//**
 * Writes type-specific details of the field to the console. 
 */
int Computed_field_alias::list()
{
	char *field_name;
	int return_code;
	
	ENTER(List_Computed_field_alias);
	if (field)
	{
		display_message(INFORMATION_MESSAGE, "    Original field : ");
		if (original_field()->manager != field->manager)
		{
			char *path = Cmiss_region_get_path(Computed_field_get_region(original_field()));
			display_message(INFORMATION_MESSAGE, "%s", path);
			DEALLOCATE(path);
		}
		if (GET_NAME(Computed_field)(original_field(), &field_name))
		{
			make_valid_token(&field_name);
			display_message(INFORMATION_MESSAGE, "%s\n", field_name);
			DEALLOCATE(field_name);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_alias.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_alias */

/***************************************************************************//**
 * Returns allocated command string for reproducing this field. Includes type.
 */
char *Computed_field_alias::get_command_string()
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_alias::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string, computed_field_alias_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (original_field()->manager != field->manager)
		{
			char *path = Cmiss_region_get_path(Computed_field_get_region(original_field()));
			append_string(&command_string, path, &error);
			DEALLOCATE(path);
		}
		if (GET_NAME(Computed_field)(original_field(), &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_alias::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_alias::get_command_string */

} //namespace

/**
 * @see Cmiss_field_create_alias
 */
Computed_field *Computed_field_create_alias(Computed_field *original_field)
{
	int number_of_source_fields;
	Computed_field *field, **source_fields;

	ENTER(Computed_field_create_alias);
	field = (Computed_field *)NULL;
	if (original_field)
	{
		int return_code = 1;
		if (!original_field->manager)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_create_alias.  Original field must be in a region");
			return_code = 0;
		}
		if (return_code)
		{
			/* 1. make dynamic allocations for any new type-specific data */
			number_of_source_fields = 1;
			if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
			{
				/* 2. create new field */
				field = ACCESS(Computed_field)(CREATE(Computed_field)(""));
				/* 3. establish the new type */
				field->number_of_components = original_field->number_of_components;
				source_fields[0] = ACCESS(Computed_field)(original_field);
				field->source_fields=source_fields;
				field->number_of_source_fields=number_of_source_fields;
				field->core = new Computed_field_alias(field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_alias.  Invalid argument(s)");
	}
	LEAVE;

	return (field);
} /* Computed_field_create_alias */

/*****************************************************************************//**
 * Command modifier function for defining a field as type computed_field_alias
 * (if not already) and allowing its contents to be modified.
 *
 * @param state  The parse state containing the command tokens.
 * @param field_modify_void  Void pointer to Computed_field_modify_data containing
 *   the field and the region it will be added to.
 * @param computed_field_alias_package_void  Void pointer to
 *   Computed_field_alias_package containing root_region.
 * @return  1 on success, 0 on failure.
 */
int define_Computed_field_type_alias(Parse_state *state,
	void *field_modify_void, void *computed_field_alias_package_void)
{
	int return_code;
	Computed_field *field;
	Computed_field_alias_package *computed_field_alias_package;
	Computed_field_modify_data *field_modify;

	ENTER(define_Computed_field_type_alias);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void)&&
			(field=field_modify->field) &&
		(computed_field_alias_package =
			(Computed_field_alias_package *)computed_field_alias_package_void))
	{
		return_code = 1;
		char *original_field_path_and_name = NULL;

		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"Creates a field which is an alias for another field, which can be from another region. "
			"This is the main mechanism for reusing field definitions from other regions. "
			"The optional region path is an absolute path from the root region if preceded by / "
			"or relative to the current region if not.");
		Option_table_add_string_entry(option_table,"field",
			&original_field_path_and_name, " [[/]REGION_PATH/]FIELD_NAME");
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		/* no errors,not asking for help */
		if (return_code)
		{
			Cmiss_region *root_region = NULL;
			char *region_path = NULL;
			char *remainder = NULL;
			Cmiss_region *region = NULL;
			Computed_field *original_field = (Computed_field *)NULL;
 			
			if (original_field_path_and_name)
			{
				if (original_field_path_and_name[0] == CMISS_REGION_PATH_SEPARATOR_CHAR)
				{
					// absolute path
					root_region = computed_field_alias_package->root_region;
				}
				else
				{
					// relative path
					root_region = field_modify->region;
				}
				if (Cmiss_region_get_partial_region_path(root_region,
					original_field_path_and_name, &region, &region_path, &remainder))
				{
					if (!(original_field = Cmiss_region_find_field_by_name(region, remainder)))
					{
						display_message(ERROR_MESSAGE,
							"gfx define field alias:  Could not find field %s", original_field_path_and_name);
						display_parse_state_location(state);
						return_code = 0;
					}
				}
				else
				{
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx define field alias:  Must specify field to alias");
				display_parse_state_location(state);
				return_code = 0;
			}
			if (return_code)
			{
				return_code = Computed_field_copy_type_specific_and_deaccess(field, 
					Computed_field_create_alias(original_field));
			}
			if (original_field)
			{
				DEACCESS(Computed_field)(&original_field);
			}
			DEALLOCATE(region_path);
			DEALLOCATE(remainder);
		}
		if (!return_code)
		{
			if ((!state->current_token) ||
				(strcmp(PARSER_HELP_STRING, state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
			{
				display_message(ERROR_MESSAGE, "gfx define field alias:  Failed");
			}
		}
		DEALLOCATE(original_field_path_and_name);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_alias.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_alias */

int Computed_field_register_type_alias(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region)
{
	int return_code;
	Computed_field_alias_package *computed_field_alias_package = 
		new Computed_field_alias_package(root_region);

	ENTER(Computed_field_register_type_alias);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_alias_type_string,
			define_Computed_field_type_alias,
			computed_field_alias_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_type_alias.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_type_alias */
