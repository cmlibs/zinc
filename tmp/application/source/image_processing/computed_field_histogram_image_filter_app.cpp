#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "image_processing/computed_field_histogram_image_filter.h"

const char computed_field_histogram_image_filter_type_string[] = "histogram_filter";

int Cmiss_field_get_type_histogram_image_filter(struct Computed_field *field,
	struct Computed_field **source_field, int **numberOfBins, double *marginalScale,
	double **histogramMinimum, double **histogramMaximum);

int define_Computed_field_type_histogram_image_filter(struct Parse_state *state,
	void *field_modify_void, void *computed_field_simple_package_void)
/*******************************************************************************
LAST MODIFIED : 18 October 2006

DESCRIPTION :
Converts <field> into type DISCRETEGAUSSIAN (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int i, original_number_of_components, previous_state_index, return_code;
	int *numberOfBins;
	double marginalScale, *histogramMinimum, *histogramMaximum;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_histogram_image_filter);
	USE_PARAMETER(computed_field_simple_package_void);
	if (state && (field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		numberOfBins = (int *)NULL;
		histogramMinimum = (double *)NULL;
		histogramMaximum = (double *)NULL;
		marginalScale = 10.0;
		original_number_of_components = 0;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_histogram_image_filter_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code =
				Cmiss_field_get_type_histogram_image_filter(field_modify->get_field(), &source_field,
					&numberOfBins, &marginalScale, &histogramMinimum, &histogramMaximum);
			original_number_of_components = source_field->number_of_components;
		}
		if (return_code)
		{
			/* must access objects for set functions */
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
					"The histogram_filter field uses the itk::ImageToHistogramGenerator code to generate binned values representing the relative frequency of the various pixel intensities.  There should be a number_of_bins for each component direction, and so the total number of bins will be a product of these, so that for a 3 component image you would get a volume histogram.  "
						"If you wanted a histogram for a single component then set the number_of_bins for the other components to 1. "
						"If you do not set the optional histogram_minimums or histogram_maximums (which is a vector of values, 1 for each source component) then the histogram will automatically range the values to the minimum and maximum values in the source image.");

				/* field */
				set_source_field_data.computed_field_manager =
					field_modify->get_field_manager();
				set_source_field_data.conditional_function = Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data = (void *)NULL;
				Option_table_add_entry(option_table, "field", &source_field,
					&set_source_field_data, set_Computed_field_conditional);
				/* histogramMinimum */
				double dummyHistogramMinimum = 0.0;
				Option_table_add_double_entry(option_table, "histogram_minimums",
					&dummyHistogramMinimum);
				/* histogramMinimum */
				double dummyHistogramMaximum = 1.0;
				Option_table_add_double_entry(option_table, "histogram_maximums",
					&dummyHistogramMaximum);
				/* numberOfBins */
				int dummyNumberOfBins = 64;
				Option_table_add_int_positive_entry(option_table, "number_of_bins",
					&dummyNumberOfBins);
				/* marginalScale */
				Option_table_add_double_entry(option_table, "marginal_scale",
					&marginalScale);

				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}

			if (return_code)
			{
				// store previous state so that we can return to it
				previous_state_index = state->current_index;

				/* parse the source field so we know the dimension. */

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
				if (!source_field)
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_histogram_image_filter.  "
						"Missing source field");
					return_code = 0;
				}
			}
			if (return_code)
			{
				if (source_field->number_of_components != original_number_of_components)
				{
					REALLOCATE(numberOfBins, numberOfBins, int,
						source_field->number_of_components);
					for (i = 0 ; i < source_field->number_of_components ; i++)
					{
						numberOfBins[i] = 64;
					}
				}

				option_table = CREATE(Option_table)();

				/* field, ignore as we have already got it */
				int field_expected_parameters = 1;
				Option_table_add_ignore_token_entry(
					option_table, "field", &field_expected_parameters);
				/* histogramMinimum */
				int histogramMinimumLength = 0;
				if (histogramMinimum)
					histogramMinimumLength = original_number_of_components;
				Option_table_add_variable_length_double_vector_entry(option_table, "histogram_minimums",
					&histogramMinimumLength, &histogramMinimum);
				/* histogramMaximum */
				int histogramMaximumLength = 0;
				if (histogramMaximum)
					histogramMaximumLength = original_number_of_components;
				Option_table_add_variable_length_double_vector_entry(option_table, "histogram_maximums",
					&histogramMaximumLength, &histogramMaximum);
				/* numberOfBins */
				Option_table_add_int_vector_entry(option_table, "number_of_bins",
					numberOfBins, &source_field->number_of_components);
				/* marginalScale */
				Option_table_add_double_entry(option_table, "marginal_scale",
					&marginalScale);

				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);

				if (histogramMinimum && histogramMinimumLength != source_field->number_of_components)
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_histogram_image_filter.  Length of histogram_minimums vector does not match source field compontents");
					return_code = 0;
				}
				if (histogramMaximum && histogramMaximumLength != source_field->number_of_components)
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_histogram_image_filter.  Length of histogram_Maximums vector does not match source field compontents");
					return_code = 0;
				}

				if (return_code)
				{
					return_code = field_modify->update_field_and_deaccess(
						Cmiss_field_module_create_histogram_image_filter(
							field_modify->get_field_module(),
							source_field, numberOfBins, marginalScale, histogramMinimum, histogramMaximum));
				}

				if (!return_code)
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_histogram_image_filter.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			if (numberOfBins)
			{
				DEALLOCATE(numberOfBins);
			}
		}
		if (histogramMinimum)
		{
			DEALLOCATE(histogramMinimum);
		}

		if (histogramMaximum)
		{
			DEALLOCATE(histogramMaximum);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_histogram_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_histogram_image_filter */

int Computed_field_register_types_histogram_image_filter(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 18 October 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_register_types_histogram_image_filter);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_histogram_image_filter_type_string,
			define_Computed_field_type_histogram_image_filter,
			Computed_field_package_get_simple_package(computed_field_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_histogram_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_histogram_image_filter */
