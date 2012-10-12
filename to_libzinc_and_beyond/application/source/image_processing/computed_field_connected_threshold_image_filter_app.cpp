#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "image_processing/computed_field_connected_threshold_image_filter.h"

const char computed_field_connected_threshold_image_filter_type_string[] = "connected_threshold_filter";

int Cmiss_field_get_type_connected_threshold_image_filter(struct Computed_field *field,
  struct Computed_field **source_field, double *lower_threshold, double *upper_threshold,
	  double *replace_value, int *num_seed_points, int *seed_dimension, double **seed_points);

int define_Computed_field_type_connected_threshold_image_filter(struct Parse_state *state,
	void *field_modify_void, void *computed_field_simple_package_void)
/*******************************************************************************
LAST MODIFIED : 30 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CONNECTED_THRESHOLD_IMAGE_FILTER (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	double lower_threshold, upper_threshold, replace_value;
	int num_seed_points;
	int seed_dimension;
  double *seed_points;
	int return_code;
	int seed_points_length;
	int previous_state_index, expected_parameters;

	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_connected_threshold_image_filter);
	USE_PARAMETER(computed_field_simple_package_void);
	if (state && (field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code = 1;

		source_field = (struct Computed_field *)NULL;
		lower_threshold = 0.0;
		upper_threshold = 1.0;
		replace_value = 1;
		num_seed_points = 0;
		seed_dimension  = 2;
		seed_points = (double *)NULL;

		// should probably default to having 1 seed point
		//		seed_points[0] = 0.5;  // pjb: is this ok?
		//		seed_points[1] = 0.5;
		seed_points_length = 0;

		/* get valid parameters for projection field */
		if ((NULL != field_modify->get_field()) &&
			(computed_field_connected_threshold_image_filter_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code =
				Cmiss_field_get_type_connected_threshold_image_filter(field_modify->get_field(), &source_field,
				&lower_threshold, &upper_threshold, &replace_value,
		  &num_seed_points, &seed_dimension, &seed_points);
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
				"The connected_threshold_filter field uses the itk::ConnectedThresholdImageFilter code to segment a field. The <field> it operates on is usually a sample_texture field, based on a texture that has been created from image file(s).  The segmentation is based on a region growing algorithm which requires at least one seed point.  To specify the seed points first set the <num_seed_points> and the <dimension> of the image.  The <seed_points> are a list of the coordinates for the first and any subsequent seed points.  Starting from the seed points any neighbouring pixels with an intensity between <lower_threshold> and the <upper_threshold> are added to the region.  Pixels within the region have their pixel intensity set to <replace_value> while the remaining pixels are set to 0. See a/testing/image_processing_2D for an example of using this field.  For more information see the itk software guide.");

				/* field */
				set_source_field_data.computed_field_manager =
					field_modify->get_field_manager();
				set_source_field_data.conditional_function = Computed_field_is_scalar;
				set_source_field_data.conditional_function_user_data = (void *)NULL;
				Option_table_add_entry(option_table, "field", &source_field,
															 &set_source_field_data, set_Computed_field_conditional);
				/* lower_threshold */
				Option_table_add_double_entry(option_table, "lower_threshold",
																			&lower_threshold);
				/* upper_threshold */
				Option_table_add_double_entry(option_table, "upper_threshold",
																			&upper_threshold);
				/* replace_value */
				Option_table_add_double_entry(option_table, "replace_value",
																			&replace_value);
				/* num_seed_points */
				Option_table_add_int_positive_entry(option_table, "num_seed_points",
																						&num_seed_points);
				Option_table_add_int_positive_entry(option_table, "dimension",
																						&seed_dimension);
				Option_table_add_double_vector_entry(option_table, "seed_points",
																						 seed_points, &seed_points_length);
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
																						&seed_dimension);
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
				seed_points_length = num_seed_points * seed_dimension;

				REALLOCATE(seed_points, seed_points, double, seed_points_length);
				// pjb: should I populate array with dummy values?

				option_table = CREATE(Option_table)();
				/* field */
				set_source_field_data.computed_field_manager =
					field_modify->get_field_manager();
				set_source_field_data.conditional_function = Computed_field_is_scalar;
				set_source_field_data.conditional_function_user_data = (void *)NULL;
				Option_table_add_entry(option_table, "field", &source_field,
															 &set_source_field_data, set_Computed_field_conditional);
				/* lower_threshold */
				Option_table_add_double_entry(option_table, "lower_threshold",
																			&lower_threshold);
				/* upper_threshold */
				Option_table_add_double_entry(option_table, "upper_threshold",
																			&upper_threshold);
				/* replace_value */
				Option_table_add_double_entry(option_table, "replace_value",
																			&replace_value);
				expected_parameters = 1;
				Option_table_add_ignore_token_entry(option_table, "num_seed_points",
																						&expected_parameters);
				expected_parameters = 1;
				Option_table_add_ignore_token_entry(option_table, "dimension",
																						&expected_parameters);
				Option_table_add_double_vector_entry(option_table, "seed_points",
																						 seed_points, &seed_points_length);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}

			/* no errors,not asking for help */
			if (return_code)
			{
				if (!source_field)
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_connected_threshold_image_filter.  "
						"Missing source field");
					return_code = 0;
				}
			}
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Cmiss_field_module_create_connected_threshold_image_filter(
						field_modify->get_field_module(),
						source_field, lower_threshold, upper_threshold, replace_value,
						num_seed_points, seed_dimension, seed_points));
			}

			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_connected_threshold_image_filter.  Failed");
				}
			}

			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}

			if (seed_points) {
				DEALLOCATE(seed_points);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_connected_threshold_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_connected_threshold_image_filter */

int Computed_field_register_types_connected_threshold_image_filter(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 30 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_register_types_connected_threshold_image_filter);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_connected_threshold_image_filter_type_string,
			define_Computed_field_type_connected_threshold_image_filter,
			Computed_field_package_get_simple_package(computed_field_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_connected_threshold_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_connected_threshold_image_filter */
