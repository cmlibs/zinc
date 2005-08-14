/******************************************************************
  FILE: computed_field_chan_contour.c

  LAST MODIFIED: 15 August 2005

  DESCRIPTION:Implement region  segmentation
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
#include "image_processing/computed_field_chan_contour.h"

#define my_Min(x,y) ((x) <= (y) ? (x) : (y))
#define my_Max(x,y) ((x) <= (y) ? (y) : (x))

struct Computed_field_chan_contour_package
/*******************************************************************************
LAST MODIFIED : 12 August 2005

DESCRIPTION :
A container for objects required to define fields in this module.
==============================================================================*/
{
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Cmiss_region *root_region;
	struct Graphics_buffer_package *graphics_buffer_package;
};


struct Computed_field_chan_contour_type_specific_data
{
	int iteration_times;
	float cached_time;
	int element_dimension;
	struct Cmiss_region *region;
	struct Graphics_buffer_package *graphics_buffer_package;
	struct Image_cache *image;
	struct MANAGER(Computed_field) *computed_field_manager;
	void *computed_field_manager_callback_id;
};

static char computed_field_chan_contour_type_string[] = "chan_contour";

int Computed_field_is_type_chan_contour(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_chan_contour);
	if (field)
	{
		return_code =
		  (field->type_string == computed_field_chan_contour_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_chan_contour.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_chan_contour */

static void Computed_field_chan_contour_field_change(
	struct MANAGER_MESSAGE(Computed_field) *message, void *field_void)
/*******************************************************************************
LAST MODIFIED : 5 December 2003

DESCRIPTION :
Manager function called back when either of the source fields change so that
we know to invalidate the image cache.
==============================================================================*/
{
	struct Computed_field *field;
	struct Computed_field_chan_contour_type_specific_data *data;

	ENTER(Computed_field_chan_contour_source_field_change);
	if (message && (field = (struct Computed_field *)field_void) && (data =
		(struct Computed_field_chan_contour_type_specific_data *)
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
			"Computed_field_chan_contour_source_field_change.  "
			"Invalid arguments.");
	}
	LEAVE;
} /* Computed_field_chan_contour_source_field_change */

static int Computed_field_chan_contour_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_chan_contour_type_specific_data *data;

	ENTER(Computed_field_chan_contour_clear_type_specific);
	if (field && (data =
		(struct Computed_field_chan_contour_type_specific_data *)
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
			"Computed_field_chan_contour_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_chan_contour_clear_type_specific */

static void *Computed_field_chan_contour_copy_type_specific(
	struct Computed_field *source_field, struct Computed_field *destination_field)
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_chan_contour_type_specific_data *destination,
		*source;

	ENTER(Computed_field_chan_contour_copy_type_specific);
	if (source_field && destination_field && (source =
		(struct Computed_field_chan_contour_type_specific_data *)
		source_field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_chan_contour_type_specific_data, 1))
		{
			destination->iteration_times = source->iteration_times;
			destination->cached_time = source->cached_time;
			destination->region = ACCESS(Cmiss_region)(source->region);
			destination->element_dimension = source->element_dimension;
			destination->graphics_buffer_package = source->graphics_buffer_package;
			destination->computed_field_manager = source->computed_field_manager;
			destination->computed_field_manager_callback_id =
				MANAGER_REGISTER(Computed_field)(
				Computed_field_chan_contour_field_change, (void *)destination_field,
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
				"Computed_field_chan_contour_copy_type_specific.  "
				"Unable to allocate memory.");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_chan_contour_copy_type_specific.  "
			"Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_chan_contour_copy_type_specific */

int Computed_field_chan_contour_clear_cache_type_specific
   (struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_chan_contour_type_specific_data *data;

	ENTER(Computed_field_chan_contour_clear_type_specific);
	if (field && (data =
		(struct Computed_field_chan_contour_type_specific_data *)
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
			"Computed_field_chan_contour_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_chan_contour_clear_type_specific */

static int Computed_field_chan_contour_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;
	struct Computed_field_chan_contour_type_specific_data *data,
		*other_data;

	ENTER(Computed_field_chan_contour_type_specific_contents_match);
	if (field && other_computed_field && (data =
		(struct Computed_field_chan_contour_type_specific_data *)
		field->type_specific_data) && (other_data =
		(struct Computed_field_chan_contour_type_specific_data *)
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
} /* Computed_field_chan_contour_type_specific_contents_match */

#define Computed_field_chan_contour_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_chan_contour_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_chan_contour_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_chan_contour_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
No special criteria.
==============================================================================*/

FE_value delta_h(FE_value x)
{       
        FE_value y;
	y = 1.0/(M_PI*(1.0 + x * x));
	return (y);
}

void delta_x_p(FE_value *data_index, FE_value *dxp, int x_size, int y_size)
{
        int x, y;
	FE_value d_x;
	for (y = 0; y < y_size; y++)
	{
		for(x =0 ; x < x_size; x++)
		{
			if (x == x_size -1)
			{
				d_x = 0.0;
			}
			else
			{
				d_x = *(data_index + y*x_size + x+1) - *(data_index + y*x_size + x);
			} 
			
			dxp[y*x_size + x] = d_x;
		}
	} 	
}/* delta_x_p */

void delta_x_n(FE_value *data_index, FE_value *dxn, int x_size, int y_size)
{
        int x, y;
	FE_value d_x;
	for (y = 0; y < y_size; y++)
	{
		for(x =0 ; x < x_size; x++)
		{
			if (x == 0)
			{
				d_x = 0.0;
			}
			else
			{
				d_x = *(data_index + y*x_size + x) - *(data_index + y*x_size + x -1);
			} 
			
			dxn[y*x_size + x] = d_x;
		}
	} 	
}/*delta_x_n*/

void delta_y_p(FE_value *data_index, FE_value *dyp, int x_size, int y_size)
{
        int x, y;
	FE_value d_y;
	for (y = 0; y < y_size; y++)
	{
		for(x =0 ; x < x_size; x++)
		{
			if (y == y_size -1)
			{
				d_y = 0.0;
			}
			else
			{
				d_y = *(data_index + (y + 1)*x_size + x) - *(data_index + y*x_size + x);
			}
			dyp[y*x_size + x] = d_y;
		}
	} 	
}/*delta_y_p*/

void delta_y_n(FE_value *data_index, FE_value *dyn, int x_size, int y_size)
{
        int x, y;
	FE_value d_y;
	for (y = 0; y < y_size; y++)
	{
		for(x =0 ; x < x_size; x++)
		{
			if (y == 0)
			{
				d_y = 0.0;
			}
			else
			{
				d_y = *(data_index + y*x_size + x) - *(data_index + (y-1)*x_size + x);
			}
			dyn[y*x_size + x] = d_y;
		}
	} 	
}/* delta_y_n */

void delta_y_pn(FE_value *data_index, FE_value *dypn, int x_size, int y_size)
{
        int x, y;
	FE_value d_y;
	for (y = 0; y < y_size; y++)
	{
		for(x =0 ; x < x_size; x++)
		{
			if (y == y_size -1)
			{
				d_y = 0.0;
			}
			else if (y == 0)
			{
			        d_y = 0.0;
			}
			else
			{
				d_y = *(data_index + (y + 1)*x_size + x) - *(data_index + (y-1)*x_size + x);
			}
			dypn[y*x_size + x] = d_y;
		}
	} 	
}/*delta_y_pn*/

void delta_x_pn(FE_value *data_index, FE_value *dxpn, int x_size, int y_size)
{
        int x, y;
	FE_value d_x;
	for (y = 0; y < y_size; y++)
	{
		for(x =0 ; x < x_size; x++)
		{
			if (x == 0)
			{
				d_x = 0.0;
			}
			else if (x == (x_size -1))
			{
			        d_x = 0.0;
			}
			else
			{
				d_x = *(data_index + y*x_size + x + 1) - *(data_index + y*x_size + x-1);
			}
			dxpn[y*x_size + x] = d_x;
		}
	} 	
}/* delta_y_n */


FE_value mean1(FE_value *data_index, FE_value *phi, int x_size, int y_size)
{
        FE_value h, mean;
	int i, x, y;
	h = 0.0;
	mean = 0.0;
	
	for (y = 0; y < y_size; y++)
	{
		for(x = 0 ; x < x_size; x++)
		{
		        i = y*x_size + x;
			h += data_index[i] * 0.5*(1.0 + 2.0 * atan(phi[i])/M_PI);
			mean += 0.5*(1.0 + 2.0 * atan(phi[i])/M_PI);
		}
	} 
	
	h /= mean;
	return (h);
}/* c1 */

FE_value mean2(FE_value *data_index, FE_value *phi, int x_size, int y_size)
{
        FE_value h, mean;
	int i, x, y;
	h = 0.0;
	mean = 0.0;
	
	for (y = 0; y < y_size; y++)
	{
		for(x = 0 ; x < x_size; x++)
		{
		        i = y*x_size + x;
			h += data_index[i] * (1.0-0.5*(1.0 + 2.0 * atan(phi[i])/M_PI));
			mean += (1.0-0.5*(1.0 + 2.0 * atan(phi[i])/M_PI));
		}
	} 
	
	h /= mean;
	return (h);
}/* c2 */


static int Image_cache_chan_contour(struct Image_cache *image, int iteration_times)
/*******************************************************************************
LAST MODIFIED : 15 August 2005

DESCRIPTION :
Perform region segmentation on the image cache.
==============================================================================*/
{
	char *storage;
	FE_value *data_index, *result_index;
	FE_value *phi, *phi1;
	FE_value *data1;
	FE_value *dxi, *dyi, *dxp, *dyp, *dxpn, *dypn;
	FE_value *temp1, *temp2, d_t, mu;
	FE_value c1, c2, radius, c_x, c_y, x_f, y_f, fm1, fm2;
	
	int return_code, size, storage_size;
	int ps, i, j, k, x, y;

	ENTER(Image_cache_chan_contour);
	if (image && (image->dimension > 0) && (image->depth > 0))
	{
		return_code = 1;
		
		/* Allocate a new storage block for our data */
		storage_size = image->depth;
		size = 1;
		for (i = 0 ; i < image->dimension ; i++)
		{
			storage_size *= image->sizes[i];
			size *= image->sizes[i];
		}
		if (ALLOCATE(storage, char, storage_size * sizeof(FE_value)) &&
			  ALLOCATE(data1, FE_value, size) &&
			  ALLOCATE(dxi, FE_value, size) &&
			  ALLOCATE(dyi, FE_value, size) &&
			  ALLOCATE(dxp, FE_value, size) &&
			  ALLOCATE(dyp, FE_value, size) &&
			  ALLOCATE(dxpn, FE_value, size) &&
			  ALLOCATE(dypn, FE_value, size) &&
			  ALLOCATE(temp1, FE_value, size) &&
			  ALLOCATE(temp2, FE_value, size) &&
			  ALLOCATE(phi, FE_value, size) &&
			  ALLOCATE(phi1, FE_value, size))
		{
		        return_code = 1;
			d_t = 0.1;
			mu = 0.01*255.0*255.0;
			
			radius = (FE_value)my_Min(image->sizes[0], image->sizes[1])/2.0 -5.0;
			c_x = (FE_value)image->sizes[0]/2.0;
			c_y = (FE_value)image->sizes[1]/2.0;
			result_index = (FE_value *)storage;
			for (i = 0 ; i < storage_size ; i++)
			{
				*result_index = 0.0;
				result_index++;
			}
			data_index = (FE_value *)image->data;
			for (i = 0 ; i < size ; i++)
			{
				data1[i] = (*data_index)*255.0;
				data_index += image->depth;
			}
			data_index = (FE_value *)image->data;
			result_index = (FE_value *)storage;
			for (y = 0; y < image->sizes[1]; y++)
			{
			        for (x = 0; x < image->sizes[0]; x++)
				{
				        ps = y*image->sizes[0] + x;
					x_f = (FE_value)x;
					y_f = (FE_value)y;
					phi[ps] = radius - sqrt((x_f-c_x)*(x_f-c_x) + (y_f-c_y)*(y_f-c_y));
				}
			}
			for (i = 0; i < iteration_times; i++)
			{
			        delta_x_p(phi,dxp,image->sizes[0], image->sizes[1]);
				delta_x_pn(phi,dxpn,image->sizes[0], image->sizes[1]);
				delta_y_p(phi,dyp,image->sizes[0], image->sizes[1]);
				delta_y_pn(phi,dypn,image->sizes[0], image->sizes[1]);
				for (j = 0; j < size; j++)
				{
				        fm1 = sqrt(dxp[j]*dxp[j]+dxpn[j]*dxpn[j]/4.0);
					fm2 = sqrt(dyp[j]*dyp[j]+dypn[j]*dypn[j]/4.0);
					if (fm1 == 0.0)
					{
					        temp1[j] = 0.0;
					}
					else
					{
					        temp1[j] = dxp[j]/fm1;
					}
					if (fm2 == 0.0)
					{
					        temp2[j] = 0.0;
					}
					else
					{
					        temp2[j] = dyp[j]/fm2;
					}
				}
				delta_x_n(temp1,dxi,image->sizes[0], image->sizes[1]);
				delta_y_n(temp2,dyi,image->sizes[0], image->sizes[1]);
				c1 = mean1(data1,phi,image->sizes[0], image->sizes[1]);
				c2 = mean2(data1,phi,image->sizes[0], image->sizes[1]);
				for (j = 0; j < size; j++)
				{
				        phi1[j] = phi[j] + d_t * delta_h(phi[j]) *
					          (mu*dxi[j]+mu*dyi[j]-(data1[j]-c1)*(data1[j]-c1)+(data1[j]-c2)*(data1[j]-c2));
				}
				for (j = 0; j < size; j++)
				{
				        phi[j] = phi1[j];
				}
			}
			for (i = 0; i < size; i++)
			{
			        if (phi[i] >= 0.0)
				{
			                for (k = 0; k < image->depth; k++)
				        {
				                 result_index[k] = 0.0;
				        }
				}
				else
				{
				        for (k = 0; k < image->depth; k++)
				        {
				                 result_index[k] = 0.7;
				        }
				}
				result_index += image->depth;
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

			DEALLOCATE(dxi);
			DEALLOCATE(dyi);
			DEALLOCATE(temp1);
			DEALLOCATE(temp2);
			DEALLOCATE(data1);
			DEALLOCATE(dxp);
			DEALLOCATE(dyp);
			DEALLOCATE(dxpn);
			DEALLOCATE(dypn);
			DEALLOCATE(phi);
			DEALLOCATE(phi1);

		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Image_cache_chan_contour.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "Image_cache_chan_contour.  "
			"Invalid arguments.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Image_cache_chan_contour */

static int Computed_field_chan_contour_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;
	struct Computed_field_chan_contour_type_specific_data *data;
	
	ENTER(Computed_field_chan_contour_evaluate_cache_at_node);
	if (field && node &&
		(data = (struct Computed_field_chan_contour_type_specific_data *)field->type_specific_data))
	{
		return_code = 1;
		/* 1. Precalculate the Image_cache */
		if (!data->image->valid)
		{
			return_code = Image_cache_update_from_fields(data->image, field->source_fields[0],
				field->source_fields[1], data->element_dimension, data->region,
				data->graphics_buffer_package);
			/* 2. Perform image processing operation */
			
			return_code = Image_cache_chan_contour(data->image, data->iteration_times);
		}
		/* 3. Evaluate texture coordinates and copy image to field */
		Computed_field_evaluate_cache_at_node(field->source_fields[1],
			node, time);
		Image_cache_evaluate_field(data->image,field);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_chan_contour_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_chan_contour_evaluate_cache_at_node */

static int Computed_field_chan_contour_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;
	struct Computed_field_chan_contour_type_specific_data *data;
       
	ENTER(Computed_field_chan_contour_evaluate_cache_in_element);
	USE_PARAMETER(calculate_derivatives);
	if (field && element && xi && (field->number_of_source_fields > 0) &&
		(field->number_of_components == field->source_fields[0]->number_of_components) &&
		(data = (struct Computed_field_chan_contour_type_specific_data *) field->type_specific_data) &&
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
			return_code = Image_cache_chan_contour(data->image, data->iteration_times);
		}
		/* 3. Evaluate texture coordinates and copy image to field */
		Computed_field_evaluate_cache_in_element(field->source_fields[1],
			element, xi, time, top_level_element, /*calculate_derivatives*/0);
		Image_cache_evaluate_field(data->image,field);
		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_chan_contour_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_chan_contour_evaluate_cache_in_element */

#define Computed_field_chan_contour_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_chan_contour_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_chan_contour_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_chan_contour_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_chan_contour_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_chan_contour_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

int Computed_field_chan_contour_get_native_resolution(struct Computed_field *field,
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
	struct Computed_field_chan_contour_type_specific_data *data;
	
	ENTER(Computed_field_chan_contour_get_native_resolution);
	if (field && (data =
		(struct Computed_field_chan_contour_type_specific_data *)
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
			"Computed_field_chan_contour_get_native_resolution.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_chan_contour_get_native_resolution */

static int list_Computed_field_chan_contour(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_chan_contour_type_specific_data *data;
        
	ENTER(List_Computed_field_chan_contour);
	if (field && (field->type_string==computed_field_chan_contour_type_string)
		&& (data = (struct Computed_field_chan_contour_type_specific_data *)
		field->type_specific_data))
	{        
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,
			"    texture coordinate field : %s\n",field->source_fields[1]->name);
		display_message(INFORMATION_MESSAGE,
			"    filter iteration_times : %d\n", data->iteration_times);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_chan_contour.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_chan_contour */

static char *Computed_field_chan_contour_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error;
	struct Computed_field_chan_contour_type_specific_data *data;
        
	ENTER(Computed_field_chan_contour_get_command_string);
	command_string = (char *)NULL;
	if (field&& (field->type_string==computed_field_chan_contour_type_string)
		&& (data = (struct Computed_field_chan_contour_type_specific_data *)
		field->type_specific_data) )
	{
		error = 0;
		append_string(&command_string,
			computed_field_chan_contour_type_string, &error);
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
		sprintf(temp_string, " dimension %d", data->image->dimension);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " iteration_times %d", data->iteration_times);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " sizes %d %d",
		                    data->image->sizes[0],data->image->sizes[1]);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " minimums %f %f",
		                    data->image->minimums[0], data->image->minimums[1]);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " maximums %f %f",
		                    data->image->maximums[0], data->image->maximums[1]);
		append_string(&command_string, temp_string, &error);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_chan_contour_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_chan_contour_get_command_string */

#define Computed_field_chan_contour_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_chan_contour(struct Computed_field *field,
	struct Computed_field *source_field,
	struct Computed_field *texture_coordinate_field,
	int iteration_times,
	int dimension, int *sizes, FE_value *minimums, FE_value *maximums,
	int element_dimension, struct MANAGER(Computed_field) *computed_field_manager,
	struct Cmiss_region *region, struct Graphics_buffer_package *graphics_buffer_package)
/*******************************************************************************
LAST MODIFIED : 1filter_size December 2003

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_chan_contour with the supplied
fields, <source_field> and <texture_coordinate_field>.  The <iteration_times> specifies
half the width and height of the filter window.  The <dimension> is the
size of the <sizes>, <minimums> and <maximums> vectors and should be less than
or equal to the number of components in the <texture_coordinate_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int depth, number_of_source_fields, return_code;
	struct Computed_field **source_fields;
	struct Computed_field_chan_contour_type_specific_data *data;
        
	ENTER(Computed_field_set_type_chan_contour);
	if (field && source_field && texture_coordinate_field &&
		(iteration_times > 0) && (depth = source_field->number_of_components) &&
		(dimension <= texture_coordinate_field->number_of_components) &&
		region && graphics_buffer_package)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=2;
		
		data = (struct Computed_field_chan_contour_type_specific_data *)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, number_of_source_fields) &&
			ALLOCATE(data, struct Computed_field_chan_contour_type_specific_data, 1) &&
			(data->image = ACCESS(Image_cache)(CREATE(Image_cache)())) &&
			Image_cache_update_dimension(
			data->image, dimension, depth, sizes, minimums, maximums) &&
			Image_cache_update_data_storage(data->image))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_chan_contour_type_string;
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			source_fields[1]=ACCESS(Computed_field)(texture_coordinate_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			data->iteration_times = iteration_times;
			data->element_dimension = element_dimension;
			data->region = ACCESS(Cmiss_region)(region);
			data->graphics_buffer_package = graphics_buffer_package;
			data->computed_field_manager = computed_field_manager;
			data->computed_field_manager_callback_id =
				MANAGER_REGISTER(Computed_field)(
				Computed_field_chan_contour_field_change, (void *)field,
				computed_field_manager);
			
			field->type_specific_data = data;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(chan_contour);
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
			"Computed_field_set_type_chan_contour.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_chan_contour */

int Computed_field_get_type_chan_contour(struct Computed_field *field,
	struct Computed_field **source_field,
	struct Computed_field **texture_coordinate_field,
	int *iteration_times, int *dimension, int **sizes, FE_value **minimums,
	FE_value **maximums, int *element_dimension)
/*******************************************************************************
LAST MODIFIED : 1filter_size December 2003

DESCRIPTION :
If the field is of type COMPUTED_FIELD_chan_contour, the
parameters defining it are returned.
==============================================================================*/
{
	int i, return_code;
	struct Computed_field_chan_contour_type_specific_data *data;
        
	ENTER(Computed_field_get_type_chan_contour);
	if (field && (field->type_string==computed_field_chan_contour_type_string)
		&& (data = (struct Computed_field_chan_contour_type_specific_data *)
		field->type_specific_data) && data->image)
	{
		*dimension = data->image->dimension;
		if (ALLOCATE(*sizes, int, *dimension)
			&& ALLOCATE(*minimums, FE_value, *dimension)
			&& ALLOCATE(*maximums, FE_value, *dimension))
		{
			*source_field = field->source_fields[0];
			*texture_coordinate_field = field->source_fields[1];
			*iteration_times = data->iteration_times;
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
				"Computed_field_get_type_chan_contour.  Unable to allocate vectors.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_chan_contour.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_chan_contour */

static int define_Computed_field_type_chan_contour(struct Parse_state *state,
	void *field_void, void *computed_field_chan_contour_package_void)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_chan_contour (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	FE_value *minimums, *maximums;
	int dimension, element_dimension, iteration_times, return_code, *sizes;
	struct Computed_field *field, *source_field, *texture_coordinate_field;
	struct Computed_field_chan_contour_package
		*computed_field_chan_contour_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data,
		set_texture_coordinate_field_data;

	ENTER(define_Computed_field_type_chan_contour);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_chan_contour_package=
		(struct Computed_field_chan_contour_package *)
		computed_field_chan_contour_package_void))
	{
		return_code=1;
		source_field = (struct Computed_field *)NULL;
		texture_coordinate_field = (struct Computed_field *)NULL;
		dimension = 0;
		sizes = (int *)NULL;
		minimums = (FE_value *)NULL;
		maximums = (FE_value *)NULL;
		element_dimension = 0;
		iteration_times = 0;
		/* field */
		set_source_field_data.computed_field_manager =
			computed_field_chan_contour_package->computed_field_manager;
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		/* texture_coordinate_field */
		set_texture_coordinate_field_data.computed_field_manager =
			computed_field_chan_contour_package->computed_field_manager;
		set_texture_coordinate_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_texture_coordinate_field_data.conditional_function_user_data = (void *)NULL;

		if (computed_field_chan_contour_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_chan_contour(field,
				&source_field, &texture_coordinate_field, &iteration_times,
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
				/* iteration_times */
				Option_table_add_int_positive_entry(option_table,
					"iteration_times", &iteration_times);
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
			/*if (return_code && (dimension < 1))
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_scale.  Must specify a dimension first.");
				return_code = 0;
			} */
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
				/* iteration_times */
				Option_table_add_int_positive_entry(option_table,
					"iteration_times", &iteration_times);
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
			if ((dimension < 1) && source_field)
			{
			        return_code = Computed_field_get_native_resolution(source_field,
				     &dimension,&sizes,&minimums,&maximums,&texture_coordinate_field);
			}
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_chan_contour(field,
					source_field, texture_coordinate_field, iteration_times, dimension,
					sizes, minimums, maximums, element_dimension,
					computed_field_chan_contour_package->computed_field_manager,
					computed_field_chan_contour_package->root_region,
					computed_field_chan_contour_package->graphics_buffer_package);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_chan_contour.  Failed");
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
			"define_Computed_field_type_chan_contour.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_chan_contour */

int Computed_field_register_types_chan_contour(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package)
/*******************************************************************************
LAST MODIFIED : 12 December 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_chan_contour_package
		computed_field_chan_contour_package;

	ENTER(Computed_field_register_types_chan_contour);
	if (computed_field_package)
	{
		computed_field_chan_contour_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		computed_field_chan_contour_package.root_region = root_region;
		computed_field_chan_contour_package.graphics_buffer_package = graphics_buffer_package;
		return_code = Computed_field_package_add_type(computed_field_package,
			            computed_field_chan_contour_type_string,
			            define_Computed_field_type_chan_contour,
			            &computed_field_chan_contour_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_chan_contour.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_chan_contour */

