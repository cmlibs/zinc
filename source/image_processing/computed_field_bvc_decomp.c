
/********************************************************************
   FILE: computed_field_bvc_decomp.c

   LAST MODIFIED: 15 March 2005

   DESCRIPTION: Implement image decomposition by minimizing a convex 
                functional of two variables.
                The method split an image into a sum of a bounded variation 
		component (bvc) and a oscillating component (oc) which 
		contains the texture and the noise.
		The minimization of the functional is based on an orthogonal
		projection algorithm.
====================================================================*/
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
#include "image_processing/computed_field_bvc_decomp.h"

#define my_Min(x,y) ((x) <= (y) ? (x) : (y))
#define my_Max(x,y) ((x) <= (y) ? (y) : (x))

struct Computed_field_bvc_decomp_package
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

struct Computed_field_bvc_decomp_type_specific_data
{
        char *result; /* <bounded_variation>, <oscillating>, or <reconstruction>  */
	int number_of_iterations; 
	double tou; /* less than 1/8 */
	double lambda; /* large than 0.0 */
	double mu; /* large than 0.0 */
	float cached_time;
	int element_dimension;
	struct Cmiss_region *region;
	struct Graphics_buffer_package *graphics_buffer_package;
	struct Image_cache *image;
	struct MANAGER(Computed_field) *computed_field_manager;
	void *computed_field_manager_callback_id;
};
static char computed_field_bvc_decomp_type_string[] = "bvc_decomp";

int Computed_field_is_type_bvc_decomp(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_bvc_decomp);
	if (field)
	{
		return_code =
		  (field->type_string == computed_field_bvc_decomp_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_bvc_decomp.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_bvc_decomp */

static void Computed_field_bvc_decomp_field_change(
	struct MANAGER_MESSAGE(Computed_field) *message, void *field_void)
/*******************************************************************************
LAST MODIFIED : 5 December 2003

DESCRIPTION :
Manager function called back when either of the source fields change so that
we know to invalidate the image cache.
==============================================================================*/
{
	struct Computed_field *field;
	struct Computed_field_bvc_decomp_type_specific_data *data;

	ENTER(Computed_field_bvc_decomp_source_field_change);
	if (message && (field = (struct Computed_field *)field_void) && (data =
		(struct Computed_field_bvc_decomp_type_specific_data *)
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
			"Computed_field_bvc_decomp_source_field_change.  "
			"Invalid arguments.");
	}
	LEAVE;
} /* Computed_field_bvc_decomp_source_field_change */

static int Computed_field_bvc_decomp_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_bvc_decomp_type_specific_data *data;

	ENTER(Computed_field_bvc_decomp_clear_type_specific);
	if (field && (data =
		(struct Computed_field_bvc_decomp_type_specific_data *)
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
			"Computed_field_bvc_decomp_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_bvc_decomp_clear_type_specific */

static void *Computed_field_bvc_decomp_copy_type_specific(
	struct Computed_field *source_field, struct Computed_field *destination_field)
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_bvc_decomp_type_specific_data *destination,
		*source;

	ENTER(Computed_field_bvc_decomp_copy_type_specific);
	if (source_field && destination_field && (source =
		(struct Computed_field_bvc_decomp_type_specific_data *)
		source_field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_bvc_decomp_type_specific_data, 1))
		{
		        destination->result = source->result;
			destination->number_of_iterations = source->number_of_iterations; 
			destination->tou = source->tou;
			destination->lambda = source->lambda;
			destination->mu = source->mu;
			destination->cached_time = source->cached_time;
			destination->region = ACCESS(Cmiss_region)(source->region);
			destination->element_dimension = source->element_dimension;
			destination->graphics_buffer_package = source->graphics_buffer_package;
			destination->computed_field_manager = source->computed_field_manager;
			destination->computed_field_manager_callback_id =
				MANAGER_REGISTER(Computed_field)(
				Computed_field_bvc_decomp_field_change, (void *)destination_field,
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
				"Computed_field_bvc_decomp_copy_type_specific.  "
				"Unable to allocate memory.");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_bvc_decomp_copy_type_specific.  "
			"Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_bvc_decomp_copy_type_specific */

int Computed_field_bvc_decomp_clear_cache_type_specific
   (struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_bvc_decomp_type_specific_data *data;

	ENTER(Computed_field_bvc_decomp_clear_type_specific);
	if (field && (data =
		(struct Computed_field_bvc_decomp_type_specific_data *)
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
			"Computed_field_bvc_decomp_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_bvc_decomp_clear_type_specific */

static int Computed_field_bvc_decomp_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;
	struct Computed_field_bvc_decomp_type_specific_data *data,
		*other_data;

	ENTER(Computed_field_bvc_decomp_type_specific_contents_match);
	if (field && other_computed_field && (data =
		(struct Computed_field_bvc_decomp_type_specific_data *)
		field->type_specific_data) && (other_data =
		(struct Computed_field_bvc_decomp_type_specific_data *)
		other_computed_field->type_specific_data))
	{
		if (data->image && other_data->image &&
		        (strcmp(data->result, other_data->result) == 0)&&
			(data->number_of_iterations == other_data->number_of_iterations) && 
			(data->tou == other_data->tou) &&
			(data->lambda == other_data->lambda) &&
			(data->mu == other_data->mu) &&
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
} /* Computed_field_bvc_decomp_type_specific_contents_match */

#define Computed_field_bvc_decomp_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_bvc_decomp_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_bvc_decomp_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_bvc_decomp_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :
No special criteria.
==============================================================================*/

int orthogonal_projection(struct Image_cache *image, 
            FE_value mean, double tou, double lambda, 
	    FE_value *q1_index, FE_value *q2_index, FE_value *v_index)
{
        FE_value *data_index;
	int x, y, storage_size, return_code;
	FE_value *delta1, *delta2, *diff, norm;
	
	storage_size = image->sizes[0] * image->sizes[1];
	if (ALLOCATE(diff, FE_value, storage_size) &&
			ALLOCATE(delta1, FE_value, storage_size) &&
			ALLOCATE(delta2, FE_value, storage_size) )
	{
	        return_code = 1;
	        data_index = (FE_value *)image->data;
        	for (y = 0; y < image->sizes[1]; y++)
		{
			for (x = 0; x < image->sizes[0]; x++)
			{
				if ((y > 0) && (y < (image->sizes[1] - 1)))
				{
					diff[y*image->sizes[0] + x] = q2_index[y*image->sizes[0] + x] - q2_index[(y-1)*image->sizes[0] + x];
				}
				else if (y == 0)
				{
					diff[y*image->sizes[0] + x] = q2_index[y*image->sizes[0] + x];
				}
				else if (y == (image->sizes[1] - 1))
				{
					diff[y*image->sizes[0] + x] = - q2_index[(y-1)*image->sizes[0] + x];
				}
				if ((x > 0) && (x < (image->sizes[0] - 1)))
				{
					diff[y*image->sizes[0] + x] += q1_index[y*image->sizes[0] + x] - q1_index[y*image->sizes[0] + x -1];
				}
				else if (x == 0)
				{
					diff[y*image->sizes[0] + x] += q1_index[y*image->sizes[0] + x];
				}
				else if (x == (image->sizes[0] - 1))
				{
					diff[y*image->sizes[0] + x] += - q1_index[y*image->sizes[0] + x - 1];
				}
				diff[y*image->sizes[0] + x] -= ((*data_index - mean) - v_index[y*image->sizes[0] + x])  / lambda;
				data_index += image->depth;
			}
		}
		for (y = 0; y < image->sizes[1]; y++)
		{
			for (x = 0; x < image->sizes[0]; x++)
			{
				if ( y < (image->sizes[1] -1))
				{
					delta2[y*image->sizes[0] + x] = diff[(y+1)*image->sizes[0] + x] - diff[y*image->sizes[0] + x];
				}
				else
				{
					delta2[y*image->sizes[0] + x] = 0.0;
				}
				if ( x < (image->sizes[0] -1))
				{
					delta1[y*image->sizes[0] + x] = diff[y*image->sizes[0] + x + 1] - diff[y*image->sizes[0] + x];
				}
				else
				{
					delta1[y*image->sizes[0] + x] = 0.0;
				}
			}
		} 
		for (y = 0; y < image->sizes[1]; y++)
		{
			for (x = 0; x < image->sizes[0]; x++)
			{
				q1_index[y*image->sizes[0] + x] += tou * delta1[y*image->sizes[0] + x];
				q2_index[y*image->sizes[0] + x] += tou * delta2[y*image->sizes[0] + x];
				norm = sqrt(delta1[y*image->sizes[0] + x] * delta1[y*image->sizes[0] + x] + delta2[y*image->sizes[0] + x] * delta2[y*image->sizes[0] + x]);
				q1_index[y*image->sizes[0] + x] /= (1.0 + tou * norm);
				q2_index[y*image->sizes[0] + x] /= (1.0 + tou * norm);  
			}
		}
	        DEALLOCATE(diff);
		DEALLOCATE(delta1);
		DEALLOCATE(delta2);
	}
	else
	{
	        display_message(ERROR_MESSAGE,
			"orthogonal_projection.  "
			"Not enough memory.");
		return_code = 0;
	}
	
	return (return_code);				
}/* orthogonal_projection */


static int Image_cache_bvc_decomp(struct Image_cache *image, char *result, int number_of_iterations,
            double tou, double lambda, double mu)
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :  Perform a image decomposition operation in Image_cache.
The method decomps the input image into the sume of a bounded variation
component and an oscillating component, f = u + v. The oscillating component v 
contains the texture and the noise. This method is based on minimizing a functional 
of two variables. When a variable is fixed, a orthogonal projection method 
is used to minimize the functional:
                F(u,v) = J(u) + (1/2*lambda) ||f-u-v||^2, 
where,   v in {G_{mu}}^d = {div(g)|g = (g1,g2), g1, g2 in L^2(R) and ||g||_{infinite} <= lambda}
         u in BV, J(u) is the total variation of u. 

REFERENCE: J._F. Aujol, et al., "Image decomposition into a bounded variation component and an oscillating component," J. of Math. Imaging and Vision, Vol. 22: 71-88, 2005
==============================================================================*/
{
	char *storage;
	FE_value *data_index, *result_index;
	FE_value *u_index, *v_index, *q1_index, *q2_index;
	int i, k, return_code, x, y;
	int c, n, flag;
	int storage_size;
	FE_value min, max, mean;
	/*FE_value *delta1, *delta2, *diff;*/

	ENTER(Image_cache_bvc_decomp);
	if (image && (image->dimension > 0) && (image->depth > 0))
	{
		return_code = 1;
		/* tou = 0.1; */
		/* lambda = 0.1; */
		/* mu = 100.0; */
		max = -100000.0;
		min = 100000.0;
		
		/* Allocate a new storage block for our data */
		storage_size = image->depth;
		for (i = 0 ; i < image->dimension ; i++)
		{
			storage_size *= image->sizes[i];
		}
		if (ALLOCATE(u_index, FE_value, (storage_size/image->depth)) &&
			ALLOCATE(v_index, FE_value, (storage_size/image->depth)) &&
			ALLOCATE(q1_index, FE_value, (storage_size/image->depth)) &&
			ALLOCATE(q2_index, FE_value, (storage_size/image->depth)) &&
			ALLOCATE(storage, char, storage_size * sizeof(FE_value)))
		{
			result_index = (FE_value *)storage;
			for (i = 0 ; i < storage_size ; i++)
			{
				*result_index = 0.0;
				result_index++;
			}
                        
			mean = 0.0;
			data_index = (FE_value *)image->data;
			for (i = 0; i < storage_size / image->depth; i++)
			{
			        mean += *data_index;
				data_index += image->depth;
			}
			mean /= (FE_value)(storage_size/image->depth);
			data_index = (FE_value *)image->data;
			for (i = 0; i < storage_size/image->depth; i++)
			{
			        u_index[i] = (*data_index - mean);
				v_index[i] = 0.0;
				data_index += image->depth;
			}
			data_index = (FE_value *)image->data;
			result_index = (FE_value *)storage;
			for (n = 0; n < number_of_iterations; n++)
			{
			        /*1. Fixed v, search for u using orthogonal operator:
				     u = f - v - P_{G}(f-v) */
				for (i = 0; i < storage_size/image->depth; i++)
				{
				        q1_index[i] = 0.0;
					q2_index[i] = 0.0;
				}
				for (c = 0; c < number_of_iterations; c++)
				{ 
				        orthogonal_projection(image, mean, tou, lambda, 
	                                       q1_index, q2_index, v_index);  
				}
				for (y = 0; y < image->sizes[1]; y++)
				{
					for (x = 0; x < image->sizes[0]; x++)
					{
						if ((y > 0) && (y < (image->sizes[1] - 1)))
						{
							u_index[y*image->sizes[0] + x] = q2_index[y*image->sizes[0] + x] - q2_index[(y-1)*image->sizes[0] + x];
						}
						else if (y == 0)
						{
							u_index[y*image->sizes[0] + x] = q2_index[y*image->sizes[0] + x];
						}
						else if (y == (image->sizes[1] - 1))
						{
							u_index[y*image->sizes[0] + x] = - q2_index[(y-1)*image->sizes[0] + x];
						}
						if ((x > 0) && (x < (image->sizes[0] - 1)))
						{
							u_index[y*image->sizes[0] + x] += q1_index[y*image->sizes[0] + x] - q1_index[y*image->sizes[0] + x -1];
						}
						else if (x == 0)
						{
						        u_index[y*image->sizes[0] + x] += q1_index[y*image->sizes[0] + x];
						}
						else if (x == (image->sizes[0] - 1))
						{
							u_index[y*image->sizes[0] + x] += - q1_index[y*image->sizes[0] + x - 1];
						}
						u_index[y*image->sizes[0] + x] = (*data_index - mean)- v_index[y*image->sizes[0] + x] - lambda * u_index[y*image->sizes[0] + x];
						/*u_index[y*image->sizes[0] + x] = *data_index - v_index[y*image->sizes[0] + x] - u_index[y*image->sizes[0] + x];*/
						data_index += image->depth;
					}
				}
				/*2. Fixed u, search for v using orthogonal operator:
				        v = P_{G}(f - u)*/
				data_index = (FE_value *)image->data;
				for (i = 0; i < storage_size/image->depth; i++)
				{
				        q1_index[i] = 0.0;
					q2_index[i] = 0.0;
				}
				for (c = 0; c < number_of_iterations; c++)
				{ 
				        orthogonal_projection(image, mean, tou, mu, 
	                                       q1_index, q2_index, u_index);   
				}
				for (y = 0; y < image->sizes[1]; y++)
				{
					for (x = 0; x < image->sizes[0]; x++)
					{
						if ((y > 0) && (y < (image->sizes[1] - 1)))
						{
							v_index[y*image->sizes[0] + x] = q2_index[y*image->sizes[0] + x] - q2_index[(y-1)*image->sizes[0] + x];
						}
						else if (y == 0)
						{
							v_index[y*image->sizes[0] + x] = q2_index[y*image->sizes[0] + x];
						}
						else if (y == (image->sizes[1] - 1))
						{
							v_index[y*image->sizes[0] + x] = - q2_index[(y-1)*image->sizes[0] + x];
						}
						if ((x > 0) && (x < (image->sizes[0] - 1)))
						{
							v_index[y*image->sizes[0] + x] += q1_index[y*image->sizes[0] + x] - q1_index[y*image->sizes[0] + x -1];
						}
						else if (x == 0)
						{
						        v_index[y*image->sizes[0] + x] += q1_index[y*image->sizes[0] + x];
						}
						else if (x == (image->sizes[0] - 1))
						{
							v_index[y*image->sizes[0] + x] += - q1_index[y*image->sizes[0] + x - 1];
						}
						v_index[y*image->sizes[0] + x] *= mu;
					}
				}
			}
			if (strcmp(result,"bounded_variation") == 0)
			{
			        flag = 1;
			}
			else if (strcmp(result,"oscillating") == 0)
			{
			        flag = 2;
			}
			else if (strcmp(result,"reconstruction") == 0)
			{
			        flag = 3;
			}
			else if (strcmp(result,"difference") == 0)
			{
			        flag = 4;
			}
			switch (flag)
			{
			        case 1 :
				        for (i = 0 ; i < storage_size / image->depth ; i++)
					{
						max = my_Max(max, mean + u_index[i]);
						min = my_Min(min, mean + u_index[i]);
						
					}
					for (i = 0 ; return_code && i < storage_size / image->depth ; i++)
					{
						for (k = 0 ; k < image->depth ; k++)
						{
                                        		result_index[k] = (mean + u_index[i] -min)/(max-min);
							/*result_index[k] = (mean*255.0 + u_index[i])/255.0;*/
						}
						result_index += image->depth;
					}
					break;
				case 2 :
				        for (i = 0 ; i < storage_size / image->depth ; i++)
					{
						max = my_Max(max, v_index[i]);
						min = my_Min(min, v_index[i]);
					}
					for (i = 0 ; return_code && i < storage_size / image->depth ; i++)
					{
						for (k = 0 ; k < image->depth ; k++)
						{
                                        		result_index[k] = (v_index[i]-min)/(max-min);
						}
						result_index += image->depth;
					}
					break;
				case 3 :
				        for (i = 0 ; i < storage_size / image->depth ; i++)
					{
						max = my_Max(max, (mean + u_index[i] + v_index[i]));
						min = my_Min(min, (mean + u_index[i] + v_index[i]));
					}
					for (i = 0 ; return_code && i < storage_size / image->depth ; i++)
					{
						for (k = 0 ; k < image->depth ; k++)
						{
                                        		result_index[k] = ((mean + u_index[i] + v_index[i])-min)/(max-min);
						}
						result_index += image->depth;
					}
					break;
				case 4:
				        for (i = 0 ; i < storage_size / image->depth ; i++)
					{
						max = my_Max(max, fabs(*data_index - (mean + u_index[i] + v_index[i])));
						min = my_Min(min, fabs(*data_index - (mean + u_index[i] + v_index[i])));
						data_index += image->depth;
					}
					data_index = (FE_value *)image->data;
					for (i = 0 ; return_code && i < storage_size / image->depth ; i++)
					{
						for (k = 0 ; k < image->depth ; k++)
						{
                                        		result_index[k] = (fabs(*data_index - (mean + u_index[i] + v_index[i]))-min)/(max-min);
						}
						result_index += image->depth;
						data_index += image->depth;
					}
					break;
				default:
				        display_message(ERROR_MESSAGE, "Image_cache_bvc_decomp.  "
			                      "Invalid result name.");
					return_code=0;
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
			
			DEALLOCATE(u_index);
			DEALLOCATE(v_index);
			DEALLOCATE(q1_index);
			DEALLOCATE(q2_index);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Image_cache_bvc_decomp.  Not enough memory.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "Image_cache_bvc_decomp.  "
			"Invalid arguments.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Image_cache_bvc_decomp */


static int Computed_field_bvc_decomp_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;
	struct Computed_field_bvc_decomp_type_specific_data *data;

	ENTER(Computed_field_bvc_decomp_evaluate_cache_at_node);
	if (field && node &&
		(data = (struct Computed_field_bvc_decomp_type_specific_data *)field->type_specific_data))
	{
		return_code = 1;
		/* 1. Precalculate the Image_cache */
		if (!data->image->valid)
		{
			return_code = Image_cache_update_from_fields(data->image, field->source_fields[0],
				field->source_fields[1], data->element_dimension, data->region,
				data->graphics_buffer_package);
			/* 2. Perform image processing operation */
			return_code = Image_cache_bvc_decomp(data->image, data->result, data->number_of_iterations, data->tou, data->lambda, data->mu);
		}
		/* 3. Evaluate texture coordinates and copy image to field */
		Computed_field_evaluate_cache_at_node(field->source_fields[1],
			node, time);
		Image_cache_evaluate_field(data->image,field);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_bvc_decomp_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_bvc_decomp_evaluate_cache_at_node */

static int Computed_field_bvc_decomp_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;
	struct Computed_field_bvc_decomp_type_specific_data *data;

	ENTER(Computed_field_bvc_decomp_evaluate_cache_in_element);
	USE_PARAMETER(calculate_derivatives);
	if (field && element && xi && (field->number_of_source_fields > 0) &&
		(field->number_of_components == field->source_fields[0]->number_of_components) &&
		(data = (struct Computed_field_bvc_decomp_type_specific_data *) field->type_specific_data) &&
		data->image && (field->number_of_components == data->image->depth))
	{
		return_code = 1;
		/* 1. Precalculate the Image_cache */
		if (!data->image->valid)
		{
			return_code = Image_cache_update_from_fields(data->image, field->source_fields[0],
				field->source_fields[1], data->element_dimension, data->region,
				data->graphics_buffer_package);
			/* 2. Perform image processing operation */
			return_code = Image_cache_bvc_decomp(data->image, data->result, data->number_of_iterations, data->tou, data->lambda, data->mu);
		}
		/* 3. Evaluate texture coordinates and copy image to field */
		Computed_field_evaluate_cache_in_element(field->source_fields[1],
			element, xi, time, top_level_element, /*calculate_derivatives*/0);
		Image_cache_evaluate_field(data->image,field);
		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_bvc_decomp_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_bvc_decomp_evaluate_cache_in_element */

#define Computed_field_bvc_decomp_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_bvc_decomp_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_bvc_decomp_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_bvc_decomp_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_bvc_decomp_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_bvc_decomp_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

int Computed_field_bvc_decomp_get_native_resolution(struct Computed_field *field,
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
	struct Computed_field_bvc_decomp_type_specific_data *data;
	
	ENTER(Computed_field_bvc_decomp_get_native_resolution);
	if (field && (data =
		(struct Computed_field_bvc_decomp_type_specific_data *)
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
			"Computed_field_bvc_decomp_get_native_resolution.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_bvc_decomp_get_native_resolution */

static int list_Computed_field_bvc_decomp(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_bvc_decomp_type_specific_data *data;

	ENTER(List_Computed_field_bvc_decomp);
	if (field && (field->type_string==computed_field_bvc_decomp_type_string)
		&& (data = (struct Computed_field_bvc_decomp_type_specific_data *)
		field->type_specific_data))
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,
			"    texture coordinate field : %s\n",field->source_fields[1]->name);
		display_message(INFORMATION_MESSAGE,
			"    result : %s\n", data->result);
		display_message(INFORMATION_MESSAGE,
			"    number_of_iterations : %d\n", data->number_of_iterations);
		display_message(INFORMATION_MESSAGE,
			"    tou : %f\n", data->tou);
		display_message(INFORMATION_MESSAGE,
			"    lambda : %f\n", data->lambda);
		display_message(INFORMATION_MESSAGE,
			"    mu : %f\n", data->mu);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_bvc_decomp.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_bvc_decomp */

static char *Computed_field_bvc_decomp_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	char temp_string[40];
	int error;
	struct Computed_field_bvc_decomp_type_specific_data *data;

	ENTER(Computed_field_bvc_decomp_get_command_string);
	command_string = (char *)NULL;
	if (field && (field->type_string==computed_field_bvc_decomp_type_string)
		&& (data = (struct Computed_field_bvc_decomp_type_specific_data *)
		field->type_specific_data))
	{
		error = 0;
		append_string(&command_string,
			computed_field_bvc_decomp_type_string, &error);
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
		sprintf(temp_string, " dimension %d ", data->image->dimension);
		append_string(&command_string, temp_string, &error);
		
		sprintf(temp_string, " number_of_iterations %d ", data->number_of_iterations);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " result %s ", data->result);
		append_string(&command_string, temp_string, &error);
		
		sprintf(temp_string, " tou %f ", data->tou);
		append_string(&command_string, temp_string, &error);
		
		sprintf(temp_string, " lambda %f ", data->lambda);
		append_string(&command_string, temp_string, &error);
		
		sprintf(temp_string, " mu %f ", data->mu);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " sizes %d %d ",
		                    data->image->sizes[0], data->image->sizes[1]);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " minimums %f %f ",
		                    data->image->minimums[0],data->image->minimums[1]);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " maximums %f %f ",
		                    data->image->maximums[0],data->image->maximums[1]);
		append_string(&command_string, temp_string, &error);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_bvc_decomp_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_bvc_decomp_get_command_string */

#define Computed_field_bvc_decomp_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_bvc_decomp(struct Computed_field *field,
	struct Computed_field *source_field,
	struct Computed_field *texture_coordinate_field,
	int number_of_iterations, int bvc_index, int oc_index,
	int rc_index, int dif_index, double tou, double lambda, double mu,
	int dimension, int *sizes, FE_value *minimums, FE_value *maximums,
	int element_dimension, struct MANAGER(Computed_field) *computed_field_manager,
	struct Cmiss_region *region, struct Graphics_buffer_package *graphics_buffer_package)
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_bvc_decomp with the supplied
fields, <source_field> and <texture_coordinate_field>.  The <dimension> is the
size of the <sizes>, <minimums> and <maximums> vectors and should be less than
or equal to the number of components in the <texture_coordinate_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int depth, number_of_source_fields, return_code;
	struct Computed_field **source_fields;
	struct Computed_field_bvc_decomp_type_specific_data *data;

	ENTER(Computed_field_set_type_bvc_decomp);
	if (field && source_field && texture_coordinate_field &&
		(depth = source_field->number_of_components) &&
		(dimension <= texture_coordinate_field->number_of_components) &&
		region && graphics_buffer_package)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=2;
		data = (struct Computed_field_bvc_decomp_type_specific_data *)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, number_of_source_fields) &&
			ALLOCATE(data, struct Computed_field_bvc_decomp_type_specific_data, 1) &&
			(data->image = ACCESS(Image_cache)(CREATE(Image_cache)())) &&
			Image_cache_update_dimension(
			data->image, dimension, depth, sizes, minimums, maximums) &&
			Image_cache_update_data_storage(data->image))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_bvc_decomp_type_string;
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			source_fields[1]=ACCESS(Computed_field)(texture_coordinate_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			data->number_of_iterations = number_of_iterations;
			if (bvc_index > 0)
			{
			        data->result = "bounded_variation";
			}
			else if (oc_index > 0)
			{
			        data->result = "oscillating";
			}
			else if (rc_index > 0)
			{
			        data->result = "reconstruction";
			}
			else if (dif_index > 0)
			{
			        data->result = "difference";
			}
			data->tou = tou;
			data->lambda = lambda;
			data->mu = mu;
			data->element_dimension = element_dimension;
			data->region = ACCESS(Cmiss_region)(region);
			data->graphics_buffer_package = graphics_buffer_package;
			data->computed_field_manager = computed_field_manager;
			data->computed_field_manager_callback_id =
				MANAGER_REGISTER(Computed_field)(
				Computed_field_bvc_decomp_field_change, (void *)field,
				computed_field_manager);

			field->type_specific_data = data;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(bvc_decomp);
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
			"Computed_field_set_type_bvc_decomp.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_bvc_decomp */

int Computed_field_get_type_bvc_decomp(struct Computed_field *field,
	struct Computed_field **source_field,
	struct Computed_field **texture_coordinate_field,
	int *number_of_iterations, int *bvc_index, int *oc_index,
	int *rc_index, int *dif_index, double *tou, double *lambda, double *mu,
	int *dimension, int **sizes, FE_value **minimums, FE_value **maximums,
	int *element_dimension)
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :
If the field is of type COMPUTED_FIELD_bvc_decomp, the
parameters defining it are returned.
==============================================================================*/
{
	int i, return_code;
	struct Computed_field_bvc_decomp_type_specific_data *data;

	ENTER(Computed_field_get_type_bvc_decomp);
	if (field && (field->type_string==computed_field_bvc_decomp_type_string)
		&& (data = (struct Computed_field_bvc_decomp_type_specific_data *)
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
			*number_of_iterations = data->number_of_iterations;
			*bvc_index = *oc_index = *rc_index = *dif_index = 0;
			if (strcmp(data->result, "bounded_variation") == 0)
			{
				*bvc_index = 1;
			}
			else if (strcmp(data->result, "oscillating") == 0)
			{
				*oc_index = 1;
			}
			else if (strcmp(data->result, "reconstruction") == 0)
			{
				*rc_index = 1;
			}
			else if (strcmp(data->result, "difference") == 0)
			{
				*dif_index = 1;
			}
			*tou = data->tou;
			*lambda = data->lambda;
			*mu = data->mu;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_bvc_decomp.  Unable to allocate vectors.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_bvc_decomp.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_bvc_decomp */

static int define_Computed_field_type_bvc_decomp(struct Parse_state *state,
	void *field_void, void *computed_field_bvc_decomp_package_void)
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_bvc_decomp (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	char bvc_string[] = "bounded_variation", oc_string[] = "oscillating", rc_string[] = "reconstruction"; 
	char dif_string[] = "difference";
	int number_of_iterations;
	double tou, lambda, mu;
	FE_value *minimums, *maximums;
	int dimension, element_dimension, return_code, *sizes;
	struct Computed_field *field, *source_field, *texture_coordinate_field;
	struct Computed_field_bvc_decomp_package
		*computed_field_bvc_decomp_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data,
		set_texture_coordinate_field_data;
        struct Set_names_from_list_data result;
	
	ENTER(define_Computed_field_type_bvc_decomp);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_bvc_decomp_package=
		(struct Computed_field_bvc_decomp_package *)
		computed_field_bvc_decomp_package_void))
	{
		return_code=1;
		source_field = (struct Computed_field *)NULL;
		texture_coordinate_field = (struct Computed_field *)NULL;
		dimension = 0;
		number_of_iterations = 0;
		tou = 0.0;
		lambda = 1.0;
		mu = 0.0;
		sizes = (int *)NULL;
		minimums = (FE_value *)NULL;
		maximums = (FE_value *)NULL;
		element_dimension = 0;
		/* field */
		set_source_field_data.computed_field_manager =
			computed_field_bvc_decomp_package->computed_field_manager;
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		/* texture_coordinate_field */
		set_texture_coordinate_field_data.computed_field_manager =
			computed_field_bvc_decomp_package->computed_field_manager;
		set_texture_coordinate_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_texture_coordinate_field_data.conditional_function_user_data = (void *)NULL;
                /* results, define a list of results */
		result.number_of_tokens = 4;
		ALLOCATE(result.tokens, struct Set_names_from_list_token, 4);
		result.tokens[0].string = bvc_string;
		result.tokens[0].index = 0;
		result.tokens[1].string = oc_string;
		result.tokens[1].index = 0;
		result.tokens[2].string = rc_string;
		result.tokens[2].index = 0;
		result.tokens[3].string = dif_string;
		result.tokens[3].index = 0;
		
		if (computed_field_bvc_decomp_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_bvc_decomp(field,
				&source_field, &texture_coordinate_field,
				&number_of_iterations, &result.tokens[0].index, &result.tokens[1].index,
				&result.tokens[2].index, &result.tokens[3].index, &tou, &lambda, &mu, 
				&dimension, &sizes, &minimums, &maximums, &element_dimension);
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
				/* number_of_iterations */
				Option_table_add_int_positive_entry(option_table, "number_of_iterations",
					&number_of_iterations);
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
				/* result */
				Option_table_add_set_names_from_list_entry(option_table,
					"result", &result);
				/* sizes */
				Option_table_add_int_vector_entry(option_table,
					"sizes", sizes, &dimension);
				/* tou */
				Option_table_add_double_entry(option_table,
					"tou", &tou);
				/* lambda */
				Option_table_add_double_entry(option_table,
					"lambda", &lambda);
				/* mu */
				Option_table_add_double_entry(option_table,
					"mu", &mu);
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
				/* number_of_iterations */
				Option_table_add_int_positive_entry(option_table, "number_of_iterations",
					&number_of_iterations);
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
				/* result */
				Option_table_add_set_names_from_list_entry(option_table,
					"result", &result);
				/* sizes */
				Option_table_add_int_vector_entry(option_table,
					"sizes", sizes, &dimension);
				/* tou */
				Option_table_add_double_entry(option_table,
					"tou", &tou);
				/* lambda */
				Option_table_add_double_entry(option_table,
					"lambda", &lambda);
				/* mu */
				Option_table_add_double_entry(option_table,
					"mu", &mu);
				/* texture_coordinate_field */
				Option_table_add_Computed_field_conditional_entry(option_table,
					"texture_coordinate_field", &texture_coordinate_field,
					&set_texture_coordinate_field_data);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			if ((dimension < 1) && source_field)
			{
			        return_code = Computed_field_get_native_resolution(source_field,
				     &dimension,&sizes,&minimums,&maximums,&texture_coordinate_field);
			}
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_bvc_decomp(field,
					source_field, texture_coordinate_field, 
					number_of_iterations, result.tokens[0].index, result.tokens[1].index,
					result.tokens[2].index, result.tokens[3].index, tou, lambda, mu, 
					dimension, sizes, minimums, maximums, element_dimension,
					computed_field_bvc_decomp_package->computed_field_manager,
					computed_field_bvc_decomp_package->root_region,
					computed_field_bvc_decomp_package->graphics_buffer_package);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_bvc_decomp.  Failed");
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
		DEALLOCATE(result.tokens);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_bvc_decomp.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_bvc_decomp */

int Computed_field_register_types_bvc_decomp(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package)
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_bvc_decomp_package
		computed_field_bvc_decomp_package;

	ENTER(Computed_field_register_types_bvc_decomp);
	if (computed_field_package)
	{
		computed_field_bvc_decomp_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		computed_field_bvc_decomp_package.root_region = root_region;
		computed_field_bvc_decomp_package.graphics_buffer_package = graphics_buffer_package;
		return_code = Computed_field_package_add_type(computed_field_package,
			            computed_field_bvc_decomp_type_string,
			            define_Computed_field_type_bvc_decomp,
			            &computed_field_bvc_decomp_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_bvc_decomp.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_bvc_decomp */

