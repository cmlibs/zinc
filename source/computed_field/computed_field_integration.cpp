/*******************************************************************************
FILE : computed_field_integration.c

LAST MODIFIED : 13 March 2003

DESCRIPTION :
Implements a computed field which integrates along elements, including
travelling between adjacent elements, using the faces for 2D and 3D elements
and the nodes for 1D elements.
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
#include <math.h>
#include <stdio.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_composite.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_set.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_adjacent_elements.h"
#include "general/compare.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/indexed_list_private.h"
#include "general/indexed_multi_range.h"
#include "general/list_private.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_integration.h"

struct Computed_field_integration_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Cmiss_region *root_region;
};

struct Computed_field_element_integration_mapping
/*******************************************************************************
LAST MODIFIED : 15 March 1999

DESCRIPTION :
==============================================================================*/
{
	/* Holds the pointer to the element, I don't access it so that the
		default object functions can be used.  The pointer is not
		referenced, just used as a label */
	struct FE_element *element;
	/* The three offsets for the xi1 = 0, xi2 = 0, xi3 = 0 corner. */
	float offset[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	/* The three differentials across the element xi1, xi2, xi3. */
	float differentials[MAXIMUM_ELEMENT_XI_DIMENSIONS];

	int access_count;
}; /* struct Computed_field_element_integration_mapping */

struct Computed_field_element_integration_mapping_fifo
/*******************************************************************************
LAST MODIFIED : 6 July 1999

DESCRIPTION :
Simple linked-list node structure for building a FIFO stack for the mapping
structure - needed for consistent growth of integration.
==============================================================================*/
{
	struct Computed_field_element_integration_mapping *mapping_item;
	struct Computed_field_element_integration_mapping_fifo *next;
}; /* struct Computed_field_element_integration_mapping_fifo */

struct Computed_field_integration_type_specific_data
{
	float cached_time;
	struct FE_element *seed_element;
	struct LIST(Computed_field_element_integration_mapping) *texture_mapping;
	struct FE_region *fe_region;
	/* last mapping successfully used by Computed_field_find_element_xi so 
		that it can first try this element again */
	struct Computed_field_element_integration_mapping *find_element_xi_mapping;
};

struct Computed_field_element_integration_mapping *CREATE(Computed_field_element_integration_mapping)
		 (void)
/*******************************************************************************
LAST MODIFIED : 16 March 1999

DESCRIPTION :
Creates a basic Computed_field with the given <name>. Its type is initially
COMPUTED_FIELD_CONSTANT with 1 component, returning a value of zero.
==============================================================================*/
{
	struct Computed_field_element_integration_mapping *mapping_item;

	ENTER(CREATE(Computed_field_element_integration_mapping));
	
	if (ALLOCATE(mapping_item,struct Computed_field_element_integration_mapping,1))
	{
		mapping_item->element = (struct FE_element *)NULL;
		mapping_item->offset[0] = 0.0;
		mapping_item->offset[1] = 0.0;
		mapping_item->offset[2] = 0.0;
		mapping_item->differentials[0] = 1.0;
		mapping_item->differentials[1] = 1.0;
		mapping_item->differentials[2] = 1.0;
		mapping_item->access_count = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Computed_field_element_integration_mapping).  Not enough memory");
		mapping_item = (struct Computed_field_element_integration_mapping *)NULL;
	}
	LEAVE;

	return (mapping_item);
} /* CREATE(Computed_field_element_integration_mapping) */

int DESTROY(Computed_field_element_integration_mapping)
	  (struct Computed_field_element_integration_mapping **mapping_address)
/*******************************************************************************
LAST MODIFIED : 26 May 1999

DESCRIPTION :
Frees memory/deaccess mapping at <*mapping_address>.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Computed_field_element_integration_mapping));
	if (mapping_address&&*mapping_address)
	{
		if (0 >= (*mapping_address)->access_count)
		{
			if ((*mapping_address)->element)
			{
				DEACCESS(FE_element)(&((*mapping_address)->element));
			}
			DEALLOCATE(*mapping_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Computed_field_element_integration_mapping).  Positive access_count");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Computed_field_element_integration_mapping).  Missing mapping");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Computed_field_element_integration_mapping) */

DECLARE_OBJECT_FUNCTIONS(Computed_field_element_integration_mapping)
DECLARE_LIST_TYPES(Computed_field_element_integration_mapping);
FULL_DECLARE_INDEXED_LIST_TYPE(Computed_field_element_integration_mapping);
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Computed_field_element_integration_mapping,
  element,struct FE_element *,compare_pointer)
DECLARE_INDEXED_LIST_FUNCTIONS(Computed_field_element_integration_mapping)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(
	Computed_field_element_integration_mapping,
	element,struct FE_element *,compare_pointer)

static int write_Computed_field_element_integration_mapping(
	struct Computed_field_element_integration_mapping *mapping,
	void *user_data)
{
	USE_PARAMETER(user_data);
	printf("Mapping %p Element %p (%f %f %f)\n",
		mapping, mapping->element,
		mapping->offset[0], mapping->offset[1], mapping->offset[2]);

	return( 1 );
}

static int Computed_field_integration_calculate_mapping_differentials(
	struct Computed_field_element_integration_mapping *mapping_item,
	struct Computed_field *integrand, struct Computed_field *coordinate_field,
	float time)
/*******************************************************************************
LAST MODIFIED : 7 November 2002

DESCRIPTION :
Calculates the differentials across the element by integrating the integrand
and coordinate field along the element edges, using xi = (t,0,0) for the first
differential, xi = (0,t,0) for the second and xi = (0,0,t) for the third.
==============================================================================*/
{
	double derivative_magnitude;
	FE_value 
		/* These two arrays are lower left diagonals matrices which for each row
			have the positions and weights of a gauss point scheme for integrating
			with the number of points corresponding to the row. */
		gauss_positions[2][2] = {{0.5, 0},
										 {0.25, 0.75}},
		gauss_weights[2][2] = {{1, 0},
									  {0.5, 0.5}},
		*temp, xi[3];
	int k, m, n, element_dimension,
		number_of_gauss_points, return_code;

	ENTER(Computed_field_integration_add_neighbours);
	if (mapping_item && mapping_item->element)
	{
		return_code = 1;
		number_of_gauss_points = 2;
		element_dimension = get_FE_element_dimension(mapping_item->element);
		for (k = 0;return_code&&(k<element_dimension);k++)
		{
			mapping_item->differentials[k] = 0.0;
			for (m = 0 ; m < number_of_gauss_points ; m++)
			{
				xi[0] = 0.0;
				xi[1] = 0.0;
				xi[2] = 0.0;
				xi[k] = gauss_positions[number_of_gauss_points - 1][m];
				/* Integrand elements should always be top level */
				Computed_field_evaluate_cache_in_element(integrand,
					mapping_item->element, xi, time, mapping_item->element,
					/*calculate_derivatives*/0);
				Computed_field_evaluate_cache_in_element(coordinate_field,
					mapping_item->element, xi, time, mapping_item->element,
					/*calculate_derivatives*/1);
#if defined (DEBUG)
				printf("Coordinate %d:  %g %g %g    %g %g %g\n", m,
					coordinate_field->values[0], coordinate_field->values[1],
					coordinate_field->values[2], coordinate_field->derivatives[0],
					coordinate_field->derivatives[1], coordinate_field->derivatives[2]);
#endif /* defined (DEBUG) */
				if (coordinate_field->number_of_components > 1)
				{
					derivative_magnitude = 0.0;
					temp=coordinate_field->derivatives+k;
					for (n = 0 ; n < coordinate_field->number_of_components ; n++)
					{
						derivative_magnitude += (double)*temp * (double)*temp;
						temp+=element_dimension;
					}
					mapping_item->differentials[k] += 
						integrand->values[0] * sqrt(derivative_magnitude) *
						gauss_weights[number_of_gauss_points - 1][m];
				}
				else
				{
					mapping_item->differentials[k] += 
						integrand->values[0] * coordinate_field->derivatives[k] *
						gauss_weights[number_of_gauss_points - 1][m];
				}
			}
#if defined (DEBUG)
			printf("Differential:  %g\n",
				mapping_item->differentials[0]);
#endif /* defined (DEBUG) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_integration_calculate_mapping_differentials.  Invalid argument(s)");
		return_code=0;
	}

	return(return_code);
	LEAVE;
} /* Computed_field_integration_calculate_mapping_differentials */

static int Computed_field_integration_calculate_mapping_update(
	struct Computed_field_element_integration_mapping *mapping_item,
	int face, struct Computed_field *integrand, struct Computed_field *coordinate_field,
	struct LIST(Computed_field_element_integration_mapping) *previous_texture_mapping,
	struct Computed_field_element_integration_mapping *seed_mapping_item,
	float time_step, float time)
/*******************************************************************************
LAST MODIFIED : 7 November 2002

DESCRIPTION :
Calculates the differentials and offsets across the element for a special 
upwind difference time integration.
==============================================================================*/
{
	FE_value
 		/* These two arrays are lower left diagonals matrices which for each row
			have the positions and weights of a gauss point scheme for integrating
			with the number of points corresponding to the row. */
		gauss_positions[2][2] = {{0.5, 0},
										 {0.25, 0.75}},
		gauss_weights[2][2] = {{1, 0},
									  {0.5, 0.5}},
			length, flow_step, *temp, xi[3];
	int derivative_magnitude, k, m, n, element_dimension,
		number_of_gauss_points, return_code;
	struct Computed_field_element_integration_mapping *previous_mapping_item;

	ENTER(Computed_field_integration_add_neighbours);
	if (mapping_item && mapping_item->element	&&
		(get_FE_element_dimension(mapping_item->element) == 1) && (previous_mapping_item = 
		FIND_BY_IDENTIFIER_IN_LIST(Computed_field_element_integration_mapping, element)
			(mapping_item->element, previous_texture_mapping)))
	{
		return_code = 1;
		number_of_gauss_points = 2;
		element_dimension = get_FE_element_dimension(mapping_item->element);
		for (k = 0;return_code&&(k<element_dimension);k++)
		{
			flow_step = 0.0;
			length = 0.0;
			for (m = 0 ; m < number_of_gauss_points ; m++)
			{
				xi[0] = 0.0;
				xi[1] = 0.0;
				xi[2] = 0.0;
				xi[k] = gauss_positions[number_of_gauss_points - 1][m];
				/* Integrand elements should always be top level */
				Computed_field_evaluate_cache_in_element(integrand,
					mapping_item->element, xi, time, mapping_item->element,
					/*calculate_derivatives*/0);
				Computed_field_evaluate_cache_in_element(coordinate_field,
					mapping_item->element, xi, time, mapping_item->element,
					/*calculate_derivatives*/1);
#if defined (DEBUG)
				printf("Coordinate %d:  %g %g %g    %g %g %g\n", m,
					coordinate_field->values[0], coordinate_field->values[1],
					coordinate_field->values[2], coordinate_field->derivatives[0],
					coordinate_field->derivatives[1], coordinate_field->derivatives[2]);
#endif /* defined (DEBUG) */
				flow_step = time_step * integrand->values[0] *
					gauss_weights[number_of_gauss_points - 1][m];
				if (coordinate_field->number_of_components > 1)
				{
					derivative_magnitude = 0.0;
					temp=coordinate_field->derivatives+k;
					for (n = 0 ; n < coordinate_field->number_of_components ; n++)
					{
						derivative_magnitude += *temp * *temp;
						temp+=element_dimension;
					}
					length += sqrt(derivative_magnitude) *
						gauss_weights[number_of_gauss_points - 1][m];
				}
				else
				{
					length += coordinate_field->derivatives[k] *
						gauss_weights[number_of_gauss_points - 1][m];
				}
			}
		}
		switch (face)
		{
			case 0:
			{
				if (length)
				{
					mapping_item->offset[0] = previous_mapping_item->offset[0] -
						(flow_step / length) *
						previous_mapping_item->differentials[0];
				}
				else
				{
					mapping_item->offset[0] = seed_mapping_item->offset[0];
				}
				mapping_item->differentials[0] = seed_mapping_item->offset[0] -
					mapping_item->offset[0];					
			} break;
			case 1:
			{
				mapping_item->offset[0] = seed_mapping_item->offset[0] +
					seed_mapping_item->differentials[0];
#if defined (DEBUG)
				if (mapping_item->offset[0] < previous_mapping_item->offset[0])
				{
					printf("Regression\n");
				}
#endif /* defined (DEBUG) */
				if (length)
				{
					mapping_item->differentials[0] = previous_mapping_item->offset[0]
						+ previous_mapping_item->differentials[0] - 
						(flow_step / length) * previous_mapping_item->differentials[0]
						- mapping_item->offset[0];
				}
				else
				{
					mapping_item->differentials[0] = 0.0;
				}
				
			} break;
		}
			
#if defined (DEBUG)
			printf("Differential:  %g\n",
				mapping_item->differentials[0]);
#endif /* defined (DEBUG) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_integration_calculate_mapping_differentials.  Invalid argument(s)");
		return_code=0;
	}

	return(return_code);
	LEAVE;
} /* Computed_field_integration_calculate_mapping_differentials */

static int Computed_field_integration_add_neighbours(
	struct Computed_field_element_integration_mapping *mapping_item,
	struct LIST(Computed_field_element_integration_mapping) *texture_mapping,
	struct Computed_field_element_integration_mapping_fifo **last_to_be_checked,
	struct Computed_field *integrand, struct Computed_field *coordinate_field,
	struct LIST(Index_multi_range) **node_element_list, struct FE_region *fe_region,
	struct LIST(Computed_field_element_integration_mapping) *previous_texture_mapping,
	float time_step, float time)
/*******************************************************************************
LAST MODIFIED : 7 November 2002

DESCRIPTION :
Add the neighbours that haven't already been put in the texture_mapping list and 
puts each new member in the texture_mapping list and the to_be_checked list.
If <previous_texture_mapping> is set then this is being used to update a time
varying integration and the behaviour is significantly different.
==============================================================================*/
{
	FE_value xi[3];
	int element_dimension, face_number, i, j, k, number_of_neighbour_elements, 
		return_code;
	struct Computed_field_element_integration_mapping *mapping_neighbour;
	struct Computed_field_element_integration_mapping_fifo *fifo_node;
	struct FE_element **neighbour_elements;
	struct FE_element_shape *shape;

	ENTER(Computed_field_integration_add_neighbours);
	if (mapping_item && texture_mapping && last_to_be_checked &&
		(*last_to_be_checked))
	{
		return_code=1;
		for (i = 0;return_code&&(i<(get_FE_element_dimension(mapping_item->element)*2));i++)
		{
			if (get_FE_element_dimension(mapping_item->element) == 1)
			{
				if(!(*node_element_list))
				{
					/* If we have 1D elements then we use the nodes to get to the 
						adjacent elements, normally use the faces */
					*node_element_list = create_node_element_list(fe_region);
				}
				if (!(adjacent_FE_element_from_nodes(mapping_item->element, i,
					&number_of_neighbour_elements, &neighbour_elements, *node_element_list,
					fe_region)))
				{
					number_of_neighbour_elements = 0;
				}
			}
			else
			{
				/* else we need an xi location to find the appropriate face for */
				xi[0] = 0.5;
				xi[1] = 0.5;
				xi[2] = 0.5;
				switch (i)
				{
					case 0:
					{
						xi[0] = 0.0;
					} break;
					case 1:
					{
						xi[0] = 1.0;
					} break;
					case 2:
					{
						xi[1] = 0.0;
					} break;
					case 3:
					{
						xi[1] = 1.0;
					} break;
					case 4:
					{
						xi[2] = 0.0;
					} break;
					case 5:
					{
						xi[2] = 1.0;
					} break;
				}
				if (!(get_FE_element_shape(mapping_item->element, &shape) &&
					FE_element_shape_find_face_number_for_xi(shape, xi, &face_number) 
					&& (adjacent_FE_element(mapping_item->element, face_number, 
					&number_of_neighbour_elements, &neighbour_elements))))
				{
					number_of_neighbour_elements = 0;
				}
			}
			if (number_of_neighbour_elements)
			{
				for (j = 0 ; j < number_of_neighbour_elements ; j++)
				{
					if(!(mapping_neighbour = FIND_BY_IDENTIFIER_IN_LIST(
						Computed_field_element_integration_mapping, element)
						(neighbour_elements[j], texture_mapping)))
					{
						if (ALLOCATE(fifo_node,
							struct Computed_field_element_integration_mapping_fifo,1)&&
							(mapping_neighbour = 
								CREATE(Computed_field_element_integration_mapping)()))
						{
							REACCESS(FE_element)(&mapping_neighbour->element,
								neighbour_elements[j]);
							element_dimension = get_FE_element_dimension(mapping_item->element);
							for (k = 0;return_code&&(k<element_dimension);k++)
							{							
								mapping_neighbour->offset[k] = mapping_item->offset[k];
							}
							if (!previous_texture_mapping)
							{
								Computed_field_integration_calculate_mapping_differentials(
									mapping_neighbour, integrand, coordinate_field, time);
								switch (i)
								{
									case 0:
									{
										mapping_neighbour->offset[0] -=
											mapping_neighbour->differentials[0];
									} break;
									case 1:
									{
										mapping_neighbour->offset[0] += 
											mapping_item->differentials[0];
									} break;
									case 2:
									{
										mapping_neighbour->offset[1] -=
											mapping_neighbour->differentials[2];
									} break;
									case 3:
									{
										mapping_neighbour->offset[1] +=
											mapping_item->differentials[1];
									} break;
									case 4:
									{
										mapping_neighbour->offset[2] -=
											mapping_neighbour->differentials[2];
									} break;
									case 5:
									{
										mapping_neighbour->offset[2] +=
											mapping_item->differentials[2];
									} break;
								}
							}
							else
							{
								/* Special time integration update step */
								Computed_field_integration_calculate_mapping_update(
									mapping_neighbour, i, integrand, coordinate_field, 
									previous_texture_mapping, mapping_item, time_step, time);
							}
							if (ADD_OBJECT_TO_LIST(Computed_field_element_integration_mapping)(
								mapping_neighbour, texture_mapping))
							{
								/* fill the fifo_node for the mapping_item; put at end of list */
								fifo_node->mapping_item=mapping_neighbour;
								fifo_node->next=
									(struct Computed_field_element_integration_mapping_fifo *)NULL;
								(*last_to_be_checked)->next = fifo_node;
								(*last_to_be_checked) = fifo_node;
							}
							else
							{
								printf("Error adding neighbour\n");
								write_Computed_field_element_integration_mapping(mapping_neighbour, NULL);
								printf("Texture mapping list\n");
								FOR_EACH_OBJECT_IN_LIST(Computed_field_element_integration_mapping)(
									write_Computed_field_element_integration_mapping, NULL, texture_mapping);
								DEALLOCATE(fifo_node);
								DESTROY(Computed_field_element_integration_mapping)(
									&mapping_neighbour);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_set_type_integration.  "
								"Unable to allocate member");
							DEALLOCATE(fifo_node);
							return_code=0;
						}
					}
				}
				DEALLOCATE(neighbour_elements);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_integration_add_neighbours.  Invalid argument(s)");
		return_code=0;
	}

	return(return_code);
	LEAVE;
} /* Computed_field_integration_add_neighbours */

static int Computed_field_integration_calculate_mapping(
	struct Computed_field *field, float time)
/*******************************************************************************
LAST MODIFIED : 7 November 2002

DESCRIPTION :
Calculates the mapping for the specified time.
==============================================================================*/
{
	int return_code;
	struct Computed_field *integrand, *coordinate_field;
	struct Computed_field_element_integration_mapping *mapping_item;
	struct Computed_field_element_integration_mapping_fifo *fifo_node,
		*first_to_be_checked, *last_to_be_checked;
	struct Computed_field_integration_type_specific_data *data;
	struct LIST(Computed_field_element_integration_mapping) *texture_mapping;
	struct LIST(Index_multi_range) *node_element_list;

	if (field && (data = (struct Computed_field_integration_type_specific_data *)
		field->type_specific_data) && (integrand = field->source_fields[0])
		&& (coordinate_field = field->source_fields[1]))
	{
		return_code = 1;
		first_to_be_checked=last_to_be_checked=
			(struct Computed_field_element_integration_mapping_fifo *)NULL;
		node_element_list=(struct LIST(Index_multi_range) *)NULL;
		if (texture_mapping = CREATE_LIST(Computed_field_element_integration_mapping)())
		{
			if (ALLOCATE(fifo_node, 
				struct Computed_field_element_integration_mapping_fifo,1)&&
				(mapping_item=CREATE(Computed_field_element_integration_mapping)()))
			{
				REACCESS(FE_element)(&mapping_item->element, data->seed_element);
				Computed_field_integration_calculate_mapping_differentials(
					mapping_item, integrand, coordinate_field, time);
				ADD_OBJECT_TO_LIST(Computed_field_element_integration_mapping)
					(mapping_item, texture_mapping);
				/* fill the fifo_node for the mapping_item; put at end of list */
				fifo_node->mapping_item=mapping_item;
				fifo_node->next=
					(struct Computed_field_element_integration_mapping_fifo *)NULL;
				first_to_be_checked=last_to_be_checked=fifo_node;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_integration.  "
					"Unable to allocate member");
				DEALLOCATE(fifo_node);
				return_code=0;
			}
			if (return_code)
			{
				while (return_code && first_to_be_checked)
				{
					return_code = Computed_field_integration_add_neighbours(
						first_to_be_checked->mapping_item, texture_mapping,
						&last_to_be_checked, integrand, coordinate_field,
						&node_element_list, data->fe_region, 
						(struct LIST(Computed_field_element_integration_mapping) *)NULL,
						0.0, time);

#if defined (DEBUG)
					printf("Item removed\n");
					write_Computed_field_element_integration_mapping(mapping_item, NULL);
#endif /* defined (DEBUG) */

					/* remove first_to_be_checked */
					fifo_node=first_to_be_checked;
					if (!(first_to_be_checked=first_to_be_checked->next))
					{
						last_to_be_checked=
							(struct Computed_field_element_integration_mapping_fifo *)NULL;
					}
					DEALLOCATE(fifo_node);

#if defined (DEBUG)
					printf("Texture mapping list\n");
					FOR_EACH_OBJECT_IN_LIST(Computed_field_element_integration_mapping)(
						write_Computed_field_element_integration_mapping, NULL, texture_mapping);
					printf("To be checked list\n");
					FOR_EACH_OBJECT_IN_LIST(Computed_field_element_integration_mapping)(
						write_Computed_field_element_integration_mapping, NULL, to_be_checked);
#endif /* defined (DEBUG) */
				}
				data->cached_time = time;
				data->texture_mapping = texture_mapping;
				data->find_element_xi_mapping=
					(struct Computed_field_element_integration_mapping *)NULL;
			}
			/* clean up to_be_checked list */
			while (first_to_be_checked)
			{
				fifo_node = first_to_be_checked;
				first_to_be_checked = first_to_be_checked->next;
				DEALLOCATE(fifo_node);
			}
			/* free cache on source fields */
			Computed_field_clear_cache(integrand);
			Computed_field_clear_cache(coordinate_field);
			if (!return_code)
			{
				DESTROY_LIST(Computed_field_element_integration_mapping)(&texture_mapping);
			}
			if (node_element_list)
			{
				DESTROY_LIST(Index_multi_range)(&node_element_list);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_integration_calculate_mapping.  "
				"Unable to create mapping list.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_integration_calculate_mapping.  "
			"Invalid arguments.");
		return_code=0;
	}
	return (return_code);
} /* Computed_field_integration_calculate_mapping */

static int Computed_field_element_integration_mapping_has_values(
	struct Computed_field_element_integration_mapping *mapping, void *user_data)
/*******************************************************************************
LAST MODIFIED : 26 May 1999

DESCRIPTION :
Compares the user_data values with the offsets in the <mapping>
==============================================================================*/
{
	FE_value *values;
	int return_code;

	ENTER(Computed_field_element_integration_mapping_has_values);
	if (mapping && (values = (FE_value *)user_data))
	{
		return_code = 0;
		if ((values[0] == mapping->offset[0])&&
			 (values[1] == mapping->offset[1])&&
			 (values[2] == mapping->offset[2]))
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_element_integration_mapping_has_values.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_element_integration_mapping_has_values */

static char computed_field_integration_type_string[] = "integration";
static char computed_field_xi_texture_coordinates_type_string[] = 
"xi_texture_coordinates";

int Computed_field_is_type_integration(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_integration);
	if (field)
	{
		return_code = (field->type_string == computed_field_integration_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_integration.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_is_type_integration */

static int Computed_field_integration_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_integration_type_specific_data *data;

	ENTER(Computed_field_integration_clear_type_specific);
	if (field && (data = 
		(struct Computed_field_integration_type_specific_data *)
		field->type_specific_data))
	{
		if (data->seed_element)
		{
			DEACCESS(FE_element)(&(data->seed_element));
		}
		if (data->texture_mapping)
		{
			DESTROY_LIST(Computed_field_element_integration_mapping)
				(&data->texture_mapping);
		}
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_integration_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_integration_clear_type_specific */

static void *Computed_field_integration_copy_type_specific(
	struct Computed_field *source_field, struct Computed_field *destination_field)
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_integration_type_specific_data *destination,
		*source;

	ENTER(Computed_field_integration_copy_type_specific);
	if (source_field && destination_field && (source = 
		(struct Computed_field_integration_type_specific_data *)
		source_field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_integration_type_specific_data, 1))
		{
			destination->seed_element = ACCESS(FE_element)(source->seed_element);
			destination->fe_region = source->fe_region;
			destination->texture_mapping = 
				(struct LIST(Computed_field_element_integration_mapping) *)NULL;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_integration_copy_type_specific.  "
				"Unable to allocate memory.");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_integration_copy_type_specific.  "
			"Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_integration_copy_type_specific */

int Computed_field_integration_clear_cache_type_specific
   (struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_integration_type_specific_data *data;

	ENTER(Computed_field_integration_clear_type_specific);
	if (field && (data = 
		(struct Computed_field_integration_type_specific_data *)
		field->type_specific_data))
	{
		if (data->find_element_xi_mapping)
		{
			DEACCESS(Computed_field_element_integration_mapping)
				(&data->find_element_xi_mapping);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_integration_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_integration_clear_type_specific */

static int Computed_field_integration_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;
	struct Computed_field_integration_type_specific_data *data,
		*other_data;

	ENTER(Computed_field_integration_type_specific_contents_match);
	if (field && other_computed_field && (data = 
		(struct Computed_field_integration_type_specific_data *)
		field->type_specific_data) && (other_data =
		(struct Computed_field_integration_type_specific_data *)
		other_computed_field->type_specific_data))
	{
		if (data->seed_element == other_data->seed_element)
		{
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
} /* Computed_field_integration_type_specific_contents_match */

#define Computed_field_integration_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_integration_is_defined_at_node \
	(Computed_field_is_defined_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
integration are not defined at nodes.
==============================================================================*/

#define Computed_field_integration_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_integration_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
No special criteria.
==============================================================================*/

#define Computed_field_integration_evaluate_cache_at_node \
   (Computed_field_evaluate_cache_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
integration are not defined at nodes.
==============================================================================*/

static int Computed_field_integration_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time,struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	FE_value element_to_top_level[9],*temp,*temp2,
		top_level_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int element_dimension, i, j, k, return_code, top_level_element_dimension;
	struct CM_element_information cm;
	struct Computed_field_element_integration_mapping *mapping;
	struct Computed_field_integration_type_specific_data *data;

	ENTER(Computed_field_integration_evaluate_cache_in_element);
	USE_PARAMETER(time);
	if (field && element && xi 
		&& (data = (struct Computed_field_integration_type_specific_data *)
		field->type_specific_data))
	{
		return_code = 1;

		if (!data->texture_mapping)
		{
			Computed_field_integration_calculate_mapping(field, time);
		}
		else
		{
			if ((time != data->cached_time)
				&& (Computed_field_has_multiple_times(field->source_fields[0])
				|| Computed_field_has_multiple_times(field->source_fields[1])))
			{
				DESTROY_LIST(Computed_field_element_integration_mapping)
					(&data->texture_mapping);
				Computed_field_integration_calculate_mapping(field, time);
			}
		}
		/* 1. Get top_level_element for types that must be calculated on them */
		element_dimension=get_FE_element_dimension(element);
		get_FE_element_identifier(element, &cm);
		if (CM_ELEMENT == cm.type)
		{
			top_level_element=element;
			for (i=0;i<element_dimension;i++)
			{
				top_level_xi[i]=xi[i];
			}
			/* do not set element_to_top_level */
			top_level_element_dimension=element_dimension;
		}
		else
		{
			/* check or get top_level element and xi coordinates for it */
			if (top_level_element=FE_element_get_top_level_element_conversion(
				element,top_level_element,(struct LIST(FE_element) *)NULL,
				-1,element_to_top_level))
			{
				/* convert xi to top_level_xi */
				top_level_element_dimension=get_FE_element_dimension(top_level_element);
				for (j=0;j<top_level_element_dimension;j++)
				{
					top_level_xi[j] = element_to_top_level[j*(element_dimension+1)];
					for (k=0;k<element_dimension;k++)
					{
						top_level_xi[j] +=
							element_to_top_level[j*(element_dimension+1)+k+1]*xi[k];
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_integration_evaluate_cache_in_element.  "
					"No top-level element found to evaluate field %s on",
					field->name);
				return_code=0;
			}
		}
		/* 2. Calculate the field */
		if (data->texture_mapping)
		{
			if (mapping = FIND_BY_IDENTIFIER_IN_LIST
				(Computed_field_element_integration_mapping,element)
				(top_level_element, data->texture_mapping))
			{
				if (element == top_level_element)
				{
					temp=field->derivatives;
					for (i = 0 ; i < element_dimension ; i++)
					{
						field->values[i] = mapping->offset[i] + 
							mapping->differentials[i] * xi[i];
						if (calculate_derivatives)
						{
							for (j=0;j<element_dimension;j++)
							{
								if (i==j)
								{
									*temp = mapping->differentials[i];
								}
								else
								{
									*temp = 0.0;
								}
								temp++;
							}
							field->derivatives_valid = 1;
						}
					}
				}
				else
				{
					temp = element_to_top_level;
					temp2 = field->derivatives;
					for (i = 0 ; i < top_level_element_dimension ; i++)
					{
						field->values[i] = mapping->offset[i] + (*temp);
						temp++;
						for (j = 0 ; j < get_FE_element_dimension(element) ; j++)
						{
							field->values[i] += (*temp) * 
								mapping->differentials[j] * xi[j];
							if (calculate_derivatives)
							{
								*temp2 = *temp * mapping->differentials[j];
								temp2++;
							}
							temp++;
						}
					}
					if (calculate_derivatives)
					{
						field->derivatives_valid = 1;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_integration_evaluate_cache_in_element."
					"  Element %d not found in Xi texture coordinate mapping field %s",
					cm.number, field->name);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_integration_evaluate_cache_in_element.  "
				"Xi texture coordinate mapping not calculated");
			return_code=0;
		}

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_integration_evaluate_cache_in_element.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_integration_evaluate_cache_in_element */

#define Computed_field_integration_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_integration_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_integration_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_integration_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_integration_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_integration_get_native_resolution \
	(Computed_field_get_native_resolution_function)NULL

static int Computed_field_integration_find_element_xi(
	struct Computed_field *field,
	FE_value *values, int number_of_values, struct FE_element **element, 
	FE_value *xi, int element_dimension, struct Cmiss_region *search_region)
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
==============================================================================*/
{
	FE_value floor_values[3];
	int i, return_code;
	struct Computed_field_element_integration_mapping *mapping;
	struct Computed_field_integration_type_specific_data *data;
	struct FE_region *fe_region;

	ENTER(Computed_field_integration_find_element_xi);
	if (field&&values&&(number_of_values==field->number_of_components)&&element&&xi&&
		search_region && (fe_region = Cmiss_region_get_FE_region(search_region)) &&
		(data = (struct Computed_field_integration_type_specific_data *)
		field->type_specific_data))
	{
		if (!data->texture_mapping)
		{
			Computed_field_integration_calculate_mapping(field, /*time*/0);
		}
		else
		{
#if defined (NEW_CODE)
			/* Until we are calculating this at the correct time there is no
				point seeing if the time has changed */
			if ((time != data->cached_time)
				&& (Computed_field_has_multiple_times(field->source_fields[0])
				|| Computed_field_has_multiple_times(field->source_fields[1])))
			{
				DESTROY_LIST(Computed_field_element_integration_mapping)
					(&data->texture_mapping);
				Computed_field_integration_calculate_mapping(field, time);
			}
#endif /* defined (NEW_CODE) */
		}
		if (number_of_values<=3)
		{
			for (i = 0 ; i < number_of_values ; i++)
			{
				floor_values[i] = floor(values[i]);
			}
			for ( ; i < 3 ; i++)
			{
				floor_values[i] = 0.0;
			}
			if (data->texture_mapping)
			{
				return_code=0;
				/* Check the last successful mapping first */
				if (data->find_element_xi_mapping&&
					Computed_field_element_integration_mapping_has_values(
						data->find_element_xi_mapping, (void *)floor_values))
				{
					return_code = 1;
				}
				else
				{
					/* Find in the list */
					if (mapping = FIRST_OBJECT_IN_LIST_THAT(Computed_field_element_integration_mapping)
						(Computed_field_element_integration_mapping_has_values, (void *)floor_values,
							data->texture_mapping))
					{
						REACCESS(Computed_field_element_integration_mapping)(&(data->find_element_xi_mapping),
							mapping);
						return_code = 1;
					}
				}
				if (return_code)
				{
					*element = data->find_element_xi_mapping->element;
					for (i = 0 ; i < get_FE_element_dimension(*element) ; i++)
					{
						xi[i] = values[i] - floor_values[i];	
					}
					if (!FE_region_contains_FE_element(fe_region, *element))
					{
						*element = (struct FE_element *)NULL;
						display_message(ERROR_MESSAGE,
							"Computed_field_integration_find_element_xi.  "
							"Element is not in specified group");
						return_code=0;
					}
					if (element_dimension && (element_dimension != get_FE_element_dimension(*element)))
					{
						*element = (struct FE_element *)NULL;
						display_message(ERROR_MESSAGE,
							"Computed_field_integration_find_element_xi.  "
							"Element is not of the required dimension");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_integration_find_element_xi.  "
						"Unable to find mapping for given values");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_integration_find_element_xi.  "
					"Xi texture coordinate mapping not calculated");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_integration_find_element_xi.  "
				"Only implemented for three or less values");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_integration_find_element_xi.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_integration_find_element_xi */

static int list_Computed_field_integration(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_integration_type_specific_data *data;

	ENTER(List_Computed_field_integration);
	if (field && (data = 
		(struct Computed_field_integration_type_specific_data *)
		field->type_specific_data))
	{
		display_message(INFORMATION_MESSAGE,"    seed_element : %d\n",
			FE_element_get_cm_number(data->seed_element));
		display_message(INFORMATION_MESSAGE,"    integrand field : %s\n",
			field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,"    coordinate field : %s\n",
			field->source_fields[1]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_integration.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_integration */

static char *Computed_field_integration_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error, xi_texture_coordinates;
	struct Computed_field_integration_type_specific_data *data;

	ENTER(Computed_field_integration_get_command_string);
	command_string = (char *)NULL;
	if (field && (data = 
		(struct Computed_field_integration_type_specific_data *)
		field->type_specific_data))
	{
		error = 0;
		if (xi_texture_coordinates = (
			Computed_field_is_constant_scalar(field->source_fields[0], 1.0) &&
			Computed_field_is_type_xi_coordinates(field->source_fields[1],
				(void *)NULL)))
		{
			append_string(&command_string,
				computed_field_xi_texture_coordinates_type_string, &error);
		}
		else
		{
			append_string(&command_string,
				computed_field_integration_type_string, &error);
		}
		sprintf(temp_string, " seed_element %d", FE_element_get_cm_number(data->seed_element));
		append_string(&command_string, temp_string, &error);
		if (!xi_texture_coordinates)
		{
			append_string(&command_string, " integrand ", &error);
			if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
			{
				make_valid_token(&field_name);
				append_string(&command_string, field_name, &error);
				DEALLOCATE(field_name);
			}
			append_string(&command_string, " coordinate ", &error);
			if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
			{
				make_valid_token(&field_name);
				append_string(&command_string, field_name, &error);
				DEALLOCATE(field_name);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_integration_get_command_string.  "
			"Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_integration_get_command_string */

static int Computed_field_update_integration_scheme(struct Computed_field *field,
	struct FE_region *fe_region,
	struct Computed_field *integrand, struct Computed_field *coordinate_field,
	float time_step)
/*******************************************************************************
LAST MODIFIED : 7 November 2002

DESCRIPTION :
This is a special method for updating with a time integration step for flow
problems.  It takes a current integration field and updates it to the next
timestep.
=============================================================================*/
{
	int return_code;
	struct Computed_field_element_integration_mapping *mapping_item, *previous_mapping_item,
		*seed_mapping_item;
	struct Computed_field_element_integration_mapping_fifo *fifo_node,
		*first_to_be_checked, *last_to_be_checked;
	struct Computed_field_integration_type_specific_data *data;
	struct LIST(Computed_field_element_integration_mapping) *texture_mapping;
	struct LIST(Index_multi_range) *node_element_list;

	ENTER(Computed_field_update_integration_scheme);
	if (field&&Computed_field_is_type_integration(field) && (data = 
		(struct Computed_field_integration_type_specific_data *)
		field->type_specific_data)&&integrand&&coordinate_field)
	{
		return_code=1;
		first_to_be_checked=last_to_be_checked=
			(struct Computed_field_element_integration_mapping_fifo *)NULL;
		node_element_list=(struct LIST(Index_multi_range) *)NULL;
		texture_mapping = CREATE_LIST(Computed_field_element_integration_mapping)();
		if (ALLOCATE(fifo_node,
			struct Computed_field_element_integration_mapping_fifo,1)&&
			(mapping_item=CREATE(Computed_field_element_integration_mapping)()))
		{
			REACCESS(FE_element)(&mapping_item->element, data->seed_element);
			previous_mapping_item = FIND_BY_IDENTIFIER_IN_LIST
				(Computed_field_element_integration_mapping,element)
				(data->seed_element, data->texture_mapping);
			seed_mapping_item = CREATE(Computed_field_element_integration_mapping)();
			seed_mapping_item->offset[0] = previous_mapping_item->offset[0];
			seed_mapping_item->differentials[0] = time_step;
			Computed_field_integration_calculate_mapping_update(
				mapping_item, 1, integrand, coordinate_field,
				data->texture_mapping, seed_mapping_item, time_step, /*time*/0);
			DESTROY(Computed_field_element_integration_mapping)(&seed_mapping_item);
			ADD_OBJECT_TO_LIST(Computed_field_element_integration_mapping)
				(mapping_item, texture_mapping);
			/* fill the fifo_node for the mapping_item; put at end of list */
			fifo_node->mapping_item=mapping_item;
			fifo_node->next=
				(struct Computed_field_element_integration_mapping_fifo *)NULL;
			first_to_be_checked=last_to_be_checked=fifo_node;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_integration.  "
				"Unable to allocate member");
			DEALLOCATE(fifo_node);
			return_code=0;
		}
		if (return_code)
		{
			while (return_code && first_to_be_checked)
			{
				return_code = Computed_field_integration_add_neighbours(
					first_to_be_checked->mapping_item, texture_mapping,
					&last_to_be_checked, integrand, coordinate_field,
					&node_element_list, fe_region, data->texture_mapping,
					time_step, /*time*/0);

				/* remove first_to_be_checked */
				fifo_node=first_to_be_checked;
				if (!(first_to_be_checked=first_to_be_checked->next))
				{
					last_to_be_checked=
						(struct Computed_field_element_integration_mapping_fifo *)NULL;
				}
				DEALLOCATE(fifo_node);
			}
			/* clean up to_be_checked list */
			while (first_to_be_checked)
			{
				fifo_node = first_to_be_checked;
				first_to_be_checked = first_to_be_checked->next;
				DEALLOCATE(fifo_node);
			}
			/* free cache on source fields */
			Computed_field_clear_cache(integrand);
			Computed_field_clear_cache(coordinate_field);
			if (!return_code)
			{
				DESTROY_LIST(Computed_field_element_integration_mapping)(&texture_mapping);
			}
			if (node_element_list)
			{
				DESTROY_LIST(Index_multi_range)(&node_element_list);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_integration.  Unable to create list");
			return_code=0;
		}
		if(return_code)
		{
			/* Replace old mapping with new one */
			DESTROY_LIST(Computed_field_element_integration_mapping)(&data->texture_mapping);
			data->texture_mapping = texture_mapping;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_integration.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_update_integration_scheme */

#define Computed_field_integration_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_integration(struct Computed_field *field,
	struct FE_element *seed_element,
	struct FE_region *fe_region,
	struct Computed_field *integrand, struct Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_INTEGRATION.
The seed element is set to the number given and the mapping calculated.
Sets the number of components to the dimension of the given element.
The <integrand> is the value that is integrated over each element and the
<coordinate_field> is used to define the arc length differential for each element.
Currently only two gauss points are supported, a linear integration.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
=============================================================================*/
{
	int number_of_source_fields, return_code;
	struct Computed_field **source_fields;
	struct Computed_field_integration_type_specific_data *data;
	struct CM_element_information cm;
	struct FE_element *element;

	ENTER(Computed_field_set_type_integration);
	if (field&&seed_element&&integrand&&coordinate_field)
	{
		return_code=1;
		number_of_source_fields=2;
		/* 1. make dynamic allocations for any new type-specific data */
		if ((ALLOCATE(source_fields,struct Computed_field *,
			number_of_source_fields)) && (ALLOCATE(data,
			struct Computed_field_integration_type_specific_data, 1)))
		{
			/* Add seed element */
			get_FE_element_identifier(seed_element, &cm);
			if ((element=FE_region_get_FE_element_from_identifier(
				fe_region, &cm)) && (element==seed_element))
			{
				data->cached_time = 0;
				data->seed_element = (struct FE_element *)NULL;
				data->texture_mapping =
					(struct LIST(Computed_field_element_integration_mapping) *)NULL;
				data->find_element_xi_mapping = 
					(struct Computed_field_element_integration_mapping *)NULL;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_integration.  "
					"Unable to find seed element");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_integration.  Unable to create type specific data");
			return_code=0;
		}
		if(return_code)
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_integration_type_string;
			field->number_of_components = get_FE_element_dimension(seed_element);
			/* source_fields: 0=integrand, 1=coordinate_field */
			source_fields[0]=ACCESS(Computed_field)(integrand);
			source_fields[1]=ACCESS(Computed_field)(coordinate_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			field->type_specific_data = (void *)data;
			data->seed_element = ACCESS(FE_element)(seed_element);
			data->texture_mapping = 
				(struct LIST(Computed_field_element_integration_mapping) *)NULL;
			data->fe_region = fe_region;
			data->find_element_xi_mapping=
				(struct Computed_field_element_integration_mapping *)NULL;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(integration);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_integration.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_integration */

int Computed_field_get_type_integration(struct Computed_field *field,
	struct FE_element **seed_element, struct Computed_field **integrand,
	struct Computed_field **coordinate_field)
/*******************************************************************************
LAST MODIFIED : 3 November 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_INTEGRATION, 
the seed element used for the mapping is returned - otherwise an error is reported.
==============================================================================*/
{
	int return_code;
	struct Computed_field_integration_type_specific_data *data;

	ENTER(Computed_field_get_type_integration);
	if (field&&(field->type_string==computed_field_integration_type_string)
		&&(data = 
		(struct Computed_field_integration_type_specific_data *)
		field->type_specific_data))
	{
		*seed_element=data->seed_element;
		*integrand=field->source_fields[0];
		*coordinate_field=field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_integration.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_integration */

static int define_Computed_field_type_integration(struct Parse_state *state,
	void *field_void,void *computed_field_integration_package_void)
/*******************************************************************************
LAST MODIFIED : 10 November 2000

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_INTEGRATION (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	float time_update;
	int return_code;
	struct Computed_field *coordinate_field, *field, *integrand;
	struct Computed_field_integration_package *computed_field_integration_package;
	struct FE_element *seed_element;	
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_integrand_field_data;

	ENTER(define_Computed_field_type_integration);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_integration_package=
			(struct Computed_field_integration_package *)computed_field_integration_package_void))
	{
		return_code=1;
		coordinate_field=(struct Computed_field *)NULL;
		integrand=(struct Computed_field *)NULL;
		seed_element = (struct FE_element *)NULL;
		time_update = 0;
		if (Computed_field_is_type_integration(field))
		{
			return_code = Computed_field_get_type_integration(field,
				&seed_element, &integrand, &coordinate_field);
		}
		if (return_code)
		{
			if (coordinate_field)
			{
				ACCESS(Computed_field)(coordinate_field);
			}
			if (integrand)
			{
				ACCESS(Computed_field)(integrand);
			}
			if (seed_element)
			{
				ACCESS(FE_element)(seed_element);
			}
			option_table = CREATE(Option_table)();
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				computed_field_integration_package->computed_field_manager;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",
				&coordinate_field,&set_coordinate_field_data,
				set_Computed_field_conditional);
			/* integrand */
			set_integrand_field_data.computed_field_manager=
				computed_field_integration_package->computed_field_manager;
			set_integrand_field_data.conditional_function=
				Computed_field_is_scalar;
			set_integrand_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table,"integrand",
				&integrand,&set_integrand_field_data,
				set_Computed_field_conditional);
			/* seed_element */
			Option_table_add_entry(option_table,"seed_element",
				&seed_element,
				Cmiss_region_get_FE_region(computed_field_integration_package->root_region),
				set_FE_element_top_level_FE_region);
			/* update_time_integration */
			Option_table_add_entry(option_table,"update_time_integration",
				&time_update, NULL, set_float);
			if (return_code = Option_table_multi_parse(option_table,state))
			{
				if (!coordinate_field)
				{
					display_message(ERROR_MESSAGE,
						"You must specify a coordinate field.");
					return_code=0;
				}
				if (!integrand)
				{
					display_message(ERROR_MESSAGE,
						"You must specify an integrand field.");
					return_code=0;
				}
			}
			if (return_code)
			{
				if (time_update && Computed_field_is_type_integration(field))
				{
					return_code=Computed_field_update_integration_scheme(field,
						Cmiss_region_get_FE_region(computed_field_integration_package->root_region),
						integrand, coordinate_field, time_update);
				}
				else
				{
					return_code=Computed_field_set_type_integration(field,
						seed_element, Cmiss_region_get_FE_region(computed_field_integration_package->root_region),
						integrand, coordinate_field);
				}
			}
			DESTROY(Option_table)(&option_table);
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (integrand)
			{
				DEACCESS(Computed_field)(&integrand);
			}
			if (seed_element)
			{
				DEACCESS(FE_element)(&seed_element);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_integration.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_integration */

static int define_Computed_field_type_xi_texture_coordinates(struct Parse_state *state,
	void *field_void,void *computed_field_integration_package_void)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_XI_TEXTURE_COORDINATES (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	FE_value value;
	int return_code;
	struct Computed_field *coordinate_field, *field, *integrand;
	struct Computed_field_integration_package *computed_field_integration_package;
	struct FE_element *seed_element;	
	struct Option_table *option_table;

	ENTER(define_Computed_field_type_xi_texture_coordinates);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_integration_package=
			(struct Computed_field_integration_package *)computed_field_integration_package_void))
	{
		return_code=1;
		if (!(coordinate_field = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
			Computed_field_is_type_xi_coordinates, (void *)NULL,
			computed_field_integration_package->computed_field_manager)))
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_xi_texture_coordinates.  xi field not found");
			return_code=0;
		}
		value = 1.0;
		if (!((integrand = ACCESS(Computed_field)(CREATE(Computed_field)("constant_1.0"))) &&
		  Computed_field_set_type_constant(integrand,1,&value)))
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_xi_texture_coordinates.  Unable to create constant field");
			return_code=0;
		}
		if (return_code)
		{
			seed_element = (struct FE_element *)NULL;
			option_table = CREATE(Option_table)();
			/* seed_element */
			Option_table_add_entry(option_table,"seed_element",
				&seed_element, 
				Cmiss_region_get_FE_region(computed_field_integration_package->root_region),
				set_FE_element_top_level_FE_region);
			if (return_code = Option_table_multi_parse(option_table,state))
			{
				return_code=Computed_field_set_type_integration(field,
					seed_element, 
					Cmiss_region_get_FE_region(computed_field_integration_package->root_region),
					integrand, coordinate_field);
			}
			DESTROY(Option_table)(&option_table);
			if (seed_element)
			{
				DEACCESS(FE_element)(&seed_element);
			}
		}
		if (integrand)
		{
			DEACCESS(Computed_field)(&integrand);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_xi_texture_coordinates.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_xi_texture_coordinates */

int Computed_field_register_type_integration(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region)
/*******************************************************************************
LAST MODIFIED : 15 January 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_integration_package 
		computed_field_integration_package;

	ENTER(Computed_field_register_type_integration);
	if (computed_field_package)
	{
		computed_field_integration_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		computed_field_integration_package.root_region = root_region;
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_integration_type_string, 
			define_Computed_field_type_integration,
			&computed_field_integration_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_xi_texture_coordinates_type_string, 
			define_Computed_field_type_xi_texture_coordinates,
			&computed_field_integration_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_type_integration.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_type_integration */

