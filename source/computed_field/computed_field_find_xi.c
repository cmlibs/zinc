/*******************************************************************************
FILE : computed_field_find_xi.c

LAST MODIFIED : 21 August 2002

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
#include "graphics/texture.h"
#if defined (DM_BUFFERS)
#include "three_d_drawing/dm_interface.h"
#endif /* defined (DM_BUFFERS) */
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
#if defined (DM_BUFFERS)
	int bit_shift;
	int minimum_element_number;
	int maximum_element_number;
	struct Dm_buffer *dmbuffer;
#endif /* defined (DM_BUFFERS) */
	int valid_values;
	struct FE_element *element;
	int number_of_values;
	FE_value *values;
	FE_value *working_values;
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
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
	int found_number_of_xi;
	FE_value *found_values;
	FE_value *found_derivatives;
	float tolerance;
}; /* Computed_field_iterative_find_element_xi_data */

#if defined (OLD_CODE)
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
					/*time*/0, (struct FE_element *)NULL, values, derivatives);

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
					case 3:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_iterative_element_conditional.  "
							"3-D Elements not supported yet");
						return_code = 0;
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
#endif /* defined (OLD_CODE) */

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
	FE_value delta, *derivatives, last_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS], *values;
	int converged, i, indx[MAXIMUM_ELEMENT_XI_DIMENSIONS], iterations, j, k,
		number_of_xi, return_code, simplex_dimensions,
		simplex_direction[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct Computed_field_iterative_find_element_xi_data *data;

	ENTER(Computed_field_iterative_element_conditional);
	if (element && element->shape &&
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
						"Computed_field_iterative_element_conditional.  "
						"Unable to allocate derivative storage");
					return_code = 0;
				}
			}
			if (return_code)
			{
				values = data->found_values;
				derivatives = data->found_derivatives;
				/* determine whether the element is simplex to limit xi space */
				simplex_dimensions = 0;
				switch (element->shape->dimension)
				{
					case 2:
					{
						if (SIMPLEX_SHAPE == element->shape->type[0])
						{
							simplex_dimensions = 2;
							simplex_direction[0] = 0;
							simplex_direction[1] = 1;
						}
					} break;
					case 3:
					{
						if (SIMPLEX_SHAPE == element->shape->type[0])
						{
							if (LINE_SHAPE == element->shape->type[3])
							{
								simplex_dimensions = 2;
								simplex_direction[0] = 0;
								simplex_direction[1] = 2;
							}
							else if (LINE_SHAPE == element->shape->type[5])
							{
								simplex_dimensions = 2;
								simplex_direction[0] = 0;
								simplex_direction[1] = 1;
							}
							else
							{
								/* tetrahedron */
								simplex_dimensions = 3;
								simplex_direction[0] = 0;
								simplex_direction[1] = 1;
								simplex_direction[2] = 2;
							}
						}
						else if (SIMPLEX_SHAPE == element->shape->type[3])
						{
							simplex_dimensions = 2;
							simplex_direction[0] = 1;
							simplex_direction[1] = 2;
						}
					} break;
				}
				/* start at centre to find element xi */
				if (simplex_dimensions)
				{
					for (i = 0 ; i < number_of_xi ; i++)
					{
						/* not quite centre of tetrahedron; but these are usually linear
							 so doesn't matter */
						data->xi[i] = 0.33;
					}
				}
				else
				{
					for (i = 0 ; i < number_of_xi ; i++)
					{
						data->xi[i] = 0.5;
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
						if (LU_decompose(number_of_xi, a, indx, &d) &&
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
								/* keep xi within simplex bounds plus tolerance */
								if (simplex_dimensions)
								{
									/* calculate distance out of element in xi space */
									delta = -1.0 - data->tolerance;
									for (i = 0; i < simplex_dimensions; i++)
									{
										delta += data->xi[simplex_direction[i]];
									}
									if (delta > 0.0)
									{
										/* subtract delta equally from all directions */
										delta /= simplex_dimensions;
										for (i = 0; i < simplex_dimensions; i++)
										{
											data->xi[simplex_direction[i]] -= delta;
										}
									}
								}
								/* keep xi within 0.0 to 1.0 bounds plus tolerance */
								for (i = 0; i < number_of_xi; i++)
								{
									if (data->xi[i] < -data->tolerance)
									{
										data->xi[i] = -data->tolerance;
									}
									else if (data->xi[i] > 1.0 + data->tolerance)
									{
										data->xi[i] = 1.0 + data->tolerance;
									}
								}
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
	FE_value *xi, struct GROUP(FE_element) *search_element_group)
/*******************************************************************************
LAST MODIFIED : 17 December 2002

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
	int i, number_of_xi, return_code;

	ENTER(Computed_field_perform_find_element_xi);
	if (field && values && (number_of_values == field->number_of_components) &&
		element && xi && search_element_group)
	{
		return_code = 1;
		if (field->find_element_xi_cache)
		{
			cache = field->find_element_xi_cache;
			if (cache->number_of_values != number_of_values)
			{
				cache->valid_values = 0;
				DEALLOCATE(cache->values);
				DEALLOCATE(cache->working_values);
			}
			if (cache->valid_values &&
				((!cache->element) || IS_OBJECT_IN_GROUP(FE_element)(
				cache->element, search_element_group)))
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
				*element = (struct FE_element *)NULL;

				/* Try the cached element first if it is in the group */
				if (field->element && IS_OBJECT_IN_GROUP(FE_element)
					(field->element, search_element_group) &&
					Computed_field_iterative_element_conditional(
						field->element, (void *)&find_element_xi_data))
				{
					*element = field->element;
				}
				/* Now try every element */
				if (!*element)
				{
					*element = FIRST_OBJECT_IN_GROUP_THAT(FE_element)
						(Computed_field_iterative_element_conditional,
							&find_element_xi_data, search_element_group);
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
				cache->element = *element;
				for (i = 0 ; i < number_of_xi ; i++)
				{
					cache->xi[i] = find_element_xi_data.xi[i];
				}
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

#if defined (DM_BUFFERS)
static int Expand_element_range(struct FE_element *element, void *data_void)
/*******************************************************************************
LAST MODIFIED : 26 June 2000

DESCRIPTION :
Stores cache data for the Computed_field_find_element_xi_special routine.
==============================================================================*/
{
	int return_code;
	struct Computed_field_find_element_xi_cache *data;
	
	ENTER(Expand_element_range);
	
	if (data = (struct Computed_field_find_element_xi_cache *)data_void)
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
			"Expand_element_range.  Missing Computed_field_find_element_xi_cache data.");
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
		  
#if defined (DEBUG)
			printf ( "%d %lf %lf %lf\n",  element->cm.number, red, green, blue);
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

			if (SIMPLEX_SHAPE== *(element->shape->type))
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
#endif /* defined (DM_BUFFERS) */

int Computed_field_find_element_xi_special(struct Computed_field *field, 
	struct Computed_field_find_element_xi_cache **cache_ptr,
	FE_value *values, int number_of_values, struct FE_element **element, 
	FE_value *xi, struct GROUP(FE_element) *search_element_group,
	struct User_interface *user_interface,
	float *hint_minimums, float *hint_maximums, float *hint_resolution)
/*******************************************************************************
LAST MODIFIED : 16 April 2002

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
	int return_code;
#if defined (DM_BUFFERS)
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
	struct Render_element_data data;
	int gl_list, i, nx, ny, px, py, scaled_number;
#endif /* defined (DM_BUFFERS) */

	ENTER(Computed_field_find_element_xi_special);
	USE_PARAMETER(number_of_values);
#if !defined (DM_BUFFERS)
	USE_PARAMETER(field);
	USE_PARAMETER(cache_ptr);
	USE_PARAMETER(values);
	USE_PARAMETER(element);
	USE_PARAMETER(xi);
	USE_PARAMETER(search_element_group);
	USE_PARAMETER(user_interface);
	USE_PARAMETER(hint_minimums);
	USE_PARAMETER(hint_maximums);
	USE_PARAMETER(hint_resolution);
#endif /* !defined (DM_BUFFERS) */

	return_code = 0;
#if defined (DM_BUFFERS)
	/* The Dm_buffers are not available with other widget systems at the
		moment */
	/* If the number of elements in the group is small then there probably isn't
		any benefit to using this method */
	/* This method is adversely affected when displaying on a remote machine as every
		pixel grab requires transfer across the network */
	if (hint_minimums && hint_maximums && hint_resolution && 
		((2 == Computed_field_get_number_of_components(field)) ||
		((3 == Computed_field_get_number_of_components(field)) &&
		(hint_resolution[2] == 1.0f))) && user_interface && search_element_group
		&& (5 < NUMBER_IN_GROUP(FE_element)(search_element_group)) /*&&
			(Computed_field_is_find_element_xi_capable(field,NULL))*/)
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
					*cache_ptr = CREATE(Computed_field_find_element_xi_cache)();
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
			if (cache->dmbuffer)
			{
				Dm_buffer_glx_make_current(cache->dmbuffer);
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
					if (*element = FIND_BY_IDENTIFIER_IN_GROUP(FE_element,
						identifier)(&cm, search_element_group))
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
#endif /* defined (DM_BUFFERS) */
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
#if defined (DM_BUFFERS)
	  cache->dmbuffer = (struct Dm_buffer *)NULL;
#endif /* defined (DM_BUFFERS) */
	  cache->valid_values = 0;
	  cache->number_of_values = 0;
	  cache->values = (FE_value *)NULL;
	  cache->working_values = (FE_value *)NULL;
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
#if defined (DM_BUFFERS)
		if ((*cache_address)->dmbuffer)
		{
			DESTROY(Dm_buffer)(&(*cache_address)->dmbuffer);
		}
#endif /* defined (DM_BUFFERS) */
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
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Computed_field_find_element_xi_cache).  Missing cache");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Computed_field_find_element_xi_cache) */
