#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "image_processing/computed_field_fast_marching_image_filter.h"

const char computed_field_fast_marching_image_filter_type_string[] = "fast_marching_filter";

int Cmiss_field_get_type_fast_marching_image_filter(struct Computed_field *field,
  struct Computed_field **source_field, double *stopping_value,
		int *num_seed_points, int *dimension, double **seed_points,
		double **seed_values, int **output_size);

int define_Computed_field_type_fast_marching_image_filter(struct Parse_state *state,
	void *field_modify_void, void *computed_field_simple_package_void)
/*******************************************************************************
LAST MODIFIED : 30 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_FAST_MARCHING_IMAGE_FILTER (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	double stopping_value;
	int num_seed_points;
	int dimension;
	double *seed_points;
	double *seed_values;
	int *output_size;
	int length_seed_points;
	int return_code;
	int previous_state_index;
	int i;

	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_fast_marching_image_filter);
	USE_PARAMETER(computed_field_simple_package_void);
	if (state && (field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code = 1;

		source_field = (struct Computed_field *)NULL;
		stopping_value = 100;
		num_seed_points = 1;
		dimension  = 2;
		length_seed_points = num_seed_points * dimension;

		seed_points = (double *)NULL;
		seed_values = (double *)NULL;
		output_size = (int *)NULL;

		ALLOCATE(seed_points, double, length_seed_points);
		ALLOCATE(seed_values, double, num_seed_points);
		ALLOCATE(output_size, int, dimension);

		// should probably default to having 1 seed
		seed_points[0] = 0.5;  // pjb: is this ok?
		seed_points[1] = 0.5;
		seed_values[0] = 0.0;
		output_size[0] = 128;
		output_size[1] = 128;

		/* get valid parameters for projection field */
		if ((NULL != field_modify->get_field()) &&
			(computed_field_fast_marching_image_filter_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code =
				Cmiss_field_get_type_fast_marching_image_filter(field_modify->get_field(), &source_field,
				  &stopping_value, &num_seed_points, &dimension, &seed_points, &seed_values, &output_size);
		}
		if (return_code)
		{

			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}

			if (state->current_token &&
				(!(strcmp(PARSER_HELP_STRING, state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token))))
			{
				/* Handle help separately */
				option_table = CREATE(Option_table)();
				Option_table_add_help(option_table,
					"The fast_marching_filter field uses the itk::FastMarchingImageFilter code to segment a field. The segmentation is based on a level set algorithm.  The <field> it operates on is used as a speed term, to govern where the level set curve grows quickly.  The speed term is usually some function (eg a sigmoid) of an image gradient field.  The output field is a time crossing map, where the value at is each pixel is the time take to reach that location from the specified seed points.  Values typically range from 0 through to extremely large (10 to the 38).  To convert the time cross map into a segmented region use a binary threshold filter. To specify the seed points first set the <num_seed_points> and the <dimension> of the image.  The <seed_points> are a list of the coordinates for the first and any subsequent seed points.   It is also possible to specify non-zero initial <seed_values> if desired and to set the <output_size> of the time crossing map. See a/segmentation for an example of using this field.  For more information see the itk software guide.");

				/* field */
				set_source_field_data.computed_field_manager =
					field_modify->get_field_manager();
				set_source_field_data.conditional_function = Computed_field_is_scalar;
				set_source_field_data.conditional_function_user_data = (void *)NULL;
				Option_table_add_entry(option_table, "field", &source_field,
					&set_source_field_data, set_Computed_field_conditional);
				/* stopping_value */
				Option_table_add_double_entry(option_table, "stopping_value",
																			&stopping_value);
				/* num_seed_points */
				Option_table_add_int_positive_entry(option_table, "num_seed_points",
																						&num_seed_points);
				/* dimension */
				Option_table_add_int_positive_entry(option_table, "dimension",
																						&dimension);
				/* seed_points */
				Option_table_add_double_vector_entry(option_table, "seed_points",
																						 seed_points, &length_seed_points);
				/* seed_values */
				Option_table_add_double_vector_entry(option_table, "seed_values",
																						 seed_values, &num_seed_points);
				/* output_size */
				Option_table_add_int_vector_entry(option_table, "output_size",
																						 output_size, &dimension);

				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}

			if (return_code)
			{

				// store previous state so that we can return to it
				previous_state_index = state->current_index;

				/* parse the two options which determine the number of seed values
					 and hence the size of the array that must be created to read in
					 the number of seed values. */

				option_table = CREATE(Option_table)();
				/* num_seed_points */
				Option_table_add_int_positive_entry(option_table, "num_seed_points",
																						&num_seed_points);
				/* dimension */
				Option_table_add_int_positive_entry(option_table, "dimension",
																						&dimension);
				/* Ignore all the other entries */
				Option_table_ignore_all_unmatched_entries(option_table);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);

				/* Return back to where we were */
				shift_Parse_state(state, previous_state_index - state->current_index);

			}

			/* parse the rest of the table */
			if (return_code)
			{
				length_seed_points = num_seed_points * dimension;

				/* reallocate memory for arrays */
				REALLOCATE(seed_points, seed_points, double, length_seed_points);
				REALLOCATE(seed_values, seed_values, double, num_seed_points);
				REALLOCATE(output_size, output_size, int, dimension);

				/* set default values, in case options are not specified in field definition. */
				for (i=0; i < length_seed_points ;i++) {
					seed_points[i] = 0;
				}
				for (i=0; i < num_seed_points ;i++) {
					seed_values[i] = 0;
				}
				for (i=0; i < dimension ;i++) {
					output_size[i] = 128;
				}

				option_table = CREATE(Option_table)();
				/* field */
				set_source_field_data.computed_field_manager =
					field_modify->get_field_manager();
				set_source_field_data.conditional_function = Computed_field_is_scalar;
				set_source_field_data.conditional_function_user_data = (void *)NULL;
				Option_table_add_entry(option_table, "field", &source_field,
															 &set_source_field_data, set_Computed_field_conditional);
				/* stopping_value */
				Option_table_add_double_entry(option_table, "stopping_value",
					&stopping_value);
				int num_seed_points_expected_parameters = 1;
				Option_table_add_ignore_token_entry(option_table, "num_seed_points",
					&num_seed_points_expected_parameters);
				int dimension_expected_parameters = 1;
				Option_table_add_ignore_token_entry(option_table, "dimension",
					&dimension_expected_parameters);
				Option_table_add_double_vector_entry(option_table, "seed_points",
					seed_points, &length_seed_points);
				Option_table_add_double_vector_entry(option_table, "seed_values",
					seed_values, &num_seed_points);
				Option_table_add_int_vector_entry(option_table, "output_size",
					output_size, &dimension);

				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);

			}

			/* no errors,not asking for help */
			if (return_code)
			{
				if (!source_field)
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_fast_marching_image_filter.  "
						"Missing source field");
					return_code = 0;
				}
			}
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Cmiss_field_module_create_fast_marching_image_filter(
						field_modify->get_field_module(),
						source_field, stopping_value, num_seed_points, dimension,
						seed_points, seed_values, output_size));
			}

			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_fast_marching_image_filter.  Failed");
				}
			}

			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}

			if (seed_points) {
				DEALLOCATE(seed_points);
			}

			if (seed_values) {
				DEALLOCATE(seed_values);
			}

			if (output_size) {
				DEALLOCATE(output_size);
			}

		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_fast_marching_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_fast_marching_image_filter */

int Computed_field_register_types_fast_marching_image_filter(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 30 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_register_types_fast_marching_image_filter);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_fast_marching_image_filter_type_string,
			define_Computed_field_type_fast_marching_image_filter,
			Computed_field_package_get_simple_package(computed_field_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_fast_marching_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_fast_marching_image_filter */
