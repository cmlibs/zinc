#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "image_processing/computed_field_binary_dilate_image_filter.h"

const char computed_field_binary_dilate_image_filter_type_string[] = "binary_dilate_filter";

int Cmiss_field_get_type_binary_dilate_image_filter(struct Computed_field *field,
	struct Computed_field **source_field, int *radius,
	double *dilate_value);

int define_Computed_field_type_binary_dilate_image_filter(struct Parse_state *state,
	void *field_modify_void, void *computed_field_simple_package_void)
/*******************************************************************************
LAST MODIFIED : 30 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_BINARYDILATEFILTER (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int radius;
	double dilate_value;
	int return_code;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_binary_dilate_image_filter);
	USE_PARAMETER(computed_field_simple_package_void);
	if (state && (field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		radius = 1;
		dilate_value = 1;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_binary_dilate_image_filter_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code =
				Cmiss_field_get_type_binary_dilate_image_filter(field_modify->get_field(), &source_field,
					&radius, &dilate_value);
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
				"The binary_dilate_filter field uses the itk::BinaryDilateImageFilter code to produce an output field which is a dilation of a binary image (an image where each pixel has 1 of 2 values).  The region identified by the pixels with intensity <dilate_value> is dilate (enlarged), by replacing each pixel with a ball.  The size of the ball is set by specifying the <radius>. The <field> it operates on is usually a thresholded or segmented field. See a/testing/image_processing_2D for an example of using this field.  For more information see the itk software guide.");

			/* field */
			set_source_field_data.computed_field_manager =
				field_modify->get_field_manager();
			set_source_field_data.conditional_function = Computed_field_is_scalar;
			set_source_field_data.conditional_function_user_data = (void *)NULL;
			Option_table_add_entry(option_table, "field", &source_field,
				&set_source_field_data, set_Computed_field_conditional);
			/* radius */
			Option_table_add_int_non_negative_entry(option_table, "radius",
				&radius);
			/* dilate_value */
			Option_table_add_double_entry(option_table, "dilate_value",
				&dilate_value);

			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);

			/* no errors,not asking for help */
			if (return_code)
			{
				if (!source_field)
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_binary_dilate_image_filter.  "
						"Missing source field");
					return_code = 0;
				}
			}
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Cmiss_field_module_create_binary_dilate_image_filter(
						field_modify->get_field_module(),
						source_field, radius, dilate_value));
			}

			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_binary_dilate_image_filter.  Failed");
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
			"define_Computed_field_type_binary_dilate_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_binary_dilate_image_filter */

int Computed_field_register_types_binary_dilate_image_filter(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 30 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_register_types_binary_dilate_image_filter);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_binary_dilate_image_filter_type_string,
			define_Computed_field_type_binary_dilate_image_filter,
			Computed_field_package_get_simple_package(computed_field_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_binary_dilate_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_binary_dilate_image_filter */
