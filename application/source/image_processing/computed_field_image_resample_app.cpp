
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "image_processing/computed_field_image_resample.h"

const char computed_field_image_resample_type_string[] = "image_resample";

int Cmiss_field_get_type_image_resample(struct Computed_field *field,
	struct Computed_field **source_field, int *dimension, int **sizes);

int define_Computed_field_type_image_resample(struct Parse_state *state,
	void *field_modify_void,void *computed_field_simple_package_void)
/*******************************************************************************
LAST MODIFIED : 7 March 2007

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_IMAGE_RESAMPLE (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int dimension, return_code, original_dimension, *sizes, *original_sizes;
	Computed_field *source_field, *texture_coordinate_field;
	Computed_field_modify_data *field_modify;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_image_resample);
	USE_PARAMETER(computed_field_simple_package_void);
	if (state && (field_modify=(Computed_field_modify_data *)field_modify_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		original_dimension = 0;
		original_sizes = (int *)NULL;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_image_resample_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code=Cmiss_field_get_type_image_resample(field_modify->get_field(),
				&source_field, &original_dimension, &original_sizes);
			ACCESS(Computed_field)(source_field);
		}
		if (return_code)
		{
			if (state->current_token &&
				(!(strcmp(PARSER_HELP_STRING, state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token))))
			{
				/* Handle help separately */
				option_table = CREATE(Option_table)();
			Option_table_add_help(option_table,
				"The image_resample field resamples the field to a new user specified size. It is especially useful for resizing image based fields.  The new size of the field is specified by using the <sizes> option with a list of values for the new size in each dimension.  See a/testing/image_processing_2D for an example of using this field.");

				/* source field */
				set_source_field_data.computed_field_manager=
					field_modify->get_field_manager();
				set_source_field_data.conditional_function=
					Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				Option_table_add_entry(option_table,"field",&source_field,
					&set_source_field_data,set_Computed_field_conditional);
				/* sizes */
				Option_table_add_int_vector_entry(option_table,
					"sizes", original_sizes, &original_dimension);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
		}
		if (return_code)
		{
			option_table = CREATE(Option_table)();
			/* source field */
			set_source_field_data.computed_field_manager=
				field_modify->get_field_manager();
			set_source_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"field",&source_field,
				&set_source_field_data,set_Computed_field_conditional);

			return_code = Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);

			if (!source_field)
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_time_image_resample.  "
					"You must specify the source field first.");
				return_code = 0;
			}
		}
		if (return_code)
		{
			return_code = Computed_field_get_native_resolution(
				source_field, &dimension, &sizes, &texture_coordinate_field);
			if (original_sizes && (original_dimension == dimension))
			{
				DEALLOCATE(sizes);
				sizes = original_sizes;
			}
			else
			{
				DEALLOCATE(original_sizes);
			}

			/* parse the rest of the table */
			if (return_code&&state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* sizes */
				Option_table_add_int_vector_entry(option_table,
					"sizes", sizes, &dimension);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			if (return_code)
			{
				return_code = field_modify->update_field_and_deaccess(
					Cmiss_field_module_create_image_resample(field_modify->get_field_module(),
						source_field, dimension, sizes));
			}

			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_time_image_resample.  Failed");
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			DESTROY(Option_table)(&option_table);
			DEALLOCATE(sizes);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_image_resample.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_image_resample */

int Computed_field_register_types_image_resample(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_register_types_image_resample);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_image_resample_type_string,
			define_Computed_field_type_image_resample,
			Computed_field_package_get_simple_package(computed_field_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_image_resample.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_image_resample */

