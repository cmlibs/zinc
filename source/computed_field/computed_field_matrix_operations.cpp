/*******************************************************************************
FILE : computed_field_vector_operations.c

LAST MODIFIED : 27 September 2000

DESCRIPTION :
Implements a number of basic vector operations on computed fields.
==============================================================================*/
#include <math.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_matrix_operations.h"
#include "computed_field/computed_field_private.h"
#include "general/debug.h"
#include "user_interface/message.h"

struct Computed_field_matrix_operations_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
};

struct Computed_field_matrix_multiply_type_specific_data
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
==============================================================================*/
{
	int number_of_rows;
}; /* struct Computed_field_matrix_multiply_type_specific_data */

static char computed_field_matrix_multiply_type_string[] = "matrix_multiply";

int Computed_field_is_type_matrix_multiply(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
Returns true if <field> has the appropriate static type string.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_matrix_multiply);
	if (field)
	{
		return_code =
			(field->type_string == computed_field_matrix_multiply_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_matrix_multiply.  Missing field.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_matrix_multiply */

static int Computed_field_matrix_multiply_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_matrix_multiply_clear_type_specific);
	if (field && field->type_specific_data)
	{
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_matrix_multiply_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_matrix_multiply_clear_type_specific */

static void *Computed_field_matrix_multiply_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_matrix_multiply_type_specific_data *destination,
		*source;

	ENTER(Computed_field_matrix_multiply_copy_type_specific);
	if (field && (source = 
		(struct Computed_field_matrix_multiply_type_specific_data *)
		field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_matrix_multiply_type_specific_data, 1))
		{
			destination->number_of_rows=source->number_of_rows;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_matrix_multiply_copy_type_specific.  "
				"Unable to allocate memory.");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_matrix_multiply_copy_type_specific.  Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_matrix_multiply_copy_type_specific */

#define Computed_field_matrix_multiply_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_matrix_multiply_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;
	struct Computed_field_matrix_multiply_type_specific_data *data,
		*other_data;

	ENTER(Computed_field_matrix_multiply_type_specific_contents_match);
	if (field && other_computed_field && (data = 
		(struct Computed_field_matrix_multiply_type_specific_data *)
		field->type_specific_data) && (other_data =
		(struct Computed_field_matrix_multiply_type_specific_data *)
		other_computed_field->type_specific_data))
	{
		if (data->number_of_rows=other_data->number_of_rows)
		{
			return_code = 1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_matrix_multiply_type_specific_contents_match */

#define Computed_field_matrix_multiply_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_matrix_multiply_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_matrix_multiply_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_matrix_multiply_can_be_destroyed \
	(Computed_field_can_be_destroyed_function)NULL
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
No special criteria on the destroy
==============================================================================*/

static int Computed_field_matrix_multiply_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value *a, *b, sum;
	int i, j, k, m, n, return_code, s;
	struct Computed_field_matrix_multiply_type_specific_data *data;

	ENTER(Computed_field_matrix_multiply_evaluate_cache_at_node);
	if (field && node && (data =
		(struct Computed_field_matrix_multiply_type_specific_data *)
		field->type_specific_data) && (m = data->number_of_rows)&&
		(n = field->source_fields[0]->number_of_components) &&
		(0 == (n % m)) && (s = n/m) &&
		(n = field->source_fields[1]->number_of_components) &&
		(0 == (n % s)) && (n /= s))
	{
		/* first multiplicand is m rows x s columns,
			 second multiplicand is s rows x n columns,
			 result is m rows x n columns */
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node))
		{
			a=field->source_fields[0]->values;
			b=field->source_fields[1]->values;
			for (i=0;i<m;i++)
			{
				for (j=0;j<n;j++)
				{
					sum=0.0;
					for (k=0;k<s;k++)
					{
						sum += a[i*s+k] * b[k*n+j];
					}
					field->values[i*n+j]=sum;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_matrix_multiply_evaluate_cache_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_matrix_multiply_evaluate_cache_at_node */

static int Computed_field_matrix_multiply_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
Evaluate the fields cache in the element.
==============================================================================*/
{
	FE_value *a,*ad,*b,*bd,sum;
	int d, element_dimension, i, j, k, m, n, return_code, s;
	struct Computed_field_matrix_multiply_type_specific_data *data;

	ENTER(Computed_field_matrix_multiply_evaluate_cache_in_element);
	if (field && element && xi && (data =
		(struct Computed_field_matrix_multiply_type_specific_data *)
		field->type_specific_data) && (m = data->number_of_rows)&&
		(n = field->source_fields[0]->number_of_components) &&
		(0 == (n % m)) && (s = n/m) &&
		(n = field->source_fields[1]->number_of_components) &&
		(0 == (n % s)) && (n /= s))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			a=field->source_fields[0]->values;
			b=field->source_fields[1]->values;
			for (i=0;i<m;i++)
			{
				for (j=0;j<n;j++)
				{
					sum=0.0;
					for (k=0;k<s;k++)
					{
						sum += a[i*s+k] * b[k*n+j];
					}
					field->values[i*n+j]=sum;
				}
			}
			if (calculate_derivatives)
			{
				element_dimension = get_FE_element_dimension(element);
				for (d=0;d<element_dimension;d++)
				{
					/* use the product rule */
					a = field->source_fields[0]->values;
					ad = field->source_fields[0]->derivatives+d;
					b = field->source_fields[1]->values;
					bd = field->source_fields[1]->derivatives+d;
					for (i=0;i<m;i++)
					{
						for (j=0;j<n;j++)
						{
							sum=0.0;
							for (k=0;k<s;k++)
							{
								sum += a[i*s+k] * bd[element_dimension*(k*n+j)] +
									ad[element_dimension*(i*s+k)] * b[k*n+j];
							}
							field->derivatives[element_dimension*(i*n+j)+d]=sum;
						}
					}
				}
				field->derivatives_valid=1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_matrix_multiply_evaluate_cache_in_element.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_matrix_multiply_evaluate_cache_in_element */

#define Computed_field_matrix_multiply_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_matrix_multiply_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_matrix_multiply_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_matrix_multiply_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_matrix_multiply_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_matrix_multiply_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_matrix_multiply(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_matrix_multiply_type_specific_data *data;

	ENTER(List_Computed_field_matrix_multiply);
	if (field && (data = 
		(struct Computed_field_matrix_multiply_type_specific_data *)
		field->type_specific_data))
	{
		display_message(INFORMATION_MESSAGE,
			"    number of rows : %d\n",data->number_of_rows);
		display_message(INFORMATION_MESSAGE,"    source fields : %s %s\n",
			field->source_fields[0]->name,field->source_fields[1]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_matrix_multiply.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_matrix_multiply */

static int list_Computed_field_matrix_multiply_commands(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_matrix_multiply_type_specific_data *data;

	ENTER(list_Computed_field_matrix_multiply_commands);
	if (field && (data = 
		(struct Computed_field_matrix_multiply_type_specific_data *)
		field->type_specific_data))
	{
		display_message(INFORMATION_MESSAGE," number_of_rows %d fields %s %s",
			data->number_of_rows,field->source_fields[0]->name,
			field->source_fields[1]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_matrix_multiply_commands.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_matrix_multiply_commands */

int Computed_field_set_type_matrix_multiply(struct Computed_field *field,
	int number_of_rows,struct Computed_field *source_field1,
	struct Computed_field *source_field2)
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_MATRIX_MULTIPLY producing the result
<source_field1> x <source_field2>, with <number_of_rows> rows in both
<source_field1> and the result. From the <number_of_rows> the columns in
<source_field1>, rows in <source_field2> and then columns in <source_field2>
are implied and checked.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int n, number_of_source_fields, return_code, s;
	struct Computed_field **temp_source_fields;
	struct Computed_field_matrix_multiply_type_specific_data *data;

	ENTER(Computed_field_set_type_matrix_multiply);
	if (field && (0 < number_of_rows) && source_field1 && source_field2)
	{
		if ((n = source_field1->number_of_components) &&
			(0 == (n % number_of_rows)) && (s = n/number_of_rows))
		{
			if ((n = source_field2->number_of_components) &&
				(0 == (n % s)) && (n /= s))
			{
				/* 1. make dynamic allocations for any new type-specific data */
				number_of_source_fields=2;
				if (ALLOCATE(temp_source_fields,struct Computed_field *,
					number_of_source_fields) && ALLOCATE(data,
						struct Computed_field_matrix_multiply_type_specific_data,1))
				{
					/* 2. free current type-specific data */
					Computed_field_clear_type(field);
					/* 3. establish the new type */
					field->type=COMPUTED_FIELD_NEW_TYPES;
					field->type_string = computed_field_matrix_multiply_type_string;
					field->number_of_components = number_of_rows*n;
					temp_source_fields[0]=ACCESS(Computed_field)(source_field1);
					temp_source_fields[1]=ACCESS(Computed_field)(source_field2);
					field->source_fields=temp_source_fields;
					field->number_of_source_fields=number_of_source_fields;
					data->number_of_rows=number_of_rows;
					field->type_specific_data = (void *)data;

					/* Set all the methods */
					field->computed_field_clear_type_specific_function =
						Computed_field_matrix_multiply_clear_type_specific;
					field->computed_field_copy_type_specific_function =
						Computed_field_matrix_multiply_copy_type_specific;
					field->computed_field_clear_cache_type_specific_function =
						Computed_field_matrix_multiply_clear_cache_type_specific;
					field->computed_field_type_specific_contents_match_function =
						Computed_field_matrix_multiply_type_specific_contents_match;
					field->computed_field_is_defined_in_element_function =
						Computed_field_matrix_multiply_is_defined_in_element;
					field->computed_field_is_defined_at_node_function =
						Computed_field_matrix_multiply_is_defined_at_node;
					field->computed_field_has_numerical_components_function =
						Computed_field_matrix_multiply_has_numerical_components;
					field->computed_field_evaluate_cache_at_node_function =
						Computed_field_matrix_multiply_evaluate_cache_at_node;
					field->computed_field_evaluate_cache_in_element_function =
						Computed_field_matrix_multiply_evaluate_cache_in_element;
					field->computed_field_evaluate_as_string_at_node_function =
						Computed_field_matrix_multiply_evaluate_as_string_at_node;
					field->computed_field_evaluate_as_string_in_element_function =
						Computed_field_matrix_multiply_evaluate_as_string_in_element;
					field->computed_field_set_values_at_node_function =
						Computed_field_matrix_multiply_set_values_at_node;
					field->computed_field_set_values_in_element_function =
						Computed_field_matrix_multiply_set_values_in_element;
					field->computed_field_get_native_discretization_in_element_function =
						Computed_field_matrix_multiply_get_native_discretization_in_element;
					field->computed_field_find_element_xi_function =
						Computed_field_matrix_multiply_find_element_xi;
					field->list_Computed_field_function = 
						list_Computed_field_matrix_multiply;
					field->list_Computed_field_commands_function = 
						list_Computed_field_matrix_multiply_commands;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_set_type_matrix_multiply.  "
						"Unable to allocate memory");
					DEALLOCATE(temp_source_fields);
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_matrix_multiply.  "
					"Field %s has wrong number of components",source_field2->name);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_matrix_multiply.  "
				"Field %s has wrong number of components",source_field1->name);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_matrix_multiply.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_matrix_multiply */

int Computed_field_get_type_matrix_multiply(struct Computed_field *field,
	int *number_of_rows, struct Computed_field **source_field1,
	struct Computed_field **source_field2)
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_MATRIX_MULTIPLY, the 
<number_of_rows> and <source_fields> used by it are returned.
==============================================================================*/
{
	int return_code;
	struct Computed_field_matrix_multiply_type_specific_data *data;

	ENTER(Computed_field_get_type_matrix_multiply);
	if (field && (COMPUTED_FIELD_NEW_TYPES == field->type) &&
		(field->type_string == computed_field_matrix_multiply_type_string) &&
		source_field1 && source_field2 && (data = 
			(struct Computed_field_matrix_multiply_type_specific_data *)
			field->type_specific_data))
	{
		*number_of_rows = data->number_of_rows;
		*source_field1 = field->source_fields[0];
		*source_field2 = field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_matrix_multiply.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_matrix_multiply */

static int define_Computed_field_type_matrix_multiply(struct Parse_state *state,
	void *field_void,void *computed_field_matrix_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_MATRIX_MULTIPLY (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	int i, number_of_rows, return_code;
	struct Computed_field *field,**source_fields;
	struct Computed_field_matrix_operations_package 
		*computed_field_matrix_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_field_array_data;
	struct Set_Computed_field_conditional_data set_field_data;

	ENTER(define_Computed_field_type_matrix_multiply);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_matrix_operations_package=
		(struct Computed_field_matrix_operations_package *)
		computed_field_matrix_operations_package_void))
	{
		return_code=1;
		if (ALLOCATE(source_fields,struct Computed_field *,2))
		{
			/* get valid parameters for matrix_multiply field */
			number_of_rows = 1;
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			if (computed_field_matrix_multiply_type_string ==
				Computed_field_get_type_string(field))
			{
				return_code=Computed_field_get_type_matrix_multiply(field,
					&number_of_rows,&(source_fields[0]),&(source_fields[1]));
			}
			/* ACCESS the source fields for set_Computed_field_array */
			for (i=0;i<2;i++)
			{
				if (source_fields[i])
				{
					ACCESS(Computed_field)(source_fields[i]);
				}
			}
			if (return_code)
			{
				/* try to handle help first */
				if (current_token=state->current_token)
				{
					if (!(strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
					{
						option_table = CREATE(Option_table)();
						Option_table_add_entry(option_table,"number_of_rows",
							&number_of_rows,NULL,set_int_positive);
						set_field_data.conditional_function=
							Computed_field_has_numerical_components;
						set_field_data.conditional_function_user_data=(void *)NULL;
						set_field_data.computed_field_manager=
							computed_field_matrix_operations_package->computed_field_manager;
						set_field_array_data.number_of_fields=2;
						set_field_array_data.conditional_data= &set_field_data;
						Option_table_add_entry(option_table,"fields",source_fields,
							&set_field_array_data,set_Computed_field_array);
						return_code=Option_table_multi_parse(option_table,state);
						DESTROY(Option_table)(&option_table);
					}
					else
					{
						/* ... only if the "number_of_rows" token is next */
						if (fuzzy_string_compare(current_token,"number_of_rows"))
						{
							option_table = CREATE(Option_table)();
							/* number_of_rows */
							Option_table_add_entry(option_table,"number_of_rows",
								&number_of_rows,NULL,set_int_positive);
							return_code = Option_table_parse(option_table,state);
							DESTROY(Option_table)(&option_table);
						}
						if (return_code)
						{
							option_table = CREATE(Option_table)();
							set_field_data.conditional_function=
								Computed_field_has_numerical_components;
							set_field_data.conditional_function_user_data=(void *)NULL;
							set_field_data.computed_field_manager=
								computed_field_matrix_operations_package->
								computed_field_manager;
							set_field_array_data.number_of_fields=2;
							set_field_array_data.conditional_data= &set_field_data;
							Option_table_add_entry(option_table,"fields",source_fields,
								&set_field_array_data,set_Computed_field_array);
							if (return_code=Option_table_multi_parse(option_table,state))
							{
								return_code = Computed_field_set_type_matrix_multiply(field,
									number_of_rows,source_fields[0],source_fields[1]);
							}
							DESTROY(Option_table)(&option_table);
						}
						if (!return_code)
						{
							/* error */
							display_message(ERROR_MESSAGE,
								"define_Computed_field_type_matrix_multiply.  Failed");
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "Missing command options.");
				}
			}
			for (i=0;i<2;i++)
			{
				if (source_fields[i])
				{
					DEACCESS(Computed_field)(&(source_fields[i]));
				}
			}
			DEALLOCATE(source_fields);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_matrix_multiply.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_matrix_multiply.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_matrix_multiply */

struct Computed_field_projection_type_specific_data
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
==============================================================================*/
{
	double *projection_matrix;
}; /* struct Computed_field_projection_type_specific_data */

static char computed_field_projection_type_string[] = "projection";

int Computed_field_is_type_projection(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
Returns true if <field> has the appropriate static type string.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_projection);
	if (field)
	{
		return_code = (computed_field_projection_type_string == field->type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_projection.  Missing field.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_projection */

static int Computed_field_projection_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_projection_type_specific_data *data;

	ENTER(Computed_field_projection_clear_type_specific);
	if (field && (data = (struct Computed_field_projection_type_specific_data *)
		field->type_specific_data))
	{
		DEALLOCATE(data->projection_matrix);
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_projection_clear_type_specific.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_projection_clear_type_specific */

static void *Computed_field_projection_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	int number_of_projection_values;
	struct Computed_field_projection_type_specific_data *destination,
		*source;

	ENTER(Computed_field_projection_copy_type_specific);
	if (field && (source = 
		(struct Computed_field_projection_type_specific_data *)
		field->type_specific_data))
	{
		number_of_projection_values =
			(field->source_fields[0]->number_of_components + 1) *
			(field->number_of_components + 1);
		if (ALLOCATE(destination,
			struct Computed_field_projection_type_specific_data, 1)&&
			ALLOCATE(destination->projection_matrix,double,
				number_of_projection_values))
		{
			memcpy(destination->projection_matrix,source->projection_matrix,
				number_of_projection_values*sizeof(double));
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_projection_copy_type_specific.  Not enough memory.");
			DEALLOCATE(destination);
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_projection_copy_type_specific.  Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_projection_copy_type_specific */

#define Computed_field_projection_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_projection_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
Compare the type specific data.
==============================================================================*/
{
	int i, number_of_projection_values, return_code;
	struct Computed_field_projection_type_specific_data *data, *other_data;

	ENTER(Computed_field_projection_type_specific_contents_match);
	if (field && other_computed_field && (data = 
		(struct Computed_field_projection_type_specific_data *)
		field->type_specific_data) && (other_data =
		(struct Computed_field_projection_type_specific_data *)
		other_computed_field->type_specific_data))
	{
		number_of_projection_values =
			(field->source_fields[0]->number_of_components + 1) *
			(field->number_of_components + 1);
		if (number_of_projection_values ==
			((other_computed_field->source_fields[0]->number_of_components + 1) *
				(other_computed_field->number_of_components + 1)))
		{
			return_code=1;
			for (i=0;return_code&&(i<number_of_projection_values);i++)
			{
				if (data->projection_matrix[i] != other_data->projection_matrix[i])
				{
					return_code=0;
				}
			}
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_projection_type_specific_contents_match */

#define Computed_field_projection_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_projection_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_projection_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_projection_can_be_destroyed \
	(Computed_field_can_be_destroyed_function)NULL
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
No special criteria on the destroy.
==============================================================================*/

static int Computed_field_evaluate_projection_matrix(
	struct Computed_field *field,int element_dimension, int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 8 May 2000

DESCRIPTION :
Function called by Computed_field_evaluate_in_element to compute a field
transformed by a projection matrix.
NOTE: Assumes that values and derivatives arrays are already allocated in
<field>, and that its source_fields are already computed (incl. derivatives)
for the same element, with the given <element_dimension> = number of Xi coords.
==============================================================================*/
{
	double dhdxi, dh1dxi, perspective;
	int coordinate_components, i, j, k,return_code;
	struct Computed_field_projection_type_specific_data *data;

	ENTER(Computed_field_evaluate_projection_matrix);
	if (field && (computed_field_projection_type_string == field->type_string) &&
		(data = (struct Computed_field_projection_type_specific_data *)
			field->type_specific_data))
	{
		if (calculate_derivatives)
		{
			field->derivatives_valid=1;
		}
		else
		{
			field->derivatives_valid=0;
		}

		/* Calculate the transformed coordinates */
		coordinate_components=field->source_fields[0]->number_of_components;
		for (i = 0 ; i < field->number_of_components ; i++)
		{
			field->values[i] = 0.0;
			for (j = 0 ; j < coordinate_components ; j++)
			{
				field->values[i] +=
					data->projection_matrix[i * (coordinate_components + 1) + j] *
					field->source_fields[0]->values[j];
			}
			/* The last source value is fixed at 1 */
			field->values[i] +=  data->projection_matrix[
				i * (coordinate_components + 1) + coordinate_components];
		}

		/* The last calculated value is the perspective value which divides through
			all the other components */
		perspective = 0.0;
		for (j = 0 ; j < coordinate_components ; j++)
		{
			perspective += data->projection_matrix[field->number_of_components
				* (coordinate_components + 1) + j] * field->source_fields[0]->values[j];
		}
		perspective += data->projection_matrix[field->number_of_components 
			* (coordinate_components + 1) + coordinate_components];
		
		if (calculate_derivatives)
		{
			for (k=0;k<element_dimension;k++)
			{
				/* Calculate the coordinate derivatives without perspective */
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					field->derivatives[i * element_dimension + k] = 0.0;
					for (j = 0 ; j < coordinate_components ; j++)
					{
						field->derivatives[i * element_dimension + k] += 
							data->projection_matrix[i * (coordinate_components + 1) + j]
							* field->source_fields[0]->derivatives[j * element_dimension + k];
					}
				}

				/* Calculate the perspective derivative */
				dhdxi = 0.0;
				for (j = 0 ; j < coordinate_components ; j++)
				{
					dhdxi += data->projection_matrix[field->number_of_components 
						* (coordinate_components + 1) + j]
						* field->source_fields[0]->derivatives[j *element_dimension + k];
				}

				/* Calculate the perspective reciprocal derivative using chaing rule */
				dh1dxi = (-1.0) / (perspective * perspective) * dhdxi;

				/* Calculate the derivatives of the perspective scaled transformed
					 coordinates, which is ultimately what we want */
				for (i = 0 ; i < field->number_of_components ; i++)
				{
					field->derivatives[i * element_dimension + k] = 
						field->derivatives[i * element_dimension + k] / perspective
						+ field->values[i] * dh1dxi;
				}
			}
		}

		/* Now apply the perspective scaling to the non derivative transformed
			 coordinates */
		for (i = 0 ; i < field->number_of_components ; i++)
		{
			field->values[i] /= perspective;
		}

		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_projection_matrix.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_evaluate_projection_matrix */

static int Computed_field_projection_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_projection_evaluate_cache_at_node);
	if (field && node)
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node))
		{
			return_code=Computed_field_evaluate_projection_matrix(field,
				/*element_dimension*/0, /*calculate_derivatives*/0);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_projection_evaluate_cache_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_projection_evaluate_cache_at_node */

static int Computed_field_projection_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
Evaluate the fields cache in the element.
==============================================================================*/
{
	int element_dimension,return_code;

	ENTER(Computed_field_projection_evaluate_cache_in_element);
	if (field && element && xi)
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, top_level_element, calculate_derivatives))
		{
			element_dimension=get_FE_element_dimension(element);
			return_code=Computed_field_evaluate_projection_matrix(field,
				element_dimension, calculate_derivatives);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_projection_evaluate_cache_in_element.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_projection_evaluate_cache_in_element */

#define Computed_field_projection_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_projection_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_projection_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_projection_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_projection_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_projection_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_projection(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
==============================================================================*/
{
	int i,number_of_projection_values,return_code;
	struct Computed_field_projection_type_specific_data *data;

	ENTER(List_Computed_field_projection);
	if (field && (data = 
		(struct Computed_field_projection_type_specific_data *)
		field->type_specific_data))
	{
		display_message(INFORMATION_MESSAGE,"    source field : %s\n",
			field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,"    projection_matrix : ");
		number_of_projection_values =
			(field->source_fields[0]->number_of_components + 1)
			* (field->number_of_components + 1);
		for (i=0;i<number_of_projection_values;i++)
		{
			display_message(INFORMATION_MESSAGE," %g",data->projection_matrix[i]);
		}
		display_message(INFORMATION_MESSAGE,"\n");
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_projection.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_projection */

static int list_Computed_field_projection_commands(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
==============================================================================*/
{
	int i,number_of_projection_values,return_code;
	struct Computed_field_projection_type_specific_data *data;

	ENTER(list_Computed_field_projection_commands);
	if (field && (data = 
		(struct Computed_field_projection_type_specific_data *)
		field->type_specific_data))
	{
		display_message(INFORMATION_MESSAGE," field %s number_of_components %d",
			field->source_fields[0]->name, field->number_of_components);
		display_message(INFORMATION_MESSAGE," projection_matrix");
		number_of_projection_values =
			(field->source_fields[0]->number_of_components + 1)
			* (field->number_of_components + 1);
		for (i=0;i<number_of_projection_values;i++)
		{
			display_message(INFORMATION_MESSAGE," %g",data->projection_matrix[i]);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_projection_commands.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_projection_commands */

int Computed_field_set_type_projection(struct Computed_field *field,
	struct Computed_field *source_field, int number_of_components, 
	double *projection_matrix)
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_PROJECTION, returning the <source_field>
with each component multiplied by the perspective <projection_matrix>.
The <projection_matrix> array must be of size
(source_field->number_of_components + 1) * (field->number_of_components + 1).
The source vector is appended with a 1 to make
source_field->number_of_components + 1 components. The extra calculated value
is a perspective value which divides through each of the other components.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_projection_values,number_of_source_fields,return_code;
	struct Computed_field **source_fields;
	struct Computed_field_projection_type_specific_data *data;

	ENTER(Computed_field_set_type_projection);
	if (field&&source_field&&projection_matrix)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_projection_values = (source_field->number_of_components + 1)
			* (number_of_components + 1);
		number_of_source_fields=1;
		if (ALLOCATE(data,struct Computed_field_projection_type_specific_data,1) &&
			ALLOCATE(data->projection_matrix,double,number_of_projection_values)&&
			ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_NEW_TYPES;
			field->type_string = computed_field_projection_type_string;
			field->number_of_components=number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			memcpy(data->projection_matrix,projection_matrix,
				number_of_projection_values*sizeof(double));
			field->type_specific_data = data;

			/* Set all the methods */
			field->computed_field_clear_type_specific_function =
			Computed_field_projection_clear_type_specific;
			field->computed_field_copy_type_specific_function =
			Computed_field_projection_copy_type_specific;
			field->computed_field_clear_cache_type_specific_function =
			Computed_field_projection_clear_cache_type_specific;
			field->computed_field_type_specific_contents_match_function =
			Computed_field_projection_type_specific_contents_match;
			field->computed_field_is_defined_in_element_function =
			Computed_field_projection_is_defined_in_element;
			field->computed_field_is_defined_at_node_function =
			Computed_field_projection_is_defined_at_node;
			field->computed_field_has_numerical_components_function =
			Computed_field_projection_has_numerical_components;
			field->computed_field_evaluate_cache_at_node_function =
			Computed_field_projection_evaluate_cache_at_node;
			field->computed_field_evaluate_cache_in_element_function =
			Computed_field_projection_evaluate_cache_in_element;
			field->computed_field_evaluate_as_string_at_node_function =
			Computed_field_projection_evaluate_as_string_at_node;
			field->computed_field_evaluate_as_string_in_element_function =
			Computed_field_projection_evaluate_as_string_in_element;
			field->computed_field_set_values_at_node_function =
			Computed_field_projection_set_values_at_node;
			field->computed_field_set_values_in_element_function =
			Computed_field_projection_set_values_in_element;
			field->computed_field_get_native_discretization_in_element_function =
			Computed_field_projection_get_native_discretization_in_element;
			field->computed_field_find_element_xi_function =
			Computed_field_projection_find_element_xi;
			field->list_Computed_field_function = 
			list_Computed_field_projection;
			field->list_Computed_field_commands_function = 
			list_Computed_field_projection_commands;
		}
		else
		{
			if (data)
			{
				DEALLOCATE(data->projection_matrix);
				DEALLOCATE(data);
			}
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_projection.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_projection */

int Computed_field_get_type_projection(struct Computed_field *field,
	struct Computed_field **source_field, int *number_of_components,
	double **projection_matrix)
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_PROJECTION, the source_field and
projection matrix used by it are returned. Since the number of projections is
equal to the number of components in the source_field (and you don't know this
yet), this function returns in *projection_matrix a pointer to an allocated 
array containing the values.
It is up to the calling function to DEALLOCATE returned <*projection_matrix>.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int number_of_projection_values,return_code;
	struct Computed_field_projection_type_specific_data *data;

	ENTER(Computed_field_get_type_projection);
	if (field && (COMPUTED_FIELD_NEW_TYPES == field->type) &&
		(field->type_string == computed_field_projection_type_string) &&
		source_field && number_of_components && projection_matrix && (data =
			(struct Computed_field_projection_type_specific_data *)
			field->type_specific_data))
	{
		number_of_projection_values =
			(field->source_fields[0]->number_of_components + 1) *
			(field->number_of_components + 1);
		if (ALLOCATE(*projection_matrix,double,number_of_projection_values))
		{
			*number_of_components = field->number_of_components;
			*source_field=field->source_fields[0];
			memcpy(*projection_matrix,data->projection_matrix,
				number_of_projection_values*sizeof(double));
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_projection.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_projection.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_projection */

static int define_Computed_field_type_projection(struct Parse_state *state,
	void *field_void,void *computed_field_matrix_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 5 May 2000

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_PROJECTION (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	double *projection_matrix,*temp_projection_matrix;
	int i,number_of_components,number_of_projection_values,return_code,
		temp_number_of_projection_values;
	static struct Modifier_entry 
		field_option_table[]=
		{
			{"field",NULL,NULL,set_Computed_field_conditional},
			{NULL,NULL,NULL,NULL}
		},
		number_of_components_option_table[]=
		{
			{"number_of_components",NULL,NULL,set_int_positive},
			{NULL,NULL,NULL,NULL}
		},
		projection_matrix_option_table[]=
		{
			{"projection_matrix",NULL,NULL,set_double_vector},
			{NULL,NULL,NULL,NULL}
		},
		help_option_table[]=
		{
			{"field",NULL,NULL,set_Computed_field_conditional},
			{"number_of_components",NULL,NULL,set_int_positive},
			{"projection_matrix",NULL,NULL,set_FE_value_array},
			{NULL,NULL,NULL,NULL}
		};
	struct Computed_field *field,*source_field;
	struct Computed_field_matrix_operations_package 
		*computed_field_matrix_operations_package;
	struct Set_Computed_field_conditional_data set_field_data;

	ENTER(define_Computed_field_type_projection);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_matrix_operations_package=
			(struct Computed_field_matrix_operations_package *)
			computed_field_matrix_operations_package_void))
	{
		return_code=1;
		set_field_data.conditional_function=
			(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
		set_field_data.conditional_function_user_data=(void *)NULL;
		set_field_data.computed_field_manager=
			computed_field_matrix_operations_package->computed_field_manager;
		/* get valid parameters for projection field */
		source_field=(struct Computed_field *)NULL;
		projection_matrix=(double *)NULL;
		if (computed_field_projection_type_string == field->type_string)
		{
			return_code=Computed_field_get_type_projection(field,
				&source_field,&number_of_components,&projection_matrix);
			number_of_projection_values = (source_field->number_of_components + 1)
				* (number_of_components + 1);
		}
		else
		{
			/* ALLOCATE and fill array of projection_matrix - with zeroes */
			number_of_components = 0;
			number_of_projection_values = 1;
			if (ALLOCATE(projection_matrix,double,1))
			{
				projection_matrix[0]=0.0;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_projection.  Not enough memory");
				return_code=0;
			}
		}
		if (return_code)
		{
			/* try to handle help first */
			if (current_token=state->current_token)
			{
				if (!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
				{
					(help_option_table[0]).to_be_modified= &source_field;
					(help_option_table[0]).user_data= &set_field_data;
					(help_option_table[1]).to_be_modified= &number_of_projection_values;
					(help_option_table[2]).to_be_modified= projection_matrix;
					(help_option_table[2]).user_data= &number_of_projection_values;
					return_code=process_multiple_options(state,help_option_table);
				}
			}
			/* keep the number_of_projection_values to maintain any current ones */
			temp_number_of_projection_values = number_of_projection_values;
			/* parse the field... */
			if (return_code&&(current_token=state->current_token))
			{
				/* ... only if the "field" token is next */
				if (fuzzy_string_compare(current_token,"field"))
				{
					(field_option_table[0]).to_be_modified= &source_field;
					(field_option_table[0]).user_data= &set_field_data;
					return_code=process_option(state,field_option_table);
				}
				if (! source_field)
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_projection.  Invalid field");
					return_code=0;
				}
			}
			/* parse the number_of_components... */
			if (return_code&&(current_token=state->current_token))
			{
				/* ... only if the "number_of_components" token is next */
				if (fuzzy_string_compare(current_token,"number_of_components"))
				{
					(number_of_components_option_table[0]).to_be_modified=
						&number_of_components;
					return_code=process_option(state,number_of_components_option_table);
				}
			}
			if (return_code)
			{
				number_of_projection_values = (source_field->number_of_components + 1)
					* (number_of_components + 1);
				if (temp_number_of_projection_values != number_of_projection_values)
				{
					if (REALLOCATE(temp_projection_matrix,projection_matrix,double,
						number_of_projection_values))
					{
						projection_matrix=temp_projection_matrix;
						/* clear any new projection_matrix to zero */
						for (i=temp_number_of_projection_values;i<number_of_projection_values;i++)
						{
							projection_matrix[i]=0.0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_projection.  Not enough memory");
						return_code=0;
					}
				}
			}
			/* parse the projection_matrix */
			if (return_code&&state->current_token)
			{
				(projection_matrix_option_table[0]).to_be_modified= projection_matrix;
				(projection_matrix_option_table[0]).user_data= &number_of_projection_values;
				return_code=process_multiple_options(state,projection_matrix_option_table);
			}
			if (return_code)
			{
				return_code=
					Computed_field_set_type_projection(field,source_field,number_of_components,projection_matrix);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_projection.  Failed");
				}
			}
			/* clean up the projection_matrix array */
			DEALLOCATE(projection_matrix);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_projection.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_projection */

int Computed_field_register_types_matrix_operations(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_matrix_operations_package 
		computed_field_matrix_operations_package;

	ENTER(Computed_field_register_types_matrix_operations);
	if (computed_field_package)
	{
		computed_field_matrix_operations_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_matrix_multiply_type_string,
			define_Computed_field_type_matrix_multiply,
			&computed_field_matrix_operations_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_projection_type_string,
			define_Computed_field_type_projection,
			&computed_field_matrix_operations_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_matrix_operations.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_matrix_operations */
