/*******************************************************************************
FILE : computed_field_trigonometry.c

LAST MODIFIED : 10 June 2004

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
#include <math.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_trigonometry.h"

struct Computed_field_component_operations_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
};

static char computed_field_sin_type_string[] = "sin";

int Computed_field_is_type_sin(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_sin);
	if (field)
	{
		return_code = 
		  (field->type_string == computed_field_sin_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_sin.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_sin */

#define Computed_field_sin_clear_type_specific \
   Computed_field_default_clear_type_specific
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_sin_copy_type_specific \
   Computed_field_default_copy_type_specific
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_sin_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

#define Computed_field_sin_type_specific_contents_match \
   Computed_field_default_type_specific_contents_match
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_sin_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_sin_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_sin_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_sin_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_sin_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_sin_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
					(FE_value)sin((double)(field->source_fields[0]->values[i]));
			}			
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_sin_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_sin_evaluate_cache_at_node */

static int Computed_field_sin_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	FE_value *derivative;
	int i, j, number_of_xi, return_code;

	ENTER(Computed_field_sin_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
					(FE_value)sin((double)(field->source_fields[0]->values[i]));
			}
			if (calculate_derivatives && field->source_fields[0]->derivatives_valid)
			{
				number_of_xi = get_FE_element_dimension(element);
				derivative = field->derivatives;
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					for (j = 0 ; j < number_of_xi ; j++)
					{
						/* d(sin u)/dx = cos u * du/dx */
						*derivative =
							(FE_value)(cos((double)(field->source_fields[0]->values[i])) *
							field->source_fields[0]->derivatives[i * number_of_xi + j]);
						derivative++;
					}
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
			"Computed_field_sin_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_sin_evaluate_cache_in_element */

#define Computed_field_sin_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_sin_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_sin_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_sin_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_sin_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_sin_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_sin_get_native_resolution \
	(Computed_field_get_native_resolution_function)NULL

static int list_Computed_field_sin(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_sin);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_sin.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_sin */

static char *Computed_field_sin_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_sin_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_sin_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_sin_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_sin_get_command_string */

#define Computed_field_sin_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_sin(struct Computed_field *field,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_SIN with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_sin);
	if (field && source_field)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_sin_type_string;
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(sin);
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
			"Computed_field_set_type_sin.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_sin */

int Computed_field_get_type_sin(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
If the field is of type COMPUTED_FIELD_SIN, the 
<source_field> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_sin);
	if (field&&(field->type_string==computed_field_sin_type_string))
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_sin.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_sin */

static int define_Computed_field_type_sin(struct Parse_state *state,
	void *field_void,void *computed_field_component_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_SIN (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	struct Computed_field_component_operations_package 
		*computed_field_component_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_sin);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_component_operations_package=
		(struct Computed_field_component_operations_package *)
		computed_field_component_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			if (computed_field_sin_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_sin(field, 
					source_fields);
			}
			if (return_code)
			{
				/* must access objects for set functions */
				if (source_fields[0])
				{
					ACCESS(Computed_field)(source_fields[0]);
				}

				option_table = CREATE(Option_table)();
				/* fields */
				set_source_field_data.computed_field_manager=
					computed_field_component_operations_package->computed_field_manager;
				set_source_field_data.conditional_function=
					Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=1;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"field",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = Computed_field_set_type_sin(field,
						source_fields[0]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_sin.  Failed");
					}
				}
				if (source_fields[0])
				{
					DEACCESS(Computed_field)(&source_fields[0]);
				}
				DESTROY(Option_table)(&option_table);
			}
			DEALLOCATE(source_fields);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_sin.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_sin.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_sin */

static char computed_field_cos_type_string[] = "cos";

int Computed_field_is_type_cos(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_cos);
	if (field)
	{
		return_code = 
		  (field->type_string == computed_field_cos_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_cos.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_cos */

#define Computed_field_cos_clear_type_specific \
   Computed_field_default_clear_type_specific
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_cos_copy_type_specific \
   Computed_field_default_copy_type_specific
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_cos_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

#define Computed_field_cos_type_specific_contents_match \
   Computed_field_default_type_specific_contents_match
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_cos_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_cos_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_cos_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_cos_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_cos_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_cos_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
					(FE_value)cos((double)(field->source_fields[0]->values[i]));
			}			
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cos_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_cos_evaluate_cache_at_node */

static int Computed_field_cos_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	FE_value *derivative;
	int i, j, number_of_xi, return_code;

	ENTER(Computed_field_cos_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
					(FE_value)cos((double)(field->source_fields[0]->values[i]));
			}
			if (calculate_derivatives && field->source_fields[0]->derivatives_valid)
			{
				number_of_xi = get_FE_element_dimension(element);
				derivative = field->derivatives;
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					for (j = 0 ; j < number_of_xi ; j++)
					{
						/* d(cos u)/dx = -sin u * du/dx */
						*derivative =
							(FE_value)(-sin((double)(field->source_fields[0]->values[i])) *
							field->source_fields[0]->derivatives[i * number_of_xi + j]);
						derivative++;
					}
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
			"Computed_field_cos_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_cos_evaluate_cache_in_element */

#define Computed_field_cos_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_cos_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_cos_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_cos_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_cos_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_cos_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_cos_get_native_resolution \
	(Computed_field_get_native_resolution_function)NULL

static int list_Computed_field_cos(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_cos);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_cos.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_cos */

static char *Computed_field_cos_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_cos_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_cos_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cos_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_cos_get_command_string */

#define Computed_field_cos_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_cos(struct Computed_field *field,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_COS with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_cos);
	if (field && source_field)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_cos_type_string;
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(cos);
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
			"Computed_field_set_type_cos.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_cos */

int Computed_field_get_type_cos(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
If the field is of type COMPUTED_FIELD_COS, the 
<source_field> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_cos);
	if (field&&(field->type_string==computed_field_cos_type_string))
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_cos.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_cos */

static int define_Computed_field_type_cos(struct Parse_state *state,
	void *field_void,void *computed_field_component_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_COS (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	struct Computed_field_component_operations_package 
		*computed_field_component_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_cos);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_component_operations_package=
		(struct Computed_field_component_operations_package *)
		computed_field_component_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			if (computed_field_cos_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_cos(field, 
					source_fields);
			}
			if (return_code)
			{
				/* must access objects for set functions */
				if (source_fields[0])
				{
					ACCESS(Computed_field)(source_fields[0]);
				}

				option_table = CREATE(Option_table)();
				/* fields */
				set_source_field_data.computed_field_manager=
					computed_field_component_operations_package->computed_field_manager;
				set_source_field_data.conditional_function=
					Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=1;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"field",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = Computed_field_set_type_cos(field,
						source_fields[0]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_cos.  Failed");
					}
				}
				if (source_fields[0])
				{
					DEACCESS(Computed_field)(&source_fields[0]);
				}
				DESTROY(Option_table)(&option_table);
			}
			DEALLOCATE(source_fields);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_cos.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_cos.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_cos */

static char computed_field_tan_type_string[] = "tan";

int Computed_field_is_type_tan(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_tan);
	if (field)
	{
		return_code = 
		  (field->type_string == computed_field_tan_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_tan.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_tan */

#define Computed_field_tan_clear_type_specific \
   Computed_field_default_clear_type_specific
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_tan_copy_type_specific \
   Computed_field_default_copy_type_specific
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_tan_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

#define Computed_field_tan_type_specific_contents_match \
   Computed_field_default_type_specific_contents_match
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_tan_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_tan_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_tan_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_tan_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_tan_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_tan_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
					(FE_value)tan((double)(field->source_fields[0]->values[i]));
			}			
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_tan_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_tan_evaluate_cache_at_node */

static int Computed_field_tan_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	FE_value *derivative;
	int i, j, number_of_xi, return_code;

	ENTER(Computed_field_tan_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
					(FE_value)tan((double)(field->source_fields[0]->values[i]));
			}
			if (calculate_derivatives && field->source_fields[0]->derivatives_valid)
			{
				number_of_xi = get_FE_element_dimension(element);
				derivative = field->derivatives;
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					for (j = 0 ; j < number_of_xi ; j++)
					{
						/* d(tan u)/dx = sec^2 u * du/dx */
						*derivative = (FE_value)(field->source_fields[0]->derivatives[i * number_of_xi + j] /
							(cos((double)(field->source_fields[0]->values[i])) * 
							cos((double)(field->source_fields[0]->values[i]))));
						derivative++;
					}
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
			"Computed_field_tan_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_tan_evaluate_cache_in_element */

#define Computed_field_tan_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_tan_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_tan_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_tan_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_tan_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_tan_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_tan_get_native_resolution \
	(Computed_field_get_native_resolution_function)NULL

static int list_Computed_field_tan(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_tan);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_tan.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_tan */

static char *Computed_field_tan_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_tan_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_tan_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_tan_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_tan_get_command_string */

#define Computed_field_tan_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_tan(struct Computed_field *field,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_TAN with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_tan);
	if (field && source_field)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_tan_type_string;
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(tan);
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
			"Computed_field_set_type_tan.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_tan */

int Computed_field_get_type_tan(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
If the field is of type COMPUTED_FIELD_TAN, the 
<source_field> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_tan);
	if (field&&(field->type_string==computed_field_tan_type_string))
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_tan.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_tan */

static int define_Computed_field_type_tan(struct Parse_state *state,
	void *field_void,void *computed_field_component_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_TAN (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	struct Computed_field_component_operations_package 
		*computed_field_component_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_tan);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_component_operations_package=
		(struct Computed_field_component_operations_package *)
		computed_field_component_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			if (computed_field_tan_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_tan(field, 
					source_fields);
			}
			if (return_code)
			{
				/* must access objects for set functions */
				if (source_fields[0])
				{
					ACCESS(Computed_field)(source_fields[0]);
				}

				option_table = CREATE(Option_table)();
				/* fields */
				set_source_field_data.computed_field_manager=
					computed_field_component_operations_package->computed_field_manager;
				set_source_field_data.conditional_function=
					Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=1;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"field",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = Computed_field_set_type_tan(field,
						source_fields[0]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_tan.  Failed");
					}
				}
				if (source_fields[0])
				{
					DEACCESS(Computed_field)(&source_fields[0]);
				}
				DESTROY(Option_table)(&option_table);
			}
			DEALLOCATE(source_fields);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_tan.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_tan.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_tan */

static char computed_field_asin_type_string[] = "asin";

int Computed_field_is_type_asin(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_asin);
	if (field)
	{
		return_code = 
		  (field->type_string == computed_field_asin_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_asin.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_asin */

#define Computed_field_asin_clear_type_specific \
   Computed_field_default_clear_type_specific
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_asin_copy_type_specific \
   Computed_field_default_copy_type_specific
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_asin_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

#define Computed_field_asin_type_specific_contents_match \
   Computed_field_default_type_specific_contents_match
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_asin_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_asin_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_asin_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_asin_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_asin_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_asin_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
					(FE_value)asin((double)(field->source_fields[0]->values[i]));
			}			
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_asin_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_asin_evaluate_cache_at_node */

static int Computed_field_asin_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	FE_value *derivative;
	int i, j, number_of_xi, return_code;

	ENTER(Computed_field_asin_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
					(FE_value)asin((double)(field->source_fields[0]->values[i]));
			}
			if (calculate_derivatives && field->source_fields[0]->derivatives_valid)
			{
				number_of_xi = get_FE_element_dimension(element);
				derivative = field->derivatives;
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					for (j = 0 ; j < number_of_xi ; j++)
					{
						/* d(asin u)/dx = 1.0/sqrt(1.0 - u^2) * du/dx */
						if (field->source_fields[0]->values[i] != 1.0)
						{
							*derivative =
								(FE_value)(field->source_fields[0]->derivatives[i * number_of_xi + j] /
								sqrt(1.0 - ((double)field->source_fields[0]->values[i] * 
								(double)field->source_fields[0]->values[i])));
						}
						else
						{
							*derivative = 0.0;
						}
						derivative++;
					}
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
			"Computed_field_asin_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_asin_evaluate_cache_in_element */

#define Computed_field_asin_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_asin_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_asin_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_asin_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_asin_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_asin_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_asin_get_native_resolution \
	(Computed_field_get_native_resolution_function)NULL

static int list_Computed_field_asin(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_asin);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_asin.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_asin */

static char *Computed_field_asin_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_asin_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_asin_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_asin_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_asin_get_command_string */

#define Computed_field_asin_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_asin(struct Computed_field *field,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_ASIN with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_asin);
	if (field && source_field)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_asin_type_string;
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(asin);
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
			"Computed_field_set_type_asin.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_asin */

int Computed_field_get_type_asin(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
If the field is of type COMPUTED_FIELD_ASIN, the 
<source_field> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_asin);
	if (field&&(field->type_string==computed_field_asin_type_string))
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_asin.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_asin */

static int define_Computed_field_type_asin(struct Parse_state *state,
	void *field_void,void *computed_field_component_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_ASIN (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	struct Computed_field_component_operations_package 
		*computed_field_component_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_asin);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_component_operations_package=
		(struct Computed_field_component_operations_package *)
		computed_field_component_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			if (computed_field_asin_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_asin(field, 
					source_fields);
			}
			if (return_code)
			{
				/* must access objects for set functions */
				if (source_fields[0])
				{
					ACCESS(Computed_field)(source_fields[0]);
				}

				option_table = CREATE(Option_table)();
				/* fields */
				set_source_field_data.computed_field_manager=
					computed_field_component_operations_package->computed_field_manager;
				set_source_field_data.conditional_function=
					Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=1;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"field",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = Computed_field_set_type_asin(field,
						source_fields[0]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_asin.  Failed");
					}
				}
				if (source_fields[0])
				{
					DEACCESS(Computed_field)(&source_fields[0]);
				}
				DESTROY(Option_table)(&option_table);
			}
			DEALLOCATE(source_fields);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_asin.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_asin.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_asin */

static char computed_field_acos_type_string[] = "acos";

int Computed_field_is_type_acos(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_acos);
	if (field)
	{
		return_code = 
		  (field->type_string == computed_field_acos_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_acos.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_acos */

#define Computed_field_acos_clear_type_specific \
   Computed_field_default_clear_type_specific
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_acos_copy_type_specific \
   Computed_field_default_copy_type_specific
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_acos_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

#define Computed_field_acos_type_specific_contents_match \
   Computed_field_default_type_specific_contents_match
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_acos_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_acos_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_acos_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_acos_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_acos_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_acos_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
					(FE_value)acos((double)(field->source_fields[0]->values[i]));
			}			
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_acos_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_acos_evaluate_cache_at_node */

static int Computed_field_acos_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	FE_value *derivative;
	int i, j, number_of_xi, return_code;

	ENTER(Computed_field_acos_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
					(FE_value)acos((double)(field->source_fields[0]->values[i]));
			}
			if (calculate_derivatives && field->source_fields[0]->derivatives_valid)
			{
				number_of_xi = get_FE_element_dimension(element);
				derivative = field->derivatives;
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					for (j = 0 ; j < number_of_xi ; j++)
					{
						/* d(asin u)/dx = -1.0/sqrt(1.0 - u^2) * du/dx */
						if (field->source_fields[0]->values[i] != 1.0)
						{
							*derivative =
								(FE_value)(-field->source_fields[0]->derivatives[i * number_of_xi + j] /
								sqrt(1.0 - ((double)field->source_fields[0]->values[i] * 
								(double)field->source_fields[0]->values[i])));
						}
						else
						{
							*derivative = 0.0;
						}
						derivative++;
					}
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
			"Computed_field_acos_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_acos_evaluate_cache_in_element */

#define Computed_field_acos_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_acos_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_acos_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_acos_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_acos_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_acos_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_acos_get_native_resolution \
	(Computed_field_get_native_resolution_function)NULL

static int list_Computed_field_acos(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_acos);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_acos.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_acos */

static char *Computed_field_acos_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_acos_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_acos_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_acos_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_acos_get_command_string */

#define Computed_field_acos_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_acos(struct Computed_field *field,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_ACOS with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_acos);
	if (field && source_field)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_acos_type_string;
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(acos);
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
			"Computed_field_set_type_acos.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_acos */

int Computed_field_get_type_acos(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
If the field is of type COMPUTED_FIELD_ACOS, the 
<source_field> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_acos);
	if (field&&(field->type_string==computed_field_acos_type_string))
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_acos.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_acos */

static int define_Computed_field_type_acos(struct Parse_state *state,
	void *field_void,void *computed_field_component_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_ACOS (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	struct Computed_field_component_operations_package 
		*computed_field_component_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_acos);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_component_operations_package=
		(struct Computed_field_component_operations_package *)
		computed_field_component_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			if (computed_field_acos_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_acos(field, 
					source_fields);
			}
			if (return_code)
			{
				/* must access objects for set functions */
				if (source_fields[0])
				{
					ACCESS(Computed_field)(source_fields[0]);
				}

				option_table = CREATE(Option_table)();
				/* fields */
				set_source_field_data.computed_field_manager=
					computed_field_component_operations_package->computed_field_manager;
				set_source_field_data.conditional_function=
					Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=1;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"field",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = Computed_field_set_type_acos(field,
						source_fields[0]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_acos.  Failed");
					}
				}
				if (source_fields[0])
				{
					DEACCESS(Computed_field)(&source_fields[0]);
				}
				DESTROY(Option_table)(&option_table);
			}
			DEALLOCATE(source_fields);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_acos.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_acos.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_acos */

static char computed_field_atan_type_string[] = "atan";

int Computed_field_is_type_atan(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_atan);
	if (field)
	{
		return_code = 
		  (field->type_string == computed_field_atan_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_atan.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_atan */

#define Computed_field_atan_clear_type_specific \
   Computed_field_default_clear_type_specific
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_atan_copy_type_specific \
   Computed_field_default_copy_type_specific
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_atan_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

#define Computed_field_atan_type_specific_contents_match \
   Computed_field_default_type_specific_contents_match
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_atan_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_atan_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_atan_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_atan_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_atan_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_atan_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
					(FE_value)atan((double)(field->source_fields[0]->values[i]));
			}			
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_atan_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_atan_evaluate_cache_at_node */

static int Computed_field_atan_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	FE_value *derivative;
	int i, j, number_of_xi, return_code;

	ENTER(Computed_field_atan_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
					(FE_value)atan((double)(field->source_fields[0]->values[i]));
			}
			if (calculate_derivatives && field->source_fields[0]->derivatives_valid)
			{
				number_of_xi = get_FE_element_dimension(element);
				derivative = field->derivatives;
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					for (j = 0 ; j < number_of_xi ; j++)
					{
						/* d(atan u)/dx = 1.0/(1.0 + u^2) * du/dx */
						*derivative =
							(FE_value)(field->source_fields[0]->derivatives[i * number_of_xi + j] /
							(1.0 + ((double)field->source_fields[0]->values[i] * 
							(double)field->source_fields[0]->values[i])));
						derivative++;
					}
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
			"Computed_field_atan_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_atan_evaluate_cache_in_element */

#define Computed_field_atan_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_atan_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_atan_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_atan_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_atan_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_atan_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_atan_get_native_resolution \
	(Computed_field_get_native_resolution_function)NULL

static int list_Computed_field_atan(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_atan);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_atan.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_atan */

static char *Computed_field_atan_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_atan_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_atan_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_atan_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_atan_get_command_string */

#define Computed_field_atan_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_atan(struct Computed_field *field,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_ATAN with the supplied
field, <source_field_one>.  Sets the number of components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_atan);
	if (field && source_field)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_atan_type_string;
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(atan);
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
			"Computed_field_set_type_atan.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_atan */

int Computed_field_get_type_atan(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
If the field is of type COMPUTED_FIELD_ATAN, the 
<source_field> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_atan);
	if (field&&(field->type_string==computed_field_atan_type_string))
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_atan.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_atan */

static int define_Computed_field_type_atan(struct Parse_state *state,
	void *field_void,void *computed_field_component_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_ATAN (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	struct Computed_field_component_operations_package 
		*computed_field_component_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_atan);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_component_operations_package=
		(struct Computed_field_component_operations_package *)
		computed_field_component_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			if (computed_field_atan_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_atan(field, 
					source_fields);
			}
			if (return_code)
			{
				/* must access objects for set functions */
				if (source_fields[0])
				{
					ACCESS(Computed_field)(source_fields[0]);
				}

				option_table = CREATE(Option_table)();
				/* fields */
				set_source_field_data.computed_field_manager=
					computed_field_component_operations_package->computed_field_manager;
				set_source_field_data.conditional_function=
					Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=1;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"field",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = Computed_field_set_type_atan(field,
						source_fields[0]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_atan.  Failed");
					}
				}
				if (source_fields[0])
				{
					DEACCESS(Computed_field)(&source_fields[0]);
				}
				DESTROY(Option_table)(&option_table);
			}
			DEALLOCATE(source_fields);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_atan.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_atan.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_atan */

static char computed_field_atan2_type_string[] = "atan2";

int Computed_field_is_type_atan2(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 02 October 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_atan2);
	if (field)
	{
		return_code = 
		  (field->type_string == computed_field_atan2_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_atan2.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_atan2 */

#define Computed_field_atan2_clear_type_specific \
   Computed_field_default_clear_type_specific
/*******************************************************************************
LAST MODIFIED : 02 October 2003

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_atan2_copy_type_specific \
   Computed_field_default_copy_type_specific
/*******************************************************************************
LAST MODIFIED : 02 October 2003

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_atan2_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 02 October 2003

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

#define Computed_field_atan2_type_specific_contents_match \
   Computed_field_default_type_specific_contents_match
/*******************************************************************************
LAST MODIFIED : 02 October 2003

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_atan2_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 02 October 2003

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_atan2_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 02 October 2003

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_atan2_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 02 October 2003

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_atan2_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 02 October 2003

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_atan2_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 02 October 2003

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_atan2_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 2) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components) && 
		(field->number_of_components ==
      field->source_fields[1]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
          (FE_value)atan2((double)(field->source_fields[0]->values[i]),
            (double)(field->source_fields[1]->values[i]));
			}			
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_atan2_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_atan2_evaluate_cache_at_node */

static int Computed_field_atan2_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 02 October 2003

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	double u, v;
	FE_value *derivative;
	int i, j, number_of_xi, return_code;

	ENTER(Computed_field_atan2_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields > 0) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] =
					(FE_value)atan2((double)(field->source_fields[0]->values[i]),
					(double)(field->source_fields[1]->values[i]));
			}
			if (calculate_derivatives && field->source_fields[0]->derivatives_valid
				&& field->source_fields[1]->derivatives_valid)
			{
				number_of_xi = get_FE_element_dimension(element);
				derivative = field->derivatives;
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					for (j = 0 ; j < number_of_xi ; j++)
					{
						/* d(atan (u/v))/dx =  ( v * du/dx - u * dv/dx ) / ( v^2 * u^2 ) */
						u = field->source_fields[0]->values[i];
						v = field->source_fields[1]->values[i];
						*derivative =
							(FE_value)(v * field->source_fields[0]->derivatives[i * number_of_xi + j] -
							u * field->source_fields[1]->derivatives[i * number_of_xi + j]) /
							(u * u + v * v);
						derivative++;
					}
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
			"Computed_field_atan2_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_atan2_evaluate_cache_in_element */

#define Computed_field_atan2_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 02 October 2003

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_atan2_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 02 October 2003

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_atan2_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 02 October 2003

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_atan2_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 02 October 2003

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_atan2_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 02 October 2003

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_atan2_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 02 October 2003

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_atan2_get_native_resolution \
	(Computed_field_get_native_resolution_function)NULL

static int list_Computed_field_atan2(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 02 October 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_atan2);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source fields : %s %s\n",field->source_fields[0]->name,
			field->source_fields[1]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_atan2.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_atan2 */

static char *Computed_field_atan2_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 02 October 2003

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_atan2_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_atan2_type_string, &error);
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
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_atan2_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_atan2_get_command_string */

#define Computed_field_atan2_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 02 October 2003

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_atan2(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
/*******************************************************************************
LAST MODIFIED : 02 October 2003

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_ATAN2 with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_atan2);
	if (field&&source_field_one&&source_field_two&&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components))
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=2;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_atan2_type_string;
			field->number_of_components = source_field_one->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field_one);
			source_fields[1]=ACCESS(Computed_field)(source_field_two);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(atan2);
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
			"Computed_field_set_type_atan2.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_atan2 */

int Computed_field_get_type_atan2(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 02 October 2003

DESCRIPTION :
If the field is of type COMPUTED_FIELD_ATAN2, the 
<source_field_one> and <source_field_two> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_atan2);
	if (field&&(field->type_string==computed_field_atan2_type_string))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_atan2.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_atan2 */

static int define_Computed_field_type_atan2(struct Parse_state *state,
	void *field_void,void *computed_field_component_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 02 October 2003

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_ATAN2 (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	struct Computed_field_component_operations_package 
		*computed_field_component_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_atan2);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_component_operations_package=
		(struct Computed_field_component_operations_package *)
		computed_field_component_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			if (computed_field_atan2_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_atan2(field, 
					source_fields, source_fields + 1);
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

				option_table = CREATE(Option_table)();
				/* fields */
				set_source_field_data.computed_field_manager=
					computed_field_component_operations_package->computed_field_manager;
				set_source_field_data.conditional_function=
          Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=2;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = Computed_field_set_type_atan2(field,
						source_fields[0], source_fields[1]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_atan2.  Failed");
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
				DESTROY(Option_table)(&option_table);
			}
			DEALLOCATE(source_fields);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_atan2.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_atan2.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_atan2 */

int Computed_field_register_types_trigonometry(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_component_operations_package 
		computed_field_component_operations_package;

	ENTER(Computed_field_register_types_component_operations);
	if (computed_field_package)
	{
		computed_field_component_operations_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_sin_type_string, 
			define_Computed_field_type_sin,
			&computed_field_component_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_cos_type_string, 
			define_Computed_field_type_cos,
			&computed_field_component_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_tan_type_string, 
			define_Computed_field_type_tan,
			&computed_field_component_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_asin_type_string, 
			define_Computed_field_type_asin,
			&computed_field_component_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_acos_type_string, 
			define_Computed_field_type_acos,
			&computed_field_component_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_atan_type_string, 
			define_Computed_field_type_atan,
			&computed_field_component_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_atan2_type_string, 
			define_Computed_field_type_atan2,
			&computed_field_component_operations_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_component_operations.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_trigonometry */

