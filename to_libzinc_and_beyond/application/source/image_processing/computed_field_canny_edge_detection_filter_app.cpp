#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "image_processing/computed_field_canny_edge_detection_filter.h"

char computed_field_canny_edge_detection_image_filter_type_string[] = "canny_edge_detection_filter";

int Cmiss_field_get_type_canny_edge_detection_image_filter(struct Computed_field *field,
	struct Computed_field **source_field, double *variance, double *maximumError,
	double *upperThreshold, double *lowerThreshold);

int define_Computed_field_type_canny_edge_detection_image_filter(struct Parse_state *state,
	void *field_modify_void, void *computed_field_simple_package_void)
/*******************************************************************************
LAST MODIFIED : 30 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CANNYEDGEDETECTIONFILTER (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	double variance;
	double maximumError;
	double upperThreshold;
	double lowerThreshold;

	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_canny_edge_detection_image_filter);
	USE_PARAMETER(computed_field_simple_package_void);
	if (state && (field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		variance = 0;
		maximumError = 0.01;
		upperThreshold = 0;
		lowerThreshold = 0;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_canny_edge_detection_image_filter_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code =
				Cmiss_field_get_type_canny_edge_detection_image_filter(field_modify->get_field(), &source_field,
				  &variance, &maximumError, &upperThreshold, &lowerThreshold);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}

			/* Discover the source field */
			option_table = CREATE(Option_table)();
			Option_table_add_help(option_table,
				"The canny_edge_detection field uses the itk::CannyEdgeDetectionImageFilter code to detect edges in a field. The <field> it operates on is usually a sample_texture field, based on a texture that has been created from image file(s).  Increasing the <variance> smooths the input image more, which reduces sensitivity to image noise at the expense of losing some detail. Decreasing the <maximum_error> also reduces edges detected as the result of noise.  The <upper_threshold> sets the level which a point must be above to use it as the start of the edge. The edge will then grow from that point until the level drops below the <lower_threshold>. Increasing the <upper_threshold> will decrease the number of edges detected. Increasing the <lower_threshold> will reduce the length of edges.  See a/testing/image_processing_2D for an example of using this field.");

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
			/* maximumError */
			Option_table_add_double_entry(option_table, "maximum_error",
				&maximumError);
			/* upperThreshold */
			Option_table_add_double_entry(option_table, "upper_threshold",
				&upperThreshold);
			/* lowerThreshold */
			Option_table_add_double_entry(option_table, "lower_threshold",
				&lowerThreshold);
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);

			if (return_code)
			{
				if (!source_field)
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_canny_edge_detection_image_filter.  "
						"Missing source field");
					return_code = 0;
				}
			}
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Cmiss_field_module_create_canny_edge_detection_image_filter(
						field_modify->get_field_module(), source_field, variance,
						maximumError, upperThreshold, lowerThreshold));
			}

			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{

					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_canny_edge_detection_image_filter.  Failed");
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
			"define_Computed_field_type_canny_edge_detection_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_canny_edge_detection_image_filter */

int Computed_field_register_types_canny_edge_detection_image_filter(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 30 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_register_types_canny_edge_detection_image_filter);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_canny_edge_detection_image_filter_type_string,
			define_Computed_field_type_canny_edge_detection_image_filter,
			Computed_field_package_get_simple_package(computed_field_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_canny_edge_detection_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_canny_edge_detection_image_filter */
