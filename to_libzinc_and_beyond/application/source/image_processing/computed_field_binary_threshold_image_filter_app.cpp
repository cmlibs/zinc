
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "image_processing/computed_field_binary_threshold_image_filter.h"

const char computed_field_binary_threshold_image_filter_type_string[] = "binary_threshold_filter";

int Cmiss_field_get_type_binary_threshold_image_filter(struct Computed_field *field,
	struct Computed_field **source_field, double *lower_threshold,
	double *upper_threshold);

/*****************************************************************************//**
 * Converts <field> into type COMPUTED_FIELD_BINARY_THRESHOLD_IMAGE_FILTER
 * (if it is not already) and allows its contents to be modified.
 *
 * @param state Parse state of field to define
 * @param field_modify_void
 * @param computed_field_simple_package_void
 * @return Return code indicating succes (1) or failure (0)
*/
int define_Computed_field_type_binary_threshold_image_filter(struct Parse_state *state,
	void *field_modify_void, void *computed_field_simple_package_void)
{
	double lower_threshold, upper_threshold;
	int return_code;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_binary_threshold_image_filter);
	USE_PARAMETER(computed_field_simple_package_void);
	if (state && (field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		lower_threshold = 0.0;
		upper_threshold = 1.0;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_binary_threshold_image_filter_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code =
				Cmiss_field_get_type_binary_threshold_image_filter(field_modify->get_field(), &source_field,
					&lower_threshold, &upper_threshold);
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
				"The binary_threshold_filter field uses the itk::BinaryThresholdImageFilter code to produce an output field where each pixel has one of two values (either 0 or 1). It is useful for separating out regions of interest. The <field> it operates on is usually a sample_texture field, based on a texture that has been created from image file(s).  Pixels with an intensity range between <lower_threshold> and the <upper_threshold> are set to 1, the rest are set to 0. See a/testing/image_processing_2D for an example of using this field.  For more information see the itk software guide.");

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

			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);

			/* no errors,not asking for help */
			if (return_code)
			{
				if (!source_field)
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_binary_threshold_image_filter.  "
						"Missing source field");
					return_code = 0;
				}
			}
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Cmiss_field_module_create_binary_threshold_image_filter(
						field_modify->get_field_module(),
						source_field, lower_threshold, upper_threshold));
			}

			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_binary_threshold_image_filter.  Failed");
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
			"define_Computed_field_type_binary_threshold_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_binary_threshold_image_filter */

int Computed_field_register_types_binary_threshold_image_filter(
	struct Computed_field_package *computed_field_package)
{
	int return_code;

	ENTER(Computed_field_register_types_binary_threshold_image_filter);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_binary_threshold_image_filter_type_string,
			define_Computed_field_type_binary_threshold_image_filter,
			Computed_field_package_get_simple_package(computed_field_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_binary_threshold_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_binary_threshold_image_filter */
