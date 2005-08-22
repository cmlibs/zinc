/*******************************************************************************
FILE : computed_field_kernel_matrix.c

LAST MODIFIED : 26 June 2002

DESCRIPTION :
Implements a number of basic vector operations on computed fields.
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
#include "image_processing/computed_field_kernel_matrix.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"

struct Computed_field_kernel_matrix_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
};

struct Computed_field_kernel_matrix_type_specific_data
/*******************************************************************************
LAST MODIFIED : 10 May 2005

DESCRIPTION :
==============================================================================*/
{
        /* the dimension of the matrix */
        int dimension;
        /* kernel size */
        int radius;
	/* the paprameter for construct Gaussian-relevent kernel */
	double sigma;
	char *filter_name;
	/* cache for kernel, matrix */
	FE_value *a;
}; /* struct Computed_field_kernel_matrix_type_specific_data */

static char computed_field_kernel_matrix_type_string[] = "kernel_matrix";

int Computed_field_is_type_kernel_matrix(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 May 2005

DESCRIPTION :
Returns true if <field> has the appropriate static type string.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_kernel_matrix);
	if (field)
	{
		return_code =
			(field->type_string == computed_field_kernel_matrix_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_kernel_matrix.  Missing field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_kernel_matrix */

int Computed_field_is_type_kernel_matrix_conditional(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 10 May 2005

DESCRIPTION :
List conditional function version of Computed_field_is_type_kernel_matrix.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_kernel_matrix_conditional);
	USE_PARAMETER(dummy_void);
	return_code = Computed_field_is_type_kernel_matrix(field);
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_kernel_matrix_conditional */

static int Computed_field_kernel_matrix_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 11 May 2005

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_kernel_matrix_type_specific_data *data;

	ENTER(Computed_field_kernel_matrix_clear_type_specific);
	if (field && (data =
		(struct Computed_field_kernel_matrix_type_specific_data *)
		field->type_specific_data))
	{
		if (data->a)
		{
			DEALLOCATE(data->a);
		}
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_kernel_matrix_clear_type_specific.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_kernel_matrix_clear_type_specific */

static void *Computed_field_kernel_matrix_copy_type_specific(
	struct Computed_field *source_field, struct Computed_field *destination_field)
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_kernel_matrix_type_specific_data *destination, *source;
	int i, kernel_size;

	ENTER(Computed_field_kernel_matrix_copy_type_specific);
	if (source_field && destination_field && (source =
		(struct Computed_field_kernel_matrix_type_specific_data *)
		source_field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_kernel_matrix_type_specific_data, 1))
		{
			/* following arrays are allocated when field calculated, cleared when
				 cache cleared */
			destination->dimension = source->dimension;
			destination->radius = source->radius;
			destination->sigma = source->sigma;
			destination->filter_name = source->filter_name;
			kernel_size = 1;
			for (i = 0 ; i < source->dimension; i++)
			{
			        kernel_size *= (2 * source->radius + 1);
			}
			for (i = 0; i < kernel_size; i++)
			{
			        destination->a[i] = source->a[i];
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_kernel_matrix_copy_type_specific.  "
				"Unable to allocate memory");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_kernel_matrix_copy_type_specific.  Invalid argument(s)");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_kernel_matrix_copy_type_specific */

static int Computed_field_kernel_matrix_clear_cache_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 May 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_kernel_matrix_type_specific_data *data;

	ENTER(Computed_field_kernel_matrix_clear_cache_type_specific);
	if (field && (data = 
		(struct Computed_field_kernel_matrix_type_specific_data *)
		field->type_specific_data))
	{
		if (data->a)
		{
			DEALLOCATE(data->a);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_kernel_matrix_clear_cache_type_specific.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_kernel_matrix_clear_cache_type_specific */

static int Computed_field_kernel_matrix_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 10 May 2005

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_kernel_matrix_type_specific_contents_match);
	if (field && other_computed_field)
	{
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_kernel_matrix_type_specific_contents_match */

#define Computed_field_kernel_matrix_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 10 May 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_kernel_matrix_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 10 May 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_kernel_matrix_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 10 May 2005

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_kernel_matrix_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 May 2005

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_evaluate_kernel_matrix(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 5 April 2002

DESCRIPTION :
Evaluates the kernel_matrix and eigenvectors of the source field of <field> in
double precision in the type_specific_data then copies the kernel_matrix to the
field->values.
==============================================================================*/
{
	int i, matrix_size, n, return_code;
	int kernel_step, radius, m, k;
	FE_value dist, sum, sg;
	struct Computed_field_kernel_matrix_type_specific_data *data;
	
	ENTER(Computed_field_evaluate_kernel_matrix);
	if (field && (field->type_string == computed_field_kernel_matrix_type_string) &&
		(data = (struct Computed_field_kernel_matrix_type_specific_data *)
			field->type_specific_data) &&
			(data->dimension > 0))
	{	        
	        if (data->radius > 0)
		{
		        radius = data->radius;
		        n = 2 * radius + 1;
			matrix_size = 1;
			for (i = 0; i < data->dimension; i++)
			{
			        matrix_size *= n;
			}
		}
		else
		{
		        radius = 1;
		        n = 3;
			matrix_size = 1;
			for (i = 0; i < data->dimension; i++)
			{
			        matrix_size *= n;
			}
		}	
		
		if ((strcmp(data->filter_name, "low_pass") == 0))
		{
			for (i = 0; i < matrix_size; i++)
			{
				data->a[i] = 1.0 / (FE_value)matrix_size;
			} 
		}
		else if ((strcmp(data->filter_name, "high_pass") == 0))
		{
			for (i = 0; i < matrix_size; i++)
			{
				data->a[i] = -1.0;
			}
			data->a[radius] = (FE_value)matrix_size; 
		}
		else if ((strcmp(data->filter_name, "unsharp_masking") == 0))
		{
			for (i = 0; i < matrix_size; i++)
			{
				data->a[i] = -1.0/(FE_value)matrix_size;
			}
			data->a[radius] = 1.0 - 1.0/(FE_value)matrix_size; 
		}
		else if ((strcmp(data->filter_name, "gaussian") == 0))
		{
			if (data->sigma > 0.0)
			{
				sg = data->sigma;
			}
			else
			{
				sg = 1.0;
			}
			sum = 0.0;
			for (i = 0; i < matrix_size; i++)
			{
				kernel_step = 1;
				dist = 0.0;
				for (m = 0; m < data->dimension; m++)
				{
				        k = ((int)((FE_value)i/((FE_value)kernel_step))) % n;
					dist += (FE_value)(k - radius) * (FE_value)(k -radius);
					kernel_step *= n; 
				}
				data->a[i] = pow(2.7, -0.5*dist/(sg * sg)) / (sg * 2.5);
				sum += data->a[i];
			}
			for (i = 0; i < matrix_size; i++)
			{
				data->a[i] /= sum;
			}      
		}
			
			/* copy the kernel_matrix into the field->values */
		for (i = 0; i < matrix_size; i++)
		{
			field->values[i] = (FE_value)(data->a[i]);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_kernel_matrix.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_evaluate_kernel_matrix */

static int Computed_field_kernel_matrix_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_kernel_matrix_evaluate_cache_at_node);
	if (field && node)
	{
		return_code =
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time) &&
			Computed_field_evaluate_kernel_matrix(field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_kernel_matrix_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_kernel_matrix_evaluate_cache_at_node */

static int Computed_field_kernel_matrix_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Evaluate the fields cache in the element.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_kernel_matrix_evaluate_cache_in_element);
	if (field && element && xi)
	{
		if (0 == calculate_derivatives)
		{
			field->derivatives_valid = 0;
			return_code = Computed_field_evaluate_source_fields_cache_in_element(
				field, element, xi, time, top_level_element, calculate_derivatives)
				&& Computed_field_evaluate_kernel_matrix(field);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_kernel_matrix_evaluate_cache_in_element.  "
				"Cannot calculate derivatives of kernel_matrix");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_kernel_matrix_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_kernel_matrix_evaluate_cache_in_element */

#define Computed_field_kernel_matrix_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 10 May 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_kernel_matrix_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 10 May 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_kernel_matrix_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 May 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_kernel_matrix_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 May 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_kernel_matrix_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 10 May 2005

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_kernel_matrix_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 May 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_kernel_matrix_get_native_resolution \
   (Computed_field_get_native_resolution_function)NULL

static int list_Computed_field_kernel_matrix(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 May 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_kernel_matrix_type_specific_data *data;

	ENTER(List_Computed_field_lernel_matrix);
	if (field && (field->type_string==computed_field_kernel_matrix_type_string)
		&& (data = (struct Computed_field_kernel_matrix_type_specific_data *)
		field->type_specific_data))
	{
		display_message(INFORMATION_MESSAGE,
			"    filter filter_name : %s\n", data->filter_name);
		display_message(INFORMATION_MESSAGE,
			"    filter dimension : %d\n", data->dimension);
		display_message(INFORMATION_MESSAGE,
			"    filter radius : %d\n", data->radius);
		display_message(INFORMATION_MESSAGE,
			"    filter sigma : %f\n", data->sigma);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_kernel_matrix.  Invalid field");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
} /* list_Computed_field_kernel_matrix */

static char *Computed_field_kernel_matrix_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, temp_string[40];
	int error;
	struct Computed_field_kernel_matrix_type_specific_data *data;

	ENTER(Computed_field_kernel_matrix_get_command_string);
	command_string = (char *)NULL;
	if (field&& (field->type_string==computed_field_kernel_matrix_type_string)
		&& (data = (struct Computed_field_kernel_matrix_type_specific_data *)
		field->type_specific_data) )
	{
		error = 0;
		append_string(&command_string,
			computed_field_kernel_matrix_type_string, &error);
		sprintf(temp_string, " dimension %d ", data->dimension);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " filter_name %s ", data->filter_name);
		append_string(&command_string, temp_string, &error);
		
		sprintf(temp_string, " radius %d ", data->radius);
		append_string(&command_string, temp_string, &error);
		
		sprintf(temp_string, " sigma %f ", data->sigma);
		append_string(&command_string, temp_string, &error);
		

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_kernel_matrix_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_kernel_matrix_get_command_string */

#define Computed_field_kernel_matrix_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 11 May 2005

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_kernel_matrix(struct Computed_field *field, int dimension,
          int radius, double sigma, int low_index, int high_index, 
	  int unsharp_index, int gaussian_index)
/*******************************************************************************
LAST MODIFIED : 11 May 2005

DESCRIPTION :
Converts <field> to type 'kernel_matrix' 
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields, return_code;
	int i, n, matrix_size;
	struct Computed_field_kernel_matrix_type_specific_data *data;

	ENTER(Computed_field_set_type_kernel_matrix);
	if (field && (dimension > 0))
	{
	        if (radius > 0)
		{
		        n = 2 * radius + 1;
			matrix_size = 1;
			for (i = 0; i < dimension; i++)
			{
			        matrix_size *= n;
			}
		}
		else
		{
		        n = 3;
			matrix_size = 1;
			for (i = 0; i < dimension; i++)
			{
			        matrix_size *= n;
			}
		}
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields = 0;
		data = (struct Computed_field_kernel_matrix_type_specific_data *)NULL;
		if ( ALLOCATE(data, struct Computed_field_kernel_matrix_type_specific_data,1) &&
		         ALLOCATE(data->a, FE_value, matrix_size))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_kernel_matrix_type_string;
			/* save the size of the kernel matrix to <number_of_components> */
			field->number_of_components = matrix_size;
			
			field->number_of_source_fields = number_of_source_fields;
			data->dimension = dimension;
			data->radius = radius;
			data->sigma = sigma;
			if (low_index > 0)
			{
			        data->filter_name = "low_pass";
			}
			else if (high_index > 0)
			{
			        data->filter_name = "high_pass";
			}
			else if (unsharp_index > 0)
			{
			        data->filter_name = "unsharp";
			}
			else if (gaussian_index > 0)
			{
			        data->filter_name = "gaussian";
			}
			field->type_specific_data = (void *)data;

				/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(kernel_matrix);
			return_code=1;
		}
		else
		{
		        if (data)
			{
				if (data->a)
				{
					DEALLOCATE(data->a);
				}
				DEALLOCATE(data);
			}
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_kernel_matrix.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_kernel_matrix.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_kernel_matrix */

int Computed_field_get_type_kernel_matrix(struct Computed_field *field, int *dimension,
          int *radius, double *sigma, int *low_index, int *high_index, 
	  int *unsharp_index, int *gaussian_index)
/*******************************************************************************
LAST MODIFIED : 10 May 2005

DESCRIPTION :
If the field is of type 'kernel_matrix'
==============================================================================*/
{
	int return_code;
	
	struct Computed_field_kernel_matrix_type_specific_data *data;

	ENTER(Computed_field_get_type_kernel_matrix);
	if(field && (field->type_string==computed_field_kernel_matrix_type_string)
		&& (data = (struct Computed_field_kernel_matrix_type_specific_data *)
		field->type_specific_data))
	{
	        *dimension = data->dimension;
		*radius = data->radius;
		*sigma = data->sigma;
		*low_index = *high_index = *unsharp_index = *gaussian_index = 0;
		if (strcmp(data->filter_name, "low_pass") == 0)
		{
			*low_index = 1;
		}
		else if (strcmp(data->filter_name, "high_pass") == 0)
		{
			*high_index = 1;
		}
		else if (strcmp(data->filter_name, "unsharp_masking") == 0)
		{
			*unsharp_index = 1;
		}
		else if (strcmp(data->filter_name, "gaussian"))
		{
		        *gaussian_index = 1;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_kernel_matrix.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_kernel_matrix */

static int define_Computed_field_type_kernel_matrix(struct Parse_state *state,
	void *field_void, void *computed_field_kernel_matrix_package_void)
/*******************************************************************************
LAST MODIFIED : 11 May 2005

DESCRIPTION :
Converts <field> into type 'kernel_matrix' (if it is not already) and allows its
contents to be modified.
==============================================================================*/
{
	int return_code;
	int radius, dimension;
	double sigma;
	char low_string[] = "low_pass", high_string[] = "high_pass";
	char unsharp_string[] = "unsharp_masking", gaussian_string[] = "gaussian";
	struct Computed_field *field;
	/*struct Computed_field_kernel_matrix_package 
		*computed_field_kernel_matrix_package;*/
	struct Option_table *option_table;
	struct Set_names_from_list_data filter_name;

	ENTER(define_Computed_field_type_kernel_matrix);
	USE_PARAMETER(computed_field_kernel_matrix_package_void);
	if (state && (field = (struct Computed_field *)field_void))
	{
		return_code=1;
		dimension = 0;
		radius = 0;
		sigma = 0.0;
		/* filters */
		filter_name.number_of_tokens = 4;
		ALLOCATE(filter_name.tokens, struct Set_names_from_list_token, 4);
		filter_name.tokens[0].string = low_string;
		filter_name.tokens[0].index = 0;
		filter_name.tokens[1].string = high_string;
		filter_name.tokens[1].index = 0;
		filter_name.tokens[2].string = unsharp_string;
		filter_name.tokens[2].index = 0;
		filter_name.tokens[3].string = gaussian_string;
		filter_name.tokens[3].index = 0;
		if (computed_field_kernel_matrix_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_kernel_matrix(field, &dimension,
                                  &radius, &sigma, &filter_name.tokens[0].index, &filter_name.tokens[1].index, &filter_name.tokens[2].index, &filter_name.tokens[3].index);
		}
		if (return_code)
		{
			option_table = CREATE(Option_table)();
			/* dimension */
			Option_table_add_int_positive_entry(option_table, "dimension",
				&dimension);
			/* filter_name */
			Option_table_add_set_names_from_list_entry(option_table,
				"filter_name", &filter_name);
			/* radius */
			Option_table_add_int_positive_entry(option_table, "radius",
				&radius);
			/* sigma */
			Option_table_add_double_entry(option_table, "sigma",
				&sigma);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_kernel_matrix(field, dimension,
                                       radius, sigma, filter_name.tokens[0].index, filter_name.tokens[1].index, filter_name.tokens[2].index, filter_name.tokens[3].index);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_kernel_matrix.  Failed");
				}
			}
			DESTROY(Option_table)(&option_table);
		}
		DEALLOCATE(filter_name.tokens);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_kernel_matrix.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_kernel_matrix */


int Computed_field_register_types_kernel_matrix(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 11 May 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_kernel_matrix_package 
		computed_field_kernel_matrix_package;

	ENTER(Computed_field_register_types_kernel_matrix);
	if (computed_field_package)
	{
		computed_field_kernel_matrix_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);

		return_code =
			Computed_field_package_add_type(computed_field_package,
				computed_field_kernel_matrix_type_string,
				define_Computed_field_type_kernel_matrix,
				&computed_field_kernel_matrix_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_kernel_matrix.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_kernel_matrix */

