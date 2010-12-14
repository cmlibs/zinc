/***************************************************************************//**
 * tessellation.cpp
 *
 * Objects for describing how elements / continuous field domains are
 * tessellated or sampled into graphics.
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
extern "C" {
#include <cstdlib>
#include "command/parser.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/manager_private.h"
#include "general/mystring.h"
#include "graphics/graphics_module.h"
}
#include "graphics/tessellation.hpp"
extern "C" {
#include "user_interface/message.h"
}

/*
Module types
------------
*/

namespace {

void list_divisions(int size, int *divisions)
{
	for (int i = 0; i < size; i++)
	{
		if (i)
		{
			display_message(INFORMATION_MESSAGE, "*");
		}
		display_message(INFORMATION_MESSAGE, "%d", divisions[i]);
	}
}

}

/***************************************************************************//**
 * Object describing how elements / continuous field domains are tessellated
 * or sampled into graphics.
 */
struct Cmiss_tessellation
{
	char *name;
	/* after clearing in create, following to be modified only by manager */
	struct MANAGER(Cmiss_tessellation) *manager;
	int manager_change_status;
	int minimum_divisions_size;
	int *minimum_divisions;
	int refinement_factors_size;
	int *refinement_factors;
	bool persistent_flag;
	int access_count;

	Cmiss_tessellation() :
		name(NULL),
		manager(NULL),
		manager_change_status(MANAGER_CHANGE_NONE(Cmiss_tessellation)),
		minimum_divisions_size(0),
		minimum_divisions(NULL),
		refinement_factors_size(0),
		refinement_factors(NULL),
		persistent_flag(false),
		access_count(1)
	{
	}

	~Cmiss_tessellation()
	{
		if (name)
		{
			DEALLOCATE(name);
		}
		if (minimum_divisions)
		{
			DEALLOCATE(minimum_divisions);
		}
		if (refinement_factors)
		{
			DEALLOCATE(refinement_factors);
		}
	}

	/** get minimum divisions for a particular dimension >= 0 */
	inline int get_minimum_divisions_value(int dimension)
	{
		if (dimension < minimum_divisions_size)
		{
			return minimum_divisions[dimension];
		}
		else if (minimum_divisions_size)
		{
			return minimum_divisions[minimum_divisions_size - 1];
		}
		return 1;
	}

	/** get refinement_factors value for a particular dimension >= 0 */
	inline int get_refinement_factors_value(int dimension)
	{
		if (dimension < refinement_factors_size)
		{
			return refinement_factors[dimension];
		}
		else if (refinement_factors_size)
		{
			return refinement_factors[refinement_factors_size - 1];
		}
		return 1;
	}

	/** assumes arguments have been checked already */
	int set_minimum_divisions(int dimensions, const int *in_minimum_divisions)
	{
		if (dimensions > minimum_divisions_size)
		{
			int *temp;
			if (!REALLOCATE(temp, minimum_divisions, int, dimensions))
				return 0;
			minimum_divisions = temp;
		}
		else if (dimensions == minimum_divisions_size)
		{
			bool no_change = true;
			for (int i = 0; i < dimensions; i++)
			{
				if (minimum_divisions[i] != in_minimum_divisions[i])
					no_change = false;
			}
			if (no_change)
				return 1;
		}
		minimum_divisions_size = dimensions;
		for (int i = 0; i < dimensions; i++)
		{
			minimum_divisions[i] = in_minimum_divisions[i];
		}
		MANAGED_OBJECT_CHANGE(Cmiss_tessellation)(this,
			MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Cmiss_tessellation));
		return 1;
	}

	/** assumes arguments have been checked already */
	int set_refinement_factors(int dimensions, const int *in_refinement_factors)
	{
		if (dimensions > refinement_factors_size)
		{
			int *temp;
			if (!REALLOCATE(temp, refinement_factors, int, dimensions))
				return 0;
			refinement_factors = temp;
		}
		else if (dimensions == refinement_factors_size)
		{
			bool no_change = true;
			for (int i = 0; i < dimensions; i++)
			{
				if (refinement_factors[i] != in_refinement_factors[i])
					no_change = false;
			}
			if (no_change)
				return 1;
		}
		refinement_factors_size = dimensions;
		for (int i = 0; i < dimensions; i++)
		{
			refinement_factors[i] =  in_refinement_factors[i];
		}
		MANAGED_OBJECT_CHANGE(Cmiss_tessellation)(this,
			MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Cmiss_tessellation));
		return 1;
	}

	void list()
	{
		display_message(INFORMATION_MESSAGE, "gfx define tessellation %s minimum_divisions \"", name);
		if (minimum_divisions_size)
		{
			list_divisions(minimum_divisions_size, minimum_divisions);
		}
		else
		{
			display_message(INFORMATION_MESSAGE, "1");
		}
		display_message(INFORMATION_MESSAGE, "\" refinement_factors \"");
		if (refinement_factors_size)
		{
			list_divisions(refinement_factors_size, refinement_factors);
		}
		else
		{
			display_message(INFORMATION_MESSAGE, "1");
		}
		display_message(INFORMATION_MESSAGE, "\";\n");
	}

}; /* struct Cmiss_tessellation */

FULL_DECLARE_INDEXED_LIST_TYPE(Cmiss_tessellation);

FULL_DECLARE_MANAGER_TYPE_WITH_OWNER(Cmiss_tessellation, Cmiss_graphics_module, void *);

/*
Module functions
----------------
*/

namespace {

/***************************************************************************//**
 * Frees the memory for the tessellations of <*tessellation_address>.
 * Sets *tessellation_address to NULL.
 */
int DESTROY(Cmiss_tessellation)(struct Cmiss_tessellation **tessellation_address)
{
	int return_code;

	ENTER(DESTROY(Cmiss_tessellation));
	if (tessellation_address && (*tessellation_address))
	{
		delete *tessellation_address;
		*tessellation_address = NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Cmiss_tessellation).  Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Cmiss_tessellation,name,const char *,strcmp)

DECLARE_LOCAL_MANAGER_FUNCTIONS(Cmiss_tessellation)

} /* anonymous namespace */

/*
Global functions
----------------
*/

DECLARE_ACCESS_OBJECT_FUNCTION(Cmiss_tessellation)

/***************************************************************************//**
 * Custom version handling persistent_flag.
 */
PROTOTYPE_DEACCESS_OBJECT_FUNCTION(Cmiss_tessellation)
{
	int return_code;
	struct Cmiss_tessellation *object;

	ENTER(DEACCESS(Cmiss_tessellation));
	if (object_address && (object = *object_address))
	{
		(object->access_count)--;
		if (object->access_count <= 0)
		{
			return_code = DESTROY(Cmiss_tessellation)(object_address);
		}
		else if ((!object->persistent_flag) && (object->manager) &&
			((1 == object->access_count) || ((2 == object->access_count) &&
				(MANAGER_CHANGE_NONE(Cmiss_tessellation) != object->manager_change_status))))
		{
			return_code =
				REMOVE_OBJECT_FROM_MANAGER(Cmiss_tessellation)(object, object->manager);
		}
		else
		{
			return_code = 1;
		}
		*object_address = (struct Cmiss_tessellation *)NULL;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DEACCESS(Cmiss_tessellation) */

PROTOTYPE_REACCESS_OBJECT_FUNCTION(Cmiss_tessellation)
{
	int return_code;

	ENTER(REACCESS(Cmiss_tessellation));
	if (object_address)
	{
		return_code = 1;
		if (new_object)
		{
			/* access the new object */
			(new_object->access_count)++;
		}
		if (*object_address)
		{
			/* deaccess the current object */
			DEACCESS(Cmiss_tessellation)(object_address);
		}
		/* point to the new object */
		*object_address = new_object;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"REACCESS(Cmiss_tessellation).  Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* REACCESS(Cmiss_tessellation) */

DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Cmiss_tessellation)

DECLARE_INDEXED_LIST_FUNCTIONS(Cmiss_tessellation)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Cmiss_tessellation,name,const char *,strcmp)
DECLARE_INDEXED_LIST_IDENTIFIER_CHANGE_FUNCTIONS(Cmiss_tessellation,name)

DECLARE_MANAGER_FUNCTIONS(Cmiss_tessellation,manager)
DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Cmiss_tessellation,manager)
DECLARE_MANAGER_IDENTIFIER_WITHOUT_MODIFY_FUNCTIONS(Cmiss_tessellation,name,const char *,manager)
DECLARE_MANAGER_OWNER_FUNCTIONS(Cmiss_tessellation, struct Cmiss_graphics_module)

int Cmiss_tessellation_manager_set_owner_private(struct MANAGER(Cmiss_tessellation) *manager,
	struct Cmiss_graphics_module *graphics_module)
{
	return MANAGER_SET_OWNER(Cmiss_tessellation)(manager, graphics_module);
}

struct Cmiss_tessellation *Cmiss_tessellation_create_private()
{
	return new Cmiss_tessellation();
}

Cmiss_tessellation_id Cmiss_tessellation_access(Cmiss_tessellation_id tessellation)
{
	return ACCESS(Cmiss_tessellation)(tessellation);
}

int Cmiss_tessellation_destroy(Cmiss_tessellation_id *tessellation_address)
{
	return DEACCESS(Cmiss_tessellation)(tessellation_address);
}

char *Cmiss_tessellation_get_name(struct Cmiss_tessellation *tessellation)
{
	char *name = NULL;
	if (tessellation && tessellation->name)
	{
		name = duplicate_string(tessellation->name);
	}
	return name;
}

int Cmiss_tessellation_set_name(struct Cmiss_tessellation *tessellation,
	const char *name)
{
	int return_code = 1;

	if (tessellation && name)
	{
		if ((NULL == tessellation->name) || (strcmp(name, tessellation->name) != 0))
		{
			if ((NULL == tessellation->manager) ||
				(NULL == FIND_BY_IDENTIFIER_IN_MANAGER(Cmiss_tessellation,name)(name,tessellation->manager)))
			{
				char *new_name = duplicate_string(name);
				if (new_name)
				{
					LIST_IDENTIFIER_CHANGE_DATA(Cmiss_tessellation, name) *identifier_change_data = NULL;
					if (tessellation->manager)
					{
						identifier_change_data =
							LIST_BEGIN_IDENTIFIER_CHANGE(Cmiss_tessellation, name)(tessellation);
						if (NULL == identifier_change_data)
						{
							display_message(ERROR_MESSAGE,
								"Cmiss_tessellation_set_name.  "
								"Could not safely change identifier in indexed lists");
							return_code = 0;
						}
					}

					if (return_code)
					{
						DEALLOCATE(tessellation->name);
						tessellation->name = new_name;
					}

					if (tessellation->manager)
					{
						if (!LIST_END_IDENTIFIER_CHANGE(Cmiss_tessellation, name)(
							&identifier_change_data))
						{
							display_message(ERROR_MESSAGE,
								"Cmiss_tessellation_set_name  "
								"Could not restore object to all indexed lists");
						}
						MANAGED_OBJECT_CHANGE(Cmiss_tessellation)(tessellation,
							MANAGER_CHANGE_IDENTIFIER(Cmiss_tessellation));
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
					"Cmiss_tessellation_set_name.  Name '%s' is already in use");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_tessellation_set_name.  Invalid arguments");
		return_code = 0;
	}
	return (return_code);
}

int Cmiss_tessellation_get_persistent(Cmiss_tessellation_id tessellation)
{
	if (tessellation)
	{
		return (int)tessellation->persistent_flag;
	}
	return 0;
}

int Cmiss_tessellation_set_persistent(
	Cmiss_tessellation_id tessellation, int persistent_flag)
{
	if (!tessellation)
		return 0;
	if (tessellation->persistent_flag != (bool)persistent_flag)
	{
		tessellation->persistent_flag = (bool)persistent_flag;
		// A fairly brutal message to send, but usually only called when not in use
		MANAGED_OBJECT_CHANGE(Cmiss_tessellation)(tessellation,
			MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Cmiss_tessellation));
	}
	return 1;
}

int Cmiss_tessellation_get_minimum_divisions_dimensions(
	Cmiss_tessellation_id tessellation)
{
	int dimensions = 0;
	if (tessellation)
	{
		dimensions = tessellation->minimum_divisions_size;
		if (!dimensions)
			dimensions = 1;
	}
	return dimensions;
}

int Cmiss_tessellation_get_minimum_divisions(Cmiss_tessellation_id tessellation,
	int dimensions, int *minimum_divisions)
{
	int return_code = 1;
	if (tessellation && (dimensions > 0) && minimum_divisions)
	{
		for (int i = 0; i < dimensions; i++)
		{
			minimum_divisions[i] = tessellation->get_minimum_divisions_value(i);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_tessellation_get_minimum_divisions.  Invalid arguments");
		return_code = 0;
	}
	return return_code;
}

int Cmiss_tessellation_set_minimum_divisions(Cmiss_tessellation_id tessellation,
	int dimensions, const int *minimum_divisions)
{
	int return_code = 1;
	if (tessellation && (dimensions > 0) && minimum_divisions)
	{
		for (int i = 0; i < dimensions; i++)
		{
			if (minimum_divisions[i] < 1)
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_tessellation_set_minimum_divisions.  "
					"Minimum divisions must be at least 1");
				return_code = 0;
			}
		}
		if (return_code)
		{
			return_code =
				tessellation->set_minimum_divisions(dimensions, minimum_divisions);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_tessellation_set_minimum_divisions.  Invalid arguments");
		return_code = 0;
	}
	return return_code;
}

int Cmiss_tessellation_get_refinement_factors_dimensions(
	Cmiss_tessellation_id tessellation)
{
	int dimensions = 0;
	if (tessellation)
	{
		dimensions = tessellation->refinement_factors_size;
		if (!dimensions)
			dimensions = 1;
	}
	return dimensions;
}

int Cmiss_tessellation_get_refinement_factors(Cmiss_tessellation_id tessellation,
	int dimensions, int *refinement_factors)
{
	int return_code = 1;
	if (tessellation && (dimensions > 0) && refinement_factors)
	{
		for (int i = 0; i < dimensions; i++)
		{
			refinement_factors[i] = tessellation->get_refinement_factors_value(i);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_tessellation_get_refinement_factors.  Invalid arguments");
		return_code = 0;
	}
	return return_code;
}

int Cmiss_tessellation_set_refinement_factors(Cmiss_tessellation_id tessellation,
	int dimensions, const int *refinement_factors)
{
	int return_code = 1;
	if (tessellation && (dimensions > 0) && refinement_factors)
	{
		for (int i = 0; i < dimensions; i++)
		{
			if (refinement_factors[i] < 1)
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_tessellation_set_refinement_factors.  "
					"Minimum divisions must be at least 1");
				return_code = 0;
			}
		}
		if (return_code)
		{
			return_code =
				tessellation->set_refinement_factors(dimensions, refinement_factors);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_tessellation_set_refinement_factors.  Invalid arguments");
		return_code = 0;
	}
	return return_code;
}

/***************************************************************************//**
 * Internal function returning true if the tessellation has coarse and fine
 * divisions both equal to the fixed divisions supplied.
 *
 * @param tessellation  The tessellation to query.
 * @param dimensions  The size of the fixed_divisions array.
 * @param fixed_divisions  Array of divisions to match.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_tessellation_has_fixed_divisions(Cmiss_tessellation_id tessellation,
	int dimensions, int *fixed_divisions)
{
	int return_code = 0;
	if (tessellation && (dimensions > 0) && fixed_divisions)
	{
		if ((dimensions >= tessellation->minimum_divisions_size) &&
			(dimensions >= tessellation->refinement_factors_size))
		{
			return_code = 1;
			for (int i = 0; i < dimensions; i++)
			{
				if ((tessellation->get_minimum_divisions_value(i) != fixed_divisions[i]) ||
					(tessellation->get_refinement_factors_value(i) != 1))
				{
					return_code = 0;
					break;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_tessellation_has_fixed_divisions.  Invalid arguments");
	}
	return return_code;
}

int string_to_divisions(const char *input, int **values_in, int *size_in)
{
	int return_code = 1;
	int *values = NULL;
	const char *str = input;
	int size = 0;
	while (input)
	{
		char *end = NULL;
		int value = (int)strtol(str, &end, /*base*/10);
		if (value <= 0)
		{
			display_message(ERROR_MESSAGE,
					"Non-positive or missing integer in string: %s", input);
			return_code = 0;
			break;
		}
		while (*end == ' ')
		{
			end++;
		}
		size++;
		int *temp;
		if (!REALLOCATE(temp, values, int, size))
		{
			return_code = 0;
			break;
		}
		values = temp;
		values[size - 1] = value;
		if (*end == '\0')
		{
			break;
		}
		if (*end == '*')
		{
			end++;
		}
		else
		{
			display_message(ERROR_MESSAGE,
					"Invalid character \'%c' where * expected", *end);
			return_code = 0;
			break;
		}
		str = end;
	}
	*size_in = size;
	*values_in = values;

	return return_code;
}


namespace {

/***************************************************************************//**
 * Modifier function for setting positive numbers of divisions separated by *.
 *
 * @param divisions_address_void  address of where to allocate return int array.
 * Must be initialised to NULL or allocated array.
 * @param size_address_void  address of int to receive array size. Must be
 * initialised to size of initial divisions_address, or 0 if none.
 */
int set_divisions(struct Parse_state *state,
	void *divisions_address_void, void *size_address_void)
{
	int return_code = 1;

	int **divisions_address = (int **)divisions_address_void;
	int *size_address = (int *)size_address_void;
	if (state && divisions_address && size_address &&
		((NULL == *divisions_address) || (0 < *size_address)))
	{
		const char *current_token = state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING, current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				int size = 0;
				int *values = NULL;
				if (current_token)
				{
					return_code = string_to_divisions(current_token, &values, &size);
				}
				if (!return_code)
				{
					display_parse_state_location(state);
				}
				if (return_code)
				{
					shift_Parse_state(state,1);
					DEALLOCATE(*divisions_address);
					*divisions_address = values;
					*size_address = size;
				}
				else
				{
					DEALLOCATE(values);
				}
			}
			else
			{
				/* write help */
				display_message(INFORMATION_MESSAGE, " \"#*#*...\"");
				if (*divisions_address)
				{
					display_message(INFORMATION_MESSAGE, "[\"");
					list_divisions(*size_address, *divisions_address);
					display_message(INFORMATION_MESSAGE, "\"]");
				}
				display_message(INFORMATION_MESSAGE, "{>=0}");
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing values");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "set_divisions.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

int gfx_define_tessellation_contents(struct Parse_state *state, void *tessellation_void,
	void *graphics_module_void)
{
	int return_code = 1;

	Cmiss_graphics_module *graphics_module = (Cmiss_graphics_module *)graphics_module_void;
	if (state && graphics_module)
	{
		Cmiss_tessellation *tessellation = (Cmiss_tessellation *)tessellation_void; // can be null
		int minimum_divisions_size = 1;
		int refinement_factors_size = 1;
		if (tessellation)
		{
			minimum_divisions_size = Cmiss_tessellation_get_minimum_divisions_dimensions(tessellation);
			refinement_factors_size = Cmiss_tessellation_get_refinement_factors_dimensions(tessellation);
		}
		int *minimum_divisions;
		int *refinement_factors;
		ALLOCATE(minimum_divisions, int, minimum_divisions_size);
		ALLOCATE(refinement_factors, int, refinement_factors_size);
		if (tessellation)
		{
			Cmiss_tessellation_get_minimum_divisions(tessellation, minimum_divisions_size, minimum_divisions);
			Cmiss_tessellation_get_refinement_factors(tessellation, refinement_factors_size, refinement_factors);
		}
		else
		{
			minimum_divisions[0] = 1;
			refinement_factors[0] = 1;
		}
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"Defines tessellation objects which control how finite elements are "
			"subdivided into graphics. The minimum_divisions option gives the "
			"minimum number of linear segments approximating geometry in each xi "
			"dimension of the element. If the coordinate field of a graphic uses "
			"non-linear basis functions the minimum_divisions is multiplied by "
			"the refinement_factors to give the refined number of segments. "
			"Both minimum_divisions and refinement_factors use the last supplied "
			"number for all higher dimensions, so \"4\" = \"4*4\" and so on.");
		Option_table_add_divisions_entry(option_table, "minimum_divisions",
			&minimum_divisions, &minimum_divisions_size);
		Option_table_add_divisions_entry(option_table, "refinement_factors",
			&refinement_factors, &refinement_factors_size);
		return_code = Option_table_multi_parse(option_table,state);
		DESTROY(Option_table)(&option_table);
		if (return_code && tessellation)
		{
			return_code =
				Cmiss_tessellation_set_minimum_divisions(tessellation, minimum_divisions_size, minimum_divisions) &&
				Cmiss_tessellation_set_refinement_factors(tessellation, refinement_factors_size, refinement_factors);
		}
		DEALLOCATE(minimum_divisions);
		DEALLOCATE(refinement_factors);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_define_tessellation_contents.  Invalid arguments");
		return_code = 0;
	}
	return return_code;
}

int list_Cmiss_tessellation_iterator(struct Cmiss_tessellation *tessellation, void *dummy_void)
{
	USE_PARAMETER(dummy_void);
	if (tessellation)
	{
		tessellation->list();
		return 1;
	}
	return 0;
}

/***************************************************************************//**
 * @see Option_table_add_set_Cmiss_tessellation
 */
int set_Cmiss_tessellation(struct Parse_state *state,
	void *tessellation_address_void, void *graphics_module_void)
{
	int return_code = 1;

	Cmiss_tessellation **tessellation_address = (Cmiss_tessellation **)tessellation_address_void;
	Cmiss_graphics_module *graphics_module = (Cmiss_graphics_module *)graphics_module_void;
	if (state && tessellation_address && graphics_module)
	{
		const char *current_token = state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				Cmiss_tessellation *tessellation = NULL;
				if (!fuzzy_string_compare(current_token, "NONE"))
				{
					tessellation = Cmiss_graphics_module_find_tessellation_by_name(graphics_module, current_token);
					if (!tessellation)
					{
						display_message(ERROR_MESSAGE, "Unknown tessellation : %s", current_token);
						display_parse_state_location(state);
						return_code = 0;
					}
				}
				if (return_code)
				{
					REACCESS(Cmiss_tessellation)(tessellation_address, tessellation);
					shift_Parse_state(state,1);
				}
				if (tessellation)
					Cmiss_tessellation_destroy(&tessellation);
			}
			else
			{
				display_message(INFORMATION_MESSAGE," TESSELLATION_NAME|none[%s]",
					(*tessellation_address) ? (*tessellation_address)->name : "none");
			}
		}
		else
		{
			display_message(WARNING_MESSAGE, "Missing tessellation name");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Cmiss_tessellation.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

} // anonymous namespace

int Option_table_add_divisions_entry(struct Option_table *option_table,
	const char *token, int **divisions_address, int *size_address)
{
	int return_code;
	if (option_table && token && divisions_address && size_address)
	{
		return_code = Option_table_add_entry(option_table, token,
			(void *)divisions_address, (void *)size_address, set_divisions);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_divisions_entry.  Invalid argument(s)");
		return_code=0;
	}
	return (return_code);
}

int Option_table_add_Cmiss_tessellation_entry(struct Option_table *option_table,
	const char *token, struct Cmiss_graphics_module *graphics_module,
	struct Cmiss_tessellation **tessellation_address)
{
	int return_code;
	if (option_table && token && graphics_module && tessellation_address)
	{
		return_code = Option_table_add_entry(option_table, token,
			(void *)tessellation_address, (void *)graphics_module,
			set_Cmiss_tessellation);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_Cmiss_tessellation_entry.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int gfx_define_tessellation(struct Parse_state *state, void *dummy_to_be_modified,
	void *graphics_module_void)
{
	int return_code = 1;

	USE_PARAMETER(dummy_to_be_modified);
	Cmiss_graphics_module *graphics_module = (Cmiss_graphics_module *)graphics_module_void;
	if (state && graphics_module)
	{
		const char *current_token = state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				MANAGER(Cmiss_tessellation) *tessellation_manager =
					Cmiss_graphics_module_get_tessellation_manager(graphics_module);
				MANAGER_BEGIN_CACHE(Cmiss_tessellation)(tessellation_manager);
				Cmiss_tessellation *tessellation =
					Cmiss_graphics_module_find_tessellation_by_name(graphics_module, current_token);
				if (!tessellation)
				{
					tessellation = Cmiss_graphics_module_create_tessellation(graphics_module);
					Cmiss_tessellation_set_name(tessellation, current_token);
				}
				shift_Parse_state(state,1);
				if (tessellation)
				{
					// set persistent here so persistent whenever created or edited
					Cmiss_tessellation_set_persistent(tessellation, 1);
					return_code = gfx_define_tessellation_contents(state, (void *)tessellation, graphics_module_void);
				}
				Cmiss_tessellation_destroy(&tessellation);
				MANAGER_END_CACHE(Cmiss_tessellation)(tessellation_manager);
			}
			else
			{
				Option_table *option_table = CREATE(Option_table)();
				Option_table_add_entry(option_table, "TESSELLATION_NAME",
					/*tessellation*/(void *)NULL, graphics_module_void,
					gfx_define_tessellation_contents);
				return_code = Option_table_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing tessellation name");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_define_tessellation.  Invalid arguments");
		return_code = 0;
	}
	return return_code;
}

int gfx_destroy_tessellation(struct Parse_state *state,
	void *dummy_to_be_modified, void *graphics_module_void)
{
	int return_code;
	USE_PARAMETER(dummy_to_be_modified);
	if (state && graphics_module_void)
	{
		Cmiss_tessellation *tessellation = NULL;
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table, /*token*/(const char *)NULL,
			(void *)&tessellation, graphics_module_void, set_Cmiss_tessellation);
		return_code = Option_table_multi_parse(option_table,state);
		if (return_code)
		{
			if (tessellation)
			{
				Cmiss_tessellation_set_persistent(tessellation, 0);
				if (tessellation->access_count > 2)
				{
					display_message(INFORMATION_MESSAGE, "Tessellation marked for destruction when no longer in use.\n");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "Missing tessellation name");
				display_parse_state_location(state);
				return_code = 0;
			}
		}
		if (tessellation)
		{
			Cmiss_tessellation_destroy(&tessellation);
		}
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_destroy_tessellation.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

int gfx_list_tessellation(struct Parse_state *state,
	void *dummy_to_be_modified, void *graphics_module_void)
{
	int return_code;
	USE_PARAMETER(dummy_to_be_modified);
	Cmiss_graphics_module *graphics_module = (Cmiss_graphics_module *)graphics_module_void;
	if (state && graphics_module)
	{
		Cmiss_tessellation *tessellation = NULL;
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table, /*token*/(const char *)NULL,
			(void *)&tessellation, graphics_module_void, set_Cmiss_tessellation);
		return_code = Option_table_multi_parse(option_table,state);
		if (return_code)
		{
			if (tessellation)
			{
				return_code = list_Cmiss_tessellation_iterator(tessellation, (void *)NULL);
			}
			else
			{
				return_code = FOR_EACH_OBJECT_IN_MANAGER(Cmiss_tessellation)(list_Cmiss_tessellation_iterator,
					(void *)NULL, Cmiss_graphics_module_get_tessellation_manager(graphics_module));
			}
		}
		if (tessellation)
		{
			Cmiss_tessellation_destroy(&tessellation);
		}
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_tessellation.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}
