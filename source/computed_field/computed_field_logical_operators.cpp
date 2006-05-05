/*******************************************************************************
FILE : computed_field_logical_operators.c

LAST MODIFIED : 3 May 2006

DESCRIPTION :
Implements a number of logical operations on computed fields.
Three methods are developed here: AND, OR, XOR, EQUAL_TO, LESS_THAN, GREATER_THAN
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
#include "computed_field/computed_field_logical_operators.h"

struct Computed_field_logical_operators_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
};

static char computed_field_or_type_string[] = "or";

DEFINE_DEFAULT_COMPUTED_FIELD_IS_TYPE_FUNCTION(or)

#define Computed_field_or_clear_type_specific \
   Computed_field_default_clear_type_specific
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_or_copy_type_specific \
   Computed_field_default_copy_type_specific
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_or_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

#define Computed_field_or_type_specific_contents_match \
   Computed_field_default_type_specific_contents_match
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_or_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_or_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_or_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_or_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_or_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_or_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 2) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components) &&
      (field->source_fields[0]->number_of_components == field->source_fields[1]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] = (0.0 != field->source_fields[0]->values[i]) || 
					(0.0 != field->source_fields[1]->values[i]);
			}		
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_or_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_or_evaluate_cache_at_node */

static int Computed_field_or_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	
	int i, return_code;

	ENTER(Computed_field_or_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields == 2) && 
		(field->number_of_components ==
                 field->source_fields[0]->number_of_components) &&
                 (field->source_fields[0]->number_of_components == field->source_fields[1]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] = (0.0 != field->source_fields[0]->values[i]) || 
					(0.0 != field->source_fields[1]->values[i]);
			}	
			if (calculate_derivatives == 0)
			{
				field->derivatives_valid = 0;
			}
			else
			{
				display_message(ERROR_MESSAGE,
				"Computed_field_or_evaluate_cache_in_element.  "
				"Cannot calculate derivatives of or");
			        return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_or_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_or_evaluate_cache_in_element */

#define Computed_field_or_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_or_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_or_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_or_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_or_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_or_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_or_get_native_resolution \
	(Computed_field_get_native_resolution_function)NULL

DEFINE_DEFAULT_LIST_COMPUTED_FIELD_FUNCTION(or, "source fields", "values")

DEFINE_DEFAULT_COMPUTED_FIELD_GET_COMMAND_STRING_FUNCTION(or, \
	"fields", "values");

#define Computed_field_or_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_or(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_XYZ with the supplied
field, <source_field> .  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_or);
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
			field->type_string = computed_field_or_type_string;
			field->number_of_components = source_field_one->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field_one);
			source_fields[1]=ACCESS(Computed_field)(source_field_two);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(or);
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
			"Computed_field_set_type_or.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_or */

int Computed_field_get_type_or(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
If the field is of type COMPUTED_FIELD_XYZ, the 
<source_field>  used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_or);
	if (field&&(field->type_string==computed_field_or_type_string))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_or.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_or */

static int define_Computed_field_type_or(struct Parse_state *state,
	void *field_void,void *computed_field_logical_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_POWER (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	struct Computed_field_logical_operators_package 
		*computed_field_logical_operators_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_or);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_logical_operators_package=
		(struct Computed_field_logical_operators_package *)
		computed_field_logical_operators_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			if (computed_field_or_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_or(field, 
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
					computed_field_logical_operators_package->computed_field_manager;
				set_source_field_data.conditional_function=Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=2;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = Computed_field_set_type_or(field,
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
							"define_Computed_field_type_or.  Failed");
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
				"define_Computed_field_type_or.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_or.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_or */

static char computed_field_and_type_string[] = "and";

DEFINE_DEFAULT_COMPUTED_FIELD_IS_TYPE_FUNCTION(and)

#define Computed_field_and_clear_type_specific \
   Computed_field_default_clear_type_specific
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_and_copy_type_specific \
   Computed_field_default_copy_type_specific
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_and_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

#define Computed_field_and_type_specific_contents_match \
   Computed_field_default_type_specific_contents_match
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_and_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_and_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_and_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_and_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_and_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_and_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 2) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components) &&
      (field->source_fields[0]->number_of_components == field->source_fields[1]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] = (0.0 != field->source_fields[0]->values[i])
					&& (0.0 != field->source_fields[1]->values[i]);
			}		
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_and_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_and_evaluate_cache_at_node */

static int Computed_field_and_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{

	int i, return_code;

	ENTER(Computed_field_and_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields == 2) && 
		(field->number_of_components ==
                 field->source_fields[0]->number_of_components) &&
                 (field->source_fields[0]->number_of_components == field->source_fields[1]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] = (0.0 != field->source_fields[0]->values[i])
					&& (0.0 != field->source_fields[1]->values[i]);
			}	
			if (calculate_derivatives == 0)
			{
				field->derivatives_valid = 0;
			}
			else
			{
				display_message(ERROR_MESSAGE,
				"Computed_field_and_evaluate_cache_in_element.  "
				"Cannot calculate derivatives of and");
			        return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_and_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_and_evaluate_cache_in_element */

#define Computed_field_and_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_and_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_and_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_and_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_and_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_and_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_and_get_native_resolution \
	(Computed_field_get_native_resolution_function)NULL

DEFINE_DEFAULT_LIST_COMPUTED_FIELD_FUNCTION(and, "source fields", "values");

DEFINE_DEFAULT_COMPUTED_FIELD_GET_COMMAND_STRING_FUNCTION(and, \
	"fields", "values");

#define Computed_field_and_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Wandks out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_and(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_AND with the supplied
field, <source_field> .  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_and);
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
			field->type_string = computed_field_and_type_string;
			field->number_of_components = source_field_one->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field_one);
			source_fields[1]=ACCESS(Computed_field)(source_field_two);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(and);
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
			"Computed_field_set_type_and.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_and */

int Computed_field_get_type_and(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
If the field is of type COMPUTED_FIELD_AND, the 
<source_field>  used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_and);
	if (field&&(field->type_string==computed_field_and_type_string))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_and.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_and */

static int define_Computed_field_type_and(struct Parse_state *state,
	void *field_void,void *computed_field_logical_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_AND (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	struct Computed_field_logical_operators_package 
		*computed_field_logical_operators_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_and);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_logical_operators_package=
		(struct Computed_field_logical_operators_package *)
		computed_field_logical_operators_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			if (computed_field_and_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_and(field, 
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
					computed_field_logical_operators_package->computed_field_manager;
				set_source_field_data.conditional_function=Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=2;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errands,not asking for help */
				if (return_code)
				{
					return_code = Computed_field_set_type_and(field,
						source_fields[0], source_fields[1]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* errand */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_and.  Failed");
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
				"define_Computed_field_type_and.  Not enough memandy");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_and.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_and */

static char computed_field_xor_type_string[] = "xor";

DEFINE_DEFAULT_COMPUTED_FIELD_IS_TYPE_FUNCTION(xor)

#define Computed_field_xor_clear_type_specific \
   Computed_field_default_clear_type_specific
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_xor_copy_type_specific \
   Computed_field_default_copy_type_specific
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_xor_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

#define Computed_field_xor_type_specific_contents_match \
   Computed_field_default_type_specific_contents_match
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_xor_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_xor_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_xor_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_xor_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_xor_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_xor_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 2) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components) &&
      (field->source_fields[0]->number_of_components == field->source_fields[1]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				if (0.0 != field->source_fields[0]->values[i])
				{
					field->values[i] = (0.0 == field->source_fields[1]->values[i]);
				}
				else
				{
					field->values[i] = (0.0 != field->source_fields[1]->values[i]);
				}
			}		
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_xor_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_xor_evaluate_cache_at_node */

static int Computed_field_xor_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_xor_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields == 2) && 
		(field->number_of_components ==
                 field->source_fields[0]->number_of_components) &&
                 (field->source_fields[0]->number_of_components == field->source_fields[1]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				if (0.0 != field->source_fields[0]->values[i])
				{
					field->values[i] = (0.0 == field->source_fields[1]->values[i]);
				}
				else
				{
					field->values[i] = (0.0 != field->source_fields[1]->values[i]);
				}
			}	
			if (calculate_derivatives == 0)
			{
				field->derivatives_valid = 0;
			}
			else
			{
				display_message(ERROR_MESSAGE,
				"Computed_field_xor_evaluate_cache_in_element.  "
				"Cannot calculate derivatives of xor");
			        return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_xor_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_xor_evaluate_cache_in_element */

#define Computed_field_xor_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_xor_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_xor_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_xor_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_xor_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_xor_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_xor_get_native_resolution \
	(Computed_field_get_native_resolution_function)NULL

DEFINE_DEFAULT_LIST_COMPUTED_FIELD_FUNCTION(xor, "source fields", "values");

DEFINE_DEFAULT_COMPUTED_FIELD_GET_COMMAND_STRING_FUNCTION(xor, \
	"fields", "values");

#define Computed_field_xor_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Wxorks out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_xor(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_XOR with the supplied
field, <source_field> .  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_xor);
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
			field->type_string = computed_field_xor_type_string;
			field->number_of_components = source_field_one->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field_one);
			source_fields[1]=ACCESS(Computed_field)(source_field_two);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(xor);
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
			"Computed_field_set_type_xor.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_xor */

int Computed_field_get_type_xor(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
If the field is of type COMPUTED_FIELD_XOR, the 
<source_field>  used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_xor);
	if (field&&(field->type_string==computed_field_xor_type_string))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_xor.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_xor */

static int define_Computed_field_type_xor(struct Parse_state *state,
	void *field_void,void *computed_field_logical_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_XOR (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	struct Computed_field_logical_operators_package 
		*computed_field_logical_operators_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_xor);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_logical_operators_package=
		(struct Computed_field_logical_operators_package *)
		computed_field_logical_operators_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			if (computed_field_xor_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_xor(field, 
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
					computed_field_logical_operators_package->computed_field_manager;
				set_source_field_data.conditional_function=Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=2;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errxors,not asking for help */
				if (return_code)
				{
					return_code = Computed_field_set_type_xor(field,
						source_fields[0], source_fields[1]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* errxor */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_xor.  Failed");
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
				"define_Computed_field_type_xor.  Not enough memxory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_xor.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_xor */

static char computed_field_equal_to_type_string[] = "equal_to";

DEFINE_DEFAULT_COMPUTED_FIELD_IS_TYPE_FUNCTION(equal_to)

#define Computed_field_equal_to_clear_type_specific \
   Computed_field_default_clear_type_specific
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_equal_to_copy_type_specific \
   Computed_field_default_copy_type_specific
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_equal_to_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

#define Computed_field_equal_to_type_specific_contents_match \
   Computed_field_default_type_specific_contents_match
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_equal_to_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_equal_to_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_equal_to_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_equal_to_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_equal_to_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_equal_to_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 2) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components) &&
      (field->source_fields[0]->number_of_components == field->source_fields[1]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] = (field->source_fields[0]->values[i]
					== field->source_fields[1]->values[i]);
			}		
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_equal_to_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_equal_to_evaluate_cache_at_node */

static int Computed_field_equal_to_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_equal_to_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields == 2) && 
		(field->number_of_components ==
                 field->source_fields[0]->number_of_components) &&
                 (field->source_fields[0]->number_of_components == field->source_fields[1]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] = (field->source_fields[0]->values[i]
					== field->source_fields[1]->values[i]);
			}	
			if (calculate_derivatives == 0)
			{
				field->derivatives_valid = 0;
			}
			else
			{
				display_message(ERROR_MESSAGE,
				"Computed_field_equal_to_evaluate_cache_in_element.  "
				"Cannot calculate derivatives of equal_to");
			        return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_equal_to_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_equal_to_evaluate_cache_in_element */

#define Computed_field_equal_to_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_equal_to_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_equal_to_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_equal_to_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_equal_to_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_equal_to_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_equal_to_get_native_resolution \
	(Computed_field_get_native_resolution_function)NULL

DEFINE_DEFAULT_LIST_COMPUTED_FIELD_FUNCTION(equal_to, "source fields", "values");

DEFINE_DEFAULT_COMPUTED_FIELD_GET_COMMAND_STRING_FUNCTION(equal_to, \
	"fields", "values");

#define Computed_field_equal_to_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Wequal_toks out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_equal_to(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_EQUAL_TO with the supplied
field, <source_field> .  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_equal_to);
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
			field->type_string = computed_field_equal_to_type_string;
			field->number_of_components = source_field_one->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field_one);
			source_fields[1]=ACCESS(Computed_field)(source_field_two);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(equal_to);
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
			"Computed_field_set_type_equal_to.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_equal_to */

int Computed_field_get_type_equal_to(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
If the field is of type COMPUTED_FIELD_EQUAL_TO, the 
<source_field>  used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_equal_to);
	if (field&&(field->type_string==computed_field_equal_to_type_string))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_equal_to.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_equal_to */

static int define_Computed_field_type_equal_to(struct Parse_state *state,
	void *field_void,void *computed_field_logical_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_EQUAL_TO (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	struct Computed_field_logical_operators_package 
		*computed_field_logical_operators_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_equal_to);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_logical_operators_package=
		(struct Computed_field_logical_operators_package *)
		computed_field_logical_operators_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			if (computed_field_equal_to_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_equal_to(field, 
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
					computed_field_logical_operators_package->computed_field_manager;
				set_source_field_data.conditional_function=Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=2;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errequal_tos,not asking for help */
				if (return_code)
				{
					return_code = Computed_field_set_type_equal_to(field,
						source_fields[0], source_fields[1]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* errequal_to */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_equal_to.  Failed");
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
				"define_Computed_field_type_equal_to.  Not enough memequal_toy");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_equal_to.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_equal_to */

static char computed_field_less_than_type_string[] = "less_than";

DEFINE_DEFAULT_COMPUTED_FIELD_IS_TYPE_FUNCTION(less_than)

#define Computed_field_less_than_clear_type_specific \
   Computed_field_default_clear_type_specific
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_less_than_copy_type_specific \
   Computed_field_default_copy_type_specific
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_less_than_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

#define Computed_field_less_than_type_specific_contents_match \
   Computed_field_default_type_specific_contents_match
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_less_than_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_less_than_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_less_than_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_less_than_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_less_than_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_less_than_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 2) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components) &&
      (field->source_fields[0]->number_of_components == field->source_fields[1]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] = (field->source_fields[0]->values[i]
					== field->source_fields[1]->values[i]);
			}		
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_less_than_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_less_than_evaluate_cache_at_node */

static int Computed_field_less_than_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_less_than_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields == 2) && 
		(field->number_of_components ==
                 field->source_fields[0]->number_of_components) &&
                 (field->source_fields[0]->number_of_components == field->source_fields[1]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] = (field->source_fields[0]->values[i]
					== field->source_fields[1]->values[i]);
			}	
			if (calculate_derivatives == 0)
			{
				field->derivatives_valid = 0;
			}
			else
			{
				display_message(ERROR_MESSAGE,
				"Computed_field_less_than_evaluate_cache_in_element.  "
				"Cannot calculate derivatives of less_than");
			        return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_less_than_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_less_than_evaluate_cache_in_element */

#define Computed_field_less_than_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_less_than_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_less_than_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_less_than_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_less_than_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_less_than_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_less_than_get_native_resolution \
	(Computed_field_get_native_resolution_function)NULL

DEFINE_DEFAULT_LIST_COMPUTED_FIELD_FUNCTION(less_than, "source fields", "values");

DEFINE_DEFAULT_COMPUTED_FIELD_GET_COMMAND_STRING_FUNCTION(less_than, \
	"fields", "values");

#define Computed_field_less_than_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Wless_thanks out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_less_than(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_LESS_THAN with the supplied
field, <source_field> .  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_less_than);
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
			field->type_string = computed_field_less_than_type_string;
			field->number_of_components = source_field_one->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field_one);
			source_fields[1]=ACCESS(Computed_field)(source_field_two);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(less_than);
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
			"Computed_field_set_type_less_than.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_less_than */

int Computed_field_get_type_less_than(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
If the field is of type COMPUTED_FIELD_LESS_THAN, the 
<source_field>  used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_less_than);
	if (field&&(field->type_string==computed_field_less_than_type_string))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_less_than.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_less_than */

static int define_Computed_field_type_less_than(struct Parse_state *state,
	void *field_void,void *computed_field_logical_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_LESS_THAN (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	struct Computed_field_logical_operators_package 
		*computed_field_logical_operators_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_less_than);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_logical_operators_package=
		(struct Computed_field_logical_operators_package *)
		computed_field_logical_operators_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			if (computed_field_less_than_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_less_than(field, 
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
					computed_field_logical_operators_package->computed_field_manager;
				set_source_field_data.conditional_function=Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=2;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errless_thans,not asking for help */
				if (return_code)
				{
					return_code = Computed_field_set_type_less_than(field,
						source_fields[0], source_fields[1]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* errless_than */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_less_than.  Failed");
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
				"define_Computed_field_type_less_than.  Not enough memless_thany");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_less_than.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_less_than */

static char computed_field_greater_than_type_string[] = "greater_than";

DEFINE_DEFAULT_COMPUTED_FIELD_IS_TYPE_FUNCTION(greater_than)

#define Computed_field_greater_than_clear_type_specific \
   Computed_field_default_clear_type_specific
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_greater_than_copy_type_specific \
   Computed_field_default_copy_type_specific
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_greater_than_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

#define Computed_field_greater_than_type_specific_contents_match \
   Computed_field_default_type_specific_contents_match
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_greater_than_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_greater_than_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_greater_than_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_greater_than_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_greater_than_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_greater_than_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 2) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components) &&
      (field->source_fields[0]->number_of_components == field->source_fields[1]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] = (field->source_fields[0]->values[i]
					== field->source_fields[1]->values[i]);
			}		
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_greater_than_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_greater_than_evaluate_cache_at_node */

static int Computed_field_greater_than_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_greater_than_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields == 2) && 
		(field->number_of_components ==
                 field->source_fields[0]->number_of_components) &&
                 (field->source_fields[0]->number_of_components == field->source_fields[1]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] = (field->source_fields[0]->values[i]
					== field->source_fields[1]->values[i]);
			}	
			if (calculate_derivatives == 0)
			{
				field->derivatives_valid = 0;
			}
			else
			{
				display_message(ERROR_MESSAGE,
				"Computed_field_greater_than_evaluate_cache_in_element.  "
				"Cannot calculate derivatives of greater_than");
			        return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_greater_than_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_greater_than_evaluate_cache_in_element */

#define Computed_field_greater_than_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_greater_than_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_greater_than_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_greater_than_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_greater_than_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_greater_than_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_greater_than_get_native_resolution \
	(Computed_field_get_native_resolution_function)NULL

DEFINE_DEFAULT_LIST_COMPUTED_FIELD_FUNCTION(greater_than, "source fields", "values");

DEFINE_DEFAULT_COMPUTED_FIELD_GET_COMMAND_STRING_FUNCTION(greater_than, \
	"fields", "values");

#define Computed_field_greater_than_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Wgreater_thanks out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_greater_than(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_GREATER_THAN with the supplied
field, <source_field> .  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_greater_than);
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
			field->type_string = computed_field_greater_than_type_string;
			field->number_of_components = source_field_one->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field_one);
			source_fields[1]=ACCESS(Computed_field)(source_field_two);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(greater_than);
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
			"Computed_field_set_type_greater_than.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_greater_than */

int Computed_field_get_type_greater_than(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
If the field is of type COMPUTED_FIELD_GREATER_THAN, the 
<source_field>  used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_greater_than);
	if (field&&(field->type_string==computed_field_greater_than_type_string))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_greater_than.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_greater_than */

static int define_Computed_field_type_greater_than(struct Parse_state *state,
	void *field_void,void *computed_field_logical_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_GREATER_THAN (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	struct Computed_field_logical_operators_package 
		*computed_field_logical_operators_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_greater_than);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_logical_operators_package=
		(struct Computed_field_logical_operators_package *)
		computed_field_logical_operators_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			if (computed_field_greater_than_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_greater_than(field, 
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
					computed_field_logical_operators_package->computed_field_manager;
				set_source_field_data.conditional_function=Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=2;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errgreater_thans,not asking for help */
				if (return_code)
				{
					return_code = Computed_field_set_type_greater_than(field,
						source_fields[0], source_fields[1]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* errgreater_than */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_greater_than.  Failed");
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
				"define_Computed_field_type_greater_than.  Not enough memgreater_thany");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_greater_than.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_greater_than */

int Computed_field_register_types_logical_operators(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_logical_operators_package 
		computed_field_logical_operators_package;

	ENTER(Computed_field_register_types_logical_operators);
	if (computed_field_package)
	{
		computed_field_logical_operators_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_or_type_string, 
			define_Computed_field_type_or,
			&computed_field_logical_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_and_type_string, 
			define_Computed_field_type_and,
			&computed_field_logical_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_xor_type_string, 
			define_Computed_field_type_xor,
			&computed_field_logical_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_equal_to_type_string, 
			define_Computed_field_type_equal_to,
			&computed_field_logical_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_less_than_type_string, 
			define_Computed_field_type_less_than,
			&computed_field_logical_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_greater_than_type_string, 
			define_Computed_field_type_greater_than,
			&computed_field_logical_operators_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_logical_operators.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_logical_operators */

