/*******************************************************************************
FILE : computed_field_derivatives.c

LAST MODIFIED : 11 July 2000

DESCRIPTION :
Implements a computed_field which maintains a graphics transformation 
equivalent to the scene_viewer assigned to it.
==============================================================================*/
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.h"
#include "general/debug.h"
#include "general/matrix_vector.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_derivatives.h"

struct Computed_field_derivatives_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
};

struct Computed_field_derivatives_type_specific_data
{
	int xi_index;
};

static char computed_field_derivative_type_string[] = "derivative";

int Computed_field_is_type_derivative(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_derivative);
	if (field)
	{
		return_code = (field->type_string == computed_field_derivative_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_derivative.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_is_type_derivative */

static int Computed_field_derivative_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_derivative_clear_type_specific);
	if (field)
	{
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_derivative_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_derivative_clear_type_specific */

static void *Computed_field_derivative_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_derivatives_type_specific_data *destination,
		*source;

	ENTER(Computed_field_derivative_copy_type_specific);
	if (field && (source = 
		(struct Computed_field_derivatives_type_specific_data *)
		field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_derivatives_type_specific_data, 1))
		{
			destination->xi_index = source->xi_index;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_derivative_copy_type_specific.  "
				"Unable to allocate memory.");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_derivative_copy_type_specific.  "
			"Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_derivative_copy_type_specific */

#define Computed_field_derivative_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_derivative_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;
	struct Computed_field_derivatives_type_specific_data *data,
		*other_data;

	ENTER(Computed_field_derivative_type_specific_contents_match);
	if (field && other_computed_field && (data = 
		(struct Computed_field_derivatives_type_specific_data *)
		field->type_specific_data) && (other_data =
		(struct Computed_field_derivatives_type_specific_data *)
		other_computed_field->type_specific_data))
	{
		if ((data->xi_index == other_data->xi_index))
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_derivative_type_specific_contents_match */

int Computed_field_derivative_is_defined_in_element(
	struct Computed_field *field, struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/
{
	int return_code;
	struct Computed_field_derivatives_type_specific_data *data;

	ENTER(Computed_field_derivative_is_defined_in_element);
	if (field && element && (data = 
		(struct Computed_field_derivatives_type_specific_data *)
		field->type_specific_data))
	{
		/* Derivatives can only be calculated up to element dimension */
		if (data->xi_index < get_FE_element_dimension(element))
		{
			/* Check the source field */
			return_code = Computed_field_default_is_defined_in_element(field,
				element);
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_derivative_is_defined_in_element.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_derivative_is_defined_in_element */


static int Computed_field_derivative_is_defined_at_node(
	struct Computed_field *field, struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_derivative_evaluate_cache_at_node);
	if (field && node)
	{
		/* Derivatives can only be calculated in elements */
		return_code = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_derivative_is_defined_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_derivative_evaluate_cache_at_node */

#define Computed_field_derivative_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_derivative_can_be_destroyed \
	(Computed_field_can_be_destroyed_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
No special criteria on the destroy
==============================================================================*/

#define Computed_field_derivative_evaluate_cache_at_node \
   (Computed_field_evaluate_cache_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/

static int Computed_field_derivative_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	struct FE_element *top_level_element, int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int i, number_of_xi, return_code;
	struct Computed_field_derivatives_type_specific_data *data;

	ENTER(Computed_field_derivative_evaluate_cache_in_element);
	USE_PARAMETER(calculate_derivatives);
	if (field && Computed_field_has_at_least_2_components(field, NULL) && 
		element && xi 
		&& (data = (struct Computed_field_derivatives_type_specific_data *)
		field->type_specific_data))
	{
		number_of_xi = get_FE_element_dimension(element);
		if (data->xi_index < number_of_xi)
		{
			/* 1. Precalculate any source fields that this field depends on,
		      we always want the derivatives */
			if (return_code = 
				Computed_field_evaluate_source_fields_cache_in_element(field, element,
					xi, top_level_element, /*calculate_derivatives*/1))
			{
				/* 2. Calculate the field */
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					field->values[i] = field->source_fields[0]->
						derivatives[i * number_of_xi + data->xi_index];
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_derivative_evaluate_cache_in_element.  "
				"Derivative %d not defined on element.",
				data->xi_index + 1);
		}
		field->derivatives_valid = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_derivative_evaluate_cache_in_element.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_derivative_evaluate_cache_in_element */

#define Computed_field_derivative_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_derivative_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_derivative_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_derivative_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_derivative_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_derivative_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_derivative(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_derivatives_type_specific_data *data;

	ENTER(List_Computed_field_derivative);
	if (field && (data = 
		(struct Computed_field_derivatives_type_specific_data *)
		field->type_specific_data))
	{
		display_message(INFORMATION_MESSAGE,
			"    field : %s\n",field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,
			"    xi number : %d\n",data->xi_index+1);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_derivative.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_derivative */

static int list_Computed_field_derivative_commands(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_derivatives_type_specific_data *data;

	ENTER(list_Computed_field_derivative_commands);
	if (field && (data = 
		(struct Computed_field_derivatives_type_specific_data *)
		field->type_specific_data))
	{
		display_message(INFORMATION_MESSAGE,
			" field %s",field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,
			" xi %d\n",data->xi_index+1);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_derivative_commands.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_derivative_commands */

int Computed_field_set_type_derivative(struct Computed_field *field,
	struct Computed_field *source_field, int xi_index)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_DERIVATIVE with the supplied
<source_field> and <xi_index>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;
	struct Computed_field_derivatives_type_specific_data *data;

	ENTER(Computed_field_set_type_derivative);
	if (field)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields)&&
			ALLOCATE(data,struct Computed_field_derivatives_type_specific_data, 1))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_NEW_TYPES;
			field->type_string = computed_field_derivative_type_string;
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)data;
			data->xi_index = xi_index;

			/* Set all the methods */
			field->computed_field_clear_type_specific_function =
				Computed_field_derivative_clear_type_specific;
			field->computed_field_copy_type_specific_function =
				Computed_field_derivative_copy_type_specific;
			field->computed_field_clear_cache_type_specific_function =
				Computed_field_derivative_clear_cache_type_specific;
			field->computed_field_type_specific_contents_match_function =
				Computed_field_derivative_type_specific_contents_match;
			field->computed_field_is_defined_in_element_function =
				Computed_field_derivative_is_defined_in_element;
			field->computed_field_is_defined_at_node_function =
				Computed_field_derivative_is_defined_at_node;
			field->computed_field_has_numerical_components_function =
				Computed_field_derivative_has_numerical_components;
			field->computed_field_can_be_destroyed_function =
				Computed_field_derivative_can_be_destroyed;
			field->computed_field_evaluate_cache_at_node_function =
				Computed_field_derivative_evaluate_cache_at_node;
			field->computed_field_evaluate_cache_in_element_function =
				Computed_field_derivative_evaluate_cache_in_element;
			field->computed_field_evaluate_as_string_at_node_function =
				Computed_field_derivative_evaluate_as_string_at_node;
			field->computed_field_evaluate_as_string_in_element_function =
				Computed_field_derivative_evaluate_as_string_in_element;
			field->computed_field_set_values_at_node_function =
				Computed_field_derivative_set_values_at_node;
			field->computed_field_set_values_in_element_function =
				Computed_field_derivative_set_values_in_element;
			field->computed_field_get_native_discretization_in_element_function =
				Computed_field_derivative_get_native_discretization_in_element;
			field->computed_field_find_element_xi_function =
				Computed_field_derivative_find_element_xi;
			field->list_Computed_field_function = 
				list_Computed_field_derivative;
			field->list_Computed_field_commands_function = 
				list_Computed_field_derivative_commands;
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_derivative.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_derivative */

int Computed_field_get_type_derivative(struct Computed_field *field,
	struct Computed_field **source_field, int *xi_index)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_DERIVATIVE, the 
<source_field> and <xi_index> used by it are returned.
==============================================================================*/
{
	int return_code;
	struct Computed_field_derivatives_type_specific_data *data;

	ENTER(Computed_field_get_type_derivative);
	if (field&&(COMPUTED_FIELD_NEW_TYPES==field->type)&&
		(field->type_string==computed_field_derivative_type_string)
		&&(data = 
		(struct Computed_field_derivatives_type_specific_data *)
		field->type_specific_data)&&source_field)
	{
		*source_field = field->source_fields[0];
		*xi_index = data->xi_index;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_derivative.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_derivative */

static int define_Computed_field_type_derivative(struct Parse_state *state,
	void *field_void,void *computed_field_derivatives_package_void)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_DERIVATIVE (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code, xi_index;
	struct Computed_field *field,*source_field;
	struct Computed_field_derivatives_package 
		*computed_field_derivatives_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_derivative);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_derivatives_package=
		(struct Computed_field_derivatives_package *)
		computed_field_derivatives_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		if (computed_field_derivative_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code=Computed_field_get_type_derivative(field,
				&source_field, &xi_index);
			xi_index++;
		}
		else
		{
			xi_index = 1;
			if (!((source_field=
				FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_has_numerical_components,(void *)NULL,
				computed_field_derivatives_package->computed_field_manager))))
			{
				if (strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
				{
					/* This is only a failure if we aren't asking for help */
					display_message(ERROR_MESSAGE,
						"At least one field of 3 components must exist for a derivative field.");
					return_code = 0;
				}
			}
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}

			option_table = CREATE(Option_table)();
			/* field */
			set_source_field_data.computed_field_manager=
				computed_field_derivatives_package->computed_field_manager;
			set_source_field_data.conditional_function=Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"field",&source_field,
				&set_source_field_data,set_Computed_field_conditional);
			/* xi_index */
			Option_table_add_entry(option_table,"xi_index",&xi_index,
				NULL,set_int_positive);
			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_derivative(field,
					source_field, xi_index - 1);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_derivative.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_derivative.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_derivative */

static char computed_field_curl_type_string[] = "curl";

int Computed_field_is_type_curl(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_curl);
	if (field)
	{
		return_code = (field->type_string == computed_field_curl_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_curl.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_is_type_curl */

static int Computed_field_curl_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_curl_clear_type_specific);
	if (field)
	{
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_curl_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_curl_clear_type_specific */

static void *Computed_field_curl_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
   void *destination;

	ENTER(Computed_field_curl_copy_type_specific);
	if (field)
	{
		destination = (void *)1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_curl_copy_type_specific.  "
			"Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_curl_copy_type_specific */

#define Computed_field_curl_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_curl_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_curl_type_specific_contents_match);
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
} /* Computed_field_curl_type_specific_contents_match */

#define Computed_field_curl_is_defined_in_element \
   Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

static int Computed_field_curl_is_defined_at_node(
	struct Computed_field *field, struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Curl can only be calculated in elements
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_curl_evaluate_cache_at_node);
	if (field && node)
	{
		/* Curl can only be calculated in elements */
		return_code = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_curl_is_defined_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_curl_evaluate_cache_at_node */

#define Computed_field_curl_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_curl_can_be_destroyed \
	(Computed_field_can_be_destroyed_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
No special criteria on the destroy
==============================================================================*/

static int Computed_field_evaluate_curl(struct Computed_field *field,
	int element_dimension)
/*******************************************************************************
LAST MODIFIED : 23 March 2000

DESCRIPTION :
Function called by Computed_field_evaluate_in_element to compute the curl.
NOTE: Assumes that values and derivatives arrays are already allocated in
<field>, and that its source_fields are already computed (incl. derivatives)
for the same element, with the given <element_dimension> = number of Xi coords.
If function fails to invert the coordinate derivatives then the curl is
returned as 0 with a warning - as may happen at certain locations of the mesh.
Note currently requires vector_field to be RC.
==============================================================================*/
{
	FE_value curl,dx_dxi[9],dxi_dx[9],x[3],*source;
	int coordinate_components,i,return_code;
	
	ENTER(Computed_field_evaluate_curl);
	if (field&&(COMPUTED_FIELD_NEW_TYPES==field->type)&&
		(field->type_string==computed_field_curl_type_string))
	{
		coordinate_components=field->source_fields[1]->number_of_components;
		/* curl is only valid in 3 dimensions */
		if ((3==element_dimension)&&(3==coordinate_components))
		{
			/* only support RC vector fields */
			if (RECTANGULAR_CARTESIAN==
				field->source_fields[0]->coordinate_system.type)
			{
				if (return_code=Computed_field_extract_rc(field->source_fields[1],
					element_dimension,x,dx_dxi))
				{
					if (invert_FE_value_matrix3(dx_dxi,dxi_dx))
					{
						source=field->source_fields[0]->derivatives;
						/* curl[0] = dVz/dy - dVy/dz */
						curl=0.0;
						for (i=0;i<element_dimension;i++)
						{
							curl += (source[6+i]*dxi_dx[3*i+1] - source[3+i]*dxi_dx[3*i+2]);
						}
						field->values[0]=curl;
						/* curl[1] = dVx/dz - dVz/dx */
						curl=0.0;
						for (i=0;i<element_dimension;i++)
						{
							curl += (source[  i]*dxi_dx[3*i+2] - source[6+i]*dxi_dx[3*i  ]);
						}
						field->values[1]=curl;
						/* curl[2] = dVy/dx - dVx/dy */
						curl=0.0;
						for (i=0;i<element_dimension;i++)
						{
							curl += (source[3+i]*dxi_dx[3*i  ] - source[  i]*dxi_dx[3*i+1]);
						}
						field->values[2]=curl;
					}
					else
					{
						display_message(WARNING_MESSAGE,
							"Could not invert coordinate derivatives; setting curl to 0");
						for (i=0;i<field->number_of_components;i++)
						{
							field->values[i]=0.0;
						}
					}
					/* cannot calculate derivatives for curl yet */
					field->derivatives_valid=0;
				}
				if (!return_code)
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_evaluate_curl.  Failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_evaluate_curl.  Vector field must be RC");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_evaluate_curl.  Elements of wrong dimension");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_curl.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_evaluate_curl */

#define Computed_field_curl_evaluate_cache_at_node \
   (Computed_field_evaluate_cache_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Cannot evaluate at a node.
==============================================================================*/

static int Computed_field_curl_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	struct FE_element *top_level_element, int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value top_level_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int element_dimension, return_code, top_level_element_dimension;

	ENTER(Computed_field_curl_evaluate_cache_in_element);
	USE_PARAMETER(calculate_derivatives);
	if (field && element && xi)
	{
		element_dimension = get_FE_element_dimension(element);
		/* 1. Precalculate any source fields that this field depends on,
			we always want the derivatives and want to use the top_level element */
		Computed_field_get_top_level_element_and_xi(element, xi, element_dimension,
			top_level_element, top_level_xi, &top_level_element_dimension);
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, top_level_element,
				top_level_xi, top_level_element, /*calculate_derivatives*/1))
		{
			/* 2. Calculate the field */
			return_code=Computed_field_evaluate_curl(field, top_level_element_dimension);
		}
		field->derivatives_valid = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_curl_evaluate_cache_in_element.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_curl_evaluate_cache_in_element */

#define Computed_field_curl_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_curl_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_curl_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_curl_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_curl_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_curl_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_curl(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_curl);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    coordinate field : %s\n",field->source_fields[1]->name);
		display_message(INFORMATION_MESSAGE,
			"    vector field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_curl.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_curl */

static int list_Computed_field_curl_commands(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(list_Computed_field_curl_commands);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			" coordinate %s vector %s",field->source_fields[1]->name,
			field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_curl_commands.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_curl_commands */

int Computed_field_set_type_curl(struct Computed_field *field,
	struct Computed_field *vector_field,struct Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_CURL, combining a vector and
coordinate field to return the curl scalar.
Sets the number of components to 3.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
<vector_field> and <coordinate_field> must both have exactly 3 components.
The vector field must also be RECTANGULAR_CARTESIAN.
Note that an error will be reported on calculation if the xi-dimension of the
element and the number of components in coordinate_field & vector_field differ.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_curl);
	if (field&&vector_field&&(3==vector_field->number_of_components)&&
		coordinate_field&&(3==coordinate_field->number_of_components))
	{
		/* only support RC vector fields */
		if (RECTANGULAR_CARTESIAN==vector_field->coordinate_system.type)
		{
			return_code=1;
			/* 1. make dynamic allocations for any new type-specific data */
			number_of_source_fields=2;
			if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
			{
				/* 2. free current type-specific data */
				Computed_field_clear_type(field);
				/* 3. establish the new type */
				field->type=COMPUTED_FIELD_NEW_TYPES;
				field->type_string = computed_field_curl_type_string;
				field->number_of_components=3;
				source_fields[0]=ACCESS(Computed_field)(vector_field);
				source_fields[1]=ACCESS(Computed_field)(coordinate_field);
				field->source_fields=source_fields;
				field->number_of_source_fields=number_of_source_fields;			
				field->type_specific_data = (void *)1;

				/* Set all the methods */
				field->computed_field_clear_type_specific_function =
					Computed_field_curl_clear_type_specific;
				field->computed_field_copy_type_specific_function =
					Computed_field_curl_copy_type_specific;
				field->computed_field_clear_cache_type_specific_function =
					Computed_field_curl_clear_cache_type_specific;
				field->computed_field_type_specific_contents_match_function =
					Computed_field_curl_type_specific_contents_match;
				field->computed_field_is_defined_in_element_function =
					Computed_field_curl_is_defined_in_element;
				field->computed_field_is_defined_at_node_function =
					Computed_field_curl_is_defined_at_node;
				field->computed_field_has_numerical_components_function =
					Computed_field_curl_has_numerical_components;
				field->computed_field_can_be_destroyed_function =
					Computed_field_curl_can_be_destroyed;
				field->computed_field_evaluate_cache_at_node_function =
					Computed_field_curl_evaluate_cache_at_node;
				field->computed_field_evaluate_cache_in_element_function =
					Computed_field_curl_evaluate_cache_in_element;
				field->computed_field_evaluate_as_string_at_node_function =
					Computed_field_curl_evaluate_as_string_at_node;
				field->computed_field_evaluate_as_string_in_element_function =
					Computed_field_curl_evaluate_as_string_in_element;
				field->computed_field_set_values_at_node_function =
					Computed_field_curl_set_values_at_node;
				field->computed_field_set_values_in_element_function =
					Computed_field_curl_set_values_in_element;
				field->computed_field_get_native_discretization_in_element_function =
					Computed_field_curl_get_native_discretization_in_element;
				field->computed_field_find_element_xi_function =
					Computed_field_curl_find_element_xi;
				field->list_Computed_field_function = 
					list_Computed_field_curl;
				field->list_Computed_field_commands_function = 
					list_Computed_field_curl_commands;
			}
			else
			{
				DEALLOCATE(source_fields);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_curl.  Vector field must be RC");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_curl.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_curl */

int Computed_field_get_type_curl(struct Computed_field *field,
	struct Computed_field **vector_field,struct Computed_field **coordinate_field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_CURL, the 
<source_field> and <coordinate_field> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_curl);
	if (field&&(COMPUTED_FIELD_NEW_TYPES==field->type)&&
		(field->type_string==computed_field_curl_type_string)
		&&vector_field&&coordinate_field)
	{
		/* source_fields: 0=vector, 1=coordinate */
		*vector_field = field->source_fields[0];
		*coordinate_field=field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_curl.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_curl */

static int define_Computed_field_type_curl(struct Parse_state *state,
	void *field_void,void *computed_field_derivatives_package_void)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CURL (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *coordinate_field,*field,*vector_field;
	struct Computed_field_derivatives_package 
		*computed_field_derivatives_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_vector_field_data;

	ENTER(define_Computed_field_type_curl);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_derivatives_package=
		(struct Computed_field_derivatives_package *)
		computed_field_derivatives_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		coordinate_field=(struct Computed_field *)NULL;
		vector_field=(struct Computed_field *)NULL;
		if (computed_field_curl_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code=Computed_field_get_type_curl(field,
				&vector_field,&coordinate_field);
		}
		else
		{
			if (!(coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_derivatives_package->computed_field_manager)))
			{
				coordinate_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_has_3_components,(void *)NULL,
					computed_field_derivatives_package->computed_field_manager);
			}
			vector_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_has_3_components,(void *)NULL,
				computed_field_derivatives_package->computed_field_manager);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (coordinate_field)
			{
				ACCESS(Computed_field)(coordinate_field);
			}
			if (vector_field)
			{
				ACCESS(Computed_field)(vector_field);
			}

			option_table = CREATE(Option_table)();
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
            computed_field_derivatives_package->computed_field_manager;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_3_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				(void *)&set_coordinate_field_data,set_Computed_field_conditional);
			/* vector */
			set_vector_field_data.computed_field_manager=
            computed_field_derivatives_package->computed_field_manager;
			set_vector_field_data.conditional_function=
				Computed_field_has_3_components;
			set_vector_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"vector",&vector_field,
				(void *)&set_vector_field_data,set_Computed_field_conditional);
			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_curl(field,
					vector_field,coordinate_field);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_curl.  Failed");
				}
			}
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (vector_field)
			{
				DEACCESS(Computed_field)(&vector_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_curl.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_curl */

static char computed_field_divergence_type_string[] = "divergence";

int Computed_field_is_type_divergence(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_divergence);
	if (field)
	{
		return_code = (field->type_string == computed_field_divergence_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_divergence.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_is_type_divergence */

static int Computed_field_divergence_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_divergence_clear_type_specific);
	if (field)
	{
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_divergence_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_divergence_clear_type_specific */

static void *Computed_field_divergence_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
   void *destination;

	ENTER(Computed_field_divergence_copy_type_specific);
	if (field)
	{
		destination = (void *)1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_divergence_copy_type_specific.  "
			"Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_divergence_copy_type_specific */

#define Computed_field_divergence_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_divergence_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_divergence_type_specific_contents_match);
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
} /* Computed_field_divergence_type_specific_contents_match */

#define Computed_field_divergence_is_defined_in_element \
   Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

static int Computed_field_divergence_is_defined_at_node(
	struct Computed_field *field, struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Divergence can only be calculated in elements
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_divergence_evaluate_cache_at_node);
	if (field && node)
	{
		/* Divergence can only be calculated in elements */
		return_code = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_divergence_is_defined_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_divergence_evaluate_cache_at_node */

#define Computed_field_divergence_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_divergence_can_be_destroyed \
	(Computed_field_can_be_destroyed_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
No special criteria on the destroy
==============================================================================*/

static int Computed_field_evaluate_divergence(struct Computed_field *field,
	int element_dimension)
/*******************************************************************************
LAST MODIFIED : 23 March 2000

DESCRIPTION :
Function called by Computed_field_evaluate_in_element to compute the divergence.
NOTE: Assumes that values and derivatives arrays are already allocated in
<field>, and that its source_fields are already computed (incl. derivatives)
for the same element, with the given <element_dimension> = number of Xi coords.
If function fails to invert the coordinate derivatives then the divergence is
returned as 0 with a warning - as may happen at certain locations of the mesh.
Note currently requires vector_field to be RC.
==============================================================================*/
{
	FE_value divergence,dx_dxi[9],dxi_dx[9],x[3],*source;
	int coordinate_components,i,j,return_code;
	
	ENTER(Computed_field_evaluate_divergence);
	if (field&&(COMPUTED_FIELD_NEW_TYPES==field->type)&&
		(field->type_string==computed_field_divergence_type_string))
	{
		coordinate_components=field->source_fields[1]->number_of_components;
		/* Following asks: can dx_dxi be inverted? */
		if (((3==element_dimension)&&(3==coordinate_components))||
			((RECTANGULAR_CARTESIAN==field->source_fields[1]->coordinate_system.type)
				&&(coordinate_components==element_dimension))||
			((CYLINDRICAL_POLAR==field->source_fields[1]->coordinate_system.type)&&
				(2==element_dimension)&&(2==coordinate_components)))
		{
			/* only support RC vector fields */
			if (RECTANGULAR_CARTESIAN==
				field->source_fields[0]->coordinate_system.type)
			{
				if (return_code=Computed_field_extract_rc(field->source_fields[1],
					element_dimension,x,dx_dxi))
				{
					/* if the element_dimension is less than 3, put ones on the main
						 diagonal to allow inversion of dx_dxi */
					if (3>element_dimension)
					{
						dx_dxi[8]=1.0;
						if (2>element_dimension)
						{
							dx_dxi[4]=1.0;
						}
					}
					if (invert_FE_value_matrix3(dx_dxi,dxi_dx))
					{
						divergence=0.0;
						source=field->source_fields[0]->derivatives;
						for (i=0;i<element_dimension;i++)
						{
							for (j=0;j<element_dimension;j++)
							{
								divergence += (*source) * dxi_dx[3*j+i];
								source++;
							}
						}
						field->values[0]=divergence;
					}
					else
					{
						display_message(WARNING_MESSAGE,
							"Could not invert coordinate derivatives; "
							"setting divergence to 0");
						field->values[0]=0.0;
					}
					/* cannot calculate derivatives for divergence yet */
					field->derivatives_valid=0;
				}
				if (!return_code)
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_evaluate_divergence.  Failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_evaluate_divergence.  Vector field must be RC");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_evaluate_divergence.  Elements of wrong dimension");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_divergence.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_evaluate_divergence */

#define Computed_field_divergence_evaluate_cache_at_node \
   (Computed_field_evaluate_cache_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/

static int Computed_field_divergence_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	struct FE_element *top_level_element, int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value top_level_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int element_dimension, return_code, top_level_element_dimension;

	ENTER(Computed_field_divergence_evaluate_cache_in_element);
	USE_PARAMETER(calculate_derivatives);
	if (field && element && xi)
	{
		element_dimension = get_FE_element_dimension(element);
		/* 1. Precalculate any source fields that this field depends on,
			we always want the derivatives and want to use the top_level element */
		Computed_field_get_top_level_element_and_xi(element, xi, element_dimension,
			top_level_element, top_level_xi, &top_level_element_dimension);
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, top_level_element,
				top_level_xi, top_level_element, /*calculate_derivatives*/1))
		{
			/* 2. Calculate the field */
			return_code=Computed_field_evaluate_divergence(field, top_level_element_dimension);
		}
		field->derivatives_valid = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_divergence_evaluate_cache_in_element.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_divergence_evaluate_cache_in_element */

#define Computed_field_divergence_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_divergence_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_divergence_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_divergence_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_divergence_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_divergence_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_divergence(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_divergence);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    coordinate field : %s\n",field->source_fields[1]->name);
		display_message(INFORMATION_MESSAGE,
			"    vector field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_divergence.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_divergence */

static int list_Computed_field_divergence_commands(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(list_Computed_field_divergence_commands);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			" coordinate %s vector %s",field->source_fields[1]->name,
			field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_divergence_commands.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_divergence_commands */

int Computed_field_set_type_divergence(struct Computed_field *field,
	struct Computed_field *vector_field,struct Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_DIVERGENCE, combining a vector and
coordinate field to return the divergence scalar.
Sets the number of components to 1.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
The number of components of <vector_field> and <coordinate_field> must be the
same and less than or equal to 3
The vector field must also be RECTANGULAR_CARTESIAN.
Note that an error will be reported on calculation if the xi-dimension of the
element and the number of components in coordinate_field & vector_field differ.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_divergence);
	if (field&&vector_field&&coordinate_field&&
		(3>=coordinate_field->number_of_components)&&
		(vector_field->number_of_components==
			coordinate_field->number_of_components))
	{
		/* only support RC vector fields */
		if (RECTANGULAR_CARTESIAN==vector_field->coordinate_system.type)
		{
			return_code=1;
			/* 1. make dynamic allocations for any new type-specific data */
			number_of_source_fields=2;
			if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
			{
				/* 2. free current type-specific data */
				Computed_field_clear_type(field);
				/* 3. establish the new type */
				field->type=COMPUTED_FIELD_NEW_TYPES;
				field->type_string = computed_field_divergence_type_string;
				field->number_of_components=1;
				source_fields[0]=ACCESS(Computed_field)(vector_field);
				source_fields[1]=ACCESS(Computed_field)(coordinate_field);
				field->source_fields=source_fields;
				field->number_of_source_fields=number_of_source_fields;			
				field->type_specific_data = (void *)1;

				/* Set all the methods */
				field->computed_field_clear_type_specific_function =
					Computed_field_divergence_clear_type_specific;
				field->computed_field_copy_type_specific_function =
					Computed_field_divergence_copy_type_specific;
				field->computed_field_clear_cache_type_specific_function =
					Computed_field_divergence_clear_cache_type_specific;
				field->computed_field_type_specific_contents_match_function =
					Computed_field_divergence_type_specific_contents_match;
				field->computed_field_is_defined_in_element_function =
					Computed_field_divergence_is_defined_in_element;
				field->computed_field_is_defined_at_node_function =
					Computed_field_divergence_is_defined_at_node;
				field->computed_field_has_numerical_components_function =
					Computed_field_divergence_has_numerical_components;
				field->computed_field_can_be_destroyed_function =
					Computed_field_divergence_can_be_destroyed;
				field->computed_field_evaluate_cache_at_node_function =
					Computed_field_divergence_evaluate_cache_at_node;
				field->computed_field_evaluate_cache_in_element_function =
					Computed_field_divergence_evaluate_cache_in_element;
				field->computed_field_evaluate_as_string_at_node_function =
					Computed_field_divergence_evaluate_as_string_at_node;
				field->computed_field_evaluate_as_string_in_element_function =
					Computed_field_divergence_evaluate_as_string_in_element;
				field->computed_field_set_values_at_node_function =
					Computed_field_divergence_set_values_at_node;
				field->computed_field_set_values_in_element_function =
					Computed_field_divergence_set_values_in_element;
				field->computed_field_get_native_discretization_in_element_function =
					Computed_field_divergence_get_native_discretization_in_element;
				field->computed_field_find_element_xi_function =
					Computed_field_divergence_find_element_xi;
				field->list_Computed_field_function = 
					list_Computed_field_divergence;
				field->list_Computed_field_commands_function = 
					list_Computed_field_divergence_commands;
			}
			else
			{
				DEALLOCATE(source_fields);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_divergence.  Vector field must be RC");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_divergence.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_divergence */

int Computed_field_get_type_divergence(struct Computed_field *field,
	struct Computed_field **vector_field,struct Computed_field **coordinate_field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_DIVERGENCE, the 
<source_field> and <coordinate_field> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_divergence);
	if (field&&(COMPUTED_FIELD_NEW_TYPES==field->type)&&
		(field->type_string==computed_field_divergence_type_string)
		&&vector_field&&coordinate_field)
	{
		/* source_fields: 0=vector, 1=coordinate */
		*vector_field = field->source_fields[0];
		*coordinate_field=field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_divergence.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_divergence */

static int define_Computed_field_type_divergence(struct Parse_state *state,
	void *field_void,void *computed_field_derivatives_package_void)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_DIVERGENCE (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *coordinate_field,*field,*vector_field;
	struct Computed_field_derivatives_package 
		*computed_field_derivatives_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_vector_field_data;

	ENTER(define_Computed_field_type_divergence);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_derivatives_package=
		(struct Computed_field_derivatives_package *)
		computed_field_derivatives_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		coordinate_field=(struct Computed_field *)NULL;
		vector_field=(struct Computed_field *)NULL;
		if (computed_field_divergence_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code=Computed_field_get_type_divergence(field,
				&vector_field,&coordinate_field);
		}
		else
		{
			if (!(coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_derivatives_package->computed_field_manager)))
			{
				coordinate_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_has_up_to_3_numerical_components,(void *)NULL,
					computed_field_derivatives_package->computed_field_manager);
			}
			vector_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_has_up_to_3_numerical_components,(void *)NULL,
				computed_field_derivatives_package->computed_field_manager);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (coordinate_field)
			{
				ACCESS(Computed_field)(coordinate_field);
			}
			if (vector_field)
			{
				ACCESS(Computed_field)(vector_field);
			}

			option_table = CREATE(Option_table)();
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
            computed_field_derivatives_package->computed_field_manager;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				(void *)&set_coordinate_field_data,set_Computed_field_conditional);
			/* vector */
			set_vector_field_data.computed_field_manager=
            computed_field_derivatives_package->computed_field_manager;
			set_vector_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_vector_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"vector",&vector_field,
				(void *)&set_vector_field_data,set_Computed_field_conditional);
			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_divergence(field,
					vector_field,coordinate_field);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_divergence.  Failed");
				}
			}
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (vector_field)
			{
				DEACCESS(Computed_field)(&vector_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_divergence.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_divergence */

static char computed_field_gradient_type_string[] = "gradient";

int Computed_field_is_type_gradient(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_gradient);
	if (field)
	{
		return_code = (field->type_string == computed_field_gradient_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_gradient.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_is_type_gradient */

static int Computed_field_gradient_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_gradient_clear_type_specific);
	if (field)
	{
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_gradient_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_gradient_clear_type_specific */

static void *Computed_field_gradient_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
   void *destination;

	ENTER(Computed_field_gradient_copy_type_specific);
	if (field)
	{
		destination = (void *)1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_gradient_copy_type_specific.  "
			"Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_gradient_copy_type_specific */

#define Computed_field_gradient_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_gradient_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_gradient_type_specific_contents_match);
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
} /* Computed_field_gradient_type_specific_contents_match */

#define Computed_field_gradient_is_defined_in_element \
   Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

static int Computed_field_gradient_is_defined_at_node(
	struct Computed_field *field, struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Gradient can only be calculated in elements
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_gradient_evaluate_cache_at_node);
	if (field && node)
	{
		/* Gradient can only be calculated in elements */
		return_code = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_gradient_is_defined_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_gradient_evaluate_cache_at_node */

#define Computed_field_gradient_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_gradient_can_be_destroyed \
	(Computed_field_can_be_destroyed_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
No special criteria on the destroy
==============================================================================*/

static int Computed_field_evaluate_gradient(struct Computed_field *field,
	int element_dimension)
/*******************************************************************************
LAST MODIFIED : 23 March 2000

DESCRIPTION :
Function called by Computed_field_evaluate_in_element to compute the gradient.
NOTE: Assumes that values and derivatives arrays are already allocated in
<field>, and that its source_fields are already computed (incl. derivatives)
for the same element, with the given <element_dimension> = number of Xi coords.
Derivatives are always calculated and set since they are always zero.
If function fails to invert the coordinate derivatives then the gradient is
returned as the 0 vector with a warning - as may happen at certain locations of
the mesh.
==============================================================================*/
{
	FE_value *destination,dx_dxi[9],dxi_dx[9],x[3],*source;
	int coordinate_components,i,j,return_code;
	
	ENTER(Computed_field_evaluate_gradient);
	if (field&&(COMPUTED_FIELD_NEW_TYPES==field->type)&&
		(field->type_string==computed_field_gradient_type_string))
	{
		coordinate_components=field->source_fields[1]->number_of_components;
		/* Following asks: can dx_dxi be inverted? */
		if (((3==element_dimension)&&(3==coordinate_components))||
			((RECTANGULAR_CARTESIAN==field->source_fields[1]->coordinate_system.type)
				&&(coordinate_components==element_dimension))||
			((CYLINDRICAL_POLAR==field->source_fields[1]->coordinate_system.type)&&
				(2==element_dimension)&&(2==coordinate_components)))
		{
			if (return_code=Computed_field_extract_rc(field->source_fields[1],
				element_dimension,x,dx_dxi))
			{
				/* if the element_dimension is less than 3, put ones on the main
					 diagonal to allow inversion of dx_dxi */
				if (3>element_dimension)
				{
					dx_dxi[8]=1.0;
					if (2>element_dimension)
					{
						dx_dxi[4]=1.0;
					}
				}
				if (invert_FE_value_matrix3(dx_dxi,dxi_dx))
				{
					destination=field->values;
					for (i=0;i<field->number_of_components;i++)
					{
						*destination=0.0;
						if (i<element_dimension)
						{
							source=field->source_fields[0]->derivatives;
							for (j=0;j<element_dimension;j++)
							{
								*destination += (*source) * dxi_dx[3*j+i];
								source++;
							}
						}
						destination++;
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,
						"Could not invert coordinate derivatives; setting gradient to 0");
					for (i=0;i<field->number_of_components;i++)
					{
						field->values[i]=0.0;
					}
				}
				/* cannot calculate derivatives for gradient yet */
				field->derivatives_valid=0;
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_evaluate_gradient.  Failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_evaluate_gradient.  Elements of wrong dimension");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_gradient.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_evaluate_gradient */

#define Computed_field_gradient_evaluate_cache_at_node \
   (Computed_field_evaluate_cache_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/

static int Computed_field_gradient_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	struct FE_element *top_level_element, int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value top_level_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int element_dimension, return_code, top_level_element_dimension;

	ENTER(Computed_field_gradient_evaluate_cache_in_element);
	USE_PARAMETER(calculate_derivatives);
	if (field && element && xi)
	{
		element_dimension = get_FE_element_dimension(element);
		/* 1. Precalculate any source fields that this field depends on,
			we always want the derivatives and want to use the top_level element */
		Computed_field_get_top_level_element_and_xi(element, xi, element_dimension,
			top_level_element, top_level_xi, &top_level_element_dimension);
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, top_level_element,
				top_level_xi, top_level_element, /*calculate_derivatives*/1))
		{
			/* 2. Calculate the field */
			return_code=Computed_field_evaluate_gradient(field, top_level_element_dimension);
		}
		field->derivatives_valid = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_gradient_evaluate_cache_in_element.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_gradient_evaluate_cache_in_element */

#define Computed_field_gradient_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_gradient_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_gradient_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_gradient_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_gradient_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_gradient_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_gradient(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_gradient);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    coordinate field : %s\n",field->source_fields[1]->name);
		display_message(INFORMATION_MESSAGE,
			"    scalar field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_gradient.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_gradient */

static int list_Computed_field_gradient_commands(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(list_Computed_field_gradient_commands);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			" coordinate %s scalar %s",field->source_fields[1]->name,
			field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_gradient_commands.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_gradient_commands */

int Computed_field_set_type_gradient(struct Computed_field *field,
	struct Computed_field *scalar_field,struct Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_GRADIENT, combining a vector and
coordinate field to return the gradient scalar.
Sets the number of components to 1.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
The number of components of <scalar_field> and <coordinate_field> must be the
same and less than or equal to 3
The vector field must also be RECTANGULAR_CARTESIAN.
Note that an error will be reported on calculation if the xi-dimension of the
element and the number of components in coordinate_field & scalar_field differ.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_gradient);
	if (field&&scalar_field&&(1==scalar_field->number_of_components)&&
		coordinate_field&&(3>=coordinate_field->number_of_components))
	{
		/* only support RC vector fields */
		if (RECTANGULAR_CARTESIAN==scalar_field->coordinate_system.type)
		{
			return_code=1;
			/* 1. make dynamic allocations for any new type-specific data */
			number_of_source_fields=2;
			if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
			{
				/* 2. free current type-specific data */
				Computed_field_clear_type(field);
				/* 3. establish the new type */
				field->type=COMPUTED_FIELD_NEW_TYPES;
				field->type_string = computed_field_gradient_type_string;
				field->number_of_components=3;
				source_fields[0]=ACCESS(Computed_field)(scalar_field);
				source_fields[1]=ACCESS(Computed_field)(coordinate_field);
				field->source_fields=source_fields;
				field->number_of_source_fields=number_of_source_fields;			
				field->type_specific_data = (void *)1;

				/* Set all the methods */
				field->computed_field_clear_type_specific_function =
					Computed_field_gradient_clear_type_specific;
				field->computed_field_copy_type_specific_function =
					Computed_field_gradient_copy_type_specific;
				field->computed_field_clear_cache_type_specific_function =
					Computed_field_gradient_clear_cache_type_specific;
				field->computed_field_type_specific_contents_match_function =
					Computed_field_gradient_type_specific_contents_match;
				field->computed_field_is_defined_in_element_function =
					Computed_field_gradient_is_defined_in_element;
				field->computed_field_is_defined_at_node_function =
					Computed_field_gradient_is_defined_at_node;
				field->computed_field_has_numerical_components_function =
					Computed_field_gradient_has_numerical_components;
				field->computed_field_can_be_destroyed_function =
					Computed_field_gradient_can_be_destroyed;
				field->computed_field_evaluate_cache_at_node_function =
					Computed_field_gradient_evaluate_cache_at_node;
				field->computed_field_evaluate_cache_in_element_function =
					Computed_field_gradient_evaluate_cache_in_element;
				field->computed_field_evaluate_as_string_at_node_function =
					Computed_field_gradient_evaluate_as_string_at_node;
				field->computed_field_evaluate_as_string_in_element_function =
					Computed_field_gradient_evaluate_as_string_in_element;
				field->computed_field_set_values_at_node_function =
					Computed_field_gradient_set_values_at_node;
				field->computed_field_set_values_in_element_function =
					Computed_field_gradient_set_values_in_element;
				field->computed_field_get_native_discretization_in_element_function =
					Computed_field_gradient_get_native_discretization_in_element;
				field->computed_field_find_element_xi_function =
					Computed_field_gradient_find_element_xi;
				field->list_Computed_field_function = 
					list_Computed_field_gradient;
				field->list_Computed_field_commands_function = 
					list_Computed_field_gradient_commands;
			}
			else
			{
				DEALLOCATE(source_fields);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_gradient.  Vector field must be RC");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_gradient.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_gradient */

int Computed_field_get_type_gradient(struct Computed_field *field,
	struct Computed_field **scalar_field,struct Computed_field **coordinate_field)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_GRADIENT, the 
<source_field> and <coordinate_field> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_gradient);
	if (field&&(COMPUTED_FIELD_NEW_TYPES==field->type)&&
		(field->type_string==computed_field_gradient_type_string)
		&&scalar_field&&coordinate_field)
	{
		/* source_fields: 0=scalar, 1=coordinate */
		*scalar_field = field->source_fields[0];
		*coordinate_field=field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_gradient.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_gradient */

static int define_Computed_field_type_gradient(struct Parse_state *state,
	void *field_void,void *computed_field_derivatives_package_void)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_GRADIENT (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *coordinate_field,*field,*scalar_field;
	struct Computed_field_derivatives_package 
		*computed_field_derivatives_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_scalar_field_data;

	ENTER(define_Computed_field_type_gradient);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_derivatives_package=
		(struct Computed_field_derivatives_package *)
		computed_field_derivatives_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		coordinate_field=(struct Computed_field *)NULL;
		scalar_field=(struct Computed_field *)NULL;
		if (computed_field_gradient_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code=Computed_field_get_type_gradient(field,
				&scalar_field,&coordinate_field);
		}
		else
		{
			if (!(coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_derivatives_package->computed_field_manager)))
			{
				coordinate_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_has_up_to_3_numerical_components,(void *)NULL,
					computed_field_derivatives_package->computed_field_manager);
			}
			scalar_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_is_scalar,(void *)NULL,
				computed_field_derivatives_package->computed_field_manager);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (coordinate_field)
			{
				ACCESS(Computed_field)(coordinate_field);
			}
			if (scalar_field)
			{
				ACCESS(Computed_field)(scalar_field);
			}

			option_table = CREATE(Option_table)();
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
            computed_field_derivatives_package->computed_field_manager;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",&coordinate_field,
				(void *)&set_coordinate_field_data,set_Computed_field_conditional);
			/* scalar */
			set_scalar_field_data.computed_field_manager=
            computed_field_derivatives_package->computed_field_manager;
			set_scalar_field_data.conditional_function=
				Computed_field_is_scalar;
			set_scalar_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"scalar",&scalar_field,
				(void *)&set_scalar_field_data,set_Computed_field_conditional);
			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_gradient(field,
					scalar_field,coordinate_field);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_gradient.  Failed");
				}
			}
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (scalar_field)
			{
				DEACCESS(Computed_field)(&scalar_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_gradient.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_gradient */

int Computed_field_register_types_derivatives(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_derivatives_package 
		computed_field_derivatives_package;

	ENTER(Computed_field_register_type_derivative);
	if (computed_field_package)
	{
		computed_field_derivatives_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_derivative_type_string, 
			define_Computed_field_type_derivative,
			&computed_field_derivatives_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_curl_type_string, 
			define_Computed_field_type_curl,
			&computed_field_derivatives_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_divergence_type_string, 
			define_Computed_field_type_divergence,
			&computed_field_derivatives_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_gradient_type_string, 
			define_Computed_field_type_gradient,
			&computed_field_derivatives_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_type_derivative.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_type_derivative */

