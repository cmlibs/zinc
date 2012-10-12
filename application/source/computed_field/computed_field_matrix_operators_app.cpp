
#include "api/cmiss_field_matrix_operators.h"
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "computed_field/computed_field_matrix_operators.hpp"

class Computed_field_matrix_operators_package : public Computed_field_type_package
{
};

const char computed_field_determinant_type_string[] = "determinant";

const char computed_field_eigenvalues_type_string[] = "eigenvalues";

const char computed_field_eigenvectors_type_string[] = "eigenvectors";

const char computed_field_matrix_invert_type_string[] = "matrix_invert";

const char computed_field_matrix_multiply_type_string[] = "matrix_multiply";

const char computed_field_projection_type_string[] = "projection";

const char computed_field_transpose_type_string[] = "transpose";

const char computed_field_quaternion_to_matrix_type_string[] = "quaternion_to_matrix";

const char computed_field_matrix_to_quaternion_type_string[] = "matrix_to_quaternion";

int Computed_field_get_type_matrix_to_quaternion(struct Computed_field *field,
	struct Computed_field **matrix_to_quaternion_field);

int Computed_field_get_type_quaternion_to_matrix(struct Computed_field *field,
	struct Computed_field **quaternion_to_matrix_field);

int Computed_field_get_type_transpose(struct Computed_field *field,
	int *source_number_of_rows, struct Computed_field **source_field);

int Computed_field_get_type_projection(struct Computed_field *field,
	struct Computed_field **source_field, struct Computed_field **projection_matrix_field);

int Computed_field_get_type_matrix_multiply(struct Computed_field *field,
	int *number_of_rows, struct Computed_field **source_field1,
	struct Computed_field **source_field2);

int Computed_field_get_type_matrix_invert(struct Computed_field *field,
	struct Computed_field **source_field);

int Computed_field_get_type_eigenvectors(struct Computed_field *field,
	struct Computed_field **eigenvalues_field);

int Computed_field_get_type_eigenvalues(struct Computed_field *field,
	struct Computed_field **source_field);

Computed_field *Computed_field_create_quaternion_to_matrix(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field);

Computed_field *Computed_field_create_quaternion_to_matrix(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field);

Computed_field *Computed_field_create_matrix_to_quaternion(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field);

int Computed_field_is_type_eigenvalues_conditional(struct Computed_field *field,
	void *dummy_void);

int Computed_field_is_square_matrix(struct Computed_field *field,
	void *dummy_void);

/***************************************************************************//**
 * Command modifier function which converts field into type 'determinant'
 * (if it is not already) and allows its contents to be modified.
 */
int define_Computed_field_type_determinant(struct Parse_state *state,
	void *field_modify_void, void *computed_field_matrix_operators_package_void)
{
	int return_code;

	ENTER(define_Computed_field_type_determinant);
	USE_PARAMETER(computed_field_matrix_operators_package_void);
	Computed_field_modify_data *field_modify =
		reinterpret_cast<Computed_field_modify_data*>(field_modify_void);
	if (state && field_modify)
	{
		return_code = 1;
		Cmiss_field_id source_field = 0;
		if (NULL != field_modify->get_field() &&
			(computed_field_determinant_type_string ==
			 Computed_field_get_type_string(field_modify->get_field())))
		{
			source_field = Cmiss_field_get_source_field(field_modify->get_field(), 1);
		}
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"Creates a field returning the scalar real determinant of a square matrix. "
			"Only supports 1, 4 (2x2) and 9 (3x3) component source fields.");
		struct Set_Computed_field_conditional_data set_source_field_data =
		{
			Computed_field_is_square_matrix,
			/*user_data*/0,
			field_modify->get_field_manager()
		};
		Option_table_add_entry(option_table, "field", &source_field,
			&set_source_field_data, set_Computed_field_conditional);
		return_code = Option_table_multi_parse(option_table, state);
		if (return_code)
		{
			return_code = field_modify->update_field_and_deaccess(
				Cmiss_field_module_create_determinant(field_modify->get_field_module(),
					source_field));
		}
		DESTROY(Option_table)(&option_table);
		if (source_field)
		{
			Cmiss_field_destroy(&source_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_determinant.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}


int define_Computed_field_type_eigenvalues(struct Parse_state *state,
	void *field_modify_void, void *computed_field_matrix_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type 'eigenvalues' (if it is not already) and allows its
contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_eigenvalues);
	USE_PARAMETER(computed_field_matrix_operators_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		source_field = (struct Computed_field *)NULL;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_eigenvalues_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code = Computed_field_get_type_eigenvalues(field_modify->get_field(), &source_field);
		}
		if (return_code)
		{
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			option_table = CREATE(Option_table)();
			Option_table_add_help(option_table,
			  "An eigenvalues field returns the n eigenvalues of an (n * n) square matrix field.  Here, a 9 component source field is interpreted as a (3 * 3) matrix with the first 3 components being the first row, the next 3 components being the middle row, and so on.  The related eigenvectors field can extract the corresponding eigenvectors for the eigenvalues. See a/large_strain for an example of using the eigenvalues and eigenvectors fields.");
			set_source_field_data.conditional_function =
				Computed_field_is_square_matrix;
			set_source_field_data.conditional_function_user_data = (void *)NULL;
			set_source_field_data.computed_field_manager =
				field_modify->get_field_manager();
			Option_table_add_entry(option_table, "field", &source_field,
				&set_source_field_data, set_Computed_field_conditional);
			return_code = Option_table_multi_parse(option_table, state);
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Cmiss_field_module_create_eigenvalues(field_modify->get_field_module(),
						source_field));
			}
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


int define_Computed_field_type_eigenvectors(struct Parse_state *state,
	void *field_modify_void,void *computed_field_matrix_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type 'eigenvectors' (if it is not  already) and allows
its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_eigenvectors);
	USE_PARAMETER(computed_field_matrix_operators_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_eigenvectors_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code=Computed_field_get_type_eigenvectors(field_modify->get_field(), &source_field);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			option_table = CREATE(Option_table)();
			Option_table_add_help(option_table,
			  "An eigenvectors field returns vectors corresponding to each eigenvalue from a source eigenvalues field.  For example, if 3 eigenvectors have been computed for a (3 * 3) matrix = 9 component field, the eigenvectors will be a 9 component field with the eigenvector corresponding to the first eigenvalue in the first 3 components, the second eigenvector in the next 3 components, and so on.  See a/large_strain for an example of using the eigenvalues and eigenvectors fields.");
			/* field */
			set_source_field_data.computed_field_manager =
				field_modify->get_field_manager();
			set_source_field_data.conditional_function =
				Computed_field_is_type_eigenvalues_conditional;
			set_source_field_data.conditional_function_user_data = (void *)NULL;
			Option_table_add_entry(option_table, "eigenvalues", &source_field,
				&set_source_field_data, set_Computed_field_conditional);
			return_code = Option_table_multi_parse(option_table, state);
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Cmiss_field_module_create_eigenvectors(field_modify->get_field_module(),
						source_field));
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
			"define_Computed_field_type_eigenvectors.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_eigenvectors */




int define_Computed_field_type_matrix_invert(struct Parse_state *state,
	void *field_modify_void, void *computed_field_matrix_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type 'matrix_invert' (if it is not already) and allows its
contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_matrix_invert);
	USE_PARAMETER(computed_field_matrix_operators_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code = 1;
		source_field = (struct Computed_field *)NULL;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_matrix_invert_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code = Computed_field_get_type_matrix_invert(field_modify->get_field(), &source_field);
		}
		if (return_code)
		{
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			option_table = CREATE(Option_table)();
			Option_table_add_help(option_table,
			  "A matrix_invert field returns the inverse of a square matrix.  Here, a 9 component source field is interpreted as a (3 * 3) matrix with the first 3 components being the first row, the next 3 components being the middle row, and so on.  See a/current_density for an example of using the matrix_invert field.");
			set_source_field_data.conditional_function =
				Computed_field_is_square_matrix;
			set_source_field_data.conditional_function_user_data = (void *)NULL;
			set_source_field_data.computed_field_manager =
				field_modify->get_field_manager();
			Option_table_add_entry(option_table, "field", &source_field,
				&set_source_field_data, set_Computed_field_conditional);
			return_code = Option_table_multi_parse(option_table, state);
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Cmiss_field_module_create_matrix_invert(field_modify->get_field_module(),
						source_field));
			}
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


int define_Computed_field_type_matrix_multiply(struct Parse_state *state,
	void *field_modify_void,void *computed_field_matrix_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_MATRIX_MULTIPLY (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	const char *current_token;
	int i, number_of_rows, return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_field_array_data;
	struct Set_Computed_field_conditional_data set_field_data;

	ENTER(define_Computed_field_type_matrix_multiply);
	USE_PARAMETER(computed_field_matrix_operators_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		if (ALLOCATE(source_fields,struct Computed_field *,2))
		{
			/* get valid parameters for matrix_multiply field */
			number_of_rows = 1;
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			if ((NULL != field_modify->get_field()) &&
				(computed_field_matrix_multiply_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
			{
				return_code=Computed_field_get_type_matrix_multiply(field_modify->get_field(),
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
				current_token=state->current_token;
				if (current_token != 0)
				{
					if (!(strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
					{
						option_table = CREATE(Option_table)();
						Option_table_add_help(option_table,
						  "A matrix_mutliply field calculates the product of two matrices, giving a new m by n matrix.  The product is represented as a field with a list of (m * n) components.   The components of the matrix are listed row by row.  The <number_of_rows> m is used to infer the dimensions of the source matrices.  The two source <fields> are multiplied, with the components of the first interpreted as a matrix with dimensions m by s and the second as a matrix with dimensions s by n.  If the matrix dimensions are not consistent then an error is returned.  See a/curvature for an example of using the matrix_multiply field.");
						Option_table_add_entry(option_table,"number_of_rows",
							&number_of_rows,NULL,set_int_positive);
						set_field_data.conditional_function=
							Computed_field_has_numerical_components;
						set_field_data.conditional_function_user_data=(void *)NULL;
						set_field_data.computed_field_manager=
							field_modify->get_field_manager();
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
								field_modify->get_field_manager();
							set_field_array_data.number_of_fields=2;
							set_field_array_data.conditional_data= &set_field_data;
							Option_table_add_entry(option_table,"fields",source_fields,
								&set_field_array_data,set_Computed_field_array);
							return_code = Option_table_multi_parse(option_table, state);
							if (return_code)
							{
								return_code = field_modify->update_field_and_deaccess(
									Cmiss_field_module_create_matrix_multiply(field_modify->get_field_module(),
										number_of_rows, source_fields[0], source_fields[1]));
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


/***************************************************************************//**
 * Converts <field> into type COMPUTED_FIELD_PROJECTION (if it is not already)
 * and allows its contents to be modified.
 */
int define_Computed_field_type_projection(struct Parse_state *state,
	void *field_modify_void,void *computed_field_matrix_operators_package_void)
{
	int return_code;

	ENTER(define_Computed_field_type_projection);
	USE_PARAMETER(computed_field_matrix_operators_package_void);
	Computed_field_modify_data *field_modify =
		reinterpret_cast<Computed_field_modify_data *>(field_modify_void);
	if (state && field_modify)
	{
		return_code = 1;
		Cmiss_field_id source_field = 0;
		Cmiss_field_id projection_matrix_field = 0;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_projection_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			Computed_field_get_type_projection(field_modify->get_field(),
				&source_field, &projection_matrix_field);
		}
		if (source_field)
		{
			ACCESS(Computed_field)(source_field);
		}
		if (projection_matrix_field)
		{
			ACCESS(Computed_field)(projection_matrix_field);
		}
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"Creates a projection field returning the result of a matrix multiplication "
			"with perspective division on the source field vector. The source_field vector "
			"is expanded to a homogeneous coordinate by appending a component of value 1, "
			"which is multiplied by the projection_matrix field, and the extra calculated "
			"value resulting from the unit component is used to divide through each of the "
			"other components to give a perspective projection in the resulting field. "
			"The projection_matrix field must have have a multiple of "
			"(source_field->number_of_components + 1) components forming a matrix with "
			"that many columns and the resulting (number_of_components + 1) rows. The "
			"first values in the projection_matrix are across the first row, followed by "
			"the next row and so on. "
			"Hence a 4x4 matrix transforms a 3-component vector to a 3-component vector.\n");
		/* field */
		struct Set_Computed_field_conditional_data set_field_data;
		set_field_data.conditional_function = Computed_field_has_numerical_components;
		set_field_data.conditional_function_user_data = (void *)NULL;
		set_field_data.computed_field_manager = field_modify->get_field_manager();
		Option_table_add_entry(option_table, "field", &source_field,
			&set_field_data, set_Computed_field_conditional);
		Option_table_add_entry(option_table, "projection_matrix", &projection_matrix_field,
			&set_field_data, set_Computed_field_conditional);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			return_code = field_modify->update_field_and_deaccess(
				Cmiss_field_module_create_projection(field_modify->get_field_module(),
					source_field, projection_matrix_field));
		}
		if (source_field)
		{
			DEACCESS(Computed_field)(&source_field);
		}
		if (projection_matrix_field)
		{
			DEACCESS(Computed_field)(&projection_matrix_field);
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


int define_Computed_field_type_transpose(struct Parse_state *state,
	void *field_modify_void,void *computed_field_matrix_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_TRANSPOSE (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	const char *current_token;
	int source_number_of_rows, return_code;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_transpose);
	USE_PARAMETER(computed_field_matrix_operators_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		/* get valid parameters for transpose field */
		source_number_of_rows = 1;
		source_field = (struct Computed_field *)NULL;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_transpose_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code=Computed_field_get_type_transpose(field_modify->get_field(),
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
			current_token = state->current_token;
			if (current_token != 0)
			{
				set_source_field_data.conditional_function =
					Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data = (void *)NULL;
				set_source_field_data.computed_field_manager =
					field_modify->get_field_manager();
				if (!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
				{
					option_table = CREATE(Option_table)();
					Option_table_add_help(option_table,
					  "A transpose field returns the transpose of a source matrix field.  If the source <field> has (m * n) components where m is specified by <source_number_of_rows> (with the first n components being the first row), the result is its (n * m) transpose.  See a/current_density for an example of using the matrix_invert field.");
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
						return_code = Option_table_multi_parse(option_table, state);
						if (return_code)
						{
							return_code = field_modify->update_field_and_deaccess(
								Cmiss_field_module_create_transpose(field_modify->get_field_module(),
									source_number_of_rows, source_field));
						}
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



int define_Computed_field_type_quaternion_to_matrix(struct Parse_state *state,
	void *field_modify_void, void *computed_field_matrix_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 18 Jun 2008

DESCRIPTION :
Converts a "quaternion" to a transformation matrix.
==============================================================================*/
{
	int return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_quaternion_to_matrix);
	USE_PARAMETER(computed_field_matrix_operators_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			 source_fields[0] = (struct Computed_field *)NULL;
				if ((NULL != field_modify->get_field()) &&
					(computed_field_quaternion_to_matrix_type_string ==
						Computed_field_get_type_string(field_modify->get_field())))
			 {
					return_code = Computed_field_get_type_quaternion_to_matrix(field_modify->get_field(),
						 source_fields);
			 }
			 if (return_code)
			 {
					if (source_fields[0])
					{
						 ACCESS(Computed_field)(source_fields[0]);
					}
					option_table = CREATE(Option_table)();
					Option_table_add_help(option_table,
						 "A computed field to convert a quaternion (w,x,y,z) to a 4x4 matrix,");
					set_source_field_data.computed_field_manager =
						field_modify->get_field_manager();
					set_source_field_data.conditional_function =
						 Computed_field_has_4_components;
					set_source_field_data.conditional_function_user_data = (void *)NULL;
					Option_table_add_entry(option_table, "field", &source_fields[0],
						 &set_source_field_data, set_Computed_field_conditional);
					return_code = Option_table_multi_parse(option_table, state);
					if (return_code)
					{
						return_code = field_modify->update_field_and_deaccess(
							Computed_field_create_quaternion_to_matrix(
								field_modify->get_field_module(), source_fields[0]));
					}
					else
					{
						 if ((!state->current_token)||
								(strcmp(PARSER_HELP_STRING,state->current_token)&&
									 strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
						 {
								/* error */
								display_message(ERROR_MESSAGE,
									 "define_Computed_field_type_quaternion_to_matrix.  Failed");
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
					"define_Computed_field_type_quaternion_to_matrix. Not enought memory");
			 return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_quaternion_to_matrix. Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_quaternion_to_matrix */



int define_Computed_field_type_matrix_to_quaternion(struct Parse_state *state,
	void *field_modify_void, void *computed_field_matrix_operators_package_void)
/*******************************************************************************
LAST MODIFIED : 18 Jun 2008

DESCRIPTION :
Converts a transformation matrix to  a "quaternion".
==============================================================================*/
{
	int return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_matrix_to_quaternion);
	USE_PARAMETER(computed_field_matrix_operators_package_void);
	if (state&&(field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			 source_fields[0] = (struct Computed_field *)NULL;
				if ((NULL != field_modify->get_field()) &&
					(computed_field_matrix_to_quaternion_type_string ==
						Computed_field_get_type_string(field_modify->get_field())))
			 {
					return_code = Computed_field_get_type_matrix_to_quaternion(field_modify->get_field(),
						 source_fields);
			 }
			 if (return_code)
			 {
					if (source_fields[0])
					{
						 ACCESS(Computed_field)(source_fields[0]);
					}
					option_table = CREATE(Option_table)();
					Option_table_add_help(option_table,
						 "A computed field to convert a 4x4 matrix to a quaternion.  "
						 "components of the matrix should be read in as follow       "
						 "    0   1   2   3                                          "
						 "    4   5   6   7                                          "
						 "    8   9   10  11                                         "
						 "    12  13  14  15                                         \n");
					set_source_field_data.computed_field_manager =
						field_modify->get_field_manager();
					set_source_field_data.conditional_function =
						 Computed_field_has_16_components;
					set_source_field_data.conditional_function_user_data = (void *)NULL;
					Option_table_add_entry(option_table, "field", &source_fields[0],
						 &set_source_field_data, set_Computed_field_conditional);
					/* process the option table */
					return_code = Option_table_multi_parse(option_table, state);
					if (return_code)
					{
						return_code = field_modify->update_field_and_deaccess(
							Computed_field_create_matrix_to_quaternion(
								field_modify->get_field_module(), source_fields[0]));
					}
					else
					{
						 if ((!state->current_token)||
								(strcmp(PARSER_HELP_STRING,state->current_token)&&
									 strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
						 {
								/* error */
								display_message(ERROR_MESSAGE,
									 "define_Computed_field_type_matrix_to_quaternion.  Failed");
						 }
					}
					/* no errors, not asking for help */
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
					"define_Computed_field_type_matrix_to_quaternion. Not enought memory");
			 return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_matrix_to_quaternion. Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_matrix_to_quaternion */

int Computed_field_register_types_matrix_operators(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	Computed_field_matrix_operators_package
		*computed_field_matrix_operators_package =
		new Computed_field_matrix_operators_package;

	ENTER(Computed_field_register_types_matrix_operators);
	if (computed_field_package)
	{
		return_code =
			Computed_field_package_add_type(computed_field_package,
				computed_field_determinant_type_string,
				define_Computed_field_type_determinant,
				computed_field_matrix_operators_package);
		return_code =
			Computed_field_package_add_type(computed_field_package,
				computed_field_eigenvalues_type_string,
				define_Computed_field_type_eigenvalues,
				computed_field_matrix_operators_package);
		return_code =
			Computed_field_package_add_type(computed_field_package,
				computed_field_eigenvectors_type_string,
				define_Computed_field_type_eigenvectors,
				computed_field_matrix_operators_package);
		return_code =
			Computed_field_package_add_type(computed_field_package,
				computed_field_matrix_invert_type_string,
				define_Computed_field_type_matrix_invert,
				computed_field_matrix_operators_package);
		return_code =
			Computed_field_package_add_type(computed_field_package,
				computed_field_matrix_multiply_type_string,
				define_Computed_field_type_matrix_multiply,
				computed_field_matrix_operators_package);
		return_code =
			Computed_field_package_add_type(computed_field_package,
				computed_field_projection_type_string,
				define_Computed_field_type_projection,
				computed_field_matrix_operators_package);
		return_code =
			Computed_field_package_add_type(computed_field_package,
				computed_field_transpose_type_string,
				define_Computed_field_type_transpose,
				computed_field_matrix_operators_package);
		return_code =
			Computed_field_package_add_type(computed_field_package,
			computed_field_quaternion_to_matrix_type_string,
			define_Computed_field_type_quaternion_to_matrix,
			computed_field_matrix_operators_package);
		return_code =
			Computed_field_package_add_type(computed_field_package,
			computed_field_matrix_to_quaternion_type_string,
			define_Computed_field_type_matrix_to_quaternion,
			computed_field_matrix_operators_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_matrix_operators.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_matrix_operators */
