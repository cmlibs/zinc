/******************************************************************
  FILE: computed_field_image_correlation.c

  LAST MODIFIED: 2 July 2004

  DESCRIPTION:Implement images correlation computing
==================================================================*/
#include <math.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_set.h"
#include "image_processing/image_cache.h"
#include "general/image_utilities.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "image_processing/computed_field_image_correlation.h"

#define my_Min(x,y) ((x) <= (y) ? (x) : (y))
#define my_Max(x,y) ((x) <= (y) ? (y) : (x))


struct Computed_field_image_correlation_package
/*******************************************************************************
LAST MODIFIED : 2 July 2004

DESCRIPTION :
A container for objects required to define fields in this module.
==============================================================================*/
{
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Cmiss_region *root_region;
	struct Graphics_buffer_package *graphics_buffer_package;
};

struct Computed_field_image_correlation_type_specific_data
{
        int dimension;
	int *input_sizes;
	int *output_sizes;
	int *template_sizes;
	struct Image_cache *template_image;
	float cached_time;
	int element_dimension;
	struct Cmiss_region *region;
	struct Graphics_buffer_package *graphics_buffer_package;
	struct Image_cache *image;
	struct MANAGER(Computed_field) *computed_field_manager;
	void *computed_field_manager_callback_id;
};

static char computed_field_image_correlation_type_string[] = "image_correlation";

int Computed_field_is_type_image_correlation(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 2 July 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;


	ENTER(Computed_field_is_type_image_correlation);
	if (field)
	{
		return_code =
		  (field->type_string == computed_field_image_correlation_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_image_correlation.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_image_correlation */

static void Computed_field_image_correlation_field_change(
	struct MANAGER_MESSAGE(Computed_field) *message, void *field_void)
/*******************************************************************************
LAST MODIFIED : 2 July 2004

DESCRIPTION :
Manager function called back when either of the source fields change so that
we know to invalidate the image cache.
==============================================================================*/
{
	struct Computed_field *field;
	struct Computed_field_image_correlation_type_specific_data *data;

	ENTER(Computed_field_image_correlation_source_field_change);
	if (message && (field = (struct Computed_field *)field_void) && (data =
		(struct Computed_field_image_correlation_type_specific_data *)
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
					field->source_fields[1], message->changed_object_list) ||
					Computed_field_depends_on_Computed_field_in_list(
					field->source_fields[2], message->changed_object_list))
				{
					if (data->image)
					{
						data->image->valid = 0;
					}
					if (data->template_image)
					{
						data->template_image->valid = 0;
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
			"Computed_field_image_correlation_source_field_change.  "
			"Invalid arguments.");
	}
	LEAVE;
} /* Computed_field_image_correlation_source_field_change */

static int Computed_field_image_correlation_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 2 July 2004

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_image_correlation_type_specific_data *data;

	ENTER(Computed_field_image_correlation_clear_type_specific);
	if (field && (data =
		(struct Computed_field_image_correlation_type_specific_data *)
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
		if (data->template_image)
		{
			DEACCESS(Image_cache)(&data->template_image);
		}
		if (data->computed_field_manager && data->computed_field_manager_callback_id)
		{
			MANAGER_DEREGISTER(Computed_field)(
				data->computed_field_manager_callback_id,
				data->computed_field_manager);
		}
		if (data->input_sizes)
		{
			DEALLOCATE(data->input_sizes);
		}
		if (data->output_sizes)
		{
			DEALLOCATE(data->output_sizes);
		}
		if (data->template_sizes)
		{
			DEALLOCATE(data->template_sizes);
		}
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_image_correlation_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_image_correlation_clear_type_specific */

static void *Computed_field_image_correlation_copy_type_specific(
	struct Computed_field *source_field, struct Computed_field *destination_field)
/*******************************************************************************
LAST MODIFIED : 2 July 2004

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	int i;
	struct Computed_field_image_correlation_type_specific_data *destination,
		*source;

	ENTER(Computed_field_image_correlation_copy_type_specific);
	if (source_field && destination_field && (source =
		(struct Computed_field_image_correlation_type_specific_data *)
		source_field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_image_correlation_type_specific_data, 1)
		   && ALLOCATE(destination->input_sizes, int, source->dimension)
		   && ALLOCATE(destination->output_sizes, int, source->dimension)
		   && ALLOCATE(destination->template_sizes, int, source->dimension))
		{
			destination->dimension = source->dimension;
			for (i = 0 ; i < source->dimension ; i++)
			{
				destination->input_sizes[i] = source->input_sizes[i];
				destination->output_sizes[i] = source->output_sizes[i];
				destination->template_sizes[i] = source->template_sizes[i];
			}
			destination->cached_time = source->cached_time;
			destination->region = ACCESS(Cmiss_region)(source->region);
			destination->element_dimension = source->element_dimension;
			destination->graphics_buffer_package = source->graphics_buffer_package;
			destination->computed_field_manager = source->computed_field_manager;
			destination->computed_field_manager_callback_id =
				MANAGER_REGISTER(Computed_field)(
				Computed_field_image_correlation_field_change, (void *)destination_field,
				destination->computed_field_manager);
			if (source->image)
			{
				destination->image = ACCESS(Image_cache)(CREATE(Image_cache)());
				Image_cache_update_dimension(destination->image,
					source->dimension, source->image->depth,
					source->input_sizes, source->image->minimums,
					source->image->maximums);
			}
			else
			{
				destination->image = (struct Image_cache *)NULL;
			}
			if (source->template_image)
			{
				destination->template_image = ACCESS(Image_cache)(CREATE(Image_cache)());
				Image_cache_update_dimension(destination->template_image,
					source->dimension, source->template_image->depth,
					source->template_sizes, source->template_image->minimums,
					source->template_image->maximums);
			}
			else
			{
				destination->template_image = (struct Image_cache *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_image_correlation_copy_type_specific.  "
				"Unable to allocate memory.");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_image_correlation_copy_type_specific.  "
			"Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_image_correlation_copy_type_specific */

int Computed_field_image_correlation_clear_cache_type_specific
   (struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 2 July 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_image_correlation_type_specific_data *data;

	ENTER(Computed_field_image_correlation_clear_type_specific);
	if (field && (data =
		(struct Computed_field_image_correlation_type_specific_data *)
		field->type_specific_data))
	{
		if (data->image)
		{
			/* data->image->valid = 0; */
		}
		if (data->template_image)
		{

		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_image_correlation_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_image_correlation_clear_type_specific */

static int Computed_field_image_correlation_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;
	struct Computed_field_image_correlation_type_specific_data *data,
		*other_data;

	ENTER(Computed_field_image_correlation_type_specific_contents_match);
	if (field && other_computed_field && (data =
		(struct Computed_field_image_correlation_type_specific_data *)
		field->type_specific_data) && (other_data =
		(struct Computed_field_image_correlation_type_specific_data *)
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
} /* Computed_field_image_correlation_type_specific_contents_match */

#define Computed_field_image_correlation_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_image_correlation_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_image_correlation_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_image_correlation_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Image_cache_image_correlation(struct Image_cache *image,
	struct Image_cache *template_image, int dimension, int *output_sizes, int *template_sizes)
/*******************************************************************************
LAST MODIFIED : 2 July 2004

DESCRIPTION :
Comput images correlation/weighted averaging on the image cache.
==============================================================================*/
{
	char *storage;
	FE_value *data_index, *result_index, *weights_index;
	int  return_code,  storage_size, weights_size, data_size;
	int i, j, k,m;
	int *xout, *xin;
	int cur_pt, weight_step, image_step, *offsets;
	FE_value *mean, *tval;
	ENTER(Image_cache_image_correlation);
	if (image && (dimension == image->dimension) && (image->dimension > 0)
		&& (image->depth > 0) && (image->depth == template_image->depth))
	{
	        return_code = 1;
		data_size = 1;
		weights_size = template_image->depth;
		storage_size = image->depth;
 		for (i = 0 ; i < dimension ; i++)
		{
			storage_size *= output_sizes[i];
			weights_size *= template_sizes[i];
			data_size *= image->sizes[i];
		}
		if (ALLOCATE(storage, char, storage_size * sizeof(FE_value))
		         && ALLOCATE(tval, FE_value, template_image->depth)
			 && ALLOCATE(xout, int, dimension)
			 && ALLOCATE(xin, int, dimension)
			 && ALLOCATE(offsets, int, weights_size)
			 && ALLOCATE(mean, FE_value, image->depth))
		{
                        return_code = 1;
			data_index = (FE_value *)image->data;
			weights_index = (FE_value *)template_image->data;
			result_index = (FE_value *)storage;
			for (i = 0; i < storage_size; i++)
			{
			        *result_index = 0.0;
				result_index++;
			}
			result_index = (FE_value *)storage;
			for (k = 0; k < template_image->depth; k++)
			{
			        tval[k] = 0.0;
			}
			for (j = 0; j < weights_size / template_image->depth; j++)
			{
			        for (k = 0; k < template_image->depth; k++)
				{
				        tval[k] += *(weights_index + k);
				}
				weights_index += template_image->depth;
				offsets[j] = 0.0;
			}

			for(j = 0; j < weights_size / template_image->depth; j++)
			{
			        weight_step = 1;
				image_step = 1;
				for(m = 0; m < image->dimension; m++)
				{
				        k = ((int)((FE_value)j/((FE_value)weight_step))) % template_sizes[m];
					offsets[j] += (k - template_sizes[m] / 2) * image_step * image->depth;
					weight_step *= template_sizes[m];
					image_step *= image->sizes[m];
				}
			}
                        for (i = 0; i < storage_size / image->depth; i++)
			{
			        weights_index = (FE_value *)template_image->data;
			        cur_pt = i;
			        for (m = 0; m < dimension; m++)
				{
				        xout[m] = cur_pt % output_sizes[m];
					cur_pt = cur_pt / output_sizes[m];
					xin[m] = (int)(((FE_value)xout[m] + 0.5) * (((FE_value)image->sizes[m] - 1.0) / ((FE_value)output_sizes[m])));
				}
				cur_pt = xin[dimension - 1];
				for (m = dimension - 2; m >= 0; m--)
				{
				        cur_pt = cur_pt * image->sizes[m] + xin[m];
				}
				for ( k = 0; k < image->depth; k++)
				{
				        mean[k] = 0.0;
				}
				for (j = 0; j < weights_size / template_image->depth; j ++)
				{
				        for (k = 0; k < image->depth; k++)
					{
					        if ((cur_pt * image->depth + offsets[j]) < 0)
						{
						        mean[k] += *(data_index + offsets[j] + (cur_pt + data_size) * image->depth + k) * *(weights_index + k);
						}
						else if ((cur_pt * image->depth + offsets[j]) >= data_size * image->depth)
						{
						        mean[k] += *(data_index + offsets[j] + (cur_pt - data_size) * image->depth + k) * *(weights_index + k);
						}
						else
						{
						        mean[k] += *(data_index + offsets[j] + cur_pt * image->depth + k) * *(weights_index + k);
						}
					}
					weights_index += template_image->depth;
				}

				for (k = 0; k < image->depth; k++)
				{
					result_index[k] = mean[k]/tval[k];
				}
				result_index += image->depth;
			}

        	        if (return_code)
        	        {
			        DEALLOCATE(image->data);
				image->data = storage;
				for (i = 0 ; i < dimension ; i++)
				{
					image->sizes[i] = output_sizes[i];
				}
				image->valid = 1;
        	        }
			else
			{
                                DEALLOCATE(storage);
			}
        		DEALLOCATE(tval);
			DEALLOCATE(xin);
			DEALLOCATE(xout);
			DEALLOCATE(offsets);
			DEALLOCATE(mean);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Image_cache_image_correlation.  Not enough memory");
			return_code = 0;
		}
       	}
	else
	{
		display_message(ERROR_MESSAGE, "Image_cache_image_correlation.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Image_cache_image_correlation */

static int Computed_field_image_correlation_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 2 July 2004

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{

	int return_code;
	struct Computed_field_image_correlation_type_specific_data *data;
	ENTER(Computed_field_image_correlation_evaluate_cache_at_node);
	if (field && node &&
		(data = (struct Computed_field_image_correlation_type_specific_data *)field->type_specific_data))
	{
		return_code = 1;

			/* 1. Precalculate the Image_cache */
			if (!data->image->valid)
			{
				return_code = Image_cache_update_from_fields(data->image, field->source_fields[0],
				         field->source_fields[1], data->element_dimension, data->region,
				        data->graphics_buffer_package);
				return_code = Image_cache_update_from_fields(data->template_image, field->source_fields[2],
				         field->source_fields[1], data->element_dimension, data->region,
				        data->graphics_buffer_package);
				/* 2. Perform image processing operation */
				return_code = Image_cache_image_correlation(data->image,
					data->template_image, data->dimension,
					data->output_sizes, data->template_sizes);
			}
			/* 3. Evaluate texture coordinates and copy image to field */
			Computed_field_evaluate_cache_at_node(field->source_fields[1],
			            node, time);
			Image_cache_evaluate_field(data->image,field);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_image_correlation_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_image_correlation_evaluate_cache_at_node */

static int Computed_field_image_correlation_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 2 July 2004

DESCRIPTION :
Evaluate the fields cache at the node.
===================================================================data->image===========*/
{

	int return_code;

	struct Computed_field_image_correlation_type_specific_data *data;

	ENTER(Computed_field_image_correlation_evaluate_cache_in_element);
	USE_PARAMETER(calculate_derivatives);
	if (field && element && xi && (field->number_of_source_fields > 0) &&
		(field->number_of_components == field->source_fields[0]->number_of_components) &&
		(data = (struct Computed_field_image_correlation_type_specific_data *) field->type_specific_data) &&
		data->image && (field->number_of_components == data->image->depth))
	{
		return_code = 1;

			/* 1. Precalculate the Image_cache */
			if (!data->image->valid)
			{
				return_code = Image_cache_update_from_fields(data->image, field->source_fields[0],
				        field->source_fields[1], data->element_dimension, data->region,
				        data->graphics_buffer_package);
				return_code = Image_cache_update_from_fields(data->template_image, field->source_fields[2],
				        field->source_fields[1], data->element_dimension, data->region,
				        data->graphics_buffer_package);
				/* 2. Perform image processing operation */
				return_code = Image_cache_image_correlation(data->image,
				        data->template_image, data->dimension,
				        data->output_sizes, data->template_sizes);
			}
			/* 3. Evaluate texture coordinates and copy image to field */
			Computed_field_evaluate_cache_in_element(field->source_fields[1],
				element, xi, time, top_level_element, /*calculate_derivatives*/0);
			Image_cache_evaluate_field(data->image,field);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_image_correlation_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_image_correlation_evaluate_cache_in_element */

#define Computed_field_image_correlation_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_image_correlation_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_image_correlation_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_image_correlation_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_image_correlation_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_image_correlation_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

int Computed_field_image_correlation_get_native_resolution(struct Computed_field *field,
        int *dimension, int **sizes, FE_value **minimums, FE_value **maximums,
	struct Computed_field **texture_coordinate_field)
/*******************************************************************************
LAST MODIFIED : 4 February 2005

DESCRIPTION :
Gets the <dimension>, <sizes>, <minimums>, <maximums> and <texture_coordinate_field> from
the <field>. These parameters will be used in image processing.

==============================================================================*/
{       
        int return_code;
	struct Computed_field_image_correlation_type_specific_data *data;
	
	ENTER(Computed_field_image_correlation_get_native_resolution);
	if (field && (data =
		(struct Computed_field_image_correlation_type_specific_data *)
		field->type_specific_data) && data->image)
	{
		Image_cache_get_native_resolution(data->image,
			dimension, sizes, minimums, maximums);
		/* Texture_coordinate_field from source fields */
		if (*texture_coordinate_field)
		{
			/* DEACCESS(Computed_field)(&(*texture_coordinate_field));
			*texture_coordinate_field = ACCESS(Computed_field)(field->source_fields[1]); */
		}
		else
		{
		        *texture_coordinate_field = ACCESS(Computed_field)(field->source_fields[1]);
		}	 
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_image_correlation_get_native_resolution.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_image_correlation_get_native_resolution */

static int list_Computed_field_image_correlation(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 2 July 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_image_correlation_type_specific_data *data;

	ENTER(List_Computed_field_image_correlation);
	if (field && (field->type_string==computed_field_image_correlation_type_string)
		&& (data = (struct Computed_field_image_correlation_type_specific_data *)
		field->type_specific_data))
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,
			"    texture coordinate field : %s\n",field->source_fields[1]->name);
		display_message(INFORMATION_MESSAGE,
			"    template_image field : %s\n", field->source_fields[2]->name);
		display_message(INFORMATION_MESSAGE,
			"    dimension : %d\n", data->dimension);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_image_correlation.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_image_correlation */

static char *Computed_field_image_correlation_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 2 July 2004

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	char temp_string[40];
	int error;
	struct Computed_field_image_correlation_type_specific_data *data;

	ENTER(Computed_field_image_correlation_get_command_string);
	command_string = (char *)NULL;
	if (field&& (field->type_string==computed_field_image_correlation_type_string)
		&& (data = (struct Computed_field_image_correlation_type_specific_data *)
		field->type_specific_data) )
	{
		error = 0;
		append_string(&command_string,
			computed_field_image_correlation_type_string, &error);
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
		append_string(&command_string, " template_field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[2], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		sprintf(temp_string, " dimension %d", data->image->dimension);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " input_sizes %d %d",
		                    data->input_sizes[0],data->input_sizes[1]);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " template_sizes %d %d",
		                    data->template_sizes[0],data->template_sizes[1]);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " output_sizes %d %d",
		                    data->output_sizes[0],data->output_sizes[1]);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " minimums %f %f",
		                    data->image->minimums[0], data->image->minimums[1]);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " maximums %f %f",
		                    data->image->maximums[0], data->image->maximums[1]);
		append_string(&command_string, temp_string, &error);;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_image_correlation_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_image_correlation_get_command_string */

#define Computed_field_image_correlation_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 2 July 2004

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_image_correlation(struct Computed_field *field,
	struct Computed_field *source_field,
	struct Computed_field *texture_coordinate_field,
	struct Computed_field *template_field,
	int dimension,
	int *input_sizes, int *output_sizes, int *template_sizes,
	FE_value *minimums, FE_value *maximums,
	int element_dimension, struct MANAGER(Computed_field) *computed_field_manager,
	struct Cmiss_region *region, struct Graphics_buffer_package *graphics_buffer_package)
/*******************************************************************************
LAST MODIFIED : 17 December 2003

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_image_correlation with the supplied
fields, <source_field> and <texture_coordinate_field>.  The <threshold_value> specifies
half the width and height of the filter window.  The <dimension> is the
size of the <sizes>, <minimums> and <maximums> vectors and should be less than
or equal to the number of components in the <texture_coordinate_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int i, depth, template_image_depth, number_of_source_fields, return_code;
	struct Computed_field **source_fields;
	struct Computed_field_image_correlation_type_specific_data *data;

	ENTER(Computed_field_set_type_image_correlation);
	if (field && source_field && texture_coordinate_field && template_field
		&& (depth = source_field->number_of_components) &&
		(template_image_depth = template_field->number_of_components) &&
		(dimension <= texture_coordinate_field->number_of_components) &&
		region && graphics_buffer_package)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=3;
		data = (struct Computed_field_image_correlation_type_specific_data *)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, number_of_source_fields) &&
			ALLOCATE(data, struct Computed_field_image_correlation_type_specific_data, 1) &&
			ALLOCATE(data->input_sizes, int, dimension) &&
			ALLOCATE(data->output_sizes, int, dimension) &&
			ALLOCATE(data->template_sizes, int, dimension) &&
			(data->image = ACCESS(Image_cache)(CREATE(Image_cache)())) &&
			Image_cache_update_dimension(
			data->image, dimension, depth, input_sizes, minimums, maximums) &&
			Image_cache_update_data_storage(data->image)&&
			(data->template_image = ACCESS(Image_cache)(CREATE(Image_cache)())) &&
			Image_cache_update_dimension(
			data->template_image, dimension, template_image_depth, template_sizes, minimums, maximums) &&
			Image_cache_update_data_storage(data->template_image))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_image_correlation_type_string;
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			source_fields[1]=ACCESS(Computed_field)(texture_coordinate_field);
			source_fields[2]=ACCESS(Computed_field)(template_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			data->dimension = dimension;

			for (i = 0 ; i < dimension ; i++)
			{
				data->input_sizes[i] = input_sizes[i];
				data->output_sizes[i] = output_sizes[i];
				data->template_sizes[i] = template_sizes[i];
			}
			data->element_dimension = element_dimension;
			data->region = ACCESS(Cmiss_region)(region);
			data->graphics_buffer_package = graphics_buffer_package;
			data->computed_field_manager = computed_field_manager;
			data->computed_field_manager_callback_id =
				MANAGER_REGISTER(Computed_field)(
				Computed_field_image_correlation_field_change, (void *)field,
				computed_field_manager);

			field->type_specific_data = data;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(image_correlation);
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
				if (data->template_image)
				{
					DESTROY(Image_cache)(&data->template_image);
				}
				DEALLOCATE(data);
			}
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_image_correlation.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_image_correlation */

int Computed_field_get_type_image_correlation(struct Computed_field *field,
	struct Computed_field **source_field,
	struct Computed_field **texture_coordinate_field,
	struct Computed_field **template_field,
	int *dimension,
	int **input_sizes, int **output_sizes, int **template_sizes,
	FE_value **minimums, FE_value **maximums, int *element_dimension)
/*******************************************************************************
LAST MODIFIED : 17 December 2003

DESCRIPTION :
If the field is of type COMPUTED_FIELD_image_correlation, the
parameters defining it are returned.
==============================================================================*/
{
	int i, return_code;
	struct Computed_field_image_correlation_type_specific_data *data;

	ENTER(Computed_field_get_type_image_correlation);
	if (field && (field->type_string==computed_field_image_correlation_type_string)
		&& (data = (struct Computed_field_image_correlation_type_specific_data *)
		field->type_specific_data) && data->image && data->template_image)
	{
		*dimension = data->dimension;

		if (ALLOCATE(*input_sizes, int, *dimension)
			&& ALLOCATE(*output_sizes, int, *dimension)
			&& ALLOCATE(*template_sizes, int, *dimension)
			&& ALLOCATE(*minimums, FE_value, *dimension)
			&& ALLOCATE(*maximums, FE_value, *dimension))
		{
			*source_field = field->source_fields[0];
			*texture_coordinate_field = field->source_fields[1];
			*template_field = field->source_fields[2];

			for (i = 0 ; i < *dimension ; i++)
			{
				(*input_sizes)[i] = data->input_sizes[i];
				(*output_sizes)[i] = data->output_sizes[i];
				(*template_sizes)[i] = data->template_sizes[i];
				(*minimums)[i] = data->image->minimums[i];
				(*maximums)[i] = data->image->maximums[i];
			}
			*element_dimension = data->element_dimension;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_image_correlation.  Unable to allocate vectors.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_image_correlation.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_image_correlation */

static int define_Computed_field_type_image_correlation(struct Parse_state *state,
	void *field_void, void *computed_field_image_correlation_package_void)
/*******************************************************************************
LAST MODIFIED : 2 July 2004

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_image_correlation (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	FE_value *minimums, *maximums;
	int dimension, element_dimension;
	int *input_sizes, *output_sizes, *template_sizes, return_code;
	struct Computed_field *field, *source_field, *texture_coordinate_field, *template_field;
	struct Computed_field_image_correlation_package
		*computed_field_image_correlation_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data,
		set_texture_coordinate_field_data, set_template_field_data;

	ENTER(define_Computed_field_type_image_correlation);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_image_correlation_package=
		(struct Computed_field_image_correlation_package *)
		computed_field_image_correlation_package_void))
	{
		return_code=1;
		source_field = (struct Computed_field *)NULL;
		texture_coordinate_field = (struct Computed_field *)NULL;
		template_field = (struct Computed_field *)NULL;
		dimension = 0;
		input_sizes = (int *)NULL;
		output_sizes = (int *)NULL;
		template_sizes = (int *)NULL;
		minimums = (FE_value *)NULL;
		maximums = (FE_value *)NULL;
		element_dimension = 0;

		/* field */
		set_source_field_data.computed_field_manager =
			computed_field_image_correlation_package->computed_field_manager;
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		/* texture_coordinate_field */
		set_texture_coordinate_field_data.computed_field_manager =
			computed_field_image_correlation_package->computed_field_manager;
		set_texture_coordinate_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_texture_coordinate_field_data.conditional_function_user_data = (void *)NULL;
		/* template_field */
		set_template_field_data.computed_field_manager =
			computed_field_image_correlation_package->computed_field_manager;
		set_template_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_template_field_data.conditional_function_user_data = (void *)NULL;

		if (computed_field_image_correlation_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_image_correlation(field,
				&source_field, &texture_coordinate_field, &template_field,
				&dimension, &input_sizes, &output_sizes, &template_sizes,
				&minimums, &maximums, &element_dimension);
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
			if (template_field)
			{
				ACCESS(Computed_field)(template_field);
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
				/* input_sizes */
				Option_table_add_int_vector_entry(option_table,
					"input_sizes", input_sizes, &dimension);
				/* maximums */
				Option_table_add_FE_value_vector_entry(option_table,
					"maximums", maximums, &dimension);
				/* minimums */
				Option_table_add_FE_value_vector_entry(option_table,
					"minimums", minimums, &dimension);
				/* output_sizes */
				Option_table_add_int_vector_entry(option_table,
					"output_sizes", output_sizes, &dimension);
				/* weights_field */
				Option_table_add_Computed_field_conditional_entry(option_table,
					"template_field", &template_field, &set_template_field_data);

				/* template_sizes */
				Option_table_add_int_vector_entry(option_table,
					"template_sizes", template_sizes, &dimension);
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
						if (!(REALLOCATE(input_sizes, input_sizes, int, dimension) &&
							REALLOCATE(output_sizes, output_sizes, int, dimension) &&
							REALLOCATE(template_sizes, template_sizes, int, dimension) &&
							REALLOCATE(minimums, minimums, FE_value, dimension) &&
							REALLOCATE(maximums, maximums, FE_value, dimension)))
						{
							return_code = 0;
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			/*if (return_code && (dimension < 1))
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_scale.  Must specify a dimension first.");
				return_code = 0;
			}*/
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
				/* input_sizes */
				Option_table_add_int_vector_entry(option_table,
					"input_sizes", input_sizes, &dimension);
				/* maximums */
				Option_table_add_FE_value_vector_entry(option_table,
					"maximums", maximums, &dimension);
				/* minimums */
				Option_table_add_FE_value_vector_entry(option_table,
					"minimums", minimums, &dimension);
				/* output_sizes */
				Option_table_add_int_vector_entry(option_table,
					"output_sizes", output_sizes, &dimension);
				/* template_field */
				Option_table_add_Computed_field_conditional_entry(option_table,
					"template_field", &template_field, &set_template_field_data);

				/* template_sizes */
				Option_table_add_int_vector_entry(option_table,
					"template_sizes", template_sizes, &dimension);
				/* texture_coordinate_field */
				Option_table_add_Computed_field_conditional_entry(option_table,
					"texture_coordinate_field", &texture_coordinate_field,
					&set_texture_coordinate_field_data);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			if (dimension < 1)
			{
			        return_code = Computed_field_get_native_resolution(source_field,
				     &dimension,&input_sizes,&minimums,&maximums,&texture_coordinate_field);
			}
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_image_correlation(field,
					source_field, texture_coordinate_field, template_field, dimension,
					input_sizes, output_sizes, template_sizes,
					minimums, maximums, element_dimension,
					computed_field_image_correlation_package->computed_field_manager,
					computed_field_image_correlation_package->root_region,
					computed_field_image_correlation_package->graphics_buffer_package);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_image_correlation.  Failed");
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
			if (template_field)
			{
				DEACCESS(Computed_field)(&template_field);
			}
			if (input_sizes)
			{
				DEALLOCATE(input_sizes);
			}
			if (output_sizes)
			{
				DEALLOCATE(output_sizes);
			}
			if (template_sizes)
			{
				DEALLOCATE(template_sizes);
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
			"define_Computed_field_type_image_correlation.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_image_correlation */

int Computed_field_register_types_image_correlation(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package)
/*******************************************************************************
LAST MODIFIED : 12 December 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_image_correlation_package
		computed_field_image_correlation_package;

	ENTER(Computed_field_register_types_image_correlation);
	if (computed_field_package)
	{
		computed_field_image_correlation_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		computed_field_image_correlation_package.root_region = root_region;
		computed_field_image_correlation_package.graphics_buffer_package = graphics_buffer_package;
		return_code = Computed_field_package_add_type(computed_field_package,
			            computed_field_image_correlation_type_string,
			            define_Computed_field_type_image_correlation,
			            &computed_field_image_correlation_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_image_correlation.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_image_correlation */

