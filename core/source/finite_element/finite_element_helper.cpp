/***************************************************************************//**
 * FILE : finite_element_helper.h
 *
 * Convenience functions for making simple finite element fields.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "finite_element/finite_element.h"
#include "finite_element/finite_element_helper.h"
#include "finite_element/finite_element_region.h"
#include "general/debug.h"
#include "general/message.h"

FE_field *FE_field_create_coordinate_3d(struct FE_region* fe_region,
	const char *name)
{
	FE_field *return_field = NULL;
	
	ENTER(FE_field_create_coordinate_3d);
	if (name)
	{
		// create FE_field for interpolated coordinates
		FE_field *coordinate_field = CREATE(FE_field)(name, fe_region);
		
		// add a reference count
		ACCESS(FE_field)(coordinate_field);

		// this field has real values (as opposed to integer, string)
		set_FE_field_value_type(coordinate_field, FE_VALUE_VALUE);

		// 3 components named x, y, z; rectangular cartesian
		const int number_of_components = 3;
		set_FE_field_number_of_components(coordinate_field, number_of_components);
		const char *component_names[] = { "x", "y", "z" };
		for (int i = 0; i < number_of_components; i++)
		{
			set_FE_field_component_name(coordinate_field, i, component_names[i]);
		}
		Coordinate_system coordinate_system;
		coordinate_system.type = RECTANGULAR_CARTESIAN;
		set_FE_field_coordinate_system(coordinate_field, &coordinate_system);

		// field type 'general' is nodal-interpolated.
		set_FE_field_type_general(coordinate_field);

		// set flag indicating this is a coordinate field type
		// (helps cmgui automatically choose this field for that purpose)
		set_FE_field_CM_field_type(coordinate_field, CM_COORDINATE_FIELD);

		// see if a matching field exists in fe_region, or add this one
		return_field = FE_region_merge_FE_field(fe_region, coordinate_field);
		ACCESS(FE_field)(return_field);

		// remove reference count to clean up (handles return_field != coordinate_field)
		DEACCESS(FE_field)(&coordinate_field);
	}
	LEAVE;

	return (return_field);
}

struct FE_element *FE_element_create_with_simplex_shape(
	struct FE_region *fe_region, int dimension)
{
	ENTER(FE_element_create_with_simplex_shape);
	FE_element *element = (struct FE_element *)NULL;
	if (fe_region && ((dimension == 2) || (dimension == 3)))
	{
		const int triangle_shape_type[] = { SIMPLEX_SHAPE, 1, SIMPLEX_SHAPE };
		const int tetrahedron_shape_type[] = { SIMPLEX_SHAPE, 1, 1, SIMPLEX_SHAPE, 1, SIMPLEX_SHAPE };
		FE_element_shape *element_shape = CREATE(FE_element_shape)(dimension,
			(dimension == 2) ? triangle_shape_type : tetrahedron_shape_type,
			fe_region); 
		ACCESS(FE_element_shape)(element_shape);

		CM_element_information element_identifier;
		element_identifier.type = CM_ELEMENT;
		element_identifier.number = 1;
		element = CREATE(FE_element)(&element_identifier, element_shape, fe_region,
			/*template_element*/(FE_element *)NULL);
		ACCESS(FE_element)(element);
	
		DEACCESS(FE_element_shape)(&element_shape);
	}
	LEAVE;

	return (element);
}

int FE_element_define_field_simple(struct FE_element *element,
	struct FE_field *field, enum FE_basis_type basis_type)
{
	ENTER(FE_element_define_field_simple);
	int return_code = 1;
	if (element && field)
	{
		if (GENERAL_FE_FIELD != get_FE_field_FE_field_type(field))
		{
			display_message(ERROR_MESSAGE,
				"FE_element_define_field_simple.  Field must be FE_field_type general");
			return_code = 0;
		}

		// check supported basis types and get FE_basis from element's region
		int basis_relation = NO_RELATION;
		switch (basis_type)
		{
		case LINEAR_LAGRANGE:
		case QUADRATIC_LAGRANGE:
		case CUBIC_LAGRANGE:
			basis_relation = NO_RELATION;
			break;
		case LINEAR_SIMPLEX:
		case QUADRATIC_SIMPLEX:
			basis_relation = 1; // linked in a simplex
			break;
		default:
			display_message(ERROR_MESSAGE,
				"FE_element_define_field_simple.  Does not support basis type %s",
				FE_basis_type_string(basis_type));
			return_code = 0;
			break;
		}
		const int element_dimension = get_FE_element_dimension(element);
		int *basis_type_array = NULL;
		if (ALLOCATE(basis_type_array, int,
			1 + (element_dimension*(1 + element_dimension)) / 2))
		{
			basis_type_array[0] = element_dimension;
			int *basis_type_entry = basis_type_array + 1;
			for (int j = 0; j < element_dimension; j++)
			{
				*basis_type_entry = basis_type;
				basis_type_entry++;
				for (int i = element_dimension-1; i > j; i--)
				{
					*basis_type_entry = basis_relation;
					basis_type_entry++;
				}
			}
		}
		else
		{
			return_code = 0;
		}
		FE_basis *basis = FE_region_get_FE_basis_matching_basis_type(
			FE_element_get_FE_region(element), basis_type_array);
		ACCESS(FE_basis)(basis);
		DEALLOCATE(basis_type_array);
		if (!basis)
		{
			display_message(ERROR_MESSAGE,
				"FE_element_define_field_simple.  Failed to create basis");
			return_code = 0;
		}

		// check element has enough nodes to support this basis
		int number_of_basis_functions = FE_basis_get_number_of_functions(basis);
		// following is true for the basis types supported by this function:
		const int basis_number_of_nodes = number_of_basis_functions;
		const int max_referenced_nodes = basis_number_of_nodes;
		int element_number_of_nodes = 0;
		get_FE_element_number_of_nodes(element, &element_number_of_nodes);
		if (element_number_of_nodes < max_referenced_nodes)
		{
			display_message(ERROR_MESSAGE, "FE_element_define_field_simple.  "
				"Element has %d nodes, needs %d for %d dimensional %s basis",
				element_number_of_nodes, max_referenced_nodes, element_dimension,
				FE_basis_type_string(basis_type));
			return_code = 0;
		}

		if (return_code)
		{
			const int number_of_components = get_FE_field_number_of_components(field);
			FE_element_field_component **components;
			ALLOCATE(components, struct FE_element_field_component *, number_of_components);
			if (components)
			{
				for (int comp_no = 0; comp_no < number_of_components; comp_no++)
				{
					components[comp_no] = (struct FE_element_field_component *)NULL;
				}
				for (int comp_no = 0; comp_no < number_of_components; comp_no++)
				{
					components[comp_no] = CREATE(FE_element_field_component)(STANDARD_NODE_TO_ELEMENT_MAP,
						basis_number_of_nodes, basis, /*modify*/NULL);
					for (int i = 0; i < basis_number_of_nodes; i++)
					{
						// index within the list of nodes in element at which to get values
						const int element_node_index = i;
						Standard_node_to_element_map *standard_node_map =
							Standard_node_to_element_map_create(element_node_index, /*number_of_values*/1);
						Standard_node_to_element_map_set_nodal_value_type(
							standard_node_map, /*element_value_index*/0, FE_NODAL_VALUE);
						/* scale_factor_index of -1 means use default unit scale factor */
						Standard_node_to_element_map_set_scale_factor_index(standard_node_map,
							/*element_value_index*/0, /*scale_factor_index*/-1);
						FE_element_field_component_set_standard_node_map(components[comp_no], /*basis_node_index*/i, standard_node_map);
					}
				}
				return_code = define_FE_field_at_element(element, field, components);
				for (int comp_no = 0; comp_no < number_of_components; comp_no++)
				{
					DESTROY(FE_element_field_component)(&(components[comp_no]));
				}
				DEALLOCATE(components);
			}
			else
			{
				return_code = 0;
			}
		}
		DEACCESS(FE_basis)(&basis);
	}
	else
	{
		return_code = 0;
	}
	LEAVE;
	
	return (return_code);
} /* FE_element_define_field_simple */
