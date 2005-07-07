/*******************************************************************************
FILE : computed_field_find_xi.c

LAST MODIFIED : 9 January 2003

DESCRIPTION :
Implements a special version of find_xi that uses OpenGL to accelerate the
lookup of the element.
==============================================================================*/
#include <stdio.h>
#include <math.h>
#include <GL/gl.h>

#include "general/debug.h"
#include "general/image_utilities.h"
#include "general/matrix_vector.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_find_xi.h"
#include "finite_element/finite_element_discretization.h"
#include "finite_element/finite_element_region.h"
#include "graphics/texture.h"
#include "three_d_drawing/graphics_buffer.h"
#include "user_interface/message.h"

struct Render_element_data
{
	int bit_shift;
	int minimum_element_number;
	int maximum_element_number;
	struct Computed_field *field;
	FE_value values[3];
};

struct Computed_field_find_element_xi_cache
{
#if defined (GRAPHICS_BUFFER_USE_OFFSCREEN_BUFFERS)
	int bit_shift;
	int minimum_element_number;
	int maximum_element_number;
	struct Graphics_buffer *graphics_buffer;
#endif /* defined (GRAPHICS_BUFFER_USE_OFFSCREEN_BUFFERS) */
	int valid_values;
	struct FE_element *element;
	struct Cmiss_region *search_region;
	int number_of_values;
	FE_value *values;
	FE_value *working_values;
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	/* Warn when trying to destroy this cache as it is being filled in */
	int in_perform_find_element_xi;
};

struct Computed_field_iterative_find_element_xi_data
/*******************************************************************************
LAST MODIFIED: 21 August 2002

DESCRIPTION:
Data for passing to Computed_field_iterative_element_conditional
Important note:
The <values> passed in this structure must not be a pointer to values
inside a field cache otherwise they may be overwritten if that field
matches the <field> in this structure or one of its source fields.
==============================================================================*/
{
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct Computed_field *field;
	int number_of_values;
	FE_value *values;
	int element_dimension;
	int found_number_of_xi;
	FE_value *found_values;
	FE_value *found_derivatives;
	float tolerance;
	int find_nearest_location;
	struct FE_element *nearest_element;
	double nearest_element_distance_squared;
}; /* Computed_field_iterative_find_element_xi_data */

#define MAX_FIND_XI_ITERATIONS 10

static int Computed_field_iterative_element_conditional(
	struct FE_element *element, void *data_void)
/*******************************************************************************
LAST MODIFIED : 21 August 2002

DESCRIPTION :
Returns true if a valid element xi is found.
Important note:
The <values> passed in the <data> structure must not be a pointer to values
inside a field cache otherwise they may be overwritten if the field is the same
as the <data> field or any of its source fields.
==============================================================================*/
{
	double a[MAXIMUM_ELEMENT_XI_DIMENSIONS*MAXIMUM_ELEMENT_XI_DIMENSIONS],
		b[MAXIMUM_ELEMENT_XI_DIMENSIONS], d, sum;
	FE_value *derivatives, last_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS], *values;
	int converged, i, indx[MAXIMUM_ELEMENT_XI_DIMENSIONS], iterations, j, k,
		number_of_xi, number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS], 
		number_of_xi_points_created[MAXIMUM_ELEMENT_XI_DIMENSIONS], return_code;
	struct Computed_field_iterative_find_element_xi_data *data;
	struct FE_element_shape *shape;
	Triple *xi_points;

	ENTER(Computed_field_iterative_element_conditional);
	if (element && (data = (struct Computed_field_iterative_find_element_xi_data *)data_void))
	{
		number_of_xi = get_FE_element_dimension(element);
		/* Filter out the elements we consider.  All elements are valid if data->element_dimension
			is zero, otherwise the element dimensions must match */
		if ((!data->element_dimension) || (number_of_xi == data->element_dimension))
		{
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
							"Computed_field_iterative_element_conditional.  "
							"Unable to allocate derivative storage");
						return_code = 0;
					}
				}
				if (return_code)
				{
					values = data->found_values;
					derivatives = data->found_derivatives;
					/* Find a good estimate of the centre to start with */
					for (i = 0 ; i < number_of_xi ; i++)
					{
						/* I want just one location in the middle */
						number_in_xi[i] = 1;
					}
					if (get_FE_element_shape(element, &shape) &&
						FE_element_shape_get_xi_points_cell_centres(shape, 
						number_in_xi, number_of_xi_points_created, &xi_points))
					{
						for (i = 0 ; i < number_of_xi ; i++)
						{
							data->xi[i] = xi_points[0][i];
							last_xi[i] = xi_points[0][i];
						}
						DEALLOCATE(xi_points);
					}
					else
					{
						for (i = 0 ; i < number_of_xi ; i++)
						{
							data->xi[i] = 0.5;
							last_xi[i] = 0.5;
						}
					}
					converged = 0;
					iterations = 0;
					while ((!converged) && return_code)
					{
						if (Computed_field_evaluate_in_element(data->field, element, data->xi,
							/*time*/0, (struct FE_element *)NULL, values, derivatives))
						{
							/* least-squares approach: make the derivatives / right hand side
								vector into square system to solve for delta-Xi */
							for (i = 0; i < number_of_xi; i++)
							{
								for (j = 0; j < number_of_xi; j++)
								{
									sum = 0.0;
									for (k = 0; k < data->number_of_values; k++)
									{
										sum += (double)derivatives[k*number_of_xi + j] *
											(double)derivatives[k*number_of_xi + i];
									}
									a[i*number_of_xi + j] = sum;
								}
								sum = 0.0;
								for (k = 0; k < data->number_of_values; k++)
								{
									sum += (double)derivatives[k*number_of_xi + i] *
										((double)data->values[k] - (double)values[k]);
								}
								b[i] = sum;
							}
							if (LU_decompose(number_of_xi, a, indx, &d,/*singular_tolerance*/1.0e-12) &&
								LU_backsubstitute(number_of_xi, a, indx, b))
							{
								converged = 1;
								for (i = 0; i < number_of_xi; i++)
								{
									/* converged if all xi increments on or within tolerance */
									if (fabs(b[i]) > data->tolerance)
									{
										converged = 0;
									}
									data->xi[i] += b[i];
								}
								iterations++;
								if (!converged)
								{
									FE_element_shape_limit_xi_to_element(shape,
										data->xi, data->tolerance);
									if (iterations == MAX_FIND_XI_ITERATIONS)
									{
										/* too many iterations; give up */
										return_code = 0;
									}
									else if (1 < iterations)
									{
										/* give up if xi not changed this iteration; usually means
											the solution is outside this element */
										return_code = 0;
										for (i = 0; i < number_of_xi; i++)
										{
											if (data->xi[i] != last_xi[i])
											{
												return_code = 1;
											}
											last_xi[i] = data->xi[i];
										}
									}
								}
							}
							else
							{
								/* Probably singular matrix, no longer report error
									as a collapsed element gets reported on every
									pixel making it unusable.*/
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_iterative_element_conditional.  "
								"Could not evaluate field");
							return_code = 0;
						}
					}
					/* if field has more components than xi-directions, must
						check all components have converged */
					if (converged && (data->number_of_values > number_of_xi))
					{
						/* strategy for determining convergence is to multiply
							the derivative for each component by the last increment
							in xi, stored in b, and ensuring it is larger than the
							difference between target and current field values.
							Broadly, this means the given component is not wishing
							the solution to shift more than the last xi increment.
							Note that the increment in xi is doubled to make this
							test slightly less strict than original convergence */
						for (k = 0; k < data->number_of_values; k++)
						{
							sum = 0.0;
							for (i = 0; i < number_of_xi; i++)
							{
								sum += (double)derivatives[k*number_of_xi + i] * b[i];
							}
							if (2.0*fabs(sum) <
								fabs((double)data->values[k] - (double)values[k]))
							{
								return_code = 0;
							}
						}
					}
					if (data->find_nearest_location)
					{
						sum = 0.0;
						for (k = 0; k < data->number_of_values; k++)
						{
							sum += ((double)data->values[k] - (double)values[k]) *
								((double)data->values[k] - (double)values[k]);
						}
						if (!data->nearest_element || 
							(sum < data->nearest_element_distance_squared))
						{
							data->nearest_element = element;
							data->nearest_element_distance_squared = sum;
						}
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_iterative_element_conditional.  "
					"Unable to solve underdetermined system");
				return_code = 0;
			}
		}
		else
		{
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

	return (return_code);
} /* Computed_field_iterative_element_conditional */

#undef MAX_FIND_XI_ITERATIONS

int Computed_field_perform_find_element_xi(struct Computed_field *field,
	FE_value *values, int number_of_values, struct FE_element **element, 
	FE_value *xi, int element_dimension, struct Cmiss_region *search_region,
	int find_nearest_location)
/*******************************************************************************
LAST MODIFIED : 18 April 2005

DESCRIPTION :
This function actually seacrches through the elements in the 
<search_element_group> trying to find an <xi> location which returns the correct
<values>.  This routine is either called directly by Computed_field_find_element_xi
or if that field is propogating it's values backwards, it is called by the 
ultimate parent finite_element field.
==============================================================================*/
{
	struct Computed_field_find_element_xi_cache *cache;
	struct Computed_field_iterative_find_element_xi_data find_element_xi_data;
	struct FE_region *fe_region;
	int i, number_of_xi, return_code;

	ENTER(Computed_field_perform_find_element_xi);
	fe_region = (struct FE_region *)NULL;
	if (field && values && (number_of_values == field->number_of_components) &&
		element && xi && ((search_region && (fe_region = Cmiss_region_get_FE_region(search_region)))
		|| *element))
	{
		return_code = 1;
		/* clear the cache if values already cached for a node as we are about to
		   evaluate for elements and if the field is caching for elements it will
		   automatically clear the cache, destroying the find_element_xi_cache it 
			is in the process of filling in. */
		if (field->node)
		{
			Computed_field_clear_cache(field);
		}
		if (field->find_element_xi_cache)
		{
			cache = field->find_element_xi_cache;
			if (cache->number_of_values != number_of_values)
			{
				cache->valid_values = 0;
				DEALLOCATE(cache->values);
				DEALLOCATE(cache->working_values);
			}
			if (cache->element &&
				(element_dimension != get_FE_element_dimension(cache->element)))
			{
				cache->valid_values = 0;
			}
			if (cache->valid_values)
			{
				if (search_region)
				{
					if (cache->search_region != search_region)
					{
						cache->valid_values = 0;
					}
					if (cache->element && 
						!FE_region_contains_FE_element(fe_region, cache->element))
					{
						cache->valid_values = 0;
					}
				}
				else
				{
					if (cache->element != *element)
					{
						cache->valid_values = 0;
					}
				}
			}
			if (cache->valid_values)
			{
				for (i = 0; i < number_of_values; i++)
				{
					if (cache->values[i] != values[i])
					{
 						cache->valid_values = 0;
					}
				}
			}
			else
			{
				cache->valid_values = 0;
			}
		}
		else
		{
			if (field->find_element_xi_cache =
				CREATE(Computed_field_find_element_xi_cache)())
			{
				cache = field->find_element_xi_cache;
			}
			else
			{
				return_code = 0;
			}
		}
		cache->in_perform_find_element_xi = 1;
		if (return_code && !cache->values)
		{
			cache->number_of_values = number_of_values;
			if (!ALLOCATE(cache->values, FE_value,
					 number_of_values))
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_perform_find_element_xi.  "
					"Unable to allocate value memory.");
				return_code = 0;
			}
		}
		if (return_code && !cache->working_values)
		{
			if (!ALLOCATE(cache->working_values, FE_value,
					 number_of_values))
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_perform_find_element_xi.  "
					"Unable to allocate working value memory.");
				return_code = 0;
			}
		}
		if (return_code)
		{
			if (cache->valid_values)
			{
				/* This could even be valid if *element is NULL */
				*element = cache->element;

				if (*element)
				{
					number_of_xi = get_FE_element_dimension(*element);
					for (i = 0 ; i < number_of_xi ; i++)
					{
						xi[i] = cache->xi[i];
					}
				}
			}
			else
			{
				find_element_xi_data.values = cache->values;
				find_element_xi_data.element_dimension = element_dimension; 
				find_element_xi_data.found_values = 
					cache->working_values;
				/* copy the source values */
				for (i = 0; i < number_of_values; i++)
				{
					find_element_xi_data.values[i] = values[i];
				}
				find_element_xi_data.field = field;
				find_element_xi_data.number_of_values = number_of_values;
				find_element_xi_data.found_number_of_xi = 0;
				find_element_xi_data.found_derivatives = (FE_value *)NULL;
				find_element_xi_data.tolerance = 1e-06;
				find_element_xi_data.find_nearest_location = find_nearest_location;
				find_element_xi_data.nearest_element = (struct FE_element *)NULL;
				find_element_xi_data.nearest_element_distance_squared = 0.0;

				if (search_region)
				{
					*element = (struct FE_element *)NULL;

					/* Try the cached element first if it is in the group */
					if (field->element && FE_region_contains_FE_element
						(fe_region, field->element) &&
						Computed_field_iterative_element_conditional(
							field->element, (void *)&find_element_xi_data))
					{
						*element = field->element;
					}
					/* Now try every element */
					if (!*element)
					{
						*element = FE_region_get_first_FE_element_that
							(fe_region, Computed_field_iterative_element_conditional,
								&find_element_xi_data);
					}
				}
				else
				{
					if ((!Computed_field_iterative_element_conditional(
							  *element, (void *)&find_element_xi_data)))
					{
						*element = (struct FE_element *)NULL;
					}
				}
				/* If an exact match is not found then accept the closest one */
				if (!*element && find_nearest_location)
				{
					*element = find_element_xi_data.nearest_element;
				}
				if (*element)
				{
					number_of_xi = get_FE_element_dimension(*element);
					for (i = 0 ; i < number_of_xi ; i++)
					{
						xi[i] = find_element_xi_data.xi[i];
					}
				}
				/* The search is valid even if the element wasn't found */
				return_code = 1;
				if (find_element_xi_data.found_derivatives)
				{
					DEALLOCATE(find_element_xi_data.found_derivatives);
				}
				/* Copy the results into the cache */
				if (cache->element = *element)
				{
					for (i = 0 ; i < number_of_xi ; i++)
					{
						cache->xi[i] = find_element_xi_data.xi[i];
					}
				}
				else
				{
					/* No element; clear xi to zero */
					for (i = 0 ; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
					{
						cache->xi[i] = 0.0;
					}
				}
				cache->search_region = search_region;
				cache->valid_values = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_perform_find_element_xi.  "
				"Unable to allocate value memory.");
			return_code = 0;
		}	
		cache->in_perform_find_element_xi = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_perform_find_element_xi.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_perform_find_element_xi */

#if defined (GRAPHICS_BUFFER_USE_OFFSCREEN_BUFFERS)
static int Expand_element_range(struct FE_element *element, void *data_void)
/*******************************************************************************
LAST MODIFIED : 26 June 2000

DESCRIPTION :
Stores cache data for the Computed_field_find_element_xi_special routine.
==============================================================================*/
{
	int return_code;
	struct Computed_field_find_element_xi_cache *data;
	struct CM_element_information cm_information;
	
	ENTER(Expand_element_range);
	
	if (data = (struct Computed_field_find_element_xi_cache *)data_void)
	{
		/* Expand the element number range */
		if (get_FE_element_identifier(element, &cm_information) &&
			(cm_information.type == CM_ELEMENT) && (2 == get_FE_element_dimension(element)))
		{
			if (cm_information.number > data->maximum_element_number)
			{
				data->maximum_element_number = cm_information.number;
			}
			if (cm_information.number < data->minimum_element_number)
			{
				data->minimum_element_number = cm_information.number;
			}			
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Expand_element_range.  Missing Computed_field_find_element_xi_cache data.");
		return_code = 0;
	}
	LEAVE;
	
	return (return_code);
} /* Expand_element_range */
#endif /* defined (GRAPHICS_BUFFER_USE_OFFSCREEN_BUFFERS) */

#if defined (GRAPHICS_BUFFER_USE_OFFSCREEN_BUFFERS)
static int Render_element_as_texture(struct FE_element *element, void *data_void)
/*******************************************************************************
LAST MODIFIED : 20 June 2000

DESCRIPTION :
Stores cache data for the Computed_field_find_element_xi_special routine.
==============================================================================*/
{
	enum FE_element_shape_type shape_type;
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	float red, green, blue;
	int return_code;
	struct CM_element_information cm_information;
	struct FE_element_shape *shape;
	struct Render_element_data *data;
 	unsigned int scaled_number;
	
	ENTER(Render_element_as_texture);
	
	if (data = (struct Render_element_data *)data_void)
	{
		/* Code the element number into the colour */
		if (get_FE_element_identifier(element, &cm_information) &&
			(cm_information.type == CM_ELEMENT) && (2 == get_FE_element_dimension(element)))
		{
			scaled_number = ((double)cm_information.number - data->minimum_element_number + 1);
			blue = ((double)((scaled_number & ((1 << data->bit_shift) - 1)) << 
				(8 - data->bit_shift)) + 0.5) / 255.0;
			scaled_number = scaled_number >> data->bit_shift;
			green = ((double)((scaled_number & ((1 << data->bit_shift) - 1)) << 
				(8 - data->bit_shift)) + 0.5) / 255.0;
			scaled_number = scaled_number >> data->bit_shift;
			red = ((double)((scaled_number & ((1 << data->bit_shift) - 1)) << 
				(8 - data->bit_shift)) + 0.5) / 255.0;

			glColor3f(red, green, blue);
		  
#if defined (DEBUG)
			printf ( "%d %lf %lf %lf\n",  cm_information.number, red, green, blue);
#endif /* defined (DEBUG) */

			xi[0] = 0.0;
			xi[1] = 0.0;
			Computed_field_evaluate_in_element(data->field, element, xi,
				/*time*/0,(struct FE_element *)NULL, data->values, (FE_value *)NULL);
			glVertex2fv(data->values);
			xi[0] = 1.0;
			xi[1] = 0.0;
			Computed_field_evaluate_in_element(data->field, element, xi,
				/*time*/0,(struct FE_element *)NULL, data->values, (FE_value *)NULL);
			glVertex2fv(data->values);

			/* Only need to check the first dimension as this is only working for 2D */
			if (get_FE_element_shape(element, &shape) &&
				get_FE_element_shape_xi_shape_type(shape,
				/*xi_number*/0,  &shape_type) && 
				(SIMPLEX_SHAPE == shape_type))
			{
				xi[0] = 0.0;
				xi[1] = 1.0;
				Computed_field_evaluate_in_element(data->field, element, xi,
					/*time*/0,(struct FE_element *)NULL, data->values, (FE_value *)NULL);
				glVertex2fv(data->values);
				glVertex2fv(data->values);
			}
			else
			{
				xi[0] = 1.0;
				xi[1] = 1.0;
				Computed_field_evaluate_in_element(data->field, element, xi,
					/*time*/0,(struct FE_element *)NULL, data->values, (FE_value *)NULL);
				glVertex2fv(data->values);
				
				xi[0] = 0.0;
				xi[1] = 1.0;
				Computed_field_evaluate_in_element(data->field, element, xi,
					/*time*/0,(struct FE_element *)NULL, data->values, (FE_value *)NULL);
				glVertex2fv(data->values);
			}
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Render_element_as_texture.  Missing Computed_field_find_element_xi_cache data.");
		return_code = 0;
	}
	LEAVE;
	
	return (return_code);
} /* Render_element_as_texture */
#endif /* defined (GRAPHICS_BUFFER_USE_OFFSCREEN_BUFFERS) */

int Computed_field_find_element_xi_special(struct Computed_field *field, 
	struct Computed_field_find_element_xi_cache **cache_ptr,
	FE_value *values, int number_of_values, struct FE_element **element, 
	FE_value *xi, struct Cmiss_region *search_region,
	int element_dimension,
	struct Graphics_buffer_package *graphics_buffer_package,
	float *hint_minimums, float *hint_maximums, float *hint_resolution)
/*******************************************************************************
LAST MODIFIED : 12 May 2004

DESCRIPTION :
This function implements the reverse of some certain computed_fields
(Computed_field_is_find_element_xi_capable) so that it tries to find an element
and xi which would evaluate to the given values.
This implementation of find_element_xi has been separated out as it uses OpenGL
to accelerate the element xi lookup.
The <graphics_buffer_package> is required to connect to the OpenGL implementation.
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
	int return_code;
#if defined (GRAPHICS_BUFFER_USE_OFFSCREEN_BUFFERS)
#define BLOCK_SIZE (20)
#if defined (DEBUG)
	int dummy[1024 * 1024];
#endif /* defined (DEBUG) */
	unsigned char *block_ptr, colour[4], colour_block[BLOCK_SIZE * BLOCK_SIZE *4],
		*next_colour;
	float ditherx, dithery;
	struct CM_element_information cm;
	struct Computed_field_find_element_xi_cache *cache;
	struct Computed_field_iterative_find_element_xi_data find_element_xi_data;
	struct FE_element *first_element;
	struct FE_region *fe_region;
	struct Render_element_data data;
	int gl_list, i, nx, ny, px, py, scaled_number;
#endif /* defined (GRAPHICS_BUFFER_USE_OFFSCREEN_BUFFERS) */

	ENTER(Computed_field_find_element_xi_special);
	USE_PARAMETER(number_of_values);
#if !defined (GRAPHICS_BUFFER_USE_OFFSCREEN_BUFFERS)
	USE_PARAMETER(field);
	USE_PARAMETER(cache_ptr);
	USE_PARAMETER(values);
	USE_PARAMETER(element);
	USE_PARAMETER(element_dimension);
	USE_PARAMETER(xi);
	USE_PARAMETER(search_region);
	USE_PARAMETER(graphics_buffer_package);
	USE_PARAMETER(hint_minimums);
	USE_PARAMETER(hint_maximums);
	USE_PARAMETER(hint_resolution);
#endif /* !defined (DM_BUFFERS) */

	return_code = 0;
#if defined (GRAPHICS_BUFFER_USE_OFFSCREEN_BUFFERS)
	/* If the number of elements in the group is small then there probably isn't
		any benefit to using this method */
	/* This method is adversely affected when displaying on a remote machine as every
		pixel grab requires transfer across the network */
	if (hint_minimums && hint_maximums && hint_resolution && 
		((2 == Computed_field_get_number_of_components(field)) ||
		((3 == Computed_field_get_number_of_components(field)) &&
		(hint_resolution[2] == 1.0f))) && graphics_buffer_package && search_region &&
		/* At some point we may want to search in any FE_regions below the search_region */
		(fe_region = Cmiss_region_get_FE_region(search_region))
		/* This special case actually only works for 2D elements */
		&& ((element_dimension == 0) || (element_dimension == 2))
		&& (5 < FE_region_get_number_of_FE_elements(fe_region))
		/*&& (Computed_field_is_find_element_xi_capable(field,NULL))*/)
	{
		find_element_xi_data.field = field;
		find_element_xi_data.values = values;
		find_element_xi_data.number_of_values = number_of_values;
		find_element_xi_data.element_dimension = element_dimension;
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
				if (first_element = FE_region_get_first_FE_element_that(fe_region,
					(LIST_CONDITIONAL_FUNCTION(FE_element) *)NULL, NULL))
				{				
					*cache_ptr = CREATE(Computed_field_find_element_xi_cache)();
					cache = *cache_ptr;

					get_FE_element_identifier(first_element, &cm);
					cache->maximum_element_number = cm.number;
					cache->minimum_element_number = cm.number;
					FE_region_for_each_FE_element(fe_region, Expand_element_range,
						(void *)cache);

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
					if (cache->graphics_buffer = create_Graphics_buffer_offscreen(
						graphics_buffer_package, hint_resolution[0], hint_resolution[1],
						GRAPHICS_BUFFER_ANY_BUFFERING_MODE, GRAPHICS_BUFFER_ANY_STEREO_MODE,
						/*minimum_colour_buffer_depth*/0, /*minimum_depth_buffer_depth*/0,
						/*minimum_accumulation_buffer_depth*/0))
					{
						data.field = field;
						data.bit_shift = cache->bit_shift;
						data.minimum_element_number = cache->minimum_element_number;
						data.maximum_element_number = cache->maximum_element_number;
						Graphics_buffer_make_current(cache->graphics_buffer);
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
							FE_region_for_each_FE_element(fe_region, Render_element_as_texture,
								(void *)&data);
							glEnd();
						
							glEndList();

							/* Dither things around a bit so that we get elements around the edges */
							ditherx = (hint_maximums[0] - hint_minimums[0]) / hint_resolution[0];
							dithery = (hint_maximums[1] - hint_minimums[1]) / hint_resolution[1];
							glLoadIdentity();
							glOrtho(hint_minimums[0] - ditherx, hint_maximums[0] - ditherx,
							        hint_minimums[1] - dithery, hint_maximums[1] - dithery,
							        -1.0, 1.0);
							glCallList(gl_list);
							glLoadIdentity();
							glOrtho(hint_minimums[0] + ditherx, hint_maximums[0] + ditherx,
							        hint_minimums[1] - dithery, hint_maximums[1] - dithery,
							        -1.0, 1.0);
							glCallList(gl_list);
							glLoadIdentity();
							glOrtho(hint_minimums[0] - ditherx, hint_maximums[0] - ditherx,
							        hint_minimums[1] + dithery, hint_maximums[1] + dithery,
							        -1.0, 1.0);
							glCallList(gl_list);
							glLoadIdentity();
							glOrtho(hint_minimums[0] + ditherx, hint_maximums[0] + ditherx,
							        hint_minimums[1] + dithery, hint_maximums[1] + dithery,
							        -1.0, 1.0);
							glCallList(gl_list);
				
							glLoadIdentity();
							glOrtho(hint_minimums[0], hint_maximums[0],
							        hint_minimums[1], hint_maximums[1],
							        -1.0, 1.0);
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
			if (cache->graphics_buffer)
			{
				Graphics_buffer_make_current(cache->graphics_buffer);
				px = (int)((values[0] - hint_minimums[0]) * hint_resolution[0] /
					(hint_maximums[0] - hint_minimums[0]));
				py = (int)((values[1] - hint_minimums[1]) * hint_resolution[1] /
					(hint_maximums[1] - hint_minimums[1]));
				glReadPixels(px, py, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, colour);
				scaled_number = (((unsigned int)colour[0] >> (8 - cache->bit_shift)) 
					<< (2 * cache->bit_shift)) +
					(((unsigned int)colour[1] >> (8 - cache->bit_shift)) 
						<< cache->bit_shift) +
					((unsigned int)colour[2] >> (8 - cache->bit_shift));
				cm.number = scaled_number + cache->minimum_element_number - 1;
				cm.type = CM_ELEMENT;
				if (scaled_number)
				{
					if (*element = FE_region_get_FE_element_from_identifier(
						fe_region, &cm))
					{
						first_element = *element;
#if defined (DEBUG)
						printf("First element %d\n", first_element->cm.number);
#endif /* defined (DEBUG) */
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
							if (px > BLOCK_SIZE / 2)
							{
								px -= BLOCK_SIZE / 2;
							}
							else
							{
								px = 0;
							}
							if (py > BLOCK_SIZE / 2)
							{
								py -= BLOCK_SIZE / 2;
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
							if (py + BLOCK_SIZE < hint_resolution[1])
							{
								ny = BLOCK_SIZE;
							}
							else
							{
								ny = hint_resolution[1] - py;
							}
							glReadPixels(px, py, nx, ny, GL_RGBA, GL_UNSIGNED_BYTE, 
								colour_block);

							while (scaled_number && (*element == (struct FE_element *)NULL))
							{
								/* each different colour represents an element to search in.
									 Following only checks each colour/value once and clears
									 that colour in the next loop. Continues until either an
									 element is found or all pixels in the block are black */
								block_ptr = colour_block + (nx * ny - 1)*4;
								next_colour = block_ptr;
								while (block_ptr >= colour_block)
								{
									if (block_ptr[0] || block_ptr[1] ||
										block_ptr[2] || block_ptr[3])
									{
										if ((block_ptr[0] == colour[0]) &&
											(block_ptr[1] == colour[1]) &&
											(block_ptr[2] == colour[2]) &&
											(block_ptr[3] == colour[3]))
										{
											block_ptr[0] = 0;
											block_ptr[1] = 0;
											block_ptr[2] = 0;
											block_ptr[3] = 0;
										}
										else
										{
											next_colour = block_ptr;
										}
									}
									block_ptr -= 4;
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
									if (*element = FE_region_get_FE_element_from_identifier(
										fe_region, &cm))
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
#if defined (DEBUG)
						if (*element)
						{
							printf("Final found element %d  %f %f %f\n", (*element)->cm.number,
								xi[0], xi[1], xi[2]);
						}
#endif /* defined (DEBUG) */
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
#endif /* defined (GRAPHICS_BUFFER_USE_OFFSCREEN_BUFFERS) */
	LEAVE;

	return (return_code);
} /* Computed_field_find_element_xi_special */

struct Computed_field_find_element_xi_cache 
*CREATE(Computed_field_find_element_xi_cache)(void)
/*******************************************************************************
LAST MODIFIED : 17 December 2002

DESCRIPTION :
Stores cache data for the find_xi routines.
==============================================================================*/
{
	struct Computed_field_find_element_xi_cache *cache;

	ENTER(CREATE(Computed_field_find_element_xi_cache));
	
	if (ALLOCATE(cache,struct Computed_field_find_element_xi_cache,1))
	{
#if defined (GRAPHICS_BUFFER_USE_OFFSCREEN_BUFFERS)
	  cache->graphics_buffer = (struct Graphics_buffer *)NULL;
#endif /* defined (GRAPHICS_BUFFER_USE_OFFSCREEN_BUFFERS) */
	  cache->valid_values = 0;
	  cache->number_of_values = 0;
	  cache->values = (FE_value *)NULL;
	  cache->working_values = (FE_value *)NULL;
	  cache->in_perform_find_element_xi = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Computed_field_find_element_xi_cache).  Not enough memory");
		cache = (struct Computed_field_find_element_xi_cache *)NULL;
	}
	LEAVE;

	return (cache);
} /* CREATE(Computed_field_find_element_xi_cache) */

int DESTROY(Computed_field_find_element_xi_cache)
	(struct Computed_field_find_element_xi_cache **cache_address)
/*******************************************************************************
LAST MODIFIED : 20 June 2000

DESCRIPTION :
Frees memory/deaccess cache at <*cache_address>.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Computed_field_find_element_xi_cache));
	if (cache_address&&*cache_address)
	{
		if ((*cache_address)->in_perform_find_element_xi)
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Computed_field_find_element_xi_cache).  "
				"This cache cannot be destroyed.");
		}
		else
		{
#if defined (GRAPHICS_BUFFER_USE_OFFSCREEN_BUFFERS)
			if ((*cache_address)->graphics_buffer)
			{
				DESTROY(Graphics_buffer)(&(*cache_address)->graphics_buffer);
			}
#endif /* defined (GRAPHICS_BUFFER_USE_OFFSCREEN_BUFFERS) */
			if ((*cache_address)->values)
			{
				DEALLOCATE((*cache_address)->values);
			}
			if ((*cache_address)->working_values)
			{
				DEALLOCATE((*cache_address)->working_values);
			}
			DEALLOCATE(*cache_address);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Computed_field_find_element_xi_cache).  Missing cache");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Computed_field_find_element_xi_cache) */
