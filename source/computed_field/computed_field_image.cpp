/*****************************************************************************//**
 * FILE : computed_field_image.cpp
 * 
 * Implements a computed_field which maintains a graphics transformation 
 * equivalent to the scene_viewer assigned to it.
 */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

extern "C" {
#include "computed_field/computed_field.h"
}
#include "computed_field/computed_field_private.hpp"
extern "C" {
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/image_utilities.h"
#include "graphics/texture.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_image.h"
}

class Computed_field_image_package : public Computed_field_type_package
{
public:
	struct MANAGER(Computed_field) *computed_field_manager;
	struct MANAGER(Texture) *texture_manager;
};

namespace {

char computed_field_image_type_string[] = "image";

class Computed_field_image : public Computed_field_core
{
public:
	Texture* texture;
	double minimum;
	double maximum;
	int native_texture;

	Computed_field_image(Computed_field *field) :
		Computed_field_core(field)
	{
		texture = ACCESS(Texture)(CREATE(Texture)(field->name));
		maximum = 1.0;
		minimum = 0.0;
		native_texture = 1;
	};

	~Computed_field_image()
	{	
		if (texture)
		{
			DEACCESS(Texture)(&(texture));
		}
	}

	int set_texture(Texture *texture_in)
	{
		int return_code;
		int new_number_of_components = Texture_get_number_of_components(texture_in);
		if ((field->number_of_components == new_number_of_components)
			|| (!field->manager)
			|| MANAGED_OBJECT_NOT_IN_USE(Computed_field)(field, field->manager))
		{
			REACCESS(Texture)(&texture, texture_in);
			field->number_of_components = new_number_of_components;
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE, "Cmiss_field_image::set_texture.  "
				"New texture has a different number of components but this "
				"cannot change when a field is in use.");
			return_code = 0;
		}
		return (return_code);
	}

	Texture *get_texture()
	{
		return (texture);
	}

	int set_output_range(double minimum_in, double maximum_in)
	{
		minimum = minimum_in;
		maximum = maximum_in;
		return (1);
	}
	
	int set_native_texture_flag(int native_texture_flag_in)
	{
		native_texture = native_texture_flag_in;
		return (1);
	}

private:
	Computed_field_core *copy(Computed_field* new_parent);

	char *get_type_string()
	{
		return(computed_field_image_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int get_native_resolution(int *dimension,
		int **sizes, Computed_field **texture_coordinate_field);	
};

inline Computed_field *Computed_field_cast(
	Cmiss_field_image *image_field)
{
	return (reinterpret_cast<Computed_field*>(image_field));
}

inline Computed_field_image *Computed_field_image_core_cast(
	Cmiss_field_image *image_field)
{
	return (static_cast<Computed_field_image*>(
		reinterpret_cast<Computed_field*>(image_field)->core));	
}

Computed_field_core* Computed_field_image::copy(Computed_field* new_parent)
/*******************************************************************************
LAST MODIFIED : 24 June 2008

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	Computed_field_image* core;

	ENTER(Computed_field_image::copy);
	if (new_parent)
	{
		core = new Computed_field_image(new_parent);
		core->set_texture(texture);
		core->set_native_texture_flag(native_texture);
		core->set_output_range(minimum, maximum);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_image::copy.  "
			"Invalid arguments.");
		core = (Computed_field_image*)NULL;
	}
	LEAVE;

	return (core);
} /* Computed_field_image::copy */

int Computed_field_image::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	Computed_field_image* other;
	int return_code;

	ENTER(Computed_field_image::compare);
	if (field && (other = dynamic_cast<Computed_field_image*>(other_core)))
	{
		if ((texture == other->texture) &&
			(minimum == other->minimum) &&
			(maximum == other->maximum) &&
			(native_texture == other->native_texture))
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_image::compare */

int Computed_field_image::evaluate_cache_at_location(Field_location* location)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	double texture_values[4];
	FE_value texture_coordinate[3];
	int i, number_of_components, return_code;

	ENTER(Computed_field_image::evaluate_cache_at_location);
	if (field && location)
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_location(field, location))
		{
			/* 2. Calculate the field */
			texture_coordinate[0] = 0.0;
			texture_coordinate[1] = 0.0;
			texture_coordinate[2] = 0.0;
			for (i = 0; i < field->source_fields[0]->number_of_components; i++)
			{
				texture_coordinate[i] = field->source_fields[0]->values[i];				
			}
			Texture_get_pixel_values(texture,
				texture_coordinate[0], texture_coordinate[1], texture_coordinate[2],
				texture_values);
			number_of_components = field->number_of_components;
			if (minimum == 0.0)
			{
				if (maximum == 1.0)
				{
					for (i = 0 ; i < number_of_components ; i++)
					{
						field->values[i] =  texture_values[i];
					}
				}
				else
				{
					for (i = 0 ; i < number_of_components ; i++)
					{
						field->values[i] =  texture_values[i] * maximum;
					}
				}
			}
			else
			{
				for (i = 0 ; i < number_of_components ; i++)
				{
					field->values[i] =  minimum +
						texture_values[i] * (maximum - minimum);
				}
			}
			field->derivatives_valid = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_image::evaluate_cache_at_location.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_image::evaluate_cache_at_location */


int Computed_field_image::get_native_resolution(int *dimension,
	int **sizes, Computed_field **texture_coordinate_field)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Gets the <dimension>, <sizes>, <minimums>, <maximums> and <texture_coordinate_field> from
the <field>. These parameters will be used in image processing.

==============================================================================*/
{       
	int return_code;
	int w, h, d;
	
	ENTER(Computed_field_image::get_native_resolution);
	if (field)
	{
		return_code = 1;
		if (native_texture)
		{
			Texture_get_size(texture, &w, &h, &d);
			Texture_get_dimension(texture,dimension);
			if (!(ALLOCATE(*sizes, int, *dimension)))
			{
				return_code = 0;
			}
			if (return_code)
			{
				switch (*dimension)
				{
					default:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_image::get_native_resolution.  "
							"Texture dimension not implemented.");
						return_code=0;
					} break;
					case 3:
					{
						(*sizes)[2] = d;
					} /* no_break */
					case 2:
					{
						(*sizes)[1] = h;
					} /* no_break */
					case 1:
					{
						(*sizes)[0] = w;
					} /* no_break */
				}
			}
			*texture_coordinate_field = field->source_fields[0];
		}
		else
		{
			/* If this field is not the native texture field
				then propagate this query to the texture coordinates
				by calling the default implementation. */
			return_code = Computed_field_core::get_native_resolution(
				dimension, sizes, texture_coordinate_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_image::get_native_resolution.  Missing field");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_image::get_native_resolution */


int Computed_field_image::list()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	char *texture_name;
	int return_code;

	ENTER(List_Computed_field_image);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    texture coordinate field : %s\n",field->source_fields[0]->name);
		if (return_code=GET_NAME(Texture)(texture,&texture_name))
		{
			display_message(INFORMATION_MESSAGE,
				"    texture : %s\n",texture_name);
			DEALLOCATE(texture_name);
		}
		display_message(INFORMATION_MESSAGE,"    minimum : %f\n",minimum);
		display_message(INFORMATION_MESSAGE,"    maximum : %f\n",maximum);
		if (native_texture)
		{
			display_message(INFORMATION_MESSAGE,"    native_texture\n");
		}
		else
		{
			display_message(INFORMATION_MESSAGE,"    not_native_texture\n");
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_image.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_image */

char *Computed_field_image::get_command_string()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40], *texture_name;
	int error;

	ENTER(Computed_field_image::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_image_type_string, &error);
		append_string(&command_string, " coordinates ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}

		append_string(&command_string, " texture ", &error);
		if (GET_NAME(Texture)(texture, &texture_name))
		{
			make_valid_token(&texture_name);
			append_string(&command_string, texture_name, &error);
			DEALLOCATE(texture_name);
		}
		sprintf(temp_string, " minimum %f", minimum);
		append_string(&command_string, temp_string, &error);
		sprintf(temp_string, " maximum %f", maximum);
		append_string(&command_string, temp_string, &error);
		if (native_texture)
		{
			append_string(&command_string, " native_texture", &error);
		}
		else
		{
			append_string(&command_string, " not_native_texture", &error);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_image::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_image::get_command_string */

} //namespace

Cmiss_field_image_id Cmiss_field_image_cast(Cmiss_field_id field)
{
	if (dynamic_cast<Computed_field_image*>(field->core))
	{
		return (reinterpret_cast<Cmiss_field_image_id>(field));
	}
	else
	{
		return (NULL);
	}
}

Computed_field *Computed_field_create_image(Computed_field *domain_field)
{
	int number_of_components, number_of_source_fields;
	Computed_field *field, **source_fields;

	ENTER(Computed_field_create_image);
	if (domain_field && 3>=domain_field->number_of_components)
	{
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		number_of_components = 1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. create new field */
			field = ACCESS(Computed_field)(CREATE(Computed_field)(""));
			/* 3. establish the new type */
			field->number_of_components = number_of_components;
			source_fields[0]=ACCESS(Computed_field)(domain_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			field->core = new Computed_field_image(field);
		}
		else
		{
			DEALLOCATE(source_fields);
			field = (Computed_field *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_image.  Invalid argument(s)");
		field = (Computed_field *)NULL;
	}
	LEAVE;

	return (field);
} /* Computed_field_create_image */

int Computed_field_get_type_image(struct Computed_field *field,
	struct Computed_field **texture_coordinate_field, struct Texture **texture,
	float *minimum, float *maximum, int *native_texture)
/*******************************************************************************
LAST MODIFIED : 5 September 2007

DESCRIPTION :
If the field is of type COMPUTED_FIELD_SAMPLE_TEXTURE, the 
<texture_coordinate_field>, <texture>, <minimum> and <maximum> used by it are 
returned.
==============================================================================*/
{
	Computed_field_image* core;
	int return_code;

	ENTER(Computed_field_get_type_image);
	if (field&&(core = dynamic_cast<Computed_field_image*>(field->core))
		&& texture)
	{
		*texture_coordinate_field = field->source_fields[0];
		*texture = core->texture;
		*minimum = core->minimum;
		*maximum = core->maximum;
		*native_texture = core->native_texture;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_image.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_image */

int define_Computed_field_type_sample_texture(struct Parse_state *state,
	void *field_void,void *computed_field_image_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_IMAGE (if it is not 
already) and allows its contents to be modified.
Maintains legacy version that is set with a texture.
==============================================================================*/
{
	char native_texture_flag, not_native_texture_flag;
	float minimum, maximum;
	int native_texture, return_code;
	struct Computed_field *field,*texture_coordinate_field;
	Computed_field_image_package 
		*computed_field_image_package;
	struct Texture *texture;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_image);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_image_package=
		(Computed_field_image_package *)
		computed_field_image_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		minimum = 0.0;
		maximum = 1.0;
		native_texture = 1;
		texture_coordinate_field = (struct Computed_field *)NULL;
		texture = (struct Texture *)NULL;
		if (computed_field_image_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_image(field,
				&texture_coordinate_field, &texture, &minimum, &maximum,
				&native_texture);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (texture_coordinate_field)
			{
				ACCESS(Computed_field)(texture_coordinate_field);
			}
			if (texture)
			{
				ACCESS(Texture)(texture);
			}
			native_texture_flag = 0;
			not_native_texture_flag = 0;

			option_table = CREATE(Option_table)();
			Option_table_add_help(option_table,
				"The image field allows you to look up the "
				"values of a <texture>.  This sample_texture interface "
				"wraps an existing texture in a image field.  The resulting field will have the same "
				"number of components as the texture it was created from.  "
				"The <coordinates> field is used as the texel location, with "
				"values from 0..texture_width, 0..texture_height and 0..texture_depth "
				"valid coordinates within the image.  "
				"Normally the resulting colour values are real values for 0 to 1.  "
				"The <minimum> and <maximum> values can be used to rerange the colour values.  "
				"The <native_texture> or <not_native_texture> flag indicates whether "
				"this sample texture computed field will supply this textures "
				"dimensions as the default resolution to a modify texture evalutate_image "
				"command that is using this field.  This is normally what you want "
				"but the flag gives you the ability to discriminate which texture "
				"should be used in a pipeline of fields.  "
				"See examples a/reimage, a/create_slices and a/image_sampling.  "
										 );

			/* coordinates */
			set_source_field_data.computed_field_manager=
				computed_field_image_package->computed_field_manager;
			set_source_field_data.conditional_function =
				Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinates",&texture_coordinate_field,
				&set_source_field_data,set_Computed_field_conditional);
			/* maximum */
			Option_table_add_entry(option_table,"maximum",&maximum,
				NULL,set_float);
			/* minimum */
			Option_table_add_entry(option_table,"minimum",&minimum,
				NULL,set_float);
			/* native_texture */
			Option_table_add_char_flag_entry(option_table,"native_texture",
				&native_texture_flag);
			/* not_native_texture */
			Option_table_add_char_flag_entry(option_table,"not_native_texture",
				&not_native_texture_flag);
			/* texture */
			Option_table_add_entry(option_table,"texture",&texture,
				computed_field_image_package->texture_manager,
				set_Texture);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors,not asking for help */
			if (return_code)
			{
				if (native_texture_flag && not_native_texture_flag)
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_image.  "
						"You cannot specify native_texture and not_native_texture.");
 					return_code = 0;					
				}
				if (native_texture_flag)
				{
					native_texture = 1;
				}
				else if (not_native_texture_flag)
				{
					native_texture = 0;
				}
 				if (!texture_coordinate_field)
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_image.  "
						"You must specify a coordinates field.");
 					return_code = 0;
				}
 				if (!texture)
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_image.  "
						"You must specify a texture.");
 					return_code = 0;
				}
			}
			if (return_code)
			{
				return_code = Computed_field_copy_type_specific_and_deaccess(field,
					Computed_field_create_image(texture_coordinate_field));
			}
			if (return_code)
			{
				Computed_field_image *image_field_core = 
					dynamic_cast<Computed_field_image*>(field->core);
				image_field_core->set_texture(texture);
				image_field_core->set_output_range(minimum, maximum);
				image_field_core->set_native_texture_flag(native_texture);
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
			if (texture)
			{
				DEACCESS(Texture)(&texture);
			}
			DESTROY(Option_table)(&option_table);
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

int Computed_field_register_type_image(
	struct Computed_field_package *computed_field_package, 
	struct MANAGER(Texture) *texture_manager)
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
	if (computed_field_package && texture_manager)
	{
		computed_field_image_package->computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		computed_field_image_package->texture_manager =
			texture_manager;
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

int Computed_field_depends_on_texture(struct Computed_field *field,
	struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 4 September 2007

DESCRIPTION :
Returns true if the field or recursively any source fields are sample
texture fields which reference <texture>.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_depends_on_texture);
	if (field)
	{
		return_code=0;
		if (computed_field_image_type_string ==
			Computed_field_get_type_string(field))
		{
			Computed_field_image* image;
			if ((image = dynamic_cast<Computed_field_image*>(
					  field->core)) && (image->texture == texture))
			{
				return_code = 1;
			}
			else
			{
				for (i=0;(i<field->number_of_source_fields)&&(!return_code);i++)
				{
					return_code=Computed_field_depends_on_texture(
						field->source_fields[i], texture);
				}
			}
		}
		else
		{
			for (i=0;(i<field->number_of_source_fields)&&(!return_code);i++)
			{
				return_code=Computed_field_depends_on_texture(
					field->source_fields[i], texture);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_depends_on_texture.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_depends_on_texture */

Cmiss_field_image_storage_information_id Cmiss_field_image_storage_information_create(void)
{
	return ((Cmiss_field_image_storage_information_id)CREATE(Cmgui_image_information)());
}

int Cmiss_field_image_storage_information_destroy(
	Cmiss_field_image_storage_information_id *storage_information_address)
{
	return (DESTROY(Cmgui_image_information)(
		(struct Cmgui_image_information **)storage_information_address));
}

int Cmiss_field_image_storage_information_add_file_name(
	Cmiss_field_image_storage_information_id storage_information,
	const char *file_name)
{
	return (Cmgui_image_information_add_file_name(
		(struct Cmgui_image_information *)storage_information,
		(char *)file_name));
}

int Cmiss_field_image_storage_information_set_format(
	Cmiss_field_image_storage_information_id storage_information,
	enum Cmiss_field_image_storage_format format)
{
	enum Image_file_format cmgui_file_format;
	int return_code;
	
	return_code = 1;
	switch (format)
	{
		case CMISS_FIELD_IMAGE_STORAGE_FORMAT_BMP:
		{
			cmgui_file_format = BMP_FILE_FORMAT;
		} break;
		case CMISS_FIELD_IMAGE_STORAGE_FORMAT_DICOM:
		{
			cmgui_file_format = DICOM_FILE_FORMAT;
		} break;
		case CMISS_FIELD_IMAGE_STORAGE_FORMAT_JPG:
		{
			cmgui_file_format = JPG_FILE_FORMAT;
		} break;
		case CMISS_FIELD_IMAGE_STORAGE_FORMAT_GIF:
		{
			cmgui_file_format = GIF_FILE_FORMAT;
		} break;
		case CMISS_FIELD_IMAGE_STORAGE_FORMAT_PNG:
		{
			cmgui_file_format = PNG_FILE_FORMAT;
		} break;
		case CMISS_FIELD_IMAGE_STORAGE_FORMAT_SGI:
		{
			cmgui_file_format = SGI_FILE_FORMAT;
		} break;
		case CMISS_FIELD_IMAGE_STORAGE_FORMAT_TIFF:
		{
			cmgui_file_format = TIFF_FILE_FORMAT;
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_field_image_storage_information_set_format.  "
				"File format not implemented yet.");
		} break;
	}
	if (return_code)
	{
		return_code = Cmgui_image_information_set_image_file_format(
			(struct Cmgui_image_information *)storage_information,
			cmgui_file_format);
	}
	return (return_code);
}

int Cmiss_field_image_storage_information_set_width(
	Cmiss_field_image_storage_information_id storage_information,
	unsigned int width)
{
	return (Cmgui_image_information_set_width(
		(struct Cmgui_image_information *)storage_information,
		width));
}

int Cmiss_field_image_storage_information_set_height(
	Cmiss_field_image_storage_information_id storage_information,
	unsigned int height)
{
	return (Cmgui_image_information_set_height(
		(struct Cmgui_image_information *)storage_information,
		height));
}

/*
int Cmiss_field_image_storage_information_set_depth(
	Cmiss_field_image_storage_information_id storage_information,
	unsigned int depth)
{
	return (Cmgui_image_information_set_depth(
		(struct Cmgui_image_information *)storage_information,
		depth));
}
*/

int Cmiss_field_image_storage_information_set_pixel_format(
	Cmiss_field_image_storage_information_id storage_information,
	enum Cmiss_field_image_storage_pixel_format pixel_format)
{
	int number_of_components, return_code;
	switch(pixel_format)
	{
		case CMISS_FIELD_IMAGE_STORAGE_PIXEL_FORMAT_LUMINANCE:
		{
			number_of_components = 1;
		} break;
		case CMISS_FIELD_IMAGE_STORAGE_PIXEL_FORMAT_LUMINANCE_ALPHA:
		{
			number_of_components = 2;
		} break;
		case CMISS_FIELD_IMAGE_STORAGE_PIXEL_FORMAT_RGB:
		{
			number_of_components = 3;
		} break;
		case CMISS_FIELD_IMAGE_STORAGE_PIXEL_FORMAT_RGBA:
		{
			number_of_components = 4;
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Texture_set_pixel_format.  Pixel format not implemented yet.");
			number_of_components = 0;
		} break;
	}
	if (number_of_components)
	{
		return_code = Cmgui_image_information_set_number_of_components(
			(struct Cmgui_image_information *)storage_information,
			number_of_components);
	}
	else
	{
		return_code = 0;
	}
	return (return_code);
}

int Cmiss_field_image_storage_information_set_number_of_bytes_per_component(
	Cmiss_field_image_storage_information_id storage_information,
	unsigned int number_of_bytes_per_component)
{
	return (Cmgui_image_information_set_number_of_bytes_per_component(
		(struct Cmgui_image_information *)storage_information,
		number_of_bytes_per_component));
}

int Cmiss_field_image_storage_information_set_compression(
	Cmiss_field_image_storage_information_id storage_information,
	enum Cmiss_field_image_storage_compression compression)
{
	enum Image_storage_compression image_compression;
	int return_code;
	
	return_code = 1;
	switch(compression)
	{
		case CMISS_FIELD_IMAGE_STORAGE_COMPRESSION_UNSPECIFIED:
		{
			image_compression = IMAGE_STORAGE_COMPRESSION_UNSPECIFIED;
		} break;
		case CMISS_FIELD_IMAGE_STORAGE_COMPRESSION_NONE:
		{
			image_compression = IMAGE_STORAGE_COMPRESSION_NONE;
		} break;
		case CMISS_FIELD_IMAGE_STORAGE_COMPRESSION_BZIP:
		{
			image_compression = IMAGE_STORAGE_COMPRESSION_BZIP;
		} break;
		case CMISS_FIELD_IMAGE_STORAGE_COMPRESSION_FAX:
		{
			image_compression = IMAGE_STORAGE_COMPRESSION_FAX;
		} break;
		case CMISS_FIELD_IMAGE_STORAGE_COMPRESSION_JPEG:
		{
			image_compression = IMAGE_STORAGE_COMPRESSION_JPEG;
		} break;
		case CMISS_FIELD_IMAGE_STORAGE_COMPRESSION_JPEG2000:
		{
			image_compression = IMAGE_STORAGE_COMPRESSION_JPEG2000;
		} break;
		case CMISS_FIELD_IMAGE_STORAGE_COMPRESSION_LOSSLESS_JPEG:
		{
			image_compression = IMAGE_STORAGE_COMPRESSION_LOSSLESS_JPEG;
		} break;
		case CMISS_FIELD_IMAGE_STORAGE_COMPRESSION_LZW:
		{
			image_compression = IMAGE_STORAGE_COMPRESSION_LZW;
		} break;
		case CMISS_FIELD_IMAGE_STORAGE_COMPRESSION_RLE:
		{
			image_compression = IMAGE_STORAGE_COMPRESSION_RLE;
		} break;
		case CMISS_FIELD_IMAGE_STORAGE_COMPRESSION_ZIP:
		{
			image_compression = IMAGE_STORAGE_COMPRESSION_ZIP;
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Texture_set_compression.  Compression type not implemented yet.");
			return_code = 0;
		} break;
	}
	if (return_code)
	{
		return_code = Cmgui_image_information_set_storage_compression(
			(struct Cmgui_image_information *)storage_information,
			image_compression);
	}
	return (return_code);
}
	
int Cmiss_field_image_storage_information_set_quality(
	Cmiss_field_image_storage_information_id storage_information,
	double quality)
{
	return (Cmgui_image_information_set_quality(
		(struct Cmgui_image_information *)storage_information,
		quality));
}

int Cmiss_field_image_storage_information_set_memory_block(
	Cmiss_field_image_storage_information_id storage_information,
	void *memory_block, unsigned int memory_block_length)
{
	return (Cmgui_image_information_set_memory_block(
		(struct Cmgui_image_information *)storage_information,
		memory_block, memory_block_length));
}

int Cmiss_field_image_storage_information_set_write_to_memory_block(
	Cmiss_field_image_storage_information_id storage_information)
{
	return (Cmgui_image_information_set_write_to_memory_block(
		(struct Cmgui_image_information *)storage_information));
}

int Cmiss_field_image_storage_information_get_memory_block(
	Cmiss_field_image_storage_information_id storage_information,
	void **memory_block, unsigned int *memory_block_length)
{
	return (Cmgui_image_information_get_memory_block(
		(struct Cmgui_image_information *)storage_information,
		memory_block, memory_block_length));
}

int Cmiss_field_image_read(Cmiss_field_image_id image_field,
	Cmiss_field_image_storage_information_id storage_information)
{
	int return_code;
	struct Cmgui_image_information *cmgui_image_information;
	struct Cmgui_image *cmgui_image;

	ENTER(Cmiss_field_image_read);
	if (image_field && (cmgui_image_information =
		(struct Cmgui_image_information *)storage_information))
	{
		Computed_field *field = Computed_field_cast(image_field);
		Computed_field_image *image_core =
			Computed_field_image_core_cast(image_field);
		
		if (cmgui_image = Cmgui_image_read(cmgui_image_information))
		{
			char *property, *value;
			Texture *texture;

			if ((texture = CREATE(Texture)(field->name)) &&
				Texture_set_image(texture, cmgui_image,
				field->name, /*file_number_pattern*/"",
				/*file_number_series_data.start*/0,
				/*file_number_series_data.stop*/0,
				/*file_number_series_data.increment*/1,
				/*image_data.crop_left_margin*/0,
				/*image_data.crop_bottom_margin*/0,
				/*image_data.crop_width*/0, /*image_data.crop_height*/0))
			{
				/* Calling get_property with wildcard ensures they
					will be available to the iterator, as well as
					any other properties */
				Cmgui_image_get_property(cmgui_image,"exif:*");
				Cmgui_image_reset_property_iterator(cmgui_image);
				while ((property = Cmgui_image_get_next_property(
					cmgui_image)) && 
					(value = Cmgui_image_get_property(cmgui_image,
					property)))
				{
					Texture_set_property(texture, property, value);
					DEALLOCATE(property);
					DEALLOCATE(value);
				}
				DESTROY(Cmgui_image)(&cmgui_image);
				return_code = 1;		
			}
			else
			{
				return_code = 0;
			}
			if (return_code)
			{
				return_code = image_core->set_texture(texture);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
			"Cmiss_field_image_read.  Could not read image file");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_image_read.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_field_image_read */

int Cmiss_field_image_write(Cmiss_field_image_id image_field,
	Cmiss_field_image_storage_information_id storage_information)
{
	int return_code;
	struct Cmgui_image_information *cmgui_image_information;
	struct Cmgui_image *cmgui_image;

	ENTER(Cmiss_field_image_write);
	if (image_field && (cmgui_image_information =
		(struct Cmgui_image_information *)storage_information))
	{
		Computed_field *field = Computed_field_cast(image_field);
		Computed_field_image *image_core =
			Computed_field_image_core_cast(image_field);

		if (cmgui_image = Texture_get_image(image_core->get_texture()))
		{
			if (!Cmgui_image_write(cmgui_image, cmgui_image_information))
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_field_image_write.  "
					"Error writing image %s", field->name);
				return_code = 0;
			}
			DESTROY(Cmgui_image)(&cmgui_image);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_field_image_write.  "
				"Could not get image from texture");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_image_write.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_field_image_write */

int Cmiss_field_image_read_file(Cmiss_field_image_id image_field,
	const char *file_name)
{
	Cmiss_field_image_storage_information_id storage_information;
	int return_code;
	
	storage_information = Cmiss_field_image_storage_information_create();
	Cmiss_field_image_storage_information_add_file_name(
		storage_information, file_name);
	return_code = Cmiss_field_image_read(image_field, storage_information);
	Cmiss_field_image_storage_information_destroy(&storage_information);
	
	return (return_code);
} /* Cmiss_field_image_read_file */

int Cmiss_field_image_write_file(Cmiss_field_image_id image_field,
	const char *file_name)
{
	Cmiss_field_image_storage_information_id storage_information;
	int return_code;
	
	storage_information = Cmiss_field_image_storage_information_create();
	Cmiss_field_image_storage_information_add_file_name(
		storage_information, file_name);
	return_code = Cmiss_field_image_write(image_field, storage_information);
	Cmiss_field_image_storage_information_destroy(&storage_information);

	return (return_code);
} /* Cmiss_field_image_write_file */

