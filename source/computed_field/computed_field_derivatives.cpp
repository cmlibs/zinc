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
#include "graphics/texture.h"
#include "graphics/scene_viewer.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_derivatives.h"

struct Computed_field_derivatives_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
};

struct Computed_field_derivatives_type_specific_data
{
	int xi_index;
	struct Computed_field_derivatives_package *package;
};

static char computed_field_derivative_type_string[] = "derivative";

char *Computed_field_derivative_type_string(void)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Return the static type string which identifies this type.
==============================================================================*/
{

	ENTER(Computed_field_derivative_type_string);
	LEAVE;

	return (computed_field_derivative_type_string);
} /* Computed_field_derivative_type_string */

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
			destination->package = source->package;
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
			return_code = 1;
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

static int Computed_field_derivative_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_derivative_evaluate_cache_at_node);
	if (field && node)
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_derivative_evaluate_cache_at_node.  "
			"Unable to calculate at a node.");
		return_code = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_derivative_evaluate_cache_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_derivative_evaluate_cache_at_node */

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
				if (field->source_fields[0]->derivatives_valid)
				{
					for (i = 0 ; i < field->number_of_components ; i++)
					{
						field->values[i] = field->source_fields[0]->
							derivatives[i * number_of_xi + data->xi_index];
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_derivative_evaluate_cache_in_element.  "
						"Could not calculate derivatives of field %s.",
						field->source_fields[0]->name);
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
	struct Computed_field *source_field, int xi_index,
	struct Computed_field_derivatives_package *package)
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
			field->type_string = Computed_field_derivative_type_string();
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)data;
			data->xi_index = xi_index;
			data->package = package;

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
		(field->type_string==Computed_field_derivative_type_string())
		&&(data = 
		(struct Computed_field_derivatives_type_specific_data *)
		field->type_specific_data)&&source_field)
	{
		*source_field = field->source_fields[0];
		*xi_index = data->xi_index;
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
					source_field, xi_index - 1,
					computed_field_derivatives_package);
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

int Computed_field_register_types_derivatives(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

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
			Computed_field_derivative_type_string(), 
			define_Computed_field_type_derivative,
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

