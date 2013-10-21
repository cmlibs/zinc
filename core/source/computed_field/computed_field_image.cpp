/**
 * FILE : computed_field_image.cpp
 *
 * Derived field type that stores an image buffer suitable for I/O and
 * texturing.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "zinc/zincconfigure.h"
#include "zinc/status.h"
#include "zinc/stream.h"
#include "zinc/fieldmodule.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_find_xi_graphics.h"
#include "computed_field/computed_field_set.h"
#include "computed_field/field_module.hpp"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/image_utilities.h"
#include "graphics/material.h"
#include "graphics/texture.h"
#include "general/message.h"
#include "computed_field/computed_field_image.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_finite_element.h"
#include <math.h>
#include "general/enumerator_conversion.hpp"

class Computed_field_image_package : public Computed_field_type_package
{
};

namespace {

const char computed_field_image_type_string[] = "image";

class Computed_field_image : public Computed_field_core
{
public:
	Texture* texture;
	double minimum;
	double maximum;
	int native_texture;
	int number_of_bytes_per_component;
	/* Flag to indicate that the texture needs to be evaluated
		 due to changes on the source fields */
	bool need_evaluate_texture;

	Computed_field_image(Texture *texture_in = NULL) :
		Computed_field_core(),
		texture(NULL)
	{
		if (texture_in)
		{
			texture = ACCESS(Texture)(texture_in);
		}
		maximum = 1.0;
		minimum = 0.0;
		native_texture = 1;
		number_of_bytes_per_component = 1;
		need_evaluate_texture = false;
	}

	virtual bool attach_to_field(Computed_field *parent)
	{
		if (Computed_field_core::attach_to_field(parent))
		{
			if (texture_is_evaluated_from_source_field())
			{
				need_evaluate_texture = true;
			}
			if (NULL == texture)
			{
				texture = ACCESS(Texture)(CREATE(Texture)(field->name));
			}
			if (texture)
			{
				return true;
			}
		}
		return false;
	}

	~Computed_field_image()
	{
		if (texture)
		{
			DEACCESS(Texture)(&(texture));
		}
	}

	/**
	 * Warning: assumes core is either unattached to field, or caller holds one accessed
	 * reference to field containing this core,
	 * otherwise check on whether number of components can change is dangerous.
	 */
	int set_texture(Texture *texture_in)
	{
		int return_code = 0;
		if (field)
		{
			int new_number_of_components;
			if (texture_is_evaluated_from_source_field())
			{
				new_number_of_components = Computed_field_get_number_of_components(
					field->source_fields[1]);
			}
			else if (texture_in)
			{
				new_number_of_components = Texture_get_number_of_components(texture_in);
			}
			else
			{
				new_number_of_components = field->number_of_components;
			}
			if ((field->number_of_components == new_number_of_components)
				|| (MANAGED_OBJECT_NOT_IN_USE(Computed_field)(field, field->manager) ||
					Computed_field_is_not_source_field_of_others(field)))
			{
				REACCESS(Texture)(&texture, texture_in);
				field->number_of_components = new_number_of_components;
				Computed_field_changed(this->field);
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE, "cmzn_field_image::set_texture.  "
					"New texture has a different number of components but this "
					"cannot change when a field is in use.");
				return_code = 0;
			}
		}
		else
		{
			// just called from copy() - copy texture reference
			REACCESS(Texture)(&texture, texture_in);
			Computed_field_changed(this->field);
			return_code = 1;
		}
		return (return_code);
	}

	Texture *get_texture()
	{
		check_evaluate_texture();
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

	int set_number_of_bytes_per_component(int number_of_bytes_per_component_in)
	{
		number_of_bytes_per_component = number_of_bytes_per_component_in;
		return (1);
	}

private:

	int evaluate_texture_from_source_field();

	void check_evaluate_texture()
	{
		if (need_evaluate_texture)
		{
			evaluate_texture_from_source_field();
		}
	}

	static void field_manager_change(
		MANAGER_MESSAGE(Computed_field) *message, void *image_field_core_void);

	Computed_field_core *copy();

	const char *get_type_string()
	{
		return(computed_field_image_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	int list();

	char* get_command_string();

	int get_native_resolution(int *dimension,
		int **sizes, Computed_field **texture_coordinate_field);

	bool texture_is_evaluated_from_source_field()
	{
		return (field && (field->number_of_source_fields > 1));
	}

	virtual int check_dependency()
	{
		int return_code = Computed_field_core::check_dependency();
		if (return_code && texture_is_evaluated_from_source_field())
		{
			need_evaluate_texture = true;
		}
		return return_code;
	}
};

inline Computed_field *Computed_field_cast(
	cmzn_field_image *image_field)
{
	return (reinterpret_cast<Computed_field*>(image_field));
}

inline Computed_field_image *Computed_field_image_core_cast(
	cmzn_field_image *image_field)
{
	return (static_cast<Computed_field_image*>(
		reinterpret_cast<Computed_field*>(image_field)->core));
}

Computed_field_core* Computed_field_image::copy()
/*******************************************************************************
LAST MODIFIED : 24 June 2008

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	Computed_field_image* core = new Computed_field_image(texture);
	core->set_native_texture_flag(native_texture);
	core->set_output_range(minimum, maximum);
	core->set_number_of_bytes_per_component(number_of_bytes_per_component);

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

int Computed_field_image::evaluate_texture_from_source_field()
{
	int image_width = -1, image_height = -1, image_depth = -1, bytes_per_pixel,
	  number_of_components = 1, dimension, *sizes,
		return_code, tex_number_of_components = 1,
		use_pixel_location = 1, source_field_is_image = 0;
	struct Computed_field *texture_coordinate_field = NULL, *source_field = NULL,
		*source_texture_coordinate_field = NULL;
	enum Texture_storage_type specify_format = TEXTURE_LUMINANCE;

	ENTER(Computed_field_image::evaluate_texture_from_source_field);
	if (texture && texture_is_evaluated_from_source_field())
	{
		if (Computed_field_is_image_type(field->source_fields[1], NULL))
		{
			return_code = 1;
			source_field_is_image = 1;
		}
		else
		{
			/* Setup sizes */
			sizes = (int *)NULL;
			texture_coordinate_field = field->source_fields[0];
			source_field = field->source_fields[1];
			if (Computed_field_get_native_resolution(source_field,
					&dimension, &sizes, &source_texture_coordinate_field))
			{
				if (dimension > 0)
				{
					image_width = sizes[0];
				}
				else
				{
					image_width = 1;
				}

				if (dimension > 1)
				{
					image_height = sizes[1];
				}
				else
				{
					image_height = 1;
				}

				if (dimension > 2)
				{
					image_depth = sizes[2];
				}
				else
				{
					image_depth = 1;
				}
				DEALLOCATE(sizes);
			}

			if (3 >= (tex_number_of_components =
				Computed_field_get_number_of_components(texture_coordinate_field)))
			{
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_image::evaluate_texture_from_source_field.  Invalid texture_coordinate field.");
				return_code = 0;
			}

			number_of_components =
				Computed_field_get_number_of_components(source_field);
			if (number_of_components == 1)
			{
				specify_format = TEXTURE_LUMINANCE;
			}
			else if (number_of_components == 2)
			{
				specify_format = TEXTURE_LUMINANCE_ALPHA;
			}
			else if (number_of_components == 3)
			{
				specify_format = TEXTURE_RGB;
			}
			else if (number_of_components == 4)
			{
				specify_format = TEXTURE_RGBA;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_image::evaluate_texture_from_source_field. No texture format supports"
					"the number of components in source field.");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_image::evaluate_texture_from_source_field.  Invalid argument(s)");
		return_code = 0;

	}

	if (return_code)
	{
		/* allocate the texture image */
		if (source_field_is_image)
		{
			cmzn_field_image_id source_image_field = cmzn_field_cast_image(field->source_fields[1]);
			REACCESS(Texture)(&texture, cmzn_field_image_get_texture(source_image_field));
			cmzn_field_image_destroy(&source_image_field);
			need_evaluate_texture = false;
		}
		else if (Texture_allocate_image(texture, image_width, image_height,
				image_depth, specify_format, number_of_bytes_per_component, field->name))
		{
			use_pixel_location = (texture_coordinate_field == source_texture_coordinate_field);
			bytes_per_pixel = number_of_components * number_of_bytes_per_component;
			// search_mesh is used to find domain location of texture coordinate field
			// and evaluate source field there; as defined here it gives no scope to
			// optimise, e.g. by searching through a subgroup of the mesh
			const int number_of_texture_coordinate_components =
				Computed_field_get_number_of_components(texture_coordinate_field);
			cmzn_fieldmodule_id field_module = cmzn_field_get_fieldmodule(field);
			cmzn_mesh_id search_mesh = cmzn_fieldmodule_find_mesh_by_dimension(field_module,
				number_of_texture_coordinate_components);
			Set_cmiss_field_value_to_texture(source_field, texture_coordinate_field,
				texture, NULL,	NULL, image_height, image_width, image_depth, bytes_per_pixel,
				number_of_bytes_per_component, use_pixel_location, specify_format, 0, NULL,
				search_mesh);
			cmzn_mesh_destroy(&search_mesh);
			cmzn_fieldmodule_destroy(&field_module);
			need_evaluate_texture = false;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_image::evaluate_texture_from_source_field.  Could not allocate image in texture");
			return_code = 0;
		}
	}
	LEAVE;

	return (return_code);
} /* Computed_field_image::evaluate_texture_from_source_field */

int Computed_field_image::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	check_evaluate_texture();
	if (texture)
	{
		RealFieldValueCache *sourceCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(cache));
		if (sourceCache)
		{
			RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
			double texture_values[4];
			FE_value texture_coordinate[3];
			texture_coordinate[0] = 0.0;
			texture_coordinate[1] = 0.0;
			texture_coordinate[2] = 0.0;
			for (int i = 0; i < field->source_fields[0]->number_of_components; i++)
			{
				texture_coordinate[i] = sourceCache->values[i];
			}
			Texture_get_pixel_values(texture,
				texture_coordinate[0], texture_coordinate[1], texture_coordinate[2],
				texture_values);
			int number_of_components = field->number_of_components;
			if (minimum == 0.0)
			{
				if (maximum == 1.0)
				{
					for (int i = 0 ; i < number_of_components ; i++)
					{
						valueCache.values[i] =  texture_values[i];
					}
				}
				else
				{
					for (int i = 0 ; i < number_of_components ; i++)
					{
						valueCache.values[i] =  texture_values[i] * maximum;
					}
				}
			}
			else
			{
				for (int i = 0 ; i < number_of_components ; i++)
				{
					valueCache.values[i] =  minimum +
						texture_values[i] * (maximum - minimum);
				}
			}
			valueCache.derivatives_valid = 0;
			return 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_image::evaluate.  No texture");
	}
	return 0;
}


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
		check_evaluate_texture();
		if (native_texture && texture)
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
		return_code=GET_NAME(Texture)(texture,&texture_name);
		if (return_code)
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
		if (texture_is_evaluated_from_source_field())
		{
			append_string(&command_string, " field ", &error);
			if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
			{
				make_valid_token(&field_name);
				append_string(&command_string, field_name, &error);
				DEALLOCATE(field_name);
				sprintf(temp_string, " number_of_bytes_per_component %d", number_of_bytes_per_component);
				append_string(&command_string, temp_string, &error);
			}
		}
		else
		{
			append_string(&command_string, " texture ", &error);
			if (GET_NAME(Texture)(texture, &texture_name))
			{
				make_valid_token(&texture_name);
				append_string(&command_string, texture_name, &error);
				DEALLOCATE(texture_name);
			}
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

cmzn_field_image_id cmzn_field_cast_image(cmzn_field_id field)
{
	if (field && (dynamic_cast<Computed_field_image*>(field->core)))
	{
		cmzn_field_access(field);
		return (reinterpret_cast<cmzn_field_image_id>(field));
	}
	else
	{
		return (NULL);
	}
}

cmzn_field_id cmzn_fieldmodule_create_field_image(cmzn_fieldmodule_id field_module)
{
	cmzn_field_id field = 0;
	if (field_module)
	{
		cmzn_field_id domainField = cmzn_fieldmodule_get_or_create_xi_field(field_module);
		field = Computed_field_create_generic(field_module,
			/*check_source_field*/false,
			/*number_of_components*/1,
			/*number_of_source_fields*/1, &domainField,
			/*number_of_source_values*/0, NULL,
			new Computed_field_image());
		cmzn_field_destroy(&domainField);
	}
	return field;
}

cmzn_field_id cmzn_fieldmodule_create_field_image_from_source(
	cmzn_fieldmodule_id field_module, cmzn_field_id source_field)
{
	cmzn_field_id field = 0;
	if (field_module && source_field &&
		Computed_field_has_up_to_4_numerical_components(source_field, 0))
	{
		cmzn_field_id domainField = 0;
		int source_dimension, *source_sizes = 0;
		if (Computed_field_get_native_resolution(source_field,
			&source_dimension, &source_sizes, &domainField) && (0 != domainField))
		{
			if (source_sizes)
			{
				DEALLOCATE(source_sizes);
				int number_of_source_fields = 2;
				cmzn_field_id source_fields[2] = { domainField, source_field };
				field = Computed_field_create_generic(field_module,
					/*check_source_field*/true,
					cmzn_field_get_number_of_components(source_field),
					number_of_source_fields, source_fields,
					/*number_of_source_values*/0, static_cast<const double *>(0),
					new Computed_field_image());
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"cmzn_fieldmodule_create_field_image_from_source.  "
					"Source field does not contain any information about sizes."
					"You may consider using image_resample field as the source field");
			}
		}
	}
	return field;
}

int cmzn_field_image_set_output_range(cmzn_field_image_id image_field, double minimum, double maximum)
{
	int return_code = 0;
	if (image_field)
	{
		Computed_field_image *image_core = Computed_field_image_core_cast(image_field);
		return_code = image_core->set_output_range(minimum, maximum);
	}

	return return_code;
}

int cmzn_field_image_set_number_of_bytes_per_component(cmzn_field_image_id image_field, int number_of_bytes_per_component)
{
	int return_code = 0;
	if (image_field)
	{
		Computed_field_image *image_core = Computed_field_image_core_cast(image_field);
		return_code = image_core->set_number_of_bytes_per_component(number_of_bytes_per_component);
	}

	return return_code;
}

int cmzn_field_image_set_native_texture_flag(cmzn_field_image_id image_field, int native_texture_flag)
{
	int return_code = 0;
	if (image_field)
	{
		Computed_field_image *image_core = Computed_field_image_core_cast(image_field);
		return_code = image_core->set_native_texture_flag(native_texture_flag);
	}

	return return_code;
}

cmzn_texture *cmzn_field_image_get_texture(cmzn_field_image_id image_field)
{
	cmzn_texture *cmiss_texture = 0;
	if (image_field)
	{
		Computed_field_image *image_core = Computed_field_image_core_cast(image_field);
		cmiss_texture = image_core->get_texture();
	}
	return cmiss_texture;
}

int Computed_field_get_type_image(struct Computed_field *field,
	struct Computed_field **texture_coordinate_field,
	struct Computed_field **source_field,
	struct Texture **texture,
	double *minimum, double *maximum, int *native_texture)
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
		if (field->number_of_source_fields > 1) // texture_is_evaluated_from_source_field()
		{
			*source_field = field->source_fields[1];
			/* do not return a texture here as it is generated
				 from the source field */
			*texture = (struct Texture *)NULL;
		}
		else
		{
			*source_field = (struct Computed_field *)NULL;
			*texture = core->texture;
		}
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

int Computed_field_depends_on_texture(struct Computed_field *field,
	struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 4 September 2007

DESCRIPTION :
Returns true if the field or recursively any source fields are sample
texture fields which reference <texture>.
==============================================================================*/
{
	int return_code = 0;

	ENTER(Computed_field_depends_on_texture);
	if (field)
	{
		Computed_field_image* image =
			dynamic_cast<Computed_field_image*>(field->core);
		if ((NULL != image) && (image->texture == texture))
		{
			return_code = 1;
		}
		else
		{
			for (int i=0;(i<field->number_of_source_fields)&&(!return_code);i++)
			{
				return_code = Computed_field_depends_on_texture(
					field->source_fields[i], texture);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_depends_on_texture.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Computed_field_depends_on_texture */

int cmzn_field_image_destroy(cmzn_field_image_id *image_address)
{
	return cmzn_field_destroy(reinterpret_cast<cmzn_field_id *>(image_address));
}

int cmzn_field_image_get_width_in_pixels(cmzn_field_image_id image)
{
	if (image)
	{
		int width = 0, height = 0, depth = 0;
		cmzn_texture *texture = cmzn_field_image_get_texture(image);
		Texture_get_original_size(texture, &width, &height, &depth);
		return width;
	}
	return 0;
}

int cmzn_field_image_get_height_in_pixels(cmzn_field_image_id image)
{
	if (image)
	{
		int width = 0, height = 0, depth = 0;
		cmzn_texture *texture = cmzn_field_image_get_texture(image);
		Texture_get_original_size(texture, &width, &height, &depth);
		return height;
	}
	return 0;
}

int cmzn_field_image_get_depth_in_pixels(cmzn_field_image_id image)
{
	if (image)
	{
		int width = 0, height = 0, depth = 0;
		cmzn_texture *texture = cmzn_field_image_get_texture(image);
		Texture_get_original_size(texture, &width, &height, &depth);
		return depth;
	}
	return 0;
}

int cmzn_field_image_get_size_in_pixels(cmzn_field_image_id image,
	int valuesCount, int *valuesOut)
{
	if (image)
	{
		cmzn_texture *texture = cmzn_field_image_get_texture(image);
		return cmzn_texture_get_pixel_sizes(texture,
			valuesCount, valuesOut);
	}
	return 0;
}

double cmzn_field_image_get_texture_coordinate_depth(cmzn_field_image_id image)
{
	if (image)
	{
		ZnReal width = 0.0, height = 0.0, depth = 0.0;
		cmzn_texture *texture = cmzn_field_image_get_texture(image);
		Texture_get_physical_size(texture, &width, &height, &depth);
		return depth;
	}
	return 0.0;
}

double cmzn_field_image_get_texture_coordinate_height(cmzn_field_image_id image)
{
	if (image)
	{
		ZnReal width = 0.0, height = 0.0, depth = 0.0;
		cmzn_texture *texture = cmzn_field_image_get_texture(image);
		Texture_get_physical_size(texture, &width, &height, &depth);
		return height;
	}
	return 0.0;
}

double cmzn_field_image_get_texture_coordinate_width(cmzn_field_image_id image)
{
	if (image)
	{
		ZnReal width = 0.0, height = 0.0, depth = 0.0;
		cmzn_texture *texture = cmzn_field_image_get_texture(image);
		Texture_get_physical_size(texture, &width, &height, &depth);
		return width;
	}
	return 0.0;
}

int cmzn_field_image_get_texture_coordinate_sizes(cmzn_field_image_id image,
	int valuesCount, double *valuesOut)
{
	if (image)
	{
		cmzn_texture *texture = cmzn_field_image_get_texture(image);
		return cmzn_texture_get_texture_coordinate_sizes(texture,
			valuesCount, valuesOut);
	}
	return 0;
}

int cmzn_field_image_set_texture_coordinate_depth(cmzn_field_image_id image, double value)
{
	if (image)
	{
		double width = 0.0, height = 0.0, depth = 0.0;
		cmzn_texture *texture = cmzn_field_image_get_texture(image);
		Texture_get_physical_size(texture, &width, &height, &depth);
		Texture_set_physical_size(texture, width, height, value);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_field_image_set_texture_coordinate_width(cmzn_field_image_id image, double value)
{
	if (image)
	{
		double width = 0.0, height = 0.0, depth = 0.0;
		cmzn_texture *texture = cmzn_field_image_get_texture(image);
		Texture_get_physical_size(texture, &width, &height, &depth);
		Texture_set_physical_size(texture, value, height, depth);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_field_image_set_texture_coordinate_height(cmzn_field_image_id image, double value)
{
	if (image)
	{
		double width = 0.0, height = 0.0, depth = 0.0;
		cmzn_texture *texture = cmzn_field_image_get_texture(image);
		Texture_get_physical_size(texture, &width, &height, &depth);
		Texture_set_physical_size(texture, width, value, depth);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_field_image_set_texture_coordinate_sizes(cmzn_field_image_id image,
	int valuesCount, const double *valuesIn)
{
	if (image)
	{
		cmzn_texture *texture = cmzn_field_image_get_texture(image);
		return cmzn_texture_set_texture_coordinate_sizes(texture,	valuesCount, valuesIn);
	}
	return CMZN_ERROR_ARGUMENT;
}

int Computed_field_is_image_type(struct Computed_field *field,
	void *dummy_void)
{
	int return_code = 0;
	struct Computed_field *texture_coordinate_field;
	struct Computed_field *source_field;
	struct Texture *texture;
	double minimum, maximum;
	int native_texture;

	ENTER(Computed_field_has_string_value_type);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		if (dynamic_cast<Computed_field_image*>(field->core))
		{
			return_code = Computed_field_get_type_image(field,
				&texture_coordinate_field, &source_field,
				&texture, &minimum, &maximum, &native_texture);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_image_type.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_has_string_value_type */


int cmzn_field_image_set_texture(cmzn_field_image_id image_field,
		struct Texture *texture)
{
	int return_code;

	ENTER(cmzn_field_image_read);
	if (image_field && texture)
	{
		Computed_field_image *image_core =
			Computed_field_image_core_cast(image_field);
		return_code = image_core->set_texture(texture);
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"cmzn_field_image_set_texture.  Could not set texture");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_field_image_read.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_field_image_read */

int list_image_field(struct Computed_field *field,void *dummy_void)
{
	int return_code = 0;
	USE_PARAMETER(dummy_void);

	if (field)
	{
		return_code = 1;
		if (Computed_field_is_image_type(field, NULL))
		{
			cmzn_field_image_id image_field = cmzn_field_cast_image(field);
			struct Texture *texture = cmzn_field_image_get_texture(image_field);
			cmzn_field_image_destroy(&image_field);
			if (texture)
			{
				return_code = list_Texture(texture,NULL);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_image_field.  Invalid argument(s)");
	}

	return (return_code);
}

int list_image_field_commands(struct Computed_field *field,void *command_prefix_void)
{
	int return_code = 0;

	if (field)
	{
		return_code = 1;
		if (Computed_field_is_image_type(field, NULL))
		{
			cmzn_field_image_id image_field = cmzn_field_cast_image(field);
			struct Texture *texture = cmzn_field_image_get_texture(image_field);
			cmzn_field_image_destroy(&image_field);
			if (texture)
			{
				return_code = list_Texture_commands(texture,command_prefix_void);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_image_field_commands.  Invalid argument(s)");
	}

	return (return_code);
}

int Set_cmiss_field_value_to_texture(struct cmzn_field *field, struct cmzn_field *texture_coordinate_field,
		struct Texture *texture, struct cmzn_spectrum *spectrum,	struct cmzn_material *fail_material,
		int image_height, int image_width, int image_depth, int bytes_per_pixel, int number_of_bytes_per_component,
		int use_pixel_location,	enum Texture_storage_type specify_format, int propagate_field,
		struct Graphics_buffer_package *graphics_buffer_package, cmzn_mesh_id search_mesh)
{
	unsigned char *image_plane, *ptr = 0;
	unsigned short *two_bytes_image_plane, *two_bytes_ptr = 0;
	int dimension, i, image_width_bytes, j, k, l,
		number_of_data_components, return_code = 1;
	FE_value *data_values, values[3], xi[3];
	ZnReal hint_minimums[3] = {0.0, 0.0, 0.0};
	ZnReal hint_maximums[3];
	ZnReal hint_resolution[3];
	ZnReal multiplier;
	struct Colour fail_colour = {0.0, 0.0, 0.0};
	ZnReal rgba[4], fail_alpha = 0.0, texture_depth, texture_height,
		texture_width;
	struct Computed_field_find_element_xi_cache *cache = NULL;
	unsigned long field_evaluate_error_count, find_element_xi_error_count,
		spectrum_render_error_count, total_number_of_pixels;
	struct FE_element *element = NULL;

	int mesh_dimension = cmzn_mesh_get_dimension(search_mesh);
	cmzn_fieldmodule_id field_module = cmzn_field_get_fieldmodule(field);
	cmzn_fieldcache_id field_cache = cmzn_fieldmodule_create_fieldcache(field_module);
	if (image_depth > 1)
	{
		dimension = 3;
	}
	else
	{
		if (image_height > 1)
		{
			dimension = 2;
		}
		else
		{
			dimension = 1;
		}
	}
	number_of_data_components =
		Computed_field_get_number_of_components(field);
	const int number_of_texture_coordinate_components =
		Computed_field_get_number_of_components(texture_coordinate_field);
	if ((number_of_texture_coordinate_components > 3) ||
		(number_of_texture_coordinate_components < dimension))
	{
		display_message(ERROR_MESSAGE,
			"Set_cmiss_field_value_to_texture.  Invalid number of texture coordinate components");
		return 0;
	}
	Texture_get_physical_size(texture, &texture_width, &texture_height,
		&texture_depth);
	/* allocate space for a single image plane */
	image_width_bytes = image_width*bytes_per_pixel;
	if (number_of_bytes_per_component == 2)
	{
		 image_plane = (unsigned char *)NULL;
		 ALLOCATE(two_bytes_image_plane, unsigned short, image_height*image_width_bytes);
	}
	else
	{
		 two_bytes_image_plane = (unsigned short *)NULL;
		 ALLOCATE(image_plane, unsigned char, image_height*image_width_bytes);
	}
	ALLOCATE(data_values, FE_value, number_of_data_components);
	for (i = 0; number_of_data_components > i;  i++)
	{
		 data_values[i]=0.0;
	}
	if (fail_material)
	{
	  Graphical_material_get_diffuse(fail_material, &fail_colour);
	  Graphical_material_get_alpha(fail_material, &fail_alpha);
	}
	if ((image_plane || two_bytes_image_plane) && data_values)
	{
		hint_resolution[0] = (ZnReal)image_width;
		hint_resolution[1] = (ZnReal)image_height;
		hint_resolution[2] = (ZnReal)image_depth;
		field_evaluate_error_count = 0;
		find_element_xi_error_count = 0;
		spectrum_render_error_count = 0;
		total_number_of_pixels = image_width*image_height*image_depth;
		hint_maximums[0] = texture_width;
		hint_maximums[1] = texture_height;
		hint_maximums[2] = texture_depth;
		for (i = 0; (i < image_depth) && return_code; i++)
		{
			/*???debug -- leave in so user knows something is happening! */
			if (1 < image_depth)
			{
				printf("Evaluating image plane %d of %d\n", i+1, image_depth);
			}
			if (number_of_bytes_per_component == 2)
			{
				 two_bytes_ptr = (unsigned short *)two_bytes_image_plane;
			}
			else
			{
				 ptr = (unsigned char *)image_plane;
			}
			values[2] = texture_depth * ((ZnReal)i + 0.5) / (ZnReal)image_depth;
			for (j = 0; (j < image_height) && return_code; j++)
			{
				values[1] = texture_height * ((ZnReal)j + 0.5) / (ZnReal)image_height;
				for (k = 0; (k < image_width) && return_code; k++)
				{
					values[0] = texture_width * ((ZnReal)k + 0.5) / (ZnReal)image_width;
#if defined (DEBUG_CODE)
					/*???debug*/
					if ((1 < image_depth) && ((0 == j) || (image_height - 1 == j)) && ((0 == k) || (image_width - 1 == k)))
					{
						printf("  field pos = %10g %10g %10g\n", values[0], values[1], values[2]);
					}
#endif /* defined (DEBUG_CODE) */
					if (use_pixel_location)
					{
						/* Try to use a pixel coordinate first */
						cmzn_fieldcache_set_field_real(field_cache, texture_coordinate_field, number_of_texture_coordinate_components, values);
						if (cmzn_field_evaluate_real(field, field_cache, number_of_data_components, data_values))
						{
							if (!spectrum)
							{
								rgba[0] = 0;
								rgba[1] = 0;
								rgba[2] = 0;
								rgba[3] = 0;
								for (l=0; l<=number_of_data_components-1; l++)
								{
									rgba[l] = (ZnReal)data_values[l];
								}
							}
							else if ((!Spectrum_value_to_rgba(spectrum,
								number_of_data_components, data_values, rgba)))
							{
								rgba[0] = fail_colour.red;
								rgba[1] = fail_colour.green;
								rgba[2] = fail_colour.blue;
								rgba[3] = fail_alpha;
								spectrum_render_error_count++;
							}
						}
						else
						{
							use_pixel_location = 0;
						}
					}
					if (!use_pixel_location)
					{
						/* Otherwise find a valid element xi location */
						/* Computed_field_find_element_xi_special returns true if it has
							performed a valid calculation even if the element isn't found
							to stop the slow Computed_field_find_element_xi being called */
						rgba[0] = fail_colour.red;
						rgba[1] = fail_colour.green;
						rgba[2] = fail_colour.blue;
						rgba[3] = fail_alpha;
						if (search_mesh && (
							(graphics_buffer_package && Computed_field_find_element_xi_special(
								 texture_coordinate_field, field_cache, &cache, values,
								 Computed_field_get_number_of_components(texture_coordinate_field), &element, xi,
								 search_mesh, graphics_buffer_package,
								 hint_minimums, hint_maximums, hint_resolution)) ||
							Computed_field_find_element_xi(texture_coordinate_field, field_cache,
								values, Computed_field_get_number_of_components(texture_coordinate_field),
								&element, xi, search_mesh, propagate_field,
								/*find_nearest_location*/0)))
						{
							if (element)
							{
#if defined (DEBUG_CODE)
								/*???debug*/
								if ((1 < image_depth) && ((0 == j) || (image_height - 1 == j)) && ((0 == k) || (image_width - 1 == k)))
								{
									printf("  xi = %10g %10g %10g\n", xi[0], xi[1], xi[2]);
								}
#endif /* defined (DEBUG_CODE) */
								if (cmzn_fieldcache_set_mesh_location(field_cache, element, mesh_dimension, xi) &&
									cmzn_field_evaluate_real(field, field_cache, number_of_data_components, data_values))
								{
									if (!spectrum)
									{
										for (l=0; l<=number_of_data_components-1; l++)
										{
											rgba[l] = (ZnReal)data_values[l];
										}
									}
									else if (!Spectrum_value_to_rgba(spectrum,
											number_of_data_components, data_values,
											rgba))
									{
										spectrum_render_error_count++;
									}
								}
								else
								{
									field_evaluate_error_count++;
								}
							}
						}
						else
						{
							find_element_xi_error_count++;
						}
					}
#if defined (DEBUG_CODE)
					/*???debug*/
					if ((1 < image_depth) && ((0 == j) || (image_height - 1 == j)) && ((0 == k) || (image_width - 1 == k)))
					{
						printf("  RGBA = %10g %10g %10g %10g\n", rgba[0], rgba[1], rgba[2], rgba[3]);
					}
#endif /* defined (DEBUG_CODE) */
					if (number_of_bytes_per_component == 2)
					{
						 multiplier = static_cast<ZnReal>(pow(256.0,number_of_bytes_per_component) - 1.0);
						 switch (specify_format)
						 {
								case TEXTURE_LUMINANCE:
								{
									 if (!spectrum)
									 {
										*two_bytes_ptr = (unsigned short)((rgba[0]) * multiplier);
									 }
									 else
									 {
										*two_bytes_ptr = (unsigned short)((rgba[0] + rgba[1] + rgba[2]) * multiplier/ 3.0);
									 }
									 two_bytes_ptr++;
								} break;
								case TEXTURE_LUMINANCE_ALPHA:
								{
									 if (!spectrum)
									 {
										*two_bytes_ptr = (unsigned short)((rgba[0]) * multiplier);
										two_bytes_ptr++;
										*two_bytes_ptr = (unsigned short)(rgba[1] * multiplier);
										two_bytes_ptr++;
									 }
									 else
									 {
										*two_bytes_ptr = (unsigned short)((rgba[0] + rgba[1] + rgba[2]) * multiplier/ 3.0);
										two_bytes_ptr++;
										*two_bytes_ptr = (unsigned short)(rgba[3] * multiplier);
										two_bytes_ptr++;
									 }
								} break;
								case TEXTURE_RGB:
								{
									 *two_bytes_ptr = (unsigned short)(rgba[0] * multiplier);
									 two_bytes_ptr++;
									 *two_bytes_ptr = (unsigned short)(rgba[1] * multiplier);
									 two_bytes_ptr++;
									 *two_bytes_ptr = (unsigned short)(rgba[2] * multiplier);
									 two_bytes_ptr++;
								} break;
								case TEXTURE_RGBA:
								{
									 *two_bytes_ptr = (unsigned short)(rgba[0] * multiplier);
									 two_bytes_ptr++;
									 *two_bytes_ptr = (unsigned short)(rgba[1] * multiplier);
									 two_bytes_ptr++;
									 *two_bytes_ptr = (unsigned short)(rgba[2] * multiplier);
									 two_bytes_ptr++;
									 *two_bytes_ptr = (unsigned short)(rgba[3] * multiplier);
									 two_bytes_ptr++;
								} break;
								case TEXTURE_ABGR:
								{
									 *two_bytes_ptr = (unsigned short)(rgba[3] * multiplier);
									 two_bytes_ptr++;
									 *two_bytes_ptr = (unsigned short)(rgba[2] * multiplier);
									 two_bytes_ptr++;
									 *two_bytes_ptr = (unsigned short)(rgba[1] * multiplier);
									 two_bytes_ptr++;
									 *two_bytes_ptr = (unsigned short)(rgba[0] * multiplier);
									 two_bytes_ptr++;
								} break;
								default:
								{
									 display_message(ERROR_MESSAGE,
											"Set_cmiss_field_value_to_texture.  Unsupported storage type");
									 return_code = 0;
								} break;
						 }
					}
					else
					{
						 multiplier = 255.0;
						 switch (specify_format)
						 {
								case TEXTURE_LUMINANCE:
								{
									 if (!spectrum)
									 {
										*ptr = (unsigned char)(rgba[0] * multiplier);
									 }
									 else
									 {
										*ptr = (unsigned char)((rgba[0] + rgba[1] + rgba[2]) * multiplier/ 3.0);
									 }
									 ptr++;
								} break;
								case TEXTURE_LUMINANCE_ALPHA:
								{
									 if (!spectrum)
									 {
										*ptr = (unsigned char)(rgba[0] * multiplier);
										ptr++;
										*ptr = (unsigned char)(rgba[1] * multiplier);
										ptr++;
									 }
									 else
									 {
										*ptr = (unsigned char)((rgba[0] + rgba[1] + rgba[2]) * multiplier/ 3.0);
										ptr++;
										*ptr = (unsigned char)(rgba[3] * multiplier);
										ptr++;
									 }
								} break;
								case TEXTURE_RGB:
								{
									 *ptr = (unsigned char)(rgba[0] * multiplier);
									 ptr++;
									 *ptr = (unsigned char)(rgba[1] * multiplier);
									 ptr++;
									 *ptr = (unsigned char)(rgba[2] * multiplier);
									 ptr++;
								} break;
								case TEXTURE_RGBA:
								{
									 *ptr = (unsigned char)(rgba[0] * multiplier);
									 ptr++;
									 *ptr = (unsigned char)(rgba[1] * multiplier);
									 ptr++;
									 *ptr = (unsigned char)(rgba[2] * multiplier);
									 ptr++;
									 *ptr = (unsigned char)(rgba[3] * multiplier);
									 ptr++;
								} break;
								case TEXTURE_ABGR:
								{
									 *ptr = (unsigned char)(rgba[3] * multiplier);
									 ptr++;
									 *ptr = (unsigned char)(rgba[2] * multiplier);
									 ptr++;
									 *ptr = (unsigned char)(rgba[1] * multiplier);
									 ptr++;
									 *ptr = (unsigned char)(rgba[0] * multiplier);
									 ptr++;
								} break;
								default:
								{
									 display_message(ERROR_MESSAGE,
											"Set_cmiss_field_value_to_texture.  Unsupported storage type");
									 return_code = 0;
								} break;
						 }
					}
				}
			}
			if (number_of_bytes_per_component == 2)
			{
				 if (!Texture_set_image_block(texture,
							 /*left*/0, /*bottom*/0, image_width, image_height, /*depth_plane*/i,
							 image_width_bytes, (unsigned char *)two_bytes_image_plane))
				 {
						display_message(ERROR_MESSAGE,
							 "Set_cmiss_field_value_to_texture.  Could not set texture block");
						return_code = 0;
				 }
			}
			else
			{
				 if (!Texture_set_image_block(texture,
							 /*left*/0, /*bottom*/0, image_width, image_height, /*depth_plane*/i,
							 image_width_bytes, image_plane))
				 {
						display_message(ERROR_MESSAGE,
							 "Set_cmiss_field_value_to_texture.  Could not set texture block");
						return_code = 0;
				 }
			}
		}
		if (spectrum)
		{
			Spectrum_end_value_to_rgba(spectrum);
		}
		if (0 < field_evaluate_error_count)
		{
			display_message(WARNING_MESSAGE, "Set_cmiss_field_value_to_texture.  "
				"Field could not be evaluated in element for %d out of %d pixels",
				field_evaluate_error_count, total_number_of_pixels);
		}
		if (0 < spectrum_render_error_count)
		{
			display_message(WARNING_MESSAGE, "Set_cmiss_field_value_to_texture.  "
				"Spectrum could not be evaluated for %d out of %d pixels",
				spectrum_render_error_count, total_number_of_pixels);
		}
		if (0 < find_element_xi_error_count)
		{
			display_message(WARNING_MESSAGE, "Set_cmiss_field_value_to_texture.  "
				"Unable to find element:xi for %d out of %d pixels",
				find_element_xi_error_count, total_number_of_pixels);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Set_cmiss_field_value_to_texture.  Not enough memory");
		return_code = 0;
	}
	DEALLOCATE(data_values);
	if (image_plane)
		 DEALLOCATE(image_plane);
	if (two_bytes_image_plane)
		 DEALLOCATE(two_bytes_image_plane);
	if (cache)
	{
		DESTROY(Computed_field_find_element_xi_cache)(&cache);
	}
	cmzn_fieldcache_destroy(&field_cache);
	cmzn_fieldmodule_destroy(&field_module);

	return return_code;
}

enum cmzn_field_image_combine_mode cmzn_field_image_get_combine_mode(
   cmzn_field_image_id image_field)
{
	cmzn_texture *texture = cmzn_field_image_get_texture(image_field);
	int mode = Texture_get_combine_mode(texture) + 1;
	enum cmzn_field_image_combine_mode combine_mode = (cmzn_field_image_combine_mode)mode;
	return combine_mode;
}

int cmzn_field_image_set_combine_mode(cmzn_field_image_id image_field,
   enum cmzn_field_image_combine_mode combine_mode)
{
	cmzn_texture *texture = cmzn_field_image_get_texture(image_field);
	int mode = combine_mode;
	if (mode < 1)
	{
		return 0;
	}
	else
	{
		mode = mode - 1;
		enum Texture_combine_mode texture_combine_mode = (Texture_combine_mode)mode;
		if (Texture_get_combine_mode(texture) != texture_combine_mode)
		{
			Texture_set_combine_mode(texture, texture_combine_mode);
			Computed_field_changed(cmzn_field_image_base_cast(image_field));
		}
		return 1;
	}
}

cmzn_field_id cmzn_field_image_get_domain_field(
	cmzn_field_image_id image_field)
{
	if (image_field)
	{
		cmzn_field_id field = cmzn_field_image_base_cast(image_field);
		return ACCESS(Computed_field)(field->source_fields[0]);
	}
	return 0;
}

int cmzn_field_image_set_domain_field(
	cmzn_field_image_id image_field, cmzn_field_id domain_field)
{
	cmzn_field_id field = cmzn_field_image_base_cast(image_field);
	if (image_field && domain_field &&
		Computed_field_has_numerical_components(domain_field, 0) &&
		Computed_field_get_region(field) == Computed_field_get_region(domain_field))
	{
		int domainDimension = cmzn_field_get_number_of_components(domain_field);
		Computed_field_image *image_core = Computed_field_image_core_cast(image_field);
		int textureDimension = 0;
		Texture_get_dimension(image_core->texture, &textureDimension);
		if (domainDimension >= textureDimension)
		{
			REACCESS(Computed_field)(&(field->source_fields[0]), domain_field);
			return CMZN_OK;
		}
	}
	return CMZN_ERROR_ARGUMENT;
}

class cmzn_field_image_combine_mode_conversion
{
public:
	static const char *to_string(enum cmzn_field_image_combine_mode mode)
	{
		const char *enum_string = 0;
		switch (mode)
		{
		case CMZN_FIELD_IMAGE_COMBINE_BLEND:
			enum_string = "BLEND";
			break;
		case CMZN_FIELD_IMAGE_COMBINE_DECAL:
			enum_string = "DECAL";
			break;
		case CMZN_FIELD_IMAGE_COMBINE_MODULATE:
			enum_string = "MODULATE";
			break;
		case CMZN_FIELD_IMAGE_COMBINE_ADD:
			enum_string = "ADD";
			break;
		case CMZN_FIELD_IMAGE_COMBINE_ADD_SIGNED:
			enum_string = "ADD_SIGNED";
			break;
		case CMZN_FIELD_IMAGE_COMBINE_MODULATE_SCALE_4:
			enum_string = "MODULATE_SCALE_4";
			break;
		case CMZN_FIELD_IMAGE_COMBINE_BLEND_SCALE_4:
			enum_string = "BLEND_SCALE_4";
			break;
		case CMZN_FIELD_IMAGE_COMBINE_SUBTRACT:
			enum_string = "SUBTRACT";
			break;
		case CMZN_FIELD_IMAGE_COMBINE_ADD_SCALE_4:
			enum_string = "ADD_SCALE_4";
			break;
		case CMZN_FIELD_IMAGE_COMBINE_SUBTRACT_SCALE_4:
			enum_string = "SUBTRACT_SCALE_4";
			break;
		case CMZN_FIELD_IMAGE_COMBINE_INVERT_ADD_SCALE_4:
			enum_string = "INVERT_ADD_SCALE_4";
			break;
		case CMZN_FIELD_IMAGE_COMBINE_INVERT_SUBTRACT_SCALE_4:
			enum_string = "INVERT_SUBTRACT_SCALE_4";
			break;
		default:
			break;
		}
		return enum_string;
	}
};

enum cmzn_field_image_combine_mode cmzn_field_image_combine_mode_enum_from_string(
	const char *string)
{
	return string_to_enum<enum cmzn_field_image_combine_mode,
	cmzn_field_image_combine_mode_conversion>(string);
}

char *cmzn_field_image_combine_mode_enum_to_string(enum cmzn_field_image_combine_mode mode)
{
	const char *mode_string = cmzn_field_image_combine_mode_conversion::to_string(mode);
	return (mode_string ? duplicate_string(mode_string) : 0);
}

enum cmzn_field_image_hardware_compression_mode cmzn_field_image_get_hardware_compression_mode(
   cmzn_field_image_id image_field)
{
	cmzn_texture *texture = cmzn_field_image_get_texture(image_field);
	int mode = Texture_get_compression_mode(texture) + 1;
	enum cmzn_field_image_hardware_compression_mode compression_mode =
		(cmzn_field_image_hardware_compression_mode)mode;
	return compression_mode;
}

int cmzn_field_image_set_hardware_compression_mode(cmzn_field_image_id image_field,
   enum cmzn_field_image_hardware_compression_mode compression_mode)
{
	cmzn_texture *texture = cmzn_field_image_get_texture(image_field);
	int mode = compression_mode;
	if (mode < 1)
	{
		return 0;
	}
	else
	{
		mode = mode - 1;
		enum Texture_compression_mode texture_compression_mode =
			(Texture_compression_mode)mode;
		if (Texture_get_compression_mode(texture) != texture_compression_mode)
		{
			Texture_set_compression_mode(texture, texture_compression_mode);
			Computed_field_changed(cmzn_field_image_base_cast(image_field));
		}
		return 1;
	}
}

class cmzn_field_image_hardware_compression_mode_conversion
{
public:
	static const char *to_string(enum cmzn_field_image_hardware_compression_mode mode)
	{
		const char *enum_string = 0;
		switch (mode)
		{
		case CMZN_FIELD_IMAGE_HARDWARE_COMPRESSION_NONE:
			enum_string = "NONE";
			break;
		case CMZN_FIELD_IMAGE_HARDWARE_COMPRESSION_AUTOMATIC:
			enum_string = "AUTOMATIC";
			break;
		default:
			break;
		}
		return enum_string;
	}
};

enum cmzn_field_image_hardware_compression_mode
	cmzn_field_image_hardware_compression_mode_enum_from_string(const char *string)
{
	return string_to_enum<enum cmzn_field_image_hardware_compression_mode,
	cmzn_field_image_hardware_compression_mode_conversion>(string);
}

char *cmzn_field_image_hardware_compression_mode_enum_to_string(
	enum cmzn_field_image_hardware_compression_mode mode)
{
	const char *mode_string = cmzn_field_image_hardware_compression_mode_conversion::to_string(mode);
	return (mode_string ? duplicate_string(mode_string) : 0);
}

enum cmzn_field_image_filter_mode cmzn_field_image_get_filter_mode(
   cmzn_field_image_id image_field)
{
	cmzn_texture *texture = cmzn_field_image_get_texture(image_field);
	int mode = Texture_get_filter_mode(texture) + 1;
	enum cmzn_field_image_filter_mode filter_mode = (cmzn_field_image_filter_mode)mode;
	return filter_mode;
}

int cmzn_field_image_set_filter_mode(cmzn_field_image_id image_field,
   enum cmzn_field_image_filter_mode filter_mode)
{
	cmzn_texture *texture = cmzn_field_image_get_texture(image_field);
	int mode = filter_mode;
	if (mode < 1)
	{
		return 0;
	}
	else
	{
		mode = mode - 1;
		enum Texture_filter_mode texture_filter_mode = (Texture_filter_mode)mode;
		if (Texture_get_filter_mode(texture) != texture_filter_mode)
		{
			Texture_set_filter_mode(texture, texture_filter_mode);
			Computed_field_changed(cmzn_field_image_base_cast(image_field));
		}
		return 1;
	}
}

char *cmzn_field_image_get_property(cmzn_field_image_id image,
	const char* property)
{
	cmzn_texture_id texture = cmzn_field_image_get_texture(image);

	return Texture_get_property(texture, property);
}

class cmzn_field_image_filter_mode_conversion
{
public:
	static const char *to_string(enum cmzn_field_image_filter_mode mode)
	{
		const char *enum_string = 0;
		switch (mode)
		{
		case CMZN_FIELD_IMAGE_FILTER_NEAREST:
			enum_string = "NEAREST";
			break;
		case CMZN_FIELD_IMAGE_FILTER_LINEAR:
			enum_string = "LINEAR";
			break;
		case CMZN_FIELD_IMAGE_FILTER_NEAREST_MIPMAP_NEAREST:
			enum_string = "NEAREST_MIPMAP_NEAREST";
			break;
		case CMZN_FIELD_IMAGE_FILTER_LINEAR_MIPMAP_NEAREST:
			enum_string = "LINEAR_MIPMAP_NEAREST";
			break;
		case CMZN_FIELD_IMAGE_FILTER_LINEAR_MIPMAP_LINEAR:
			enum_string = "LINEAR_MIPMAP_LINEAR";
			break;
		default:
			break;
		}
		return enum_string;
	}
};

enum cmzn_field_image_filter_mode cmzn_field_image_filter_mode_enum_from_string(
	const char *string)
{
	return string_to_enum<enum cmzn_field_image_filter_mode,
	cmzn_field_image_filter_mode_conversion>(string);
}

char *cmzn_field_image_filter_mode_enum_to_string(enum cmzn_field_image_filter_mode mode)
{
	const char *mode_string = cmzn_field_image_filter_mode_conversion::to_string(mode);
	return (mode_string ? duplicate_string(mode_string) : 0);
}

enum cmzn_field_image_wrap_mode cmzn_field_image_get_wrap_mode(
   cmzn_field_image_id image_field)
{
	cmzn_texture *texture = cmzn_field_image_get_texture(image_field);
	int mode = Texture_get_wrap_mode(texture) + 1;
	enum cmzn_field_image_wrap_mode wrap_mode = (cmzn_field_image_wrap_mode)mode;
	return wrap_mode;
}

int cmzn_field_image_set_wrap_mode(cmzn_field_image_id image_field,
   enum cmzn_field_image_wrap_mode wrap_mode)
{
	cmzn_texture *texture = cmzn_field_image_get_texture(image_field);
	int mode = wrap_mode;
	if (mode < 1)
	{
		return 0;
	}
	else
	{
		mode = mode - 1;
		enum Texture_wrap_mode texture_wrap_mode = (Texture_wrap_mode)mode;
		if (Texture_get_wrap_mode(texture) != texture_wrap_mode)
		{
			Texture_set_wrap_mode(texture, texture_wrap_mode);
			Computed_field_changed(cmzn_field_image_base_cast(image_field));
		}
		return 1;
	}
}

class cmzn_field_image_wrap_mode_conversion
{
public:
	static const char *to_string(enum cmzn_field_image_wrap_mode mode)
	{
		const char *enum_string = 0;
		switch (mode)
		{
		case CMZN_FIELD_IMAGE_WRAP_CLAMP:
			enum_string = "CLAMP";
			break;
		case CMZN_FIELD_IMAGE_WRAP_REPEAT:
			enum_string = "REPEAT";
			break;
		case CMZN_FIELD_IMAGE_WRAP_EDGE_CLAMP:
			enum_string = "EDGE_CLAMP";
			break;
		case CMZN_FIELD_IMAGE_WRAP_BORDER_CLAMP:
			enum_string = "BORDER_CLAMP";
			break;
		case CMZN_FIELD_IMAGE_WRAP_MIRROR_REPEAT:
			enum_string = "MIRROR_REPEAT";
			break;
		default:
			break;
		}
		return enum_string;
	}
};

enum cmzn_field_image_wrap_mode cmzn_field_image_wrap_mode_enum_from_string(
	const char *string)
{
	return string_to_enum<enum cmzn_field_image_wrap_mode,
	cmzn_field_image_wrap_mode_conversion>(string);
}

char *cmzn_field_image_wrap_mode_enum_to_string(enum cmzn_field_image_wrap_mode mode)
{
	const char *mode_string = cmzn_field_image_wrap_mode_conversion::to_string(mode);
	return (mode_string ? duplicate_string(mode_string) : 0);
}
