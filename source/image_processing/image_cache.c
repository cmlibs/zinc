/******************************************************************
  FILE: image_cache.c

  LAST MODIFIED: 27 February 2004

  DESCRIPTION: Define Image_cache structure and implement basic operations
==================================================================*/
#include <math.h>
#include <stdio.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_set.h"
#include "general/image_utilities.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "image_processing/image_cache.h"



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

int Image_cache_update_dimension(struct Image_cache *image,
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

int Image_cache_update_data_storage(struct Image_cache *image)
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

int Image_cache_update_from_fields(struct Image_cache *image,
	struct Computed_field *source_field,
	struct Computed_field *texture_coordinate_field,
	int element_dimension,
	struct Cmiss_region *region, struct Graphics_buffer_package *graphics_buffer_package)
/*******************************************************************************
LAST MODIFIED : 12 May 2004

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
					region, element_dimension, graphics_buffer_package,
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

int Image_cache_evaluate_field(struct Image_cache *image, struct Computed_field *field)
/*******************************************************************************
    LAST MODIFIED: 23 February 2004
    DESCRIPTION: Copy the result image to computed_field
=================================================================================*/
{
        int i, in_image, offset, return_code;
	FE_value *texture_coordinates, *values;

	ENTER(Image_cache_evaluate_field);
	if(image)
	{
	        return_code = 1;
	       /* Evaluate texture coordinates */
		in_image = 1;
		offset = 0;
		texture_coordinates = field->source_fields[1]->values;
		for (i = image->dimension - 1 ; in_image && (i >= 0) ; i--)
		{
			offset *= image->sizes[i];
			if ((texture_coordinates[i] >= image->minimums[i]) &&
				(texture_coordinates[i] <= image->maximums[i]))
			{
				offset += (image->sizes[i] - 1) *
					((texture_coordinates[i] - image->minimums[i])
					/ (image->maximums[i] - image->minimums[i]));
			}
			else
			{
				in_image = 0;
			}
		}
		/* Copy values from image to field values cache */
		if (in_image)
		{
			values = ((FE_value *)image->data)+ offset * image->depth ;
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
			"Copy_image_to_field.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
}/* Image_cache_evaluate_field */
