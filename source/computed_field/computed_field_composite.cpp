/*******************************************************************************
FILE : computed_field_composite.c

LAST MODIFIED : 21 January 2002

DESCRIPTION :
Implements a "composite" computed_field which converts fields, field components
and real values in any order into a single vector field.
==============================================================================*/
#include <stdlib.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_composite.h"
#include "computed_field/computed_field_private.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

struct Computed_field_component
/*******************************************************************************
LAST MODIFIED : 22 January 1999

DESCRIPTION :
Used to specify a component of a Computed_field with function
set_Computed_field_component.
???RC Note that in its current use the field is NOT assumed to be accessed by
this structure in set_Computed_field_component.
==============================================================================*/
{
	struct Computed_field *field;
	int component_no;
}; /* struct Computed_field_component */

struct Computed_field_composite_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
};

struct Computed_field_composite_type_specific_data
{
	/* following are allocated with enough space for the number of components
		 in the field. <source_field_numbers> are indices into the source fields
		 for the field, with <source_values_numbers> referring to component number,
		 with both arrays starting at 0. If <source_field_numbers> is -1 then
		 the <source_value_numbers> at the same index contains the index into the
		 source values for the field. */
	int *source_field_numbers;
	int *source_value_numbers;
};

static char computed_field_composite_type_string[] = "composite";

int Computed_field_is_type_composite(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 19 October 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_composite);
	if (field)
	{
		return_code = (field->type_string == computed_field_composite_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_composite.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_is_type_composite */

static int Computed_field_composite_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 19 October 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_composite_type_specific_data *data;

	ENTER(Computed_field_composite_clear_type_specific);
	if (field && (data =
		(struct Computed_field_composite_type_specific_data *)
		field->type_specific_data))
	{
		if (data->source_field_numbers)
		{
			DEALLOCATE(data->source_field_numbers);
		}
		if (data->source_value_numbers)
		{
			DEALLOCATE(data->source_value_numbers);
		}
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_composite_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_composite_clear_type_specific */

static void *Computed_field_composite_copy_type_specific(
	struct Computed_field *source_field, struct Computed_field *destination_field)
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	int i;
	struct Computed_field_composite_type_specific_data *destination,*source;

	ENTER(Computed_field_composite_copy_type_specific);
	if (source_field && destination_field && (source = 
		(struct Computed_field_composite_type_specific_data *)
		source_field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_composite_type_specific_data, 1) &&
			ALLOCATE(destination->source_field_numbers,int,
				source_field->number_of_components) &&
			ALLOCATE(destination->source_value_numbers,int,
				source_field->number_of_components))
		{
			for (i = 0; i < source_field->number_of_components; i++)
			{
				destination->source_field_numbers[i] = source->source_field_numbers[i];
				destination->source_value_numbers[i] = source->source_value_numbers[i];
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_composite_copy_type_specific.  "
				"Unable to allocate memory.");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_composite_copy_type_specific.  Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_composite_copy_type_specific */

#define Computed_field_composite_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 19 October 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_composite_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 19 October 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int i, return_code;
	struct Computed_field_composite_type_specific_data *data, *other_data;

	ENTER(Computed_field_composite_type_specific_contents_match);
	if (field && other_computed_field && (data = 
		(struct Computed_field_composite_type_specific_data *)
		field->type_specific_data) && (other_data =
		(struct Computed_field_composite_type_specific_data *)
		other_computed_field->type_specific_data))
	{
		return_code=1;
		for (i=0;return_code&&(i<field->number_of_components);i++)
		{
			if ((data->source_field_numbers[i] !=
				other_data->source_field_numbers[i]) ||
				(data->source_value_numbers[i] !=
					other_data->source_value_numbers[i]))
			{
				return_code=0;
			}
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_composite_type_specific_contents_match */

#define Computed_field_composite_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 19 October 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_composite_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 19 October 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_composite_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 19 October 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_composite_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_composite_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 19 October 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int component_number, i, return_code, source_field_number;
	struct Computed_field_composite_type_specific_data *data;

	ENTER(Computed_field_composite_evaluate_cache_at_node);
	if (field && node && (data =
		(struct Computed_field_composite_type_specific_data *)
		field->type_specific_data))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			for (i=0;i<field->number_of_components;i++)
			{
				if (0 <= data->source_field_numbers[i])
				{
					/* source field component */
					source_field_number = data->source_field_numbers[i];
					component_number = data->source_value_numbers[i];
					field->values[i] =
						field->source_fields[source_field_number]->values[component_number];
				}
				else
				{
					/* source value */
					field->values[i] =
						field->source_values[data->source_value_numbers[i]];
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_composite_evaluate_cache_at_node.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_composite_evaluate_cache_at_node */

static int Computed_field_composite_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value *destination, *source;
	int component_number, element_dimension, i, j, return_code,
		source_field_number;
	struct Computed_field_composite_type_specific_data *data;

	ENTER(Computed_field_composite_evaluate_cache_in_element);
	if (field && element && xi && (data =
		(struct Computed_field_composite_type_specific_data *)
		field->type_specific_data))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			element_dimension=get_FE_element_dimension(element);
			destination=field->derivatives;
			for (i=0;i<field->number_of_components;i++)
			{
				if (0 <= data->source_field_numbers[i])
				{
					/* source field component */
					source_field_number = data->source_field_numbers[i];
					component_number = data->source_value_numbers[i];
					field->values[i] =
						field->source_fields[source_field_number]->values[component_number];
					if (calculate_derivatives)
					{
						source = field->source_fields[source_field_number]->derivatives +
							component_number*element_dimension;
						for (j=0;j<element_dimension;j++)
						{
							*destination = *source;
							destination++;
							source++;
						}
					}
				}
				else
				{
					/* source value */
					field->values[i] =
						field->source_values[data->source_value_numbers[i]];
					if (calculate_derivatives)
					{
						for (j=0;j<element_dimension;j++)
						{
							*destination = 0.0;
							destination++;
						}
					}
				}
			}
			field->derivatives_valid = calculate_derivatives;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_composite_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_composite_evaluate_cache_in_element */

#define Computed_field_composite_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 19 October 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_composite_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 19 October 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

static int Computed_field_composite_set_values_at_node(
	struct Computed_field *field,struct FE_node *node,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 19 October 2000

DESCRIPTION :
Sets the <values> of the computed <field> at <node>.
Note that if a field component is repeated in the composite field, the last
value for it in <values> will be used.
==============================================================================*/
{
	int i, return_code, source_field_number;
	FE_value *source_values;
	struct Computed_field_composite_type_specific_data *data;
	
	ENTER(Computed_field_composite_set_values_at_node);
	if (field && node && values && 
		(computed_field_composite_type_string == field->type_string) &&
		(data = (struct Computed_field_composite_type_specific_data *)
			field->type_specific_data))
	{
		return_code=1;
		/* go through each source field, getting current values, changing values
			 for components that are used in the composite field, then setting the
			 whole field in one hit */
		for (source_field_number=0;
			(source_field_number<field->number_of_source_fields)&&return_code;
			source_field_number++)
		{
			if (ALLOCATE(source_values,FE_value,
				field->source_fields[source_field_number]->number_of_components))
			{
				if (Computed_field_evaluate_at_node(
						 field->source_fields[source_field_number],node,/*time*/0,
						 source_values))
				{
					for (i=0;i<field->number_of_components;i++)
					{
						if (source_field_number == data->source_field_numbers[i])
						{
							source_values[data->source_value_numbers[i]] = values[i];
						}
					}
					return_code=Computed_field_set_values_at_node(
						field->source_fields[source_field_number],node,source_values);
				}
				else
				{
					return_code=0;
				}
				DEALLOCATE(source_values);
			}
			else
			{
				return_code=0;
			}
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_composite_set_values_at_node.  Failed");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_composite_set_values_at_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_composite_set_values_at_node */

static int Computed_field_composite_set_values_in_element(
	struct Computed_field *field, struct FE_element *element,int *number_in_xi,
	FE_value *values)
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
Sets the <values> of the computed <field> over the <element>.
==============================================================================*/
{
	FE_value *source_values;
	int element_dimension, i, number_of_points, return_code, source_field_number;
	struct Computed_field_composite_type_specific_data *data;
	
	ENTER(Computed_field_composite_set_values_in_element);
	if (field && element && number_in_xi && values && (data =
		(struct Computed_field_composite_type_specific_data *)
		field->type_specific_data))
	{
		return_code=1;
		element_dimension=get_FE_element_dimension(element);
		number_of_points=1;
		for (i=0;(i<element_dimension)&&return_code;i++)
		{
			if (0<number_in_xi[i])
			{
				number_of_points *= (number_in_xi[i]+1);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_composite_set_values_in_element.  "
					"number_in_xi must be positive");
				return_code=0;
			}
		}
		if (return_code)
		{
			/* go through each source field, getting current values, changing values
				 for components that are used in the composite field, then setting the
				 whole field in one hit */
			for (source_field_number=0;
				(source_field_number<field->number_of_source_fields)&&return_code;
				source_field_number++)
			{
				if (Computed_field_get_values_in_element(
					field->source_fields[source_field_number],element,number_in_xi,
					&source_values, /*time*/0))
				{
					for (i=0;i<field->number_of_components;i++)
					{
						if (source_field_number == data->source_field_numbers[i])
						{
							memcpy(
								source_values + data->source_value_numbers[i]*number_of_points,
								values + i*number_of_points,
								number_of_points*sizeof(FE_value));
						}
					}
					return_code=Computed_field_set_values_in_element(
						field->source_fields[source_field_number],element,number_in_xi,
						source_values);
					DEALLOCATE(source_values);
				}
				else
				{
					return_code=0;
				}
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_composite_set_values_in_element.  Failed");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_composite_set_values_in_element.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_composite_set_values_in_element */

#define Computed_field_composite_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 19 October 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

static int Computed_field_composite_find_element_xi(
	struct Computed_field *field, FE_value *values, int number_of_values, 
	struct FE_element **element, FE_value *xi, int element_dimension,
	struct Cmiss_region *search_region)
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Currently only tries to work if there is only one and exactly one source field.
Zero is used for any source field values that aren't set from the composite field.
==============================================================================*/
{
	int i, number_of_source_values, return_code, source_field_number;
	FE_value *source_values;
	struct Computed_field_composite_type_specific_data *data;
	
	ENTER(Computed_field_composite_find_element_xiset_values_at_node);
	if (field && element && xi && values && (number_of_values == field->number_of_components)
		&& (computed_field_composite_type_string == field->type_string) &&
		(data = (struct Computed_field_composite_type_specific_data *)
			field->type_specific_data))
	{
		return_code=1;
		if (field->number_of_source_fields == 1)
		{
			source_field_number = 0;
			number_of_source_values = field->source_fields[source_field_number]->number_of_components;
			if (ALLOCATE(source_values,FE_value,number_of_source_values))
			{
				/* Put zero into every value initially */
				for (i=0;i<number_of_source_values;i++)
				{
					source_values[i] = 0.0;
				}
				for (i=0;i<field->number_of_components;i++)
				{
					if (source_field_number == data->source_field_numbers[i])
					{
						source_values[data->source_value_numbers[i]] = values[i];
					}
				}
				return_code=Computed_field_find_element_xi(
					field->source_fields[source_field_number],source_values,
					number_of_source_values,element, xi, element_dimension, search_region,
					/*propagate_field*/1);
				DEALLOCATE(source_values);
			}
			else
			{
				return_code=0;
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_composite_find_element_xi.  Failed");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_composite_find_element_xi.  "
				"Unable to find element xi on a composite field involving more than one source field.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_composite_find_element_xi.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_composite_find_element_xi */

static char *Computed_field_composite_get_source_string(
	struct Computed_field *field, int commands)
/*******************************************************************************
LAST MODIFIED : 10 January 2002

DESCRIPTION :
Returns the field components and constants that make up the composite field in
a single string. If all components of a field are used consecutively, just the
field name is put in the final string, otherwise individual components appear.
Field/component names are made into valid tokens for commands.
If <commands> is set, field/components are made into valid tokens.
==============================================================================*/
{
	char *component_name, *source_string, tmp_string[40], *token;
	int error, i, j, source_number_of_components, token_error, whole_field;
	struct Computed_field *source_field;
	struct Computed_field_composite_type_specific_data *data;

	ENTER(Computed_field_composite_get_source_string);
	source_string = (char *)NULL;
	if (field && (data = 
		(struct Computed_field_composite_type_specific_data *)
		field->type_specific_data))
	{
		error = 0;
		for (i = 0; (i < field->number_of_components) && !error; i++)
		{
			if (0 < i)
			{
				append_string(&source_string, " ", &error);
			}
			if (0 <= data->source_field_numbers[i])
			{
				source_field = field->source_fields[data->source_field_numbers[i]];
				/* source field component */
				if (GET_NAME(Computed_field)(source_field, &token))
				{
					token_error = 0;
					source_number_of_components = source_field->number_of_components;
					whole_field = 1;
					for (j = 0; whole_field && (j < source_number_of_components); j++)
					{
						whole_field = ((i + j) < field->number_of_components) &&
							(j == data->source_value_numbers[i + j]);
					}
					if (whole_field)
					{
						i += source_number_of_components - 1;
					}
					else
					{
						if (component_name = Computed_field_get_component_name(source_field,
							data->source_value_numbers[i]))
						{
							append_string(&token, ".", &token_error);
							append_string(&token, component_name, &token_error);
							DEALLOCATE(component_name);
						}
					}
					if (commands)
					{
						make_valid_token(&token);
					}
					append_string(&source_string, token, &error);
					DEALLOCATE(token);
				}
			}
			else
			{
				/* source value */
				sprintf(tmp_string, "%g",
					field->source_values[data->source_value_numbers[i]]);
				append_string(&source_string, tmp_string, &error);
			}
		}
		if (error)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_composite_get_source_string.  Invalid argument(s)");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_composite_get_source_string.  Invalid argument(s)");
	}
	LEAVE;

	return (source_string);
} /* Computed_field_composite_get_source_string */

static int list_Computed_field_composite(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 20 October 2000

DESCRIPTION :
==============================================================================*/
{
	char *source_string;
	int return_code;

	ENTER(List_Computed_field_composite);
	if (field)
	{
		if (source_string =
			Computed_field_composite_get_source_string(field, /*commands*/0))
		{
			display_message(INFORMATION_MESSAGE,"    source data : %s\n",
				source_string);
			DEALLOCATE(source_string);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"list_Computed_field_composite.  Failed");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_composite.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_composite */

static char *Computed_field_composite_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *source_string;
	int error;

	ENTER(Computed_field_composite_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		/* could restore constant/component types here too */
		append_string(&command_string,
			computed_field_composite_type_string, &error);
		append_string(&command_string, " ", &error);
		if (source_string = Computed_field_composite_get_source_string(field,
			/*commands*/1))
		{
			append_string(&command_string, source_string, &error);
			DEALLOCATE(source_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_composite_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_composite_get_command_string */

#define Computed_field_composite_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_composite(struct Computed_field *field,
	int number_of_components,
	int number_of_source_fields,struct Computed_field **source_fields,
	int number_of_source_values,FE_value *source_values,
	int *source_field_numbers,int *source_value_numbers)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> into a composite field which returns a combination of field
components and constants in a given order.
The <number_of_source_fields> <source_fields> may be zero if the field is
purely constant, but fields may not be repeated.
The <number_of_source_values> <source_values> may similarly be omitted.
The <source_field_numbers> and <source_value_numbers> must each contain
<number_of_components> integers. For each component the <source_field_number>
says which <source_field> to use, and the <source_value_number> specifies the
component number. If the <source_field_number> is -1 for any component, the
<source_value_number> is the offset into the <source_values>.

Note: rigorous checking is performed to ensure that no values are invalid and
that the <source_fields> are presented in the order they are first used in the
<source_field_numbers>, and also that <source_values> are used in the order
they are given and that none are used more than once.

Some of these restrictions are enforced to ensure type-specific contents can be
compared easily -- otherwise there would be more than one way to describe the
same source data.

If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	FE_value *temp_source_values;
	int i, j, return_code, source_field_number, source_value_number,
		*temp_source_field_numbers, *temp_source_value_numbers;
	struct Computed_field *source_field, **temp_source_fields;
	struct Computed_field_composite_type_specific_data *data;

	ENTER(Computed_field_set_type_composite);
	if (field && (0<number_of_components) &&
		((0==number_of_source_fields) ||
			((0<number_of_source_fields) && source_fields)) &&
		((0==number_of_source_values) ||
			((0<number_of_source_values) && source_values)) &&
		source_field_numbers && source_value_numbers)
	{
		return_code=1;
		/* check all source fields exist and are not repeated */
		for (i=0;i<number_of_source_fields;i++)
		{
			if (source_fields[i])
			{
				for (j=0;j<i;j++)
				{
					if (source_fields[j] == source_fields[i])
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_set_type_composite.  Repeated source field");
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_composite.  Missing source field");
				return_code=0;
			}
		}
		if (return_code)
		{
			/* Check source_field_numbers refer to existing source_fields or equal -1.
				 Check components or source_values exist for the source_value_numbers
				 depending on whether field component or value specified. Also
				 ensure that values and fields are first used in the order they
				 appear in the source_values and source_fields */
			source_field_number=0;
			source_value_number=0;
			for (i=0;i<number_of_components;i++)
			{
				if (-1 == source_field_numbers[i])
				{
					/* source value numbers must be non-repeating and consecutive */
					if (source_value_number != source_value_numbers[i])
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_set_type_composite.  "
							"Source value numbers out of order");
						return_code=0;
					}
					source_value_number++;
				}
				else if ((0 <= source_field_numbers[i]) &&
					(source_field_numbers[i] < number_of_source_fields))
				{
					if (source_field_numbers[i] <= source_field_number)
					{
						if (source_field_numbers[i] == source_field_number)
						{
							source_field_number++;
						}
						source_field = source_fields[source_field_numbers[i]];
						if ((0 > source_value_numbers[i]) ||
							(source_value_numbers[i] >= source_field->number_of_components))
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_set_type_composite.  "
								"Component %d is out of range for field %s",
								source_value_numbers[i],source_field->name);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_set_type_composite.  "
							"source fields not used in order");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_set_type_composite.  "
						"Invalid source field number %d",source_field_numbers[i]);
					return_code=0;
				}
			}
			if (source_field_number < number_of_source_fields)
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_composite.  Not all source fields used");
				return_code=0;
			}
			if (source_value_number < number_of_source_values)
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_composite.  Not all source values used");
				return_code=0;
			}
		}
		if (return_code)
		{
			/* 1. make dynamic allocations for any new type-specific data */
			temp_source_fields = (struct Computed_field **)NULL;
			temp_source_values = (FE_value *)NULL;
			temp_source_field_numbers = (int *)NULL;
			temp_source_value_numbers = (int *)NULL;
			data = (struct Computed_field_composite_type_specific_data *)NULL;
			if (((0 == number_of_source_fields) || ALLOCATE(temp_source_fields,
				struct Computed_field *,number_of_source_fields)) &&
				((0 == number_of_source_values) || ALLOCATE(temp_source_values,
					FE_value,number_of_source_values)) &&
				ALLOCATE(temp_source_field_numbers,int,number_of_components) &&
				ALLOCATE(temp_source_value_numbers,int,number_of_components) &&
				ALLOCATE(data,struct Computed_field_composite_type_specific_data,1))
			{
				/* 2. free current type-specific data */
				Computed_field_clear_type(field);
				/* 3. establish the new type */
				field->type_string = computed_field_composite_type_string;
				field->number_of_components = number_of_components;
				for (i=0;i<number_of_source_fields;i++)
				{
					temp_source_fields[i] = ACCESS(Computed_field)(source_fields[i]);
				}
				field->source_fields = temp_source_fields;
				field->number_of_source_fields = number_of_source_fields;			
				for (i=0;i<number_of_source_values;i++)
				{
					temp_source_values[i] = source_values[i];
				}
				field->source_values = temp_source_values;
				field->number_of_source_values = number_of_source_values;			
				field->type_specific_data = (void *)data;
				for (i=0;i<number_of_components;i++)
				{
					temp_source_field_numbers[i] = source_field_numbers[i];
					temp_source_value_numbers[i] = source_value_numbers[i];
				}
				data->source_field_numbers = temp_source_field_numbers;
				data->source_value_numbers = temp_source_value_numbers;

				/* Set all the methods */
				COMPUTED_FIELD_ESTABLISH_METHODS(composite);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_composite.  Not enough memory");
				DEALLOCATE(temp_source_fields);
				DEALLOCATE(temp_source_values);
				DEALLOCATE(temp_source_field_numbers);
				DEALLOCATE(temp_source_value_numbers);
				DEALLOCATE(data);
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_composite.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_composite */

int Computed_field_get_type_composite(struct Computed_field *field,
	int *number_of_components,
	int *number_of_source_fields,struct Computed_field ***source_fields,
	int *number_of_source_values,FE_value **source_values,
	int **source_field_numbers,int **source_value_numbers)
/*******************************************************************************
LAST MODIFIED : 20 October 2000

DESCRIPTION :
If the field is of type composite, its source data is returned.
Note that <source_fields> and <source_values> are ALLOCATED (or NULL if none),
and <source_field_numbers> and <source_value_numbers> are always allocated on
return, and it is therefore up to the calling function to deallocate.
Note returned fields are not allocated in arrays.
==============================================================================*/
{
	int i, return_code;
	struct Computed_field_composite_type_specific_data *data;

	ENTER(Computed_field_get_type_composite);
	if (field && (field->type_string == computed_field_composite_type_string) 
		&& (data = (struct Computed_field_composite_type_specific_data *)
		field->type_specific_data) && number_of_components &&
		number_of_source_fields && source_fields &&
		number_of_source_values && source_values &&
		source_field_numbers && source_value_numbers)
	{
		*number_of_components = field->number_of_components;
		*number_of_source_fields = field->number_of_source_fields;
		*source_fields = (struct Computed_field **)NULL;
		*number_of_source_values = field->number_of_source_values;
		*source_values = (FE_value *)NULL;
		*source_field_numbers = (int *)NULL;
		*source_value_numbers = (int *)NULL;
		if (((0 == field->number_of_source_fields) || ALLOCATE(*source_fields,
			struct Computed_field *, field->number_of_source_fields)) &&
			((0 == field->number_of_source_values) || ALLOCATE(*source_values,
				FE_value, field->number_of_source_values)) &&
			ALLOCATE(*source_field_numbers, int, field->number_of_components) &&
			ALLOCATE(*source_value_numbers, int, field->number_of_components))
		{
			for (i = 0; i < field->number_of_source_fields; i++)
			{
				(*source_fields)[i] = field->source_fields[i];
			}
			for (i = 0; i < field->number_of_source_values; i++)
			{
				(*source_values)[i] = field->source_values[i];
			}
			for (i = 0; i < field->number_of_components; i++)
			{
				(*source_field_numbers)[i] = data->source_field_numbers[i];
				(*source_value_numbers)[i] = data->source_value_numbers[i];
			}
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_composite.  Not enough memory");
			DEALLOCATE(*source_fields);
			DEALLOCATE(*source_values);
			DEALLOCATE(*source_field_numbers);
			DEALLOCATE(*source_value_numbers);
			DEALLOCATE(data);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_composite.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_composite */

struct Computed_field_composite_source_data
/*******************************************************************************
LAST MODIFIED : 20 October 2000

DESCRIPTION :
Data structure filled by set_Computed_field_composite_source_data.
==============================================================================*/
{
	int number_of_components;
	int number_of_source_fields;
	struct Computed_field **source_fields;
	int number_of_source_values;
	FE_value *source_values;
	int *source_field_numbers;
	int *source_value_numbers;
}; /* struct Computed_field_composite_source_data */

static int set_Computed_field_composite_source_data(struct Parse_state *state,
	void *source_data_void,void *computed_field_manager_void)
/*******************************************************************************
LAST MODIFIED : 14 December 2001

DESCRIPTION :
Note that fields are not ACCESSed by this function and should not be
ACCESSed in the initial source_data.
==============================================================================*/
{
	char *current_token, *field_component_name, *source_string, *temp_name;
	FE_value *temp_source_values, value;
	int component_no, components_to_add, i, number_of_characters, return_code,
		source_field_number, source_value_number, *temp_source_field_numbers,
		*temp_source_value_numbers;
	struct Computed_field *field, **temp_source_fields;
	struct Computed_field_composite_source_data *source_data;
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(set_Computed_field_composite_source_data);
	if (state && (source_data =
		(struct Computed_field_composite_source_data *)source_data_void) &&
		(computed_field_manager=
			(struct MANAGER(Computed_field) *)computed_field_manager_void))
	{
		return_code=1;
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				/* clear source_data */
				source_data->number_of_components = 0;
				source_data->number_of_source_fields = 0;
				if (source_data->source_fields)
				{
					DEALLOCATE(source_data->source_fields);
					source_data->source_fields = (struct Computed_field **)NULL;
				}
				source_data->number_of_source_values = 0;
				if (source_data->source_values)
				{
					DEALLOCATE(source_data->source_values);
					source_data->source_values = (FE_value *)NULL;
				}
				if (source_data->source_field_numbers)
				{
					DEALLOCATE(source_data->source_field_numbers);
					source_data->source_field_numbers = (int *)NULL;
				}
				if (source_data->source_value_numbers)
				{
					DEALLOCATE(source_data->source_value_numbers);
					source_data->source_value_numbers = (int *)NULL;
				}
				while (return_code && (current_token = state->current_token))
				{
					/* first try to find a number in the token */
					if (strchr("+-0123456789",current_token[0]))
					{
						if ((1 == sscanf(current_token, FE_VALUE_INPUT_STRING "%n",
							&value, &number_of_characters)) &&
							((int)strlen(current_token) == number_of_characters))
						{
							shift_Parse_state(state,1);
							if (REALLOCATE(temp_source_values, source_data->source_values,
								FE_value, source_data->number_of_source_values+1))
							{
								source_data->source_values = temp_source_values;
								temp_source_values[source_data->number_of_source_values] =
									value;
								components_to_add = 1;
								source_field_number = -1;
								source_value_number = source_data->number_of_source_values;
								(source_data->number_of_source_values)++;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"set_Computed_field_composite_source_data.  "
									"Not enough memory");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Invalid characters in composite value: %s",current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						if (field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
							current_token,computed_field_manager))
						{
							components_to_add = field->number_of_components;
							/* following is the first source_value_number to add */
							source_value_number = 0;
							shift_Parse_state(state,1);
						}
						else if (field_component_name=strchr(current_token,'.'))
						{
							*field_component_name='\0';
							field_component_name++;
							if (field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
								current_token,computed_field_manager))
							{
								component_no = -1;
								for (i=0;(0>component_no)&&(i<field->number_of_components)&&
									return_code;i++)
								{
									if (temp_name = Computed_field_get_component_name(field,i))
									{
										if (0 == strcmp(field_component_name,temp_name))
										{
											component_no = i;
										}
										DEALLOCATE(temp_name);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"set_Computed_field_composite_source_data.  "
											"Could not get component name");
										return_code=0;
									}
								}
								if (return_code)
								{
									if (0 <= component_no)
									{
										components_to_add = 1;
										source_value_number = component_no;
										shift_Parse_state(state,1);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"Unknown field component %s.%s",current_token,
											field_component_name);
										display_parse_state_location(state);
										return_code=0;
									}
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"Unknown field: %s",
									current_token);
								display_parse_state_location(state);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown field: %s",current_token);
							display_parse_state_location(state);
							return_code=0;
						}
						if (return_code)
						{
							/* try to find field in current source_fields */
							source_field_number = -1;
							for (i=0;(i<source_data->number_of_source_fields)&&
								(0>source_field_number);i++)
							{
								if (field == source_data->source_fields[i])
								{
									source_field_number = i;
								}
							}
							if (-1 == source_field_number)
							{
								if (REALLOCATE(temp_source_fields,
									source_data->source_fields,struct Computed_field *,
									source_data->number_of_source_fields+1))
								{
									source_data->source_fields = temp_source_fields;
									temp_source_fields[source_data->number_of_source_fields] =
										field;
									source_field_number = source_data->number_of_source_fields;
									(source_data->number_of_source_fields)++;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"set_Computed_field_composite_source_data.  "
										"Not enough memory");
									return_code=0;
								}
							}
						}
					}
					if (return_code)
					{
						if (REALLOCATE(temp_source_field_numbers,
							source_data->source_field_numbers,int,
							source_data->number_of_components+components_to_add))
						{
							source_data->source_field_numbers = temp_source_field_numbers;
							if (REALLOCATE(temp_source_value_numbers,
								source_data->source_value_numbers,int,
								source_data->number_of_components+components_to_add))
							{
								source_data->source_value_numbers = temp_source_value_numbers;
								for (i=0;i<components_to_add;i++)
								{
									source_data->source_field_numbers
										[source_data->number_of_components] = source_field_number;
									source_data->source_value_numbers
										[source_data->number_of_components] =
										source_value_number + i;
									(source_data->number_of_components)++;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"set_Computed_field_composite_source_data.  "
									"Not enough memory");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"set_Computed_field_composite_source_data.  Not enough memory");
							return_code=0;
						}
					}
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #|FIELD[.COMPONENT_NAME] "
					"#|FIELD[.COMPONENT_NAME] #|FIELD[.COMPONENT_NAME] ... ");
				if (0<source_data->number_of_components)
				{
					if (field=CREATE(Computed_field)("composite"))
					{
						if (Computed_field_set_type_composite(field,
							source_data->number_of_components,
							source_data->number_of_source_fields,source_data->source_fields,
							source_data->number_of_source_values,source_data->source_values,
							source_data->source_field_numbers,
							source_data->source_value_numbers)&&
							(source_string=Computed_field_composite_get_source_string(field,
								/*commands*/1)))
						{
							display_message(INFORMATION_MESSAGE,"[%s]",source_string);
							DEALLOCATE(source_string);
						}
						DESTROY(Computed_field)(&field);
					}
				}
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"Missing composition fields/components and values");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Computed_field_composite_source_data.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Computed_field_composite_source_data */

static int define_Computed_field_type_composite(struct Parse_state *state,
	void *field_void,void *computed_field_composite_package_void)
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_COMPOSITE (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field;
	struct Computed_field_composite_package *computed_field_composite_package;
	struct Computed_field_composite_source_data source_data;
	struct Option_table *option_table;

	ENTER(define_Computed_field_type_composite);
	if (state && (field = (struct Computed_field *)field_void) &&
		(computed_field_composite_package=
			(struct Computed_field_composite_package *)
			computed_field_composite_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_data.number_of_components = 0;
		source_data.number_of_source_fields = 0;
		source_data.source_fields = (struct Computed_field **)NULL;
		source_data.number_of_source_values = 0;
		source_data.source_values = (FE_value *)NULL;
		source_data.source_field_numbers = (int *)NULL;
		source_data.source_value_numbers = (int *)NULL;
		if (computed_field_composite_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_composite(field,
				&(source_data.number_of_components),
				&(source_data.number_of_source_fields), &(source_data.source_fields),
				&(source_data.number_of_source_values), &(source_data.source_values),
				&(source_data.source_field_numbers),
				&(source_data.source_value_numbers));
		}
		if (return_code)
		{
			/*???RC begin temporary code */
			/*???RC swallow up old "number_of_scalars # scalars" tokens used by old
				composite field command for backward compatibility */
			if (((state->current_index+2) < state->number_of_tokens) &&
				(fuzzy_string_compare(state->tokens[state->current_index],
					"number_of_scalars")) &&
				(0<atoi(state->tokens[state->current_index+1])) &&
				(fuzzy_string_compare(state->tokens[state->current_index+2],
					"scalars")))
			{
				shift_Parse_state(state,3);
			}
			/*???RC end temporary code */

			option_table = CREATE(Option_table)();
			/* default option: composite field definition */
			Option_table_add_entry(option_table, (char *)NULL, &source_data,
				computed_field_composite_package->computed_field_manager,
				set_Computed_field_composite_source_data);
			return_code = Option_table_multi_parse(option_table,state) &&
				Computed_field_set_type_composite(field,
				source_data.number_of_components,
				source_data.number_of_source_fields, source_data.source_fields,
				source_data.number_of_source_values, source_data.source_values,
				source_data.source_field_numbers,
				source_data.source_value_numbers);
			DESTROY(Option_table)(&option_table);
		}
		/* clean up the source data */
		if (source_data.source_fields)
		{
			DEALLOCATE(source_data.source_fields);
		}
		if (source_data.source_values)
		{
			DEALLOCATE(source_data.source_values);
		}
		if (source_data.source_field_numbers)
		{
			DEALLOCATE(source_data.source_field_numbers);
		}
		if (source_data.source_value_numbers)
		{
			DEALLOCATE(source_data.source_value_numbers);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_composite.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_composite */

int Computed_field_set_type_constant(struct Computed_field *field,
	int number_of_values, FE_value *values)
/*******************************************************************************
LAST MODIFIED : 14 December 2001

DESCRIPTION :
Changes <field> into type composite with <number_of_values> values listed in
the <values> array.
Since a constant field performs a subset of the capabilities of the composite
field type but the latter is somewhat complicated to set up, this is a
convenience function for building a composite field which has <number_of_values>
<values>. This function handles sorting so that no values are repeated.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int i, return_code, *source_field_numbers, *source_value_numbers;

	ENTER(Computed_field_set_type_constant);
	if (field && (0 < number_of_values) && values)
	{
		ALLOCATE(source_field_numbers, int, number_of_values);
		ALLOCATE(source_value_numbers, int, number_of_values);
		if (source_field_numbers && source_value_numbers)
		{
			for (i = 0; i < number_of_values; i++)
			{
				source_field_numbers[i] = -1;
				source_value_numbers[i] = i;
			}
			return_code = Computed_field_set_type_composite(field,
				/*number_of_components*/number_of_values,
				/*number_of_source_fields*/0,
				/*source_fields*/(struct Computed_field **)NULL,
				/*number_of_source_values*/number_of_values, /*source_values*/values,
				source_field_numbers, source_value_numbers);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_constant.  Not enough memory");
			return_code = 0;
		}
		DEALLOCATE(source_field_numbers);
		DEALLOCATE(source_value_numbers);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_constant.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_constant */

int Computed_field_is_constant(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 December 2001

DESCRIPTION :
Returns true if the field is of type composite but with no source_fields, only
source_values.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_constant);
	if (field)
	{
		return_code =
			(field->type_string == computed_field_composite_type_string) &&
			(0 == field->number_of_source_fields);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_constant.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_constant */

int Computed_field_is_constant_scalar(struct Computed_field *field,
	FE_value scalar)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns true if the field is of type composite but with no source_fields, only
a single source value, equal to <scalar>.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_constant_scalar);
	if (field)
	{
		return_code =
			(field->type_string == computed_field_composite_type_string) &&
			(0 == field->number_of_source_fields) &&
			(1 == field->number_of_components) &&
			(scalar == field->source_values[0]);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_constant_scalar.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_constant_scalar */

static int define_Computed_field_type_constant(struct Parse_state *state,
	void *field_void, void *computed_field_composite_package_void)
/*******************************************************************************
LAST MODIFIED : 19 December 2001

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CONSTANT (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	FE_value *values, *temp_values;
	int i, number_of_values, previous_number_of_values, return_code;
	struct Computed_field *field;
	struct Option_table *option_table;

	ENTER(define_Computed_field_type_constant);
	USE_PARAMETER(computed_field_composite_package_void);
	if (state && (field = (struct Computed_field *)field_void))
	{
		return_code = 1;
		previous_number_of_values = 0;
		values = (FE_value *)NULL;
		if (Computed_field_is_constant(field))
		{
			previous_number_of_values = field->number_of_source_values;
			if (ALLOCATE(values, FE_value, previous_number_of_values))
			{
				for (i = 0; i < previous_number_of_values; i++)
				{
					values[i] = field->source_values[i];
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_constant.  Could not allocate values");
				return_code = 0;
			}
		}
		if (return_code)
		{
			/*???RC begin temporary code */
			/*???RC swallow up old "number_of_values # values" tokens used by old
				constant field command for backward compatibility */
			if (((state->current_index + 2) < state->number_of_tokens) &&
				(fuzzy_string_compare(state->tokens[state->current_index],
					"number_of_values")) &&
				(0<atoi(state->tokens[state->current_index+1])) &&
				(fuzzy_string_compare(state->tokens[state->current_index+2],
					"values")))
			{
				shift_Parse_state(state,3);
			}
			/*???RC swallow up old "value N" tokens used by old constant field if
				default number_of_values=1 used ... for backward compatibility */
			else if (((state->current_index + 2) == state->number_of_tokens) &&
				fuzzy_string_compare(state->tokens[state->current_index], "values"))
			{
				shift_Parse_state(state, 1);
			}
			/*???RC end temporary code */

			if ((current_token = state->current_token) &&
				(!(strcmp(PARSER_HELP_STRING, current_token) &&
					strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))))
			{
				number_of_values = previous_number_of_values;
			}
			else if (0 < (number_of_values =
				state->number_of_tokens - state->current_index))
			{
				if (REALLOCATE(temp_values, values, FE_value, number_of_values))
				{
					values = temp_values;
					/* clear new values to 0.0 */
					for (i = previous_number_of_values; i < number_of_values; i++)
					{
						values[i] = 0.0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_constant.  "
						"Could not reallocate values");
					return_code = 0;
				}
			}
			else
			{
				number_of_values = 0;
			}
			if (return_code)
			{
				option_table = CREATE(Option_table)();
				/* default option: composite field definition */
				Option_table_add_entry(option_table, (char *)NULL, values,
					&number_of_values, set_FE_value_array);
				if (return_code = Option_table_multi_parse(option_table, state))
				{
					return_code =
						Computed_field_set_type_constant(field, number_of_values, values);
				}
				DESTROY(Option_table)(&option_table);
			}
			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING,state->current_token) &&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_constant.  Failed");
				}
			}
		}
		if (values)
		{
			DEALLOCATE(values);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_constant.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_constant */

int Computed_field_set_type_component(struct Computed_field *field,
	struct Computed_field *source_field, int component_number)
/*******************************************************************************
LAST MODIFIED : 14 December 2001

DESCRIPTION :
Changes <field> into type composite returning the single field component.
Since a component field performs a subset of the capabilities of the composite
field type but the latter is somewhat complicated to set up, this is a
convenience function for building a composite field for a scalar component.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int return_code, source_field_number;

	ENTER(Computed_field_set_type_component);
	if (field && source_field && (0 <= component_number) &&
		(component_number < source_field->number_of_components))
	{
		source_field_number = 0;
		return_code = Computed_field_set_type_composite(field,
			/*number_of_components*/1,
			/*number_of_source_fields*/1, /*source_fields*/&source_field,
			/*number_of_source_values*/0, /*source_values*/(FE_value *)NULL,
			&source_field_number, &component_number);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_component.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_component */

int Computed_field_is_component_wrapper(struct Computed_field *field,
	void *field_component_void)
/*******************************************************************************
LAST MODIFIED : 14 December 2001

DESCRIPTION :
Returns true if the field is of type composite with a single component matching
the passed Computed_field_component.
==============================================================================*/
{
	int return_code;
	struct Computed_field_component *field_component;
	struct Computed_field_composite_type_specific_data *data;

	ENTER(Computed_field_is_component_wrapper);
	if (field && (field_component =
		(struct Computed_field_component *)field_component_void))
	{
		return_code = (
			(field->type_string == computed_field_composite_type_string) &&
			(1 == field->number_of_components) &&
			(1 == field->number_of_source_fields) &&
			(field_component->field == field->source_fields[0]) &&
			(data = (struct Computed_field_composite_type_specific_data *)
				field->type_specific_data) &&
			(0 == data->source_field_numbers[0]) &&
			(field_component->component_no == data->source_value_numbers[0]));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_component_wrapper.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_is_component_wrapper */

struct Computed_field *Computed_field_manager_get_component_wrapper(
	struct MANAGER(Computed_field) *computed_field_manager,
	struct Computed_field *field, int component_number)
/*******************************************************************************
LAST MODIFIED : 14 December 2001

DESCRIPTION :
Returns a composite field that computes <component_number> of <field>. First
tries to find one in the manager that does this, otherwise makes one of name
'field.component_name', adds it to the manager and returns it.
==============================================================================*/
{
	char *component_field_name, *component_name;
	struct Computed_field *component_field;
	struct Computed_field_component field_component;

	ENTER(Computed_field_manager_get_component_wrapper);
	component_field = (struct Computed_field *)NULL;
	if (computed_field_manager && field && (0 <= component_number) &&
		(component_number < field->number_of_components))
	{
		field_component.field = field;
		field_component.component_no = component_number;
		/* try to find an existing wrapper for this component */
		if (!(component_field = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
			Computed_field_is_component_wrapper, &field_component,
			computed_field_manager)))
		{
			if (component_name =
				Computed_field_get_component_name(field, component_number))
			{
				if (ALLOCATE(component_field_name, char,
					strlen(field->name) + strlen(component_name) + 2))
				{
					sprintf(component_field_name, "%s.%s", field->name, component_name);
					if (component_field = CREATE(Computed_field)(component_field_name))
					{
						if (!(Computed_field_set_type_component(component_field, field,
							component_number) &&
							ADD_OBJECT_TO_MANAGER(Computed_field)(component_field,
								computed_field_manager)))
						{
							DESTROY(Computed_field)(&component_field);
						}
					}
					DEALLOCATE(component_field_name);
				}
				DEALLOCATE(component_name);
			}
			if (!component_field)
			{
				display_message(WARNING_MESSAGE,
					"Computed_field_manager_get_component_wrapper.  Failed");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_manager_get_component_wrapper.  Invalid argument(s)");
	}
	LEAVE;

	return (component_field);
} /* Computed_field_manager_get_component_wrapper */

int Computed_field_register_types_composite(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 19 December 2001

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_composite_package
		computed_field_composite_package;

	ENTER(Computed_field_register_types_composite);
	if (computed_field_package)
	{
		computed_field_composite_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(computed_field_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_composite_type_string,
			define_Computed_field_type_composite,
			&computed_field_composite_package);
		/* "constant" = alias for composite included for backward compatibility */
		return_code = Computed_field_package_add_type(computed_field_package,
			"constant", define_Computed_field_type_constant,
			&computed_field_composite_package);
		/* "component" = alias for composite included for backward compatibility */
		return_code = Computed_field_package_add_type(computed_field_package,
			"component", define_Computed_field_type_composite,
			&computed_field_composite_package);
		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_composite.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_composite */
