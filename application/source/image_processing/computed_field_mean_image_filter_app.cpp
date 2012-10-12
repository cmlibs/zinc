#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "image_processing/computed_field_mean_image_filter.h"

char computed_field_mean_image_filter_type_string[] = "mean_filter";

int Cmiss_field_get_type_mean_image_filter(struct Computed_field *field,
	struct Computed_field **source_field, int **radius_sizes);

int define_Computed_field_type_mean_image_filter(struct Parse_state *state,
	void *field_modify_void, void *computed_field_simple_package_void)
/*******************************************************************************
LAST MODIFIED : 30 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_MEANIMAGEFILTER (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int dimension, expected_parameters, i, old_dimension, previous_state_index,
		*radius_sizes, return_code, *sizes;
	struct Computed_field *source_field, *texture_coordinate_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_mean_image_filter);
	USE_PARAMETER(computed_field_simple_package_void);
	if (state && (field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		radius_sizes = (int *)NULL;
		old_dimension = 0;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_mean_image_filter_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code =
				Cmiss_field_get_type_mean_image_filter(field_modify->get_field(), &source_field,
					&radius_sizes);
			return_code = Computed_field_get_native_resolution(source_field,
				&old_dimension, &sizes, &texture_coordinate_field);
			if (return_code)
			{
				DEALLOCATE(sizes);
			}
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}

			if ((!state->current_token) ||
				(strcmp(PARSER_HELP_STRING, state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
			{

				previous_state_index = state->current_index;

				/* Discover the source field */
				option_table = CREATE(Option_table)();
				/* field */
				set_source_field_data.computed_field_manager =
					field_modify->get_field_manager();
				set_source_field_data.conditional_function = Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data = (void *)NULL;
				Option_table_add_entry(option_table, "field", &source_field,
					&set_source_field_data, set_Computed_field_conditional);
				/* Ignore all the other entries */
				Option_table_ignore_all_unmatched_entries(option_table);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
				/* Return back to where we were */
				shift_Parse_state(state, previous_state_index - state->current_index);
				/* no errors,not asking for help */
				if (return_code)
				{
					if (!source_field)
					{
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_mean_image_filter.  "
							"Missing source field");
						return_code = 0;
					}
				}
				if (return_code)
				{
					return_code = Computed_field_get_native_resolution(source_field,
						&dimension, &sizes, &texture_coordinate_field);
					if (return_code)
					{
						DEALLOCATE(sizes);

						if (!radius_sizes || (old_dimension != dimension))
						{
							REALLOCATE(radius_sizes, radius_sizes, int, dimension);
							for (i = old_dimension ; i < dimension ; i++)
							{
								radius_sizes[i] = 1;
							}
						}
						/* Read the radius sizes */
						option_table = CREATE(Option_table)();
						/* field */
						expected_parameters = 1;
						Option_table_add_ignore_token_entry(option_table, "field",
							&expected_parameters);
						/* radius_sizes */
						Option_table_add_int_vector_entry(option_table,
							"radius_sizes", radius_sizes, &dimension);
						return_code = Option_table_multi_parse(option_table, state);
						DESTROY(Option_table)(&option_table);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_mean_image_filter.  "
							"Source field does not have an image dimension.");
						return_code = 0;
					}
				}
				if (return_code)
				{
					return_code = field_modify->update_field_and_deaccess(
						Cmiss_field_module_create_mean_image_filter(
							field_modify->get_field_module(),
							source_field, radius_sizes));
				}

				if (!return_code)
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_mean_image_filter.  Failed");
				}
			}
			else
			{
				/* Write help */
					option_table = CREATE(Option_table)();
					Option_table_add_help(option_table,
				"The mean_filter field uses the itk::MeanImageFilter code to replace each pixel with the mean intensity of the pixel and its surrounding neighbours.  It is useful for reducing the level of noise.   The <field> it operates on is usually a sample_texture field, based on a texture that has been created from image file(s).   The size of the neighbourhood of pixels used to calculate the mean is determined be a list of <radius_sizes>, one value for each dimension.  Each radius size sets how many pixels to include either side of the central pixel for the corresponding dimension. If radius values are increased, more neighbouring pixels are included and the image becomes smoother. See a/testing/image_processing_2D for an example of using this field. For more information see the itk software guide.");

					/* field */
					set_source_field_data.computed_field_manager =
						field_modify->get_field_manager();
					set_source_field_data.conditional_function = Computed_field_has_numerical_components;
					set_source_field_data.conditional_function_user_data = (void *)NULL;
					Option_table_add_entry(option_table, "field", &source_field,
						&set_source_field_data, set_Computed_field_conditional);
					/* radius_sizes */
					Option_table_add_int_vector_entry(option_table,
						"radius_sizes", radius_sizes, &dimension);
					return_code = Option_table_multi_parse(option_table, state);
					DESTROY(Option_table)(&option_table);
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			if (radius_sizes)
			{
				DEALLOCATE(radius_sizes);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_mean_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_mean_image_filter */

int Computed_field_register_types_mean_image_filter(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 30 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_register_types_mean_image_filter);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_mean_image_filter_type_string,
			define_Computed_field_type_mean_image_filter,
			Computed_field_package_get_simple_package(computed_field_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_mean_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_mean_image_filter */
