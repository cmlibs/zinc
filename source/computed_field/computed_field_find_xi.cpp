/*******************************************************************************
FILE : computed_field_find_xi.c

LAST MODIFIED : 4 July 2000

DESCRIPTION :
Implements a special version of find_xi that uses OpenGL to accelerate the
lookup of the element.
==============================================================================*/
#include <stdio.h>
#include <math.h>

#include "general/debug.h"
#include "general/image_utilities.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_find_xi.h"
#include "three_d_drawing/dm_interface.h"
#include "user_interface/message.h"

struct Render_element_data
{
	int bit_shift;
	int minimum_element_number;
	int maximum_element_number;
	struct Computed_field *field;
	FE_value values[3];
};

struct Computed_field_find_element_xi_special_cache
{
	int bit_shift;
	int minimum_element_number;
	int maximum_element_number;
	struct Dm_buffer *dmbuffer;
};

int Computed_field_iterative_element_conditional(
	struct FE_element *element, void *data_void)
/*******************************************************************************
LAST MODIFIED: 16 June 2000

DESCRIPTION:
Returns true if a valid element xi is found.
==============================================================================*/
{
	FE_value determinant, *derivatives, *values;
	int i, number_of_xi, return_code;
	struct Computed_field_iterative_find_element_xi_data *data;

	ENTER(Computed_field_iterative_element_conditional);

	if (element &&
		(data = (struct Computed_field_iterative_find_element_xi_data *)data_void))
	{
		number_of_xi = get_FE_element_dimension(element);
		if (number_of_xi <= data->number_of_values)
		{
			return_code = 1;
			if (data->found_number_of_xi != number_of_xi)
			{
				if (REALLOCATE(derivatives, data->found_derivatives, FE_value,
					data->number_of_values * number_of_xi))
				{
					data->found_derivatives = derivatives;
					data->found_number_of_xi = number_of_xi;
					return_code = 1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_iterative_element_conditional.  Unable to allocate derivative storage");
					return_code = 0;
				}
			}
			if (return_code)
			{
				values = data->found_values;
				derivatives = data->found_derivatives;

				/* Start at centre to find element xi */
				for (i = 0 ; i < number_of_xi ; i++)
				{
					data->xi[i] = 0.5;
				}
				Computed_field_evaluate_in_element(data->field, element, data->xi,
					(struct FE_element *)NULL, values, derivatives);

				/* Solve optimally for each number of xi */
				switch (number_of_xi)
				{
					case 1:
					{
						data->xi[0] = (data->values[0] - values[0]) / derivatives[0] + 0.5;
						if ((data->xi[0] >= -data->tolerance) && (data->xi[0] <= 1.0 + data->tolerance))
						{
							return_code = 1;
						}
						else
						{
							return_code = 0;
						}
						for (i = 1 ; return_code && (i < data->number_of_values); i++)
						{
							if (data->tolerance > fabs ((data->values[0] - values[0]) / derivatives[0]
								+ 0.5 - data->xi[0]))
							{
								return_code = 0;
							}
						}
					} break;
					case 2:
					{
						determinant = derivatives[0] * derivatives[3] - derivatives[1] * derivatives[2];
						if ((determinant > 1e-12) || (determinant < -1e-12))
						{
							data->xi[0] = (derivatives[3] * (data->values[0] - values[0]) -
								derivatives[1] * (data->values[1] - values[1]))
								/ determinant + 0.5;
							if ((derivatives[1] > 1e-12) || (derivatives[1] < -1e-12))
							{
								data->xi[1] = (data->values[0] - values[0] - derivatives[0] * (data->xi[0] - 0.5))
									/ derivatives[1] + 0.5;
							}
							else
							{
								data->xi[1] = (data->values[1] - values[1] - derivatives[2] * (data->xi[0] - 0.5))
									/ derivatives[3] + 0.5;								
							}
						}
						else
						{
							data->xi[0] = -1;
							data->xi[1] = -1;
						}
						if (SIMPLEX_SHAPE== *(element->shape->type))
						{
							if ((data->xi[0] >= -data->tolerance) && (data->xi[1] >= -data->tolerance)
								&& (data->xi[0] + data->xi[1] <= 1.0 + data->tolerance))
							{
								return_code = 1;
							}
							else
							{
								return_code = 0;
							}
							for (i = 2 ; return_code && (i < data->number_of_values); i++)
							{
								/* Check tolerance */
							}
						}
						else
						{
							if ((data->xi[0] >= -data->tolerance) && (data->xi[0] <= 1.0 + data->tolerance)
								&& (data->xi[1] >= -data->tolerance) && (data->xi[1] <= 1.0 + data->tolerance))
							{
								return_code = 1;
							}
							else
							{
								return_code = 0;
							}
							for (i = 2 ; return_code && (i < data->number_of_values); i++)
							{
								/* Check tolerance */
							}
						}
					} break;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_iterative_element_conditional.  Unable to solve undertermined system");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_iterative_element_conditional.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return(return_code);
} /* Computed_field_iterative_element_conditional */

static int Expand_element_range(struct FE_element *element, void *data_void)
/*******************************************************************************
LAST MODIFIED : 26 June 2000

DESCRIPTION :
Stores cache data for the Computed_field_find_element_xi_special routine.
==============================================================================*/
{
	int return_code;
	struct Computed_field_find_element_xi_special_cache *data;
	
	ENTER(Expand_element_range);
	
	if (data = (struct Computed_field_find_element_xi_special_cache *)data_void)
	{
		/* Expand the element number range */
		if ((element->cm.type == CM_ELEMENT) && (2 == get_FE_element_dimension(element)))
		{
			if (element->cm.number > data->maximum_element_number)
			{
				data->maximum_element_number = element->cm.number;
			}
			if (element->cm.number < data->minimum_element_number)
			{
				data->minimum_element_number = element->cm.number;
			}			
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Expand_element_range.  Missing Computed_field_find_element_xi_special_cache data.");
		return_code = 0;
	}
	LEAVE;
	
	return (return_code);
} /* Expand_element_range */

static int Render_element_as_texture(struct FE_element *element, void *data_void)
/*******************************************************************************
LAST MODIFIED : 20 June 2000

DESCRIPTION :
Stores cache data for the Computed_field_find_element_xi_special routine.
==============================================================================*/
{
	unsigned int scaled_number;
	int return_code;
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct Render_element_data *data;
	float red, green, blue;
	
	ENTER(Render_element_as_texture);
	
	if (data = (struct Render_element_data *)data_void)
	{
		/* Code the element number into the colour */
		if ((element->cm.type == CM_ELEMENT) && (2 == get_FE_element_dimension(element)))
		{
			scaled_number = ((double)element->cm.number - data->minimum_element_number + 1);
			blue = ((double)((scaled_number & ((1 << data->bit_shift) - 1)) << 
				(8 - data->bit_shift)) + 0.5) / 255.0;
			scaled_number = scaled_number >> data->bit_shift;
			green = ((double)((scaled_number & ((1 << data->bit_shift) - 1)) << 
				(8 - data->bit_shift)) + 0.5) / 255.0;
			scaled_number = scaled_number >> data->bit_shift;
			red = ((double)((scaled_number & ((1 << data->bit_shift) - 1)) << 
				(8 - data->bit_shift)) + 0.5) / 255.0;

			glColor3f(red, green, blue);
		  
			xi[0] = 0.0;
			xi[1] = 0.0;
			Computed_field_evaluate_in_element(data->field, element, xi,
				(struct FE_element *)NULL, data->values, (FE_value *)NULL);
			glVertex2fv(data->values);

			xi[0] = 1.0;
			xi[1] = 0.0;
			Computed_field_evaluate_in_element(data->field, element, xi,
				(struct FE_element *)NULL, data->values, (FE_value *)NULL);
			glVertex2fv(data->values);

			if (SIMPLEX_SHAPE== *(element->shape->type))
			{
				xi[0] = 0.0;
				xi[1] = 1.0;
				Computed_field_evaluate_in_element(data->field, element, xi,
					(struct FE_element *)NULL, data->values, (FE_value *)NULL);
				glVertex2fv(data->values);
				glVertex2fv(data->values);
			}
			else
			{
				xi[0] = 1.0;
				xi[1] = 1.0;
				Computed_field_evaluate_in_element(data->field, element, xi,
					(struct FE_element *)NULL, data->values, (FE_value *)NULL);
				glVertex2fv(data->values);
				
				xi[0] = 0.0;
				xi[1] = 1.0;
				Computed_field_evaluate_in_element(data->field, element, xi,
					(struct FE_element *)NULL, data->values, (FE_value *)NULL);
				glVertex2fv(data->values);
			}
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Render_element_as_texture.  Missing Computed_field_find_element_xi_special_cache data.");
		return_code = 0;
	}
	LEAVE;
	
	return (return_code);
} /* Render_element_as_texture */

int Computed_field_find_element_xi_special(struct Computed_field *field, 
	struct Computed_field_find_element_xi_special_cache **cache_ptr,
	FE_value *values, int number_of_values, struct FE_element **element, 
	FE_value *xi, struct GROUP(FE_element) *search_element_group,
	struct User_interface *user_interface,
	float *hint_minimums, float *hint_maximums, float *hint_resolution)
/*******************************************************************************
LAST MODIFIED : 22 June 2000

DESCRIPTION :
This function implements the reverse of some certain computed_fields
(Computed_field_is_find_element_xi_capable) so that it tries to find an element
and xi which would evaluate to the given values.
This implementation of find_element_xi has been separated out as it uses OpenGL
to accelerate the element xi lookup.
The <user_interface> is required to connect to the OpenGL implementation.
The <find_element_xi_data> is passed in just to avoid reimplementing the code
from Computed_field_find_element_xi.
<hint_minimums> and <hint_maximums> are used to indicate the range over which
the values supplied will vary and <hint_resolution> indicates the resolution
at which values will be sampled for element_xi, as this algorithm will generate
an element lookup image using these parameters.
The return code indicates if the algorithm should be relied on or whether a
sequential element_xi lookup should now be performed.
==============================================================================*/
{
#define BLOCK_SIZE (20)
#if defined (DEBUG)
	int dummy[1024 * 1024];
#endif /* defined (DEBUG) */
	unsigned char colour[4], colour_block[BLOCK_SIZE * BLOCK_SIZE][4], *next_colour;
	float ditherx, dithery;
	struct CM_element_information cm;
	struct Computed_field_find_element_xi_special_cache *cache;
	struct Computed_field_iterative_find_element_xi_data find_element_xi_data;
	struct FE_element *first_element;
	struct Render_element_data data;
	int *block_ptr, gl_list, i, nx, ny, px, py, return_code, scaled_number;

	ENTER(Computed_field_find_element_xi_special);
	USE_PARAMETER(number_of_values);
	return_code = 0;
	/* If the number of elements in the group is small then there probably isn't
		any benefit to using this method */
	/* This method is adversely affected when displaying on a remote machine as every
		pixel grab requires transfer across the network */
	if (hint_minimums && hint_maximums && hint_resolution && 
		((2 == Computed_field_get_number_of_components(field)) ||
		((3 == Computed_field_get_number_of_components(field)) &&
		(hint_resolution[2] == 0.0))) && user_interface && search_element_group
		&& (5 < NUMBER_IN_GROUP(FE_element)(search_element_group)))
	{
		find_element_xi_data.field = field;
		find_element_xi_data.values = values;
		find_element_xi_data.number_of_values = number_of_values;
		find_element_xi_data.found_number_of_xi = 0;
		find_element_xi_data.found_derivatives = (FE_value *)NULL;
		find_element_xi_data.tolerance = 1e-06;
		if (ALLOCATE(find_element_xi_data.found_values, FE_value, number_of_values))
		{
			if (*cache_ptr)
			{
				cache = *cache_ptr;
			}
			else
			{
				/* Get the element number range so we can spread the colour range
					as widely as possible */
				if (first_element = FIRST_OBJECT_IN_GROUP_THAT(FE_element)
					((GROUP_CONDITIONAL_FUNCTION(FE_element) *)NULL, NULL, search_element_group))
				{				
					*cache_ptr = CREATE(Computed_field_find_element_xi_special_cache)();
					cache = *cache_ptr;

					cache->maximum_element_number = first_element->cm.number;
					cache->minimum_element_number = first_element->cm.number;
					FOR_EACH_OBJECT_IN_GROUP(FE_element)(Expand_element_range,
						(void *)cache, search_element_group);

					cache->bit_shift = ceil(log((double)(cache->maximum_element_number - 
						cache->minimum_element_number + 2)) / log (2.0));
					cache->bit_shift = (cache->bit_shift / 3) + 1;
					/* 1024 is the limit on an Octane and the limit incorrectly reported by
						the O2 proxy query.  I didn't want to limit the CREATE(Dm_buffer) to
						1024 on an O2 so it doesn't check or handle limits properly so I do 
						it here instead. */
					if (hint_resolution[0] > 1024.0)
					{
						hint_resolution[0] = 1024;
					}
					if (hint_resolution[1] > 1024.0)
					{
						hint_resolution[1] = 1024;
					}
					if (cache->dmbuffer = CREATE(Dm_buffer)(hint_resolution[0], hint_resolution[1],
						/* depth_buffer */0, /*shared_display_buffer*/0, user_interface))
					{
						data.field = field;
						data.bit_shift = cache->bit_shift;
						data.minimum_element_number = cache->minimum_element_number;
						data.maximum_element_number = cache->maximum_element_number;
						Dm_buffer_glx_make_current(cache->dmbuffer);
						glClearColor(0.0, 0.0, 0.0, 0.0);
						glClear(GL_COLOR_BUFFER_BIT);

						glDisable(GL_ALPHA_TEST);
						glDisable(GL_DEPTH_TEST);
						glDisable(GL_SCISSOR_TEST);

						gl_list = glGenLists(1);
						glNewList(gl_list, GL_COMPILE);
						if (gl_list)
						{

							glBegin(GL_QUADS);
							FOR_EACH_OBJECT_IN_GROUP(FE_element)(Render_element_as_texture,
								(void *)&data, search_element_group);
							glEnd();
						
							glEndList();

							/* Dither things around a bit so that we get elements around the edges */
							ditherx = 1.0 / hint_resolution[0];
							dithery = 1.0 / hint_resolution[1];
							glLoadIdentity();
							glOrtho(0.0 - ditherx, 1.0 - ditherx, 0.0 - dithery, 1.0 - dithery,
								-1.0, 1.0);
							glCallList(gl_list);
							glLoadIdentity();
							glOrtho(0.0 + ditherx, 1.0 + ditherx, 0.0 - dithery, 1.0 - dithery,
								-1.0, 1.0);
							glCallList(gl_list);
							glLoadIdentity();
							glOrtho(0.0 - ditherx, 1.0 - ditherx, 0.0 + dithery, 1.0 + dithery,
								-1.0, 1.0);
							glCallList(gl_list);
							glLoadIdentity();
							glOrtho(0.0 + ditherx, 1.0 + ditherx, 0.0 + dithery, 1.0 + dithery,
								-1.0, 1.0);
							glCallList(gl_list);
				
							glLoadIdentity();
							glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
							glCallList(gl_list);

							glDeleteLists(gl_list, 1);

#if defined (DEBUG)
							glReadPixels(values[0] * hint_resolution[0],
								values[1] * hint_resolution[1],
								hint_resolution[0], hint_resolution[1], GL_RGBA, GL_UNSIGNED_BYTE,
								dummy);
				
							write_rgb_image_file("bob.rgb", 4, 1, hint_resolution[1],
								hint_resolution[0], 0, (long unsigned *)dummy);
#endif /* defined (DEBUG) */
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"CREATE(Computed_field_find_element_xi_special).  Unable to make display list.");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"CREATE(Computed_field_find_element_xi_special).  Unable to create offscreen buffer.");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CREATE(Computed_field_find_element_xi_special).  No elements in group.");
				}
			}
			if (cache->dmbuffer)
			{
				Dm_buffer_glx_make_current(cache->dmbuffer);
				glReadPixels(values[0] * hint_resolution[0],
					values[1] * hint_resolution[1],
					1, 1, GL_RGBA, GL_UNSIGNED_BYTE, colour);

				scaled_number = (((unsigned int)colour[0] >> (8 - cache->bit_shift)) 
					<< (2 * cache->bit_shift)) +
					(((unsigned int)colour[1] >> (8 - cache->bit_shift)) 
						<< cache->bit_shift) +
					((unsigned int)colour[2] >> (8 - cache->bit_shift));
				cm.number = scaled_number + cache->minimum_element_number - 1;
				cm.type = CM_ELEMENT;
				if (scaled_number)
				{
					if (*element = FIND_BY_IDENTIFIER_IN_GROUP(FE_element,
						identifier)(&cm, search_element_group))
					{
						first_element = *element;
						find_element_xi_data.tolerance = 1e-06;
						if (Computed_field_iterative_element_conditional(
							*element, (void *)&find_element_xi_data))
						{
							for (i = 0 ; i < find_element_xi_data.found_number_of_xi ; i++)
							{
								xi[i] = find_element_xi_data.xi[i];
							}			
						}
						else
						{
							*element = (struct FE_element *)NULL;
							/* Look in all the surrounding pixels for elements */
							if (values[0] * hint_resolution[0] > BLOCK_SIZE / 2)
							{
								px = values[0] * hint_resolution[0] - BLOCK_SIZE / 2;
							}
							else
							{
								px = 0;
							}
							if (values[1] * hint_resolution[1] > BLOCK_SIZE / 2)
							{
								py = values[1] * hint_resolution[1] - BLOCK_SIZE / 2;
							}
							else
							{
								py = 0;
							}
							if (px + BLOCK_SIZE < hint_resolution[0])
							{
								nx = BLOCK_SIZE;
							}
							else
							{
								nx = hint_resolution[0] - px;
							}
							if (py + BLOCK_SIZE < hint_resolution[0])
							{
								ny = BLOCK_SIZE;
							}
							else
							{
								ny = hint_resolution[0] - py;
							}
							glReadPixels(px, py, nx, ny, GL_RGBA, GL_UNSIGNED_BYTE, 
								colour_block);

							while (scaled_number && (*element == (struct FE_element *)NULL))
							{
								/* OK, so I don't want to assume what the byte order is inside
									an int but I do want to compare these multiple byte blocks
									with a single comparison.  However when I extract a value
									I want to do it byte by byte into the integer */
								block_ptr = ((int *)colour_block) + nx * ny - 1;
								next_colour = (unsigned char *)block_ptr;
								while(block_ptr >= (int *)colour_block)
								{
									if (*block_ptr)
									{
										if (*block_ptr == *((int *)colour))
										{
											*block_ptr = 0;
										}
										else
										{
											next_colour = (unsigned char *)block_ptr;
										}
									}
									block_ptr--;
								}
								colour[0] = next_colour[0];
								colour[1] = next_colour[1];
								colour[2] = next_colour[2];
								colour[3] = next_colour[3];
								scaled_number = (((unsigned int)colour[0] >> (8 - cache->bit_shift)) 
									<< (2 * cache->bit_shift)) +
									(((unsigned int)colour[1] >> (8 - cache->bit_shift)) 
										<< cache->bit_shift) +
									((unsigned int)colour[2] >> (8 - cache->bit_shift));
								cm.number = scaled_number + cache->minimum_element_number - 1;
							
								if (scaled_number)
								{
									if (*element = FIND_BY_IDENTIFIER_IN_GROUP(FE_element,
										identifier)(&cm, search_element_group))
									{
										if (Computed_field_iterative_element_conditional(
											*element, (void *)&find_element_xi_data))
										{
											for (i = 0 ; i < find_element_xi_data.found_number_of_xi ; i++)
											{
												xi[i] = find_element_xi_data.xi[i];
											}			
										}
										else
										{
											*element = (struct FE_element *)NULL;
										}
									}
								}
							}

							if (! *element)
							{
								/* Revert to the originally found element and extrapolate if it close (to allow for boundaries) */
								find_element_xi_data.tolerance = 1.0;
								if (Computed_field_iterative_element_conditional(
									first_element, (void *)&find_element_xi_data))
								{
									*element = first_element;
									for (i = 0 ; i < find_element_xi_data.found_number_of_xi ; i++)
									{
										xi[i] = find_element_xi_data.xi[i];
									}			
								}
							}
						}
					}
				}
				else
				{
					*element = (struct FE_element *)NULL;
				}
				return_code = 1;
				
			}
			else
			{
				/* Do something else again */
			}
			DEALLOCATE(find_element_xi_data.found_values);
			if (find_element_xi_data.found_derivatives)
			{
				DEALLOCATE(find_element_xi_data.found_derivatives);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_find_element_xi_special.  Unable to allocate value memory.");
			return_code = 0;
		}
	}
	LEAVE;

	return (return_code);
} /* Computed_field_find_element_xi_special */

struct Computed_field_find_element_xi_special_cache 
*CREATE(Computed_field_find_element_xi_special_cache)
		 (void)
/*******************************************************************************
LAST MODIFIED : 20 June 2000

DESCRIPTION :
Stores cache data for the Computed_field_find_element_xi_special routine.
==============================================================================*/
{
	struct Computed_field_find_element_xi_special_cache *cache;

	ENTER(CREATE(Computed_field_find_element_xi_special_cache));
	
	if (ALLOCATE(cache,struct Computed_field_find_element_xi_special_cache,1))
	{
	  cache->dmbuffer = (struct Dm_buffer *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Computed_field_find_element_xi_special_cache).  Not enough memory");
		cache = (struct Computed_field_find_element_xi_special_cache *)NULL;
	}
	LEAVE;

	return (cache);
} /* CREATE(Computed_field_find_element_xi_special_cache) */

int DESTROY(Computed_field_find_element_xi_special_cache)
	  (struct Computed_field_find_element_xi_special_cache **cache_address)
/*******************************************************************************
LAST MODIFIED : 20 June 2000

DESCRIPTION :
Frees memory/deaccess cache at <*cache_address>.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Computed_field_find_element_xi_special_cache));
	if (cache_address&&*cache_address)
	{
		DESTROY(Dm_buffer)(&(*cache_address)->dmbuffer);
		DEALLOCATE(*cache_address);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Computed_field_find_element_xi_special_cache).  Missing cache");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Computed_field_find_element_xi_special_cache) */


