/************************************************************************************
   FILE: computed_field_minimal_path.c

   LAST MODIFIED: 12 July 2004

   DESCRIPTION: Perform image resample on Computed field using <nearest> and <bicubic>
   methods.
===================================================================================*/
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
#include "image_processing/computed_field_minimal_path.h"

#define my_Min(x,y) ((x) <= (y) ? (x) : (y))
#define my_Max(x,y) ((x) <= (y) ? (y) : (x))
#define Infinite (1 << 14)

struct Computed_field_minimal_path_package
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

struct Computed_field_minimal_path_type_specific_data
{
	int dimension;
	double iteration_step;
	int *start_position;
	int *end_position;
	float cached_time;
	int element_dimension;
	struct Cmiss_region *region;
	struct Graphics_buffer_package *graphics_buffer_package;
	struct Image_cache *image;
	struct MANAGER(Computed_field) *computed_field_manager;
	void *computed_field_manager_callback_id;
};

static char computed_field_minimal_path_type_string[] = "minimal_path";

int Computed_field_is_type_minimal_path(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_minimal_path);
	if (field)
	{
		return_code =
		  (field->type_string == computed_field_minimal_path_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_minimal_path.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_minimal_path */

static void Computed_field_minimal_path_field_change(
	struct MANAGER_MESSAGE(Computed_field) *message, void *field_void)
/*******************************************************************************
LAST MODIFIED : 5 April 2004

DESCRIPTION :
Manager function called back when either of the source fields change so that
we know to invalidate the image cache.
==============================================================================*/
{
	struct Computed_field *field;
	struct Computed_field_minimal_path_type_specific_data *data;

	ENTER(Computed_field_minimal_path_source_field_change);
	if (message && (field = (struct Computed_field *)field_void) && (data =
		(struct Computed_field_minimal_path_type_specific_data *)
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
			"Computed_field_minimal_path_source_field_change.  "
			"Invalid arguments.");
	}
	LEAVE;
} /* Computed_field_minimal_path_source_field_change */

static int Computed_field_minimal_path_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_minimal_path_type_specific_data *data;

	ENTER(Computed_field_minimal_path_clear_type_specific);
	if (field && (data =
		(struct Computed_field_minimal_path_type_specific_data *)
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
		if (data->start_position)
		{
			DEALLOCATE(data->start_position);
		}
		if (data->end_position)
		{
			DEALLOCATE(data->end_position);
		}
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_minimal_path_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_minimal_path_clear_type_specific */

static void *Computed_field_minimal_path_copy_type_specific(
	struct Computed_field *source_field, struct Computed_field *destination_field)
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_minimal_path_type_specific_data *destination,
		*source;
	int i;

	ENTER(Computed_field_minimal_path_copy_type_specific);
	if (source_field && destination_field && (source =
		(struct Computed_field_minimal_path_type_specific_data *)
		source_field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_minimal_path_type_specific_data, 1) &&
			ALLOCATE(destination->start_position, int, source->dimension) &&
			ALLOCATE(destination->end_position, int, source->dimension))
		{
			destination->dimension = source->dimension;
			destination->iteration_step = source->iteration_step;
			for (i = 0; i < source->dimension; i++)
			{
				destination->start_position[i] = source->start_position[i];
				destination->end_position[i] = source->end_position[i];
			}
			destination->cached_time = source->cached_time;
			destination->region = ACCESS(Cmiss_region)(source->region);
			destination->element_dimension = source->element_dimension;
			destination->graphics_buffer_package = source->graphics_buffer_package;
			destination->computed_field_manager = source->computed_field_manager;
			destination->computed_field_manager_callback_id =
				MANAGER_REGISTER(Computed_field)(
				Computed_field_minimal_path_field_change, (void *)destination_field,
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
				"Computed_field_minimal_path_copy_type_specific.  "
				"Unable to allocate memory.");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_minimal_path_copy_type_specific.  "
			"Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_minimal_path_copy_type_specific */

int Computed_field_minimal_path_clear_cache_type_specific
   (struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_minimal_path_type_specific_data *data;

	ENTER(Computed_field_minimal_path_clear_type_specific);
	if (field && (data =
		(struct Computed_field_minimal_path_type_specific_data *)
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
			"Computed_field_minimal_path_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_minimal_path_clear_type_specific */

static int Computed_field_minimal_path_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;
	struct Computed_field_minimal_path_type_specific_data *data,
		*other_data;

	ENTER(Computed_field_minimal_path_type_specific_contents_match);
	if (field && other_computed_field && (data =
		(struct Computed_field_minimal_path_type_specific_data *)
		field->type_specific_data) && (other_data =
		(struct Computed_field_minimal_path_type_specific_data *)
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
} /* Computed_field_minimal_path_type_specific_contents_match */

#define Computed_field_minimal_path_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_minimal_path_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_minimal_path_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_minimal_path_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 4 May 2005

DESCRIPTION :
No special criteria.
==============================================================================*/


static int Image_cache_minimal_action(FE_value *potential_data, 
         int dimension, int *sizes, int start_posi)
/*******************************************************************************
LAST MODIFIED : 4 May 2005

DESCRIPTION :

==============================================================================*/
{
	FE_value *storage;
	FE_value *data_index, *result_index;
	int  return_code, kernel_size, storage_size;
	int i, *offsets, trial_num, *dir;
	int current, new_start_position;
	FE_value umin;
	
	ENTER(Image_cache_minimal_action);
	if (potential_data && (dimension >0))
	{
	        return_code = 1;
		kernel_size = 1;
		storage_size = 1;
		for (i = 0 ; i < dimension ; i++)
		{
			kernel_size *= 3;
			storage_size *= sizes[i];
		}
		if (ALLOCATE(offsets, int, kernel_size) &&
		        ALLOCATE(storage, FE_value, 4*storage_size) &&
			ALLOCATE(dir, int, 6))
		{
                        return_code = 1;
			trial_num = 0;
			new_start_position = start_posi;
			data_index = (FE_value *)potential_data;
			result_index = (FE_value *)storage;
			for (i = 0; i < storage_size; i++)
			{
			        result_index[0] = Infinite;/* initial potential*/
				result_index[1] = 1.0; /*label as far */
				result_index[2] = 0.0; /* label as non trial */
				result_index[3] = 0.0; /* label as non alive */
				if (i == new_start_position)
				{
				       result_index[0] = 0.0;
				       result_index[2] = 1.0; /* label as trial */
				}
				result_index += 4;
			}
			trial_num++;
			result_index = (FE_value *)storage;
			Filter_offsets(offsets, dimension, 1, sizes, 4);
			if (dimension == 2)
			{
				FE_value u, v, u1, v1, s, delta;
				dir[0] = offsets[1];
				dir[1] = offsets[7];
				dir[2] = offsets[3];
				dir[3] = offsets[5];
				
				result_index[new_start_position * 4 + 2] = 0.0;
				result_index[new_start_position * 4 + 3] = 1.0;
				
				trial_num--;
				for (i = 0; i < 4; i++)
				{
				/* the point corresponding to direction dir[i] */
				if ((result_index+new_start_position *4 + dir[i]>=(FE_value *)storage)&&
				          (result_index+new_start_position *4 + dir[i] < 
					  (FE_value *)storage + storage_size * 4))
				{
				        if (result_index[new_start_position *4 + dir[i] + 1] == Infinite)
					{
					        result_index[new_start_position *4 + dir[i] + 2] = 1.0;
						trial_num++;  
					}
					
					if (result_index[new_start_position *4 + dir[i] + 3] == 0.0)
					{
					        current = new_start_position * 4 + dir[i];
						if ((result_index + current + offsets[1]>=(FE_value *)storage) &&
				                    (result_index + current + offsets[1] < 
						    (FE_value *)storage + storage_size * 4))
						{
						        u = result_index[current + offsets[1]];
						}
						else
						{
						        u = Infinite;
						}
						if ((result_index + current + offsets[7]>=(FE_value *)storage) &&
				                    (result_index + current + offsets[7] < 
						    (FE_value *)storage + storage_size * 4))
						{
						        u = my_Min(u,result_index[current + offsets[7]]);
						}
						else
						{
						        u = my_Min(u,Infinite);
						}
						if ((result_index + current + offsets[3]>=(FE_value *)storage) &&
				                    (result_index + current + offsets[3] < 
						    (FE_value *)storage + storage_size * 4))
						{
						        v = result_index[current + offsets[3]];
						}
						else
						{
						        v = Infinite;
						}
						if ((result_index + current + offsets[5]>=(FE_value *)storage) &&
				                    (result_index + current + offsets[5] < 
						    (FE_value *)storage + storage_size * 4))
						{
						        v = my_Min(v,result_index[current + offsets[5]]);
						}
						else
						{
						        v = my_Min(v,Infinite);
						}
						u1 = my_Min (u,v);
						v1 = my_Max (u,v);
						delta = 2.0 * *(data_index + current / 4) * *(data_index + current / 4) 
						          - (u1- v1) * (u1-v1);
						if (delta >= 0.0)
						{
						        s = (u1 + v1 + sqrt(delta))/2.0;
							if (s >= v1)
							{
						        	result_index[current] = s;
							}
							else
							{
								result_index[current] = u1 + *(data_index + current/4);
							}
						}
						else
						{
						        result_index[current] = u1 + *(data_index + current/4);
						}
					} 
				}
				}
				
				/* loop */
				if (trial_num > 0)
				{
				        umin = Infinite;
					for (i = 0; i < storage_size; i++)
					{
					        if (result_index[2] == 1.0)
						{
						       umin = my_Min(umin,result_index[0]);
						}
						result_index += 4;
					}
					for (i = storage_size - 1; i >= 0; i--)
					{
					        result_index -= 4;
						if (result_index[0] == umin)
						new_start_position = i;
						break;
					}
					result_index = (FE_value *)storage;
					result_index[new_start_position * 4 + 2] = 0.0;
				        result_index[new_start_position * 4 + 3] = 1.0;
					
					trial_num--;
					for (i = 0; i < 4; i++)
					{
					/* the point corresponding to direction dir[i]*/
					if ((result_index+new_start_position *4 + dir[i]>=(FE_value *)storage)&&
				             (result_index+new_start_position *4 + dir[i] < (FE_value *)storage + storage_size * 4))
					{
				        	if (result_index[new_start_position *4 + dir[i] + 1] == Infinite)
						{
					        	result_index[new_start_position *4 + dir[i] + 2] = 1.0;
							trial_num++;  
						}
						
						if (result_index[new_start_position *4 + dir[i] + 3] == 0.0)
						{
					        	current = new_start_position * 4 + dir[i];
							if ((result_index + current + offsets[1]>=(FE_value *)storage) &&
				                              (result_index + current + offsets[1] < (FE_value *)storage + storage_size * 4))
							{
						        	u = result_index[current + offsets[1]];
							}
							else
							{
						        	u = Infinite;
							}
							if ((result_index + current + offsets[7]>=(FE_value *)storage) &&
				                             (result_index + current + offsets[7] < (FE_value *)storage + storage_size * 4))
							{
						        	u = my_Min(u,result_index[current + offsets[7]]);
							}
							else
							{
						        	u = my_Min(u,Infinite);
							}
							if ((result_index + current + offsets[3]>=(FE_value *)storage) &&
				                             (result_index + current + offsets[3] < (FE_value *)storage + storage_size * 4))
							{
						        	v = result_index[current + offsets[3]];
							}
							else
							{
						        	v = Infinite;
							}
							if ((result_index + current + offsets[5]>=(FE_value *)storage) &&
				                             (result_index + current + offsets[5] < (FE_value *)storage + storage_size * 4))
							{
						        	v = my_Min(v,result_index[current + offsets[5]]);
							}
							else
							{
						        	v = my_Min(v,Infinite);
							}
							u1 = my_Min (u,v);
							v1 = my_Max (u,v);
							delta = 2.0 * *(data_index + current / 4) * *(data_index + current / 4) - (u1- v1) * (u1-v1);
							if (delta >= 0.0)
							{
						        	s = (u1 + v1 + sqrt(delta))/2.0;
							        if (s >= v1)
							        {
						        	        result_index[current] = s;
							        }
							        else
							        {
								        result_index[current] = u1 + *(data_index + current/4);
							        }
							}
							else
							{
						        	result_index[current] = u1 + *(data_index + current/4);
							}
						} 
					}
					}
				}	
			}
			else if (dimension == 3)
			{
			        
				FE_value u, v, w, u1, v1, w1, s, s1, delta, delta1;
				dir[0] = offsets[4];
				dir[1] = offsets[22];
				dir[2] = offsets[10];
				dir[3] = offsets[16];
				dir[4] = offsets[12];
				dir[5] = offsets[14];
				
				result_index[new_start_position * 4 + 2] = 0.0;
				result_index[new_start_position * 4 + 3] = 1.0;
				
				trial_num--;
				for (i = 0; i < 6; i++)
				{
				/* the point corresponding to direction dir[i]*/
				if ((result_index+new_start_position *4 + dir[i]>=(FE_value *)storage)&&
				          (result_index+new_start_position *4 + dir[i] <
					   (FE_value *)storage + storage_size * 4))
				{
				        if (result_index[new_start_position *4 + dir[i] + 1] == Infinite)
					{
					        result_index[new_start_position *4 + dir[i] + 2] = 1.0;
						trial_num++;  
					}
					
					if (result_index[new_start_position *4 + dir[i] + 3] == 0.0)
					{
					        current = new_start_position * 4 + dir[i];
						if ((result_index + current + offsets[4]>=(FE_value *)storage) &&
				                    (result_index + current + offsets[4] < (FE_value *)storage + storage_size * 4))
						{
						        u = result_index[current + offsets[4]];
						}
						else
						{
						        u = Infinite;
						}
						if ((result_index + current + offsets[22]>=(FE_value *)storage) &&
				                    (result_index + current + offsets[22] < (FE_value *)storage + storage_size * 4))
						{
						        u = my_Min(u,result_index[current + offsets[22]]);
						}
						else
						{
						        u = my_Min(u,Infinite);
						}
						if ((result_index + current + offsets[10]>=(FE_value *)storage) &&
				                    (result_index + current + offsets[10] < (FE_value *)storage + storage_size * 4))
						{
						        v = result_index[current + offsets[10]];
						}
						else
						{
						        v = Infinite;
						}
						if ((result_index + current + offsets[16]>=(FE_value *)storage) &&
				                    (result_index + current + offsets[16] < (FE_value *)storage + storage_size * 4))
						{
						        v = my_Min(v,result_index[current + offsets[16]]);
						}
						else
						{
						        v = my_Min(v,Infinite);
						}
						
						if ((result_index + current + offsets[12]>=(FE_value *)storage) &&
				                    (result_index + current + offsets[12] < (FE_value *)storage + storage_size * 4))
						{
						        w = result_index[current + offsets[12]];
						}
						else
						{
						        w = Infinite;
						}
						if ((result_index + current + offsets[14]>=(FE_value *)storage) &&
				                    (result_index + current + offsets[14] < (FE_value *)storage + storage_size * 4))
						{
						        w = my_Min(w,result_index[current + offsets[14]]);
						}
						else
						{
						        w = my_Min(w,Infinite);
						}
						u1 = my_Min(my_Min (u,v),w);
						w1 = my_Max(my_Max (u,v), w);
						v1 = (u + v + w) - u1 - w1; /*u1 <= v1 <= w1*/
						delta = 3.0 * *(data_index + current / 4) * *(data_index + current / 4);
						delta += 2.0 * (u1 * v1 + u1*w1 + v1*w1);
						delta -= 2.0 *(u1*u1 + v1*v1 + w1*w1);
						if (delta >= 0.0)
						{
						        s = (u1 + v1 + w1 + sqrt(delta))/3.0;
							if (s >= w1)
							{
							        result_index[current] = s;
							}
							else
							{
							        delta1 = 2.0 * *(data_index + current / 4) * *(data_index + current / 4) - (u1- v1) * (u1-v1);
								if (delta1 >= 0.0)
								{
								        s1 = (u1 + v1 + sqrt(delta1))/2.0;
									if (s1 >= v1)
									{
						        		        result_index[current] = s1;
									}
									else
									{
									        result_index[current] = u1 + *(data_index + current/4);
									}
								}
								else
								{
						        		result_index[current] = u1 + *(data_index + current/4);
								}
							}
						}
						else
						{
						        delta1 = 2.0 * *(data_index + current / 4) * *(data_index + current / 4) - (u1- v1) * (u1-v1);
							if (delta1 >= 0.0)
							{
								s1 = (u1 + v1 + sqrt(delta1))/2.0;
								if (s1 >= v1)
								{
						        		result_index[current] = s1;
								}
								else
								{
									result_index[current] = u1 + *(data_index + current/4);
								}
							}
							else
							{
						        	result_index[current] = u1 + *(data_index + current/4);
							}
						}
					} 
				} 
				}
				
				/* loop */
				if (trial_num > 0)
				{
				        umin = Infinite;
					for (i = 0; i < storage_size; i++)
					{
					        if (result_index[2] == 1.0)
						{
						       umin = my_Min(umin,result_index[0]);
						}
						result_index += 4;
					}
					for (i = storage_size - 1; i >= 0; i--)
					{
					        result_index -= 4;
						if (result_index[0] == umin)
						new_start_position = i;
						break;
					}
					result_index = (FE_value *)storage;
					result_index[new_start_position * 4 + 2] = 0.0;
				        result_index[new_start_position * 4 + 3] = 1.0;
					
					trial_num--;
					for (i = 0; i < 6; i++)
					{
					/* the point corresponding to direction dir[i] */
					if ((result_index+new_start_position *4 + dir[i]>=(FE_value *)storage)&&
				          (result_index+new_start_position *4 + dir[i] < (FE_value *)storage + storage_size * 4))
					{
				        	if (result_index[new_start_position *4 + dir[i] + 1] == Infinite)
						{
					        	result_index[new_start_position *4 + dir[i] + 2] = 1.0;
							trial_num++;  
						}
						
						if (result_index[new_start_position *4 + dir[i] + 3] == 0.0)
						{
					        	current = new_start_position * 4 + dir[i];
							if ((result_index + current + offsets[4]>=(FE_value *)storage) &&
				                   	 (result_index + current + offsets[4] < (FE_value *)storage + storage_size * 4))
							{
						        	u = result_index[current + offsets[4]];
							}
							else
							{
						        	u = Infinite;
							}
							if ((result_index + current + offsets[22]>=(FE_value *)storage) &&
				                    	(result_index + current + offsets[22] < (FE_value *)storage + storage_size * 4))
							{
						        	u = my_Min(u,result_index[current + offsets[22]]);
							}
							else
							{
						        	u = my_Min(u,Infinite);
							}
							if ((result_index + current + offsets[10]>=(FE_value *)storage) &&
				                    	(result_index + current + offsets[10] < (FE_value *)storage + storage_size * 4))
							{
						        	v = result_index[current + offsets[10]];
							}
							else
							{
						        	v = Infinite;
							}
							if ((result_index + current + offsets[16]>=(FE_value *)storage) &&
				                    	(result_index + current + offsets[16] < (FE_value *)storage + storage_size * 4))
							{
						        	v = my_Min(v,result_index[current + offsets[16]]);
							}
							else
							{
						        	v = my_Min(v,Infinite);
							}
						
							if ((result_index + current + offsets[12]>=(FE_value *)storage) &&
				                    	(result_index + current + offsets[12] < (FE_value *)storage + storage_size * 4))
							{
						        	w = result_index[current + offsets[12]];
							}
							else
							{
						        	w = Infinite;
							}
							if ((result_index + current + offsets[14]>=(FE_value *)storage) &&
				                    	(result_index + current + offsets[14] < (FE_value *)storage + storage_size * 4))
							{
						        	w = my_Min(w,result_index[current + offsets[14]]);
							}
							else
							{
						        	w = my_Min(w,Infinite);
							}
							u1 = my_Min(my_Min (u,v),w);
							w1 = my_Max(my_Max (u,v), w);
							v1 = (u + v + w) - u1 - w1; /*u1 <= v1 <= w1*/
							delta = 3.0 * *(data_index + current / 4) * *(data_index + current / 4);
							delta += 2.0 * (u1 * v1 + u1*w1 + v1*w1);
							delta -= 2.0 *(u1*u1 + v1*v1 + w1*w1);
							if (delta >= 0.0)
							{
						        	s = (u1 + v1 + w1 + sqrt(delta))/3.0;
								if (s >= w1)
								{
							        	result_index[current] = s;
								}
								else
								{
							        	delta1 = 2.0 * *(data_index + current / 4) * *(data_index + current / 4) - (u1- v1) * (u1-v1);
									if (delta1 >= 0.0)
									{
								        	s1 = (u1 + v1 + sqrt(delta1))/2.0;
										if (s1 >= v1)
										{
						        		        	result_index[current] = s1;
										}
										else
										{
									        	result_index[current] = u1 + *(data_index + current/4);
										}
									}
									else
									{
						        			result_index[current] = u1 + *(data_index + current/4);
									}
								}
							}
							else
							{
						        	delta1 = 2.0 * *(data_index + current / 4) * *(data_index + current / 4) - (u1- v1) * (u1-v1);
								if (delta1 >= 0.0)
								{
									s1 = (u1 + v1 + sqrt(delta1))/2.0;
									if (s1 >= v1)
									{
						        			result_index[current] = s1;
									}
									else
									{
										result_index[current] = u1 + *(data_index + current/4);
									}
								}
								else
								{
						        		result_index[current] = u1 + *(data_index + current/4);
								}
							}
						} 
					}
					}    
				
				}
				
			}
			for (i = 0; i < storage_size; i++)
			{
			        potential_data[i] = result_index[0];
				result_index += 4;     
			}
			DEALLOCATE(offsets);
			DEALLOCATE(storage);
			DEALLOCATE(dir);	
		}
		else
		{
			display_message(ERROR_MESSAGE,
			"Image_cache_minimal_path.  Not enough memory");
			return_code = 0;
		}
       	}
	else
	{
		display_message(ERROR_MESSAGE, "Image_cache_minimal_path.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Image_cache_minimal_action */

static int Image_cache_minimal_path(struct Image_cache *image, double it_step, 
         int *start_position, int *end_position)
/*******************************************************************************
LAST MODIFIED : 4 May 2005

DESCRIPTION :

==============================================================================*/
{
	char *storage;
	FE_value *data_index, *result_index, *potential_data;
	int  return_code;
	int i, k, image_step;
	int data_size, storage_size, start_ps, end_ps;
	FE_value start_end_value;
	
	ENTER(Image_cache_minimal_path);
	if (image && (image->dimension > 0) && (image->depth > 0))
	{
	        return_code = 1;
	        storage_size = image->depth;
		for (i = 0 ; i < image->dimension ; i++)
		{
			storage_size *= image->sizes[i];
		}
		data_size = storage_size / image->depth; 
		if (ALLOCATE(potential_data, FE_value, data_size) &&
			  ALLOCATE(storage, char, storage_size * sizeof(FE_value)))
		{
		        return_code = 1;
			result_index = (FE_value *)storage;
			for (i = 0 ; i < storage_size ; i++)
			{
				*result_index = 0.0;
				result_index++;
			}
			result_index = (FE_value *)storage;
			data_index = (FE_value *)image->data;
			image_step = 1;
			start_ps = 0;
			end_ps = 0;
			for (i = 0; i < image->dimension; i++)
			{
			        start_ps += start_position[i] * image_step;
				end_ps += end_position[i] * image_step;
				image_step *= image->sizes[i];
			}
			
			start_end_value = (*(data_index + start_ps * image->depth) +
			                    *(data_index + end_ps * image->depth))/2.0;
			for (i = 0; i < data_size; i++)
			{
			        potential_data[i] = fabs(*data_index - start_end_value) + 1.0;
				data_index += image->depth;
			}
			data_index = (FE_value *)image->data;
			return_code = Image_cache_minimal_action(potential_data, 
			                  image->dimension, image->sizes, start_ps);
			if (return_code)
			{
				if (image->dimension == 2)
				{
				        FE_value x, y;
					FE_value g_x, g_y;
					int current, previous;
					x = (FE_value)end_position[0];
					y = (FE_value)end_position[1];
					while ((x != (FE_value)start_position[0]) || (y != (FE_value)start_position[1]))
					{
					        /*calculate the gradient of action function*/
					        current = (int)x + (int)y * image->sizes[0];
						for (k = 0; k < image->depth; k++)
						{
						       result_index[current*image->depth + k] = 0.0;
						}
						result_index[current*image->depth] = 1.0;
						previous = current - 1;
						if (((int)x -1) < 0)
						{
						       previous = current;
						}
						g_x = potential_data[current] - potential_data[previous];
						previous = current - image->sizes[0];
						if (((int)y -1) < 0)
						{
						       previous = current;
						}
						g_y = potential_data[current] - potential_data[previous];
						x -= it_step * g_x;
						y -= it_step * g_y;
					}
					current = start_position[0] + start_position[1] * image->sizes[0];
					for (k =0 ; k < image->depth; k++)
					{
					        result_index[current*image->depth + k] = 0.0;
					}
					result_index[current*image->depth] = 1.0;
				}
				else if (image->dimension == 3)
				{
				        FE_value x, y, z;
					FE_value g_x, g_y, g_z;
					int current, previous;
					x = (FE_value)end_position[0];
					y = (FE_value)end_position[1];
					z = (FE_value)end_position[2];
					while ((x != (FE_value)start_position[0]) || (y != (FE_value)start_position[1]) || (z != (FE_value)start_position[2]))
					{
					        /*calculate the gradient of action function*/
					        current = (int)x + (int)y * image->sizes[0] + (int)z * image->sizes[0] * image->sizes[1];
						for (k = 0; k < image->depth; k++)
						{
						       result_index[current*image->depth + k] = 0.0;
						}
						result_index[current*image->depth] = 1.0;
						previous = current - 1;
						if (((int)x -1) < 0)
						{
						       previous = current;
						}
						g_x = potential_data[current] - potential_data[previous];
						previous = current - image->sizes[0];
						if (((int)y -1) < 0)
						{
						       previous = current;
						}
						g_y = potential_data[current] - potential_data[previous];
						previous = current - image->sizes[0]*image->sizes[1];
						if (((int)z -1) < 0)
						{
						       previous = current;
						}
						g_z = potential_data[current] - potential_data[previous];
						x -= it_step * g_x;
						y -= it_step * g_y;
						z -= it_step * g_z;
					}
					current = start_position[0] + start_position[1] * image->sizes[0] + start_position[2] * image->sizes[0] * image->sizes[1];
					for (k =0 ; k < image->depth; k++)
					{
					        result_index[current*image->depth + k] = 0.0;
					}
					result_index[current*image->depth] = 1.0;
				}
			}
			for (i = 0; i < data_size; i++)
			{
			        if (result_index[0]== 0.0)
				for (k = 0; k < image->depth; k++)
				{
				        result_index[k]= *data_index;
				}
				data_index += image->depth;
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
			DEALLOCATE(potential_data);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Image_cache_median_filter.  Not enough memory");
			return_code = 0;
		}	
	}
	else
	{
		display_message(ERROR_MESSAGE, "Image_cache_minimal_path.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}
	

static int Computed_field_minimal_path_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;
	struct Computed_field_minimal_path_type_specific_data *data;

	ENTER(Computed_field_minimal_path_evaluate_cache_at_node);
	if (field && node &&
		(data = (struct Computed_field_minimal_path_type_specific_data *)field->type_specific_data))
	{
		return_code = 1;
		/* 1. Precalculate the Image_cache */
		if (!data->image->valid)
		{
		        Image_cache_update_dimension(data->image,
				data->dimension, data->image->depth,
				data->image->sizes, data->image->minimums,
				data->image->maximums);
			return_code = Image_cache_update_from_fields(data->image, field->source_fields[0],
				field->source_fields[1], data->element_dimension, data->region,
				data->graphics_buffer_package);
			/* 2. Perform image processing operation */

			return_code = Image_cache_minimal_path(data->image,
				data->iteration_step, data->start_position, data->end_position);
		}
		/* 3. Evaluate texture coordinates and copy image to field */
		Computed_field_evaluate_cache_at_node(field->source_fields[1],
			node, time);
		Image_cache_evaluate_field(data->image,field);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_minimal_path_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_minimal_path_evaluate_cache_at_node */

static int Computed_field_minimal_path_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;
	struct Computed_field_minimal_path_type_specific_data *data;

	ENTER(Computed_field_minimal_path_evaluate_cache_in_element);
	USE_PARAMETER(calculate_derivatives);
	if (field && element && xi && (field->number_of_source_fields > 0) &&
		(data = (struct Computed_field_minimal_path_type_specific_data *) field->type_specific_data) &&
		data->image )
	{
		return_code = 1;
		/* 1. Precalculate the Image_cache */
		if (!data->image->valid)
		{
			return_code = Image_cache_update_from_fields(data->image, field->source_fields[0],
				field->source_fields[1], data->element_dimension, data->region,
				data->graphics_buffer_package);
			/* 2. Perform image processing operation */
			return_code = Image_cache_minimal_path(data->image, 
				data->iteration_step, data->start_position, data->end_position);
		}
		/* 3. Evaluate texture coordinates and copy image to field */
		Computed_field_evaluate_cache_in_element(field->source_fields[1],
			element, xi, time, top_level_element, /*calculate_derivatives*/0);
		Image_cache_evaluate_field(data->image,field);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_minimal_path_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_minimal_path_evaluate_cache_in_element */

#define Computed_field_minimal_path_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_minimal_path_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_minimal_path_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_minimal_path_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_minimal_path_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_minimal_path_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

int Computed_field_minimal_path_get_native_resolution(struct Computed_field *field,
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
	struct Computed_field_minimal_path_type_specific_data *data;
	
	ENTER(Computed_field_minimal_path_get_native_resolution);
	if (field && (data =
		(struct Computed_field_minimal_path_type_specific_data *)
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
			"Computed_field_median_filter_get_native_resolution.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_minimal_path_get_native_resolution */

static int list_Computed_field_minimal_path(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 4 April 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_minimal_path_type_specific_data *data;

	ENTER(List_Computed_field_minimal_path);
	if (field && (field->type_string==computed_field_minimal_path_type_string)
		&& (data = (struct Computed_field_minimal_path_type_specific_data *)
		field->type_specific_data))
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,
			"    texture coordinate field : %s\n",field->source_fields[1]->name);
		display_message(INFORMATION_MESSAGE,
			"    iteration_step : %f\n", data->iteration_step);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_minimal_path.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_minimal_path */

static char *Computed_field_minimal_path_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 4 April 2004

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error;
	struct Computed_field_minimal_path_type_specific_data *data;

	ENTER(Computed_field_minimal_path_get_command_string);
	command_string = (char *)NULL;
	if (field && (field->type_string==computed_field_minimal_path_type_string)
		&& (data = (struct Computed_field_minimal_path_type_specific_data *)
		field->type_specific_data) )
	{
		error = 0;
		append_string(&command_string,
			computed_field_minimal_path_type_string, &error);
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

		sprintf(temp_string, " iteration_step %f ", data->iteration_step);
		append_string(&command_string, temp_string, &error);
		
		sprintf(temp_string, " start_position %d %d %d",
		                    data->start_position[0],data->start_position[1],data->start_position[2]);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " end_position %d %d %d",
		                    data->end_position[0],data->end_position[1],data->end_position[2]);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " minimums %f %f %f ",
		                    data->image->minimums[0],data->image->minimums[1], data->image->minimums[2]);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " maximums %f %f %f ",
		                    data->image->maximums[0],data->image->maximums[1],data->image->maximums[2]);
		append_string(&command_string, temp_string, &error);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_minimal_path_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_minimal_path_get_command_string */

#define Computed_field_minimal_path_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 4 April 2004

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_minimal_path(struct Computed_field *field,
	struct Computed_field *source_field,
	struct Computed_field *texture_coordinate_field,
	int dimension, double iteration_step, 
	int *start_position, int *end_position, int *sizes, 
	FE_value *minimums, FE_value *maximums,
	int element_dimension, struct MANAGER(Computed_field) *computed_field_manager,
	struct Cmiss_region *region, struct Graphics_buffer_package *graphics_buffer_package)
/*******************************************************************************
LAST MODIFIED : 4 May 2005

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_minimal_path with the supplied
fields, <source_field> and <texture_coordinate_field>.  The <dimension> is the
size of the <sizes>, <minimums> and <maximums> vectors and should be less than
or equal to the number of components in the <texture_coordinate_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int depth, number_of_source_fields, return_code, i;
	struct Computed_field **source_fields;
	struct Computed_field_minimal_path_type_specific_data *data;

	ENTER(Computed_field_set_type_minimal_path);
	if (field && source_field && texture_coordinate_field &&
		(depth = source_field->number_of_components) &&
		(dimension <= texture_coordinate_field->number_of_components) &&
		region && graphics_buffer_package)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=2;
		data = (struct Computed_field_minimal_path_type_specific_data *)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, number_of_source_fields) &&
			ALLOCATE(data, struct Computed_field_minimal_path_type_specific_data, 1) &&
			ALLOCATE(data->start_position, int, dimension) &&
			ALLOCATE(data->end_position, int, dimension) &&
			(data->image = ACCESS(Image_cache)(CREATE(Image_cache)())) &&
			Image_cache_update_dimension(
			data->image, dimension, depth, sizes, minimums, maximums) &&
			Image_cache_update_data_storage(data->image))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_minimal_path_type_string;
			
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			source_fields[1]=ACCESS(Computed_field)(texture_coordinate_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			data->dimension = dimension;
			for (i = 0; i < dimension; i++)
			{
				data->start_position[i] = start_position[i];
				data->end_position[i] = end_position[i];
			}
			data->iteration_step = iteration_step;
			data->element_dimension = element_dimension;
			data->region = ACCESS(Cmiss_region)(region);
			data->graphics_buffer_package = graphics_buffer_package;
			data->computed_field_manager = computed_field_manager;
			data->computed_field_manager_callback_id =
				MANAGER_REGISTER(Computed_field)(
				Computed_field_minimal_path_field_change, (void *)field,
				computed_field_manager);

			field->type_specific_data = data;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(minimal_path);
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
			"Computed_field_set_type_minimal_path.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_minimal_path */

int Computed_field_get_type_minimal_path(struct Computed_field *field,
	struct Computed_field **source_field,
	struct Computed_field **texture_coordinate_field,
	int *dimension, double *iteration_step, 
	int **start_position, int **end_position, int **sizes, FE_value **minimums,
	FE_value **maximums, int *element_dimension)
/*******************************************************************************
LAST MODIFIED : 4 May 2005

DESCRIPTION :
If the field is of type COMPUTED_FIELD_minimal_path, the
parameters defining it are returned.
==============================================================================*/
{
	int i, return_code;
	struct Computed_field_minimal_path_type_specific_data *data;

	ENTER(Computed_field_get_type_minimal_path);
	if (field && (field->type_string==computed_field_minimal_path_type_string)
		&& (data = (struct Computed_field_minimal_path_type_specific_data *)
		field->type_specific_data) && data->image)
	{
		*dimension = data->image->dimension;
		if (ALLOCATE(*start_position, int, *dimension)
			&& ALLOCATE(*end_position, int, *dimension)
			&& ALLOCATE(*sizes, int, *dimension)
			&& ALLOCATE(*minimums, FE_value, *dimension)
			&& ALLOCATE(*maximums, FE_value, *dimension))
		{
			*source_field = field->source_fields[0];
			*texture_coordinate_field = field->source_fields[1];
			for (i = 0 ; i < *dimension ; i++)
			{
				(*start_position)[i] = data->start_position[i];
				(*end_position)[i] = data->end_position[i];
				(*sizes)[i] = data->image->sizes[i];
				(*minimums)[i] = data->image->minimums[i];
				(*maximums)[i] = data->image->maximums[i];
			}
			*iteration_step = data->iteration_step;
			*element_dimension = data->element_dimension;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_minimal_path.  Unable to allocate vectors.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_minimal_path.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_minimal_path */

static int define_Computed_field_type_minimal_path(struct Parse_state *state,
	void *field_void, void *computed_field_minimal_path_package_void)
/*******************************************************************************
LAST MODIFIED : 12 July 2004

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_minimal_path (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
        char *current_token;
	FE_value *minimums, *maximums;
	int dimension, element_dimension, return_code;
	double iteration_step;
	int *sizes, *start_position, *end_position;
	
	struct Computed_field *field, *source_field, *texture_coordinate_field;
	struct Computed_field_minimal_path_package
		*computed_field_minimal_path_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data,
		set_texture_coordinate_field_data;

	ENTER(define_Computed_field_type_minimal_path);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_minimal_path_package=
		(struct Computed_field_minimal_path_package *)
		computed_field_minimal_path_package_void))
	{
		return_code=1;
		source_field = (struct Computed_field *)NULL;
		texture_coordinate_field = (struct Computed_field *)NULL;
		dimension = 0;
		iteration_step = 0.0;
		start_position = (int *)NULL;
		end_position = (int *)NULL;
		sizes = (int *)NULL;
		minimums = (FE_value *)NULL;
		maximums = (FE_value *)NULL;
		element_dimension = 0;
		/* field */
		set_source_field_data.computed_field_manager =
			computed_field_minimal_path_package->computed_field_manager;
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		
		/* texture_coordinate_field */
		set_texture_coordinate_field_data.computed_field_manager =
			computed_field_minimal_path_package->computed_field_manager;
		set_texture_coordinate_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_texture_coordinate_field_data.conditional_function_user_data = (void *)NULL;

		if (computed_field_minimal_path_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_minimal_path(field,
				&source_field, &texture_coordinate_field, &dimension,  
				&iteration_step, &start_position, &end_position,
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
				/* iteration_step */
				Option_table_add_double_entry(option_table,
					"iteration_step", &iteration_step);
				/* start_position */
				Option_table_add_int_vector_entry(option_table,
					"start_position", start_position, &dimension);
				/* end_position */
				Option_table_add_int_vector_entry(option_table,
					"end_position", end_position, &dimension);
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
						if (!(REALLOCATE(start_position, start_position, int, dimension) &&
							REALLOCATE(end_position, end_position, int, dimension) &&
							REALLOCATE(sizes, sizes, int, dimension) &&
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
				/* iteration_step */
				Option_table_add_double_entry(option_table,
					"iteration_step", &iteration_step);
				/* start_position */
				Option_table_add_int_vector_entry(option_table,
					"start_position", start_position, &dimension);
				/* end_position */
				Option_table_add_int_vector_entry(option_table,
					"end_position", end_position, &dimension);
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
			if (source_field && (!sizes || !maximums || !texture_coordinate_field))
			{
			        return_code = Computed_field_get_native_resolution(source_field,
				     &dimension,&sizes,&minimums,&maximums,&texture_coordinate_field);
			}
			
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_minimal_path(field,
					source_field, texture_coordinate_field, dimension,
					iteration_step, start_position, end_position,
					sizes, minimums, maximums, element_dimension,
					computed_field_minimal_path_package->computed_field_manager,
					computed_field_minimal_path_package->root_region,
					computed_field_minimal_path_package->graphics_buffer_package);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_minimal_path.  Failed");
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
			if (start_position)
			{
				DEALLOCATE(start_position);
			}
			if (end_position)
			{
				DEALLOCATE(end_position);
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
			"define_Computed_field_type_minimal_path.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_minimal_path */

int Computed_field_register_types_minimal_path(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package)
/*******************************************************************************
LAST MODIFIED : 12 April 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_minimal_path_package
		computed_field_minimal_path_package;

	ENTER(Computed_field_register_types_minimal_path);
	if (computed_field_package)
	{
		computed_field_minimal_path_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		computed_field_minimal_path_package.root_region = root_region;
		computed_field_minimal_path_package.graphics_buffer_package = graphics_buffer_package;
		return_code = Computed_field_package_add_type(computed_field_package,
			            computed_field_minimal_path_type_string,
			            define_Computed_field_type_minimal_path,
			            &computed_field_minimal_path_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_minimal_path.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_minimal_path */

