/*******************************************************************************
FILE : computed_field_color_conversions.c

LAST MODIFIED : 24 August 2005

DESCRIPTION :
Implements a number of color conversions on computed fields.
Four methods are developed here: RGB to XYZ, RGB to HSL, RGB to L*a*b*, 
and RGB to GRAYSCALE.
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
#include "image_processing/computed_field_color_conversions.h"

#define my_Min(x,y) ((x) <= (y) ? (x) : (y))
#define my_Max(x,y) ((x) <= (y) ? (y) : (x))
#define BLACK 20.0
#define YELLOW 70.0

struct Computed_field_color_conversions_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
};

static char computed_field_rgb_to_xyz_type_string[] = "rgb_to_xyz";

int Computed_field_is_type_rgb_to_xyz(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_rgb_to_xyz);
	if (field)
	{
		return_code = 
		  (field->type_string == computed_field_rgb_to_xyz_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_rgb_to_xyz.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_rgb_to_xyz */

#define Computed_field_rgb_to_xyz_clear_type_specific \
   Computed_field_default_clear_type_specific
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_rgb_to_xyz_copy_type_specific \
   Computed_field_default_copy_type_specific
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_rgb_to_xyz_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

#define Computed_field_rgb_to_xyz_type_specific_contents_match \
   Computed_field_default_type_specific_contents_match
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_rgb_to_xyz_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_rgb_to_xyz_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_rgb_to_xyz_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_rgb_to_xyz_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_rgb_to_xyz_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_rgb_to_xyz_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
      field->source_fields[0]->number_of_components) &&
      field->source_fields[0]->number_of_components == 3)
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			field->values[0] = 0.412453 * field->source_fields[0]->values[0] + 0.357580 * field->source_fields[0]->values[1] + 0.180423 * field->source_fields[0]->values[2];
			field->values[1] = 0.212671 * field->source_fields[0]->values[0] + 0.715160 * field->source_fields[0]->values[1] + 0.072169 * field->source_fields[0]->values[2];
			field->values[2] = (0.019334 * field->source_fields[0]->values[0] + 0.119193 * field->source_fields[0]->values[1] + 0.950227 * field->source_fields[0]->values[2]) * 0.918483657;			
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_rgb_to_xyz_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_rgb_to_xyz_evaluate_cache_at_node */

static int Computed_field_rgb_to_xyz_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	FE_value *derivative;
	int j, number_of_xi, return_code;

	ENTER(Computed_field_rgb_to_xyz_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields > 0) && 
		(field->number_of_components ==
                 field->source_fields[0]->number_of_components) &&
                 field->source_fields[0]->number_of_components == 3)
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			field->values[0] = 0.412453 * field->source_fields[0]->values[0] + 0.357580 * field->source_fields[0]->values[1] + 0.180423 * field->source_fields[0]->values[2];
			field->values[1] = 0.212671 * field->source_fields[0]->values[0] + 0.715160 * field->source_fields[0]->values[1] + 0.072169 * field->source_fields[0]->values[2];
			field->values[2] = (0.019334 * field->source_fields[0]->values[0] + 0.119193 * field->source_fields[0]->values[1] + 0.950227 * field->source_fields[0]->values[2]) * 0.918483657;	
			if (calculate_derivatives && field->source_fields[0]->derivatives_valid)
			{
                                number_of_xi = get_FE_element_dimension(element);
				derivative = field->derivatives;
				
				for (j = 0 ; j < number_of_xi ; j++)
				{
            
					*derivative = 0.412453 * field->source_fields[0]->derivatives[0 * number_of_xi + j] + 0.357580 * field->source_fields[0]->derivatives[1 * number_of_xi + j] + 0.180423 * field->source_fields[0]->derivatives[2 * number_of_xi + j];
					derivative++;
				}
				for (j = 0 ; j < number_of_xi ; j++)
				{
            
					*derivative = 0.212671 * field->source_fields[0]->derivatives[0 * number_of_xi + j] + 0.715160 * field->source_fields[0]->derivatives[1 * number_of_xi + j] + 0.072169 * field->source_fields[0]->derivatives[2 * number_of_xi + j];
					derivative++;
				}
				for (j = 0 ; j < number_of_xi ; j++)
				{
            
					*derivative = (0.019334 * field->source_fields[0]->derivatives[0 * number_of_xi + j] + 0.119193 * field->source_fields[0]->derivatives[1 * number_of_xi + j] + 0.950227 * field->source_fields[0]->derivatives[2 * number_of_xi + j]) * 0.918483657;
					derivative++;
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
			"Computed_field_rgb_to_xyz_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_rgb_to_xyz_evaluate_cache_in_element */

#define Computed_field_rgb_to_xyz_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_rgb_to_xyz_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_rgb_to_xyz_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_rgb_to_xyz_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_rgb_to_xyz_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_rgb_to_xyz_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_rgb_to_xyz_get_native_resolution \
	(Computed_field_get_native_resolution_function)NULL

static int list_Computed_field_rgb_to_xyz(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_rgb_to_xyz);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_rgb_to_xyz.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_rgb_to_xyz */

static char *Computed_field_rgb_to_xyz_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_rgb_to_xyz_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_rgb_to_xyz_type_string, &error);
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
			"Computed_field_rgb_to_xyz_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_rgb_to_xyz_get_command_string */

#define Computed_field_rgb_to_xyz_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_rgb_to_xyz(struct Computed_field *field,
	struct Computed_field *source_field)
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

	ENTER(Computed_field_set_type_rgb_to_xyz);
	if (field&&source_field&&
		(source_field->number_of_components == 3))
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_rgb_to_xyz_type_string;
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(rgb_to_xyz);
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
			"Computed_field_set_type_rgb_to_xyz.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_rgb_to_xyz */

int Computed_field_get_type_rgb_to_xyz(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
If the field is of type COMPUTED_FIELD_XYZ, the 
<source_field>  used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_rgb_to_xyz);
	if (field&&(field->type_string==computed_field_rgb_to_xyz_type_string))
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_rgb_to_xyz.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_rgb_to_xyz */

static int define_Computed_field_type_rgb_to_xyz(struct Parse_state *state,
	void *field_void,void *computed_field_color_conversions_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_POWER (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	struct Computed_field_color_conversions_package 
		*computed_field_color_conversions_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_rgb_to_xyz);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_color_conversions_package=
		(struct Computed_field_color_conversions_package *)
		computed_field_color_conversions_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			if (computed_field_rgb_to_xyz_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_rgb_to_xyz(field, 
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
					computed_field_color_conversions_package->computed_field_manager;
				set_source_field_data.conditional_function=Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=1;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = Computed_field_set_type_rgb_to_xyz(field,
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
							"define_Computed_field_type_rgb_to_xyz.  Failed");
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
				"define_Computed_field_type_rgb_to_xyz.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_rgb_to_xyz.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_rgb_to_xyz */

static char computed_field_rgb_to_hsl_type_string[] = "rgb_to_hsl";

int Computed_field_is_type_rgb_to_hsl(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_rgb_to_hsl);
	if (field)
	{
		return_code = 
		  (field->type_string == computed_field_rgb_to_hsl_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_rgb_to_hsl.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_rgb_to_hsl */

#define Computed_field_rgb_to_hsl_clear_type_specific \
   Computed_field_default_clear_type_specific
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_rgb_to_hsl_copy_type_specific \
   Computed_field_default_copy_type_specific
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_rgb_to_hsl_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

#define Computed_field_rgb_to_hsl_type_specific_contents_match \
   Computed_field_default_type_specific_contents_match
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_rgb_to_hsl_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_rgb_to_hsl_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_rgb_to_hsl_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_rgb_to_hsl_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_rgb_to_hsl_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;
        FE_value H, S, L, R, G, B, cmax, cmin;
	FE_value Rdelta, Gdelta, Bdelta;
	ENTER(Computed_field_rgb_to_hsl_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
                field->source_fields[0]->number_of_components) &&
                field->source_fields[0]->number_of_components == 3)
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			R = field->source_fields[0]->values[0];
			G = field->source_fields[0]->values[1];
			B = field->source_fields[0]->values[2];
			cmax = my_Max(my_Max(R,G),B);
			cmin = my_Min(my_Min(R,G),B);
			L = (cmax + cmin + 1.0)/2.0;
			if (cmax == cmin)
			{
			        S = 0.0;
				H = 2.0/3.0;
			}
			else 
			{
			        if(L <= 0.5)
				{
				        S = ((cmax-cmin)+(cmax+cmin)/2.0)/(cmax+cmin);
				}
				else
				{
				        S = ((cmax-cmin) + (2.0-cmax-cmin)/2.0)/(2.0-cmax-cmin);
				}
				Rdelta = (((cmax-R)/6.0) + ((cmax-cmin)/2.0) ) / (cmax-cmin);
				Gdelta = (((cmax-G)/6.0) + ((cmax-cmin)/2.0) ) / (cmax-cmin);
				Bdelta = (((cmax-B)/6.0) + ((cmax-cmin)/2.0) ) / (cmax-cmin);
				if (R == cmax)
				{
				        H = Bdelta - Gdelta;
				}
				else if (G == cmax)
				{
				        H = 1.0/3.0 + Rdelta - Bdelta;
				}
				else /* B == cMax */
				{
				        H = 2.0/3.0 + Gdelta - Rdelta;
				}
				if (H > 1.0) 
				{
				        H -= 1.0;
				}
			}
			field->values[0] = L;
			field->values[1] = S;
			field->values[2] = H;		
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_rgb_to_hsl_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_rgb_to_hsl_evaluate_cache_at_node */

static int Computed_field_rgb_to_hsl_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	FE_value H, S, L, R, G, B, cmax, cmin;
	FE_value Rdelta, Gdelta, Bdelta;
	int return_code;

	ENTER(Computed_field_rgb_to_hsl_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields > 0) && 
		(field->number_of_components ==
                 field->source_fields[0]->number_of_components) &&
                 field->source_fields[0]->number_of_components == 3)
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			R = field->source_fields[0]->values[0];
			G = field->source_fields[0]->values[1];
			B = field->source_fields[0]->values[2];
			cmax = my_Max(my_Max(R,G),B);
			cmin = my_Min(my_Min(R,G),B);
			L = (cmax + cmin + 1.0)/2.0;
			if (cmax == cmin)
			{
			        S = 0.0;
				H = 2.0/3.0;
			}
			else 
			{
			        if(L <= 0.5)
				{
				        S = ((cmax-cmin)+(cmax+cmin)/2.0)/(cmax+cmin);
				}
				else
				{
				        S = ((cmax-cmin) + (2.0-cmax-cmin)/2.0)/(2.0-cmax-cmin);
				}
				Rdelta = (((cmax-R)/6.0) + ((cmax-cmin)/2.0) ) / (cmax-cmin);
				Gdelta = (((cmax-G)/6.0) + ((cmax-cmin)/2.0) ) / (cmax-cmin);
				Bdelta = (((cmax-B)/6.0) + ((cmax-cmin)/2.0) ) / (cmax-cmin);
				if (R == cmax)
				{
				        H = Bdelta - Gdelta;
				}
				else if (G == cmax)
				{
				        H = 1.0/3.0 + Rdelta - Bdelta;
				}
				else /* B == cMax */
				{
				        H = 2.0/3.0 + Gdelta - Rdelta;
				}
				if (H > 1.0) 
				{
				        H -= 1.0;
				}
			}
			field->values[0] = L;
			field->values[1] = S;
			field->values[2] = H;	
			if (calculate_derivatives == 0)
			{
				field->derivatives_valid = 0;
			}
			else
			{
				display_message(ERROR_MESSAGE,
				"Computed_field_rgb_to_hsl_evaluate_cache_in_element.  "
				"Cannot calculate derivatives of rgb_to_hsl");
			        return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_rgb_to_hsl_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_rgb_to_hsl_evaluate_cache_in_element */

#define Computed_field_rgb_to_hsl_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_rgb_to_hsl_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_rgb_to_hsl_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_rgb_to_hsl_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_rgb_to_hsl_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_rgb_to_hsl_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_rgb_to_hsl_get_native_resolution \
	(Computed_field_get_native_resolution_function)NULL

static int list_Computed_field_rgb_to_hsl(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_rgb_to_hsl);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_rgb_to_hsl.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_rgb_to_hsl */

static char *Computed_field_rgb_to_hsl_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_rgb_to_hsl_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_rgb_to_hsl_type_string, &error);
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
			"Computed_field_rgb_to_hsl_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_rgb_to_hsl_get_command_string */

#define Computed_field_rgb_to_hsl_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_rgb_to_hsl(struct Computed_field *field,
	struct Computed_field *source_field)
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

	ENTER(Computed_field_set_type_rgb_to_hsl);
	if (field&&source_field&&
		(source_field->number_of_components == 3))
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_rgb_to_hsl_type_string;
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(rgb_to_hsl);
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
			"Computed_field_set_type_rgb_to_hsl.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_rgb_to_hsl */

int Computed_field_get_type_rgb_to_hsl(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
If the field is of type COMPUTED_FIELD_XYZ, the 
<source_field>  used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_rgb_to_hsl);
	if (field&&(field->type_string==computed_field_rgb_to_hsl_type_string))
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_rgb_to_hsl.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_rgb_to_hsl */

static int define_Computed_field_type_rgb_to_hsl(struct Parse_state *state,
	void *field_void,void *computed_field_color_conversions_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_POWER (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	struct Computed_field_color_conversions_package 
		*computed_field_color_conversions_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_rgb_to_hsl);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_color_conversions_package=
		(struct Computed_field_color_conversions_package *)
		computed_field_color_conversions_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			if (computed_field_rgb_to_hsl_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_rgb_to_hsl(field, 
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
					computed_field_color_conversions_package->computed_field_manager;
				set_source_field_data.conditional_function=Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=1;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = Computed_field_set_type_rgb_to_hsl(field,
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
							"define_Computed_field_type_rgb_to_hsl.  Failed");
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
				"define_Computed_field_type_rgb_to_hsl.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_rgb_to_hsl.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_rgb_to_hsl */

static char computed_field_rgb_to_lab_type_string[] = "rgb_to_lab";

int Computed_field_is_type_rgb_to_lab(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_rgb_to_lab);
	if (field)
	{
		return_code = 
		  (field->type_string == computed_field_rgb_to_lab_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_rgb_to_lab.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_rgb_to_lab */

#define Computed_field_rgb_to_lab_clear_type_specific \
   Computed_field_default_clear_type_specific
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_rgb_to_lab_copy_type_specific \
   Computed_field_default_copy_type_specific
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_rgb_to_lab_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

#define Computed_field_rgb_to_lab_type_specific_contents_match \
   Computed_field_default_type_specific_contents_match
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_rgb_to_lab_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_rgb_to_lab_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_rgb_to_lab_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_rgb_to_lab_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_rgb_to_lab_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;
	FE_value R, G, B, X, Y, Z, fX, fY, fZ;
	FE_value L, a, b;

	ENTER(Computed_field_rgb_to_lab_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
                 field->source_fields[0]->number_of_components) &&
                 field->source_fields[0]->number_of_components == 3)
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			R = field->source_fields[0]->values[0];
			G = field->source_fields[0]->values[1];
			B = field->source_fields[0]->values[2];
			
			X = 0.412453*R + 0.357580*G + 0.180423*B;
			Y = 0.212671*R + 0.715160*G + 0.072169*B;
			Z = 0.019334*R + 0.119193*G + 0.950227*B;

			X /= 0.950456;
			Z /= 1.088754;

			if (Y > 0.008856)
			{
				fY = pow(Y, 1.0/3.0);
				L = 116.0*fY - 16.0;
			}
			else
			{
				fY = 7.787*Y + 16.0/116.0;
				L = 903.3*Y;
			}

			if (X > 0.008856)
			{
				fX = pow(X, 1.0/3.0);
			}
			else
			{
				fX = 7.787*X + 16.0/116.0;
			}

			if (Z > 0.008856)
			{
				fZ = pow(Z, 1.0/3.0);
			}
			else
			{
				fZ = 7.787*Z + 16.0/116.0;
			}

			a = 500.0*(fX - fY);
			b = 200.0*(fY - fZ);

			if (L < BLACK) 
			{
				a *= exp((L - BLACK) / (BLACK / 4.0));
				b *= exp((L - BLACK) / (BLACK / 4.0));
				L = BLACK;
			}
			if (b > YELLOW)
			{
				b = YELLOW;		
			}
			field->values[0] = L;
			field->values[1] = a;
			field->values[2] = b;	
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_rgb_to_lab_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_rgb_to_lab_evaluate_cache_at_node */

static int Computed_field_rgb_to_lab_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	int return_code;
	FE_value R, G, B, X, Y, Z, fX, fY, fZ;
	FE_value L, a, b;

	ENTER(Computed_field_rgb_to_lab_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields > 0) && 
		(field->number_of_components ==
                 field->source_fields[0]->number_of_components) &&
                 field->source_fields[0]->number_of_components == 3)
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			R = field->source_fields[0]->values[0];
			G = field->source_fields[0]->values[1];
			B = field->source_fields[0]->values[2];
			
			X = 0.412453*R + 0.357580*G + 0.180423*B;
			Y = 0.212671*R + 0.715160*G + 0.072169*B;
			Z = 0.019334*R + 0.119193*G + 0.950227*B;

			X /= 0.950456;
			Z /= 1.088754;

			if (Y > 0.008856)
			{
				fY = pow(Y, 1.0/3.0);
				L = 116.0*fY - 16.0;
			}
			else
			{
				fY = 7.787*Y + 16.0/116.0;
				L = 903.3*Y;
			}

			if (X > 0.008856)
			{
				fX = pow(X, 1.0/3.0);
			}
			else
			{
				fX = 7.787*X + 16.0/116.0;
			}

			if (Z > 0.008856)
			{
				fZ = pow(Z, 1.0/3.0);
			}
			else
			{
				fZ = 7.787*Z + 16.0/116.0;
			}

			a = 500.0*(fX - fY);
			b = 200.0*(fY - fZ);

			if (L < BLACK) 
			{
				a *= exp((L - BLACK) / (BLACK / 4.0));
				b *= exp((L - BLACK) / (BLACK / 4.0));
				L = BLACK;
			}
			if (b > YELLOW)
			{
				b = YELLOW;		
			}
			field->values[0] = L;
			field->values[1] = a;
			field->values[2] = b;		
			if (calculate_derivatives == 0)
			{
				field->derivatives_valid = 0;
			}
			else
			{
				display_message(ERROR_MESSAGE,
				"Computed_field_rgb_to_lab_evaluate_cache_in_element.  "
				"Cannot calculate derivatives of rgb_to_lab");
			        return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_rgb_to_lab_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_rgb_to_lab_evaluate_cache_in_element */

#define Computed_field_rgb_to_lab_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_rgb_to_lab_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_rgb_to_lab_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_rgb_to_lab_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_rgb_to_lab_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_rgb_to_lab_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_rgb_to_lab_get_native_resolution \
	(Computed_field_get_native_resolution_function)NULL

static int list_Computed_field_rgb_to_lab(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_rgb_to_lab);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_rgb_to_lab.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_rgb_to_lab */

static char *Computed_field_rgb_to_lab_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_rgb_to_lab_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_rgb_to_lab_type_string, &error);
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
			"Computed_field_rgb_to_lab_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_rgb_to_lab_get_command_string */

#define Computed_field_rgb_to_lab_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_rgb_to_lab(struct Computed_field *field,
	struct Computed_field *source_field_one)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_XYZ with the supplied
field, <source_field_one> .  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_rgb_to_lab);
	if (field&&source_field_one&&
		(source_field_one->number_of_components == 3))
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_rgb_to_lab_type_string;
			field->number_of_components = source_field_one->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field_one);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(rgb_to_lab);
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
			"Computed_field_set_type_rgb_to_lab.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_rgb_to_lab */

int Computed_field_get_type_rgb_to_lab(struct Computed_field *field,
	struct Computed_field **source_field_one)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
If the field is of type COMPUTED_FIELD_XYZ, the 
<source_field_one>  used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_rgb_to_lab);
	if (field&&(field->type_string==computed_field_rgb_to_lab_type_string))
	{
		*source_field_one = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_rgb_to_lab.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_rgb_to_lab */

static int define_Computed_field_type_rgb_to_lab(struct Parse_state *state,
	void *field_void,void *computed_field_color_conversions_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_POWER (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	struct Computed_field_color_conversions_package 
		*computed_field_color_conversions_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_rgb_to_lab);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_color_conversions_package=
		(struct Computed_field_color_conversions_package *)
		computed_field_color_conversions_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			if (computed_field_rgb_to_lab_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_rgb_to_lab(field, 
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
					computed_field_color_conversions_package->computed_field_manager;
				set_source_field_data.conditional_function=Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=1;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = Computed_field_set_type_rgb_to_lab(field,
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
							"define_Computed_field_type_rgb_to_lab.  Failed");
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
				"define_Computed_field_type_rgb_to_lab.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_rgb_to_lab.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_rgb_to_lab */


static char computed_field_rgb_to_grayscale_type_string[] = "rgb_to_grayscale";

int Computed_field_is_type_rgb_to_grayscale(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_rgb_to_grayscale);
	if (field)
	{
		return_code = 
		  (field->type_string == computed_field_rgb_to_grayscale_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_rgb_to_grayscale.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_rgb_to_grayscale */

#define Computed_field_rgb_to_grayscale_clear_type_specific \
   Computed_field_default_clear_type_specific
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_rgb_to_grayscale_copy_type_specific \
   Computed_field_default_copy_type_specific
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_rgb_to_grayscale_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

#define Computed_field_rgb_to_grayscale_type_specific_contents_match \
   Computed_field_default_type_specific_contents_match
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_rgb_to_grayscale_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_rgb_to_grayscale_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_rgb_to_grayscale_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_rgb_to_grayscale_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_rgb_to_grayscale_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_rgb_to_grayscale_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 1) && 
		(field->number_of_components ==
                 field->source_fields[0]->number_of_components) &&
                 field->source_fields[0]->number_of_components == 3)
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			for (i = 0; i < field->number_of_components; i++)
			{
			        field->values[i] = 0.299 * field->source_fields[0]->values[0] + 0.587 * field->source_fields[0]->values[1] + 0.114 * field->source_fields[0]->values[2];
			}		
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_rgb_to_grayscale_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_rgb_to_grayscale_evaluate_cache_at_node */

static int Computed_field_rgb_to_grayscale_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Evaluate the fields cache at the element.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_rgb_to_grayscale_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields > 0) && 
		(field->number_of_components ==
                 field->source_fields[0]->number_of_components) &&
                 field->source_fields[0]->number_of_components == 3)
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			for (i = 0; i < field->number_of_components; i++)
			{
			        field->values[i] = 0.299 * field->source_fields[0]->values[0] + 0.587 * field->source_fields[0]->values[1] + 0.114 * field->source_fields[0]->values[2];
			}		
			if (calculate_derivatives == 0)
			{
				field->derivatives_valid = 0;
			}
			else
			{
				display_message(ERROR_MESSAGE,
				"Computed_field_rgb_to_grayscale_evaluate_cache_in_element.  "
				"Cannot calculate derivatives of rgb_to_grayscale");
			        return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_rgb_to_grayscale_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_rgb_to_grayscale_evaluate_cache_in_element */

#define Computed_field_rgb_to_grayscale_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_rgb_to_grayscale_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_rgb_to_grayscale_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_rgb_to_grayscale_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_rgb_to_grayscale_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_rgb_to_grayscale_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_rgb_to_grayscale_get_native_resolution \
	(Computed_field_get_native_resolution_function)NULL

static int list_Computed_field_rgb_to_grayscale(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_rgb_to_grayscale);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_rgb_to_grayscale.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_rgb_to_grayscale */

static char *Computed_field_rgb_to_grayscale_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_rgb_to_grayscale_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_rgb_to_grayscale_type_string, &error);
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
			"Computed_field_rgb_to_grayscale_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_rgb_to_grayscale_get_command_string */

#define Computed_field_rgb_to_grayscale_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_rgb_to_grayscale(struct Computed_field *field,
	struct Computed_field *source_field)
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

	ENTER(Computed_field_set_type_rgb_to_grayscale);
	if (field&&source_field&&
		(source_field->number_of_components == 3))
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_rgb_to_grayscale_type_string;
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(rgb_to_grayscale);
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
			"Computed_field_set_type_rgb_to_grayscale.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_rgb_to_grayscale */

int Computed_field_get_type_rgb_to_grayscale(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
If the field is of type COMPUTED_FIELD_XYZ, the 
<source_field>  used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_rgb_to_grayscale);
	if (field&&(field->type_string==computed_field_rgb_to_grayscale_type_string))
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_rgb_to_grayscale.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_rgb_to_grayscale */

static int define_Computed_field_type_rgb_to_grayscale(struct Parse_state *state,
	void *field_void,void *computed_field_color_conversions_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_POWER (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	struct Computed_field_color_conversions_package 
		*computed_field_color_conversions_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_rgb_to_grayscale);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_color_conversions_package=
		(struct Computed_field_color_conversions_package *)
		computed_field_color_conversions_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			if (computed_field_rgb_to_grayscale_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_rgb_to_grayscale(field, 
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
					computed_field_color_conversions_package->computed_field_manager;
				set_source_field_data.conditional_function=Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=1;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = Computed_field_set_type_rgb_to_grayscale(field,
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
							"define_Computed_field_type_rgb_to_grayscale.  Failed");
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
				"define_Computed_field_type_rgb_to_grayscale.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_rgb_to_grayscale.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_rgb_to_grayscale */


int Computed_field_register_types_color_conversions(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_color_conversions_package 
		computed_field_color_conversions_package;

	ENTER(Computed_field_register_types_color_conversions);
	if (computed_field_package)
	{
		computed_field_color_conversions_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_rgb_to_xyz_type_string, 
			define_Computed_field_type_rgb_to_xyz,
			&computed_field_color_conversions_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_rgb_to_hsl_type_string, 
			define_Computed_field_type_rgb_to_hsl,
			&computed_field_color_conversions_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_rgb_to_lab_type_string, 
			define_Computed_field_type_rgb_to_lab,
			&computed_field_color_conversions_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_rgb_to_grayscale_type_string, 
			define_Computed_field_type_rgb_to_grayscale,
			&computed_field_color_conversions_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_color_conversions.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_color_conversions */

