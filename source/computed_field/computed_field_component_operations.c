/*******************************************************************************
FILE : computed_field_component_operations.c

LAST MODIFIED : 13 December 2001

DESCRIPTION :
Implements a number of basic component wise operations on computed fields.
==============================================================================*/
#include <math.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.h"
#include "general/debug.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_component_operations.h"

struct Computed_field_component_operations_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
};

static char computed_field_multiply_components_type_string[] = "multiply_components";

int Computed_field_is_type_multiply_components(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_multiply_components);
	if (field)
	{
		return_code = 
		  (field->type_string == computed_field_multiply_components_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_multiply_components.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_multiply_components */

static int Computed_field_multiply_components_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_multiply_components_clear_type_specific);
	if (field)
	{
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_multiply_components_clear_type_specific.  "
			"Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_multiply_components_clear_type_specific */

static void *Computed_field_multiply_components_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	void *destination

	ENTER(Computed_field_multiply_components_copy_type_specific);
	if (field)
	{
		destination = (void *)1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_multiply_components_copy_type_specific.  "
			"Invalid field");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_multiply_components_copy_type_specific */

#define Computed_field_multiply_components_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_multiply_components_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_component_operations_type_specific_contents_match);
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
} /* Computed_field_component_operations_type_specific_contents_match */

#define Computed_field_multiply_components_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_multiply_components_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_multiply_components_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_multiply_components_can_be_destroyed \
	(Computed_field_can_be_destroyed_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
No special criteria on the destroy
==============================================================================*/

static int Computed_field_multiply_components_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_multiply_components_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 2) && 
		(field->number_of_components == field->source_fields[0]->number_of_components) && 
		(field->number_of_components == field->source_fields[1]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] = field->source_fields[0]->values[i] * 
				  field->source_fields[1]->values[i];
			}			
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_multiply_components_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_multiply_components_evaluate_cache_at_node */

static int Computed_field_multiply_components_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value *derivative;
	int i, j, number_of_xi, return_code;

	ENTER(Computed_field_multiply_components_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields > 0) && 
		(field->number_of_components == field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] = field->source_fields[0]->values[i] * 
					field->source_fields[1]->values[i];
			}
			if (calculate_derivatives && field->source_fields[0]->derivatives_valid
				&& field->source_fields[1]->derivatives_valid)
			{
				derivative = field->derivatives;
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					for (j = 0 ; j < number_of_xi ; j++)
					{
						*derivative = 
							field->source_fields[0]->derivatives[i * number_of_xi + j] *
							field->source_fields[1]->values[i] + 
							field->source_fields[1]->derivatives[i * number_of_xi + j] *
							field->source_fields[0]->values[i];
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
			"Computed_field_multiply_components_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_multiply_components_evaluate_cache_in_element */

#define Computed_field_multiply_components_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_multiply_components_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_multiply_components_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_multiply_components_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_multiply_components_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_multiply_components_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_multiply_components(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_multiply_components);
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
			"list_Computed_field_multiply_components.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_multiply_components */

static int list_Computed_field_multiply_components_commands(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(list_Computed_field_multiply_components_commands);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			" fields %s %s\n",field->source_fields[0]->name,
			field->source_fields[1]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_multiply_components_commands.  "
			"Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_multiply_components_commands */

int Computed_field_set_type_multiply_components(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_MULTIPLY_COMPONENTS with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_multiply_components);
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
			field->type=COMPUTED_FIELD_NEW_TYPES;
			field->type_string = computed_field_multiply_components_type_string;
			field->number_of_components = source_field_one->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field_one);
			source_fields[1]=ACCESS(Computed_field)(source_field_two);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			field->computed_field_clear_type_specific_function =
				Computed_field_multiply_components_clear_type_specific;
			field->computed_field_copy_type_specific_function =
				Computed_field_multiply_components_copy_type_specific;
			field->computed_field_clear_cache_type_specific_function =
				Computed_field_multiply_components_clear_cache_type_specific;
			field->computed_field_type_specific_contents_match_function =
				Computed_field_multiply_components_type_specific_contents_match;
			field->computed_field_is_defined_in_element_function =
				Computed_field_multiply_components_is_defined_in_element;
			field->computed_field_is_defined_at_node_function =
				Computed_field_multiply_components_is_defined_at_node;
			field->computed_field_has_numerical_components_function =
				Computed_field_multiply_components_has_numerical_components;
			field->computed_field_can_be_destroyed_function =
				Computed_field_multiply_components_can_be_destroyed;
			field->computed_field_evaluate_cache_at_node_function =
				Computed_field_multiply_components_evaluate_cache_at_node;
			field->computed_field_evaluate_cache_in_element_function =
				Computed_field_multiply_components_evaluate_cache_in_element;
			field->computed_field_evaluate_as_string_at_node_function =
				Computed_field_multiply_components_evaluate_as_string_at_node;
			field->computed_field_evaluate_as_string_in_element_function =
				Computed_field_multiply_components_evaluate_as_string_in_element;
			field->computed_field_set_values_at_node_function =
				Computed_field_multiply_components_set_values_at_node;
			field->computed_field_set_values_in_element_function =
				Computed_field_multiply_components_set_values_in_element;
			field->computed_field_get_native_discretization_in_element_function =
				Computed_field_multiply_components_get_native_discretization_in_element;
			field->computed_field_find_element_xi_function =
				Computed_field_multiply_components_find_element_xi;
			field->list_Computed_field_function = 
				list_Computed_field_multiply_components;
			field->list_Computed_field_commands_function = 
				list_Computed_field_multiply_components_commands;
			field->computed_field_has_multiple_times_function = 
				Computed_field_default_has_multiple_times;
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
			"Computed_field_set_type_multiply_components.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_multiply_components */

int Computed_field_get_type_multiply_components(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_MULTIPLY_COMPONENTS, the 
<source_field_one> and <source_field_two> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_multiply_components);
	if (field&&(COMPUTED_FIELD_NEW_TYPES==field->type)&&
		(field->type_string==computed_field_multiply_components_type_string))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_multiply_components.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_multiply_components */

static int define_Computed_field_type_multiply_components(struct Parse_state *state,
	void *field_void,void *computed_field_component_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_MULTIPLY_COMPONENTS (if it is not 
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

	ENTER(define_Computed_field_type_multiply_components);
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
			if (computed_field_multiply_components_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_multiply_components(field, 
					source_fields, source_fields + 1);
			}
			else
			{
				if (source_fields[0]=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_has_numerical_components,(void *)NULL,
					computed_field_component_operations_package->computed_field_manager))
				{
					source_fields[1] = source_fields[0];
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_multiply_components.  No fields defined");
					return_code = 0;
				}
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
					return_code = Computed_field_set_type_multiply_components(field,
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
							"define_Computed_field_type_multiply_components.  Failed");
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
				"define_Computed_field_type_multiply_components.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_multiply_components.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_multiply_components */

static char computed_field_divide_components_type_string[] = "divide_components";

int Computed_field_is_type_divide_components(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_divide_components);
	if (field)
	{
		return_code = 
		  (field->type_string == computed_field_divide_components_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_divide_components.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_divide_components */

static int Computed_field_divide_components_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_divide_components_clear_type_specific);
	if (field)
	{
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_divide_components_clear_type_specific.  "
			"Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_divide_components_clear_type_specific */

static void *Computed_field_divide_components_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	void *destination

	ENTER(Computed_field_divide_components_copy_type_specific);
	if (field)
	{
		destination = (void *)1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_divide_components_copy_type_specific.  "
			"Invalid field");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_divide_components_copy_type_specific */

#define Computed_field_divide_components_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_divide_components_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_component_operations_type_specific_contents_match);
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
} /* Computed_field_component_operations_type_specific_contents_match */

#define Computed_field_divide_components_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_divide_components_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_divide_components_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_divide_components_can_be_destroyed \
	(Computed_field_can_be_destroyed_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
No special criteria on the destroy
==============================================================================*/

static int Computed_field_divide_components_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_divide_components_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 2) && 
		(field->number_of_components == field->source_fields[0]->number_of_components) && 
		(field->number_of_components == field->source_fields[1]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] = field->source_fields[0]->values[i] /
				  field->source_fields[1]->values[i];
			}			
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_divide_components_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_divide_components_evaluate_cache_at_node */

static int Computed_field_divide_components_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value *derivative, vsquared;
	int i, j, number_of_xi, return_code;

	ENTER(Computed_field_divide_components_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields == 2) && 
		(field->number_of_components == field->source_fields[0]->number_of_components && 
		(field->number_of_components == field->source_fields[1]->number_of_components)))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] = field->source_fields[0]->values[i] / 
					field->source_fields[1]->values[i];
			}
			if (calculate_derivatives && field->source_fields[0]->derivatives_valid
				&& field->source_fields[1]->derivatives_valid)
			{
				derivative = field->derivatives;
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					vsquared = field->source_fields[1]->values[i] 
						* field->source_fields[1]->values[i];
					for (j = 0 ; j < number_of_xi ; j++)
					{
						*derivative = 
							(field->source_fields[0]->derivatives[i * number_of_xi + j] *
							field->source_fields[1]->values[i] -
							field->source_fields[1]->derivatives[i * number_of_xi + j] *
							field->source_fields[0]->values[i]) / vsquared;
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
			"Computed_field_divide_components_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_divide_components_evaluate_cache_in_element */

#define Computed_field_divide_components_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_divide_components_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_divide_components_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_divide_components_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_divide_components_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_divide_components_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_divide_components(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_divide_components);
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
			"list_Computed_field_divide_components.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_divide_components */

static int list_Computed_field_divide_components_commands(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(list_Computed_field_divide_components_commands);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			" fields %s %s\n",field->source_fields[0]->name,
			field->source_fields[1]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_divide_components_commands.  "
			"Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_divide_components_commands */

int Computed_field_set_type_divide_components(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_DIVIDE_COMPONENTS with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_divide_components);
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
			field->type=COMPUTED_FIELD_NEW_TYPES;
			field->type_string = computed_field_divide_components_type_string;
			field->number_of_components = source_field_one->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field_one);
			source_fields[1]=ACCESS(Computed_field)(source_field_two);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			field->computed_field_clear_type_specific_function =
				Computed_field_divide_components_clear_type_specific;
			field->computed_field_copy_type_specific_function =
				Computed_field_divide_components_copy_type_specific;
			field->computed_field_clear_cache_type_specific_function =
				Computed_field_divide_components_clear_cache_type_specific;
			field->computed_field_type_specific_contents_match_function =
				Computed_field_divide_components_type_specific_contents_match;
			field->computed_field_is_defined_in_element_function =
				Computed_field_divide_components_is_defined_in_element;
			field->computed_field_is_defined_at_node_function =
				Computed_field_divide_components_is_defined_at_node;
			field->computed_field_has_numerical_components_function =
				Computed_field_divide_components_has_numerical_components;
			field->computed_field_can_be_destroyed_function =
				Computed_field_divide_components_can_be_destroyed;
			field->computed_field_evaluate_cache_at_node_function =
				Computed_field_divide_components_evaluate_cache_at_node;
			field->computed_field_evaluate_cache_in_element_function =
				Computed_field_divide_components_evaluate_cache_in_element;
			field->computed_field_evaluate_as_string_at_node_function =
				Computed_field_divide_components_evaluate_as_string_at_node;
			field->computed_field_evaluate_as_string_in_element_function =
				Computed_field_divide_components_evaluate_as_string_in_element;
			field->computed_field_set_values_at_node_function =
				Computed_field_divide_components_set_values_at_node;
			field->computed_field_set_values_in_element_function =
				Computed_field_divide_components_set_values_in_element;
			field->computed_field_get_native_discretization_in_element_function =
				Computed_field_divide_components_get_native_discretization_in_element;
			field->computed_field_find_element_xi_function =
				Computed_field_divide_components_find_element_xi;
			field->list_Computed_field_function = 
				list_Computed_field_divide_components;
			field->list_Computed_field_commands_function = 
				list_Computed_field_divide_components_commands;
			field->computed_field_has_multiple_times_function = 
				Computed_field_default_has_multiple_times;
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
			"Computed_field_set_type_divide_components.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_divide_components */

int Computed_field_get_type_divide_components(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_DIVIDE_COMPONENTS, the 
<source_field_one> and <source_field_two> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_divide_components);
	if (field&&(COMPUTED_FIELD_NEW_TYPES==field->type)&&
		(field->type_string==computed_field_divide_components_type_string))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_divide_components.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_divide_components */

static int define_Computed_field_type_divide_components(struct Parse_state *state,
	void *field_void,void *computed_field_component_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_DIVIDE_COMPONENTS (if it is not 
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

	ENTER(define_Computed_field_type_divide_components);
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
			if (computed_field_divide_components_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_divide_components(field, 
					source_fields, source_fields + 1);
			}
			else
			{
				if (source_fields[0]=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_has_numerical_components,(void *)NULL,
					computed_field_component_operations_package->computed_field_manager))
				{
					source_fields[1] = source_fields[0];
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_divide_components.  No fields defined");
					return_code = 0;
				}
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
					return_code = Computed_field_set_type_divide_components(field,
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
							"define_Computed_field_type_divide_components.  Failed");
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
				"define_Computed_field_type_divide_components.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_divide_components.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_divide_components */

static char computed_field_add_type_string[] = "add";

int Computed_field_is_type_add(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_add);
	if (field)
	{
		return_code = 
		  (field->type_string == computed_field_add_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_add.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_add */

static int Computed_field_add_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_add_clear_type_specific);
	if (field)
	{
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_add_clear_type_specific.  "
			"Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_add_clear_type_specific */

static void *Computed_field_add_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	void *destination

	ENTER(Computed_field_add_copy_type_specific);
	if (field)
	{
		destination = (void *)1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_add_copy_type_specific.  "
			"Invalid field");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_add_copy_type_specific */

#define Computed_field_add_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_add_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_component_operations_type_specific_contents_match);
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
} /* Computed_field_component_operations_type_specific_contents_match */

#define Computed_field_add_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_add_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_add_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_add_can_be_destroyed \
	(Computed_field_can_be_destroyed_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
No special criteria on the destroy
==============================================================================*/

static int Computed_field_add_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_add_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 2) && 
		(field->number_of_components == field->source_fields[0]->number_of_components) && 
		(field->number_of_components == field->source_fields[1]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			for (i=0;i<field->number_of_components;i++)
			{
				field->values[i]=
					field->source_values[0]*field->source_fields[0]->values[i]+
					field->source_values[1]*field->source_fields[1]->values[i];
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_add_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_add_evaluate_cache_at_node */

static int Computed_field_add_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value *temp, *temp1, *temp2;
	int element_dimension, i, return_code;

	ENTER(Computed_field_add_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields == 2) && 
		(field->number_of_components == field->source_fields[0]->number_of_components) && 
		(field->number_of_components == field->source_fields[1]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		element_dimension=element->shape->dimension;
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			for (i=0;i<field->number_of_components;i++)
			{
				field->values[i]=
					field->source_values[0]*field->source_fields[0]->values[i]+
					field->source_values[1]*field->source_fields[1]->values[i];
			}
			if (calculate_derivatives)
			{
				temp=field->derivatives;
				temp1=field->source_fields[0]->derivatives;
				temp2=field->source_fields[1]->derivatives;
				for (i=(field->number_of_components*element_dimension);
					  0<i;i--)
				{
					(*temp)=field->source_values[0]*(*temp1)+
						field->source_values[1]*(*temp2);
					temp++;
					temp1++;
					temp2++;
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
			"Computed_field_add_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_add_evaluate_cache_in_element */

#define Computed_field_add_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_add_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_add_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_add_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_add_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_add_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_add(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_add);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    field 1 : %s\n    scale factor 1 : %g\n",
			field->source_fields[0]->name,field->source_values[0]);
		display_message(INFORMATION_MESSAGE,
			"    field 2 : %s\n    scale factor 2 : %g\n",
			field->source_fields[1]->name,field->source_values[1]);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_add.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_add */

static int list_Computed_field_add_commands(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(list_Computed_field_add_commands);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			" fields %s %s scale_factors %g %g",
			field->source_fields[0]->name,field->source_fields[1]->name,
			field->source_values[0],field->source_values[1]);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_add_commands.  "
			"Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_add_commands */

int Computed_field_set_type_add(struct Computed_field *field,
	struct Computed_field *source_field_one, FE_value scale_factor1,
	struct Computed_field *source_field_two, FE_value scale_factor2)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_ADD with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	FE_value *source_values;
	int number_of_source_fields,number_of_source_values,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_add);
	if (field&&source_field_one&&source_field_two&&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components))
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=2;
		number_of_source_values=2;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields)&&
			ALLOCATE(source_values,FE_value,number_of_source_values))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_NEW_TYPES;
			field->type_string = computed_field_add_type_string;
			field->number_of_components = source_field_one->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field_one);
			source_fields[1]=ACCESS(Computed_field)(source_field_two);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			source_values[0]=scale_factor1;
			source_values[1]=scale_factor2;
			field->source_values=source_values;
			field->number_of_source_values=number_of_source_values;
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			field->computed_field_clear_type_specific_function =
				Computed_field_add_clear_type_specific;
			field->computed_field_copy_type_specific_function =
				Computed_field_add_copy_type_specific;
			field->computed_field_clear_cache_type_specific_function =
				Computed_field_add_clear_cache_type_specific;
			field->computed_field_type_specific_contents_match_function =
				Computed_field_add_type_specific_contents_match;
			field->computed_field_is_defined_in_element_function =
				Computed_field_add_is_defined_in_element;
			field->computed_field_is_defined_at_node_function =
				Computed_field_add_is_defined_at_node;
			field->computed_field_has_numerical_components_function =
				Computed_field_add_has_numerical_components;
			field->computed_field_can_be_destroyed_function =
				Computed_field_add_can_be_destroyed;
			field->computed_field_evaluate_cache_at_node_function =
				Computed_field_add_evaluate_cache_at_node;
			field->computed_field_evaluate_cache_in_element_function =
				Computed_field_add_evaluate_cache_in_element;
			field->computed_field_evaluate_as_string_at_node_function =
				Computed_field_add_evaluate_as_string_at_node;
			field->computed_field_evaluate_as_string_in_element_function =
				Computed_field_add_evaluate_as_string_in_element;
			field->computed_field_set_values_at_node_function =
				Computed_field_add_set_values_at_node;
			field->computed_field_set_values_in_element_function =
				Computed_field_add_set_values_in_element;
			field->computed_field_get_native_discretization_in_element_function =
				Computed_field_add_get_native_discretization_in_element;
			field->computed_field_find_element_xi_function =
				Computed_field_add_find_element_xi;
			field->list_Computed_field_function = 
				list_Computed_field_add;
			field->list_Computed_field_commands_function = 
				list_Computed_field_add_commands;
			field->computed_field_has_multiple_times_function = 
				Computed_field_default_has_multiple_times;
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
			"Computed_field_set_type_add.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_add */

int Computed_field_get_type_add(struct Computed_field *field,
	struct Computed_field **source_field_one, FE_value *scale_factor1,
	struct Computed_field **source_field_two, FE_value *scale_factor2)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_ADD, the 
<source_field_one> and <source_field_two> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_add);
	if (field&&(COMPUTED_FIELD_NEW_TYPES==field->type)&&
		(field->type_string==computed_field_add_type_string))
	{
		*source_field_one = field->source_fields[0];
		*scale_factor1 = field->source_values[0];
		*source_field_two = field->source_fields[1];
		*scale_factor2 = field->source_values[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_add.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_add */

static int define_Computed_field_type_add(struct Parse_state *state,
	void *field_void,void *computed_field_component_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_ADD (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	FE_value *scale_factors;
	int number_of_scale_factors,return_code;
	struct Computed_field *field,**source_fields;
	struct Computed_field_component_operations_package 
		*computed_field_component_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_add);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_component_operations_package=
		(struct Computed_field_component_operations_package *)
		computed_field_component_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		scale_factors=(FE_value *)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2)&&
			ALLOCATE(scale_factors, FE_value, 2))
		{
			if (computed_field_add_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_add(field, 
					source_fields, scale_factors,
					source_fields + 1, scale_factors + 1);
			}
			else
			{
				if (source_fields[0]=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_has_numerical_components,(void *)NULL,
					computed_field_component_operations_package->computed_field_manager))
				{
					source_fields[1] = source_fields[0];
					/* default scale factors for simple add */
					scale_factors[0] = 1.0;
					scale_factors[1] = 1.0;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_add.  No fields defined");
					return_code = 0;
				}
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
				set_source_field_data.conditional_function=Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=2;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				number_of_scale_factors=2;
				Option_table_add_entry(option_table,"scale_factors",scale_factors,
					&number_of_scale_factors,set_FE_value_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = Computed_field_set_type_add(field,
						source_fields[0], scale_factors[0],
						source_fields[1], scale_factors[1]);
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_add.  Failed");
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
			DEALLOCATE(scale_factors);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_add.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_add.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_add */

static char computed_field_scale_type_string[] = "scale";

int Computed_field_is_type_scale(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_scale);
	if (field)
	{
		return_code = (field->type_string == computed_field_scale_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_scale.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_scale */

static int Computed_field_scale_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_scale_clear_type_specific);
	if (field)
	{
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_scale_clear_type_specific.  "
			"Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_scale_clear_type_specific */

static void *Computed_field_scale_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	void *destination

	ENTER(Computed_field_scale_copy_type_specific);
	if (field)
	{
		destination = (void *)1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_scale_copy_type_specific.  "
			"Invalid field");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_scale_copy_type_specific */

#define Computed_field_scale_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_scale_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_component_operations_type_specific_contents_match);
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
} /* Computed_field_component_operations_type_specific_contents_match */

#define Computed_field_scale_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_scale_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_scale_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_scale_can_be_destroyed \
	(Computed_field_can_be_destroyed_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
No special criteria on the destroy
==============================================================================*/

static int Computed_field_scale_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_scale_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 1) && 
		(field->number_of_components == field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			for (i=0;i<field->number_of_components;i++)
			{
				field->values[i]=
					field->source_values[i]*field->source_fields[0]->values[i];
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_scale_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_scale_evaluate_cache_at_node */

static int Computed_field_scale_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value *temp, *temp2;
	int element_dimension, i, j, return_code;

	ENTER(Computed_field_scale_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields == 1) && 
		(field->number_of_components == field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			element_dimension=get_FE_element_dimension(element);
			/* 2. Calculate the field */
			for (i=0;i<field->number_of_components;i++)
			{
				field->values[i]=
					field->source_values[i]*field->source_fields[0]->values[i];
			}
			if (calculate_derivatives)
			{
				temp=field->derivatives;
				temp2=field->source_fields[0]->derivatives;
				for (i=0;i<field->number_of_components;i++)
				{
					for (j=0;j<element_dimension;j++)
					{
						(*temp)=field->source_values[i]*(*temp2);
						temp++;
						temp2++;
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
			"Computed_field_scale_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_scale_evaluate_cache_in_element */

#define Computed_field_scale_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_scale_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

static int Computed_field_scale_set_values_at_node(struct Computed_field *field,
	struct FE_node *node,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
==============================================================================*/
{
	FE_value *source_values;
	int i,return_code;

	ENTER(Computed_field_scale_set_values_at_node);
	if (field && node && values)
	{
		return_code = 1;
		/* reverse the scaling - unless any scale_factors are zero */
		if (ALLOCATE(source_values,FE_value,field->number_of_components))
		{
			for (i=0;(i<field->number_of_components)&&return_code;i++)
			{
				if (0.0 != field->source_values[i])
				{
					source_values[i] = values[i] / field->source_values[i];
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_set_values_at_node.  "
						"Cannot invert scale field %s with zero scale factor",
						field->name);
					return_code = 0;
				}
			}
			if (return_code)
			{
				return_code=Computed_field_set_values_at_node(
					field->source_fields[0],node,source_values);
			}
			DEALLOCATE(source_values);
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_scale_set_values_at_node.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_scale_set_values_at_node */

static int Computed_field_scale_set_values_in_element(struct Computed_field *field,
	struct FE_element *element,int *number_in_xi,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
==============================================================================*/
{
	FE_value scale_value, *source_values;
	int element_dimension, i, j, k, number_of_points, offset,return_code;

	ENTER(Computed_field_scale_set_values_in_element);
	if (field && element && number_in_xi && values)
	{
		return_code=1;
		element_dimension=element->shape->dimension;
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
					"Computed_field_scale_set_values_in_element.  "
					"number_in_xi must be positive");
				return_code = 0;
			}
		}
		if (return_code)
		{
			/* reverse the scaling */
			if (ALLOCATE(source_values,FE_value,
				number_of_points*field->number_of_components))
			{
				for (k=0;(k<field->number_of_components)&&return_code;k++)
				{
					offset=k*number_of_points;
					scale_value=field->source_values[k];
					if (0.0 != scale_value)
					{
						for (j=0;j<number_of_points;j++)
						{
							source_values[offset+j] = values[offset+j] / scale_value;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_scale_set_values_in_element.  "
							"Cannot invert scale field %s with zero scale factor",
							field->name);
						return_code = 0;
					}
				}
				if (return_code)
				{
					return_code=Computed_field_set_values_in_element(
						field->source_fields[0],element,number_in_xi,source_values);
				}
				DEALLOCATE(source_values);
			}
			else
			{
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_scale_set_values_in_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_scale_set_values_in_element */

#define Computed_field_scale_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_scale_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_scale(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;

	ENTER(List_Computed_field_scale);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    field : %s\n",
			field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,"    scale_factors :");
		for (i=0;i<field->source_fields[0]->number_of_components;i++)
		{
			display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
		}
		display_message(INFORMATION_MESSAGE,"\n");
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_scale.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_scale */

static int list_Computed_field_scale_commands(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;

	ENTER(list_Computed_field_scale_commands);
	if (field)
	{
		display_message(INFORMATION_MESSAGE," field %s scale_factors",
			field->source_fields[0]->name);
		for (i=0;i<field->source_fields[0]->number_of_components;i++)
		{
			display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_scale_commands.  "
			"Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_scale_commands */

int Computed_field_set_type_scale(struct Computed_field *field,
	struct Computed_field *source_field, FE_value *scale_factors)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_SCALE which scales the values of the
<source_field> by <scale_factors>.
Sets the number of components equal to that of <source_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int i,number_of_source_fields,number_of_source_values,return_code;
	FE_value *source_values;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_scale);
	if (field&&source_field&&(source_field->number_of_components>0))
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_values=source_field->number_of_components;
		number_of_source_fields=1;
		if (ALLOCATE(source_values,FE_value,number_of_source_values)&&
			ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_NEW_TYPES;
			field->type_string = computed_field_scale_type_string;
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			for (i=0;i<number_of_source_values;i++)
			{
				source_values[i]=scale_factors[i];
			}
			field->source_values=source_values;
			field->number_of_source_values=number_of_source_values;
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			field->computed_field_clear_type_specific_function =
				Computed_field_scale_clear_type_specific;
			field->computed_field_copy_type_specific_function =
				Computed_field_scale_copy_type_specific;
			field->computed_field_clear_cache_type_specific_function =
				Computed_field_scale_clear_cache_type_specific;
			field->computed_field_type_specific_contents_match_function =
				Computed_field_scale_type_specific_contents_match;
			field->computed_field_is_defined_in_element_function =
				Computed_field_scale_is_defined_in_element;
			field->computed_field_is_defined_at_node_function =
				Computed_field_scale_is_defined_at_node;
			field->computed_field_has_numerical_components_function =
				Computed_field_scale_has_numerical_components;
			field->computed_field_can_be_destroyed_function =
				Computed_field_scale_can_be_destroyed;
			field->computed_field_evaluate_cache_at_node_function =
				Computed_field_scale_evaluate_cache_at_node;
			field->computed_field_evaluate_cache_in_element_function =
				Computed_field_scale_evaluate_cache_in_element;
			field->computed_field_evaluate_as_string_at_node_function =
				Computed_field_scale_evaluate_as_string_at_node;
			field->computed_field_evaluate_as_string_in_element_function =
				Computed_field_scale_evaluate_as_string_in_element;
			field->computed_field_set_values_at_node_function =
				Computed_field_scale_set_values_at_node;
			field->computed_field_set_values_in_element_function =
				Computed_field_scale_set_values_in_element;
			field->computed_field_get_native_discretization_in_element_function =
				Computed_field_scale_get_native_discretization_in_element;
			field->computed_field_find_element_xi_function =
				Computed_field_scale_find_element_xi;
			field->list_Computed_field_function = 
				list_Computed_field_scale;
			field->list_Computed_field_commands_function = 
				list_Computed_field_scale_commands;
			field->computed_field_has_multiple_times_function = 
				Computed_field_default_has_multiple_times;
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
			"Computed_field_set_type_scale.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_scale */

int Computed_field_get_type_scale(struct Computed_field *field,
	struct Computed_field **source_field, FE_value **scale_factors)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_SCALE, the 
<source_field_one> and <source_field_two> used by it are returned.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_get_type_scale);
	if (field&&(COMPUTED_FIELD_NEW_TYPES==field->type)&&
		(field->type_string==computed_field_scale_type_string))
	{
		if (ALLOCATE(*scale_factors,FE_value,
			field->source_fields[0]->number_of_components))
		{
			*source_field=field->source_fields[0];
			for (i=0;i<field->source_fields[0]->number_of_components;i++)
			{
				(*scale_factors)[i]=field->source_values[i];
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_scale.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_scale.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_scale */

static int define_Computed_field_type_scale(struct Parse_state *state,
	void *field_void,void *computed_field_component_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 6 November 2001

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_SCALE (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	FE_value *scale_factors, *temp_scale_factors;
	int i, number_of_scale_factors, previous_number_of_scale_factors, return_code;
	struct Computed_field *field,*source_field;
	struct Computed_field_component_operations_package 
		*computed_field_component_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_scale);
	if (state && (field = (struct Computed_field *)field_void) &&
		(computed_field_component_operations_package =
			(struct Computed_field_component_operations_package *)
			computed_field_component_operations_package_void))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		set_source_field_data.computed_field_manager =
			computed_field_component_operations_package->computed_field_manager;
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		source_field = (struct Computed_field *)NULL;
		scale_factors = (FE_value *)NULL;
		previous_number_of_scale_factors = 0;
		if (computed_field_scale_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code =
				Computed_field_get_type_scale(field, &source_field, &scale_factors);
		}
		if (return_code)
		{
			if (source_field)
			{
				previous_number_of_scale_factors = source_field->number_of_components;
				ACCESS(Computed_field)(source_field);
			}
			if ((current_token=state->current_token) &&
				(!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
				option_table = CREATE(Option_table)();					
				Option_table_add_entry(option_table, "field", &source_field,
					&set_source_field_data, set_Computed_field_conditional);
				Option_table_add_entry(option_table, "scale_factors", scale_factors,
					&previous_number_of_scale_factors, set_FE_value_array);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			/* parse the field ... */
			if (return_code && (current_token = state->current_token))
			{
				/* ... only if the "field" token is next */
				if (fuzzy_string_compare(current_token, "field"))
				{
					option_table = CREATE(Option_table)();
					/* field */
					Option_table_add_entry(option_table, "field", &source_field,
						&set_source_field_data, set_Computed_field_conditional);
					if (return_code = Option_table_parse(option_table, state))
					{
						if (source_field)
						{
							number_of_scale_factors = source_field->number_of_components;
							if (REALLOCATE(temp_scale_factors, scale_factors, FE_value,
								number_of_scale_factors))
							{
								scale_factors = temp_scale_factors;
								/* make any new scale_factors equal to 1.0 */
								for (i = previous_number_of_scale_factors;
									i < number_of_scale_factors; i++)
								{
									scale_factors[i] = 1.0;
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
								"define_Computed_field_type_scale.  Invalid field");
							return_code = 0;
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			/* parse the scale_factors */
			if (return_code&&state->current_token)
			{
				option_table = CREATE(Option_table)();
				number_of_scale_factors=source_field->number_of_components;
				Option_table_add_entry(option_table,"scale_factors",scale_factors,
					&number_of_scale_factors,set_FE_value_array);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_scale(field,
					source_field, scale_factors);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_scale.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			DESTROY(Option_table)(&option_table);
		}
		DEALLOCATE(scale_factors);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_scale.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_scale */

static char computed_field_clamp_maximum_type_string[] = "clamp_maximum";

int Computed_field_is_type_clamp_maximum(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_clamp_maximum);
	if (field)
	{
		return_code = (field->type_string == computed_field_clamp_maximum_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_clamp_maximum.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_clamp_maximum */

static int Computed_field_clamp_maximum_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_clamp_maximum_clear_type_specific);
	if (field)
	{
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_clamp_maximum_clear_type_specific.  "
			"Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_clamp_maximum_clear_type_specific */

static void *Computed_field_clamp_maximum_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	void *destination

	ENTER(Computed_field_clamp_maximum_copy_type_specific);
	if (field)
	{
		destination = (void *)1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_clamp_maximum_copy_type_specific.  "
			"Invalid field");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_clamp_maximum_copy_type_specific */

#define Computed_field_clamp_maximum_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_clamp_maximum_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_component_operations_type_specific_contents_match);
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
} /* Computed_field_component_operations_type_specific_contents_match */

#define Computed_field_clamp_maximum_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_clamp_maximum_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_clamp_maximum_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_clamp_maximum_can_be_destroyed \
	(Computed_field_can_be_destroyed_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
No special criteria on the destroy
==============================================================================*/

static int Computed_field_clamp_maximum_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_clamp_maximum_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 1) && 
		(field->number_of_components == field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			for (i=0;i<field->number_of_components;i++)
			{
				if (field->source_fields[0]->values[i] < field->source_values[i])
				{
					field->values[i]=field->source_fields[0]->values[i];
				}
				else
				{
					field->values[i]=field->source_values[i];
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_clamp_maximum_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_clamp_maximum_evaluate_cache_at_node */

static int Computed_field_clamp_maximum_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value *temp, *temp2;
	int element_dimension, i, j, return_code;

	ENTER(Computed_field_clamp_maximum_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields == 1) && 
		(field->number_of_components == field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			element_dimension=get_FE_element_dimension(element);
			/* 2. Calculate the field */
			for (i=0;i<field->number_of_components;i++)
			{
				if (calculate_derivatives)
				{
					temp=field->derivatives;
					temp2=field->source_fields[0]->derivatives;
					field->derivatives_valid = 1;
				}
				for (i=0;i<field->number_of_components;i++)
				{
					if (field->source_fields[0]->values[i] < field->source_values[i])
					{
						field->values[i]=field->source_fields[0]->values[i];
						if (calculate_derivatives)
						{
							for (j=0;j<element_dimension;j++)
							{
								(*temp)=(*temp2);
								temp++;
								temp2++;
							}
						}
					}
					else
					{
						field->values[i]=field->source_values[i];
						if (calculate_derivatives)
						{
							for (j=0;j<element_dimension;j++)
							{
								(*temp)=0.0;
								temp++;
								temp2++; /* To ensure that the following components match */
							}
						}
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_clamp_maximum_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_clamp_maximum_evaluate_cache_in_element */

#define Computed_field_clamp_maximum_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_clamp_maximum_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

static int Computed_field_clamp_maximum_set_values_at_node(struct Computed_field *field,
	struct FE_node *node,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
==============================================================================*/
{
	FE_value *source_values;
	int i,return_code;

	ENTER(Computed_field_clamp_maximum_set_values_at_node);
	if (field && node && values)
	{
		return_code = 1;
		/* clamps to limits of maximums when setting values too */
		if (ALLOCATE(source_values,FE_value,field->number_of_components))
		{
			for (i=0;i<field->number_of_components;i++)
			{
				if (values[i] > field->source_values[i])
				{
					source_values[i] = field->source_values[i];
				}
				else
				{
					source_values[i] = values[i];
				}
			}
			return_code=Computed_field_set_values_at_node(
				field->source_fields[0],node,source_values);
			DEALLOCATE(source_values);
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_clamp_maximum_set_values_at_node.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_clamp_maximum_set_values_at_node */

static int Computed_field_clamp_maximum_set_values_in_element(struct Computed_field *field,
	struct FE_element *element,int *number_in_xi,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
==============================================================================*/
{
	FE_value max, *source_values;
	int element_dimension, i, j, k, number_of_points, return_code;

	ENTER(Computed_field_clamp_maximum_set_values_in_element);
	if (field && element && number_in_xi && values)
	{
		return_code=1;
		element_dimension=element->shape->dimension;
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
					"Computed_field_clamp_maximum_set_values_in_element.  "
					"number_in_xi must be positive");
				return_code = 0;
			}
		}
		if (return_code)
		{
			/* clamps to limits of maximums when setting values too */
			if (ALLOCATE(source_values,FE_value,
				number_of_points*field->number_of_components))
			{
				i=0;
				for (k=0;k<field->number_of_components;k++)
				{
					max=field->source_values[k];
					for (j=0;j<number_of_points;j++)
					{
						if (values[i] > max)
						{
							source_values[i] = max;
						}
						else
						{
							source_values[i] = values[i];
						}
						i++;
					}
				}
				return_code=Computed_field_set_values_in_element(
					field->source_fields[0],element,number_in_xi,source_values);
				DEALLOCATE(source_values);
			}
			else
			{
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_clamp_maximum_set_values_in_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_clamp_maximum_set_values_in_element */

#define Computed_field_clamp_maximum_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_clamp_maximum_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_clamp_maximum(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;

	ENTER(List_Computed_field_clamp_maximum);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    field : %s\n",
			field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,"    maximums :");
		for (i=0;i<field->source_fields[0]->number_of_components;i++)
		{
			display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
		}
		display_message(INFORMATION_MESSAGE,"\n");
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_clamp_maximum.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_clamp_maximum */

static int list_Computed_field_clamp_maximum_commands(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;

	ENTER(list_Computed_field_clamp_maximum_commands);
	if (field)
	{
		display_message(INFORMATION_MESSAGE," field %s maximums",
			field->source_fields[0]->name);
		for (i=0;i<field->source_fields[0]->number_of_components;i++)
		{
			display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_clamp_maximum_commands.  "
			"Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_clamp_maximum_commands */

int Computed_field_set_type_clamp_maximum(struct Computed_field *field,
	struct Computed_field *source_field, FE_value *maximums)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_CLAMP_MAXIMUM with the supplied
<source_field> and <maximums>.  Each component is clamped by its respective limit
in <maximums>.
The <maximums> array must therefore contain as many FE_values as there are
components in <source_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int i,number_of_source_fields,number_of_source_values,return_code;
	FE_value *source_values;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_clamp_maximum);
	if (field&&source_field&&maximums&&
		(source_field->number_of_components>0))
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_values=source_field->number_of_components;
		number_of_source_fields=1;
		if (ALLOCATE(source_values,FE_value,number_of_source_values)&&
			ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_NEW_TYPES;
			field->type_string = computed_field_clamp_maximum_type_string;
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			for (i=0;i<number_of_source_values;i++)
			{
				source_values[i]=maximums[i];
			}
			field->source_values=source_values;
			field->number_of_source_values=number_of_source_values;
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			field->computed_field_clear_type_specific_function =
				Computed_field_clamp_maximum_clear_type_specific;
			field->computed_field_copy_type_specific_function =
				Computed_field_clamp_maximum_copy_type_specific;
			field->computed_field_clear_cache_type_specific_function =
				Computed_field_clamp_maximum_clear_cache_type_specific;
			field->computed_field_type_specific_contents_match_function =
				Computed_field_clamp_maximum_type_specific_contents_match;
			field->computed_field_is_defined_in_element_function =
				Computed_field_clamp_maximum_is_defined_in_element;
			field->computed_field_is_defined_at_node_function =
				Computed_field_clamp_maximum_is_defined_at_node;
			field->computed_field_has_numerical_components_function =
				Computed_field_clamp_maximum_has_numerical_components;
			field->computed_field_can_be_destroyed_function =
				Computed_field_clamp_maximum_can_be_destroyed;
			field->computed_field_evaluate_cache_at_node_function =
				Computed_field_clamp_maximum_evaluate_cache_at_node;
			field->computed_field_evaluate_cache_in_element_function =
				Computed_field_clamp_maximum_evaluate_cache_in_element;
			field->computed_field_evaluate_as_string_at_node_function =
				Computed_field_clamp_maximum_evaluate_as_string_at_node;
			field->computed_field_evaluate_as_string_in_element_function =
				Computed_field_clamp_maximum_evaluate_as_string_in_element;
			field->computed_field_set_values_at_node_function =
				Computed_field_clamp_maximum_set_values_at_node;
			field->computed_field_set_values_in_element_function =
				Computed_field_clamp_maximum_set_values_in_element;
			field->computed_field_get_native_discretization_in_element_function =
				Computed_field_clamp_maximum_get_native_discretization_in_element;
			field->computed_field_find_element_xi_function =
				Computed_field_clamp_maximum_find_element_xi;
			field->list_Computed_field_function = 
				list_Computed_field_clamp_maximum;
			field->list_Computed_field_commands_function = 
				list_Computed_field_clamp_maximum_commands;
			field->computed_field_has_multiple_times_function = 
				Computed_field_default_has_multiple_times;
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
			"Computed_field_set_type_clamp_maximum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_clamp_maximum */

int Computed_field_get_type_clamp_maximum(struct Computed_field *field,
	struct Computed_field **source_field, FE_value **maximums)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_CLAMP_MAXIMUM, the 
<source_field_one> and <source_field_two> used by it are returned.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_get_type_clamp_maximum);
	if (field&&(COMPUTED_FIELD_NEW_TYPES==field->type)&&
		(field->type_string==computed_field_clamp_maximum_type_string)
		&&source_field&&maximums)
	{
		if (ALLOCATE(*maximums,FE_value,field->number_of_components))
		{
			*source_field=field->source_fields[0];
			for (i=0;i<field->number_of_components;i++)
			{
				(*maximums)[i]=field->source_values[i];
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_clamp_maximum.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_clamp_maximum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_clamp_maximum */

static int define_Computed_field_type_clamp_maximum(struct Parse_state *state,
	void *field_void,void *computed_field_component_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 16 August 2000

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CLAMP_MAXIMUM (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	FE_value *maximums, *temp_maximums;
	int i, number_of_maximums,return_code;
	struct Computed_field *field,*source_field;
	struct Computed_field_component_operations_package 
		*computed_field_component_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_clamp_maximum);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_component_operations_package=
		(struct Computed_field_component_operations_package *)
		computed_field_component_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		set_source_field_data.computed_field_manager=
			computed_field_component_operations_package->computed_field_manager;
		set_source_field_data.conditional_function=Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data=(void *)NULL;
		source_field = (struct Computed_field *)NULL;
		maximums=(FE_value *)NULL;
		if (computed_field_clamp_maximum_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code=Computed_field_get_type_clamp_maximum(field,
				&source_field,&maximums);
		}
		else
		{
			/* get first available field, and set maximums for it to 0.0 */
			if (source_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_has_numerical_components,(void *)NULL,
				computed_field_component_operations_package->computed_field_manager))
			{
				number_of_maximums=source_field->number_of_components;
			}
			else
			{
				number_of_maximums=1;
			}
			if (ALLOCATE(maximums,FE_value,number_of_maximums))
			{
				for (i=0;i<number_of_maximums;i++)
				{
					maximums[i]=0.0;
				}
			}
			else
			{
				return_code = 0;
			}
		}
		if (return_code)
		{
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			if ((current_token=state->current_token) &&
				(!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
				number_of_maximums=source_field->number_of_components;
				option_table = CREATE(Option_table)();					
				Option_table_add_entry(option_table,"field",&source_field,
					&set_source_field_data,set_Computed_field_conditional);
				Option_table_add_entry(option_table,"maximums",maximums,
					&number_of_maximums,set_FE_value_array);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			/* parse the field ... */
			if (return_code&&(current_token=state->current_token))
			{
				/* ... only if the "field" token is next */
				if (fuzzy_string_compare(current_token,"field"))
				{
					number_of_maximums=source_field->number_of_components;
					option_table = CREATE(Option_table)();
					/* fields */
					set_source_field_data.computed_field_manager=
						computed_field_component_operations_package->computed_field_manager;
					set_source_field_data.conditional_function=Computed_field_has_numerical_components;
					set_source_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"field",&source_field,
						&set_source_field_data,set_Computed_field_conditional);
					if (return_code=Option_table_parse(option_table,state))
					{
						if (source_field)
						{
							if (REALLOCATE(temp_maximums,maximums,FE_value,
								source_field->number_of_components))
							{
								maximums=temp_maximums;
								/* make any new maximums equal to 1.0 */
								for (i=number_of_maximums;
									i<source_field->number_of_components;i++)
								{
									maximums[i]=1.0;
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
								"define_Computed_field_type_clamp_maximum.  Invalid field");
							return_code = 0;
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			/* parse the maximums */
			if (return_code&&state->current_token)
			{
				option_table = CREATE(Option_table)();
				number_of_maximums=source_field->number_of_components;
				Option_table_add_entry(option_table,"maximums",maximums,
					&number_of_maximums,set_FE_value_array);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_clamp_maximum(field,
					source_field, maximums);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_clamp_maximum.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			DESTROY(Option_table)(&option_table);
		}
		DEALLOCATE(maximums);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_clamp_maximum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_clamp_maximum */

static char computed_field_clamp_minimum_type_string[] = "clamp_minimum";

int Computed_field_is_type_clamp_minimum(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_clamp_minimum);
	if (field)
	{
		return_code = (field->type_string == computed_field_clamp_minimum_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_clamp_minimum.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_clamp_minimum */

static int Computed_field_clamp_minimum_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_clamp_minimum_clear_type_specific);
	if (field)
	{
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_clamp_minimum_clear_type_specific.  "
			"Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_clamp_minimum_clear_type_specific */

static void *Computed_field_clamp_minimum_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	void *destination

	ENTER(Computed_field_clamp_minimum_copy_type_specific);
	if (field)
	{
		destination = (void *)1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_clamp_minimum_copy_type_specific.  "
			"Invalid field");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_clamp_minimum_copy_type_specific */

#define Computed_field_clamp_minimum_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_clamp_minimum_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_component_operations_type_specific_contents_match);
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
} /* Computed_field_component_operations_type_specific_contents_match */

#define Computed_field_clamp_minimum_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_clamp_minimum_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_clamp_minimum_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_clamp_minimum_can_be_destroyed \
	(Computed_field_can_be_destroyed_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
No special criteria on the destroy
==============================================================================*/

static int Computed_field_clamp_minimum_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_clamp_minimum_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 1) && 
		(field->number_of_components == field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			for (i=0;i<field->number_of_components;i++)
			{
				if (field->source_fields[0]->values[i] > field->source_values[i])
				{
					field->values[i]=field->source_fields[0]->values[i];
				}
				else
				{
					field->values[i]=field->source_values[i];
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_clamp_minimum_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_clamp_minimum_evaluate_cache_at_node */

static int Computed_field_clamp_minimum_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value *temp, *temp2;
	int element_dimension, i, j, return_code;

	ENTER(Computed_field_clamp_minimum_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields == 1) && 
		(field->number_of_components == field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			element_dimension=get_FE_element_dimension(element);
			/* 2. Calculate the field */
			for (i=0;i<field->number_of_components;i++)
			{
				if (calculate_derivatives)
				{
					temp=field->derivatives;
					temp2=field->source_fields[0]->derivatives;
					field->derivatives_valid = 1;
				}
				for (i=0;i<field->number_of_components;i++)
				{
					if (field->source_fields[0]->values[i] > field->source_values[i])
					{
						field->values[i]=field->source_fields[0]->values[i];
						if (calculate_derivatives)
						{
							for (j=0;j<element_dimension;j++)
							{
								(*temp)=(*temp2);
								temp++;
								temp2++;
							}
						}
					}
					else
					{
						field->values[i]=field->source_values[i];
						if (calculate_derivatives)
						{
							for (j=0;j<element_dimension;j++)
							{
								(*temp)=0.0;
								temp++;
								temp2++; /* To ensure that the following components match */
							}
						}
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_clamp_minimum_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_clamp_minimum_evaluate_cache_in_element */

#define Computed_field_clamp_minimum_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_clamp_minimum_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

static int Computed_field_clamp_minimum_set_values_at_node(struct Computed_field *field,
	struct FE_node *node,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
==============================================================================*/
{
	FE_value *source_values;
	int i,return_code;

	ENTER(Computed_field_clamp_minimum_set_values_at_node);
	if (field && node && values)
	{
		return_code = 1;
		/* clamps to limits of minimums when setting values too */
		if (ALLOCATE(source_values,FE_value,field->number_of_components))
		{
			for (i=0;i<field->number_of_components;i++)
			{
				if (values[i] < field->source_values[i])
				{
					source_values[i] = field->source_values[i];
				}
				else
				{
					source_values[i] = values[i];
				}
			}
			return_code=Computed_field_set_values_at_node(
				field->source_fields[0],node,source_values);
			DEALLOCATE(source_values);
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_clamp_minimum_set_values_at_node.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_clamp_minimum_set_values_at_node */

static int Computed_field_clamp_minimum_set_values_in_element(struct Computed_field *field,
	struct FE_element *element,int *number_in_xi,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
==============================================================================*/
{
	FE_value min, *source_values;
	int element_dimension, i, j, k, number_of_points, return_code;

	ENTER(Computed_field_clamp_minimum_set_values_in_element);
	if (field && element && number_in_xi && values)
	{
		return_code=1;
		element_dimension=element->shape->dimension;
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
					"Computed_field_clamp_minimum_set_values_in_element.  "
					"number_in_xi must be positive");
				return_code = 0;
			}
		}
		if (return_code)
		{
			/* clamps to limits of minimums when setting values too */
			if (ALLOCATE(source_values,FE_value,
				number_of_points*field->number_of_components))
			{
				i=0;
				for (k=0;k<field->number_of_components;k++)
				{
					min=field->source_values[k];
					for (j=0;j<number_of_points;j++)
					{
						if (values[i] < min)
						{
							source_values[i] = min;
						}
						else
						{
							source_values[i] = values[i];
						}
						i++;
					}
				}
				return_code=Computed_field_set_values_in_element(
					field->source_fields[0],element,number_in_xi,source_values);
				DEALLOCATE(source_values);
			}
			else
			{
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_clamp_minimum_set_values_in_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_clamp_minimum_set_values_in_element */

#define Computed_field_clamp_minimum_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_clamp_minimum_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_clamp_minimum(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;

	ENTER(List_Computed_field_clamp_minimum);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    field : %s\n",
			field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,"    minimums :");
		for (i=0;i<field->source_fields[0]->number_of_components;i++)
		{
			display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
		}
		display_message(INFORMATION_MESSAGE,"\n");
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_clamp_minimum.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_clamp_minimum */

static int list_Computed_field_clamp_minimum_commands(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;

	ENTER(list_Computed_field_clamp_minimum_commands);
	if (field)
	{
		display_message(INFORMATION_MESSAGE," field %s minimums",
			field->source_fields[0]->name);
		for (i=0;i<field->source_fields[0]->number_of_components;i++)
		{
			display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_clamp_minimum_commands.  "
			"Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_clamp_minimum_commands */

int Computed_field_set_type_clamp_minimum(struct Computed_field *field,
	struct Computed_field *source_field, FE_value *minimums)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_CLAMP_MINIMUM with the supplied
<source_field> and <minimums>.  Each component is clamped by its respective limit
in <minimums>.
The <minimums> array must therefore contain as many FE_values as there are
components in <source_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int i,number_of_source_fields,number_of_source_values,return_code;
	FE_value *source_values;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_clamp_minimum);
	if (field&&source_field&&minimums&&
		(source_field->number_of_components>0))
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_values=source_field->number_of_components;
		number_of_source_fields=1;
		if (ALLOCATE(source_values,FE_value,number_of_source_values)&&
			ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_NEW_TYPES;
			field->type_string = computed_field_clamp_minimum_type_string;
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			for (i=0;i<number_of_source_values;i++)
			{
				source_values[i]=minimums[i];
			}
			field->source_values=source_values;
			field->number_of_source_values=number_of_source_values;
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			field->computed_field_clear_type_specific_function =
				Computed_field_clamp_minimum_clear_type_specific;
			field->computed_field_copy_type_specific_function =
				Computed_field_clamp_minimum_copy_type_specific;
			field->computed_field_clear_cache_type_specific_function =
				Computed_field_clamp_minimum_clear_cache_type_specific;
			field->computed_field_type_specific_contents_match_function =
				Computed_field_clamp_minimum_type_specific_contents_match;
			field->computed_field_is_defined_in_element_function =
				Computed_field_clamp_minimum_is_defined_in_element;
			field->computed_field_is_defined_at_node_function =
				Computed_field_clamp_minimum_is_defined_at_node;
			field->computed_field_has_numerical_components_function =
				Computed_field_clamp_minimum_has_numerical_components;
			field->computed_field_can_be_destroyed_function =
				Computed_field_clamp_minimum_can_be_destroyed;
			field->computed_field_evaluate_cache_at_node_function =
				Computed_field_clamp_minimum_evaluate_cache_at_node;
			field->computed_field_evaluate_cache_in_element_function =
				Computed_field_clamp_minimum_evaluate_cache_in_element;
			field->computed_field_evaluate_as_string_at_node_function =
				Computed_field_clamp_minimum_evaluate_as_string_at_node;
			field->computed_field_evaluate_as_string_in_element_function =
				Computed_field_clamp_minimum_evaluate_as_string_in_element;
			field->computed_field_set_values_at_node_function =
				Computed_field_clamp_minimum_set_values_at_node;
			field->computed_field_set_values_in_element_function =
				Computed_field_clamp_minimum_set_values_in_element;
			field->computed_field_get_native_discretization_in_element_function =
				Computed_field_clamp_minimum_get_native_discretization_in_element;
			field->computed_field_find_element_xi_function =
				Computed_field_clamp_minimum_find_element_xi;
			field->list_Computed_field_function = 
				list_Computed_field_clamp_minimum;
			field->list_Computed_field_commands_function = 
				list_Computed_field_clamp_minimum_commands;
			field->computed_field_has_multiple_times_function = 
				Computed_field_default_has_multiple_times;
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
			"Computed_field_set_type_clamp_minimum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_clamp_minimum */

int Computed_field_get_type_clamp_minimum(struct Computed_field *field,
	struct Computed_field **source_field, FE_value **minimums)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_CLAMP_MINIMUM, the 
<source_field_one> and <source_field_two> used by it are returned.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_get_type_clamp_minimum);
	if (field&&(COMPUTED_FIELD_NEW_TYPES==field->type)&&
		(field->type_string==computed_field_clamp_minimum_type_string)
		&&source_field&&minimums)
	{
		if (ALLOCATE(*minimums,FE_value,field->number_of_components))
		{
			*source_field=field->source_fields[0];
			for (i=0;i<field->number_of_components;i++)
			{
				(*minimums)[i]=field->source_values[i];
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_clamp_minimum.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_clamp_minimum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_clamp_minimum */

static int define_Computed_field_type_clamp_minimum(struct Parse_state *state,
	void *field_void,void *computed_field_component_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 16 August 2000

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CLAMP_MINIMUM (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	FE_value *minimums, *temp_minimums;
	int i, number_of_minimums,return_code;
	struct Computed_field *field,*source_field;
	struct Computed_field_component_operations_package 
		*computed_field_component_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_clamp_minimum);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_component_operations_package=
		(struct Computed_field_component_operations_package *)
		computed_field_component_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		set_source_field_data.computed_field_manager=
			computed_field_component_operations_package->computed_field_manager;
		set_source_field_data.conditional_function=Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data=(void *)NULL;
		source_field = (struct Computed_field *)NULL;
		minimums=(FE_value *)NULL;
		if (computed_field_clamp_minimum_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code=Computed_field_get_type_clamp_minimum(field,
				&source_field,&minimums);
		}
		else
		{
			/* get first available field, and set minimums for it to 0.0 */
			if (source_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_has_numerical_components,(void *)NULL,
				computed_field_component_operations_package->computed_field_manager))
			{
				number_of_minimums=source_field->number_of_components;
			}
			else
			{
				number_of_minimums=1;
			}
			if (ALLOCATE(minimums,FE_value,number_of_minimums))
			{
				for (i=0;i<number_of_minimums;i++)
				{
					minimums[i]=0.0;
				}
			}
			else
			{
				return_code = 0;
			}
		}
		if (return_code)
		{
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			if ((current_token=state->current_token) &&
				(!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
				number_of_minimums=source_field->number_of_components;
				option_table = CREATE(Option_table)();					
				Option_table_add_entry(option_table,"field",&source_field,
					&set_source_field_data,set_Computed_field_conditional);
				Option_table_add_entry(option_table,"minimums",minimums,
					&number_of_minimums,set_FE_value_array);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			/* parse the field ... */
			if (return_code&&(current_token=state->current_token))
			{
				/* ... only if the "field" token is next */
				if (fuzzy_string_compare(current_token,"field"))
				{
					number_of_minimums=source_field->number_of_components;
					option_table = CREATE(Option_table)();
					/* fields */
					set_source_field_data.computed_field_manager=
						computed_field_component_operations_package->computed_field_manager;
					set_source_field_data.conditional_function=Computed_field_has_numerical_components;
					set_source_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"field",&source_field,
						&set_source_field_data,set_Computed_field_conditional);
					if (return_code=Option_table_parse(option_table,state))
					{
						if (source_field)
						{
							if (REALLOCATE(temp_minimums,minimums,FE_value,
								source_field->number_of_components))
							{
								minimums=temp_minimums;
								/* make any new minimums equal to 1.0 */
								for (i=number_of_minimums;
									i<source_field->number_of_components;i++)
								{
									minimums[i]=1.0;
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
								"define_Computed_field_type_clamp_minimum.  Invalid field");
							return_code = 0;
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			/* parse the minimums */
			if (return_code&&state->current_token)
			{
				option_table = CREATE(Option_table)();
				number_of_minimums=source_field->number_of_components;
				Option_table_add_entry(option_table,"minimums",minimums,
					&number_of_minimums,set_FE_value_array);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_clamp_minimum(field,
					source_field, minimums);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_clamp_minimum.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			DESTROY(Option_table)(&option_table);
		}
		DEALLOCATE(minimums);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_clamp_minimum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_clamp_minimum */

static char computed_field_offset_type_string[] = "offset";

int Computed_field_is_type_offset(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_offset);
	if (field)
	{
		return_code = (field->type_string == computed_field_offset_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_offset.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_offset */

static int Computed_field_offset_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_offset_clear_type_specific);
	if (field)
	{
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_offset_clear_type_specific.  "
			"Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_offset_clear_type_specific */

static void *Computed_field_offset_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	void *destination

	ENTER(Computed_field_offset_copy_type_specific);
	if (field)
	{
		destination = (void *)1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_offset_copy_type_specific.  "
			"Invalid field");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_offset_copy_type_specific */

#define Computed_field_offset_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_offset_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_component_operations_type_specific_contents_match);
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
} /* Computed_field_component_operations_type_specific_contents_match */

#define Computed_field_offset_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_offset_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_offset_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_offset_can_be_destroyed \
	(Computed_field_can_be_destroyed_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
No special criteria on the destroy
==============================================================================*/

static int Computed_field_offset_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_offset_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 1) && 
		(field->number_of_components == field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			for (i=0;i<field->number_of_components;i++)
			{
				field->values[i]=
					field->source_values[i] + field->source_fields[0]->values[i];
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_offset_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_offset_evaluate_cache_at_node */

static int Computed_field_offset_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value *temp, *temp2;
	int element_dimension, i, j, return_code;

	ENTER(Computed_field_offset_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields == 1) && 
		(field->number_of_components == field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			element_dimension=get_FE_element_dimension(element);
			/* 2. Calculate the field */
			for (i=0;i<field->number_of_components;i++)
			{
				field->values[i]=
					field->source_values[i]+field->source_fields[0]->values[i];
			}
			if (calculate_derivatives)
			{
				temp=field->derivatives;
				temp2=field->source_fields[0]->derivatives;
				for (i=0;i<field->number_of_components;i++)
				{
					for (j=0;j<element_dimension;j++)
					{
						(*temp)=(*temp2);
						temp++;
						temp2++;
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
			"Computed_field_offset_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_offset_evaluate_cache_in_element */

#define Computed_field_offset_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_offset_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

static int Computed_field_offset_set_values_at_node(struct Computed_field *field,
	struct FE_node *node,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
==============================================================================*/
{
	FE_value *source_values;
	int i,return_code;

	ENTER(Computed_field_offset_set_values_at_node);
	if (field && node && values)
	{
		return_code = 1;
		/* reverse the offset */
		if (ALLOCATE(source_values,FE_value,field->number_of_components))
		{
			for (i=0;i<field->number_of_components;i++)
			{
				source_values[i] = values[i] - field->source_values[i];
			}
			return_code=Computed_field_set_values_at_node(
				field->source_fields[0],node,source_values);
			DEALLOCATE(source_values);
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_offset_set_values_at_node.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_offset_set_values_at_node */

static int Computed_field_offset_set_values_in_element(struct Computed_field *field,
	struct FE_element *element,int *number_in_xi,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
==============================================================================*/
{
	FE_value offset_value, *source_values;
	int element_dimension, i, j, k, number_of_points, offset,return_code;

	ENTER(Computed_field_offset_set_values_in_element);
	if (field && element && number_in_xi && values)
	{
		return_code=1;
		element_dimension=element->shape->dimension;
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
					"Computed_field_offset_set_values_in_element.  "
					"number_in_xi must be positive");
				return_code = 0;
			}
		}
		if (return_code)
		{
			/* reverse the offset */
			if (ALLOCATE(source_values,FE_value,
				number_of_points*field->number_of_components))
			{
				for (k=0;k<field->number_of_components;k++)
				{
					offset=k*number_of_points;
					offset_value=field->source_values[k];
					for (j=0;j<number_of_points;j++)
					{
						source_values[offset+j] = values[offset+j] - offset_value;
					}
				}
				return_code=Computed_field_set_values_in_element(
					field->source_fields[0],element,number_in_xi,source_values);
				DEALLOCATE(source_values);
			}
			else
			{
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_offset_set_values_in_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_offset_set_values_in_element */

#define Computed_field_offset_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_offset_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_offset(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;

	ENTER(List_Computed_field_offset);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    field : %s\n",
			field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,"    offsets :");
		for (i=0;i<field->source_fields[0]->number_of_components;i++)
		{
			display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
		}
		display_message(INFORMATION_MESSAGE,"\n");
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_offset.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_offset */

static int list_Computed_field_offset_commands(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;

	ENTER(list_Computed_field_offset_commands);
	if (field)
	{
		display_message(INFORMATION_MESSAGE," field %s offsets",
			field->source_fields[0]->name);
		for (i=0;i<field->source_fields[0]->number_of_components;i++)
		{
			display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_offset_commands.  "
			"Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_offset_commands */

int Computed_field_set_type_offset(struct Computed_field *field,
	struct Computed_field *source_field, FE_value *offsets)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_OFFSET which returns the values of the
<source_field> plus the <offsets>.
The <offsets> array must therefore contain as many FE_values as there are
components in <source_field>; this is the number of components in the field.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int i,number_of_source_fields,number_of_source_values,return_code;
	FE_value *source_values;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_offset);
	if (field&&source_field&&offsets&&
		(source_field->number_of_components>0))
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_values=source_field->number_of_components;
		number_of_source_fields=1;
		if (ALLOCATE(source_values,FE_value,number_of_source_values)&&
			ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_NEW_TYPES;
			field->type_string = computed_field_offset_type_string;
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			for (i=0;i<number_of_source_values;i++)
			{
				source_values[i]=offsets[i];
			}
			field->source_values=source_values;
			field->number_of_source_values=number_of_source_values;
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			field->computed_field_clear_type_specific_function =
				Computed_field_offset_clear_type_specific;
			field->computed_field_copy_type_specific_function =
				Computed_field_offset_copy_type_specific;
			field->computed_field_clear_cache_type_specific_function =
				Computed_field_offset_clear_cache_type_specific;
			field->computed_field_type_specific_contents_match_function =
				Computed_field_offset_type_specific_contents_match;
			field->computed_field_is_defined_in_element_function =
				Computed_field_offset_is_defined_in_element;
			field->computed_field_is_defined_at_node_function =
				Computed_field_offset_is_defined_at_node;
			field->computed_field_has_numerical_components_function =
				Computed_field_offset_has_numerical_components;
			field->computed_field_can_be_destroyed_function =
				Computed_field_offset_can_be_destroyed;
			field->computed_field_evaluate_cache_at_node_function =
				Computed_field_offset_evaluate_cache_at_node;
			field->computed_field_evaluate_cache_in_element_function =
				Computed_field_offset_evaluate_cache_in_element;
			field->computed_field_evaluate_as_string_at_node_function =
				Computed_field_offset_evaluate_as_string_at_node;
			field->computed_field_evaluate_as_string_in_element_function =
				Computed_field_offset_evaluate_as_string_in_element;
			field->computed_field_set_values_at_node_function =
				Computed_field_offset_set_values_at_node;
			field->computed_field_set_values_in_element_function =
				Computed_field_offset_set_values_in_element;
			field->computed_field_get_native_discretization_in_element_function =
				Computed_field_offset_get_native_discretization_in_element;
			field->computed_field_find_element_xi_function =
				Computed_field_offset_find_element_xi;
			field->list_Computed_field_function = 
				list_Computed_field_offset;
			field->list_Computed_field_commands_function = 
				list_Computed_field_offset_commands;
			field->computed_field_has_multiple_times_function = 
				Computed_field_default_has_multiple_times;
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
			"Computed_field_set_type_offset.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_offset */

int Computed_field_get_type_offset(struct Computed_field *field,
	struct Computed_field **source_field, FE_value **offsets)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
If the field type_string matches computed_field_offset_type_string, 
the source_field and offsets used by it are returned. Since the number of 
offsets is equal to the number of components in the source_field (and you don't 
know this yet), this function returns in *offsets a pointer to an allocated array
containing the FE_values.
It is up to the calling function to DEALLOCATE the returned <*offsets>.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_get_type_offset);
	if (field&&(COMPUTED_FIELD_NEW_TYPES==field->type)&&
		(field->type_string==computed_field_offset_type_string)
		&&source_field&&offsets)
	{
		if (ALLOCATE(*offsets,FE_value,field->number_of_components))
		{
			*source_field=field->source_fields[0];
			for (i=0;i<field->number_of_components;i++)
			{
				(*offsets)[i]=field->source_values[i];
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_offset.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_offset.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_offset */

static int define_Computed_field_type_offset(struct Parse_state *state,
	void *field_void,void *computed_field_component_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_OFFSET (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	FE_value *offsets, *temp_offsets;
	int i, number_of_offsets, previous_number_of_offsets, return_code;
	struct Computed_field *field,*source_field;
	struct Computed_field_component_operations_package 
		*computed_field_component_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_offset);
	if (state && (field = (struct Computed_field *)field_void) &&
		(computed_field_component_operations_package =
			(struct Computed_field_component_operations_package *)
			computed_field_component_operations_package_void))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		set_source_field_data.computed_field_manager =
			computed_field_component_operations_package->computed_field_manager;
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		source_field = (struct Computed_field *)NULL;
		offsets = (FE_value *)NULL;
		previous_number_of_offsets = 0;
		if (computed_field_offset_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code =
				Computed_field_get_type_offset(field, &source_field, &offsets);
		}
		if (return_code)
		{
			if (source_field)
			{
				previous_number_of_offsets = source_field->number_of_components;
				ACCESS(Computed_field)(source_field);
			}
			if ((current_token=state->current_token) &&
				(!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
				option_table = CREATE(Option_table)();					
				/* field */
				Option_table_add_entry(option_table, "field", &source_field,
					&set_source_field_data, set_Computed_field_conditional);
				/* offsets */
				Option_table_add_entry(option_table, "offsets", offsets,
					&previous_number_of_offsets, set_FE_value_array);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			/* parse the field ... */
			if (return_code && (current_token = state->current_token))
			{
				/* ... only if the "field" token is next */
				if (fuzzy_string_compare(current_token, "field"))
				{
					option_table = CREATE(Option_table)();
					/* field */
					Option_table_add_entry(option_table, "field", &source_field,
						&set_source_field_data, set_Computed_field_conditional);
					if (return_code = Option_table_parse(option_table,state))
					{
						if (source_field)
						{
							number_of_offsets = source_field->number_of_components;
							if (REALLOCATE(temp_offsets, offsets, FE_value,
								number_of_offsets))
							{
								offsets = temp_offsets;
								/* set new offsets to 0.0 */
								for (i = previous_number_of_offsets; i < number_of_offsets; i++)
								{
									offsets[i] = 0.0;
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
								"define_Computed_field_type_offset.  Invalid field");
							return_code = 0;
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			/* parse the offsets */
			if (return_code && state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* offsets */
				number_of_offsets=source_field->number_of_components;
				Option_table_add_entry(option_table,"offsets",offsets,
					&number_of_offsets,set_FE_value_array);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_offset(field,
					source_field, offsets);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_offset.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			DESTROY(Option_table)(&option_table);
		}
		DEALLOCATE(offsets);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_offset.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_offset */

static char computed_field_sum_components_type_string[] = "sum_components";

int Computed_field_is_type_sum_components(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_sum_components);
	if (field)
	{
		return_code =
			(field->type_string == computed_field_sum_components_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_sum_components.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_sum_components */

static int Computed_field_sum_components_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_sum_components_clear_type_specific);
	if (field)
	{
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_sum_components_clear_type_specific.  "
			"Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_sum_components_clear_type_specific */

static void *Computed_field_sum_components_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	void *destination

	ENTER(Computed_field_sum_components_copy_type_specific);
	if (field)
	{
		destination = (void *)1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_sum_components_copy_type_specific.  "
			"Invalid field");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_sum_components_copy_type_specific */

#define Computed_field_sum_components_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_sum_components_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_component_operations_type_specific_contents_match);
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
} /* Computed_field_component_operations_type_specific_contents_match */

#define Computed_field_sum_components_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_sum_components_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_sum_components_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_sum_components_can_be_destroyed \
	(Computed_field_can_be_destroyed_function)NULL
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
No special criteria on the destroy
==============================================================================*/

static int Computed_field_sum_components_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value sum, *temp;
	int i, return_code;

	ENTER(Computed_field_sum_components_evaluate_cache_at_node);
	if (field && node && (1 == field->number_of_components) &&
		(field->number_of_source_fields == 1) && 
		(field->number_of_source_values ==
			field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			/* weighted sum of components of source field */
			temp = field->source_fields[0]->values;
			sum = 0.0;
			for (i = 0; i < field->number_of_source_values; i++)
			{
				sum += (*temp)*field->source_values[i];
				temp++;
			}
			field->values[0] = sum;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_sum_components_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_sum_components_evaluate_cache_at_node */

static int Computed_field_sum_components_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value sum, *temp;
	int element_dimension, i, j, return_code;

	ENTER(Computed_field_sum_components_evaluate_cache_in_element);
	if (field && element && xi && (1 == field->number_of_components) &&
		(field->number_of_source_fields == 1) && 
		(field->number_of_source_values ==
			field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			element_dimension = get_FE_element_dimension(element);
			/* 2. Calculate the field */
			/* weighted sum of components of source field */
			temp = field->source_fields[0]->values;
			sum = 0.0;
			for (i = 0; i < field->number_of_source_values; i++)
			{
				sum += (*temp)*field->source_values[i];
				temp++;
			}
			field->values[0] = sum;
			if (calculate_derivatives)
			{
				for (j = 0; j < element_dimension; j++)
				{
					temp = field->source_fields[0]->derivatives + j;
					sum = 0.0;
					for (i = 0; i < field->number_of_source_values; i++)
					{
						sum += (*temp)*field->source_values[i];
						temp += element_dimension;
					}
					field->derivatives[j] = sum;
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
			"Computed_field_sum_components_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_sum_components_evaluate_cache_in_element */

#define Computed_field_sum_components_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_sum_components_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_sum_components_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_sum_components_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_sum_components_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_sum_components_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_sum_components(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;

	ENTER(List_Computed_field_sum_components);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    field : %s\n",
			field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,"    weights :");
		for (i = 0; i < field->number_of_source_values; i++)
		{
			display_message(INFORMATION_MESSAGE, " %g", field->source_values[i]);
		}
		display_message(INFORMATION_MESSAGE,"\n");
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_sum_components.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_sum_components */

static int list_Computed_field_sum_components_commands(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;

	ENTER(list_Computed_field_sum_components_commands);
	if (field)
	{
		display_message(INFORMATION_MESSAGE," field %s weights",
			field->source_fields[0]->name);
		for (i = 0; i < field->number_of_source_values; i++)
		{
			display_message(INFORMATION_MESSAGE, " %g", field->source_values[i]);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_sum_components_commands.  "
			"Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_sum_components_commands */

int Computed_field_set_type_sum_components(struct Computed_field *field,
	struct Computed_field *source_field, FE_value *weights)
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_SUM_COMPONENTS with the supplied which
returns a scalar weighted sum of the components of <source_field>.
The <weights> array must therefore contain as many FE_values as there are
components in <source_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int i, number_of_source_fields, number_of_source_values, return_code;
	FE_value *source_values;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_sum_components);
	if (field && source_field && (source_field->number_of_components > 0))
	{
		return_code = 1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_values = source_field->number_of_components;
		number_of_source_fields = 1;
		if (ALLOCATE(source_values, FE_value, number_of_source_values) &&
			ALLOCATE(source_fields, struct Computed_field *, number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type = COMPUTED_FIELD_NEW_TYPES;
			field->type_string = computed_field_sum_components_type_string;
			field->number_of_components = 1;
			source_fields[0] = ACCESS(Computed_field)(source_field);
			field->source_fields = source_fields;
			field->number_of_source_fields = number_of_source_fields;			
			for (i = 0; i < number_of_source_values; i++)
			{
				source_values[i] = weights[i];
			}
			field->source_values = source_values;
			field->number_of_source_values = number_of_source_values;
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			field->computed_field_clear_type_specific_function =
				Computed_field_sum_components_clear_type_specific;
			field->computed_field_copy_type_specific_function =
				Computed_field_sum_components_copy_type_specific;
			field->computed_field_clear_cache_type_specific_function =
				Computed_field_sum_components_clear_cache_type_specific;
			field->computed_field_type_specific_contents_match_function =
				Computed_field_sum_components_type_specific_contents_match;
			field->computed_field_is_defined_in_element_function =
				Computed_field_sum_components_is_defined_in_element;
			field->computed_field_is_defined_at_node_function =
				Computed_field_sum_components_is_defined_at_node;
			field->computed_field_has_numerical_components_function =
				Computed_field_sum_components_has_numerical_components;
			field->computed_field_can_be_destroyed_function =
				Computed_field_sum_components_can_be_destroyed;
			field->computed_field_evaluate_cache_at_node_function =
				Computed_field_sum_components_evaluate_cache_at_node;
			field->computed_field_evaluate_cache_in_element_function =
				Computed_field_sum_components_evaluate_cache_in_element;
			field->computed_field_evaluate_as_string_at_node_function =
				Computed_field_sum_components_evaluate_as_string_at_node;
			field->computed_field_evaluate_as_string_in_element_function =
				Computed_field_sum_components_evaluate_as_string_in_element;
			field->computed_field_set_values_at_node_function =
				Computed_field_sum_components_set_values_at_node;
			field->computed_field_set_values_in_element_function =
				Computed_field_sum_components_set_values_in_element;
			field->computed_field_get_native_discretization_in_element_function =
				Computed_field_sum_components_get_native_discretization_in_element;
			field->computed_field_find_element_xi_function =
				Computed_field_sum_components_find_element_xi;
			field->list_Computed_field_function = 
				list_Computed_field_sum_components;
			field->list_Computed_field_commands_function = 
				list_Computed_field_sum_components_commands;
			field->computed_field_has_multiple_times_function = 
				Computed_field_default_has_multiple_times;
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
			"Computed_field_set_type_sum_components.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_sum_components */

int Computed_field_get_type_sum_components(struct Computed_field *field,
	struct Computed_field **source_field, FE_value **weights)
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
If the field is of type COMPUTED_FIELD_SUM_COMPONENTS, the 
<source_field> and <weights> used by it are returned.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_get_type_sum_components);
	if (field && (COMPUTED_FIELD_NEW_TYPES == field->type) &&
		(field->type_string == computed_field_sum_components_type_string))
	{
		if (ALLOCATE(*weights, FE_value,
			field->source_fields[0]->number_of_components))
		{
			*source_field = field->source_fields[0];
			for (i = 0; i < field->source_fields[0]->number_of_components; i++)
			{
				(*weights)[i] = field->source_values[i];
			}
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_sum_components.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_sum_components.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_sum_components */

static int define_Computed_field_type_sum_components(struct Parse_state *state,
	void *field_void,void *computed_field_component_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_SUM_COMPONENTS (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	FE_value *weights, *temp_weights;
	int i, number_of_weights, previous_number_of_weights, return_code;
	struct Computed_field *field,*source_field;
	struct Computed_field_component_operations_package 
		*computed_field_component_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_sum_components);
	if (state && (field = (struct Computed_field *)field_void) &&
		(computed_field_component_operations_package =
			(struct Computed_field_component_operations_package *)
			computed_field_component_operations_package_void))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		set_source_field_data.computed_field_manager =
			computed_field_component_operations_package->computed_field_manager;
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		source_field = (struct Computed_field *)NULL;
		weights = (FE_value *)NULL;
		previous_number_of_weights = 0;
		if (computed_field_sum_components_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code =
				Computed_field_get_type_sum_components(field, &source_field, &weights);
		}
		if (return_code)
		{
			if (source_field)
			{
				previous_number_of_weights = source_field->number_of_components;
				ACCESS(Computed_field)(source_field);
			}
			if ((current_token = state->current_token) &&
				(!(strcmp(PARSER_HELP_STRING, current_token) &&
					strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))))
			{
				option_table = CREATE(Option_table)();					
				Option_table_add_entry(option_table, "field", &source_field,
					&set_source_field_data, set_Computed_field_conditional);
				Option_table_add_entry(option_table, "weights", weights,
					&previous_number_of_weights, set_FE_value_array);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			/* parse the field ... */
			if (return_code && (current_token = state->current_token))
			{
				/* ... only if the "field" token is next */
				if (fuzzy_string_compare(current_token, "field"))
				{
					option_table = CREATE(Option_table)();
					/* field */
					Option_table_add_entry(option_table, "field", &source_field,
						&set_source_field_data, set_Computed_field_conditional);
					if (return_code = Option_table_parse(option_table, state))
					{
						if (source_field)
						{
							number_of_weights = source_field->number_of_components;
							if (REALLOCATE(temp_weights, weights, FE_value,
								number_of_weights))
							{
								weights = temp_weights;
								/* make any new weights equal to 1.0 */
								for (i = previous_number_of_weights; i < number_of_weights; i++)
								{
									weights[i] = 1.0;
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
								"define_Computed_field_type_sum_components.  Invalid field");
							return_code = 0;
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			/* parse the weights */
			if (return_code && state->current_token)
			{
				option_table = CREATE(Option_table)();
				number_of_weights = source_field->number_of_components;
				Option_table_add_entry(option_table, "weights", weights,
					&number_of_weights, set_FE_value_array);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_sum_components(field,
					source_field, weights);
			}
			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token) &&
						strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_sum_components.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			DESTROY(Option_table)(&option_table);
		}
		DEALLOCATE(weights);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_sum_components.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_sum_components */

int Computed_field_register_types_component_operations(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 13 December 2001

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
			computed_field_multiply_components_type_string, 
			define_Computed_field_type_multiply_components,
			&computed_field_component_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_divide_components_type_string, 
			define_Computed_field_type_divide_components,
			&computed_field_component_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_add_type_string,
			define_Computed_field_type_add,
			&computed_field_component_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_scale_type_string,
			define_Computed_field_type_scale,
			&computed_field_component_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_clamp_maximum_type_string, 
			define_Computed_field_type_clamp_maximum,
			&computed_field_component_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_clamp_minimum_type_string, 
			define_Computed_field_type_clamp_minimum,
			&computed_field_component_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_offset_type_string, 
			define_Computed_field_type_offset,
			&computed_field_component_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_sum_components_type_string, 
			define_Computed_field_type_sum_components,
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
} /* Computed_field_register_types_component_operations */

