/*******************************************************************************
FILE : computed_field_matrix_operations.c

LAST MODIFIED : 21 January 2002

DESCRIPTION :
Implements a number of basic vector operations on computed fields.
==============================================================================*/
#include <math.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_matrix_operations.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/matrix_vector.h"
#include "general/mystring.h"
#include "user_interface/message.h"

struct Computed_field_matrix_operations_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
};

static int Computed_field_get_square_matrix_size(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 11 December 2000

DESCRIPTION :
If <field> can represent a square matrix with numerical components, the number
of rows = number of columns is returned.
==============================================================================*/
{
	int n, return_code, size;

	ENTER(Computed_field_get_square_matrix_size);
	return_code = 0;
	if (field)
	{
		size = field->number_of_components;
		n = 1;
		while (n * n < size)
		{
			n++;
		}
		if (n * n == size)
		{
			return_code = n;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_square_matrix_size.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_square_matrix_size */

static int Computed_field_is_square_matrix(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 11 December 2000

DESCRIPTION :
Returns true if <field> can represent a square matrix, on account of having n*n
components, where n is a positive integer. If matrix is square, n is returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_square_matrix);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code =
			Computed_field_has_numerical_components(field, (void *)NULL) &&
			(0 != Computed_field_get_square_matrix_size(field));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_square_matrix.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_square_matrix */

struct Computed_field_eigenvalues_type_specific_data
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
==============================================================================*/
{
	/* cache for matrix, eigenvalues and eigenvectors */
	double *a, *d, *v;
}; /* struct Computed_field_eigenvalues_type_specific_data */

static char computed_field_eigenvalues_type_string[] = "eigenvalues";

int Computed_field_is_type_eigenvalues(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Returns true if <field> has the appropriate static type string.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_eigenvalues);
	if (field)
	{
		return_code =
			(field->type_string == computed_field_eigenvalues_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_eigenvalues.  Missing field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_eigenvalues */

int Computed_field_is_type_eigenvalues_conditional(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
List conditional function version of Computed_field_is_type_eigenvalues.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_eigenvalues_conditional);
	USE_PARAMETER(dummy_void);
	return_code = Computed_field_is_type_eigenvalues(field);
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_eigenvalues_conditional */

static int Computed_field_eigenvalues_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 11 December 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_eigenvalues_type_specific_data *data;

	ENTER(Computed_field_eigenvalues_clear_type_specific);
	if (field && (data =
		(struct Computed_field_eigenvalues_type_specific_data *)
		field->type_specific_data))
	{
		if (data->a)
		{
			DEALLOCATE(data->a);
		}
		if (data->d)
		{
			DEALLOCATE(data->d);
		}
		if (data->v)
		{
			DEALLOCATE(data->v);
		}
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_eigenvalues_clear_type_specific.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_eigenvalues_clear_type_specific */

static void *Computed_field_eigenvalues_copy_type_specific(
	struct Computed_field *source_field, struct Computed_field *destination_field)
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_eigenvalues_type_specific_data *destination;

	ENTER(Computed_field_eigenvalues_copy_type_specific);
	if (source_field && destination_field)
	{
		if (ALLOCATE(destination,
			struct Computed_field_eigenvalues_type_specific_data, 1))
		{
			/* following arrays are allocated when field calculated, cleared when
				 cache cleared */
			destination->a = (double *)NULL;
			destination->d = (double *)NULL;
			destination->v = (double *)NULL;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_eigenvalues_copy_type_specific.  "
				"Unable to allocate memory");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_eigenvalues_copy_type_specific.  Invalid argument(s)");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_eigenvalues_copy_type_specific */

static int Computed_field_eigenvalues_clear_cache_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_eigenvalues_type_specific_data *data;

	ENTER(Computed_field_eigenvalues_clear_cache_type_specific);
	if (field && (data = 
		(struct Computed_field_eigenvalues_type_specific_data *)
		field->type_specific_data))
	{
		if (data->a)
		{
			DEALLOCATE(data->a);
		}
		if (data->d)
		{
			DEALLOCATE(data->d);
		}
		if (data->v)
		{
			DEALLOCATE(data->v);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_eigenvalues_clear_cache_type_specific.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_eigenvalues_clear_cache_type_specific */

static int Computed_field_eigenvalues_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_eigenvalues_type_specific_contents_match);
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
} /* Computed_field_eigenvalues_type_specific_contents_match */

#define Computed_field_eigenvalues_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_eigenvalues_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_eigenvalues_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_eigenvalues_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_evaluate_eigenvalues(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Evaluates the eigenvalues and eigenvectors of the source field of <field> in
double precision in the type_specific_data then copies the eigenvalues to the
field->values.
==============================================================================*/
{
	int i, matrix_size, n, nrot, return_code;
	struct Computed_field *source_field;
	struct Computed_field_eigenvalues_type_specific_data *data;
	
	ENTER(Computed_field_evaluate_eigenvalues);
	if (field && (field->type_string == computed_field_eigenvalues_type_string) &&
		(data = (struct Computed_field_eigenvalues_type_specific_data *)
			field->type_specific_data))
	{
		n = field->number_of_components;
		matrix_size = n * n;
		if ((data->a || ALLOCATE(data->a, double, matrix_size)) &&
			(data->d || ALLOCATE(data->d, double, n)) &&
			(data->v || ALLOCATE(data->v, double, matrix_size)))
		{
			source_field = field->source_fields[0];
			for (i = 0; i < matrix_size; i++)
			{
				data->a[i] = (double)(source_field->values[i]);
			}
			if (!matrix_is_symmetric(n, data->a, 1.0E-6))
			{
				display_message(WARNING_MESSAGE,
					"Eigenanalysis of field %s may be wrong as matrix not symmetric",
					source_field->name);
			}
			if (Jacobi_eigenanalysis(n, data->a, data->d, data->v, &nrot))
			{
				/* d now contains the eigenvalues, v the eigenvectors in columns, while
					 values of a above the main diagonal are destroyed */
				/* copy the eigenvalues into the field->values */
				for (i = 0; i < n; i++)
				{
					field->values[i] = (FE_value)(data->d[i]);
				}
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_evaluate_eigenvalues.  Eigenanalysis failed");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_evaluate_eigenvalues.  Could not allocate cache");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_eigenvalues.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_evaluate_eigenvalues */

static int Computed_field_eigenvalues_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_eigenvalues_evaluate_cache_at_node);
	if (field && node)
	{
		return_code =
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time) &&
			Computed_field_evaluate_eigenvalues(field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_eigenvalues_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_eigenvalues_evaluate_cache_at_node */

static int Computed_field_eigenvalues_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Evaluate the fields cache in the element.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_eigenvalues_evaluate_cache_in_element);
	if (field && element && xi)
	{
		if (0 == calculate_derivatives)
		{
			field->derivatives_valid = 0;
			return_code = Computed_field_evaluate_source_fields_cache_in_element(
				field, element, xi, time, top_level_element, calculate_derivatives)
				&& Computed_field_evaluate_eigenvalues(field);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_eigenvalues_evaluate_cache_in_element.  "
				"Cannot calculate derivatives of eigenvalues");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_eigenvalues_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_eigenvalues_evaluate_cache_in_element */

#define Computed_field_eigenvalues_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_eigenvalues_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_eigenvalues_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_eigenvalues_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_eigenvalues_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_eigenvalues_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_eigenvalues(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_eigenvalues);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    source field : %s\n",
			field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_eigenvalues.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_eigenvalues */

static char *Computed_field_eigenvalues_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_eigenvalues_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_eigenvalues_type_string, &error);
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
			"Computed_field_eigenvalues_get_command_string.  "
			"Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_eigenvalues_get_command_string */

#define Computed_field_eigenvalues_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_eigenvalues(struct Computed_field *field,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> to type 'eigenvalues' which performs a full eigenanalysis on
the symmetric matrix in <source_field> and returns the n eigenvalues, where
<source_field> must have n x n components.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields, return_code;
	struct Computed_field **source_fields;
	struct Computed_field_eigenvalues_type_specific_data *data;

	ENTER(Computed_field_set_type_eigenvalues);
	if (field && source_field)
	{
		if (Computed_field_is_square_matrix(source_field, (void *)NULL))
		{
			/* 1. make dynamic allocations for any new type-specific data */
			number_of_source_fields = 1;
			if (ALLOCATE(source_fields,struct Computed_field *,
				number_of_source_fields) && ALLOCATE(data,
					struct Computed_field_eigenvalues_type_specific_data,1))
			{
				/* 2. free current type-specific data */
				Computed_field_clear_type(field);
				/* 3. establish the new type */
				field->type_string = computed_field_eigenvalues_type_string;
				field->number_of_components =
					Computed_field_get_square_matrix_size(source_field);
				source_fields[0] = ACCESS(Computed_field)(source_field);
				field->source_fields = source_fields;
				field->number_of_source_fields = number_of_source_fields;
				data->a = (double *)NULL;
				data->d = (double *)NULL;
				data->v = (double *)NULL;
				field->type_specific_data = (void *)data;

				/* Set all the methods */
				COMPUTED_FIELD_ESTABLISH_METHODS(eigenvalues);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_eigenvalues.  Not enough memory");
				DEALLOCATE(source_fields);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Computed_field_set_type_eigenvalues.  "
				"Field %s cannot hold a square matrix",source_field->name);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_eigenvalues.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_eigenvalues */

int Computed_field_get_type_eigenvalues(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
If the field is of type 'eigenvalues', the <source_field> it calculates the
eigenvalues of is returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_eigenvalues);
	if (field && (field->type_string == computed_field_eigenvalues_type_string) &&
		source_field)
	{
		*source_field = field->source_fields[0];
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_eigenvalues.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_eigenvalues */

static int define_Computed_field_type_eigenvalues(struct Parse_state *state,
	void *field_void, void *computed_field_matrix_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 11 December 2000

DESCRIPTION :
Converts <field> into type 'eigenvalues' (if it is not already) and allows its
contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field, *source_field;
	struct Computed_field_matrix_operations_package 
		*computed_field_matrix_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_eigenvalues);
	if (state && (field = (struct Computed_field *)field_void) &&
		(computed_field_matrix_operations_package=
			(struct Computed_field_matrix_operations_package *)
			computed_field_matrix_operations_package_void))
	{
		return_code=1;
		source_field = (struct Computed_field *)NULL;
		if (computed_field_eigenvalues_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_eigenvalues(field, &source_field);
		}
		if (return_code)
		{
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			option_table = CREATE(Option_table)();
			set_source_field_data.conditional_function =
				Computed_field_is_square_matrix;
			set_source_field_data.conditional_function_user_data = (void *)NULL;
			set_source_field_data.computed_field_manager =
				computed_field_matrix_operations_package->computed_field_manager;
			Option_table_add_entry(option_table, "field", &source_field,
				&set_source_field_data, set_Computed_field_conditional);
			return_code = Option_table_multi_parse(option_table, state) &&
				Computed_field_set_type_eigenvalues(field, source_field);
			DESTROY(Option_table)(&option_table);
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_eigenvalues.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_eigenvalues */

static char computed_field_eigenvectors_type_string[] = "eigenvectors";

int Computed_field_is_type_eigenvectors(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_eigenvectors);
	if (field)
	{
		return_code =
			(field->type_string == computed_field_eigenvectors_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_eigenvectors.  Missing field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_eigenvectors */

#define Computed_field_eigenvectors_clear_type_specific \
   Computed_field_default_clear_type_specific
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_eigenvectors_copy_type_specific \
   Computed_field_default_copy_type_specific
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_eigenvectors_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

#define Computed_field_eigenvectors_type_specific_contents_match \
   Computed_field_default_type_specific_contents_match
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
No type specific data
==============================================================================*/

#define Computed_field_eigenvectors_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_eigenvectors_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_eigenvectors_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_eigenvectors_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_evaluate_eigenvectors(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Extracts the eigenvectors out of the source eigenvalues field.
Note the source field should already have been evaluated.
==============================================================================*/
{
	double *v;
	int i, j, n, return_code;
	struct Computed_field *source_field;
	struct Computed_field_eigenvalues_type_specific_data *data;
	
	ENTER(Computed_field_evaluate_eigenvectors);
	if (field && (field->type_string == computed_field_eigenvectors_type_string))
	{
		source_field = field->source_fields[0];
		if (Computed_field_is_type_eigenvalues(source_field) &&
			(data = (struct Computed_field_eigenvalues_type_specific_data *)
				source_field->type_specific_data))
		{
			if (v = data->v)
			{
				n = source_field->number_of_components;
				/* return the vectors across the rows of the field values */
				for (i = 0; i < n; i++)
				{
					for (j = 0; j < n; j++)
					{
						field->values[i*n + j] = (FE_value)(v[j*n + i]);
					}
				}
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_evaluate_eigenvectors.  Missing eigenvalues cache");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Computed_field_evaluate_eigenvectors.  "
				"Source field is not an eigenvalues field");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_eigenvectors.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_evaluate_eigenvectors */

static int Computed_field_eigenvectors_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_eigenvectors_evaluate_cache_at_node);
	if (field && node)
	{
		return_code =
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time) &&
			Computed_field_evaluate_eigenvectors(field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_eigenvectors_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_eigenvectors_evaluate_cache_at_node */

static int Computed_field_eigenvectors_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_eigenvectors_evaluate_cache_in_element);
	if (field && element && xi)
	{
		if (0 == calculate_derivatives)
		{
			field->derivatives_valid = 0;
			return_code = 
				Computed_field_evaluate_source_fields_cache_in_element(field, element,
					xi, time, top_level_element, calculate_derivatives) &&
				Computed_field_evaluate_eigenvectors(field);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_eigenvectors_evaluate_cache_in_element.  "
				"Cannot calculate derivatives of eigenvectors");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_eigenvectors_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_eigenvectors_evaluate_cache_in_element */

#define Computed_field_eigenvectors_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_eigenvectors_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_eigenvectors_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_eigenvectors_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_eigenvectors_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_eigenvectors_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_eigenvectors(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_eigenvectors);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    eigenvalues field : %s\n",field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_eigenvectors.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_eigenvectors */

static char *Computed_field_eigenvectors_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_eigenvectors_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_eigenvectors_type_string, &error);
		append_string(&command_string, " eigenvalues ", &error);
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
			"Computed_field_eigenvectors_get_command_string.  "
			"Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_eigenvectors_get_command_string */

#define Computed_field_eigenvectors_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_eigenvectors(struct Computed_field *field,
	struct Computed_field *eigenvalues_field)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> to type 'eigenvectors' extracting the eigenvectors out of the
source <eigenvalues_field>. Sets the number of components equal to n x n, where
n is the number of components in the <eigenvalues_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int n, number_of_source_fields, return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_eigenvectors);
	if (field && eigenvalues_field)
	{
		if (Computed_field_is_type_eigenvalues(eigenvalues_field))
		{
			/* 1. make dynamic allocations for any new type-specific data */
			number_of_source_fields = 1;
			if (ALLOCATE(source_fields, struct Computed_field *,
				number_of_source_fields))
			{
				/* 2. free current type-specific data */
				Computed_field_clear_type(field);
				/* 3. establish the new type */
				field->type_string = computed_field_eigenvectors_type_string;
				n = eigenvalues_field->number_of_components;
				field->number_of_components = n * n;
				source_fields[0] = ACCESS(Computed_field)(eigenvalues_field);
				field->source_fields = source_fields;
				field->number_of_source_fields = number_of_source_fields;			
				field->type_specific_data = (void *)1;

				/* Set all the methods */
				COMPUTED_FIELD_ESTABLISH_METHODS(eigenvectors);
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_eigenvectors.  Not enough memory");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Computed_field_set_type_eigenvectors.  "
				"Must be given an eigenvalues source field");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_eigenvectors.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_eigenvectors */

int Computed_field_get_type_eigenvectors(struct Computed_field *field,
	struct Computed_field **eigenvalues_field)
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
If the field is of type 'eigenvectors', the <eigenvalues_field> used by it is
returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_eigenvectors);
	if (field && (field->type_string == computed_field_eigenvectors_type_string) &&
		eigenvalues_field)
	{
		*eigenvalues_field = field->source_fields[0];
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_eigenvectors.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_eigenvectors */

static int define_Computed_field_type_eigenvectors(struct Parse_state *state,
	void *field_void,void *computed_field_matrix_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Converts <field> into type 'eigenvectors' (if it is not  already) and allows
its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field, *source_field;
	struct Computed_field_matrix_operations_package 
		*computed_field_matrix_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_eigenvectors);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_matrix_operations_package=
		(struct Computed_field_matrix_operations_package *)
		computed_field_matrix_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		if (computed_field_eigenvectors_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code=Computed_field_get_type_eigenvectors(field, &source_field);
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
			set_source_field_data.computed_field_manager =
				computed_field_matrix_operations_package->computed_field_manager;
			set_source_field_data.conditional_function =
				Computed_field_is_type_eigenvalues_conditional;
			set_source_field_data.conditional_function_user_data = (void *)NULL;
			Option_table_add_entry(option_table, "eigenvalues", &source_field,
				&set_source_field_data, set_Computed_field_conditional);
			return_code = Option_table_multi_parse(option_table,state) &&
				Computed_field_set_type_eigenvectors(field, source_field);
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
			"define_Computed_field_type_eigenvectors.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_eigenvectors */

struct Computed_field_matrix_invert_type_specific_data
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
==============================================================================*/
{
	/* cache for LU decomposed matrix, RHS vector and pivots */
	double *a, *b;
	int *indx;
}; /* struct Computed_field_matrix_invert_type_specific_data */

static char computed_field_matrix_invert_type_string[] = "matrix_invert";

int Computed_field_is_type_matrix_invert(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 11 December 2000

DESCRIPTION :
Returns true if <field> has the appropriate static type string.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_matrix_invert);
	if (field)
	{
		return_code =
			(field->type_string == computed_field_matrix_invert_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_matrix_invert.  Missing field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_matrix_invert */

static int Computed_field_matrix_invert_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 11 December 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_matrix_invert_type_specific_data *data;

	ENTER(Computed_field_matrix_invert_clear_type_specific);
	if (field && (data =
		(struct Computed_field_matrix_invert_type_specific_data *)
		field->type_specific_data))
	{
		if (data->a)
		{
			DEALLOCATE(data->a);
		}
		if (data->b)
		{
			DEALLOCATE(data->b);
		}
		if (data->indx)
		{
			DEALLOCATE(data->indx);
		}
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_matrix_invert_clear_type_specific.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_matrix_invert_clear_type_specific */

static void *Computed_field_matrix_invert_copy_type_specific(
	struct Computed_field *source_field, struct Computed_field *destination_field)
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_matrix_invert_type_specific_data *destination;

	ENTER(Computed_field_matrix_invert_copy_type_specific);
	if (source_field && destination_field)
	{
		if (ALLOCATE(destination,
			struct Computed_field_matrix_invert_type_specific_data, 1))
		{
			/* following arrays are allocated when field calculated, cleared when
				 cache cleared */
			destination->a = (double *)NULL;
			destination->b = (double *)NULL;
			destination->indx = (int *)NULL;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_eigenvalues_copy_type_specific.  "
				"Unable to allocate memory");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_matrix_invert_copy_type_specific.  Invalid argument(s)");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_matrix_invert_copy_type_specific */

static int Computed_field_matrix_invert_clear_cache_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_matrix_invert_type_specific_data *data;

	ENTER(Computed_field_matrix_invert_clear_cache_type_specific);
	if (field && (data = 
		(struct Computed_field_matrix_invert_type_specific_data *)
		field->type_specific_data))
	{
		if (data->a)
		{
			DEALLOCATE(data->a);
		}
		if (data->b)
		{
			DEALLOCATE(data->b);
		}
		if (data->indx)
		{
			DEALLOCATE(data->indx);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_matrix_invert_clear_cache_type_specific.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_matrix_invert_clear_cache_type_specific */

static int Computed_field_matrix_invert_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 11 December 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_matrix_invert_type_specific_contents_match);
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
} /* Computed_field_matrix_invert_type_specific_contents_match */

#define Computed_field_matrix_invert_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 11 December 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_matrix_invert_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 11 December 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_matrix_invert_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 11 December 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_matrix_invert_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 11 December 2000

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_evaluate_matrix_invert(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 11 December 2000

DESCRIPTION :
Evaluates the inverse of the matrix held in the <source_field>. The cache is
used to store intermediate LU-decomposed matrix and RHS vector in double
precision, as well as the integer pivot indx. Expects that source_field has
been pre-calculated before calling this.
==============================================================================*/
{
	double d;
	int i, j, matrix_size, n, return_code;
	struct Computed_field *source_field;
	struct Computed_field_matrix_invert_type_specific_data *data;
	
	ENTER(Computed_field_evaluate_matrix_invert);
	if (field && (field->type_string == computed_field_matrix_invert_type_string) &&
		(data = (struct Computed_field_matrix_invert_type_specific_data *)
			field->type_specific_data))
	{
		source_field = field->source_fields[0];
		n = Computed_field_get_square_matrix_size(source_field);
		matrix_size = n * n;
		if ((data->a || ALLOCATE(data->a, double, matrix_size)) &&
			(data->b || ALLOCATE(data->b, double, n)) &&
			(data->indx || ALLOCATE(data->indx, int, n)))
		{
			for (i = 0; i < matrix_size; i++)
			{
				data->a[i] = (double)(source_field->values[i]);
			}
			if (LU_decompose(n, data->a, data->indx, &d))
			{
				return_code = 1;
				for (i = 0; (i < n) && return_code; i++)
				{
					/* take a column of the identity matrix */
					for (j = 0; j < n; j++)
					{
						data->b[j] = 0.0;
					}
					data->b[i] = 1.0;
					if (LU_backsubstitute(n, data->a, data->indx, data->b))
					{
						/* extract a column of the inverse matrix */
						for (j = 0; j < n; j++)
						{
							field->values[j*n + i] = (FE_value)data->b[j];
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_evaluate_matrix_invert.  "
							"Could not LU backsubstitute matrix");
						return_code = 0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_evaluate_matrix_invert.  "
					"Could not LU decompose matrix");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_evaluate_matrix_invert.  Could not allocate cache");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_matrix_invert.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_evaluate_matrix_invert */

static int Computed_field_matrix_invert_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 11 December 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_matrix_invert_evaluate_cache_at_node);
	if (field && node)
	{
		return_code =
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time) &&
			Computed_field_evaluate_matrix_invert(field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_matrix_invert_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_matrix_invert_evaluate_cache_at_node */

static int Computed_field_matrix_invert_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time,struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 11 December 2000

DESCRIPTION :
Evaluate the fields cache in the element.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_matrix_invert_evaluate_cache_in_element);
	if (field && element && xi)
	{
		if (0 == calculate_derivatives)
		{
			field->derivatives_valid = 0;
			return_code = Computed_field_evaluate_source_fields_cache_in_element(
				field, element, xi, time, top_level_element, calculate_derivatives)
				&& Computed_field_evaluate_matrix_invert(field);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_matrix_invert_evaluate_cache_in_element.  "
				"Cannot calculate derivatives of matrix_invert");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_matrix_invert_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_matrix_invert_evaluate_cache_in_element */

#define Computed_field_matrix_invert_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 11 December 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_matrix_invert_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 11 December 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_matrix_invert_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 11 December 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_matrix_invert_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 11 December 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_matrix_invert_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 11 December 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_matrix_invert_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 11 December 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_matrix_invert(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 11 December 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_matrix_invert);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    source field : %s\n",
			field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_matrix_invert.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_matrix_invert */

static char *Computed_field_matrix_invert_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_matrix_invert_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_matrix_invert_type_string, &error);
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
			"Computed_field_matrix_invert_get_command_string.  "
			"Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_matrix_invert_get_command_string */

#define Computed_field_matrix_invert_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_matrix_invert(struct Computed_field *field,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> to type 'matrix_invert' which performs a full eigenanalysis on
the symmetric matrix in <source_field> and returns the n matrix_invert, where
<source_field> must have n x n components.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields, return_code;
	struct Computed_field **source_fields;
	struct Computed_field_matrix_invert_type_specific_data *data;

	ENTER(Computed_field_set_type_matrix_invert);
	if (field && source_field)
	{
		if (Computed_field_is_square_matrix(source_field, (void *)NULL))
		{
			/* 1. make dynamic allocations for any new type-specific data */
			number_of_source_fields = 1;
			if (ALLOCATE(source_fields,struct Computed_field *,
				number_of_source_fields) && ALLOCATE(data,
					struct Computed_field_matrix_invert_type_specific_data, 1))
			{
				/* 2. free current type-specific data */
				Computed_field_clear_type(field);
				/* 3. establish the new type */
				field->type_string = computed_field_matrix_invert_type_string;
				field->number_of_components =
					Computed_field_get_number_of_components(source_field);
				source_fields[0] = ACCESS(Computed_field)(source_field);
				field->source_fields = source_fields;
				field->number_of_source_fields = number_of_source_fields;
				field->type_specific_data = (void *)1;
				data->a = (double *)NULL;
				data->b = (double *)NULL;
				data->indx = (int *)NULL;
				field->type_specific_data = (void *)data;

				/* Set all the methods */
				COMPUTED_FIELD_ESTABLISH_METHODS(matrix_invert);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_matrix_invert.  Not enough memory");
				DEALLOCATE(source_fields);
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Computed_field_set_type_matrix_invert.  "
				"Field %s cannot hold a square matrix", source_field->name);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_matrix_invert.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_matrix_invert */

int Computed_field_get_type_matrix_invert(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 11 December 2000

DESCRIPTION :
If the field is of type 'matrix_invert', the <source_field> it calculates the
matrix_invert of is returned.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_matrix_invert);
	if (field && (field->type_string == computed_field_matrix_invert_type_string) &&
		source_field)
	{
		*source_field = field->source_fields[0];
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_matrix_invert.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_matrix_invert */

static int define_Computed_field_type_matrix_invert(struct Parse_state *state,
	void *field_void, void *computed_field_matrix_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 11 December 2000

DESCRIPTION :
Converts <field> into type 'matrix_invert' (if it is not already) and allows its
contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field, *source_field;
	struct Computed_field_matrix_operations_package 
		*computed_field_matrix_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_matrix_invert);
	if (state && (field = (struct Computed_field *)field_void) &&
		(computed_field_matrix_operations_package=
			(struct Computed_field_matrix_operations_package *)
			computed_field_matrix_operations_package_void))
	{
		return_code = 1;
		source_field = (struct Computed_field *)NULL;
		if (computed_field_matrix_invert_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_matrix_invert(field, &source_field);
		}
		if (return_code)
		{
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			option_table = CREATE(Option_table)();
			set_source_field_data.conditional_function =
				Computed_field_is_square_matrix;
			set_source_field_data.conditional_function_user_data = (void *)NULL;
			set_source_field_data.computed_field_manager =
				computed_field_matrix_operations_package->computed_field_manager;
			Option_table_add_entry(option_table, "field", &source_field,
				&set_source_field_data, set_Computed_field_conditional);
			return_code = Option_table_multi_parse(option_table, state) &&
				Computed_field_set_type_matrix_invert(field, source_field);
			DESTROY(Option_table)(&option_table);
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_matrix_invert.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_matrix_invert */

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
			"Computed_field_is_type_matrix_multiply.  Missing field");
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
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_matrix_multiply_clear_type_specific */

static void *Computed_field_matrix_multiply_copy_type_specific(
	struct Computed_field *source_field, struct Computed_field *destination_field)
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_matrix_multiply_type_specific_data *destination,
		*source;

	ENTER(Computed_field_matrix_multiply_copy_type_specific);
	if (source_field && destination_field && (source = 
		(struct Computed_field_matrix_multiply_type_specific_data *)
		source_field->type_specific_data))
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
				"Unable to allocate memory");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_matrix_multiply_copy_type_specific.  Invalid argument(s)");
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
		if (data->number_of_rows == other_data->number_of_rows)
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

#define Computed_field_matrix_multiply_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 26 September 2000

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_matrix_multiply_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
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
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
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
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_matrix_multiply_evaluate_cache_at_node */

static int Computed_field_matrix_multiply_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
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
				xi, time, top_level_element, calculate_derivatives))
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
			"Invalid argument(s)");
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
			"list_Computed_field_matrix_multiply.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_matrix_multiply */

static char *Computed_field_matrix_multiply_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error;
	struct Computed_field_matrix_multiply_type_specific_data *data;

	ENTER(Computed_field_matrix_multiply_get_command_string);
	command_string = (char *)NULL;
	if (field && (data = 
		(struct Computed_field_matrix_multiply_type_specific_data *)
		field->type_specific_data))
	{
		error = 0;
		append_string(&command_string,
			computed_field_matrix_multiply_type_string, &error);
		sprintf(temp_string, " number_of_rows %d", data->number_of_rows);
		append_string(&command_string, temp_string, &error);
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
			"Computed_field_matrix_multiply_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_matrix_multiply_get_command_string */

#define Computed_field_matrix_multiply_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_matrix_multiply(struct Computed_field *field,
	int number_of_rows,struct Computed_field *source_field1,
	struct Computed_field *source_field2)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

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
					field->type_string = computed_field_matrix_multiply_type_string;
					field->number_of_components = number_of_rows*n;
					temp_source_fields[0]=ACCESS(Computed_field)(source_field1);
					temp_source_fields[1]=ACCESS(Computed_field)(source_field2);
					field->source_fields=temp_source_fields;
					field->number_of_source_fields=number_of_source_fields;
					data->number_of_rows=number_of_rows;
					field->type_specific_data = (void *)data;

					/* Set all the methods */
					COMPUTED_FIELD_ESTABLISH_METHODS(matrix_multiply);
					return_code=1;
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
	if (field && (field->type_string == computed_field_matrix_multiply_type_string) &&
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
			if (return_code)
			{
				/* ACCESS the source fields for set_Computed_field_array */
				for (i=0;i<2;i++)
				{
					if (source_fields[i])
					{
						ACCESS(Computed_field)(source_fields[i]);
					}
				}
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
						/* if the "number_of_rows" token is next, read it */
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
					display_message(ERROR_MESSAGE, "Missing command options");
					return_code = 0;
				}
				for (i=0;i<2;i++)
				{
					if (source_fields[i])
					{
						DEACCESS(Computed_field)(&(source_fields[i]));
					}
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
			"Computed_field_is_type_projection.  Missing field");
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
			"Computed_field_projection_clear_type_specific.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_projection_clear_type_specific */

static void *Computed_field_projection_copy_type_specific(
	struct Computed_field *source_field, struct Computed_field *destination_field)
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	int number_of_projection_values;
	struct Computed_field_projection_type_specific_data *destination,
		*source;

	ENTER(Computed_field_projection_copy_type_specific);
	if (source_field && destination_field && (source = 
		(struct Computed_field_projection_type_specific_data *)
		source_field->type_specific_data))
	{
		number_of_projection_values =
			(source_field->source_fields[0]->number_of_components + 1) *
			(source_field->number_of_components + 1);
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
				"Computed_field_projection_copy_type_specific.  Not enough memory");
			DEALLOCATE(destination);
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_projection_copy_type_specific.  Invalid argument(s)");
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

#define Computed_field_projection_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 27 September 2000

DESCRIPTION :
No special criteria..
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
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

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
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			return_code=Computed_field_evaluate_projection_matrix(field,
				/*element_dimension*/0, /*calculate_derivatives*/0);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_projection_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_projection_evaluate_cache_at_node */

static int Computed_field_projection_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time,struct FE_element *top_level_element,int calculate_derivatives)
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
				xi, time, top_level_element, calculate_derivatives))
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
			"Invalid argument(s)");
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
			"list_Computed_field_projection.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_projection */

static char *Computed_field_projection_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error, i, number_of_projection_values;
	struct Computed_field_projection_type_specific_data *data;

	ENTER(Computed_field_projection_get_command_string);
	command_string = (char *)NULL;
	if (field && (data = 
		(struct Computed_field_projection_type_specific_data *)
		field->type_specific_data))
	{
		error = 0;
		append_string(&command_string,
			computed_field_projection_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		sprintf(temp_string, " number_of_components %d",
			field->number_of_components);
		append_string(&command_string, temp_string, &error);
		append_string(&command_string, " projection_matrix", &error);
		number_of_projection_values =
			(field->source_fields[0]->number_of_components + 1)
			* (field->number_of_components + 1);
		for (i = 0; i < number_of_projection_values; i++)
		{
			sprintf(temp_string, " %g", data->projection_matrix[i]);
			append_string(&command_string, temp_string, &error);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_projection_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_projection_get_command_string */

#define Computed_field_projection_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_projection(struct Computed_field *field,
	struct Computed_field *source_field, int number_of_components, 
	double *projection_matrix)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_PROJECTION, returning the <source_field>
with each component multiplied by the perspective <projection_matrix>.
The <projection_matrix> array must be of size
<source_field->number_of_components + 1> * <field->number_of_components + 1>.
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
			field->type_string = computed_field_projection_type_string;
			field->number_of_components=number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			memcpy(data->projection_matrix,projection_matrix,
				number_of_projection_values*sizeof(double));
			field->type_specific_data = data;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(projection);
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
	if (field && (field->type_string == computed_field_projection_type_string) &&
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
LAST MODIFIED : 18 December 2001

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_PROJECTION (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	double *projection_matrix, *temp_projection_matrix;
	int i, number_of_components, number_of_projection_values, return_code,
		temp_number_of_projection_values;
	struct Computed_field *field, *source_field;
	struct Computed_field_matrix_operations_package 
		*computed_field_matrix_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_projection);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_matrix_operations_package=
			(struct Computed_field_matrix_operations_package *)
			computed_field_matrix_operations_package_void))
	{
		return_code = 1;
		set_source_field_data.conditional_function =
			(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		set_source_field_data.computed_field_manager =
			computed_field_matrix_operations_package->computed_field_manager;
		source_field = (struct Computed_field *)NULL;
		projection_matrix = (double *)NULL;
		if (computed_field_projection_type_string == field->type_string)
		{
			if (return_code = Computed_field_get_type_projection(field,
				&source_field, &number_of_components, &projection_matrix))
			{
				number_of_projection_values = (source_field->number_of_components + 1)
					* (number_of_components + 1);
			}
		}
		else
		{
			/* ALLOCATE and fill array of projection_matrix - with identity */
			number_of_components = 0;
			number_of_projection_values = 1;
			if (ALLOCATE(projection_matrix, double, 1))
			{
				projection_matrix[0] = 0.0;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_projection.  Not enough memory");
				return_code = 0;
			}
		}
		if (return_code)
		{
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}

			/* try to handle help first */
			if ((current_token = state->current_token) &&
				(!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
				option_table = CREATE(Option_table)();					
				Option_table_add_entry(option_table, "field", &source_field,
					&set_source_field_data, set_Computed_field_conditional);
				Option_table_add_entry(option_table, "number_of_components",
					&number_of_components, NULL, set_int_positive);
				Option_table_add_entry(option_table, "projection_matrix",
					projection_matrix, &number_of_projection_values,
					set_double_vector);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}

			/* keep the number_of_projection_values to maintain any current ones */
			temp_number_of_projection_values = number_of_projection_values;
			/* parse the field... */
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
						if (!source_field)
						{
							display_message(ERROR_MESSAGE,
								"define_Computed_field_type_projection.  No field");
							return_code=0;
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}

			/* parse the number_of_components... */
			if (return_code && (current_token = state->current_token))
			{
				/* ... only if the "number_of_components" token is next */
				if (fuzzy_string_compare(current_token, "number_of_components"))
				{
					option_table = CREATE(Option_table)();					
					Option_table_add_entry(option_table, "number_of_components",
						&number_of_components, NULL, set_int_positive);
					return_code = Option_table_parse(option_table, state);
					DESTROY(Option_table)(&option_table);
				}
			}

			/* ensure projection matrix is correct size; set new values to zero */
			if (return_code)
			{
				number_of_projection_values = (source_field->number_of_components + 1)
					* (number_of_components + 1);
				if (temp_number_of_projection_values != number_of_projection_values)
				{
					if (REALLOCATE(temp_projection_matrix, projection_matrix, double,
						number_of_projection_values))
					{
						projection_matrix = temp_projection_matrix;
						/* clear any new projection_matrix to zero */
						for (i = temp_number_of_projection_values;
							i < number_of_projection_values; i++)
						{
							projection_matrix[i] = 0.0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_projection.  "
							"Could not reallocate projection matrix");
						return_code = 0;
					}
				}
			}

			/* parse the projection_matrix */
			if (return_code && state->current_token)
			{
				option_table = CREATE(Option_table)();					
				Option_table_add_entry(option_table, "projection_matrix",
					projection_matrix, &number_of_projection_values,
					set_double_vector);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			if (return_code)
			{
				return_code = Computed_field_set_type_projection(field, source_field,
					number_of_components, projection_matrix);
			}
			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token) &&
					strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_projection.  Failed");
				}
			}
			/* clean up the projection_matrix array */
			DEALLOCATE(projection_matrix);
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_projection.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_projection */

struct Computed_field_transpose_type_specific_data
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
==============================================================================*/
{
	/* the number of rows in the source [matrix] field */
	int source_number_of_rows;
}; /* struct Computed_field_transpose_type_specific_data */

static char computed_field_transpose_type_string[] = "transpose";

int Computed_field_is_type_transpose(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Returns true if <field> has the appropriate static type string.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_transpose);
	if (field)
	{
		return_code =
			(field->type_string == computed_field_transpose_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_transpose.  Missing field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_transpose */

static int Computed_field_transpose_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_transpose_clear_type_specific);
	if (field && field->type_specific_data)
	{
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_transpose_clear_type_specific.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_transpose_clear_type_specific */

static void *Computed_field_transpose_copy_type_specific(
	struct Computed_field *source_field, struct Computed_field *destination_field)
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_transpose_type_specific_data *destination,*source;

	ENTER(Computed_field_transpose_copy_type_specific);
	if (source_field && destination_field && (source = 
		(struct Computed_field_transpose_type_specific_data *)
		source_field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_transpose_type_specific_data, 1))
		{
			destination->source_number_of_rows = source->source_number_of_rows;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_transpose_copy_type_specific.  "
				"Unable to allocate memory");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_transpose_copy_type_specific.  Invalid argument(s)");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_transpose_copy_type_specific */

#define Computed_field_transpose_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_transpose_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;
	struct Computed_field_transpose_type_specific_data *data,
		*other_data;

	ENTER(Computed_field_transpose_type_specific_contents_match);
	if (field && other_computed_field && (data = 
		(struct Computed_field_transpose_type_specific_data *)
		field->type_specific_data) && (other_data =
		(struct Computed_field_transpose_type_specific_data *)
		other_computed_field->type_specific_data))
	{
		if (data->source_number_of_rows == other_data->source_number_of_rows)
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
} /* Computed_field_transpose_type_specific_contents_match */

#define Computed_field_transpose_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_transpose_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_transpose_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_transpose_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Computed_field_transpose_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value *source_values;
	int i, j, m, n, return_code;
	struct Computed_field_transpose_type_specific_data *data;

	ENTER(Computed_field_transpose_evaluate_cache_at_node);
	if (field && node && (data =
		(struct Computed_field_transpose_type_specific_data *)
		field->type_specific_data) && (m = data->source_number_of_rows) &&
		(n = (field->source_fields[0]->number_of_components / m)))
	{
		/* returns n row x m column transpose of m row x n column source field,
			 where values always change along rows fastest */
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node, time))
		{
			source_values = field->source_fields[0]->values;
			for (i = 0; i < n; i++)
			{
				for (j = 0; j < m; j++)
				{
					field->values[i*m + j] = source_values[j*n + i];
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_transpose_evaluate_cache_at_node.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_transpose_evaluate_cache_at_node */

static int Computed_field_transpose_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time,struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Evaluate the fields cache in the element.
==============================================================================*/
{
	FE_value *destination_derivatives, *source_derivatives, *source_values;
	int d, element_dimension, i, j, m, n, return_code;
	struct Computed_field_transpose_type_specific_data *data;

	ENTER(Computed_field_transpose_evaluate_cache_in_element);
	if (field && element && xi && (data =
		(struct Computed_field_transpose_type_specific_data *)
		field->type_specific_data) && (m = data->source_number_of_rows) &&
		(n = (field->source_fields[0]->number_of_components / m)))
	{
		/* returns n row x m column tranpose of m row x n column source field,
			 where values always change along rows fastest */
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = Computed_field_evaluate_source_fields_cache_in_element(
			field, element, xi, time, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			source_values = field->source_fields[0]->values;
			for (i = 0; i < n; i++)
			{
				for (j = 0; j < m; j++)
				{
					field->values[i*m + j] = source_values[j*n + i];
				}
			}
			if (calculate_derivatives)
			{
				/* transpose derivatives in same way as values */
				element_dimension = get_FE_element_dimension(element);
				for (i = 0; i < n; i++)
				{
					for (j = 0; j < m; j++)
					{
						source_derivatives = field->source_fields[0]->derivatives +
							element_dimension*(j*n + i);
						destination_derivatives = field->derivatives +
							element_dimension*(i*m + j);
						for (d = 0; d < element_dimension; d++)
						{
							destination_derivatives[d] = source_derivatives[d];
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
			"Computed_field_transpose_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_transpose_evaluate_cache_in_element */

#define Computed_field_transpose_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_transpose_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_transpose_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_transpose_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_transpose_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_transpose_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_transpose(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_transpose_type_specific_data *data;

	ENTER(List_Computed_field_transpose);
	if (field && (data = 
		(struct Computed_field_transpose_type_specific_data *)
		field->type_specific_data))
	{
		display_message(INFORMATION_MESSAGE,
			"    source number of rows : %d\n",data->source_number_of_rows);
		display_message(INFORMATION_MESSAGE,"    source field : %s\n",
			field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_transpose.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_transpose */

static char *Computed_field_transpose_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error;
	struct Computed_field_transpose_type_specific_data *data;

	ENTER(Computed_field_transpose_get_command_string);
	command_string = (char *)NULL;
	if (field && (data = 
		(struct Computed_field_transpose_type_specific_data *)
		field->type_specific_data))
	{
		error = 0;
		append_string(&command_string,
			computed_field_transpose_type_string, &error);
		sprintf(temp_string, " source_number_of_rows %d",
			data->source_number_of_rows);
		append_string(&command_string, temp_string, &error);
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
			"Computed_field_transpose_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_transpose_get_command_string */

#define Computed_field_transpose_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_transpose(struct Computed_field *field,
	int source_number_of_rows,struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_TRANSPOSE which computes the transpose
of the <source_number_of_rows> by source_number_of_columns <source_field>. The
source_number_of_columns is computed as source_field->number_of_components
divided by <source_number_of_rows>, and this division must have no remainder.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields, return_code;
	struct Computed_field **temp_source_fields;
	struct Computed_field_transpose_type_specific_data *data;

	ENTER(Computed_field_set_type_transpose);
	if (field && (0 < source_number_of_rows) && source_field)
	{
		if (0 == (source_field->number_of_components % source_number_of_rows))
		{
			/* 1. make dynamic allocations for any new type-specific data */
			number_of_source_fields=1;
			if (ALLOCATE(temp_source_fields,struct Computed_field *,
				number_of_source_fields) && ALLOCATE(data,
					struct Computed_field_transpose_type_specific_data,1))
			{
				/* 2. free current type-specific data */
				Computed_field_clear_type(field);
				/* 3. establish the new type */
				field->type_string = computed_field_transpose_type_string;
				field->number_of_components = source_field->number_of_components;
				temp_source_fields[0] = ACCESS(Computed_field)(source_field);
				field->source_fields = temp_source_fields;
				field->number_of_source_fields = number_of_source_fields;
				data->source_number_of_rows = source_number_of_rows;
				field->type_specific_data = (void *)data;

				/* Set all the methods */
				COMPUTED_FIELD_ESTABLISH_METHODS(transpose);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_transpose.  Unable to allocate memory");
				DEALLOCATE(temp_source_fields);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Computed_field_set_type_transpose.  "
				"Source field %s has %d components, hence cannot have %d rows",
				source_field->name,source_field->number_of_components,
				source_number_of_rows);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_transpose.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_transpose */

int Computed_field_get_type_transpose(struct Computed_field *field,
	int *source_number_of_rows, struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_TRANSPOSE, the 
<source_number_of_rows> and <source_field> used by it are returned.
==============================================================================*/
{
	int return_code;
	struct Computed_field_transpose_type_specific_data *data;

	ENTER(Computed_field_get_type_transpose);
	if (field && (field->type_string == computed_field_transpose_type_string) &&
		source_field && (data = 
			(struct Computed_field_transpose_type_specific_data *)
			field->type_specific_data))
	{
		*source_number_of_rows = data->source_number_of_rows;
		*source_field = field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_transpose.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_transpose */

static int define_Computed_field_type_transpose(struct Parse_state *state,
	void *field_void,void *computed_field_matrix_operations_package_void)
/*******************************************************************************
LAST MODIFIED : 18 December 2001

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_TRANSPOSE (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	int source_number_of_rows, return_code;
	struct Computed_field *field,*source_field;
	struct Computed_field_matrix_operations_package 
		*computed_field_matrix_operations_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_transpose);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_matrix_operations_package=
		(struct Computed_field_matrix_operations_package *)
		computed_field_matrix_operations_package_void))
	{
		return_code=1;
		/* get valid parameters for transpose field */
		source_number_of_rows = 1;
		source_field = (struct Computed_field *)NULL;
		if (computed_field_transpose_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code=Computed_field_get_type_transpose(field,
				&source_number_of_rows,&source_field);
		}
		if (return_code)
		{
			/* ACCESS the source fields for set_Computed_field_conditional */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			/* try to handle help first */
			if (current_token = state->current_token)
			{
				set_source_field_data.conditional_function =
					Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data = (void *)NULL;
				set_source_field_data.computed_field_manager =
					computed_field_matrix_operations_package->computed_field_manager;
				if (!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
				{
					option_table = CREATE(Option_table)();
					Option_table_add_entry(option_table,"source_number_of_rows",
						&source_number_of_rows,NULL,set_int_positive);
					Option_table_add_entry(option_table,"field",&source_field,
						&set_source_field_data,set_Computed_field_conditional);
					return_code=Option_table_multi_parse(option_table,state);
					DESTROY(Option_table)(&option_table);
				}
				else
				{
					/* if the "source_number_of_rows" token is next, read it */
					if (fuzzy_string_compare(current_token,"source_number_of_rows"))
					{
						option_table = CREATE(Option_table)();
						/* source_number_of_rows */
						Option_table_add_entry(option_table,"source_number_of_rows",
							&source_number_of_rows,NULL,set_int_positive);
						return_code = Option_table_parse(option_table,state);
						DESTROY(Option_table)(&option_table);
					}
					if (return_code)
					{
						option_table = CREATE(Option_table)();
						Option_table_add_entry(option_table,"field",&source_field,
							&set_source_field_data,set_Computed_field_conditional);
						return_code = Option_table_multi_parse(option_table,state) &&
							Computed_field_set_type_transpose(field,
								source_number_of_rows,source_field);
						DESTROY(Option_table)(&option_table);
					}
					if (!return_code)
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_transpose.  Failed");
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "Missing command options");
				return_code = 0;
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
			"define_Computed_field_type_transpose.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_transpose */

int Computed_field_register_types_matrix_operations(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 11 December 2000

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

		return_code =
			Computed_field_package_add_type(computed_field_package,
				computed_field_eigenvalues_type_string,
				define_Computed_field_type_eigenvalues,
				&computed_field_matrix_operations_package) &&
			Computed_field_package_add_type(computed_field_package,
				computed_field_eigenvectors_type_string,
				define_Computed_field_type_eigenvectors,
				&computed_field_matrix_operations_package) &&
			Computed_field_package_add_type(computed_field_package,
				computed_field_matrix_invert_type_string,
				define_Computed_field_type_matrix_invert,
				&computed_field_matrix_operations_package) &&
			Computed_field_package_add_type(computed_field_package,
				computed_field_matrix_multiply_type_string,
				define_Computed_field_type_matrix_multiply,
				&computed_field_matrix_operations_package) &&
			Computed_field_package_add_type(computed_field_package,
				computed_field_projection_type_string,
				define_Computed_field_type_projection,
				&computed_field_matrix_operations_package) &&
			Computed_field_package_add_type(computed_field_package,
				computed_field_transpose_type_string,
				define_Computed_field_type_transpose,
				&computed_field_matrix_operations_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_matrix_operations.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_matrix_operations */

