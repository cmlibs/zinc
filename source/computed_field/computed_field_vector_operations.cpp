/*******************************************************************************
FILE : computed_field_vector_operations.c

LAST MODIFIED : 21 January 2002

DESCRIPTION :
Implements a number of basic vector operations on computed fields.
==============================================================================*/
#include <math.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/matrix_vector.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_vector_operations.h"

struct Computed_field_vector_operations_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
};

static char computed_field_normalise_type_string[] = "normalise";

int Computed_field_is_type_normalise(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_normalise);
	if (field)
	{
		return_code =
			(field->type_string == computed_field_normalise_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_normalise.  Missing field.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_normalise */

static int Computed_field_normalise_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_normalise_clear_type_specific);
	if (field)
	{
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_normalise_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_normalise_clear_type_specific */

static void *Computed_field_normalise_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	void *destination;

	ENTER(Computed_field_normalise_copy_type_specific);
	if (field)
	{
		/* Return a TRUE value */
		destination = (void *)1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_normalise_copy_type_specific.  "
			"Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_normalise_copy_type_specific */

#define Computed_field_normalise_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_normalise_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_vector_operations_type_specific_contents_match);
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
} /* Computed_field_vector_operations_type_specific_contents_match */

#define Computed_field_normalise_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_normalise_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_normalise_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_normalise_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_normalise_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	double size;
	int i, return_code;

	ENTER(Computed_field_normalise_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields > 0) && 
		(field->number_of_components == field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			size = 0.0;
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				size += field->source_fields[0]->values[i] *
					field->source_fields[0]->values[i];
			}
			size = sqrt(size);
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] = field->source_fields[0]->values[i] / size;
			}			
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_normalise_evaluate_cache_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_normalise_evaluate_cache_at_node */

static int Computed_field_normalise_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	double size;
	FE_value *derivative, *source_derivative;
	int i, number_of_xi, return_code;

	ENTER(Computed_field_normalise_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields > 0) && 
		(field->number_of_components == field->source_fields[0]->number_of_components))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			number_of_xi = get_FE_element_dimension(element);
			size = 0.0;
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				size += field->source_fields[0]->values[i] *
					field->source_fields[0]->values[i];
			}
			size = sqrt(size);
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				field->values[i] = field->source_fields[0]->values[i] / size;
			}
			if (calculate_derivatives && field->source_fields[0]->derivatives_valid)
			{
				derivative = field->derivatives;
				source_derivative = field->source_fields[0]->derivatives;
				for (i = 0 ; i < field->number_of_components * number_of_xi ; i++)
				{
					*derivative = *source_derivative / size;
					derivative++;
					source_derivative++;
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
			"Computed_field_normalise_evaluate_cache_in_element.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_normalise_evaluate_cache_in_element */

#define Computed_field_normalise_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_normalise_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_normalise_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_normalise_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_normalise_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_normalise_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_normalise(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_normalise);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_normalise.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_normalise */

static char *Computed_field_normalise_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_normalise_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_normalise_type_string, &error);
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
			"Computed_field_normalise_get_command_string.  "
			"Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_normalise_get_command_string */

#define Computed_field_normalise_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_normalise(struct Computed_field *field,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_NORMALISE with the supplied
<source_field>.  Sets the number of components equal to the <source_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_normalise);
	if (field&&source_field)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_NEW_TYPES;
			field->type_string = computed_field_normalise_type_string;
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(normalise);
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
			"Computed_field_set_type_normalise.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_normalise */

int Computed_field_get_type_normalise(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_NORMALISE, the 
<source_field> used by it is returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_normalise);
	if (field && (COMPUTED_FIELD_NEW_TYPES == field->type) &&
		(field->type_string == computed_field_normalise_type_string) &&
		source_field)
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_normalise.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_normalise */

static int define_Computed_field_type_normalise(struct Parse_state *state,
	void *field_void,void *computed_field_vector_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 18 December 2001

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_NORMALISE (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,*source_field;
	struct Computed_field_vector_operations_package 
		*computed_field_vector_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_normalise);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_vector_operations_package=
		(struct Computed_field_vector_operations_package *)
		computed_field_vector_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		if (computed_field_normalise_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_normalise(field, &source_field);
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
				computed_field_vector_operations_package->computed_field_manager;
			set_source_field_data.conditional_function=Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"field",&source_field,
				&set_source_field_data,set_Computed_field_conditional);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_normalise(field,
					source_field);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_normalise.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			DESTROY(Option_table)(&option_table);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_normalise.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_normalise */

static char computed_field_cross_product_type_string[] = "cross_product";

int Computed_field_is_type_cross_product(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_cross_product);
	if (field)
	{
		return_code =
			(field->type_string == computed_field_cross_product_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_cross_product.  Missing field.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_cross_product */

static int Computed_field_cross_product_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_cross_product_clear_type_specific);
	if (field)
	{
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cross_product_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_cross_product_clear_type_specific */

static void *Computed_field_cross_product_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	void *destination;

	ENTER(Computed_field_cross_product_copy_type_specific);
	if (field)
	{
		/* Return a TRUE value */
		destination = (void *)1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cross_product_copy_type_specific.  "
			"Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_cross_product_copy_type_specific */

#define Computed_field_cross_product_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_cross_product_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_vector_operations_type_specific_contents_match);
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
} /* Computed_field_vector_operations_type_specific_contents_match */

#define Computed_field_cross_product_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_cross_product_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_cross_product_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_cross_product_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_cross_product_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_cross_product_evaluate_cache_at_node);
	if (field && node && 
		(field->number_of_source_fields == field->number_of_components - 1))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			switch (field->number_of_components)
			{
				case 1:
				{
					field->values = 0;
				} break;
				case 2:
				{
					field->values[0] = -field->source_fields[0]->values[1];
					field->values[1] = field->source_fields[0]->values[0];
				} break;
				case 3:
				{
					cross_product_FE_value_vector3(field->source_fields[0]->values,
						field->source_fields[1]->values, field->values);
				} break;
				case 4:
				{
					cross_product_FE_value_vector4(field->source_fields[0]->values,
						field->source_fields[1]->values, 
						field->source_fields[2]->values, field->values);
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_cross_product_evaluate_cache_at_node.  "
						"Unsupported number of components.");
					return_code = 0;
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cross_product_evaluate_cache_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_cross_product_evaluate_cache_at_node */

static int Computed_field_cross_product_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Evaluate the fields cache in the element.
==============================================================================*/
{
	FE_value *derivative, *source_derivative, *temp_vector;
	int i, j, number_of_xi, return_code;

	ENTER(Computed_field_cross_product_evaluate_cache_in_element);
	if (field && element && xi &&
		(field->number_of_source_fields == field->number_of_components - 1))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			switch (field->number_of_components)
			{
				case 1:
				{
					field->values = 0;
				} break;
				case 2:
				{
					field->values[0] = -field->source_fields[0]->values[1];
					field->values[1] = field->source_fields[0]->values[0];
				} break;
				case 3:
				{
					cross_product_FE_value_vector3(field->source_fields[0]->values,
						field->source_fields[1]->values, field->values);
				} break;
				case 4:
				{
					cross_product_FE_value_vector4(field->source_fields[0]->values,
						field->source_fields[1]->values, 
						field->source_fields[2]->values, field->values);
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_cross_product_evaluate_cache_at_node.  "
						"Unsupported number of components.");
					return_code = 0;
				} break;
			}
			if (calculate_derivatives)
			{
				if (ALLOCATE(temp_vector, FE_value, field->number_of_components *
					field->number_of_components))
				{
					number_of_xi = get_FE_element_dimension(element);
					switch (field->number_of_components)
					{
						case 1:
						{
							derivative = field->derivatives;
							for (i = 0 ; i < number_of_xi ; i++)
							{
								*derivative = 0;
								derivative++;
							}
						} break;
						case 2:
						{
							derivative = field->derivatives;
							source_derivative = field->derivatives + number_of_xi;
							for (i = 0 ; i < number_of_xi ; i++)
							{
								*derivative = -*source_derivative;
								derivative++;
								source_derivative++;
							}
							source_derivative = field->derivatives + number_of_xi;
							for (i = 0 ; i < number_of_xi ; i++)
							{
								*derivative = *source_derivative;
								derivative++;
								source_derivative++;
							}
						} break;
						case 3:
						{
							for (j = 0 ; j < number_of_xi ; j++)
							{
								for (i = 0 ; i < 3 ; i++)
								{
									temp_vector[i] = field->source_fields[0]->
										derivatives[i * number_of_xi + j];
									temp_vector[i + 3] = field->source_fields[1]->
										derivatives[i * number_of_xi + j];
								}
								cross_product_FE_value_vector3(temp_vector,
									field->source_fields[1]->values, temp_vector + 6);
								for (i = 0 ; i < 3 ; i++)
								{
									field->derivatives[i * number_of_xi + j] = 
										temp_vector[i + 6];
								}
								cross_product_FE_value_vector3(
									field->source_fields[0]->values,
									temp_vector + 3, temp_vector + 6);
								for (i = 0 ; i < 3 ; i++)
								{
									field->derivatives[i * number_of_xi + j] += 
										temp_vector[i + 6];
								}
							}
						} break;
						case 4:
						{
							for (j = 0 ; j < number_of_xi ; j++)
							{
								for (i = 0 ; i < 4 ; i++)
								{
									temp_vector[i] = field->source_fields[0]->
										derivatives[i * number_of_xi + j];
									temp_vector[i + 4] = field->source_fields[1]->
										derivatives[i * number_of_xi + j];
									temp_vector[i + 8] = field->source_fields[2]->
										derivatives[i * number_of_xi + j];
								}
								cross_product_FE_value_vector4(temp_vector,
									field->source_fields[1]->values, 
									field->source_fields[2]->values, temp_vector + 12);
								for (i = 0 ; i < 4 ; i++)
								{
									field->derivatives[i * number_of_xi + j] = 
										temp_vector[i + 12];
								}
								cross_product_FE_value_vector4(
									field->source_fields[0]->values, temp_vector + 4,
									field->source_fields[2]->values, temp_vector + 12);
								for (i = 0 ; i < 4 ; i++)
								{
									field->derivatives[i * number_of_xi + j] += 
										temp_vector[i + 12];
								}
								cross_product_FE_value_vector4(
									field->source_fields[0]->values, 
									field->source_fields[1]->values, 
									temp_vector + 8, temp_vector + 12);
								for (i = 0 ; i < 4 ; i++)
								{
									field->derivatives[i * number_of_xi + j] += 
										temp_vector[i + 12];
								}
							}
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_cross_product_evaluate_cache_at_node.  "
								"Unsupported number of components.");
							field->derivatives_valid = 0;
							return_code = 0;
						} break;
					}
					DEALLOCATE(temp_vector);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_cross_product_evaluate_cache_at_node.  "
						"Unable to allocate temporary vector.");
					field->derivatives_valid = 0;
					return_code = 0;
				}
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
			"Computed_field_cross_product_evaluate_cache_in_element.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_cross_product_evaluate_cache_in_element */

#define Computed_field_cross_product_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_cross_product_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_cross_product_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_cross_product_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_cross_product_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_cross_product_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_cross_product(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;

	ENTER(List_Computed_field_cross_product);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    dimension : %d\n",field->number_of_components);
		display_message(INFORMATION_MESSAGE,"    source fields :");
		for (i = 0 ; i < field->number_of_components - 1 ; i++)
		{
			display_message(INFORMATION_MESSAGE," %s",
				field->source_fields[i]->name);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_cross_product.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_cross_product */

static char *Computed_field_cross_product_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error, i;

	ENTER(Computed_field_cross_product_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_cross_product_type_string, &error);
		sprintf(temp_string, " dimension %d", field->number_of_components);
		append_string(&command_string, temp_string, &error);
		append_string(&command_string, " fields", &error);
		for (i = 0 ; i < field->number_of_components - 1 ; i++)
		{
			if (GET_NAME(Computed_field)(field->source_fields[i], &field_name))
			{
				make_valid_token(&field_name);
				append_string(&command_string, " ", &error);
				append_string(&command_string, field_name, &error);
				DEALLOCATE(field_name);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cross_product_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_cross_product_get_command_string */

#define Computed_field_cross_product_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_cross_product(struct Computed_field *field,
	int dimension, struct Computed_field **source_fields)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_CROSS_PRODUCT with the supplied
<dimension> and the corresponding (dimension-1) <source_fields>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int i, number_of_source_fields,return_code;
	struct Computed_field **temp_source_fields;

	ENTER(Computed_field_set_type_cross_product);
	if (field && (2 <= dimension) && (4 >= dimension) && source_fields)
	{
		return_code = 1;
		for (i = 0 ; return_code && (i < dimension - 1) ; i++)
		{
			if (!source_fields[i] || 
				(source_fields[i]->number_of_components != dimension))
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_cross_product.  "
					"The number of components of the %s field does not match the dimension",
					source_fields[i]->name);
				return_code = 0;
			}
		}
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=dimension - 1;
		if (return_code &&
			ALLOCATE(temp_source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_NEW_TYPES;
			field->type_string = computed_field_cross_product_type_string;
			field->number_of_components = dimension;
			for (i = 0 ; i < number_of_source_fields ; i++)
			{
				temp_source_fields[i]=ACCESS(Computed_field)(source_fields[i]);
			}
			field->source_fields=temp_source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(cross_product);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_cross_product.  Unable to allocate memory");
			DEALLOCATE(source_fields);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_cross_product.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_cross_product */

int Computed_field_get_type_cross_product(struct Computed_field *field,
	int *dimension, struct Computed_field ***source_fields)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_CROSS_PRODUCT, the 
<dimension> and <source_fields> used by it are returned.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_get_type_cross_product);
	if (field&&(COMPUTED_FIELD_NEW_TYPES==field->type)&&
		(field->type_string==computed_field_cross_product_type_string)
		&&source_fields)
	{
		*dimension = field->number_of_components;
		if (ALLOCATE(*source_fields,struct Computed_field *,
			field->number_of_source_fields))
		{
			for (i=0;i<field->number_of_source_fields;i++)
			{
				(*source_fields)[i]=field->source_fields[i];
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_cross_product.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_cross_product.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_cross_product */

static int define_Computed_field_type_cross_product(struct Parse_state *state,
	void *field_void,void *computed_field_vector_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 18 December 2001

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CROSS_PRODUCT (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	int dimension, i, number_of_source_fields, return_code,
		temp_number_of_source_fields;
	struct Computed_field *field, **source_fields, **temp_source_fields;
	struct Computed_field_vector_operations_package 
		*computed_field_vector_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_field_array_data;
	struct Set_Computed_field_conditional_data set_field_data;

	ENTER(define_Computed_field_type_cross_product);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_vector_operations_package=
		(struct Computed_field_vector_operations_package *)
		computed_field_vector_operations_package_void))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (computed_field_cross_product_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_cross_product(field, &dimension,
				&source_fields);
			number_of_source_fields = dimension - 1;
		}
		else
		{
			dimension = 3;
			number_of_source_fields = dimension - 1;
			if (ALLOCATE(source_fields, struct Computed_field *,
				number_of_source_fields))
			{
				for (i = 0; i < number_of_source_fields; i++)
				{
					source_fields[i] = (struct Computed_field *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_cross_product.  "
					"Could not allocate source fields array");
				return_code = 0;
			}
		}
		if (return_code)
		{
			/* try to handle help first */
			if (current_token = state->current_token)
			{
				if (!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
				{
					option_table = CREATE(Option_table)();
					Option_table_add_entry(option_table, "dimension", &dimension,
						NULL, set_int_positive);
					set_field_data.conditional_function = Computed_field_has_n_components;
					set_field_data.conditional_function_user_data = (void *)&dimension;
					set_field_data.computed_field_manager =
						computed_field_vector_operations_package->computed_field_manager;
					set_field_array_data.number_of_fields = number_of_source_fields;
					set_field_array_data.conditional_data = &set_field_data;
					Option_table_add_entry(option_table, "fields", source_fields,
						&set_field_array_data, set_Computed_field_array);
					return_code = Option_table_multi_parse(option_table, state);
					DESTROY(Option_table)(&option_table);
				}
				else
				{
					/* ... only if the "dimension" token is next */
					if (fuzzy_string_compare(current_token, "dimension"))
					{
						option_table = CREATE(Option_table)();
						/* dimension */
						Option_table_add_entry(option_table, "dimension", &dimension,
							NULL, set_int_positive);
						return_code = Option_table_parse(option_table, state);
						DESTROY(Option_table)(&option_table);
						if (number_of_source_fields != dimension - 1)
						{
							temp_number_of_source_fields = dimension - 1;
							if (REALLOCATE(temp_source_fields, source_fields,
								struct Computed_field *, temp_number_of_source_fields))
							{
								source_fields = temp_source_fields;
								/* make all the new source fields NULL */
								for (i = number_of_source_fields;
									i < temp_number_of_source_fields; i++)
								{
									source_fields[i] = (struct Computed_field *)NULL;
								}
								number_of_source_fields = temp_number_of_source_fields;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"define_Computed_field_type_cross_product.  "
									"Could not reallocate source fields array");
								return_code = 0;
							}
						}
						if ((dimension < 2) || (dimension > 4))
						{
							display_message(ERROR_MESSAGE,
								"Only dimensions from 2 to 4 are supported");
							return_code = 0;							
						}
					}
					if (return_code)
					{
						/* ACCESS the source fields for set_Computed_field_array */
						for (i = 0; i < number_of_source_fields; i++)
						{
							if (source_fields[i])
							{
								ACCESS(Computed_field)(source_fields[i]);
							}
						}
						option_table = CREATE(Option_table)();
						set_field_data.conditional_function =
							Computed_field_has_n_components;
						set_field_data.conditional_function_user_data = (void *)&dimension;
						set_field_data.computed_field_manager =
							computed_field_vector_operations_package->computed_field_manager;
						set_field_array_data.number_of_fields = number_of_source_fields;
						set_field_array_data.conditional_data = &set_field_data;
						Option_table_add_entry(option_table, "fields", source_fields,
							&set_field_array_data, set_Computed_field_array);
						if (return_code = Option_table_multi_parse(option_table, state))
						{
							return_code = Computed_field_set_type_cross_product(field,
								dimension, source_fields);
						}
						for (i = 0; i < number_of_source_fields; i++)
						{
							if (source_fields[i])
							{
								DEACCESS(Computed_field)(&source_fields[i]);
							}
						}
						DESTROY(Option_table)(&option_table);
					}
					if (!return_code)
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_cross_product.  Failed");
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "Missing command options.");
			}
		}
		if (source_fields)
		{
			DEALLOCATE(source_fields);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_cross_product.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_cross_product */

static char computed_field_dot_product_type_string[] = "dot_product";

int Computed_field_is_type_dot_product(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_dot_product);
	if (field)
	{
		return_code =
			(field->type_string == computed_field_dot_product_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_dot_product.  Missing field.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_dot_product */

static int Computed_field_dot_product_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_dot_product_clear_type_specific);
	if (field)
	{
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_dot_product_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_dot_product_clear_type_specific */

static void *Computed_field_dot_product_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	void *destination;

	ENTER(Computed_field_dot_product_copy_type_specific);
	if (field)
	{
		destination = (void *)1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_dot_product_copy_type_specific.  "
			"Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_dot_product_copy_type_specific */

#define Computed_field_dot_product_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_dot_product_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_vector_operations_type_specific_contents_match);
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
} /* Computed_field_vector_operations_type_specific_contents_match */

#define Computed_field_dot_product_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_dot_product_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_dot_product_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_dot_product_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_dot_product_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value *temp, *temp2;
	int i, return_code;

	ENTER(Computed_field_dot_product_evaluate_cache_at_node);
	if (field && node && (field->number_of_source_fields == 2))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			field->values[0] = 0.0;
			temp=field->source_fields[0]->values;
			temp2=field->source_fields[1]->values;
			for (i=0;i < field->source_fields[0]->number_of_components;i++)
			{
				field->values[0] += *temp * *temp2;
				temp++;
				temp2++;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_dot_product_evaluate_cache_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_dot_product_evaluate_cache_at_node */

static int Computed_field_dot_product_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Evaluate the fields cache in the element.
==============================================================================*/
{
	FE_value *temp, *temp2;
	int element_dimension, i, j, return_code;

	ENTER(Computed_field_dot_product_evaluate_cache_in_element);
	if (field && element && xi && (field->number_of_source_fields == 2))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			element_dimension=get_FE_element_dimension(element);
			field->values[0] = 0.0;
			if (calculate_derivatives)
			{
				for (j=0;j<element_dimension;j++)
				{
					field->derivatives[j]=0.0;
				}
			}
			temp=field->source_fields[0]->values;
			temp2=field->source_fields[1]->values;
			for (i=0;i < field->source_fields[0]->number_of_components;i++)
			{
				field->values[0] += (*temp) * (*temp2);
				temp++;
				temp2++;
			}
			if (calculate_derivatives)
			{
				temp=field->source_fields[0]->values;
				temp2=field->source_fields[1]->derivatives;
				for (i=0;i < field->source_fields[0]->number_of_components;i++)
				{
					for (j=0;j<element_dimension;j++)
					{
						field->derivatives[j] += (*temp)*(*temp2);
						temp2++;
					}
					temp++;
				}
				temp=field->source_fields[1]->values;
				temp2=field->source_fields[0]->derivatives;
				for (i=0;i < field->source_fields[0]->number_of_components;i++)
				{
					for (j=0;j<element_dimension;j++)
					{
						field->derivatives[j] += (*temp)*(*temp2);
						temp2++;
					}
					temp++;
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
			"Computed_field_dot_product_evaluate_cache_in_element.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_dot_product_evaluate_cache_in_element */

#define Computed_field_dot_product_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_dot_product_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_dot_product_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_dot_product_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_dot_product_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_dot_product_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_dot_product(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_dot_product);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    field 1 : %s\n    field 2 : %s\n",
			field->source_fields[0]->name, field->source_fields[1]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_dot_product.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_dot_product */

static char *Computed_field_dot_product_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_dot_product_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_dot_product_type_string, &error);
		append_string(&command_string, " fields ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_dot_product_get_command_string.  "
			"Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_dot_product_get_command_string */

#define Computed_field_dot_product_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_dot_product(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_DOT_PRODUCT with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components to one.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_dot_product);
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
			field->type_string = computed_field_dot_product_type_string;
			field->number_of_components = 1;
			source_fields[0]=ACCESS(Computed_field)(source_field_one);
			source_fields[1]=ACCESS(Computed_field)(source_field_two);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(dot_product);
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
			"Computed_field_set_type_dot_product.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_dot_product */

int Computed_field_get_type_dot_product(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two)
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_DOT_PRODUCT, the 
<source_field_one> and <source_field_two> used by it are returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_dot_product);
	if (field && (COMPUTED_FIELD_NEW_TYPES == field->type) &&
		(field->type_string == computed_field_dot_product_type_string) &&
		source_field_one && source_field_two)
	{
		*source_field_one = field->source_fields[0];
		*source_field_two = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_dot_product.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_dot_product */

static int define_Computed_field_type_dot_product(struct Parse_state *state,
	void *field_void,void *computed_field_vector_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 18 December 2001

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_DOT_PRODUCT (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,**source_fields;
	struct Computed_field_vector_operations_package 
		*computed_field_vector_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_dot_product);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_vector_operations_package=
		(struct Computed_field_vector_operations_package *)
		computed_field_vector_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			if (computed_field_dot_product_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_dot_product(field, 
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
					computed_field_vector_operations_package->computed_field_manager;
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
					return_code = Computed_field_set_type_dot_product(field,
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
							"define_Computed_field_type_dot_product.  Failed");
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
				"define_Computed_field_type_dot_product.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_dot_product.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_dot_product */

static char computed_field_magnitude_type_string[] = "magnitude";

int Computed_field_is_type_magnitude(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 2 October 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_magnitude);
	if (field)
	{
		return_code =
			(field->type_string == computed_field_magnitude_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_magnitude.  Missing field.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_magnitude */

static int Computed_field_magnitude_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 2 October 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_magnitude_clear_type_specific);
	if (field)
	{
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_magnitude_clear_type_specific.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_magnitude_clear_type_specific */

static void *Computed_field_magnitude_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 2 October 2000

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	void *destination;

	ENTER(Computed_field_magnitude_copy_type_specific);
	if (field)
	{
		/* Return a TRUE value */
		destination = (void *)1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_magnitude_copy_type_specific.  Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_magnitude_copy_type_specific */

#define Computed_field_magnitude_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 2 October 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_magnitude_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 2 October 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_vector_operations_type_specific_contents_match);
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
} /* Computed_field_vector_operations_type_specific_contents_match */

#define Computed_field_magnitude_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 2 October 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_magnitude_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 2 October 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_magnitude_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 2 October 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_magnitude_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_magnitude_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 2 October 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value *source_values, sum;
	int i, return_code, source_number_of_components;

	ENTER(Computed_field_magnitude_evaluate_cache_at_node);
	if (field && node && (0 < field->number_of_source_fields))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			/* 2. Calculate the field */
			source_number_of_components =
				field->source_fields[0]->number_of_components;
			source_values = field->source_fields[0]->values;
			sum = 0.0;
			for (i=0;i<source_number_of_components;i++)
			{
				sum += source_values[i]*source_values[i];
			}
			field->values[0] = sqrt(sum);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_magnitude_evaluate_cache_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_magnitude_evaluate_cache_at_node */

static int Computed_field_magnitude_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 2 October 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value *source_derivatives, *source_values, sum;
	int element_dimension, i, j, return_code, source_number_of_components;

	ENTER(Computed_field_magnitude_evaluate_cache_in_element);
	if (field && element && xi && (0 < field->number_of_source_fields))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			source_number_of_components =
				field->source_fields[0]->number_of_components;
			source_values = field->source_fields[0]->values;
			sum = 0.0;
			for (i=0;i<source_number_of_components;i++)
			{
				sum += source_values[i]*source_values[i];
			}
			field->values[0] = sqrt(sum);

			if (calculate_derivatives)
			{
				source_derivatives=field->source_fields[0]->derivatives;
				element_dimension=get_FE_element_dimension(element);
				for (j=0;j<element_dimension;j++)
				{
					sum = 0.0;
					for (i=0;i<source_number_of_components;i++)
					{
						sum += source_values[i]*source_derivatives[i*element_dimension+j];
					}
					field->derivatives[j] = sum / field->values[0];
				}
				field->derivatives_valid = 1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_magnitude_evaluate_cache_in_element.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_magnitude_evaluate_cache_in_element */

#define Computed_field_magnitude_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 2 October 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_magnitude_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 2 October 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

static int Computed_field_magnitude_set_values_at_node(
	struct Computed_field *field,struct FE_node *node,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 2 October 2000

DESCRIPTION :
Sets the <values> of the computed <field> at <node>.
==============================================================================*/
{
	FE_value magnitude,*source_values;
	int i,return_code,source_number_of_components;

	ENTER(Computed_field_magnitude_set_values_at_node);
	if (field && node && values)
	{
		/* need current vector field values "magnify" */
		source_number_of_components=field->source_fields[0]->number_of_components;
		if (ALLOCATE(source_values,FE_value,source_number_of_components))
		{
			if (Computed_field_evaluate_at_node(field->source_fields[0],node,
					 /*time*/0,source_values))
			{
				return_code=1;
				/* if the source field is not a zero vector, set its magnitude to
					 the given value */
				magnitude = 0.0;
				for (i=0;i<source_number_of_components;i++)
				{
					magnitude += source_values[i]*source_values[i];
				}
				if (0.0 < magnitude)
				{
					magnitude = values[0] / sqrt(magnitude);
					for (i=0;i<source_number_of_components;i++)
					{
						source_values[i] *= magnitude;
					}
					return_code=Computed_field_set_values_at_node(
						field->source_fields[0],node,source_values);
				}
				else
				{
					/* not an error; just a warning */
					display_message(WARNING_MESSAGE,
						"Magnitude field %s cannot be inverted for zero vector",
						field->name);
				}
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
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_magnitude_set_values_at_node.  "
				"Could not set magnitude field %s at node",field->name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_magnitude_set_values_at_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_magnitude_set_values_at_node */

static int Computed_field_magnitude_set_values_in_element(
	struct Computed_field *field, struct FE_element *element,int *number_in_xi,
	FE_value *values)
/*******************************************************************************
LAST MODIFIED : 2 October 2000

DESCRIPTION :
Sets the <values> of the computed <field> over the <element>.
==============================================================================*/
{
	FE_value magnitude,*source_values;
	int element_dimension,i,j,k,number_of_points,return_code,
		source_number_of_components,zero_warning;

	ENTER(Computed_field_magnitude_set_values_in_element);
	if (field && element && number_in_xi && values)
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
					"Computed_field_magnitude_set_values_in_element.  "
					"number_in_xi must be positive");
				return_code=0;
			}
		}
		if (return_code)
		{
			/* need current field values to "magnify" */
			if (Computed_field_get_values_in_element(field->source_fields[0],
					 element,number_in_xi,&source_values,/*time*/0))
			{
				source_number_of_components =
					field->source_fields[0]->number_of_components;
				zero_warning=0;
				for (j=0;j<number_of_points;j++)
				{
					/* if the source field is not a zero vector, set its magnitude to
						 the given value */
					magnitude = 0.0;
					for (k=0;k<source_number_of_components;k++)
					{
						magnitude += source_values[k*number_of_points+j]*
							source_values[k*number_of_points+j];
					}
					if (0.0 < magnitude)
					{
						magnitude = values[j] / sqrt(magnitude);
						for (k=0;k<source_number_of_components;k++)
						{
							source_values[k*number_of_points+j] *= magnitude;
						}
					}
					else
					{
						zero_warning=1;
					}
				}
				if (zero_warning)
				{
					/* not an error; just a warning */
					display_message(WARNING_MESSAGE,
						"Magnitude field %s cannot be inverted for zero vectors",
						field->name);
				}
				return_code=Computed_field_set_values_in_element(
					field->source_fields[0],element,number_in_xi,source_values);
				DEALLOCATE(source_values);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_magnitude_set_values_in_element.  "
					"Could not evaluate source field for %s.",field->name);
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_magnitude_set_values_in_element.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_magnitude_set_values_in_element */

#define Computed_field_magnitude_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 2 October 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_magnitude_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 2 October 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_magnitude(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 2 October 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_magnitude);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_magnitude.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_magnitude */

static char *Computed_field_magnitude_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_magnitude_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_magnitude_type_string, &error);
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
			"Computed_field_magnitude_get_command_string.  "
			"Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_magnitude_get_command_string */

#define Computed_field_magnitude_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_magnitude(struct Computed_field *field,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_MAGNITUDE with the supplied
<source_field>.  Sets the number of components equal to the <source_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_magnitude);
	if (field&&source_field)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_NEW_TYPES;
			field->type_string = computed_field_magnitude_type_string;
			field->number_of_components = 1;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)1;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(magnitude);
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
			"Computed_field_set_type_magnitude.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_magnitude */

int Computed_field_get_type_magnitude(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_MAGNITUDE, the 
<source_field> used by it is returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_magnitude);
	if (field && (COMPUTED_FIELD_NEW_TYPES == field->type) &&
		(field->type_string == computed_field_magnitude_type_string) &&
		source_field)
	{
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_magnitude.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_magnitude */

static int define_Computed_field_type_magnitude(struct Parse_state *state,
	void *field_void,void *computed_field_vector_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 18 December 2001

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_MAGNITUDE (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field,*source_field;
	struct Computed_field_vector_operations_package 
		*computed_field_vector_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_magnitude);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_vector_operations_package=
		(struct Computed_field_vector_operations_package *)
		computed_field_vector_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		if (computed_field_magnitude_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_magnitude(field, &source_field);
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
				computed_field_vector_operations_package->computed_field_manager;
			set_source_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"field",&source_field,
				&set_source_field_data,set_Computed_field_conditional);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_magnitude(field, source_field);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_magnitude.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			DESTROY(Option_table)(&option_table);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_magnitude.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_magnitude */

int Computed_field_register_types_vector_operations(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 2 October 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_vector_operations_package 
		computed_field_vector_operations_package;

	ENTER(Computed_field_register_types_vector_operations);
	if (computed_field_package)
	{
		computed_field_vector_operations_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_magnitude_type_string,
			define_Computed_field_type_magnitude,
			&computed_field_vector_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_normalise_type_string,
			define_Computed_field_type_normalise,
			&computed_field_vector_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_cross_product_type_string,
			define_Computed_field_type_cross_product,
			&computed_field_vector_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_dot_product_type_string,
			define_Computed_field_type_dot_product,
			&computed_field_vector_operations_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_vector_operations.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_vector_operations */
