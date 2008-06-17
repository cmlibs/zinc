/*******************************************************************************
FILE : computed_field_find_xi.c

LAST MODIFIED : 24 August 2006

DESCRIPTION :
Implements a special version of find_xi that uses OpenGL to accelerate the
lookup of the element.
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

extern "C" {
#include <stdio.h>
#include <math.h>
}

extern "C" {
#include "general/debug.h"
#include "general/matrix_vector.h"
#include "computed_field/computed_field.h"
}
#include "computed_field/computed_field_private.hpp"
extern "C" {
#include "computed_field/computed_field_find_xi.h"
}
#include "computed_field/computed_field_find_xi_private.hpp"
extern "C" {
#include "finite_element/finite_element_discretization.h"
#include "finite_element/finite_element_region.h"
#include "user_interface/message.h"
}

#define MAX_FIND_XI_ITERATIONS 50

int Computed_field_iterative_element_conditional(
	struct FE_element *element, void *data_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

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
					get_FE_element_shape(element, &shape);
					if (data->start_with_data_xi)
					{
						for (i = 0 ; i < number_of_xi ; i++)
						{
							last_xi[i] = data->xi[i];
						}
					}
					else
					{
						/* Find a good estimate of the centre to start with */
						for (i = 0 ; i < number_of_xi ; i++)
						{
							/* I want just one location in the middle */
							number_in_xi[i] = 1;
						}
						if (FE_element_shape_get_xi_points_cell_centres(shape, 
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
											if (fabs(data->xi[i] - last_xi[i]) > data->tolerance)
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
							for (i = 0; i < number_of_xi; i++)
							{
								data->nearest_xi[i] = data->xi[i];
							}
							data->nearest_element_distance_squared = sum;
						}
					}
#if defined (DEBUG)
					display_message(INFORMATION_MESSAGE,
						"Computed_field_iterative_element_conditional.  "
						"Converged %d iterations %d\n", converged, iterations);
#endif /* defined (DEBUG) */

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
LAST MODIFIED : 24 August 2006

DESCRIPTION :
This function actually seacrches through the elements in the 
<search_element_group> trying to find an <xi> location which returns the correct
<values>.  This routine is either called directly by Computed_field_find_element_xi
or if that field is propogating it's values backwards, it is called by the 
ultimate parent finite_element field.
==============================================================================*/
{
	Computed_field_find_element_xi_base_cache *cache;
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
		cache = (Computed_field_find_element_xi_base_cache*)NULL;
		if (field->find_element_xi_cache && field->find_element_xi_cache->cache_data)
		{
			cache = field->find_element_xi_cache->cache_data;
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
				CREATE(Computed_field_find_element_xi_cache)(new Computed_field_find_element_xi_base_cache()))
			{
				cache = field->find_element_xi_cache->cache_data;
			}
			else
			{
				return_code = 0;
			}
		}
		if (cache)
		{
			cache->in_perform_find_element_xi = 1;
		}
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
				find_element_xi_data.tolerance = 1e-05;
				find_element_xi_data.find_nearest_location = find_nearest_location;
				find_element_xi_data.nearest_element = (struct FE_element *)NULL;
				find_element_xi_data.nearest_element_distance_squared = 0.0;
				find_element_xi_data.start_with_data_xi = 0;

				if (search_region)
				{
					*element = (struct FE_element *)NULL;

					/* Try the cached element first if it is in the group */
					if (cache->element && FE_region_contains_FE_element
						(fe_region, cache->element))
					{
						/* Start with the xi that worked before too */
						number_of_xi = get_FE_element_dimension(cache->element);
						for (i = 0 ; i < number_of_xi ; i++)
						{
							find_element_xi_data.xi[i] = cache->xi[i];
						}
						find_element_xi_data.start_with_data_xi = 1;
						if (Computed_field_iterative_element_conditional(
							cache->element, (void *)&find_element_xi_data))
						{
							*element = cache->element;
						}
						find_element_xi_data.start_with_data_xi = 0;
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
					number_of_xi = get_FE_element_dimension(*element);
					for (i = 0 ; i < number_of_xi ; i++)
					{
						xi[i] = find_element_xi_data.nearest_xi[i];
					}
				}
				else if (*element)
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
		if (cache)
		{
			cache->in_perform_find_element_xi = 0;
		}
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

struct Computed_field_find_element_xi_cache
	*CREATE(Computed_field_find_element_xi_cache)(
		Computed_field_find_element_xi_base_cache *cache_data)
/*******************************************************************************
LAST MODIFIED : 13 June 2008

DESCRIPTION :
Stores cache data for find_element_xi routines.
The new object takes ownership of the <cache_data>.
==============================================================================*/
{
	struct Computed_field_find_element_xi_cache *cache;

	ENTER(CREATE(Computed_field_find_element_xi_cache));
	cache = (struct Computed_field_find_element_xi_cache *)NULL;
	if (cache_data)
	{
		if (ALLOCATE(cache,struct Computed_field_find_element_xi_cache,1))
		{
			cache->cache_data = cache_data;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Computed_field_find_element_xi_cache).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Computed_field_find_element_xi_cache).  Invalid argument(s)");
	}
	LEAVE;

	return (cache);
} /* CREATE(Computed_field_find_element_xi_cache) */

int DESTROY(Computed_field_find_element_xi_cache)
	(struct Computed_field_find_element_xi_cache **cache_address)
/*******************************************************************************
LAST MODIFIED : 13 June 2008

DESCRIPTION :
Frees memory/deaccess cache at <*cache_address>.
==============================================================================*/
{
	int return_code;
	class Computed_field_find_element_xi_base_cache

	ENTER(DESTROY(Computed_field_find_element_xi_cache));
	if (cache_address&&*cache_address)
	{
		if (((*cache_address)->cache_data) &&
			(*cache_address)->cache_data->in_perform_find_element_xi)
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Computed_field_find_element_xi_cache).  "
				"This cache cannot be destroyed.");
		}
		else
		{
			delete (*cache_address)->cache_data;
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
