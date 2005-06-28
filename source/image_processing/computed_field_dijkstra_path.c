/************************************************************************************
   FILE: computed_field_dijkstra_path.c

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
#include "image_processing/computed_field_dijkstra_path.h"

#define my_Min(x,y) ((x) <= (y) ? (x) : (y))
#define my_Max(x,y) ((x) <= (y) ? (y) : (x))
#define abs(a)   ((a) < 0 ? -(a) : (a))
#define Infinite (1 << 14)

#define drow(row,i) (row+offset[i][0])
#define dcol(col,i) (col+offset[i][1])

struct Computed_field_dijkstra_path_package
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

struct Computed_field_dijkstra_path_type_specific_data
{
	int dimension;
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

static char computed_field_dijkstra_path_type_string[] = "dijkstra_path";

int Computed_field_is_type_dijkstra_path(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_dijkstra_path);
	if (field)
	{
		return_code =
		  (field->type_string == computed_field_dijkstra_path_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_dijkstra_path.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_dijkstra_path */

static void Computed_field_dijkstra_path_field_change(
	struct MANAGER_MESSAGE(Computed_field) *message, void *field_void)
/*******************************************************************************
LAST MODIFIED : 5 April 2004

DESCRIPTION :
Manager function called back when either of the source fields change so that
we know to invalidate the image cache.
==============================================================================*/
{
	struct Computed_field *field;
	struct Computed_field_dijkstra_path_type_specific_data *data;

	ENTER(Computed_field_dijkstra_path_source_field_change);
	if (message && (field = (struct Computed_field *)field_void) && (data =
		(struct Computed_field_dijkstra_path_type_specific_data *)
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
			"Computed_field_dijkstra_path_source_field_change.  "
			"Invalid arguments.");
	}
	LEAVE;
} /* Computed_field_dijkstra_path_source_field_change */

static int Computed_field_dijkstra_path_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_dijkstra_path_type_specific_data *data;

	ENTER(Computed_field_dijkstra_path_clear_type_specific);
	if (field && (data =
		(struct Computed_field_dijkstra_path_type_specific_data *)
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
			"Computed_field_dijkstra_path_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_dijkstra_path_clear_type_specific */

static void *Computed_field_dijkstra_path_copy_type_specific(
	struct Computed_field *source_field, struct Computed_field *destination_field)
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_dijkstra_path_type_specific_data *destination,
		*source;
	int i;

	ENTER(Computed_field_dijkstra_path_copy_type_specific);
	if (source_field && destination_field && (source =
		(struct Computed_field_dijkstra_path_type_specific_data *)
		source_field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_dijkstra_path_type_specific_data, 1) &&
			ALLOCATE(destination->start_position, int, source->dimension) &&
			ALLOCATE(destination->end_position, int, source->dimension))
		{
			destination->dimension = source->dimension;
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
				Computed_field_dijkstra_path_field_change, (void *)destination_field,
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
				"Computed_field_dijkstra_path_copy_type_specific.  "
				"Unable to allocate memory.");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_dijkstra_path_copy_type_specific.  "
			"Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_dijkstra_path_copy_type_specific */

int Computed_field_dijkstra_path_clear_cache_type_specific
   (struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_dijkstra_path_type_specific_data *data;

	ENTER(Computed_field_dijkstra_path_clear_type_specific);
	if (field && (data =
		(struct Computed_field_dijkstra_path_type_specific_data *)
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
			"Computed_field_dijkstra_path_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_dijkstra_path_clear_type_specific */

static int Computed_field_dijkstra_path_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;
	struct Computed_field_dijkstra_path_type_specific_data *data,
		*other_data;

	ENTER(Computed_field_dijkstra_path_type_specific_contents_match);
	if (field && other_computed_field && (data =
		(struct Computed_field_dijkstra_path_type_specific_data *)
		field->type_specific_data) && (other_data =
		(struct Computed_field_dijkstra_path_type_specific_data *)
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
} /* Computed_field_dijkstra_path_type_specific_contents_match */

#define Computed_field_dijkstra_path_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_dijkstra_path_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_dijkstra_path_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_dijkstra_path_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 4 May 2005

DESCRIPTION :
No special criteria.
==============================================================================*/

PQUEUE *pqueue_new(void)
{
        PQUEUE *pq;
	ENTER(pqueue_new);
	if (ALLOCATE(pq, PQUEUE, 1))
	{
	        pq->count = 0;
		pq->priorities = NULL;
	}
	else
	{
	        display_message(ERROR_MESSAGE,
			"Pqueue_new.  Not enough memory");
	        pq = (PQUEUE *)NULL;
	}
	LEAVE;
	return (pq);
}

int pqueue_insert(PQUEUE *pq, int row, int col, int wt)
{
        ELIST *e;
	PLIST *p, *pp;
	int return_code;
	ENTER(pqueue_insert);
	if (pq)
	{
	        return_code = 1;
		if (ALLOCATE(e, ELIST,1) && ALLOCATE(p, PLIST, 1) && ALLOCATE(pp, PLIST,1))
		{
		        return_code = 1;
		        e->row = row;
			e->col = col;
			e->next = NULL;

                         /* Test empty queue or before the first element of the queue */
			if (pq->priorities == NULL || pq->priorities->wt > wt) 
			{
			         p->wt = wt;
				 p->elements = e;
				 p->next = pq->priorities;
				 pq->priorities = p;
				 pq->count++;
			}
			else
			{
			        p = pq->priorities;
				if (p->next && p->next->wt <= wt)
				{
				        p = p->next;
				}
				pq->count++;
				if (p->wt == wt) 
				{
				        e->next = p->elements;
					p->elements = e;
				}
				else
				{
				        pp->wt = wt;
					pp->elements = e;
					pp->next = p->next;
					p->next = pp;
					pq->count++;
				}
			}
			//DEALLOCATE(e);
			//DEALLOCATE(p);
			//DEALLOCATE(pp);
		}
		else
		{
		        display_message(ERROR_MESSAGE,
			"pqueue_insert.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
	        display_message(ERROR_MESSAGE,
			"pqueue_insert.  Missing stack. ");
		return_code = 0;
	}
	LEAVE;
	return(return_code);
}

int pqueue_peekmin(PQUEUE *pq, int *row, int *col)
{
        int return_code;
	ENTER(pqueue_peekmin);
	if (pq == NULL || pq->priorities == NULL || pq->priorities->elements == NULL)
	{
	        display_message(ERROR_MESSAGE,
			"pqueue_peekmin.  Missing stack. ");
		return_code = 0;
	}
	else
	{
	        *row = pq->priorities->elements->row;
		*col = pq->priorities->elements->col;
		return_code = 1;
	}
	LEAVE;
        return(return_code);
}

int pqueue_popmin(PQUEUE *pq, int *row, int *col)
{
        ELIST *tmpe;
	PLIST *tmpp;
	int return_code;
	
	ENTER(pqueue_popmin);
	return_code = pqueue_peekmin(pq, row, col);
	if (return_code)
	{
	        pq->count--;
		tmpe = pq->priorities->elements;
		pq->priorities->elements = pq->priorities->elements->next;
		DEALLOCATE(tmpe);
		if (pq->priorities->elements == NULL) 
		{
		        tmpp = pq->priorities;
			pq->priorities = pq->priorities->next;
			DEALLOCATE(tmpp);
		}
	}
	else
	{
	       display_message(ERROR_MESSAGE,
			"pqueue_popmin.  Failure.. ");
		return_code = 0;
	}
	LEAVE;
	
	return (return_code);
}


FE_value h_2d(int r1, int c1, int r2, int c2)
{
   return((FE_value)my_Max(abs(r2 - r1), abs(c2 - c1)));
}

int build_path_2d(FE_value *potential_data, int width, int height,
                FE_value *dist, int *parent, int *s_rc, int *e_rc)
/**************************************************************************
 LAST MODIFIED: 21 June 2005

 DISCRIPTION:
   This routine is entire algorithm:
 
   push the starting point onto the queue
   while there is an element of least weight in the queue, say (r,c)
   foreach square around (r,c), say (r',c') where
 	  dist[to (r,c)] + weight(r',c') < dist[to (r',c')]
     do
 	  dist[to (r',c')] = dist[to (r,c)] + weight(r',c')
 	  push (r',c') onto the queue
===========================================================================*/
{
        PQUEUE * pq;
        int   row, col, start_r, start_c, end_r, end_c;
	int i, return_code;
	int pushes, pops;
	int offset[8][2] = {{1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}};
	#define drow(row,i) (row+offset[i][0])
        #define dcol(col,i) (col+offset[i][1])
	
	ENTER(build_path_2d);
	pq = pqueue_new();
	start_r = s_rc[1];
	start_c = s_rc[0];
	end_r = e_rc[1];
	end_c = e_rc[0];
	if (start_r < 0 || start_r >= height || start_c < 0 || start_c >= width ||
	           end_r < 0 || end_r >= height || end_c < 0 || end_c >= width)
	{
	        display_message(ERROR_MESSAGE,
			"Build_path. Invalid arguments.");
	        return_code = 0;
	}
	else
	{
	        return_code = 1;
		pushes = 0;
		pops = 0;
	        dist[start_r*width+start_c] = 0;
		parent[(start_r*width+start_c)*2] = -1;
		parent[(start_r*width+start_c)*2+1] = -1;
		/*return_code = pqueue_insert(pq, start_r, start_c, 0 + h_2d(start_r,start_c,end_r,end_c));*/
		pqueue_insert(pq, start_r, start_c, 0 + h_2d(start_r,start_c,end_r,end_c));
		pushes++;
		
		while (pqueue_popmin(pq, &row, &col)) 
		{
		        /* DEBUG INFORMATION */
			pops++;
			if (row == end_r && col == end_c) 
			{
			         /* STOP HERE */
				 break;
			}
			/* FOREACH square adjacent to (row,col), call it drow(row,i), dcol(col,i) */
			for (i = 0; i < 8; i++) 
			{
			         /* Check to make sure the square is on the map */
				 if (drow(row,i) < 0 || drow(row,i) >= height ||
		                           dcol(col,i) < 0 || dcol(col,i) >= width)
					 continue;

	                        /* Check to see if we've found a shorter path */
				if (dist[row*width+col] + potential_data[drow(row,i)*width + dcol(col,i)] 
				           < dist[drow(row,i)*width+dcol(col,i)]) 
				{
		
		                        /* We have a shorter path, update the information */
					dist[drow(row,i)*width+dcol(col,i)] = 
					      potential_data[drow(row,i)*width + dcol(col,i)] + dist[row*width+col];
					parent[(drow(row,i)*width+dcol(col,i))*2] = row;
					parent[(drow(row,i)*width+dcol(col,i))*2+1] = col;
		                        /* Push the modified square onto the priority queue */
					pqueue_insert(pq, drow(row,i), dcol(col,i), dist[drow(row,i)*width+dcol(col,i)] + h_2d(drow(row, i), dcol(col, i), end_r, end_c));
					pushes++;
				} 
			}
		}
	}
	DEALLOCATE(pq);
	LEAVE;
        return (return_code);
}/* Build_path_2d */

void limit_path_2d(int width, int height, FE_value *dist, int *parent, int *e_rc)
/**********************************************************************
 LAST MODIFIED: 21 June 2005
 
 DISCRIPTION:
  This routine will set the parent of every square which is not on the
  shortest path we selected between the starting and ending points.
  It does this by tracing back the path and multiplying the distance of
  the shortest path to that point by -1 to identify the squares which
  lie on the path.
  Then, the entire set of squares is run through to return the distance
  values to their original values and clear the parent information for
  all squares in the path.
  This is not a pretty way of getting the path but it's the easiest way
  to do it in place in such a way that we can easily dump the map to
  the screen.
 =======================================================================*/
{
        int row, col;
	int tmp;
        
	ENTER(limit_path_2d);
	row = e_rc[1];
	col = e_rc[0];
	do 
	{
	        dist[row*width+col] = -dist[row*width+col];
		tmp = parent[(row*width+col)*2];
		col = parent[(row*width+col)*2 + 1];
		row = tmp;
	} while(row >= 0 && col >= 0);
        for (row = 0; row < height; row++)
	{
	        for (col = 0; col < width; col++)
		{
	                if (dist[row*width+col] < 0)
			{
		                dist[row*width+col] = -dist[row*width+col];
			}
			else
			{
			         parent[(row*width+col)*2] = -1;
			}
		}
	}
	LEAVE;
	
}/* Limit_path_2d */

static int Image_cache_dijkstra_path(struct Image_cache *image, 
         int *start_posi, int *end_posi)
/*******************************************************************************
LAST MODIFIED : 21 June 2005

DESCRIPTION :
        This is a small demo program which illustrates a modified Djisktra
  algorithm for finding the shortest path between two points.	In fact,
  the algorithm finds all shortest paths from some starting point to
  all other points in the map and then we display the path with the
  user requested end-point.

==============================================================================*/
{
	char *storage;
	FE_value *data_index, *result_index, *potential_data;
	int  return_code;
	int i, k, image_step;
	int data_size, storage_size, start_ps, end_ps;
	FE_value start_end_value;
	FE_value *dist;
	int *parent;
	
	ENTER(Image_cache_dijkstra_path);
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
			  ALLOCATE(storage, char, storage_size * sizeof(FE_value)) &&
			  ALLOCATE(dist, FE_value, data_size) &&
			  ALLOCATE(parent, int, data_size*2))
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
			        start_ps += start_posi[i] * image_step;
				end_ps += end_posi[i] * image_step;
				image_step *= image->sizes[i];
			}
			
			start_end_value = (*(data_index + start_ps * image->depth) +
			                    *(data_index + end_ps * image->depth))/2.0;
			for (i = 0; i < data_size; i++)
			{
			        potential_data[i] = fabs(*data_index - start_end_value)*256.0 + 26.0;
				//if (potential_data[i] > 100)
				//{
				        //potential_data[i] = Infinite;
				//}
				data_index += image->depth;
			}
			data_index = (FE_value *)image->data;
			if (image->dimension == 2)
			{
			        int wt, ht, row, col, bpp;
				int start_r, start_c, end_r, end_c;
				wt = image->sizes[0];
				ht = image->sizes[1];
				start_r = start_posi[1];
				start_c = start_posi[0];
				end_r = start_posi[1];
				end_c = start_posi[0];
				bpp = image->depth;
			        for (i = 0; i < data_size; i++)
				{
				        dist[i] = Infinite;
					parent[2*i]= -1;
					parent[2*i+1] = -1;
				}
			        return_code = build_path_2d(potential_data, wt, ht, dist, 
				                   parent, start_posi, end_posi);
				DEALLOCATE(potential_data);
				if (return_code)
				{
				        limit_path_2d(wt, ht, dist, parent, end_posi);
					DEALLOCATE(dist);
					for (row = 0; row < ht; row++) 
				        {
						for (col = 0; col < wt; col++) 
						{
	        					if (start_r == row && start_c == col)
	        					{
							        result_index[(row*wt+col)*bpp] = 1.0;
								for (k = 1; k < bpp; k++)
								{
								         result_index[(row*wt+col)*bpp+k] = 0.0;
								        
								}
	        					}
							else if (end_r == row && end_c == col)
							{
							        result_index[(row*wt+col)*bpp] = 1.0;
								for (k = 1; k < bpp; k++)
								{
								         result_index[(row*wt+col)*bpp+k] = 0.0;
								        
								}
							}
							else if (parent[(row*wt+col)*2] >= 0 && parent[(row*wt+col)*2+1] >= 0) 
							{
							        result_index[(row*wt+col)*bpp] = 1.0;
								for (k = 1; k < bpp; k++)
								{
								         result_index[(row*wt+col)*bpp+k] = 0.0;
								        
								}
							} 
						}
					}
					DEALLOCATE(parent);
				}
				
			}
			else if (image->dimension == 3)
			{
				
			}
			result_index = (FE_value *)storage;
			for (i = 0; i < data_size; i++)
			{
			        if (result_index[0]== 0.0)
				{
				        for (k = 0; k < image->depth; k++)
				        {
				               result_index[k]= *(data_index+k);
				        }
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
		display_message(ERROR_MESSAGE, "Image_cache_dijkstra_path.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}
	

static int Computed_field_dijkstra_path_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;
	struct Computed_field_dijkstra_path_type_specific_data *data;

	ENTER(Computed_field_dijkstra_path_evaluate_cache_at_node);
	if (field && node &&
		(data = (struct Computed_field_dijkstra_path_type_specific_data *)field->type_specific_data))
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

			return_code = Image_cache_dijkstra_path(data->image,
				data->start_position, data->end_position);
		}
		/* 3. Evaluate texture coordinates and copy image to field */
		Computed_field_evaluate_cache_at_node(field->source_fields[1],
			node, time);
		Image_cache_evaluate_field(data->image,field);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_dijkstra_path_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_dijkstra_path_evaluate_cache_at_node */

static int Computed_field_dijkstra_path_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;
	struct Computed_field_dijkstra_path_type_specific_data *data;

	ENTER(Computed_field_dijkstra_path_evaluate_cache_in_element);
	USE_PARAMETER(calculate_derivatives);
	if (field && element && xi && (field->number_of_source_fields > 0) &&
		(data = (struct Computed_field_dijkstra_path_type_specific_data *) field->type_specific_data) &&
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
			return_code = Image_cache_dijkstra_path(data->image, 
				data->start_position, data->end_position);
		}
		/* 3. Evaluate texture coordinates and copy image to field */
		Computed_field_evaluate_cache_in_element(field->source_fields[1],
			element, xi, time, top_level_element, /*calculate_derivatives*/0);
		Image_cache_evaluate_field(data->image,field);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_dijkstra_path_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_dijkstra_path_evaluate_cache_in_element */

#define Computed_field_dijkstra_path_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_dijkstra_path_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_dijkstra_path_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_dijkstra_path_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_dijkstra_path_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_dijkstra_path_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 5 May 2005

DESCRIPTION :
Not implemented yet.
==============================================================================*/

int Computed_field_dijkstra_path_get_native_resolution(struct Computed_field *field,
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
	struct Computed_field_dijkstra_path_type_specific_data *data;
	
	ENTER(Computed_field_dijkstra_path_get_native_resolution);
	if (field && (data =
		(struct Computed_field_dijkstra_path_type_specific_data *)
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
} /* Computed_field_dijkstra_path_get_native_resolution */

static int list_Computed_field_dijkstra_path(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 4 April 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_dijkstra_path_type_specific_data *data;

	ENTER(List_Computed_field_dijkstra_path);
	if (field && (field->type_string==computed_field_dijkstra_path_type_string)
		&& (data = (struct Computed_field_dijkstra_path_type_specific_data *)
		field->type_specific_data))
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
			"list_Computed_field_dijkstra_path.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_dijkstra_path */

static char *Computed_field_dijkstra_path_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 4 April 2004

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error;
	struct Computed_field_dijkstra_path_type_specific_data *data;

	ENTER(Computed_field_dijkstra_path_get_command_string);
	command_string = (char *)NULL;
	if (field && (field->type_string==computed_field_dijkstra_path_type_string)
		&& (data = (struct Computed_field_dijkstra_path_type_specific_data *)
		field->type_specific_data) )
	{
		error = 0;
		append_string(&command_string,
			computed_field_dijkstra_path_type_string, &error);
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
			"Computed_field_dijkstra_path_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_dijkstra_path_get_command_string */

#define Computed_field_dijkstra_path_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 4 April 2004

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_dijkstra_path(struct Computed_field *field,
	struct Computed_field *source_field,
	struct Computed_field *texture_coordinate_field,
	int dimension, 
	int *start_position, int *end_position, int *sizes, 
	FE_value *minimums, FE_value *maximums,
	int element_dimension, struct MANAGER(Computed_field) *computed_field_manager,
	struct Cmiss_region *region, struct Graphics_buffer_package *graphics_buffer_package)
/*******************************************************************************
LAST MODIFIED : 4 May 2005

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_dijkstra_path with the supplied
fields, <source_field> and <texture_coordinate_field>.  The <dimension> is the
size of the <sizes>, <minimums> and <maximums> vectors and should be less than
or equal to the number of components in the <texture_coordinate_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int depth, number_of_source_fields, return_code, i;
	struct Computed_field **source_fields;
	struct Computed_field_dijkstra_path_type_specific_data *data;

	ENTER(Computed_field_set_type_dijkstra_path);
	if (field && source_field && texture_coordinate_field &&
		(depth = source_field->number_of_components) &&
		(dimension <= texture_coordinate_field->number_of_components) &&
		region && graphics_buffer_package)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=2;
		data = (struct Computed_field_dijkstra_path_type_specific_data *)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, number_of_source_fields) &&
			ALLOCATE(data, struct Computed_field_dijkstra_path_type_specific_data, 1) &&
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
			field->type_string = computed_field_dijkstra_path_type_string;
			
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
			data->element_dimension = element_dimension;
			data->region = ACCESS(Cmiss_region)(region);
			data->graphics_buffer_package = graphics_buffer_package;
			data->computed_field_manager = computed_field_manager;
			data->computed_field_manager_callback_id =
				MANAGER_REGISTER(Computed_field)(
				Computed_field_dijkstra_path_field_change, (void *)field,
				computed_field_manager);

			field->type_specific_data = data;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(dijkstra_path);
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
			"Computed_field_set_type_dijkstra_path.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_dijkstra_path */

int Computed_field_get_type_dijkstra_path(struct Computed_field *field,
	struct Computed_field **source_field,
	struct Computed_field **texture_coordinate_field,
	int *dimension, 
	int **start_position, int **end_position, int **sizes, FE_value **minimums,
	FE_value **maximums, int *element_dimension)
/*******************************************************************************
LAST MODIFIED : 4 May 2005

DESCRIPTION :
If the field is of type COMPUTED_FIELD_dijkstra_path, the
parameters defining it are returned.
==============================================================================*/
{
	int i, return_code;
	struct Computed_field_dijkstra_path_type_specific_data *data;

	ENTER(Computed_field_get_type_dijkstra_path);
	if (field && (field->type_string==computed_field_dijkstra_path_type_string)
		&& (data = (struct Computed_field_dijkstra_path_type_specific_data *)
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
			*element_dimension = data->element_dimension;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_dijkstra_path.  Unable to allocate vectors.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_dijkstra_path.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_dijkstra_path */

static int define_Computed_field_type_dijkstra_path(struct Parse_state *state,
	void *field_void, void *computed_field_dijkstra_path_package_void)
/*******************************************************************************
LAST MODIFIED : 12 July 2004

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_dijkstra_path (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
        char *current_token;
	FE_value *minimums, *maximums;
	int dimension, element_dimension, return_code;
	int *sizes, *start_position, *end_position;
	
	struct Computed_field *field, *source_field, *texture_coordinate_field;
	struct Computed_field_dijkstra_path_package
		*computed_field_dijkstra_path_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data,
		set_texture_coordinate_field_data;

	ENTER(define_Computed_field_type_dijkstra_path);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_dijkstra_path_package=
		(struct Computed_field_dijkstra_path_package *)
		computed_field_dijkstra_path_package_void))
	{
		return_code=1;
		source_field = (struct Computed_field *)NULL;
		texture_coordinate_field = (struct Computed_field *)NULL;
		dimension = 0;
		start_position = (int *)NULL;
		end_position = (int *)NULL;
		sizes = (int *)NULL;
		minimums = (FE_value *)NULL;
		maximums = (FE_value *)NULL;
		element_dimension = 0;
		/* field */
		set_source_field_data.computed_field_manager =
			computed_field_dijkstra_path_package->computed_field_manager;
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		
		/* texture_coordinate_field */
		set_texture_coordinate_field_data.computed_field_manager =
			computed_field_dijkstra_path_package->computed_field_manager;
		set_texture_coordinate_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_texture_coordinate_field_data.conditional_function_user_data = (void *)NULL;

		if (computed_field_dijkstra_path_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_dijkstra_path(field,
				&source_field, &texture_coordinate_field, &dimension,  
				&start_position, &end_position,
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
				return_code = Computed_field_set_type_dijkstra_path(field,
					source_field, texture_coordinate_field, dimension,
					start_position, end_position,
					sizes, minimums, maximums, element_dimension,
					computed_field_dijkstra_path_package->computed_field_manager,
					computed_field_dijkstra_path_package->root_region,
					computed_field_dijkstra_path_package->graphics_buffer_package);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_dijkstra_path.  Failed");
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
			"define_Computed_field_type_dijkstra_path.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_dijkstra_path */

int Computed_field_register_types_dijkstra_path(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package)
/*******************************************************************************
LAST MODIFIED : 12 April 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_dijkstra_path_package
		computed_field_dijkstra_path_package;

	ENTER(Computed_field_register_types_dijkstra_path);
	if (computed_field_package)
	{
		computed_field_dijkstra_path_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		computed_field_dijkstra_path_package.root_region = root_region;
		computed_field_dijkstra_path_package.graphics_buffer_package = graphics_buffer_package;
		return_code = Computed_field_package_add_type(computed_field_package,
			            computed_field_dijkstra_path_type_string,
			            define_Computed_field_type_dijkstra_path,
			            &computed_field_dijkstra_path_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_dijkstra_path.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_dijkstra_path */

