#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "computed_field/computed_field_image.h"

class Computed_field_image_package : public Computed_field_type_package
{
};

char computed_field_image_type_string[] = "image";

int Computed_field_get_type_image(struct Computed_field *field,
	struct Computed_field **texture_coordinate_field,
	struct Computed_field **source_field,
	struct Texture **texture,
	double *minimum, double *maximum, int *native_texture);

int Cmiss_field_image_set_number_of_bytes_per_component(Cmiss_field_image_id image_field, int number_of_bytes_per_component);

int Cmiss_field_image_set_output_range(Cmiss_field_image_id image_field, double minimum, double maximum);

int Cmiss_field_image_set_native_texture_flag(Cmiss_field_image_id image_field, int native_texture_flag);

int define_Computed_field_type_sample_texture(struct Parse_state *state,
	void *field_modify_void,void *computed_field_image_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_IMAGE (if it is not
already) and allows its contents to be modified.
Maintains legacy version that is set with a texture.
==============================================================================*/
{
	double minimum, maximum;
	int return_code;
	struct Computed_field *source_field,*texture_coordinate_field;
	Computed_field_image_package
		*computed_field_image_package;
	Computed_field_modify_data *field_modify;
	struct Texture *texture;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;
	int number_of_bytes_per_component;
	int native_texture = 1;

	ENTER(define_Computed_field_type_image);
	if (state && (field_modify=(Computed_field_modify_data *)field_modify_void) &&
		(computed_field_image_package=
		(Computed_field_image_package *)
		computed_field_image_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		minimum = 0.0;
		maximum = 1.0;
		texture_coordinate_field = (struct Computed_field *)NULL;
		source_field = (struct Computed_field *)NULL;
		texture = (struct Texture *)NULL;
		number_of_bytes_per_component = 1;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_image_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code = Computed_field_get_type_image(field_modify->get_field(),
				&texture_coordinate_field, &source_field, &texture, &minimum, &maximum,
				&native_texture);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (texture_coordinate_field)
			{
				ACCESS(Computed_field)(texture_coordinate_field);
			}
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}

			option_table = CREATE(Option_table)();
			Option_table_add_help(option_table,
				"The image field allows you to look up the "
				"values of a <texture>.  This sample_texture interface "
				"wraps an existing texture in a image field.  The resulting field will have the same "
				"number of components as the texture it was created from.  "
				"If <field> is specified, it will be used as the source and the image field "
				"will create a texture using the field values either when <coordinates> is provided "
				"or source field has a texture coordinates defined already; the format of texture "
				"it creates depends on the number of components of the provided field. "
				"1 component field creates a LUMINANCE texture, "
				"2 component field creates a LUMINANCE_ALPHA texture, "
				"3 component field creates a RGB texture, "
				"4 component field creates a RGBA texture. "
				"The native_texture, maximum and minimum setting does not affect image field "
				"generated from another field. "
				"The <number_of_bytes_per_pixel> values only works with image field based on a field, "
				"it will affect the number of bytes of the generated image. "
				"The <coordinates> field is used as the texel location, with "
				"values from 0..texture_width, 0..texture_height and 0..texture_depth "
				"valid coordinates within the image.  "
				"Normally the resulting colour values are real values for 0 to 1.  "
				"The <minimum> and <maximum> values can be used to rerange the colour values.  "
				"The <native_texture> or <not_native_texture> flag indicates whether "
				"this sample texture computed field will supply this textures "
				"dimensions as the default resolution to a modify texture evaluate_image "
				"command that is using this field.  This is normally what you want "
				"but the flag gives you the ability to discriminate which texture "
				"should be used in a pipeline of fields.  "
				"See examples a/reimage, a/create_slices and a/image_sampling.  "
			);
			/* coordinates */
			set_source_field_data.computed_field_manager=
				field_modify->get_field_manager();
			set_source_field_data.conditional_function =
				Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinates",&texture_coordinate_field,
				&set_source_field_data,set_Computed_field_conditional);
			Option_table_add_entry(option_table, "field", &source_field,
				&set_source_field_data, set_Computed_field_conditional);
			/* specify_number_of_bytes_per_component */
			Option_table_add_entry(option_table,
					"number_of_bytes_per_component",
				&number_of_bytes_per_component,NULL,set_int_non_negative);
			/* maximum */
			Option_table_add_entry(option_table,"maximum",&maximum,
				NULL,set_float);
			/* minimum */
			Option_table_add_entry(option_table,"minimum",&minimum,
				NULL,set_float);
			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			/* no errors,not asking for help */
			if (return_code)
			{
				if (source_field)
				{
					native_texture = 1;
					maximum = 1.0;
					minimum = 0.0;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_image.  "
						"You must specify either a source field.");
					return_code = 0;
				}
			}
			if (return_code)
			{
				Computed_field *field = Computed_field_create_image(
					field_modify->get_field_module(),
					texture_coordinate_field, source_field);
				if (field)
				{
					Cmiss_field_image_id field_image = Cmiss_field_cast_image(field);
					Cmiss_field_image_set_output_range(field_image, minimum, maximum);
					Cmiss_field_image_set_native_texture_flag(field_image, native_texture);
					Cmiss_field_image_set_number_of_bytes_per_component(field_image, number_of_bytes_per_component);
					Cmiss_field_image_destroy(&field_image);
				}
				return_code = field_modify->update_field_and_deaccess(field);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_image.  Failed");
				}
			}
			if (texture_coordinate_field)
			{
				DEACCESS(Computed_field)(&texture_coordinate_field);
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
			"define_Computed_field_type_image.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_image */

int Computed_field_register_types_image(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	Computed_field_image_package
		*computed_field_image_package =
		new Computed_field_image_package;

	ENTER(Computed_field_register_type_image);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_image_type_string,
			define_Computed_field_type_sample_texture,
			computed_field_image_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			"sample_texture",
			define_Computed_field_type_sample_texture,
			computed_field_image_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_type_image.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_type_image */


