/*******************************************************************************
FILE : computed_field_sample_texture.c

LAST MODIFIED : 25 August 2006

DESCRIPTION :
Implements a computed_field which maintains a graphics transformation 
equivalent to the scene_viewer assigned to it.
==============================================================================*/
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
#include "graphics/texture.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_sample_texture.h"
}

class Computed_field_sample_texture_package : public Computed_field_type_package
{
public:
	struct MANAGER(Computed_field) *computed_field_manager;
	struct MANAGER(Texture) *texture_manager;
};

namespace {

char computed_field_sample_texture_type_string[] = "sample_texture";

class Computed_field_sample_texture : public Computed_field_core
{
public:
	Texture* texture;
	float minimum;
	float maximum;

	Computed_field_sample_texture(Computed_field *field, Texture* texture,
		float minimum, float maximum) :
		Computed_field_core(field), texture(ACCESS(Texture)(texture)),
		minimum(minimum), maximum(maximum)	  
	{
	};

	~Computed_field_sample_texture()
	{	
		if (texture)
		{
			DEACCESS(Texture)(&(texture));
		}
	}

private:
	Computed_field_core *copy(Computed_field* new_parent);

	char *get_type_string()
	{
		return(computed_field_sample_texture_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int get_native_resolution(int *dimension,
		int **sizes, Computed_field **texture_coordinate_field);
};

Computed_field_core* Computed_field_sample_texture::copy(Computed_field* new_parent)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	Computed_field_sample_texture* core;

	ENTER(Computed_field_sample_texture::copy);
	if (new_parent)
	{
		core = new Computed_field_sample_texture(new_parent,
			texture, minimum, maximum);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_sample_texture::copy.  "
			"Invalid arguments.");
		core = (Computed_field_sample_texture*)NULL;
	}
	LEAVE;

	return (core);
} /* Computed_field_sample_texture::copy */

int Computed_field_sample_texture::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	Computed_field_sample_texture* other;
	int return_code;

	ENTER(Computed_field_sample_texture::compare);
	if (field && (other = dynamic_cast<Computed_field_sample_texture*>(other_core)))
	{
		if ((texture == other->texture) &&
			(minimum == other->minimum) &&
			(maximum == other->maximum))
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
} /* Computed_field_sample_texture::compare */

int Computed_field_sample_texture::evaluate_cache_at_location(Field_location* location)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	double texture_values[4];
	FE_value texture_coordinate[3];
	int i, number_of_components, return_code;

	ENTER(Computed_field_sample_texture::evaluate_cache_at_location);
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
			"Computed_field_sample_texture::evaluate_cache_at_location.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_sample_texture::evaluate_cache_at_location */


int Computed_field_sample_texture::get_native_resolution(int *dimension,
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
	
	ENTER(Computed_field_sample_texture::get_native_resolution);
	if (field)
	{
		return_code = 1;
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
						"Computed_field_sample_texture::get_native_resolution.  "
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
		display_message(ERROR_MESSAGE,
			"Computed_field_sample_texture::get_native_resolution.  Missing field");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_sample_texture::get_native_resolution */


int Computed_field_sample_texture::list()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	char *texture_name;
	int return_code;

	ENTER(List_Computed_field_sample_texture);
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
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_sample_texture.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_sample_texture */

char *Computed_field_sample_texture::get_command_string()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40], *texture_name;
	int error;

	ENTER(Computed_field_sample_texture::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_sample_texture_type_string, &error);
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
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_sample_texture::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_sample_texture::get_command_string */

} //namespace

int Computed_field_set_type_sample_texture(struct Computed_field *field,
	struct Computed_field *texture_coordinate_field, struct Texture *texture,
	float minimum, float maximum)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_SAMPLE_TEXTURE with the supplied
texture.  Sets the number of components to equal the number of components in
the texture.
The returned values are scaled so that they range from <minimum> to <maximum>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_components, number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_sample_texture);
	if (field&&texture&&(3>=texture_coordinate_field->number_of_components))
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		number_of_components = Texture_get_number_of_components(texture);
		if (number_of_components <= 4)
		{
			/* The Computed_field_sample_texture_evaluate_* code assumes 4 or less components */
			if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
			{
				/* 2. free current type-specific data */
				Computed_field_clear_type(field);
				/* 3. establish the new type */
				field->number_of_components = number_of_components;
				source_fields[0]=ACCESS(Computed_field)(texture_coordinate_field);
				field->source_fields=source_fields;
				field->number_of_source_fields=number_of_source_fields;
				field->core = new Computed_field_sample_texture(field, 
					texture, minimum, maximum);
			}
			else
			{
				DEALLOCATE(source_fields);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_sample_texture.  "
				"Textures with more than four components are not supported.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_sample_texture.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_sample_texture */

int Computed_field_get_type_sample_texture(struct Computed_field *field,
	struct Computed_field **texture_coordinate_field, struct Texture **texture,
	float *minimum, float *maximum)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_SAMPLE_TEXTURE, the 
<texture_coordinate_field>, <texture>, <minimum> and <maximum> used by it are 
returned.
==============================================================================*/
{
	Computed_field_sample_texture* core;
	int return_code;

	ENTER(Computed_field_get_type_sample_texture);
	if (field&&(core = dynamic_cast<Computed_field_sample_texture*>(field->core))
		&& texture)
	{
		*texture_coordinate_field = field->source_fields[0];
		*texture = core->texture;
		*minimum = core->minimum;
		*maximum = core->maximum;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_sample_texture.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_sample_texture */

int define_Computed_field_type_sample_texture(struct Parse_state *state,
	void *field_void,void *computed_field_sample_texture_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_SAMPLE_TEXTURE (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	float minimum, maximum;
	int return_code;
	struct Computed_field *field,*texture_coordinate_field;
	Computed_field_sample_texture_package 
		*computed_field_sample_texture_package;
	struct Texture *texture;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_sample_texture);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_sample_texture_package=
		(Computed_field_sample_texture_package *)
		computed_field_sample_texture_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		minimum = 0.0;
		maximum = 1.0;
		texture_coordinate_field = (struct Computed_field *)NULL;
		texture = (struct Texture *)NULL;
		if (computed_field_sample_texture_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_sample_texture(field,
				&texture_coordinate_field, &texture, &minimum, &maximum);
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

			option_table = CREATE(Option_table)();
			/* coordinates */
			set_source_field_data.computed_field_manager=
				computed_field_sample_texture_package->computed_field_manager;
			set_source_field_data.conditional_function=Computed_field_has_at_least_2_components;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinates",&texture_coordinate_field,
				&set_source_field_data,set_Computed_field_conditional);
			/* maximum */
			Option_table_add_entry(option_table,"maximum",&maximum,
				NULL,set_float);
			/* minimum */
			Option_table_add_entry(option_table,"minimum",&minimum,
				NULL,set_float);
			/* texture */
			Option_table_add_entry(option_table,"texture",&texture,
				computed_field_sample_texture_package->texture_manager,
				set_Texture);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors,not asking for help */
			if (return_code)
			{
 				if (!texture_coordinate_field)
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_sample_texture.  "
						"You must specify a coordinates field.");
 					return_code = 0;
				}
 				if (!texture)
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_sample_texture.  "
						"You must specify a texture.");
 					return_code = 0;
				}
			}
			if (return_code)
			{
				return_code = Computed_field_set_type_sample_texture(field,
					texture_coordinate_field, texture, minimum, maximum);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_sample_texture.  Failed");
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
			"define_Computed_field_type_sample_texture.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_sample_texture */

int Computed_field_register_type_sample_texture(
	struct Computed_field_package *computed_field_package, 
	struct MANAGER(Texture) *texture_manager)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	Computed_field_sample_texture_package
		*computed_field_sample_texture_package =
		new Computed_field_sample_texture_package;

	ENTER(Computed_field_register_type_sample_texture);
	if (computed_field_package && texture_manager)
	{
		computed_field_sample_texture_package->computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		computed_field_sample_texture_package->texture_manager =
			texture_manager;
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_sample_texture_type_string, 
			define_Computed_field_type_sample_texture,
			computed_field_sample_texture_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_type_sample_texture.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_type_sample_texture */

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
		if (computed_field_sample_texture_type_string ==
			Computed_field_get_type_string(field))
		{
			Computed_field_sample_texture* sample_texture;
			if ((sample_texture = dynamic_cast<Computed_field_sample_texture*>(
					  field->core)) && (sample_texture->texture == texture))
			{
				return_code = 1;
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

