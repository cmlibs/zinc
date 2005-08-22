/*******************************************************************************
FILE : computed_field_image_processing.c

LAST MODIFIED : 4 December 2003

DESCRIPTION :
Implements a number of basic component wise operations on computed fields.
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
#include <math.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_image_processing.h"

struct Computed_field_image_processing_package 
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
A container for objects required to define fields in this module.
==============================================================================*/
{
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Cmiss_region *root_region;
	struct User_interface *user_interface;
};

struct Image_cache
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
Stores an intermediate representation of a field in an image structure so that
spatially dependent algorithms can be implemented in computed fields.
==============================================================================*/
{
	int dimension;
	int *sizes;
	FE_value *minimums;
	FE_value *maximums;
	int depth;
	char *data;

	int valid;

	int access_count;
}; /* struct Image_cache */

struct Computed_field_sobel_filter_type_specific_data
{
	float cached_time;
	int element_dimension;
	struct Cmiss_region *region;
	struct User_interface *user_interface;
	struct Image_cache *image;
	struct MANAGER(Computed_field) *computed_field_manager;
	void *computed_field_manager_callback_id;
};

struct Image_cache *CREATE(Image_cache)(void)
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
==============================================================================*/
{
	struct Image_cache *image;

	ENTER(CREATE(Image_cache));
	
	if (ALLOCATE(image, struct Image_cache, 1))
	{
		image->dimension = 0;
		image->sizes = (int *)NULL;
		image->minimums = (FE_value *)NULL;
		image->maximums = (FE_value *)NULL;
		image->depth = 0;
		image->data = (char *)NULL;
		image->valid = 0;
		image->access_count = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Image_cache).  Not enough memory");
		image = (struct Image_cache *)NULL;
	}
	LEAVE;

	return (image);
} /* CREATE(Image_cache) */

int DESTROY(Image_cache)(struct Image_cache **image_address)
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
Frees memory/deaccess mapping at <*mapping_address>.
==============================================================================*/
{
	int return_code;
	struct Image_cache *image;

	ENTER(DESTROY(Image_cache));
	if (image_address && (image = *image_address))
	{
		if (image->access_count == 0)
		{
			if (image->data)
			{
				DEALLOCATE(image->data);
			}
			if (image->sizes)
			{
				DEALLOCATE(image->sizes);
			}
			if (image->minimums)
			{
				DEALLOCATE(image->minimums);
			}
			if (image->maximums)
			{
				DEALLOCATE(image->maximums);
			}
			DEALLOCATE(*image_address);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Image_cache).  Access count is not zero.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Image_cache).  Missing mapping");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Image_cache) */

DECLARE_OBJECT_FUNCTIONS(Image_cache)

static int Image_cache_update_dimension(struct Image_cache *image,
	int dimension, int depth, int *sizes, FE_value *minimums, FE_value *maximums)
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
Resizes the dynamic storage in the cache for the specified <dimension> and 
<depth> and recording the sizes, minimums and maximums.
==============================================================================*/
{
	int i, return_code;

	ENTER(Image_cache_update_dimension);
	if (image && (dimension > 0) && (depth > 0))
	{
		return_code = 1;
		if (image->sizes && image->minimums && image->maximums)
		{
			if (dimension != image->dimension)
			{
				if (!(REALLOCATE(image->sizes, image->sizes, int, dimension) &&
					REALLOCATE(image->minimums, image->minimums, FE_value, dimension) &&
					REALLOCATE(image->maximums, image->maximums, FE_value, dimension)))
				{
					return_code = 0;
				}
			}
		}
		else
		{
			/* Allocate memory for new image */
			if (!(ALLOCATE(image->sizes, int, dimension) &&
				ALLOCATE(image->minimums, FE_value, dimension) &&
				ALLOCATE(image->maximums, FE_value, dimension)))
			{
				return_code = 0;
			}
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE, "Image_cache_update_dimension.  "
				"Error allocating memory for cache.");
			DEALLOCATE(image->sizes);
			DEALLOCATE(image->minimums);
			DEALLOCATE(image->maximums);
		}
		image->dimension = dimension;
		image->depth = depth;
		for (i = 0 ; i < image->dimension ; i++)
		{
			image->sizes[i] = sizes[i];
			image->minimums[i] = minimums[i];
			image->maximums[i] = maximums[i];
		}
		image->valid = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Image_cache_update_dimension.  "
			"Invalid arguments.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Image_cache_update_dimension */

static int Image_cache_update_data_storage(struct Image_cache *image)
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
Resizes the dynamic data storage in the cache.
==============================================================================*/
{
	int i, image_bytes, return_code;

	ENTER(Image_cache_update_data_storage);
	if (image && (image->dimension > 0) && (image->depth > 0))
	{
		return_code = 1;
		image_bytes = image->depth * sizeof(FE_value);
		for (i = 0 ; i < image->dimension ; i++)
		{
			image_bytes *= image->sizes[i];
		}
		if (!(REALLOCATE(image->data, image->data, char, image_bytes)))
		{
			display_message(ERROR_MESSAGE, "Image_cache_update_data_storage.  "
				"Error allocating memory for cache.");
			return_code = 0;
		}
		image->valid = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Image_cache_update_data_storage.  "
			"Invalid arguments.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Image_cache_update_data_storage */

static int Image_cache_update_from_fields(struct Image_cache *image,
	struct Computed_field *source_field,
	struct Computed_field *texture_coordinate_field,
	int element_dimension, 
	struct Cmiss_region *region, struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
Creates an Image_cache, <image> that represents the source field by evaluating the 
<source_field> at each pixel and using the element_xi from find_element_xi on
the <texture_coordinate_field>.  The <image> is created with the specified
<dimension> and the corresponding vectors of number of pixels <sizes> and
<minimums> and <maximums> for the texture coordinate ranges.
An <element_dimension> of 0 searches in elements of all dimension, any other
value searches just elements of that dimension.
==============================================================================*/
{
	FE_value *data_index, *texture_coordinates, xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	float *sizes_floats;
	int depth, field_evaluate_error_count, find_element_xi_error_count, finished,
		i, *indices, j, number_of_texture_coordinate_components, return_code,
		total_number_of_pixels;
	struct FE_element *element;

	ENTER(Image_cache_update_from_fields);
	if (image && source_field && (depth = 
		Computed_field_get_number_of_components(source_field)) && (depth > 0)
		&& texture_coordinate_field && (number_of_texture_coordinate_components = 
		Computed_field_get_number_of_components(texture_coordinate_field)) &&
		(image->dimension > 0) && (number_of_texture_coordinate_components >= image->dimension) &&
		image->sizes && image->minimums && image->maximums && region)
	{
		return_code = Image_cache_update_data_storage(image);
		if (return_code)
		{
			return_code = ALLOCATE(indices, int, image->dimension) && 
				ALLOCATE(sizes_floats, float, image->dimension) && 
				ALLOCATE(texture_coordinates, FE_value, image->dimension);
		}
		if (return_code)
		{
			total_number_of_pixels = 1;
			for (i = 0 ; i < image->dimension ; i++)
			{
				indices[i] = 0;
				sizes_floats[i] = image->sizes[i];
				texture_coordinates[i] = image->minimums[i];
				total_number_of_pixels *= image->sizes[i];
			}
			data_index = (FE_value *)image->data;
			field_evaluate_error_count = 0;
			find_element_xi_error_count = 0;
			finished = 0;
			while (return_code && !finished)
			{
				/* Computed_field_find_element_xi_special returns true if it has
					performed a valid calculation even if the element isn't found
					to stop the slow Computed_field_find_element_xi being called */
				if (Computed_field_find_element_xi_special(
					texture_coordinate_field, &texture_coordinate_field->find_element_xi_cache,
					texture_coordinates,
					number_of_texture_coordinate_components, &element, xi,
					region, element_dimension, user_interface,
					image->minimums, image->maximums, sizes_floats) ||
					Computed_field_find_element_xi(texture_coordinate_field,
					texture_coordinates, number_of_texture_coordinate_components, 
					&element, xi, element_dimension, region, /*propagate_field*/0))
				{
					if (element)
					{
						if (!(Computed_field_evaluate_in_element(source_field,
							element, xi, /*time*/0, (struct FE_element *)NULL,
							data_index, (FE_value *)NULL)))
						{
							for (i = 0 ; i < depth ; i++)
							{
								data_index[i] = 0.0;
							}
							field_evaluate_error_count++;
						}
					}
					else
					{
						for (i = 0 ; i < depth ; i++)
						{
							data_index[i] = 0.0;
						}
					}
				}
				else
				{
					for (i = 0 ; i < depth ; i++)
					{
						data_index[i] = 0.0;
					}
					find_element_xi_error_count++;
				}

				data_index += depth;
				i = 0;
				while ((i < image->dimension) && (indices[i] >= image->sizes[i] - 1))
				{
					i++;
				}
				if (i < image->dimension)
				{
					for (j = 0 ; j < i ; j++)
					{
						indices[j] = 0;
						texture_coordinates[j] = image->minimums[j];
					}
					indices[i]++;
					texture_coordinates[j] = image->minimums[j] + ((FE_value)indices[j]) *
						(image->maximums[j] - image->minimums[j]) / ((FE_value)(image->sizes[i] - 1));
				}
				else
				{
					finished = 1;
				}
			}
			if (return_code)
			{
				image->valid = 1;
			}
			Computed_field_clear_cache(source_field);
			/* Not if we can avoid it as this destroys the find_xi_caches */
			/* Computed_field_clear_cache(texture_coordinate_field); */
			if (0 < field_evaluate_error_count)
			{
				display_message(WARNING_MESSAGE, "Image_cache_update_from_fields.  "
					"Field could not be evaluated in element for %d out of %d pixels",
					field_evaluate_error_count, total_number_of_pixels);
			}
			if (0 < find_element_xi_error_count)
			{
				display_message(WARNING_MESSAGE, "Image_cache_update_from_fields.  "
					"Unable to find element:xi for %d out of %d pixels",
					find_element_xi_error_count, total_number_of_pixels);
			}
			DEALLOCATE(indices);
			DEALLOCATE(sizes_floats);
			DEALLOCATE(texture_coordinates);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Image_cache_update_from_fields.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Image_cache_update_from_fields.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Image_cache_update_from_fields */

static char computed_field_sobel_filter_type_string[] = "sobel_filter";

int Computed_field_is_type_sobel_filter(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_sobel_filter);
	if (field)
	{
		return_code = 
		  (field->type_string == computed_field_sobel_filter_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_sobel_filter.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_sobel_filter */

static void Computed_field_sobel_filter_field_change(
	struct MANAGER_MESSAGE(Computed_field) *message, void *field_void)
/*******************************************************************************
LAST MODIFIED : 5 December 2003

DESCRIPTION :
Manager function called back when either of the source fields change so that
we know to invalidate the image cache.
==============================================================================*/
{
	struct Computed_field *field;
	struct Computed_field_sobel_filter_type_specific_data *data;

	ENTER(Computed_field_sobel_filter_source_field_change);
	if (message && (field = (struct Computed_field *)field_void) && (data = 
		(struct Computed_field_sobel_filter_type_specific_data *)
		field->type_specific_data))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Computed_field):
			case MANAGER_CHANGE_OBJECT(Computed_field):
			{
				if (Computed_field_depends_on_Computed_field_in_list(
					field->source_fields[0], message->changed_object_list) ||
					Computed_field_depends_on_Computed_field_in_list(
					field->source_fields[1], message->changed_object_list))
				{
					if (data->image)
					{
						data->image->valid = 0;
					}
				}
			} break;
			case MANAGER_CHANGE_ADD(Computed_field):
			case MANAGER_CHANGE_REMOVE(Computed_field):
			case MANAGER_CHANGE_IDENTIFIER(Computed_field):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_sobel_filter_source_field_change.  "
			"Invalid arguments.");
	}
	LEAVE;
} /* Computed_field_sobel_filter_source_field_change */

static int Computed_field_sobel_filter_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_sobel_filter_type_specific_data *data;

	ENTER(Computed_field_sobel_filter_clear_type_specific);
	if (field && (data = 
		(struct Computed_field_sobel_filter_type_specific_data *)
		field->type_specific_data))
	{
		if (data->region)
		{
			DEACCESS(Cmiss_region)(&data->region);
		}
		if (data->image)
		{
			DEACCESS(Image_cache)(&data->image);
		}
		if (data->computed_field_manager && data->computed_field_manager_callback_id)
		{
			MANAGER_DEREGISTER(Computed_field)(
				data->computed_field_manager_callback_id,
				data->computed_field_manager);
		}
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_sobel_filter_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_sobel_filter_clear_type_specific */

static void *Computed_field_sobel_filter_copy_type_specific(
	struct Computed_field *source_field, struct Computed_field *destination_field)
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_sobel_filter_type_specific_data *destination,
		*source;

	ENTER(Computed_field_sobel_filter_copy_type_specific);
	if (source_field && destination_field && (source = 
		(struct Computed_field_sobel_filter_type_specific_data *)
		source_field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_sobel_filter_type_specific_data, 1))
		{
			destination->cached_time = source->cached_time;
			destination->region = ACCESS(Cmiss_region)(source->region);
			destination->element_dimension = source->element_dimension;
			destination->user_interface = source->user_interface;
			destination->computed_field_manager = source->computed_field_manager;
			destination->computed_field_manager_callback_id = 
				MANAGER_REGISTER(Computed_field)(
				Computed_field_sobel_filter_field_change, (void *)destination_field,
				destination->computed_field_manager);
			if (source->image)
			{
				destination->image = ACCESS(Image_cache)(CREATE(Image_cache)());
				Image_cache_update_dimension(destination->image,
					source->image->dimension, source->image->depth,
					source->image->sizes, source->image->minimums,
					source->image->maximums);
			}
			else
			{
				destination->image = (struct Image_cache *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_sobel_filter_copy_type_specific.  "
				"Unable to allocate memory.");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_sobel_filter_copy_type_specific.  "
			"Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_sobel_filter_copy_type_specific */

int Computed_field_sobel_filter_clear_cache_type_specific
   (struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_sobel_filter_type_specific_data *data;

	ENTER(Computed_field_sobel_filter_clear_type_specific);
	if (field && (data = 
		(struct Computed_field_sobel_filter_type_specific_data *)
		field->type_specific_data))
	{
		if (data->image)
		{
			/* data->image->valid = 0; */
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_sobel_filter_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_sobel_filter_clear_type_specific */

static int Computed_field_sobel_filter_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;
	struct Computed_field_sobel_filter_type_specific_data *data,
		*other_data;

	ENTER(Computed_field_sobel_filter_type_specific_contents_match);
	if (field && other_computed_field && (data = 
		(struct Computed_field_sobel_filter_type_specific_data *)
		field->type_specific_data) && (other_data =
		(struct Computed_field_sobel_filter_type_specific_data *)
		other_computed_field->type_specific_data))
	{
		if (data->image && other_data->image &&
			(data->image->dimension == other_data->image->dimension) &&
			(data->image->depth == other_data->image->depth))
		{
			/* Check sizes and minimums and maximums */
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
} /* Computed_field_sobel_filter_type_specific_contents_match */

#define Computed_field_sobel_filter_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_sobel_filter_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_sobel_filter_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_sobel_filter_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Image_cache_sobel_filter(struct Image_cache *image)
/*******************************************************************************
LAST MODIFIED : 5 December 2003

DESCRIPTION :
Perform a reverse image operation on the image cache.
==============================================================================*/
{
	char *storage;
	FE_value *data_index, *result_index, *stencil, *sums;
	int filter_size, filter_step, i, image_step, *indices, j, k, *offsets,
		pass, return_code, stencil_size, storage_size;

	ENTER(Image_cache_sobel_filter);
	if (image && (image->dimension > 0) && (image->depth > 0))
	{
		return_code = 1;
		filter_size = 3;
		/* We only need the one stencil as it is just a reordering for the other dimensions */
		stencil_size = 1;
		for (i = 0 ; i < image->dimension ; i++)
		{
			stencil_size *= filter_size;
		}
		/* Allocate a new storage block for our data */
		storage_size = image->depth;
		for (i = 0 ; i < image->dimension ; i++)
		{
			storage_size *= image->sizes[i];
		}
		if (ALLOCATE(indices, int, image->dimension) &&
			ALLOCATE(stencil, FE_value, stencil_size) &&
			ALLOCATE(offsets, int, stencil_size) &&
			ALLOCATE(storage, char, storage_size * sizeof(FE_value)) &&
			ALLOCATE(sums, FE_value, image->depth))
		{
			result_index = (FE_value *)storage;
			for (i = 0 ; i < storage_size ; i++)
			{
				*result_index = 0.0;
				result_index++;
			}
			for (j = 0 ; j < stencil_size ; j++)
			{
				offsets[j] = 0;
			}
			filter_step = 1;
			image_step = 1;
			for (i = 0 ; i < image->dimension ; i++)
			{
				for (j = 0 ; j < stencil_size ; j++)
				{
					switch ((j / filter_step) % filter_size)
					{
						case 0:
						{
							offsets[j] += -image_step * image->depth;
						} break;
						case 2:
						{
							offsets[j] += image_step * image->depth;
						} break;
					}
				}
				filter_step *= filter_size;
				image_step *= image->sizes[i];
			}
			for (pass = 0 ; pass < image->dimension ; pass++)
			{
				for (i = 0 ; i < image->dimension ; i++)
				{
					indices[i] = 0;
				}
				for (j = 0 ; j < stencil_size ; j++)
				{
					stencil[j] = 1;
				}
				filter_step = 1;
				for (i = 0 ; i < image->dimension ; i++)
				{
					if (i == pass)
					{
						for (j = 0 ; j < stencil_size ; j++)
						{
							switch ((j / filter_step) % filter_size)
							{
								/* case 0:
								{
									stencil[j] *= 1;
								} break; */
								case 1:
								{
									stencil[j] *= 2;
								} break;
								/* case 2:
								{
									stencil[j] *= 1;
								} break; */
							}
						}
					}
					else
					{
						for (j = 0 ; j < stencil_size ; j++)
						{
							switch ((j / filter_step) % filter_size)
							{
								case 0:
								{
									stencil[j] *= -1;
								} break;
								case 1:
								{
									stencil[j] = 0;
								} break;
								/* case 2:
								{
									stencil[j] *= 1;
								} break; */
							}
						}
					}
					filter_step *= filter_size;
				}
				data_index = (FE_value *)image->data;
				result_index = (FE_value *)storage;
				for (i = 0 ; return_code && i < storage_size / image->depth ; i++)
				{
					for (k = 0 ; k < image->depth ; k++)
					{
						sums[k] = 0;
					}
					for (j = 0 ; j < stencil_size ; j++)
					{
						if (result_index + offsets[j] < ((FE_value *)storage))
						{
							/* Wrapping around */
							for (k = 0 ; k < image->depth ; k++)
							{
								sums[k] += stencil[j] *
									*(data_index + offsets[j] + storage_size + k);
							}
						}
						else if (result_index + offsets[j] >= ((FE_value *)storage) + storage_size)
						{
							/* Wrapping back */
							for (k = 0 ; k < image->depth ; k++)
							{
								sums[k] += stencil[j] *
									*(data_index + offsets[j] - storage_size + k);
							}							
						}
						else
						{
							/* standard */
							for (k = 0 ; k < image->depth ; k++)
							{
								sums[k] += stencil[j] *
									*(data_index + offsets[j] + k);
							}
						}
					}
					/* Accumulate the absolute value of this pass */
					for (k = 0 ; k < image->depth ; k++)
					{
						if (sums[k] < 0.0)
						{
							result_index[k] -= sums[k];
						}
						else
						{
							result_index[k] += sums[k];
						}
					}
					data_index += image->depth;
					result_index += image->depth;
				}
			}
			if (return_code)
			{
				DEALLOCATE(image->data);
				image->data = storage;
				image->valid = 1;
			}
			else
			{
				DEALLOCATE(storage);
			}
			DEALLOCATE(indices);
			DEALLOCATE(stencil);
			DEALLOCATE(offsets);
			DEALLOCATE(sums);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Image_cache_sobel_filter.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "Image_cache_sobel_filter.  "
			"Invalid arguments.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Image_cache_sobel_filter */

static int Computed_field_sobel_filter_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value *texture_coordinates, *values;
	int i, in_image, offset, return_code;
	struct Computed_field_sobel_filter_type_specific_data *data;

	ENTER(Computed_field_sobel_filter_evaluate_cache_at_node);
	if (field && node &&
		(data = (struct Computed_field_sobel_filter_type_specific_data *)field->type_specific_data))
	{
		return_code = 1;
		/* 1. Precalculate the Image_cache */
		if (!data->image->valid)
		{
			return_code = Image_cache_update_from_fields(data->image, field->source_fields[0],
				field->source_fields[1], data->element_dimension, data->region,
				data->user_interface);
			/* 2. Perform image processing operation */
			return_code = Image_cache_sobel_filter(data->image);
		}
		/* 3. Evaluate texture coordinates */
		Computed_field_evaluate_cache_at_node(field->source_fields[1],
			node, time);
		in_image = 1;
		offset = 0;
		texture_coordinates = field->source_fields[1]->values;
		for (i = data->image->dimension - 1 ; in_image && (i >= 0) ; i--)
		{
			offset *= data->image->sizes[i];
			if ((texture_coordinates[i] >= data->image->minimums[i]) &&
				(texture_coordinates[i] <= data->image->maximums[i]))
			{
				offset += data->image->sizes[i] * 
					((texture_coordinates[i] - data->image->minimums[i])
					/ (data->image->maximums[i] - data->image->minimums[i]));
			}
			else
			{
				in_image = 0;
			}
		}
		/* 4. Copy values from image to field values cache */
		if (in_image)
		{
			values = ((FE_value *)data->image->data)+ offset * data->image->depth ;
			for (i = 0 ; (i < field->number_of_components) ; i++)
			{
				field->values[i] = values[i];
			}
		}
		else
		{
			for (i = 0 ; (i < field->number_of_components) ; i++)
			{
				field->values[i] = 0.5;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_sobel_filter_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_sobel_filter_evaluate_cache_at_node */

static int Computed_field_sobel_filter_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value *texture_coordinates, *values;
	int i, in_image, offset, return_code;
	struct Computed_field_sobel_filter_type_specific_data *data;

	ENTER(Computed_field_sobel_filter_evaluate_cache_in_element);
	USE_PARAMETER(calculate_derivatives);
	if (field && element && xi && (field->number_of_source_fields > 0) && 
		(field->number_of_components == field->source_fields[0]->number_of_components) &&
		(data = (struct Computed_field_sobel_filter_type_specific_data *) field->type_specific_data) &&
		data->image && (field->number_of_components == data->image->depth))
	{
		return_code = 1;
		/* 1. Precalculate the Image_cache */
		if (!data->image->valid)
		{
			return_code = Image_cache_update_from_fields(data->image, field->source_fields[0],
				field->source_fields[1], data->element_dimension, data->region,
				data->user_interface);
			/* 2. Perform image processing operation */
			return_code = Image_cache_sobel_filter(data->image);
		}
		/* 3. Evaluate texture coordinates */
		Computed_field_evaluate_cache_in_element(field->source_fields[1],
			element, xi, time, top_level_element, /*calculate_derivatives*/0);
		in_image = 1;
		offset = 0;
		texture_coordinates = field->source_fields[1]->values;
		for (i = data->image->dimension - 1 ; in_image && (i >= 0) ; i--)
		{
			offset *= data->image->sizes[i];
			if ((texture_coordinates[i] >= data->image->minimums[i]) &&
				(texture_coordinates[i] <= data->image->maximums[i]))
			{
				offset += data->image->sizes[i] * 
					((texture_coordinates[i] - data->image->minimums[i])
					/ (data->image->maximums[i] - data->image->minimums[i]));
			}
			else
			{
				in_image = 0;
			}
		}
		/* 4. Copy values from image to field values cache */
		if (in_image)
		{
			values = ((FE_value *)data->image->data)+ offset * data->image->depth ;
			for (i = 0 ; (i < field->number_of_components) ; i++)
			{
				field->values[i] = values[i];
			}
		}
		else
		{
			for (i = 0 ; (i < field->number_of_components) ; i++)
			{
				field->values[i] = 0.5;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_sobel_filter_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_sobel_filter_evaluate_cache_in_element */

#define Computed_field_sobel_filter_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_sobel_filter_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_sobel_filter_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_sobel_filter_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_sobel_filter_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_sobel_filter_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_sobel_filter(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_sobel_filter);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,
			"    texture coordinate field : %s\n",field->source_fields[1]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_sobel_filter.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_sobel_filter */

static char *Computed_field_sobel_filter_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_sobel_filter_get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_sobel_filter_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " texture_coordinate_field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_sobel_filter_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_sobel_filter_get_command_string */

#define Computed_field_sobel_filter_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_sobel_filter(struct Computed_field *field,
	struct Computed_field *source_field,
	struct Computed_field *texture_coordinate_field,
	int dimension, int *sizes, FE_value *minimums, FE_value *maximums,
	int element_dimension, struct MANAGER(Computed_field) *computed_field_manager,
	struct Cmiss_region *region, struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_SOBEL_FILTER with the supplied
fields, <source_field> and <texture_coordinate_field>.  The <dimension> is the
size of the <sizes>, <minimums> and <maximums> vectors and should be less than
or equal to the number of components in the <texture_coordinate_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int depth, number_of_source_fields, return_code;
	struct Computed_field **source_fields;
	struct Computed_field_sobel_filter_type_specific_data *data;

	ENTER(Computed_field_set_type_sobel_filter);
	if (field && source_field && texture_coordinate_field &&
		(depth = source_field->number_of_components) &&
		(dimension <= texture_coordinate_field->number_of_components) &&
		region /* && user_interface can be NULL, just no special find_xi */)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=2;
		data = (struct Computed_field_sobel_filter_type_specific_data *)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, number_of_source_fields) &&
			ALLOCATE(data, struct Computed_field_sobel_filter_type_specific_data, 1) &&
			(data->image = ACCESS(Image_cache)(CREATE(Image_cache)())) && 
			Image_cache_update_dimension(
			data->image, dimension, depth, sizes, minimums, maximums) &&
			Image_cache_update_data_storage(data->image))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_sobel_filter_type_string;
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			source_fields[1]=ACCESS(Computed_field)(texture_coordinate_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			data->element_dimension = element_dimension;
			data->region = ACCESS(Cmiss_region)(region);
			data->user_interface = user_interface;
			data->computed_field_manager = computed_field_manager;
			data->computed_field_manager_callback_id = 
				MANAGER_REGISTER(Computed_field)(
				Computed_field_sobel_filter_field_change, (void *)field,
				computed_field_manager);
			
			field->type_specific_data = data;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(sobel_filter);
		}
		else
		{
			DEALLOCATE(source_fields);
			if (data)
			{
				if (data->image)
				{
					DESTROY(Image_cache)(&data->image);
				}
				DEALLOCATE(data);
			}
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_sobel_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_sobel_filter */

int Computed_field_get_type_sobel_filter(struct Computed_field *field,
	struct Computed_field **source_field,
	struct Computed_field **texture_coordinate_field,
	int *dimension, int **sizes, FE_value **minimums, FE_value **maximums,
	int *element_dimension)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
If the field is of type COMPUTED_FIELD_SOBEL_FILTER, the 
parameters defining it are returned.
==============================================================================*/
{
	int i, return_code;
	struct Computed_field_sobel_filter_type_specific_data *data;

	ENTER(Computed_field_get_type_sobel_filter);
	if (field && (field->type_string==computed_field_sobel_filter_type_string)
		&& (data = (struct Computed_field_sobel_filter_type_specific_data *)
		field->type_specific_data) && data->image)
	{
		*dimension = data->image->dimension;
		if (ALLOCATE(*sizes, int, *dimension)
			&& ALLOCATE(*minimums, FE_value, *dimension)
			&& ALLOCATE(*maximums, FE_value, *dimension))
		{
			*source_field = field->source_fields[0];
			*texture_coordinate_field = field->source_fields[1];
			for (i = 0 ; i < *dimension ; i++)
			{
				(*sizes)[i] = data->image->sizes[i];
				(*minimums)[i] = data->image->minimums[i];
				(*maximums)[i] = data->image->maximums[i];
			}
			*element_dimension = data->element_dimension;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_sobel_filter.  Unable to allocate vectors.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_sobel_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_sobel_filter */

static int define_Computed_field_type_sobel_filter(struct Parse_state *state,
	void *field_void, void *computed_field_image_processing_package_void)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_SOBEL_FILTER (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	FE_value *minimums, *maximums;
	int dimension, element_dimension, return_code, *sizes;
	struct Computed_field *field, *source_field, *texture_coordinate_field;
	struct Computed_field_image_processing_package 
		*computed_field_image_processing_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data,
		set_texture_coordinate_field_data;

	ENTER(define_Computed_field_type_sobel_filter);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_image_processing_package=
		(struct Computed_field_image_processing_package *)
		computed_field_image_processing_package_void))
	{
		return_code=1;
		source_field = (struct Computed_field *)NULL;
		texture_coordinate_field = (struct Computed_field *)NULL;
		dimension = 0;
		sizes = (int *)NULL;
		minimums = (FE_value *)NULL;
		maximums = (FE_value *)NULL;
		element_dimension = 0;
		/* field */
		set_source_field_data.computed_field_manager =
			computed_field_image_processing_package->computed_field_manager;
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		/* texture_coordinate_field */
		set_texture_coordinate_field_data.computed_field_manager =
			computed_field_image_processing_package->computed_field_manager;
		set_texture_coordinate_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_texture_coordinate_field_data.conditional_function_user_data = (void *)NULL;

		if (computed_field_sobel_filter_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_sobel_filter(field, 
				&source_field, &texture_coordinate_field, &dimension,
				&sizes, &minimums, &maximums, &element_dimension);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			if (texture_coordinate_field)
			{
				ACCESS(Computed_field)(texture_coordinate_field);
			}

			if ((current_token=state->current_token) &&
				(!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
				option_table = CREATE(Option_table)();
				/* dimension */
				Option_table_add_int_positive_entry(option_table, "dimension",
					&dimension);
				/* element_dimension */
				Option_table_add_int_non_negative_entry(option_table, "element_dimension",
					&element_dimension);
				/* field */
				Option_table_add_Computed_field_conditional_entry(option_table,
					"field", &source_field, &set_source_field_data);
				/* maximums */
				Option_table_add_FE_value_vector_entry(option_table,
					"maximums", maximums, &dimension);
				/* minimums */
				Option_table_add_FE_value_vector_entry(option_table,
					"minimums", minimums, &dimension);
				/* sizes */
				Option_table_add_int_vector_entry(option_table,
					"sizes", sizes, &dimension);
				/* texture_coordinate_field */
				Option_table_add_Computed_field_conditional_entry(option_table,
					"texture_coordinate_field", &texture_coordinate_field,
					&set_texture_coordinate_field_data);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			/* parse the dimension ... */
			if (return_code && (current_token = state->current_token))
			{
				/* ... only if the "dimension" token is next */
				if (fuzzy_string_compare(current_token, "dimension"))
				{
					option_table = CREATE(Option_table)();
					/* dimension */
					Option_table_add_int_positive_entry(option_table, "dimension",
						&dimension);
					if (return_code = Option_table_parse(option_table, state))
					{
						if (!(REALLOCATE(sizes, sizes, int, dimension) &&
							REALLOCATE(minimums, minimums, FE_value, dimension) &&
							REALLOCATE(maximums, maximums, FE_value, dimension)))
						{
							return_code = 0;
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			if (return_code && (dimension < 1))
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_scale.  Must specify a dimension first.");
				return_code = 0;
			}
			/* parse the rest of the table */
			if (return_code&&state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* element_dimension */
				Option_table_add_int_non_negative_entry(option_table, "element_dimension",
					&element_dimension);
				/* field */
				Option_table_add_Computed_field_conditional_entry(option_table,
					"field", &source_field, &set_source_field_data);
				/* maximums */
				Option_table_add_FE_value_vector_entry(option_table,
					"maximums", maximums, &dimension);
				/* minimums */
				Option_table_add_FE_value_vector_entry(option_table,
					"minimums", minimums, &dimension);
				/* sizes */
				Option_table_add_int_vector_entry(option_table,
					"sizes", sizes, &dimension);
				/* texture_coordinate_field */
				Option_table_add_Computed_field_conditional_entry(option_table,
					"texture_coordinate_field", &texture_coordinate_field,
					&set_texture_coordinate_field_data);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_sobel_filter(field,
					source_field, texture_coordinate_field, dimension,
					sizes, minimums, maximums, element_dimension,
					computed_field_image_processing_package->computed_field_manager,
					computed_field_image_processing_package->root_region,
					computed_field_image_processing_package->user_interface);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_sobel_filter.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			if (texture_coordinate_field)
			{
				DEACCESS(Computed_field)(&texture_coordinate_field);
			}
			if (sizes)
			{
				DEALLOCATE(sizes);
			}
			if (minimums)
			{
				DEALLOCATE(minimums);
			}
			if (maximums)
			{
				DEALLOCATE(maximums);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_sobel_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_sobel_filter */

int Computed_field_register_types_image_processing(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_image_processing_package 
		computed_field_image_processing_package;

	ENTER(Computed_field_register_types_image_processing);
	if (computed_field_package)
	{
		computed_field_image_processing_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		computed_field_image_processing_package.root_region = root_region;
		computed_field_image_processing_package.user_interface = user_interface;
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_sobel_filter_type_string, 
			define_Computed_field_type_sobel_filter,
			&computed_field_image_processing_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_image_processing.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_image_processing */

