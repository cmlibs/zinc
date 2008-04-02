/*******************************************************************************
FILE : computed_field_function.c

LAST MODIFIED : 31 March 2008

DESCRIPTION :
Implements a "function" computed_field which converts fields, field components
and real values in any order into a single vector field.
==============================================================================*/
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
#include "computed_field/computed_field_function.h"
}
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_set.h"
extern "C" {
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
}

/*
Module types
------------
*/

class Computed_field_function_package : public Computed_field_type_package
{
public:
	struct MANAGER(Computed_field) *computed_field_manager;
};

namespace {

char computed_field_function_type_string[] = "function";

class Computed_field_function : public Computed_field_core
{
public:

	Computed_field_function(Computed_field *field) : Computed_field_core(field)
	{
	}

	~Computed_field_function()
	{
	}

private:
	Computed_field_core* copy(Computed_field* new_parent);

	char* get_type_string()
	{
		return(computed_field_function_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int set_values_at_location(Field_location* location, FE_value *values);

};

Computed_field_core* Computed_field_function::copy(Computed_field *new_parent)
/*******************************************************************************
LAST MODIFIED : 31 March 2008

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	Computed_field_function* core;

	ENTER(Computed_field_function::copy);
	if (new_parent)
	{
		core = new Computed_field_function(new_parent);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_function::copy.  Invalid arguments.");
		core = (Computed_field_function*)NULL;
	}
	LEAVE;

	return (core);
} /* Computed_field_function::copy */

int Computed_field_function::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 31 March 2008

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;
	Computed_field_function* other;

	ENTER(Computed_field_function::compare);
	if (field && (other = dynamic_cast<Computed_field_function*>(other_core)))
	{
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_function::compare */

int Computed_field_function::evaluate_cache_at_location(
	Field_location* location)
/*******************************************************************************
LAST MODIFIED : 31 March 2008

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_function::evaluate_cache_at_location);
	if (field && location)
	{
		/* 1. Precalculate first source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_cache_at_location(field->source_fields[0],
			location))
		{
			if ((field->source_fields[0]->number_of_components ==
				field->source_fields[2]->number_of_components))
			{
				Field_coordinate_location coordinate_location(
					field->source_fields[2],
					field->source_fields[0]->number_of_components,
					field->source_fields[0]->values, location->get_time());
				if (return_code=Computed_field_evaluate_cache_at_location(
						 field->source_fields[1], &coordinate_location))
				{
					/* copy values from cache to <values> */
					for (i=0;i<field->number_of_components;i++)
					{
						field->values[i]=field->source_fields[1]->values[i];
					}
				}
			}
			else
			{
				/* Apply the scalar function operation to each source
					field component */
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					Field_coordinate_location coordinate_location(
						field->source_fields[2],
						1, field->source_fields[0]->values +i,
						location->get_time());
					if (return_code=Computed_field_evaluate_cache_at_location(
						field->source_fields[1], &coordinate_location))
					{
						field->values[i]=field->source_fields[1]->values[0];
					}
				}
			}
			field->derivatives_valid = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_function::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_function::evaluate_cache_at_location */

int Computed_field_function::set_values_at_location(
	Field_location* location, FE_value *values)
/*******************************************************************************
LAST MODIFIED : 31 March 2008

DESCRIPTION :
==============================================================================*/
{
	FE_value *temp_values;
	int i, return_code;
	
	ENTER(Computed_field_function::set_values_at_location);
	if (field && location && values)
	{
		if ((field->source_fields[0]->number_of_components ==
			field->source_fields[2]->number_of_components))
		{
			Field_coordinate_location coordinate_location(field->source_fields[2],
				field->source_fields[0]->number_of_components,
				field->source_fields[0]->values, location->get_time());
			if (return_code=Computed_field_set_values_at_location(
					 field->source_fields[1], &coordinate_location, values))
			{
				return_code = 
					Computed_field_set_values_at_location(field->source_fields[0],
						location, field->source_fields[2]->values);
			}
		}
		else
		{
			ALLOCATE(temp_values, FE_value, field->number_of_components);
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				Field_coordinate_location coordinate_location(
					field->source_fields[2],
					1, temp_values + i, location->get_time());
				if (return_code=Computed_field_set_values_at_location(
					field->source_fields[1], &coordinate_location, values + i))
				{
					temp_values[i] = field->source_fields[2]->values[0];
				}
			}
			if (return_code)
			{
				return_code = 
					Computed_field_set_values_at_location(field->source_fields[0],
						location, temp_values);
			}
			DEALLOCATE(temp_values);
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_function::set_values_at_location.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_function::set_values_at_location.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_function::set_values_at_location */

int Computed_field_function::list()
/*******************************************************************************
LAST MODIFIED : 31 March 2008

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	
	ENTER(List_Computed_field_function);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,
			"    result field : %s\n",field->source_fields[1]->name);
		display_message(INFORMATION_MESSAGE,
			"    reference field : %s\n",field->source_fields[2]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_function.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_function */

char *Computed_field_function::get_command_string()
/*******************************************************************************
LAST MODIFIED : 31 March 2008

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_function::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_function_type_string, &error);

		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " result_field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}

		append_string(&command_string, " reference_field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[2], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_function::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_function::get_command_string */

} //namespace

int Computed_field_set_type_function(Computed_field *field,
	Computed_field *source_field, Computed_field *result_field,
	Computed_field *reference_field)
/*******************************************************************************
LAST MODIFIED : 31 March 2008

DESCRIPTION :
Converts <field> into a function field which returns the values of
<result_field> with respect to the <source_field> values 
being the inputs for <reference_field>.
The sequence of operations <reference_field> to <result_field> 
become a function operating on the input <source_field> values.
Either the number of components in the <source_field> and <reference_field>
should be the same, and then the number of components of this <field>
will be the same as the number of components in the <result_field>,
or if the <reference_field> and <result_field> are scalar then the
function operation will be applied as many times as required for each 
component in the <source_field> and then this <field> will have as many
components as the <source_field>.
==============================================================================*/
{
	int number_of_source_fields, return_code;
	Computed_field **source_fields;

	ENTER(Computed_field_set_type_function);
	if (field&&source_field&&result_field&&reference_field)
	{
		return_code = 1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields = 3;
		if ((source_field->number_of_components ==
			reference_field->number_of_components) ||
			((1 == reference_field->number_of_components)
				&& (1 == result_field->number_of_components)))

		{
			if (ALLOCATE(source_fields, Computed_field *,
					number_of_source_fields))
			{
				/* 2. free current type-specific data */
				Computed_field_clear_type(field);
				/* 3. establish the new type */
				if ((source_field->number_of_components ==
						reference_field->number_of_components))
				{
					field->number_of_components=
						result_field->number_of_components;
				}
				else
				{
					field->number_of_components=
						source_field->number_of_components;
				}
				source_fields[0]=ACCESS(Computed_field)(source_field);
				source_fields[1]=ACCESS(Computed_field)(result_field);
				source_fields[2]=ACCESS(Computed_field)(reference_field);
				field->source_fields=source_fields;
				field->number_of_source_fields=number_of_source_fields;

				field->core = new Computed_field_function(field);
			}
			else
			{
				DEALLOCATE(source_fields);
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_function.  "
				"Incompatible fields");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_function.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_function */

int Computed_field_get_type_function(Computed_field *field,
	Computed_field **source_field, Computed_field **result_field,
	Computed_field **reference_field)
/*******************************************************************************
LAST MODIFIED : 31 March 2008

DESCRIPTION :
If the field is of type COMPUTED_FIELD_FUNCTION, the function returns the three
fields which define the field.
Note that the fields are not ACCESSed and the <region_path> points to the
internally used path.
==============================================================================*/
{
	int return_code;
	Computed_field_function* function_core;

	ENTER(Computed_field_get_type_function);
	if (field && (function_core = dynamic_cast<Computed_field_function*>(field->core)) &&
		source_field && result_field && reference_field)
	{
		*source_field = field->source_fields[0];
		*result_field = field->source_fields[1];
		*reference_field = field->source_fields[2];
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_function.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_function */

int define_Computed_field_type_function(Parse_state *state,
	void *field_void, void *computed_field_function_package_void)
/*******************************************************************************
LAST MODIFIED : 31 March 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_FUNCTION (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	Computed_field *field, *source_field, *result_field,
		*reference_field;
	Computed_field_function_package *computed_field_function_package;
	Option_table *option_table;
	Set_Computed_field_conditional_data set_source_field_data,
		set_result_field_data, set_reference_field_data;

	ENTER(define_Computed_field_type_function);
	if (state && (field = (Computed_field *)field_void) &&
		(computed_field_function_package =
			(Computed_field_function_package *)
			computed_field_function_package_void))
	{
		return_code = 1;
		source_field = (Computed_field *)NULL;
		result_field = (Computed_field *)NULL;
		reference_field = (Computed_field *)NULL;

		if (computed_field_function_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_function(field, 
				&source_field, &result_field,
				&reference_field);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			if (result_field)
			{
				ACCESS(Computed_field)(result_field);
			}
			if (reference_field)
			{
				ACCESS(Computed_field)(reference_field);
			}

			option_table = CREATE(Option_table)();
			Option_table_add_help(option_table,
				"The value of a function field is found by evaluating the <source_field> values, and then evaluating the <result_field> with respect to the <reference_field> using the values from the source field.  The sequence of operations <reference_field> to <result_field> become a function operating on the input <source_field> values.  Either the number of components in the <source_field> and <reference_field> should be the same, and then the number of components of this <field> will be the same as the number of components in the <result_field>, or if the <reference_field> and <result_field> are scalar then the function operation will be applied as many times as required for each component in the <source_field> and then this <field> will have as many components as the <source_field>.");
			/* reference_field */
			set_reference_field_data.computed_field_manager =
				computed_field_function_package->computed_field_manager;
			set_reference_field_data.conditional_function = 
				Computed_field_has_numerical_components;
			set_reference_field_data.conditional_function_user_data = 
				(void *)NULL;
			Option_table_add_entry(option_table, "reference_field", 
				&reference_field, &set_reference_field_data, 
				set_Computed_field_conditional);
			/* result_field */
			set_result_field_data.computed_field_manager =
				computed_field_function_package->computed_field_manager;
			set_result_field_data.conditional_function = 
				Computed_field_has_numerical_components;
			set_result_field_data.conditional_function_user_data = 
				(void *)NULL;
			Option_table_add_entry(option_table, "result_field", 
				&result_field, &set_result_field_data, 
				set_Computed_field_conditional);
			/* source_field */
			set_source_field_data.computed_field_manager =
				computed_field_function_package->computed_field_manager;
			set_source_field_data.conditional_function = 
				Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data = 
				(void *)NULL;
			Option_table_add_entry(option_table, "source_field", 
				&source_field, &set_source_field_data, 
				set_Computed_field_conditional);
			return_code = Option_table_multi_parse(option_table, state);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code=Computed_field_set_type_function(field,
					source_field, result_field, reference_field);
			}
			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_function.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			if (result_field)
			{
				DEACCESS(Computed_field)(&result_field);
			}
			if (reference_field)
			{
				DEACCESS(Computed_field)(&reference_field);
			}
			DESTROY(Option_table)(&option_table);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_function.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_function */

int Computed_field_register_types_function(
	Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 31 March 2008

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	Computed_field_function_package
		*computed_field_function_package = 
		new Computed_field_function_package;

	ENTER(Computed_field_register_types_function);
	if (computed_field_package)
	{
		computed_field_function_package->computed_field_manager =
			Computed_field_package_get_computed_field_manager(computed_field_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_function_type_string,
			define_Computed_field_type_function,
			computed_field_function_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_function.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_function */
