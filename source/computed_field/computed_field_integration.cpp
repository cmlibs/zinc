/*******************************************************************************
FILE : computed_field_integration.c

LAST MODIFIED : 24 August 2006

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
extern "C" {
#include <math.h>
#include <stdio.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_composite.h"
#include "computed_field/computed_field_finite_element.h"
}
#include "computed_field/computed_field_private.hpp"
extern "C" {
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
}

namespace {

struct Computed_field_integration_package 
{
	MANAGER(Computed_field) *computed_field_manager;
	Cmiss_region *root_region;
};

struct Computed_field_element_integration_mapping
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	/* Holds the pointer to the element */
	FE_element *element;
	/* The values for the xi1 = 0, xi2 = 0, xi3 = 0 corner,
		a vector of size number_of_components. */
	float *values;

	int access_count;
}; /* Computed_field_element_integration_mapping */

struct Computed_field_node_integration_mapping
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	/* Holds the pointer to the node, can't use node as this is a symbol in the LISTS */
	FE_node *node_ptr;
	/* The field values for the node. */
	float *values;

	int access_count;
}; /* Computed_field_node_integration_mapping */

struct Computed_field_element_integration_mapping_fifo
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Simple linked-list node structure for building a FIFO stack for the mapping
structure - needed for consistent growth of integration.
==============================================================================*/
{
	Computed_field_element_integration_mapping *mapping_item;
	Computed_field_element_integration_mapping_fifo *next;
}; /* Computed_field_element_integration_mapping_fifo */

struct Computed_field_integration_has_values_data
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int number_of_values;
	float *values;
}; /* Computed_field_integration_has_values_data */

Computed_field_element_integration_mapping *CREATE(Computed_field_element_integration_mapping)
	(FE_element *element, int number_of_components)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Creates a basic Computed_field with the given <name>. Its type is initially
COMPUTED_FIELD_CONSTANT with 1 component, returning a value of zero.
==============================================================================*/
{
	int i;
	Computed_field_element_integration_mapping *mapping_item;

	ENTER(CREATE(Computed_field_element_integration_mapping));
	
	if (element &&
		ALLOCATE(mapping_item,Computed_field_element_integration_mapping,1) &&
		ALLOCATE(mapping_item->values, FE_value, number_of_components))
	{
		mapping_item->element = ACCESS(FE_element)(element);
		for (i = 0 ; i < number_of_components ; i++)
		{
			mapping_item->values[i] = 0.0;
		}
		mapping_item->access_count = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Computed_field_element_integration_mapping).  Not enough memory");
		mapping_item = (Computed_field_element_integration_mapping *)NULL;
	}
	LEAVE;

	return (mapping_item);
} /* CREATE(Computed_field_element_integration_mapping) */

int DESTROY(Computed_field_element_integration_mapping)
	  (Computed_field_element_integration_mapping **mapping_address)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

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
			DEALLOCATE((*mapping_address)->values);
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
  element,FE_element *,compare_pointer)
DECLARE_INDEXED_LIST_FUNCTIONS(Computed_field_element_integration_mapping)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(
	Computed_field_element_integration_mapping,
	element,FE_element *,compare_pointer)

Computed_field_node_integration_mapping *CREATE(Computed_field_node_integration_mapping)
	(FE_node *node, int number_of_components)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Creates a basic Computed_field with the given <name>. Its type is initially
COMPUTED_FIELD_CONSTANT with 1 component, returning a value of zero.
==============================================================================*/
{
	int i;
	Computed_field_node_integration_mapping *mapping_item;

	ENTER(CREATE(Computed_field_node_integration_mapping));
	
	if (node && 
		ALLOCATE(mapping_item,Computed_field_node_integration_mapping,1) &&
		ALLOCATE(mapping_item->values, FE_value, number_of_components))
	{
		mapping_item->node_ptr = ACCESS(FE_node)(node);
		for (i = 0 ; i < number_of_components ; i++)
		{
			mapping_item->values[i] = 0.0;
		}	
		mapping_item->access_count = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Computed_field_node_integration_mapping).  Not enough memory");
		mapping_item = (Computed_field_node_integration_mapping *)NULL;
	}
	LEAVE;

	return (mapping_item);
} /* CREATE(Computed_field_node_integration_mapping) */

int DESTROY(Computed_field_node_integration_mapping)
	  (Computed_field_node_integration_mapping **mapping_address)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Frees memory/deaccess mapping at <*mapping_address>.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Computed_field_node_integration_mapping));
	if (mapping_address&&*mapping_address)
	{
		if (0 >= (*mapping_address)->access_count)
		{
			if ((*mapping_address)->node_ptr)
			{
				DEACCESS(FE_node)(&((*mapping_address)->node_ptr));
			}
			DEALLOCATE((*mapping_address)->values);
			DEALLOCATE(*mapping_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Computed_field_node_integration_mapping).  Positive access_count");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Computed_field_node_integration_mapping).  Missing mapping");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Computed_field_node_integration_mapping) */

DECLARE_OBJECT_FUNCTIONS(Computed_field_node_integration_mapping)
DECLARE_LIST_TYPES(Computed_field_node_integration_mapping);
FULL_DECLARE_INDEXED_LIST_TYPE(Computed_field_node_integration_mapping);
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Computed_field_node_integration_mapping,
  node_ptr,FE_node *,compare_pointer)
DECLARE_INDEXED_LIST_FUNCTIONS(Computed_field_node_integration_mapping)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(
	Computed_field_node_integration_mapping,
	node_ptr,FE_node *,compare_pointer)

int write_Computed_field_element_integration_mapping(
	Computed_field_element_integration_mapping *mapping,
	void *user_data)
{
	USE_PARAMETER(user_data);
	printf("Mapping %p Element %p (%f)\n",
		mapping, mapping->element, mapping->values[0]);

	return( 1 );
}

char computed_field_integration_type_string[] = "integration";
char computed_field_xi_texture_coordinates_type_string[] = 
"xi_texture_coordinates";

class Computed_field_integration : public Computed_field_core
{
public:
	float cached_time;
	FE_element *seed_element;
	FE_region *fe_region;
	/* Whether to integrate wrt to each coordinate separately or wrt the 
		magnitude of the coordinate field */
	int magnitude_coordinates;
	LIST(Computed_field_element_integration_mapping) *texture_mapping;
	LIST(Computed_field_node_integration_mapping) *node_mapping;
	/* last mapping successfully used by Computed_field_find_element_xi so 
		that it can first try this element again */
	Computed_field_element_integration_mapping *find_element_xi_mapping;

	Computed_field_integration(Computed_field *field, 
		FE_element *seed_element, FE_region *fe_region, int magnitude_coordinates) : 
		Computed_field_core(field), seed_element(ACCESS(FE_element)(seed_element)),
		fe_region(ACCESS(FE_region)(fe_region)), magnitude_coordinates(magnitude_coordinates)
	{
		cached_time = 0;
		texture_mapping = 
			(LIST(Computed_field_element_integration_mapping) *)NULL;
		node_mapping = 
			(LIST(Computed_field_node_integration_mapping) *)NULL;
		find_element_xi_mapping=
			(Computed_field_element_integration_mapping *)NULL;
	};

	~Computed_field_integration();

private:
	Computed_field_core *copy(Computed_field* new_parent);

	char *get_type_string()
	{
		return(computed_field_integration_type_string);
	}

	int compare(Computed_field_core* other_field);

	int evaluate_cache_at_location(Field_location* location);

	int list();

	char* get_command_string();

	int integrate_path(FE_element *element,
		FE_value *initial_values, FE_value *initial_xi, FE_value *final_xi,
		int number_of_gauss_points, Computed_field *integrand, 
		int magnitude_coordinates, Computed_field *coordinate_field,
		float time, FE_value *values);

	int add_neighbours(
		Computed_field_element_integration_mapping *mapping_item,
		LIST(Computed_field_element_integration_mapping) *texture_mapping,
		Computed_field_element_integration_mapping_fifo **last_to_be_checked,
		Computed_field *integrand, 
		int magnitude_coordinates, Computed_field *coordinate_field,
		LIST(Index_multi_range) **node_element_list, FE_region *fe_region,
		LIST(Computed_field_element_integration_mapping) *previous_texture_mapping,
		float time_step, float time,
		LIST(Computed_field_node_integration_mapping) *node_mapping);

	int calculate_mapping(float time);

	int clear_cache();

	int is_defined_at_location(Field_location *location);

	int is_defined_at_node(FE_node *node);

	int find_element_xi(
		FE_value *values, int number_of_values, FE_element **element, 
		FE_value *xi, int element_dimension, Cmiss_region *search_region);
};

int Computed_field_integration::integrate_path(FE_element *element,
	FE_value *initial_values, FE_value *initial_xi, FE_value *final_xi,
	int number_of_gauss_points, Computed_field *integrand, 
	int magnitude_coordinates, Computed_field *coordinate_field,
	float time, FE_value *values)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Calculates the <values> by adding to the initial values the integrals evaluated
along the line from <initial_xi> to <final_xi> using the <number_of_gauss_points>
specified.
==============================================================================*/
{
	double dsdxi;
#define MAXIMUM_GAUSS_POINTS_DEFINED (2)
	FE_value final_position, initial_position,
		/* These two arrays are lower left diagonals matrices which for each row
			have the positions and weights of a gauss point scheme for integrating
			with the number of points corresponding to the row. */
		gauss_positions[MAXIMUM_GAUSS_POINTS_DEFINED][MAXIMUM_GAUSS_POINTS_DEFINED]
		   = {{0.5, 0},
			  {0.25, 0.75}},
		gauss_weights[MAXIMUM_GAUSS_POINTS_DEFINED][MAXIMUM_GAUSS_POINTS_DEFINED]
		   = {{1, 0},
			  {0.5, 0.5}},
		xi[MAXIMUM_ELEMENT_XI_DIMENSIONS],
		xi_vector[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int coordinate_dimension, j, k, m, element_dimension, return_code;
			
	ENTER(Computed_field_integration_integrate_path);
	if (integrand && coordinate_field &&
		initial_values && values && initial_xi && final_xi &&
		(number_of_gauss_points > 0) &&
		(number_of_gauss_points <= MAXIMUM_GAUSS_POINTS_DEFINED))
	{
		Field_element_xi_location location(element, xi, time, element);
		Field_element_xi_location location_with_derivatives(element, xi, time, element,
			get_FE_element_dimension(element));

		return_code = 1;
		element_dimension = get_FE_element_dimension(element);
		coordinate_dimension = Computed_field_get_number_of_components(coordinate_field);
		if (Computed_field_is_type_xi_coordinates(coordinate_field, NULL))
		{
			/* Unlike the xi field we only deal with top level elements of a 
				single dimension so we can match that dimension for our number
				of coordinates */
			coordinate_dimension = element_dimension;
		}
		if (magnitude_coordinates)
		{
			values[0] = initial_values[0];
		}
		else
		{
			for (j = 0 ; j < coordinate_dimension ; j++)
			{
				values[j] = initial_values[j];
			}
		}
		for (k = 0 ; k < element_dimension ; k++)
		{
			xi_vector[k] = final_xi[k] - initial_xi[k];
		}
		for (m = 0 ; m < number_of_gauss_points ; m++)
		{
			final_position = gauss_positions[number_of_gauss_points - 1][m];
			initial_position = (1.0 - final_position);
			for (k = 0 ; k < element_dimension ; k++)
			{
				xi[k] = initial_xi[k] * initial_position
					+ final_xi[k] * final_position;
			}
			/* Integrand elements should always be top level */
			Computed_field_evaluate_cache_at_location(integrand,
				&location);
			Computed_field_evaluate_cache_at_location(coordinate_field,
				&location_with_derivatives);

			if (magnitude_coordinates)
			{
				for (k = 0 ; k < element_dimension ; k++)
				{
					dsdxi = 0.0;
					for (j = 0 ; j < coordinate_dimension ; j++)
					{
						dsdxi += 
							coordinate_field->derivatives[j * element_dimension + k] *
							coordinate_field->derivatives[j * element_dimension + k];
					}
					dsdxi = sqrt(dsdxi);
					values[0] += integrand->values[0] * 
						xi_vector[k] * dsdxi *
						gauss_weights[number_of_gauss_points - 1][m];
				}
			}
			else
			{
				for (k = 0 ; k < element_dimension ; k++)
				{
					for (j = 0 ; j < coordinate_dimension ; j++)
					{
						values[j] += integrand->values[0] * xi_vector[k] *
							coordinate_field->derivatives[j * element_dimension + k] *
							gauss_weights[number_of_gauss_points - 1][m];
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_integration_integrate_path.  Invalid argument(s)");
		return_code=0;
	}

	return(return_code);
	LEAVE;
} /* Computed_field_integration_integrate_path */

#if defined (OLD_CODE)
int Computed_field_integration_calculate_mapping_update(
	Computed_field_element_integration_mapping *mapping_item,
	int face, Computed_field *integrand, Computed_field *coordinate_field,
	LIST(Computed_field_element_integration_mapping) *previous_texture_mapping,
	Computed_field_element_integration_mapping *seed_mapping_item,
	float time_step, float time)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

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
			length, flow_step, *temp, xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int derivative_magnitude, k, m, n, element_dimension,
		number_of_gauss_points, return_code;
	Computed_field_element_integration_mapping *previous_mapping_item;

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
					mapping_item->location, mapping_item->element,
					/*calculate_derivatives*/0);
				Computed_field_evaluate_cache_in_element(coordinate_field,
					mapping_item->location, mapping_item->element,
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
} /* Computed_field_integration_calculate_mapping_update */
#endif /* defined (OLD_CODE) */

int Computed_field_integration::add_neighbours(
	Computed_field_element_integration_mapping *mapping_item,
	LIST(Computed_field_element_integration_mapping) *texture_mapping,
	Computed_field_element_integration_mapping_fifo **last_to_be_checked,
	Computed_field *integrand, 
	int magnitude_coordinates, Computed_field *coordinate_field,
	LIST(Index_multi_range) **node_element_list, FE_region *fe_region,
	LIST(Computed_field_element_integration_mapping) *previous_texture_mapping,
	float time_step, float time,
	LIST(Computed_field_node_integration_mapping) *node_mapping)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Add the neighbours that haven't already been put in the texture_mapping list and 
puts each new member in the texture_mapping list and the to_be_checked list.
If <previous_texture_mapping> is set then this is being used to update a time
varying integration and the behaviour is significantly different.
==============================================================================*/
{
	FE_value final_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS], 
		initial_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS],
		xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int element_dimension, face_number, i, j, k, number_of_components, 
		number_of_element_field_nodes, number_of_faces,
		number_of_neighbour_elements, return_code;
	Computed_field_element_integration_mapping *mapping_neighbour;
	Computed_field_element_integration_mapping_fifo *fifo_node;
	Computed_field_node_integration_mapping *node_map;
	FE_element *integrate_element, **neighbour_elements;
	FE_element_shape *shape;
	FE_field *fe_field;
	FE_node **element_field_nodes_array;
	LIST(FE_field) *fe_field_list;

	ENTER(Computed_field_integration_add_neighbours);
	if (mapping_item && texture_mapping && last_to_be_checked &&
		(*last_to_be_checked))
	{
		return_code=1;
		element_dimension = get_FE_element_dimension(mapping_item->element);
		number_of_faces = element_dimension*2;
		if (magnitude_coordinates)
		{
			number_of_components = 1;
		}
		else
		{
			number_of_components = Computed_field_get_number_of_components(
				coordinate_field);
		}
		for (i = 0;return_code&&(i<number_of_faces);i++)
		{
			if (element_dimension == 1)
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
							Computed_field_element_integration_mapping_fifo,1)&&
							(mapping_neighbour = 
								CREATE(Computed_field_element_integration_mapping)(
									neighbour_elements[j], number_of_components)))
						{
#if defined (OLD_CODE)
							if (!previous_texture_mapping)
							{
#endif /* defined (OLD_CODE) */
								for (k = 0 ; k < element_dimension ; k++)
								{
									initial_xi[k] = 0.0;
									final_xi[k] = 0.0;
								}
								switch (i)
								{
									case 0:
									case 2:
									case 4:
									{
										initial_xi[i / 2] = 1.0;
										integrate_element = mapping_neighbour->element;
									} break;
									case 1:
									case 3:
									case 5:
									{
										final_xi[(i - 1)/ 2] = 1.0;
										integrate_element = mapping_item->element;
									} break;
								}
								integrate_path(integrate_element,
									mapping_item->values, initial_xi, final_xi,
									/*number_of_gauss_points*/2, integrand, 
									magnitude_coordinates, coordinate_field,
									time, mapping_neighbour->values);
							USE_PARAMETER(previous_texture_mapping);
							USE_PARAMETER(time_step);
#if defined (OLD_CODE)
							}
							else
							{
								/* Special time integration update step */
								Computed_field_integration_calculate_mapping_update(
									mapping_neighbour, i, integrand, coordinate_field, 
									previous_texture_mapping, mapping_item, time_step, time);
							}
#endif /* defined (OLD_CODE) */
							if (ADD_OBJECT_TO_LIST(Computed_field_element_integration_mapping)(
								mapping_neighbour, texture_mapping))
							{
								/* fill the fifo_node for the mapping_item; put at end of list */
								fifo_node->mapping_item=mapping_neighbour;
								fifo_node->next=
									(Computed_field_element_integration_mapping_fifo *)NULL;
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
		/* Try to add mappings for nodes too. */
		if ((fe_field_list = Computed_field_get_defining_FE_field_list(coordinate_field))
			&& (1 == NUMBER_IN_LIST(FE_field)(fe_field_list)))
		{
			fe_field = FIRST_OBJECT_IN_LIST_THAT(FE_field)(
				(LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL, (void *)NULL,
				fe_field_list);
		}
		else
		{
			/* We would like this to work for xi and other computed fields that
				have no fe_fields so we use the 'feature' that allows us to pass NULL */
			fe_field = (FE_field *)NULL;
		}
		if (fe_field_list)
		{
			DESTROY(LIST(FE_field))(&fe_field_list);
		}
		if (node_mapping
			&& get_FE_element_shape(mapping_item->element, &shape)
			&& FE_element_shape_is_line(shape)
			&& calculate_FE_element_field_nodes(mapping_item->element,
				fe_field, &number_of_element_field_nodes,
				&element_field_nodes_array, mapping_item->element))
		{
			/* Make assumptions about the distribution of the nodes */
			if (number_of_element_field_nodes == pow(2.0, element_dimension))
			{
				/* Add nodes not already included */
				for (i = 0 ; i < number_of_element_field_nodes ; i++)
				{
					if (!(FIND_BY_IDENTIFIER_IN_LIST(
						Computed_field_node_integration_mapping, node_ptr)
						(element_field_nodes_array[i], node_mapping)))
					{
						node_map = CREATE(Computed_field_node_integration_mapping)(
							element_field_nodes_array[i], number_of_components);
						
						for (k = 0 ; k < element_dimension ; k++)
						{
							initial_xi[k] = 0.0;
							if (i & (1 << k))
							{
								final_xi[k] = 1.0;
							}
							else
							{
								final_xi[k] = 0.0;
							}
						}
						integrate_path(mapping_item->element,
							mapping_item->values, initial_xi, final_xi,
							/*number_of_gauss_points*/2, integrand, 
							magnitude_coordinates, coordinate_field,
							time, node_map->values);
						
						ADD_OBJECT_TO_LIST(Computed_field_node_integration_mapping)(
							node_map, node_mapping);
					}
				}
			}
			/* else we don't know what we are looking at so don't do anything */
			
			for (i = 0 ; i < number_of_element_field_nodes ; i++)
			{
				DEACCESS(FE_node)(element_field_nodes_array + i);
			}
			DEALLOCATE(element_field_nodes_array);
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

int Computed_field_integration::calculate_mapping(float time)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Calculates the mapping for the specified time.
==============================================================================*/
{
	int return_code;
	Computed_field *integrand, *coordinate_field;
	Computed_field_element_integration_mapping *mapping_item;
	Computed_field_element_integration_mapping_fifo *fifo_node,
		*first_to_be_checked, *last_to_be_checked;
	LIST(Index_multi_range) *node_element_list;

	if (field && (integrand = field->source_fields[0])
		&& (coordinate_field = field->source_fields[1]))
	{
		return_code = 1;
		first_to_be_checked=last_to_be_checked=
			(Computed_field_element_integration_mapping_fifo *)NULL;
		node_element_list=(LIST(Index_multi_range) *)NULL;
		if ((texture_mapping = CREATE_LIST(Computed_field_element_integration_mapping)())
			&& (node_mapping = CREATE_LIST(Computed_field_node_integration_mapping)()))
		{
			if (ALLOCATE(fifo_node, 
				Computed_field_element_integration_mapping_fifo,1)&&
				(mapping_item=CREATE(Computed_field_element_integration_mapping)(
					 seed_element, Computed_field_get_number_of_components(field))))
			{
				ADD_OBJECT_TO_LIST(Computed_field_element_integration_mapping)
					(mapping_item, texture_mapping);
				/* fill the fifo_node for the mapping_item; put at end of list */
				fifo_node->mapping_item=mapping_item;
				fifo_node->next=
					(Computed_field_element_integration_mapping_fifo *)NULL;
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
					return_code = add_neighbours(
						first_to_be_checked->mapping_item, texture_mapping,
						&last_to_be_checked, integrand,
						magnitude_coordinates, coordinate_field,
						&node_element_list, fe_region, 
						(LIST(Computed_field_element_integration_mapping) *)NULL,
						0.0, time, node_mapping);

#if defined (DEBUG)
					printf("Item removed\n");
					write_Computed_field_element_integration_mapping(mapping_item, NULL);
#endif /* defined (DEBUG) */

					/* remove first_to_be_checked */
					fifo_node=first_to_be_checked;
					if (!(first_to_be_checked=first_to_be_checked->next))
					{
						last_to_be_checked=
							(Computed_field_element_integration_mapping_fifo *)NULL;
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
				cached_time = time;
				if (0 == NUMBER_IN_LIST(Computed_field_node_integration_mapping)(node_mapping))
				{
					DESTROY(LIST(Computed_field_node_integration_mapping)(&node_mapping));
				}
				find_element_xi_mapping=
					(Computed_field_element_integration_mapping *)NULL;
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

int Computed_field_element_integration_mapping_has_values(
	Computed_field_element_integration_mapping *mapping, void *user_data)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Compares the user_data values with the offsets in the <mapping>
==============================================================================*/
{
	int i, return_code;
	Computed_field_integration_has_values_data *data;

	ENTER(Computed_field_element_integration_mapping_has_values);
	if (mapping && (data = 
			(Computed_field_integration_has_values_data *)user_data))
	{
		return_code = 1;
		for (i = 0 ; return_code && (i < data->number_of_values) ; i++)
		{
			if (data->values[i] != mapping->values[i])
			{
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_element_integration_mapping_has_values.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_element_integration_mapping_has_values */

Computed_field_integration::~Computed_field_integration()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{

	ENTER(Computed_field_integration::~Computed_field_integration);
	if (field)
	{
		if (fe_region)
		{
			DEACCESS(FE_region)(&fe_region);
		}
		if (seed_element)
		{
			DEACCESS(FE_element)(&(seed_element));
		}
		if (texture_mapping)
		{
			DESTROY_LIST(Computed_field_element_integration_mapping)
				(&texture_mapping);
		}
		if (node_mapping)
		{
			DESTROY_LIST(Computed_field_node_integration_mapping)
				(&node_mapping);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_integration::~Computed_field_integration.  "
			"Invalid arguments.");
	}
	LEAVE;

} /* Computed_field_integration::~Computed_field_integration */

Computed_field_core* Computed_field_integration::copy(Computed_field* new_parent)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	Computed_field_integration *core;

	ENTER(Computed_field_integration::copy);
	if (new_parent)
	{
		core = new Computed_field_integration(new_parent,
			seed_element, fe_region, magnitude_coordinates);
	}
	else
	{
		core = (Computed_field_integration*)NULL;
	}
	LEAVE;

	return (core);
} /* Computed_field_integration::copy */

int Computed_field_integration::clear_cache()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_integration::clear_cache);
	if (field)
	{
		if (find_element_xi_mapping)
		{
			DEACCESS(Computed_field_element_integration_mapping)
				(&find_element_xi_mapping);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_integration::clear_cache.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return(return_code);
} /* Computed_field_integration::clear_cache */

int Computed_field_integration::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;
	Computed_field_integration *other;

	ENTER(Computed_field_integration::compare);
	if (field && (other = dynamic_cast<Computed_field_integration*>(other_core)))
	{
		if (seed_element == other->seed_element)
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
} /* Computed_field_integration::compare */

int Computed_field_integration::is_defined_at_location(Field_location *location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns 1 if the all the source fields are defined in the supplied <element> and
the mapping is defined for this element.
==============================================================================*/
{
	FE_value element_to_top_level[9];
	int return_code;
	CM_element_information cm;
	FE_element *top_level_element;

	ENTER(Computed_field_default::is_defined_at_location);
	if (field && location)
	{
		if (return_code = Computed_field_core::is_defined_at_location(
			location))
		{
			Field_element_xi_location *element_xi_location;
			Field_node_location *node_location;
			
			if (element_xi_location = 
				dynamic_cast<Field_element_xi_location*>(location))
			{
				FE_element* element = element_xi_location->get_element();
				
				if (!texture_mapping)
				{
					/* Try time 0 */
					calculate_mapping(/*time*/0.0);
				}
				else
				{
					/* Use the mapping from whatever time */
				}
				/* 1. Get top_level_element for types that must be calculated on them */
				get_FE_element_identifier(element, &cm);
				if (CM_ELEMENT == cm.type)
				{
					top_level_element=element;
				}
				else
				{
					/* check or get top_level element, 
						we don't have a top_level element to test with */
					if (!(top_level_element=FE_element_get_top_level_element_conversion(
						element,(struct FE_element *)NULL,(struct LIST(FE_element) *)NULL,
						-1,element_to_top_level)))
					{
						return_code=0;
					}
				}
				/* 2. Calculate the field */
				if (return_code && texture_mapping)
				{
					if (FIND_BY_IDENTIFIER_IN_LIST
						(Computed_field_element_integration_mapping,element)
						(top_level_element, texture_mapping))
					{
						return_code = 1;
					}
					else
					{
						return_code = 0;
					}
				}
			}
			else if (node_location = 
				dynamic_cast<Field_node_location*>(location))
			{
				FE_node *node = node_location->get_node();

				if (!texture_mapping)
				{
					/* Try time 0 */
					calculate_mapping(/*time*/0.0);
				}
				else
				{
					/* Use the mapping from whatever time */
				}
				/* If we calculated the mapping but no node_mapping was created then
					the field is not defined at nodes. */
				if (node_mapping)
				{
					/* If we have a mapping look for the node */
					if (FIND_BY_IDENTIFIER_IN_LIST
						(Computed_field_node_integration_mapping,node_ptr)
						(node, node_mapping))
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
			}
			else
			{
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_default::is_defined_at_location.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_default::is_defined_at_location */
	
int Computed_field_integration::is_defined_at_node(FE_node *node)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns 1 if the mapping is defined for this node.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_default_is_defined_at_node);
	if (field && node)
	{
		if (!texture_mapping)
		{
			/* Try time 0 */
			calculate_mapping(/*time*/0.0);
		}
		else
		{
			/* Use the mapping from whatever time */
		}
		/* If we calculated the mapping but no node_mapping was created then
		 the field is not defined at nodes. */
		if (node_mapping)
		{
			/* If we have a mapping look for the node */
			if (FIND_BY_IDENTIFIER_IN_LIST
				(Computed_field_node_integration_mapping,node_ptr)
				(node, node_mapping))
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
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_default_is_defined_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_default_is_defined_at_node */

int Computed_field_integration::evaluate_cache_at_location(
    Field_location* location)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Evaluate the fields cache at the location
==============================================================================*/
{
	double dsdxi;
	FE_value element_to_top_level[9],initial_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS],
		top_level_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int coordinate_dimension, element_dimension, i, j, k, return_code, 
		top_level_element_dimension;
	CM_element_information cm;
	Computed_field *coordinate_field, *integrand;

	ENTER(Computed_field_integration::evaluate_cache_at_location);
	if (field && location)
	{
		return_code = 1;

		Field_element_xi_location *element_xi_location;
		Field_node_location *node_location;

		if (element_xi_location = 
			dynamic_cast<Field_element_xi_location*>(location))
		{
			FE_element* element = element_xi_location->get_element();
 			FE_element* top_level_element = element_xi_location->get_top_level_element();
			FE_value time = element_xi_location->get_time();
			FE_value* xi = element_xi_location->get_xi();
			int number_of_derivatives = location->get_number_of_derivatives();

			Computed_field_element_integration_mapping *mapping;

			if (!texture_mapping)
			{
				calculate_mapping(time);
			}
			else
			{
				if ((time != cached_time)
					&& (Computed_field_has_multiple_times(field->source_fields[0])
						|| Computed_field_has_multiple_times(field->source_fields[1])))
				{
					DESTROY_LIST(Computed_field_element_integration_mapping)
						(&texture_mapping);
					calculate_mapping(time);
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
						"Computed_field_integration::evaluate_cache_at_location.  "
						"No top-level element found to evaluate field %s on",
						field->name);
					return_code=0;
				}
			}
			integrand = field->source_fields[0];
			coordinate_field = field->source_fields[1];
			coordinate_dimension = 
				Computed_field_get_number_of_components(field->source_fields[1]);
			if (Computed_field_is_type_xi_coordinates(field->source_fields[1], NULL))
			{
				/* Unlike the xi field we only deal with top level elements of a 
					single dimension so we can match that dimension for our number
					of coordinates */
				coordinate_dimension = top_level_element_dimension;
			}
			/* 2. Calculate the field */
			if (texture_mapping)
			{
				if (mapping = FIND_BY_IDENTIFIER_IN_LIST
					(Computed_field_element_integration_mapping,element)
					(top_level_element, texture_mapping))
				{
					/* Integrate to the specified top_level_xi location */
					for (i = 0 ; i < top_level_element_dimension ; i++)
					{
						initial_xi[i] = 0.0;
					}
					integrate_path(top_level_element,
						mapping->values, initial_xi, top_level_xi,
						/*number_of_gauss_points*/2, field->source_fields[0], 
						magnitude_coordinates, field->source_fields[1],
						time, field->values);
					if (number_of_derivatives)
					{
						Field_element_xi_location top_level_location(
							top_level_element, top_level_xi, time,
							top_level_element);
						Field_element_xi_location top_level_location_with_derivatives(
							top_level_element, top_level_xi, time,
							top_level_element, get_FE_element_dimension(element));

						/* Evaluate the fields at this location */
						Computed_field_evaluate_cache_at_location(integrand,
							&top_level_location);
						Computed_field_evaluate_cache_at_location(coordinate_field,
							&top_level_location_with_derivatives);
						if (magnitude_coordinates)
						{
							for (k = 0 ; k < element_dimension ; k++)
							{
								dsdxi = 0.0;
								for (j = 0 ; j < coordinate_dimension ; j++)
								{
									dsdxi += 
										coordinate_field->derivatives[j * element_dimension + k] *
										coordinate_field->derivatives[j * element_dimension + k];
								}
								dsdxi = sqrt(dsdxi);
								field->derivatives[k] = integrand->values[0] * dsdxi;
							}
						}
						else
						{
							for (k = 0 ; k < element_dimension ; k++)
							{
								for (j = 0 ; j < coordinate_dimension ; j++)
								{
									field->derivatives[j * element_dimension + k] = 
										integrand->values[0] *
										coordinate_field->derivatives[j * element_dimension + k];
								}
							}
						}
						field->derivatives_valid = 1;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_integration::evaluate_cache_at_location."
						"  Element %d not found in Xi texture coordinate mapping field %s",
						cm.number, field->name);
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_integration::evaluate_cache_at_location.  "
					"Xi texture coordinate mapping not calculated");
				return_code=0;
			}
		}
		else if (node_location = 
			dynamic_cast<Field_node_location*>(location))
		{
			FE_node *node = node_location->get_node();
			FE_value time = node_location->get_time();

			Computed_field_node_integration_mapping *mapping;

			return_code = 1;

			if (!texture_mapping)
			{
				calculate_mapping(time);
			}
			else
			{
				if ((time != cached_time)
					&& (Computed_field_has_multiple_times(field->source_fields[0])
						|| Computed_field_has_multiple_times(field->source_fields[1])))
				{
					DESTROY_LIST(Computed_field_element_integration_mapping)
						(&texture_mapping);
					calculate_mapping(time);
				}
			}
			/* 2. Calculate the field */
			if (node_mapping)
			{
				if (mapping = FIND_BY_IDENTIFIER_IN_LIST
					(Computed_field_node_integration_mapping,node_ptr)
					(node, node_mapping))
				{
					for(i = 0 ; i < field->number_of_components ; i++)
					{
						field->values[i] = mapping->values[i];
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_integration_evaluate_cache_at_node."
						"  Node %d not found in Xi texture coordinate mapping field %s",
						get_FE_node_identifier(node));
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_integration_evaluate_cache_at_node.  "
					"Xi texture coordinate mapping not calculated");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_integration::evaluate_cache_at_location.  "
				"Location type unknown or not implemented.");
			return_code = 0;
		}
		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_integration::evaluate_cache_at_location.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_integration::evaluate_cache_at_location */


int Computed_field_integration::find_element_xi(
	FE_value *values, int number_of_values, FE_element **element, 
	FE_value *xi, int element_dimension, Cmiss_region *search_region)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	FE_value floor_values[3];
	int i, return_code;
	Computed_field_element_integration_mapping *mapping;
	Computed_field_integration_has_values_data has_values_data;
	FE_region *fe_region;

	ENTER(Computed_field_integration::find_element_xi);
	if (field&&values&&(number_of_values==field->number_of_components)&&element&&xi&&
		search_region && (fe_region = Cmiss_region_get_FE_region(search_region)))
	{
		if (!texture_mapping)
		{
			calculate_mapping(/*time*/0);
		}
		else
		{
#if defined (NEW_CODE)
			/* Until we are calculating this at the correct time there is no
				point seeing if the time has changed */
			if ((time != cached_time)
				&& (Computed_field_has_multiple_times(field->source_fields[0])
				|| Computed_field_has_multiple_times(field->source_fields[1])))
			{
				DESTROY_LIST(Computed_field_element_integration_mapping)
					(&texture_mapping);
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
			has_values_data.values = floor_values;
			has_values_data.number_of_values = number_of_values;
			if (texture_mapping)
			{
				return_code=0;
				/* Check the last successful mapping first */
				if (find_element_xi_mapping&&
					Computed_field_element_integration_mapping_has_values(
						find_element_xi_mapping, (void *)&has_values_data))
				{
					return_code = 1;
				}
				else
				{
					/* Find in the list */
					if (mapping = FIRST_OBJECT_IN_LIST_THAT(Computed_field_element_integration_mapping)
						(Computed_field_element_integration_mapping_has_values, (void *)&has_values_data,
							texture_mapping))
					{
						REACCESS(Computed_field_element_integration_mapping)(&(find_element_xi_mapping),
							mapping);
						return_code = 1;
					}
				}
				if (return_code)
				{
					*element = find_element_xi_mapping->element;
					for (i = 0 ; i < get_FE_element_dimension(*element) ; i++)
					{
						xi[i] = values[i] - floor_values[i];	
					}
					if (!FE_region_contains_FE_element(fe_region, *element))
					{
						*element = (FE_element *)NULL;
						display_message(ERROR_MESSAGE,
							"Computed_field_integration::find_element_xi.  "
							"Element is not in specified group");
						return_code=0;
					}
					if (element_dimension && (element_dimension != get_FE_element_dimension(*element)))
					{
						*element = (FE_element *)NULL;
						display_message(ERROR_MESSAGE,
							"Computed_field_integration::find_element_xi.  "
							"Element is not of the required dimension");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_integration::find_element_xi.  "
						"Unable to find mapping for given values");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_integration::find_element_xi.  "
					"Xi texture coordinate mapping not calculated");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_integration::find_element_xi.  "
				"Only implemented for three or less values");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_integration::find_element_xi.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_integration::find_element_xi */

int Computed_field_integration::list(
	)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_integration);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    seed_element : %d\n",
			FE_element_get_cm_number(seed_element));
		display_message(INFORMATION_MESSAGE,"    integrand field : %s\n",
			field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,"    coordinate field : %s\n",
			field->source_fields[1]->name);
		if (magnitude_coordinates)
		{
			display_message(INFORMATION_MESSAGE,"    magnitude_coordinates : true\n");
		}
		else
		{
			display_message(INFORMATION_MESSAGE,"    magnitude_coordinates : false\n");
		}
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

char *Computed_field_integration::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error, xi_texture_coordinates;

	ENTER(Computed_field_integration::get_command_string);
	command_string = (char *)NULL;
	if (field)
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
		sprintf(temp_string, " seed_element %d", FE_element_get_cm_number(seed_element));
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
		if (magnitude_coordinates)
		{
			append_string(&command_string, " magnitude_coordinates", &error);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_integration::get_command_string.  "
			"Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_integration::get_command_string */

#if defined (OLD_CODE)
int Computed_field_update_integration_scheme(
	FE_region *fe_region,
	Computed_field *integrand, Computed_field *coordinate_field,
	float time_step)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
This is a special method for updating with a time integration step for flow
problems.  It takes a current integration field and updates it to the next
timestep.
=============================================================================*/
{
	int return_code;
	Computed_field_element_integration_mapping *mapping_item, *previous_mapping_item,
		*seed_mapping_item;
	Computed_field_element_integration_mapping_fifo *fifo_node,
		*first_to_be_checked, *last_to_be_checked;
	LIST(Computed_field_element_integration_mapping) *texture_mapping;
	LIST(Index_multi_range) *node_element_list;

	ENTER(Computed_field_update_integration_scheme);
	if (field&&Computed_field_is_type_integration(field) && (data = 
		(Computed_field_integration_type_specific_data *)
		field->type_specific_data)&&integrand&&coordinate_field)
	{
		return_code=1;
		first_to_be_checked=last_to_be_checked=
			(Computed_field_element_integration_mapping_fifo *)NULL;
		node_element_list=(LIST(Index_multi_range) *)NULL;
		texture_mapping = CREATE_LIST(Computed_field_element_integration_mapping)();
		if (ALLOCATE(fifo_node,
			Computed_field_element_integration_mapping_fifo,1)&&
			(mapping_item=CREATE(Computed_field_element_integration_mapping)(
				 seed_element, field->number_of_components)))
		{
			previous_mapping_item = FIND_BY_IDENTIFIER_IN_LIST
				(Computed_field_element_integration_mapping,element)
				(seed_element, texture_mapping);
			seed_mapping_item = CREATE(Computed_field_element_integration_mapping)(
				(FE_element *)NULL, field->number_of_components);
			seed_mapping_item->values[0] = previous_mapping_item->values[0];
			seed_mapping_item->differentials[0] = time_step;
			Computed_field_integration_calculate_mapping_update(
				mapping_item, 1, integrand, coordinate_field,
				texture_mapping, seed_mapping_item, time_step, /*time*/0);
			DESTROY(Computed_field_element_integration_mapping)(&seed_mapping_item);
			ADD_OBJECT_TO_LIST(Computed_field_element_integration_mapping)
				(mapping_item, texture_mapping);
			/* fill the fifo_node for the mapping_item; put at end of list */
			fifo_node->mapping_item=mapping_item;
			fifo_node->next=
				(Computed_field_element_integration_mapping_fifo *)NULL;
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
					&last_to_be_checked, integrand,
					magnitude_coordinates, coordinate_field,
					&node_element_list, fe_region, texture_mapping,
					time_step, /*time*/0,
					(LIST(Computed_field_node_integration_mapping) *)NULL);

				/* remove first_to_be_checked */
				fifo_node=first_to_be_checked;
				if (!(first_to_be_checked=first_to_be_checked->next))
				{
					last_to_be_checked=
						(Computed_field_element_integration_mapping_fifo *)NULL;
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
			DESTROY_LIST(Computed_field_element_integration_mapping)(&texture_mapping);
			texture_mapping = texture_mapping;
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
#endif /* defined (OLD_CODE) */

} //namespace

int Computed_field_is_type_integration(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns true if <field> has the appropriate static type string.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_integration);
	if (field)
	{
		if (dynamic_cast<Computed_field_integration*>(field->core))
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
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_integration.  Missing field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_integration */

int Computed_field_set_type_integration(Computed_field *field,
	FE_element *seed_element,
	FE_region *fe_region, Computed_field *integrand, 
	int magnitude_coordinates, Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_INTEGRATION.
The seed element is set to the number given and the mapping calculated.
Sets the number of components to be the same as the <integrand> field.
The <integrand> is the value that is integrated over each element and the
<coordinate_field> is used to define the arc length differential for each element.
Currently only two gauss points are supported, a linear integration.
If <magnitude_coordinates> is false then the resulting field has the same number
of components as the <coordinate_field> and each component is the integration
with respect to each of the components, if <magnitude_components> is true then
the field will have a single component and the magnitude of the <coordinate_field>
derivatives are used to calculate arc lengths at each gauss pointa.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
=============================================================================*/
{
	int number_of_source_fields, return_code;
	Computed_field **source_fields;
	CM_element_information cm;
	FE_element *element;

	ENTER(Computed_field_set_type_integration);
	if (field&&seed_element&&integrand&&coordinate_field&&
		(1==Computed_field_get_number_of_components(integrand)))
	{
		return_code=1;
		number_of_source_fields=2;
		/* 1. make dynamic allocations for any new type-specific data */
		if ((ALLOCATE(source_fields,Computed_field *,
			number_of_source_fields)))
		{
			/* Add seed element */
			get_FE_element_identifier(seed_element, &cm);
			if (!((element=FE_region_get_FE_element_from_identifier(
						 fe_region, &cm)) && (element==seed_element)))
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
			if (magnitude_coordinates)
			{
				field->number_of_components = 1;
			}
			else
			{
				field->number_of_components = coordinate_field->number_of_components;
				if (Computed_field_is_type_xi_coordinates(coordinate_field, NULL))
				{
					/* Unlike the xi field we only deal with top level elements of a 
						single dimension so we can match that dimension for our number
						of coordinates */
					field->number_of_components = get_FE_element_dimension(seed_element);
				}
			}
			/* source_fields: 0=integrand, 1=coordinate_field */
			source_fields[0]=ACCESS(Computed_field)(integrand);
			source_fields[1]=ACCESS(Computed_field)(coordinate_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			field->core = new Computed_field_integration(field,
				seed_element, fe_region, magnitude_coordinates);
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

int Computed_field_get_type_integration(Computed_field *field,
	FE_element **seed_element, Computed_field **integrand,
	int *magnitude_coordinates, Computed_field **coordinate_field)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
If the field is of type COMPUTED_FIELD_INTEGRATION, 
the seed element used for the mapping is returned - otherwise an error is reported.
==============================================================================*/
{
	Computed_field_integration* core;
	int return_code;

	ENTER(Computed_field_get_type_integration);
	if (field&&(core = dynamic_cast<Computed_field_integration*>(field->core)))
	{
		*seed_element=core->seed_element;
		*integrand=field->source_fields[0];
		*magnitude_coordinates=core->magnitude_coordinates;
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

int define_Computed_field_type_integration(Parse_state *state,
	void *field_void,void *computed_field_integration_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_INTEGRATION (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	char magnitude_coordinates_flag;
	char* region_path;
	Cmiss_region *region;
	Computed_field *coordinate_field, *field, *integrand;
	Computed_field_integration_package *computed_field_integration_package;
	FE_value value;
	FE_element *seed_element;	
	float time_update;
	int expected_parameters, magnitude_coordinates, previous_state_index, 
		return_code;
	Option_table *option_table;
	Set_Computed_field_conditional_data set_coordinate_field_data,
		set_integrand_field_data;

	ENTER(define_Computed_field_type_integration);
	if (state&&(field=(Computed_field *)field_void)&&
		(computed_field_integration_package=
			(Computed_field_integration_package *)computed_field_integration_package_void))
	{
		return_code=1;
		coordinate_field=(Computed_field *)NULL;
		integrand=(Computed_field *)NULL;
		magnitude_coordinates = 0;
		seed_element = (FE_element *)NULL;
		time_update = 0;
		if (Computed_field_is_type_integration(field))
		{
			return_code = Computed_field_get_type_integration(field,
				&seed_element, &integrand, &magnitude_coordinates, &coordinate_field);
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
			else
			{
				/* Make a default integrand of one */
				value = 1.0;
				if (!((integrand = ACCESS(Computed_field)(CREATE(Computed_field)("constant_1.0"))) &&
						Computed_field_set_type_constant(integrand,1,&value)))
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_xi_texture_coordinates.  Unable to create constant field");
					return_code=0;
				}
			}
			Cmiss_region_get_root_region_path(&region_path);
			if (seed_element)
			{
				ACCESS(FE_element)(seed_element);
			}
			magnitude_coordinates_flag = 0;
			if ((!state->current_token) ||
				(strcmp(PARSER_HELP_STRING, state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
			{
				if (return_code)
				{
					previous_state_index = state->current_index;

					option_table = CREATE(Option_table)();
					/* region */
					Option_table_add_set_Cmiss_region_path(option_table, "region", 
						computed_field_integration_package->root_region, &region_path);
					/* Ignore everything else */
					Option_table_ignore_all_unmatched_entries(option_table);
					return_code = Option_table_multi_parse(option_table,state);
					DESTROY(Option_table)(&option_table);

					/* Return back to where we were */
					shift_Parse_state(state, previous_state_index - state->current_index);
				}
				if (return_code)
				{
					return_code = Cmiss_region_get_region_from_path(
						computed_field_integration_package->root_region, 
						region_path, &region);
					ACCESS(Cmiss_region)(region);
				}
				if (return_code)
				{
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
					/* magnitude_coordinates */
					Option_table_add_char_flag_entry(option_table, "magnitude_coordinates",
						&magnitude_coordinates_flag);
					/* region, ignore it this time */
					expected_parameters = 1;
					Option_table_add_ignore_token_entry(option_table, "region",
						&expected_parameters);
					/* seed_element */
					Option_table_add_entry(option_table,"seed_element",
						&seed_element, Cmiss_region_get_FE_region(region),
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
						if (magnitude_coordinates_flag)
						{
							/* Currently no flag to turn this off again */
							magnitude_coordinates = 1;
						}
					}
					if (return_code)
					{
						if (time_update && Computed_field_is_type_integration(field))
						{
							display_message(ERROR_MESSAGE,
								"The update_time_integration code has not been updated"
								"with the latest changes.");
							return_code=0;
#if defined (OLD_CODE)					
							return_code=Computed_field_update_integration_scheme(field,
								Cmiss_region_get_FE_region(region),
								integrand, coordinate_field, time_update);
#endif /* defined (OLD_CODE) */
						}
						else
						{
							return_code=Computed_field_set_type_integration(field,
								seed_element, Cmiss_region_get_FE_region(region),
								integrand, magnitude_coordinates, coordinate_field);
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			else
			{
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
				/* magnitude_coordinates */
				Option_table_add_char_flag_entry(option_table, "magnitude_coordinates",
					&magnitude_coordinates_flag);
				/* region */
				Option_table_add_set_Cmiss_region_path(option_table, "region", 
					computed_field_integration_package->root_region, &region_path);
				/* seed_element */
				Option_table_add_entry(option_table,"seed_element",
					&seed_element,
					Cmiss_region_get_FE_region(computed_field_integration_package->root_region),
					set_FE_element_top_level_FE_region);
				/* update_time_integration */
				Option_table_add_entry(option_table,"update_time_integration",
					&time_update, NULL, set_float);
				return_code = Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
					
			}
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

int define_Computed_field_type_xi_texture_coordinates(Parse_state *state,
	void *field_void,void *computed_field_integration_package_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_XI_TEXTURE_COORDINATES (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	char* region_path;
	Cmiss_region* region;
	FE_value value;
	int expected_parameters, previous_state_index, return_code;
	Computed_field *coordinate_field, *field, *integrand;
	Computed_field_integration_package *computed_field_integration_package;
	FE_element *seed_element;	
	Option_table *option_table;

	ENTER(define_Computed_field_type_xi_texture_coordinates);
	if (state&&(field=(Computed_field *)field_void)&&
		(computed_field_integration_package=
			(Computed_field_integration_package *)computed_field_integration_package_void))
	{
		region = (struct Cmiss_region *)NULL;
		region_path = (char *)NULL;
		coordinate_field = (Computed_field *)NULL;
		integrand = (Computed_field *)NULL;
		return_code=1;
		if (!(coordinate_field = ACCESS(Computed_field)(
			FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
			Computed_field_is_type_xi_coordinates, (void *)NULL,
			computed_field_integration_package->computed_field_manager))))
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
		Cmiss_region_get_root_region_path(&region_path);
		seed_element = (FE_element *)NULL;
		if ((!state->current_token) ||
			(strcmp(PARSER_HELP_STRING, state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
		{
			if (return_code)
			{
				previous_state_index = state->current_index;

				option_table = CREATE(Option_table)();
				/* region */
				Option_table_add_set_Cmiss_region_path(option_table, "region", 
					computed_field_integration_package->root_region, &region_path);
				/* Ignore everything else */
				Option_table_ignore_all_unmatched_entries(option_table);
				return_code = Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);

				/* Return back to where we were */
				shift_Parse_state(state, previous_state_index - state->current_index);
			}
			if (return_code)
			{
				return_code = Cmiss_region_get_region_from_path(
					computed_field_integration_package->root_region, 
					region_path, &region);
				ACCESS(Cmiss_region)(region);
			}
			if (return_code)
			{
				option_table = CREATE(Option_table)();
				/* region, ignore it this time */
				expected_parameters = 1;
				Option_table_add_ignore_token_entry(option_table, "region",
					&expected_parameters);
				/* seed_element */
				Option_table_add_entry(option_table,"seed_element",
					&seed_element, Cmiss_region_get_FE_region(region),
					set_FE_element_top_level_FE_region);
				if (return_code = Option_table_multi_parse(option_table,state))
				{
					return_code=Computed_field_set_type_integration(field,
						seed_element, Cmiss_region_get_FE_region(region),
						integrand, /*magnitude_coordinates*/0, coordinate_field);
				}
				DESTROY(Option_table)(&option_table);
			}
		}
		else
		{
			/* Help */
			option_table = CREATE(Option_table)();
			/* region */
			Option_table_add_set_Cmiss_region_path(option_table, "region", 
				computed_field_integration_package->root_region, &region_path);
			/* seed_element */
			Option_table_add_entry(option_table,"seed_element",
				&seed_element,
				Cmiss_region_get_FE_region(computed_field_integration_package->root_region),
				set_FE_element_top_level_FE_region);
			return_code = Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
		}
		if (coordinate_field)
		{
			DEACCESS(Computed_field)(&coordinate_field);
		}
		if (integrand)
		{
			DEACCESS(Computed_field)(&integrand);
		}
		if (region)
		{
			DEACCESS(Cmiss_region)(&region);
		}
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
		if (seed_element)
		{
			DEACCESS(FE_element)(&seed_element);
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
	Computed_field_package *computed_field_package,
	Cmiss_region *root_region)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static Computed_field_integration_package 
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

