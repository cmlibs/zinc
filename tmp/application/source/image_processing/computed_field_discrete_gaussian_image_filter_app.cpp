#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "image_processing/computed_field_discrete_gaussian_image_filter.h"

const char computed_field_discrete_gaussian_image_filter_type_string[] = "discrete_gaussian_filter";

int Cmiss_field_get_type_discrete_gaussian_image_filter(struct Computed_field *field,
	struct Computed_field **source_field, double *variance, int *maxKernelWidth);

int define_Computed_field_type_discrete_gaussian_image_filter(struct Parse_state *state,
	void *field_modify_void, void *computed_field_simple_package_void)
/*******************************************************************************
LAST MODIFIED : 18 October 2006

DESCRIPTION :
Converts <field> into type DISCRETEGAUSSIAN (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	double variance;
	int maxKernelWidth;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_discrete_gaussian_image_filter);
	USE_PARAMETER(computed_field_simple_package_void);
	if (state && (field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		variance = 1;
		maxKernelWidth = 4;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_discrete_gaussian_image_filter_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code =
				Cmiss_field_get_type_discrete_gaussian_image_filter(field_modify->get_field(), &source_field,
					&variance, &maxKernelWidth);
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
				"The discrete_gaussian_filter field uses the itk::DiscreteGaussianImageFilter code to smooth a field. It is useful for removing noise or unwanted detail.  The <field> it operates on is usually a sample_texture field, based on a texture that has been created from image file(s).  The effect of applying a discrete gaussian image filter is that a pixel value is based on a weighted average of surrounding pixel values, where the closer the pixel the more weight its value is given. Increasing the <variance> increases the width of the gaussian distribution used and hence the number of pixels used to calculate the weighted average. This smooths the image more.  A limit is set on the <max_kernel_width> used to approximate the guassian to ensure the calculation completes.  See a/testing/image_processing_2D for an example of using this field. For more information see the itk software guide.");

			/* field */
			set_source_field_data.computed_field_manager =
				field_modify->get_field_manager();
			set_source_field_data.conditional_function = Computed_field_is_scalar;
			set_source_field_data.conditional_function_user_data = (void *)NULL;
			Option_table_add_entry(option_table, "field", &source_field,
				&set_source_field_data, set_Computed_field_conditional);
			/* variance */
			Option_table_add_double_entry(option_table, "variance",
				&variance);
			/* maxkernelwidth */
			Option_table_add_int_positive_entry(option_table, "max_kernel_width",
				&maxKernelWidth);

			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);

			/* no errors,not asking for help */
			if (return_code)
			{
				if (!source_field)
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_discrete_gaussian_image_filter.  "
						"Missing source field");
					return_code = 0;
				}
			}
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Cmiss_field_module_create_discrete_gaussian_image_filter(
						field_modify->get_field_module(),
						source_field, variance, maxKernelWidth));
			}

			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_discrete_gaussian_image_filter.  Failed");
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
			"define_Computed_field_type_discrete_gaussian_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_discrete_gaussian_image_filter */

int Computed_field_register_types_discrete_gaussian_image_filter(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 18 October 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_register_types_discrete_gaussian_image_filter);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_discrete_gaussian_image_filter_type_string,
			define_Computed_field_type_discrete_gaussian_image_filter,
			Computed_field_package_get_simple_package(computed_field_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_discrete_gaussian_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_discrete_gaussian_image_filter */
