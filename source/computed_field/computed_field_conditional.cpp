/*******************************************************************************
FILE : computed_field_conditional.cpp

LAST MODIFIED : 16 May 2008

DESCRIPTION :
Implements a number of basic component wise operations on computed fields.
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
#include <math.h>
#include "computed_field/computed_field.h"
}
#include "computed_field/computed_field_private.hpp"
extern "C" {
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_conditional.h"
}

class Computed_field_conditional_package : public Computed_field_type_package
{
public:
	struct MANAGER(Computed_field) *computed_field_manager;
};

namespace {

char computed_field_if_type_string[] = "if";

class Computed_field_if : public Computed_field_core
{
public:
	Computed_field_if(Computed_field *field) : Computed_field_core(field)
	{
	};

private:
	Computed_field_core *copy(Computed_field* new_parent)
	{
		return new Computed_field_if(new_parent);
	}

	char *get_type_string()
	{
		return(computed_field_if_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_if*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();
};

int Computed_field_if::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 27 July 2007

DESCRIPTION :
Evaluate the fields cache at the location.
==============================================================================*/
{
	FE_value *derivative;
	int source_field, calculate_field_two, calculate_field_three, i, j, number_of_xi,
		return_code;

	ENTER(Computed_field_if::evaluate_cache_at_location);
	if (field && location && (field->number_of_source_fields > 0) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate only the source fields */
		if (return_code = 
			Computed_field_evaluate_cache_at_location(field->source_fields[0],
				location))
		{
			/* 2. Work out whether we need to evaluate source_field_two
			 or source_field_three or both. */
			calculate_field_two = 0;
			calculate_field_three = 0;
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				if (field->source_fields[0]->values[i])
				{
					calculate_field_two = 1;
				}
				else
				{
					calculate_field_three = 1;
				}
			}
			if (calculate_field_two)
			{
				return_code = Computed_field_evaluate_cache_at_location(
					field->source_fields[1], location);
			}
			if (calculate_field_three)
			{
				return_code = Computed_field_evaluate_cache_at_location(
					field->source_fields[2], location);
			}
			number_of_xi = location->get_number_of_derivatives();
			derivative = field->derivatives;
			field->derivatives_valid = 1;
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				if (field->source_fields[0]->values[i])
				{
					source_field = 1;
				}
				else
				{
					source_field = 2;
				}
				field->values[i] = field->source_fields[source_field]->values[i];
				if (field->source_fields[source_field]->derivatives_valid)
				{
					for (j = 0 ; j < number_of_xi ; j++)
					{
						*derivative = field->source_fields[source_field]
							->derivatives[i * number_of_xi + j];
						derivative++;
					}
				}
				else
				{
					field->derivatives_valid = 0;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_if::evaluate_cache_at_location.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_if::evaluate_cache_at_location */


int Computed_field_if::list()
/*******************************************************************************
LAST MODIFIED : 27 July 2007

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_if);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source fields : %s %s\n",field->source_fields[0]->name,
			field->source_fields[1]->name, field->source_fields[2]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_if.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_if */

char *Computed_field_if::get_command_string()
/*******************************************************************************
LAST MODIFIED : 27 July 2007

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_if::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_if_type_string, &error);
		append_string(&command_string, " fields ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, " ", &error);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		if (GET_NAME(Computed_field)(field->source_fields[2], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, " ", &error);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_if::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_if::get_command_string */

} //namespace

Computed_field *Computed_field_create_if(
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two,
	struct Computed_field *source_field_three)
/*******************************************************************************
LAST MODIFIED : 16 May 2008

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_IF with the supplied
fields, <source_field_one>, <source_field_two> and <source_field_three>.
Sets the number of components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields;
	Computed_field *field, **source_fields;

	ENTER(Computed_field_create_if);
	if (source_field_one&&source_field_two&&source_field_three&&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components)&&
		(source_field_one->number_of_components ==
			source_field_three->number_of_components))
	{
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=3;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. create new field */
			field = ACCESS(Computed_field)(CREATE(Computed_field)(""));
			/* 3. establish the new type */
			field->number_of_components = source_field_one->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field_one);
			source_fields[1]=ACCESS(Computed_field)(source_field_two);
			source_fields[2]=ACCESS(Computed_field)(source_field_three);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->core = new Computed_field_if(field);
		}
		else
		{
			DEALLOCATE(source_fields);
			field = (Computed_field *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_if.  Invalid argument(s)");
		field = (Computed_field *)NULL;
	}
	LEAVE;

	return (field);
} /* Computed_field_create_if */

int Computed_field_get_type_if(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two,
	struct Computed_field **source_field_three)
/*******************************************************************************
LAST MODIFIED : 27 July 2007

DESCRIPTION :
If the field is of type COMPUTED_FIELD_IF, the 
<source_field_one>, <source_field_two> and <source_field_three> used by it
are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_if);
	if (field&&(dynamic_cast<Computed_field_if*>(field->core)))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		*source_field_three = field->source_fields[2];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_if.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_if */

int define_Computed_field_type_if(struct Parse_state *state,
	void *field_void,void *computed_field_conditional_package_void)
/*******************************************************************************
LAST MODIFIED : 27 July 2007

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_IF (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	Computed_field_conditional_package 
		*computed_field_conditional_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_if);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_conditional_package=
		(Computed_field_conditional_package *)
		computed_field_conditional_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 3))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			source_fields[2] = (struct Computed_field *)NULL;
			if (computed_field_if_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_if(field, 
					source_fields, source_fields + 1, source_fields + 2);
			}
			if (return_code)
			{
				/* must access objects for set functions */
				if (source_fields[0])
				{
					ACCESS(Computed_field)(source_fields[0]);
				}
				if (source_fields[1])
				{
					ACCESS(Computed_field)(source_fields[1]);
				}
				if (source_fields[2])
				{
					ACCESS(Computed_field)(source_fields[2]);
				}

				option_table = CREATE(Option_table)();
				Option_table_add_help(option_table,
					"The if field uses three input fields.  "
					"The first field is evaluated and for each component "
					"if the value of the component is not zero (== true) then "
					"the value for that component is copied from the second field.  "
					"Otherwise (the first field value was zero == false) "
					"the value for that component is copied from the third field");

				/* fields */
				set_source_field_data.computed_field_manager=
					computed_field_conditional_package->computed_field_manager;
				set_source_field_data.conditional_function=
          Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=3;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = Computed_field_copy_type_specific_and_deaccess(field, Computed_field_create_if(
						source_fields[0], source_fields[1], source_fields[2]));
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_if.  Failed");
					}
				}
				if (source_fields[0])
				{
					DEACCESS(Computed_field)(&source_fields[0]);
				}
				if (source_fields[1])
				{
					DEACCESS(Computed_field)(&source_fields[1]);
				}
				if (source_fields[2])
				{
					DEACCESS(Computed_field)(&source_fields[2]);
				}
				DESTROY(Option_table)(&option_table);
			}
			DEALLOCATE(source_fields);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_if.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_if.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_if */

int Computed_field_register_types_conditional(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 27 July 2007

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	Computed_field_conditional_package
		*computed_field_conditional_package = 
		new Computed_field_conditional_package;

	ENTER(Computed_field_register_types_conditional);
	if (computed_field_package)
	{
		computed_field_conditional_package->computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_if_type_string, 
			define_Computed_field_type_if,
			computed_field_conditional_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_conditional.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_conditional */

