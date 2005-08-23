/************************************************************************************
   FILE: computed_field_image_resample.c

   LAST MODIFIED: 12 July 2004

   DESCRIPTION: Perform image resample on Computed field using <nearest> and <bicubic>
   methods.
===================================================================================*/
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
#include <time.h>
#include "general/random.h"
#include <stdlib.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_set.h"
#include "image_processing/image_cache.h"
#include "general/image_utilities.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "image_processing/computed_field_image_resample.h"

#define my_Min(x,y) ((x) <= (y) ? (x) : (y))
#define my_Max(x,y) ((x) <= (y) ? (y) : (x))

struct Computed_field_image_resample_package
/*******************************************************************************
LAST MODIFIED : 17 February 2004

DESCRIPTION :
A container for objects required to define fields in this module.
==============================================================================*/
{
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Cmiss_region *root_region;
	struct Graphics_buffer_package *graphics_buffer_package;
};

struct Computed_field_image_resample_type_specific_data
{
	int dimension;
	char *mode; /* method <nearest> or <bicubic> */
	int *input_sizes;/* the sizes of the original image */
	int *output_sizes;/* the sizes of desired output image */
	float cached_time;
	/*int element_dimension;*/
	struct Cmiss_region *region;
	struct Graphics_buffer_package *graphics_buffer_package;
	struct Image_cache *image;
	struct MANAGER(Computed_field) *computed_field_manager;
	void *computed_field_manager_callback_id;
};

static char computed_field_image_resample_type_string[] = "image_resample";

int Computed_field_is_type_image_resample(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_image_resample);
	if (field)
	{
		return_code =
		  (field->type_string == computed_field_image_resample_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_image_resample.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_image_resample */

static void Computed_field_image_resample_field_change(
	struct MANAGER_MESSAGE(Computed_field) *message, void *field_void)
/*******************************************************************************
LAST MODIFIED : 5 April 2004

DESCRIPTION :
Manager function called back when either of the source fields change so that
we know to invalidate the image cache.
==============================================================================*/
{
	struct Computed_field *field;
	struct Computed_field_image_resample_type_specific_data *data;

	ENTER(Computed_field_image_resample_source_field_change);
	if (message && (field = (struct Computed_field *)field_void) && (data =
		(struct Computed_field_image_resample_type_specific_data *)
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
			"Computed_field_image_resample_source_field_change.  "
			"Invalid arguments.");
	}
	LEAVE;
} /* Computed_field_image_resample_source_field_change */

static int Computed_field_image_resample_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_image_resample_type_specific_data *data;

	ENTER(Computed_field_image_resample_clear_type_specific);
	if (field && (data =
		(struct Computed_field_image_resample_type_specific_data *)
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
		if (data->input_sizes)
		{
			DEALLOCATE(data->input_sizes);
		}
		if (data->output_sizes)
		{
			DEALLOCATE(data->output_sizes);
		}
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_image_resample_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_image_resample_clear_type_specific */

static void *Computed_field_image_resample_copy_type_specific(
	struct Computed_field *source_field, struct Computed_field *destination_field)
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_image_resample_type_specific_data *destination,
		*source;
	int i;

	ENTER(Computed_field_image_resample_copy_type_specific);
	if (source_field && destination_field && (source =
		(struct Computed_field_image_resample_type_specific_data *)
		source_field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_image_resample_type_specific_data, 1)&&
			ALLOCATE(destination->input_sizes, int, source->dimension) &&
			ALLOCATE(destination->output_sizes, int, source->dimension))
		{
			destination->dimension = source->dimension;
			destination->mode = source->mode;
			for (i = 0; i < source->dimension; i++)
			{
				destination->input_sizes[i] = source->input_sizes[i];
				destination->output_sizes[i] = source->output_sizes[i];
			}
			destination->cached_time = source->cached_time;
			destination->region = ACCESS(Cmiss_region)(source->region);
			/*destination->element_dimension = source->element_dimension;*/
			destination->graphics_buffer_package = source->graphics_buffer_package;
			destination->computed_field_manager = source->computed_field_manager;
			destination->computed_field_manager_callback_id =
				MANAGER_REGISTER(Computed_field)(
				Computed_field_image_resample_field_change, (void *)destination_field,
				destination->computed_field_manager);
			if (source->image)
			{
				destination->image = ACCESS(Image_cache)(CREATE(Image_cache)());
				Image_cache_update_dimension(destination->image,
					source->image->dimension, source->image->depth,
					source->input_sizes);
			}
			else
			{
				destination->image = (struct Image_cache *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_image_resample_copy_type_specific.  "
				"Unable to allocate memory.");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_image_resample_copy_type_specific.  "
			"Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_image_resample_copy_type_specific */

int Computed_field_image_resample_clear_cache_type_specific
   (struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_image_resample_type_specific_data *data;

	ENTER(Computed_field_image_resample_clear_type_specific);
	if (field && (data =
		(struct Computed_field_image_resample_type_specific_data *)
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
			"Computed_field_image_resample_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_image_resample_clear_type_specific */

static int Computed_field_image_resample_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;
	struct Computed_field_image_resample_type_specific_data *data,
		*other_data;

	ENTER(Computed_field_image_resample_type_specific_contents_match);
	if (field && other_computed_field && (data =
		(struct Computed_field_image_resample_type_specific_data *)
		field->type_specific_data) && (other_data =
		(struct Computed_field_image_resample_type_specific_data *)
		other_computed_field->type_specific_data))
	{
		if ((strcmp(data->mode, other_data->mode) == 0) &&
		        data->image && other_data->image &&
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
} /* Computed_field_image_resample_type_specific_contents_match */

#define Computed_field_image_resample_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_image_resample_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_image_resample_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_image_resample_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 4 May 2005

DESCRIPTION :
No special criteria.
==============================================================================*/
FE_value b3spline(FE_value x)
/*******************************************************************************
LAST MODIFIED : 4 May 2005

DESCRIPTION :
Spline function.
==============================================================================*/    
{
	FE_value a, b, c, d;

	if((x + 2.0) <= 0.0) a = 0.0; else a = (FE_value)pow((x + 2.0), 3.0);
	if((x + 1.0) <= 0.0) b = 0.0; else b = (FE_value)pow((x + 1.0), 3.0);
	if(x <= 0) c = 0.0; else c = (FE_value)pow(x, 3.0);  
	if((x - 1.0) <= 0.0) d = 0.0; else d = (FE_value)pow((x - 1.0), 3.0);

	return (0.1667 * (a - (4.0 * b) + (6.0 * c) - (4.0 * d)));
}/* b3spline */

void Bicubic_resample2d(FE_value *data_index, int depth,
       int *sizes, int i_x, int i_y, FE_value a, FE_value b, FE_value *sum)
/*******************************************************************************
LAST MODIFIED : 4 May 2005

DESCRIPTION :
2D spline interpolating.
==============================================================================*/      
{
        int xx, yy;
	int m, n, k;
	FE_value r1, r2;
	
        for (k = 0; k < depth; k++)
	{
		sum[k] = 0.0;
	}
	for(m=-1; m<3; m++) 
	{
		r1 = b3spline((FE_value) m - a);
		for(n=-1; n<3; n++) 
		{
		        r2 = b3spline(-1.0*((FE_value)n - b)); 
			xx = i_x+n+2;
			yy = i_y+m+2;
			if (xx < 0) xx=0;
			if (yy < 0) yy=0;
			if (xx >= sizes[0]) 
			{
				xx = sizes[0]-1;
			}
			if (yy >= sizes[1]) 
			{
				yy = sizes[1]-1;
			}
			for (k = 0; k < depth; k++)
			{
				sum[k] += data_index[(yy * sizes[0] + xx) * depth + k] * r1 * r2;
			}
		}
	}
}/* Bicubic_resample2d */

void Bicubic_resample3d(FE_value *data_index, int depth, int *sizes, 
        int i_x, int i_y, int i_z, FE_value a, FE_value b, FE_value c, FE_value *sum)
/*******************************************************************************
LAST MODIFIED : 4 May 2005

DESCRIPTION :
3D spline interpolating.
==============================================================================*/    
{
        int xx, yy, zz;
	int l, m, n, k;
	FE_value r1, r2, r3;
	
        for (k = 0; k < depth; k++)
	{
		sum[k] = 0.0;
	}
	
	for (l = -1; l < 3; l++)
	{
		r1 = b3spline((FE_value) l - a);
		for (m=-1; m<3; m++) 
		{
			r2 = b3spline((FE_value) m - b);
			for(n=-1; n<3; n++) 
			{
				r3 = b3spline(-1.0*((FE_value)n - c)); 
				xx = i_x+n+2;
				yy = i_y+m+2;
				zz = i_z+l+2;
				if (xx < 0) xx=0;
				if (yy < 0) yy=0;
				if (zz < 0) zz=0;
				if (xx >= sizes[0]) 
				{
					xx = sizes[0]-1;
				}
				if (yy >= sizes[1]) 
				{
					yy = sizes[1]-1;
				}
				if (zz >= sizes[2]) 
				{
					zz = sizes[2]-1;
				}
				for (k = 0; k < depth; k++)
				{
					sum[k] += data_index[(zz* sizes[1] * sizes[0] 
					   + yy * sizes[0] + xx) * depth + k] * r1 * r2 * r3;
				}
			}
		}
	}
}/* Bicubic_resample3d */

static int Image_cache_image_resample(struct Image_cache *image, 
         int dimension, int *output_sizes, char *mode)
/*******************************************************************************
LAST MODIFIED : 4 May 2005

DESCRIPTION :
Perform image resample on the image cache. The <nearest> method sacles an image
into the desired dimensions with pixel sampling.  Unlike other scaling methods, 
this method does not introduce any additional color into the scaled image.
The <bicubic> method using bicubic interpolating (spline filter) to generate a new image.
==============================================================================*/
{
	char *storage;
	FE_value *data_index, *result_index;
	int  return_code, out_storage_size;
	int i, flag1, flag2, flag;
	
	ENTER(Image_cache_image_resample);
	if (image && (dimension == image->dimension) && (image->depth > 0))
	{
	        return_code = 1;
		flag1 = 0;
		flag2 = 0;
		for (i = 0; i < dimension; i++)
		{
		        flag1 += output_sizes[i];
			if (image->sizes[i] == output_sizes[i])
			{   
			        /*do nothing */
			}
			else
			{
			        flag2++;
			}
		}
		if (flag1 == 0)
		{
		        display_message(ERROR_MESSAGE, "Image_cache_image_resample.  "
			"Invalid output sizes.");
		        return_code = 0;
		}
		else if (flag2 == 0)
		{        
		        /* do nothing */
		}
		else
		{
                	
			out_storage_size = image->depth;

			for (i = 0 ; i < dimension ; i++)
			{
				out_storage_size *= output_sizes[i];
			}
			if (ALLOCATE(storage,char, out_storage_size * sizeof(FE_value)))
			{
                        	return_code = 1;
				if (strcmp(mode,"nearest") == 0)
				{
			        	flag = 1;
				}
				else if (strcmp(mode,"bicubic") == 0)
				{
			        	flag = 2;
				}
				data_index = (FE_value *)image->data;
				result_index = (FE_value *)storage;
				for (i = 0; i < out_storage_size; i++)
				{
			        	*result_index = 0.0;
					result_index++;
				}
				result_index = (FE_value *)storage;
				switch (flag)
				{
				        case 1: 
						{
						        if (image->dimension == 2)
							{
							        int x, y, k, n, xctr, yctr;
								
								for (y = 0; y < output_sizes[1]; y++)
								{
								        for (x = 0; x < output_sizes[0]; x++)
									{
									        xctr = (int)(((FE_value)x * ((FE_value)image->sizes[0] - 1.0) / ((FE_value)output_sizes[0]-1.0)));
						                                yctr = (int)(((FE_value)y * ((FE_value)image->sizes[1] - 1.0) / ((FE_value)output_sizes[1]-1.0)));

										n = yctr * (FE_value)image->sizes[0];
										n += xctr;
										
										for (k = 0; k< image->depth; k++)
										{
										        result_index[k] = *(data_index + n * image->depth + k);
										}
										result_index += image->depth;
									}
								}
							}
							else if (image->dimension == 3)
							{
							        int x, y, z, n, k, xctr, yctr, zctr;
								
								for (z = 0; z < output_sizes[2]; z++)
								{
								        for (y = 0; y< output_sizes[1]; y++)
									{
									        for (x =0; x < output_sizes[0]; x++)
										{
			                                                                 xctr = (int)(((FE_value)x * ((FE_value)image->sizes[0] - 1.0) / ((FE_value)output_sizes[0]-1.0)));
						                                         yctr = (int)(((FE_value)y * ((FE_value)image->sizes[1] - 1.0) / ((FE_value)output_sizes[1]-1.0)));
						                                         zctr = (int)(((FE_value)z * ((FE_value)image->sizes[2] - 1.0) / ((FE_value)output_sizes[2]-1.0)));
										         n = yctr* (FE_value)image->sizes[0];
										         n += xctr;
										         n += zctr* (FE_value)image->sizes[0] * (FE_value)image->sizes[1];
										         
										         for (k = 0; k< image->depth; k++)
										         {
										                  result_index[k] = *(data_index + n * image->depth + k);
										         }
										         result_index += image->depth;
										}
									}
								}
								
							}
							/*int j, k, d, org_step;
							FE_value n;
					        	for (i = 0; i < out_storage_size/image->depth; i++)
					        	{
						        	d = i;
								org_step = 1;
								n = 0.0;
								for (j = 0; j < image->dimension; j++)
								{
							        	n += (FE_value)(d % output_sizes[j]) * 
								      (FE_value)(image->sizes[j] * org_step) / (FE_value)output_sizes[j];
									d /= image->sizes[j];
									org_step *= image->sizes[j];
								}
								for (k = 0; k < image->depth; k++)
								{
							        	result_index[k] = *(data_index + ((int)n) * image->depth + k);
								}
								result_index += image->depth;	      
					        	}*/
					        	break;
						}
					case 2: 
					        if (image->dimension == 2)
						{
						        FE_value f_x, f_y, a, b, yScale, xScale;
							int   i_x, i_y, k;
							int   x, y, storage_size;
							FE_value *sum;
							FE_value *data_index1;
							storage_size = image->depth;
							for (i = 0 ; i < dimension ; i++)
							{
							        storage_size *= image->sizes[i];
							}
							if (ALLOCATE(sum, FE_value, image->depth)&&
							         ALLOCATE(data_index1, FE_value, storage_size))
							{
							         return_code = 1;
								for (i = 0; i < storage_size; i++)
								{
							        	data_index1[i] = *data_index;
									data_index++;
								}
								yScale = (FE_value)image->sizes[1]/(FE_value)output_sizes[1];
								xScale = (FE_value)image->sizes[0]/(FE_value)output_sizes[0];
								for(y = 0; y < output_sizes[1]; y++)
								{
									f_y = (FE_value) y * yScale;
									i_y = (int) floor(f_y);
									a   = f_y - (FE_value)floor(f_y);
									for(x = 0; x < output_sizes[0]; x++)
									{
										f_x = (FE_value) x * xScale;
										i_x = (int) floor(f_x);
										b   = f_x - (FE_value)floor(f_x);
                                                                        	Bicubic_resample2d(data_index1, 
									        	image->depth, image->sizes, i_x, i_y, 
									        	a, b, sum);
									
										for (k = 0; k < image->depth; k++)
										{
									        	result_index[k] = sum[k];
										}
										result_index += image->depth;
									}
								}
							}
							else
							{
							        display_message(ERROR_MESSAGE,
				                                 "Image_cache_image_resample.  Not enough memory");
				                                return_code = 0;
							}
						}
						else if (image->dimension == 3)
						{
						        FE_value f_x, f_y, f_z, a, b, c, yScale, xScale, zScale;
							int   i_x, i_y, i_z, k;
							int   x, y, z, storage_size;
							FE_value *sum;
							FE_value *data_index1;
							storage_size = image->depth;
							for (i = 0 ; i < dimension ; i++)
							{
							        storage_size *= image->sizes[i];
							}
							
							if (ALLOCATE(sum, FE_value, image->depth) &&
							         ALLOCATE(data_index1, FE_value, storage_size))
							{
							        return_code = 1;
								for (i = 0; i < storage_size; i++)
								{
							        	data_index1[i] = *data_index;
									data_index++;
								}
								zScale = (FE_value)image->sizes[2]/(FE_value)output_sizes[2];
								yScale = (FE_value)image->sizes[1]/(FE_value)output_sizes[1];
								xScale = (FE_value)image->sizes[0]/(FE_value)output_sizes[0];
								for(z = 0; z < output_sizes[2]; z++)
								{
									f_z = (FE_value) z * zScale;
									i_z = (int) floor(f_z);
									a   = f_z - (FE_value)floor(f_z);
									for(y = 0; y < output_sizes[1]; y++)
							        	{
								        	f_y = (FE_value) y * yScale;
								        	i_y = (int) floor(f_y);
								        	b   = f_y - (FE_value)floor(f_y);
								        	for(x = 0; x < output_sizes[0]; x++)
								        	{
									        	f_x = (FE_value) x * xScale;
									        	i_x = (int) floor(f_x);
									        	c   = f_x - (FE_value)floor(f_x);

									        	Bicubic_resample3d(data_index1, 
										        	image->depth, image->sizes, 
										        	i_x, i_y, i_z, a, b, c, sum);
									        	for (k = 0; k < image->depth; k++)
									        	{
									                	result_index[k] = sum[k];
									        	}
									        	result_index += image->depth;
								        	}
									}
								}
							}
							else
							{
							        display_message(ERROR_MESSAGE,
				                                 "Image_cache_image_resample.  Not enough memory");
				                                return_code = 0;
							}
						}
					        break;
					default:
					        display_message(ERROR_MESSAGE,
				                                 "Image_cache_image_resample.  Invalide arguments.");
				                return_code = 0;
				}
			        
				if (return_code)
				{
					DEALLOCATE(image->data);
					image->data = storage;
					image->valid = 1;
					for (i = 0 ; i < dimension ; i++)
					{
						image->sizes[i] = output_sizes[i];
					}
				}
				else
				{
					DEALLOCATE(storage);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
				"Image_cache_image_resample.  Not enough memory");
				return_code = 0;
			}
		}
       	}
	else
	{
		display_message(ERROR_MESSAGE, "Image_cache_image_resample.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Image_cache_image_resample */

static int Computed_field_image_resample_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;
	struct Computed_field_image_resample_type_specific_data *data;

	ENTER(Computed_field_image_resample_evaluate_cache_at_node);
	if (field && node &&
		(data = (struct Computed_field_image_resample_type_specific_data *)field->type_specific_data))
	{
		return_code = 1;
		/* 1. Precalculate the Image_cache */
		if (!data->image->valid)
		{
		        Image_cache_update_dimension(data->image,
				data->dimension, data->image->depth,
				data->input_sizes);
			return_code = Image_cache_update_from_fields(data->image, field->source_fields[0],
				field->source_fields[1],  data->region,
				data->graphics_buffer_package);
			/* 2. Perform image processing operation */

			return_code = Image_cache_image_resample(data->image,
				data->dimension, data->output_sizes, data->mode);
		}
		/* 3. Evaluate texture coordinates and copy image to field */
		Computed_field_evaluate_cache_at_node(field->source_fields[1],
			node, time);
		Image_cache_evaluate_field(data->image,field);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_image_resample_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_image_resample_evaluate_cache_at_node */

static int Computed_field_image_resample_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;
	struct Computed_field_image_resample_type_specific_data *data;

	ENTER(Computed_field_image_resample_evaluate_cache_in_element);
	USE_PARAMETER(calculate_derivatives);
	if (field && element && xi && (field->number_of_source_fields > 0) &&
		(data = (struct Computed_field_image_resample_type_specific_data *) field->type_specific_data) &&
		data->image )
	{
		return_code = 1;
		/* 1. Precalculate the Image_cache */
		if (!data->image->valid)
		{
			return_code = Image_cache_update_from_fields(data->image, field->source_fields[0],
				field->source_fields[1],  data->region,
				data->graphics_buffer_package);
			/* 2. Perform image processing operation */
			return_code = Image_cache_image_resample(data->image, 
				data->dimension, data->output_sizes, data->mode);
		}
		/* 3. Evaluate texture coordinates and copy image to field */
		Computed_field_evaluate_cache_in_element(field->source_fields[1],
			element, xi, time, top_level_element, /*calculate_derivatives*/0);
		Image_cache_evaluate_field(data->image,field);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_image_resample_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_image_resample_evaluate_cache_in_element */

#define Computed_field_image_resample_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_image_resample_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_image_resample_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_image_resample_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_image_resample_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_image_resample_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

int Computed_field_image_resample_get_native_resolution(struct Computed_field *field,
        int *dimension, int **sizes, 
	struct Computed_field **texture_coordinate_field)
/*******************************************************************************
LAST MODIFIED : 4 February 2005

DESCRIPTION :
Gets the <dimension>, <sizes>, <minimums>, <maximums> and <texture_coordinate_field> from
the <field>. These parameters will be used in image processing.

==============================================================================*/
{       
        int return_code;
	struct Computed_field_image_resample_type_specific_data *data;
	
	ENTER(Computed_field_image_resample_get_native_resolution);
	if (field && (data =
		(struct Computed_field_image_resample_type_specific_data *)
		field->type_specific_data) && data->image)
	{
		Image_cache_get_native_resolution(data->image,
			dimension, sizes);
		/* Texture_coordinate_field from source fields */
		if (*texture_coordinate_field)
		{
			REACCESS(Computed_field)(&(*texture_coordinate_field), field->source_fields[1]);
		}
		else
		{
		        *texture_coordinate_field = ACCESS(Computed_field)(field->source_fields[1]);
		}	 
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_median_filter_get_native_resolution.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_image_resample_get_native_resolution */

static int list_Computed_field_image_resample(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 4 April 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_image_resample_type_specific_data *data;

	ENTER(List_Computed_field_image_resample);
	if (field && (field->type_string==computed_field_image_resample_type_string)
		&& (data = (struct Computed_field_image_resample_type_specific_data *)
		field->type_specific_data))
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,
			"    texture coordinate field : %s\n",field->source_fields[1]->name);
		display_message(INFORMATION_MESSAGE,
			"    mode : %s\n", data->mode);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_image_resample.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_image_resample */

static char *Computed_field_image_resample_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 4 April 2004

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error;
	struct Computed_field_image_resample_type_specific_data *data;

	ENTER(Computed_field_image_resample_get_command_string);
	command_string = (char *)NULL;
	if (field && (field->type_string==computed_field_image_resample_type_string)
		&& (data = (struct Computed_field_image_resample_type_specific_data *)
		field->type_specific_data) )
	{
		error = 0;
		append_string(&command_string,
			computed_field_image_resample_type_string, &error);
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
		sprintf(temp_string, " dimension %d ", data->dimension);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " mode %s ", data->mode);
		append_string(&command_string, temp_string, &error);
		
		sprintf(temp_string, " input_sizes %d %d %d",
		                    data->input_sizes[0],data->input_sizes[1],data->input_sizes[2]);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " output_sizes %d %d %d",
		                    data->output_sizes[0],data->output_sizes[1],data->output_sizes[2]);
		append_string(&command_string, temp_string, &error);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_image_resample_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_image_resample_get_command_string */

#define Computed_field_image_resample_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 4 April 2004

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_image_resample(struct Computed_field *field,
	struct Computed_field *source_field,
	struct Computed_field *texture_coordinate_field,
	int dimension, 
	int nearest_index, int bicubic_index, 
	int *input_sizes, int *output_sizes,
	struct MANAGER(Computed_field) *computed_field_manager,
	struct Cmiss_region *region, struct Graphics_buffer_package *graphics_buffer_package)
/*******************************************************************************
LAST MODIFIED : 4 May 2005

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_image_resample with the supplied
fields, <source_field> and <texture_coordinate_field>.  The <dimension> is the
size of the <sizes>.
==============================================================================*/
{
	int depth, number_of_source_fields, return_code, i;
	struct Computed_field **source_fields;
	struct Computed_field_image_resample_type_specific_data *data;

	ENTER(Computed_field_set_type_image_resample);
	if (field && source_field && texture_coordinate_field &&
		(depth = source_field->number_of_components) &&
		(dimension <= texture_coordinate_field->number_of_components) &&
		region && graphics_buffer_package)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=2;
		data = (struct Computed_field_image_resample_type_specific_data *)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, number_of_source_fields) &&
			ALLOCATE(data, struct Computed_field_image_resample_type_specific_data, 1) &&
			ALLOCATE(data->input_sizes, int, dimension) &&
			ALLOCATE(data->output_sizes, int, dimension) &&
			(data->image = ACCESS(Image_cache)(CREATE(Image_cache)())) &&
			Image_cache_update_dimension(
			data->image, dimension, depth, input_sizes) &&
			Image_cache_update_data_storage(data->image))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_image_resample_type_string;
			if (nearest_index > 0)
			{
			        data->mode = "nearest";
			}
			else if (bicubic_index > 0)
			{
			        data->mode = "bicubic";
			}
			
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			source_fields[1]=ACCESS(Computed_field)(texture_coordinate_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			data->dimension = dimension;
			for (i = 0; i < dimension; i++)
			{
				data->input_sizes[i] = input_sizes[i];
				data->output_sizes[i] = output_sizes[i];
			}
			/*data->element_dimension = element_dimension;*/
			data->region = ACCESS(Cmiss_region)(region);
			data->graphics_buffer_package = graphics_buffer_package;
			data->computed_field_manager = computed_field_manager;
			data->computed_field_manager_callback_id =
				MANAGER_REGISTER(Computed_field)(
				Computed_field_image_resample_field_change, (void *)field,
				computed_field_manager);

			field->type_specific_data = data;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(image_resample);
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
			"Computed_field_set_type_image_resample.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_image_resample */

int Computed_field_get_type_image_resample(struct Computed_field *field,
	struct Computed_field **source_field,
	struct Computed_field **texture_coordinate_field,
	int *dimension, 
	int *nearest_index, int *bicubic_index, 
	int **input_sizes, int **output_sizes)
/*******************************************************************************
LAST MODIFIED : 4 May 2005

DESCRIPTION :
If the field is of type COMPUTED_FIELD_image_resample, the
parameters defining it are returned.
==============================================================================*/
{
	int i, return_code;
	struct Computed_field_image_resample_type_specific_data *data;

	ENTER(Computed_field_get_type_image_resample);
	if (field && (field->type_string==computed_field_image_resample_type_string)
		&& (data = (struct Computed_field_image_resample_type_specific_data *)
		field->type_specific_data) && data->image)
	{
		*dimension = data->image->dimension;
		if (ALLOCATE(*input_sizes, int, *dimension)
			&& ALLOCATE(*output_sizes, int, *dimension))
		{
			*source_field = field->source_fields[0];
			*texture_coordinate_field = field->source_fields[1];
			for (i = 0 ; i < *dimension ; i++)
			{
				(*input_sizes)[i] = data->input_sizes[i];
				(*output_sizes)[i] = data->output_sizes[i];
			}
			*nearest_index = *bicubic_index  = 0;
			if (strcmp(data->mode, "nearest") == 0)
			{
				*nearest_index = 1;
			}
			else if (strcmp(data->mode, "bicubic") == 0)
			{
				*bicubic_index = 1;
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_image_resample.  Unable to allocate vectors.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_image_resample.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_image_resample */

static int define_Computed_field_type_image_resample(struct Parse_state *state,
	void *field_void, void *computed_field_image_resample_package_void)
/*******************************************************************************
LAST MODIFIED : 12 July 2004

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_image_resample (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
        char *current_token;
	char nearest_string[] = "nearest", bicubic_string[] = "bicubic";
	int dimension, return_code, *input_sizes, *output_sizes;
	
	struct Computed_field *field, *source_field, *texture_coordinate_field;
	struct Computed_field_image_resample_package
		*computed_field_image_resample_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data,
		set_texture_coordinate_field_data;
	struct Set_names_from_list_data mode;

	ENTER(define_Computed_field_type_image_resample);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_image_resample_package=
		(struct Computed_field_image_resample_package *)
		computed_field_image_resample_package_void))
	{
		return_code=1;
		source_field = (struct Computed_field *)NULL;
		texture_coordinate_field = (struct Computed_field *)NULL;
		dimension = 0;
		input_sizes = (int *)NULL;
		output_sizes = (int *)NULL;
		/* field */
		set_source_field_data.computed_field_manager =
			computed_field_image_resample_package->computed_field_manager;
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		/* modes */
		mode.number_of_tokens = 2;
		ALLOCATE(mode.tokens, struct Set_names_from_list_token, 2);
		mode.tokens[0].string = nearest_string;
		mode.tokens[0].index = 0;
		mode.tokens[1].string = bicubic_string;
		mode.tokens[1].index = 0;
		
		/* texture_coordinate_field */
		set_texture_coordinate_field_data.computed_field_manager =
			computed_field_image_resample_package->computed_field_manager;
		set_texture_coordinate_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_texture_coordinate_field_data.conditional_function_user_data = (void *)NULL;

		if (computed_field_image_resample_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_image_resample(field,
				&source_field, &texture_coordinate_field, &dimension,  
				&mode.tokens[0].index, &mode.tokens[1].index, 
				&input_sizes, &output_sizes);
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
				/* field */
				Option_table_add_Computed_field_conditional_entry(option_table,
					"field", &source_field, &set_source_field_data);
				/* modes */
				Option_table_add_set_names_from_list_entry(option_table,
					"mode", &mode);
				/* input_sizes */
				Option_table_add_int_vector_entry(option_table,
					"input_sizes", input_sizes, &dimension);
				/* output_sizes */
				Option_table_add_int_vector_entry(option_table,
					"output_sizes", output_sizes, &dimension);
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
							REALLOCATE(output_sizes, output_sizes, int, dimension)))
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
				/* field */
				Option_table_add_Computed_field_conditional_entry(option_table,
					"field", &source_field, &set_source_field_data);
				/* modes */
				Option_table_add_set_names_from_list_entry(option_table,
					"mode", &mode);
				/* input_sizes */
				Option_table_add_int_vector_entry(option_table,
					"input_sizes", input_sizes, &dimension);
				/* output_sizes */
				Option_table_add_int_vector_entry(option_table,
					"output_sizes", output_sizes, &dimension);
				/* texture_coordinate_field */
				Option_table_add_Computed_field_conditional_entry(option_table,
					"texture_coordinate_field", &texture_coordinate_field,
					&set_texture_coordinate_field_data);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			if (source_field && (!input_sizes ||!texture_coordinate_field))
			{
			        return_code = Computed_field_get_native_resolution(source_field,
				     &dimension,&input_sizes,&texture_coordinate_field);
			}
			
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_image_resample(field,
					source_field, texture_coordinate_field, dimension, mode.tokens[0].index, mode.tokens[1].index,
					input_sizes, output_sizes, 
					computed_field_image_resample_package->computed_field_manager,
					computed_field_image_resample_package->root_region,
					computed_field_image_resample_package->graphics_buffer_package);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_image_resample.  Failed");
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
			if (input_sizes)
			{
				DEALLOCATE(input_sizes);
			}
			if (output_sizes)
			{
				DEALLOCATE(output_sizes);
			}
		}
		DEALLOCATE(mode.tokens);
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
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package)
/*******************************************************************************
LAST MODIFIED : 12 April 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_image_resample_package
		computed_field_image_resample_package;

	ENTER(Computed_field_register_types_image_resample);
	if (computed_field_package)
	{
		computed_field_image_resample_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		computed_field_image_resample_package.root_region = root_region;
		computed_field_image_resample_package.graphics_buffer_package = graphics_buffer_package;
		return_code = Computed_field_package_add_type(computed_field_package,
			            computed_field_image_resample_type_string,
			            define_Computed_field_type_image_resample,
			            &computed_field_image_resample_package);
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

