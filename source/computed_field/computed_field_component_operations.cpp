/*******************************************************************************
FILE : computed_field_component_operations.c

LAST MODIFIED : 13 July 2000

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

struct Computed_field_component_operations_type_specific_data
{
	struct Computed_field_component_operations_package *package;
};

static char computed_field_multiply_components_type_string[] = "multiply_components";

char *Computed_field_multiply_components_type_string(void)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Return the static type string which identifies this type.
==============================================================================*/
{

	ENTER(Computed_field_multiply_components_type_string);
	LEAVE;

	return (computed_field_multiply_components_type_string);
} /* Computed_field_multiply_components_type_string */

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
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_multiply_components_clear_type_specific.  "
			"Invalid arguments.");
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
	struct Computed_field_component_operations_type_specific_data *destination,
		*source;

	ENTER(Computed_field_multiply_components_copy_type_specific);
	if (field && (source = 
		(struct Computed_field_component_operations_type_specific_data *)
		field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_component_operations_type_specific_data, 1))
		{
			destination->package = source->package;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_multiply_components_copy_type_specific.  "
				"Unable to allocate memory.");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_multiply_components_copy_type_specific.  "
			"Invalid arguments.");
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

static int Computed_field_multiply_components_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

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
			Computed_field_evaluate_source_fields_cache_at_node(field, node))
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
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_multiply_components_evaluate_cache_at_node */

static int Computed_field_multiply_components_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

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
				xi, top_level_element, calculate_derivatives))
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
			"Invalid arguments.");
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
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_multiply_components.  Invalid arguments.");
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
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_multiply_components_commands.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_multiply_components_commands */

int Computed_field_set_type_multiply_components(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two,
	struct Computed_field_component_operations_package *package)
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
	struct Computed_field_component_operations_type_specific_data *data;

	ENTER(Computed_field_set_type_multiply_components);
	if (field&&source_field_one&&source_field_two&&package&&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components))
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=2;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields)&&
			ALLOCATE(data,struct Computed_field_component_operations_type_specific_data, 1))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_NEW_TYPES;
			field->type_string = Computed_field_multiply_components_type_string();
			field->number_of_components = source_field_one->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field_one);
			source_fields[1]=ACCESS(Computed_field)(source_field_two);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)data;
			data->package = package;

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
			"Computed_field_set_type_multiply_components.  Invalid argument(s)");
		return_code=0;
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
		(field->type_string==Computed_field_multiply_components_type_string()))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_multiply_components.  Invalid argument(s)");
		return_code=0;
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
					return_code=0;
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
						source_fields[0], source_fields[1],
						computed_field_component_operations_package);
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
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_multiply_components.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_multiply_components.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_multiply_components */

static char computed_field_divide_components_type_string[] = "divide_components";

char *Computed_field_divide_components_type_string(void)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Return the static type string which identifies this type.
==============================================================================*/
{

	ENTER(Computed_field_divide_components_type_string);
	LEAVE;

	return (computed_field_divide_components_type_string);
} /* Computed_field_divide_components_type_string */

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
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_divide_components_clear_type_specific.  "
			"Invalid arguments.");
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
	struct Computed_field_component_operations_type_specific_data *destination,
		*source;

	ENTER(Computed_field_divide_components_copy_type_specific);
	if (field && (source = 
		(struct Computed_field_component_operations_type_specific_data *)
		field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_component_operations_type_specific_data, 1))
		{
			destination->package = source->package;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_divide_components_copy_type_specific.  "
				"Unable to allocate memory.");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_divide_components_copy_type_specific.  "
			"Invalid arguments.");
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

static int Computed_field_divide_components_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node)
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
			Computed_field_evaluate_source_fields_cache_at_node(field, node))
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
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_divide_components_evaluate_cache_at_node */

static int Computed_field_divide_components_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value *derivative, vsquared;
	int i, j, number_of_xi, return_code;

	ENTER(Computed_field_divide_components_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields > 0) && 
		(field->number_of_components == field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, top_level_element, calculate_derivatives))
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
			"Invalid arguments.");
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
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_divide_components.  Invalid arguments.");
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
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_divide_components_commands.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_divide_components_commands */

int Computed_field_set_type_divide_components(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two,
	struct Computed_field_component_operations_package *package)
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
	struct Computed_field_component_operations_type_specific_data *data;

	ENTER(Computed_field_set_type_divide_components);
	if (field&&source_field_one&&source_field_two&&package&&
		(source_field_one->number_of_components ==
			source_field_two->number_of_components))
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=2;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields)&&
			ALLOCATE(data,struct Computed_field_component_operations_type_specific_data, 1))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_NEW_TYPES;
			field->type_string = Computed_field_divide_components_type_string();
			field->number_of_components = source_field_one->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field_one);
			source_fields[1]=ACCESS(Computed_field)(source_field_two);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)data;
			data->package = package;

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
			"Computed_field_set_type_divide_components.  Invalid argument(s)");
		return_code=0;
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
		(field->type_string==Computed_field_divide_components_type_string()))
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_divide_components.  Invalid argument(s)");
		return_code=0;
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
					return_code=0;
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
						source_fields[0], source_fields[1],
						computed_field_component_operations_package);
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
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_divide_components.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_divide_components.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_divide_components */

int Computed_field_register_types_component_operations(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

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
			Computed_field_multiply_components_type_string(), 
			define_Computed_field_type_multiply_components,
			&computed_field_component_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			Computed_field_divide_components_type_string(), 
			define_Computed_field_type_divide_components,
			&computed_field_component_operations_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_component_operations.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_component_operations */

