/*******************************************************************************
FILE : export_cm_files.c

LAST MODIFIED : 22 March 2006

DESCRIPTION :
Functions for exporting finite element data to a file.
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
#include <stdio.h>
#include "finite_element/finite_element.h"
#include "general/debug.h"
#include "general/geometry.h"
#include "user_interface/message.h"
#include "finite_element/export_cm_files.h"

/*
Module types
------------
*/

struct write_cm_files_data
{
	int number_of_bases;
	struct FE_basis **basis_array;
}; /* struct write_cm_files_data */

static int write_ipcoor_file(FILE *ipcoor_file, struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 22 March 2006

DESCRIPTION :
Writes text for an <ipcoor_file> to support <field>.
==============================================================================*/
{
	int return_code;
	struct Coordinate_system *coordinate_system;

	ENTER(write_ipcoor_file);
	if (ipcoor_file && field)
	{
		return_code = 1;
		fprintf(ipcoor_file, " CMISS Version 1.21 ipcoor File Version 1\n");
		fprintf(ipcoor_file, " Heading: cmgui generated file\n\n"); 
		fprintf(ipcoor_file, " The global coordinates for region 1 are [1]:\n");
		fprintf(ipcoor_file, "   (1) rectangular cartesian (x,y,z)\n");
		fprintf(ipcoor_file, "   (2) cylindrical polar (r,theta,z)\n");
		fprintf(ipcoor_file, "   (3) spherical polar (r,theta,phi)\n");
		fprintf(ipcoor_file, "   (4) prolate spheroidal (lambda,mu,theta)\n");
		fprintf(ipcoor_file, "   (5) oblate  spheroidal (lambda,mu,theta)\n");
		coordinate_system = get_FE_field_coordinate_system(field);
		switch (coordinate_system->type)
		{
			case RECTANGULAR_CARTESIAN:
			{
				fprintf(ipcoor_file, "    1\n");
			} break;
			default:
			{
				display_message(ERROR_MESSAGE, "write_ipcoor_file.  "
					"Coordinate system %s not implemented yet.", Coordinate_system_string(coordinate_system));
				return_code = 0;
			} break;
		}
		fprintf(ipcoor_file, " Enter the number of global coordinates [3]: %d\n",
			get_FE_field_number_of_components(field));
		fprintf(ipcoor_file, " Do you want to specify another coord. system for dependent variables [N]? N\n");
		fprintf(ipcoor_file, " Enter x,y,z origin of coords relative to region 0 [0,0,0]:0.00000D+00  0.00000D+00  0.00000D+00\n");
		fprintf(ipcoor_file, " Are there any non-standard mappings [N]? N\n");
	}
	else
	{
		display_message(ERROR_MESSAGE, "write_ipcoor_file.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_ipcoor_file */

struct FE_element_field_add_basis_data
{
	int number_of_components;
	struct FE_field *field;
	struct FE_region *region;
	struct LIST(FE_basis) *basis_types;
}; /* struct FE_element_field_add_basis_data */

static int FE_element_field_add_basis_to_list(
	struct FE_element *element, void *data_void)
/*******************************************************************************
LAST MODIFIED : 22 March 2006

DESCRIPTION :
FE_element iterator which adds the FE_basis representing data->field to the 
data->basis_list if it isn't there already.
==============================================================================*/
{
	int basis_type_array[4], dimension, i, return_code, xi1, xi2;
	struct FE_basis *face_basis, *fe_basis;
	struct FE_element_field_add_basis_data *data;

	ENTER(FE_element_field_add_basis_to_list);
	if (element && (data = (struct FE_element_field_add_basis_data *)data_void))
	{
		return_code = 1;
		if (FE_element_field_is_standard_node_based(element, data->field))
		{
			for (i = 0 ; return_code && (i < data->number_of_components) ; i++)
			{
				return_code = FE_element_field_get_component_FE_basis(element, data->field, 
					/*component_number*/i, &fe_basis);
				if (!IS_OBJECT_IN_LIST(FE_basis)(fe_basis, data->basis_types))
				{
					return_code = ADD_OBJECT_TO_LIST(FE_basis)(fe_basis,
						data->basis_types);

					FE_basis_get_dimension(fe_basis, &dimension);
					if (dimension == 3)
					{
						/* Add the face bases into the list */
						basis_type_array[0] = /*dimension*/2;
						basis_type_array[2] = /*off diagonal*/NO_RELATION;
						for (xi1 = 0 ; xi1 < dimension ; xi1++)
						{
							FE_basis_get_xi_basis_type(fe_basis, xi1, &basis_type_array[1]);
							for (xi2 = xi1 + 1 ; xi2 < dimension ; xi2++)
							{
								FE_basis_get_xi_basis_type(fe_basis, xi2, &basis_type_array[3]);
								face_basis = make_FE_basis(basis_type_array,
									FE_region_get_basis_manager(data->region));
								if (!IS_OBJECT_IN_LIST(FE_basis)(face_basis, data->basis_types))
								{
									return_code = ADD_OBJECT_TO_LIST(FE_basis)(face_basis,
										data->basis_types);
								}
							}
						}
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_add_FE_field_to_list.  Missing element_field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_field_add_FE_field_to_list */

static int write_ipbase_file(FILE *ipbase_file, struct FE_region *region,
	struct FE_field *field, struct write_cm_files_data *data)
/*******************************************************************************
LAST MODIFIED : 22 March 2006

DESCRIPTION :
Writes text for an <ipbase_file> to support <field>.
==============================================================================*/
{
	enum FE_basis_type basis_type;
	int basis_number, dimension, finished,
		has_derivatives[MAXIMUM_ELEMENT_XI_DIMENSIONS], interpolant_index, 
		node_flags[MAXIMUM_ELEMENT_XI_DIMENSIONS], number_of_gauss_points, 
		number_of_nodes[MAXIMUM_ELEMENT_XI_DIMENSIONS], return_code, xi_number;
	struct FE_element_field_add_basis_data add_basis_data;
	struct FE_basis *basis;

	ENTER(write_ipbase_file);
	if (ipbase_file && field)
	{
		return_code = 1;

		add_basis_data.basis_types = CREATE(LIST(FE_basis))();
		add_basis_data.field = field;
		add_basis_data.region = region;
		add_basis_data.number_of_components = get_FE_field_number_of_components(field);
		FE_region_for_each_FE_element(region,
			FE_element_field_add_basis_to_list, (void *)&add_basis_data);

		data->number_of_bases = NUMBER_IN_LIST(FE_basis)(add_basis_data.basis_types);

		/* Need an array so that we have an integer index for each basis */
		ALLOCATE(data->basis_array, struct FE_basis *, data->number_of_bases);

		fprintf(ipbase_file, " CMISS Version 1.21 ipbase File Version 2\n");
		fprintf(ipbase_file, " Heading: cmgui generated file\n\n"); 
		fprintf(ipbase_file, " Enter the number of types of basis function [1]: %d\n\n",
			data->number_of_bases);
		
		basis_number = 0;
		while (basis = FIRST_OBJECT_IN_LIST_THAT(FE_basis)(
			(LIST_CONDITIONAL_FUNCTION(FE_basis) *)NULL, (void *)NULL,
			add_basis_data.basis_types))
		{
			/* Put it in the array */
			data->basis_array[basis_number] = basis;

			fprintf(ipbase_file, " For basis function type %d the type of nodal interpolation is [1]:\n",
				basis_number + 1);
			fprintf(ipbase_file, "   (0) Auxiliary basis only\n");
			fprintf(ipbase_file, "   (1) Lagrange/Hermite tensor prod\n");
			fprintf(ipbase_file, "   (2) Simplex/Serendipity/Sector\n");
			fprintf(ipbase_file, "   (3) B-spline tensor product\n");
			fprintf(ipbase_file, "   (4) Fourier Series/Lagrange/Hermite tensor prod\n");
			fprintf(ipbase_file, "   (5) Boundary Element Lagrange/Hermite tensor pr.\n");
			fprintf(ipbase_file, "   (6) Boundary Element Simplex/Serendipity/Sector\n");
			fprintf(ipbase_file, "   (7) Extended Lagrange (multigrid collocation)\n");
			fprintf(ipbase_file, "    1\n");

			FE_basis_get_dimension(basis, &dimension);
			fprintf(ipbase_file, " Enter the number of Xi-coordinates [1]: %d\n\n", dimension);

			for (xi_number = 0 ; return_code && (xi_number < dimension) ; xi_number++)
			{
				FE_basis_get_xi_basis_type(basis, xi_number, &basis_type);
				has_derivatives[xi_number] = 0;
				switch (basis_type)
				{
					case LINEAR_LAGRANGE:
					{
						interpolant_index = 1;
						number_of_gauss_points = 2;
						number_of_nodes[xi_number] = 2;
					} break;
					case QUADRATIC_LAGRANGE:
					{
						interpolant_index = 2;
						number_of_gauss_points = 3;
						number_of_nodes[xi_number] = 3;
					} break;
					case CUBIC_LAGRANGE:
					{
						interpolant_index = 3;
						number_of_gauss_points = 4;
						number_of_nodes[xi_number] = 4;
					} break;
					case CUBIC_HERMITE:
					{
						interpolant_index = 5;
						number_of_gauss_points = 4;
						number_of_nodes[xi_number] = 2;
						has_derivatives[xi_number] = 1;
					} break;
					default:
					{
						display_message(ERROR_MESSAGE, "write_ipcoor_file.  "
							"Basis type %s not implemented or supported.", FE_basis_type_string(basis_type));
						interpolant_index = 0;
						number_of_gauss_points = 0;
						return_code = 0;
					}
				}
				fprintf(ipbase_file, " The interpolant in the Xi(%d) direction is [1]:\n",
					xi_number + 1);
				fprintf(ipbase_file, "   (1) Linear Lagrange\n");
				fprintf(ipbase_file, "   (2) Quadratic Lagrange\n");
				fprintf(ipbase_file, "   (3) Cubic Lagrange\n");
				fprintf(ipbase_file, "   (4) Quadratic Hermite\n");
				fprintf(ipbase_file, "   (5) Cubic Hermite\n");
				fprintf(ipbase_file, "    %d\n", interpolant_index);
				
				fprintf(ipbase_file, " Enter the number of Gauss points in the Xi(%d) direction [2]: %d\n",
					xi_number + 1, number_of_gauss_points);
			}
			if (return_code)
			{
				fprintf(ipbase_file, " Do you want to set cross derivatives to zero [N]? N\n");
				fprintf(ipbase_file, " Enter the node position indices []:");
				for (xi_number = 0 ; xi_number < dimension ; xi_number++)
				{
					node_flags[xi_number] = 1;
				}
				finished = 0;
				while (!finished)
				{
					for (xi_number = 0 ; xi_number < dimension ; xi_number++)
					{
						fprintf(ipbase_file, " %d", node_flags[xi_number]);
					}
					node_flags[0]++;
					for (xi_number = 0 ; (node_flags[xi_number] > number_of_nodes[xi_number]) && (xi_number < dimension) ; xi_number++)
					{
						node_flags[xi_number] = 1;
						node_flags[xi_number + 1]++;
					}
					if (xi_number == dimension)
					{
						finished = 1;
					}
				}
				fprintf(ipbase_file, "\n");
				fprintf(ipbase_file, " Enter the derivative order indices []: ");
				for (xi_number = 0 ; xi_number < dimension ; xi_number++)
				{
					node_flags[xi_number] = 1;
				}
				finished = 0;
				while (!finished)
				{
					for (xi_number = 0 ; xi_number < dimension ; xi_number++)
					{
						if (has_derivatives[xi_number])
						{
							fprintf(ipbase_file, " %d", node_flags[xi_number]);
						}
					}
					node_flags[0]++;
					for (xi_number = 0 ; (node_flags[xi_number] > (2 * has_derivatives[xi_number])) && (xi_number < dimension) ;
						xi_number++)
					{
						node_flags[xi_number] = 1;
						node_flags[xi_number + 1]++;
					}
					if (xi_number == dimension)
					{
						finished = 1;
					}
				}
				fprintf(ipbase_file, "\n");
				fprintf(ipbase_file, " Enter the number of auxiliary element parameters [0]:  0\n\n");
				fprintf(ipbase_file, " For basis function type %d scale factors are [6]:\n",
					basis_number + 1);
				fprintf(ipbase_file, "   (1) Unit\n");
				fprintf(ipbase_file, "   (2) Read in - Element based\n");
				fprintf(ipbase_file, "   (3) Read in - Node based\n");
				fprintf(ipbase_file, "   (4) Calculated from angle change\n");
				fprintf(ipbase_file, "   (5) Calculated from arc length\n");
				fprintf(ipbase_file, "   (6) Calculated from arithmetic mean arc length\n");
				fprintf(ipbase_file, "   (7) Calculated from harmonic mean arc length\n");
				fprintf(ipbase_file, "    1\n");
			}

			basis_number++;
			REMOVE_OBJECT_FROM_LIST(FE_basis)(basis, add_basis_data.basis_types);
		}
		DESTROY(LIST(FE_basis))(&add_basis_data.basis_types);
		
	}
	else
	{
		display_message(ERROR_MESSAGE, "write_ipbase_file.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_ipbase_file */

struct FE_node_write_cm_check_node_values_data
{
	int number_of_nodes;
	struct FE_field *field;
	int number_of_components;
	int number_of_derivatives[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int has_versions[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	FILE *ipnode_file;
}; /* struct FE_node_write_cm_check_node_values_data */

static int FE_node_write_cm_check_node_values(
	struct FE_node *node, void *data_void)
/*******************************************************************************
LAST MODIFIED : 22 March 2006

DESCRIPTION :
Counts how many nodes have the field defined and checks that the nodes
for which the field is defined are consistent in the number of derivatives for
each component.
Also sets a flag if any of components of the field have versions.
==============================================================================*/
{
	int i, return_code;
	struct FE_node_write_cm_check_node_values_data *data;

	ENTER(FE_node_write_cm_check_node_values);
	if (node && (data = (struct FE_node_write_cm_check_node_values_data *)data_void))
	{
		return_code = 1;
		if (FE_field_is_defined_at_node(data->field,node))
		{
			if (data->number_of_nodes)
			{
				/* Check for consistent number of derivatives */
				for (i = 0 ; i < data->number_of_components ; i++)
				{
					if (data->number_of_derivatives[i] != 
						get_FE_node_field_component_number_of_derivatives(
							node, data->field, i))
					{
						display_message(ERROR_MESSAGE, "FE_node_write_cm_check_node_values.  "
							"Node %d has inconsistent numbers of derivatives.",
							get_FE_node_identifier(node));
						return_code = 0;
					}
					if (1 != get_FE_node_field_component_number_of_versions(
							 node, data->field, i))
					{
						data->has_versions[i] = 1;
					}
				}
			}
			else
			{
				/* This is the first node */
				for (i = 0 ; i < data->number_of_components ; i++)
				{
					data->number_of_derivatives[i] =
						get_FE_node_field_component_number_of_derivatives(
							node, data->field, i);
					if (1 != get_FE_node_field_component_number_of_versions(
							 node, data->field, i))
					{
						data->has_versions[i] = 1;
					}
				}
			}
			data->number_of_nodes++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_write_cm_check_node_values.  Missing element_field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_write_cm_check_node_values */

static int write_cm_FE_node(
	struct FE_node *node, void *data_void)
/*******************************************************************************
LAST MODIFIED : 22 March 2006

DESCRIPTION :
Writes the node to an ipnode file.
==============================================================================*/
{
	char *value_strings[] = {" 1", " 2", "s 1 & 2", " 3", "s 1 & 3", "s 2 & 3",
		"s 1, 2 & 3"};
	enum FE_nodal_value_type value_type[] = {FE_NODAL_D_DS1,
		FE_NODAL_D_DS2, FE_NODAL_D2_DS1DS2, FE_NODAL_D_DS3, FE_NODAL_D2_DS1DS3,
		FE_NODAL_D2_DS2DS3, FE_NODAL_D3_DS1DS2DS3};
	FE_value value;
	int i, j, k, number_of_versions, return_code;
	struct FE_node_write_cm_check_node_values_data *data;

	ENTER(FE_node_write_cm_check_node_values);
	if (node && (data = (struct FE_node_write_cm_check_node_values_data *)data_void))
	{
		return_code = 1;
		if (FE_field_is_defined_at_node(data->field, node))
		{
			fprintf(data->ipnode_file, " Node number [    1]:     %d\n",
				get_FE_node_identifier(node));
			for (i = 0 ; i < data->number_of_components ; i++)
			{
				if (data->has_versions[i])
				{
					number_of_versions = get_FE_node_field_component_number_of_versions(node, data->field, i);
					fprintf(data->ipnode_file, " The number of versions for nj=%d is [1]:  %d\n",
						i + 1, number_of_versions);
				}
				else
				{
					number_of_versions = 1;
				}
				for (j = 0 ; j < number_of_versions ; j++)
				{
					if (1 < number_of_versions)
					{
						fprintf(data->ipnode_file, " For version number%2d:\n", j + 1);
					}
					get_FE_nodal_FE_value_value(node, data->field, /*component_number*/i, /*version*/j,
						FE_NODAL_VALUE, /*time*/0.0, &value);
					fprintf(data->ipnode_file, " The Xj(%d) coordinate is [ 0.00000E+00]:  %le\n",
						i + 1, value);
					
					for (k = 0 ; k < data->number_of_derivatives[i] ; k++)
					{
						get_FE_nodal_FE_value_value(node, data->field, /*component_number*/i, /*version*/j,
							value_type[k], /*time*/0.0, &value);
						fprintf(data->ipnode_file, " The derivative wrt direction%s is [ 0.00000E+00]:  %le\n",
							value_strings[k], value);
					}
				}
			}
			fprintf(data->ipnode_file, "\n");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_cm_FE_node.  Missing element_field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_cm_FE_node */

static int write_ipnode_file(FILE *ipnode_file, struct FE_region *region,
	struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 22 March 2006

DESCRIPTION :
Writes text for an <ipbase_file> to support <field>.
==============================================================================*/
{
	int i, return_code;
	struct FE_node_write_cm_check_node_values_data cm_node_data;

	ENTER(write_ipnode_file);
	if (ipnode_file && region && field)
	{
		return_code = 1;

		fprintf(ipnode_file, " CMISS Version 1.21 ipnode File Version 2\n");
		fprintf(ipnode_file, " Heading: cmgui generated file\n\n"); 

		cm_node_data.number_of_nodes = 0;
		cm_node_data.field = field;
		cm_node_data.number_of_components = get_FE_field_number_of_components(field);
		for (i = 0 ; i < cm_node_data.number_of_components ; i++)
		{
			cm_node_data.number_of_derivatives[i] = -1;
			cm_node_data.has_versions[i] = 0;
		}
		cm_node_data.ipnode_file = ipnode_file;
		if (return_code = FE_region_for_each_FE_node(region,
				FE_node_write_cm_check_node_values, (void *)&cm_node_data))
		{
			fprintf(ipnode_file, " The number of nodes is [1]: %d\n", 
				cm_node_data.number_of_nodes);
			fprintf(ipnode_file, " Number of coordinates [3]: %d\n",
				cm_node_data.number_of_components);
			for (i = 0 ; i < cm_node_data.number_of_components ; i++)
			{
				if (cm_node_data.has_versions[i])
				{
					fprintf(ipnode_file, " Do you want prompting for different versions of nj=%d [N]? Y\n",
						i + 1);
				}
				else
				{
					fprintf(ipnode_file, " Do you want prompting for different versions of nj=%d [N]? N\n",
						i + 1);
				}
			}
			for (i = 0 ; i < cm_node_data.number_of_components ; i++)
			{
				fprintf(ipnode_file, " The number of derivatives for coordinate %d is [0]: %d\n",
					i + 1, cm_node_data.number_of_derivatives[i]);
			}

			return_code = FE_region_for_each_FE_node(region,
				write_cm_FE_node, (void *)&cm_node_data);
		}
		else
		{
			display_message(ERROR_MESSAGE, "write_ipnode_file.  "
				"Nodes do not have the required consistency to write an ipnode file.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "write_ipnode_file.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_ipnode_file */

struct FE_element_write_cm_check_element_values_data
{
	int number_of_elements;
	struct FE_field *field;
	int number_of_components;
	int number_of_bases;
	struct FE_basis **basis_array;
	FILE *ipelem_file;
}; /* struct FE_element_write_cm_check_element_values_data */

static int FE_element_write_cm_check_element_values(
	struct FE_element *element, void *data_void)
/*******************************************************************************
LAST MODIFIED : 22 March 2006

DESCRIPTION :
Counts how many elements have the field defined.
==============================================================================*/
{
	int return_code;
	struct FE_element_write_cm_check_element_values_data *data;

	ENTER(FE_element_write_cm_check_element_values);
	if (element && (data = (struct FE_element_write_cm_check_element_values_data *)data_void))
	{
		return_code = 1;
		if (FE_field_is_defined_in_element(data->field,element) && 
			FE_element_field_is_standard_node_based(element, data->field))
		{
			data->number_of_elements++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_write_cm_check_element_values.  Missing element_field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_write_cm_check_element_values */

static int write_cm_FE_element(
	struct FE_element *element, void *data_void)
/*******************************************************************************
LAST MODIFIED : 22 March 2006

DESCRIPTION :
Writes the element to an ipelem file.
==============================================================================*/
{
	int basis_dimension, basis_number, derivatives_in_node, dimension,
		first_basis, i, j, k, nodal_value_index,
		node_index, *node_number_array, number_of_element_field_nodes,
		number_of_nodal_values, occurences, return_code, version, *versions_array;
	struct CM_element_information element_id;
	struct FE_basis *fe_basis;
	struct FE_element_field_component *component;
	struct FE_element_write_cm_check_element_values_data *data;
	struct FE_node *node;
	struct Standard_node_to_element_map *standard_node_map;

	ENTER(FE_element_write_cm_check_element_values);
	if (element && (data = (struct FE_element_write_cm_check_element_values_data *)data_void))
	{
		return_code = 1;
		if (FE_field_is_defined_in_element(data->field, element) && 
			FE_element_field_is_standard_node_based(element, data->field))
		{
			get_FE_element_identifier(element, &element_id);
			fprintf(data->ipelem_file, " Element number [    1]:     %d\n",
				element_id.number);
			dimension = get_FE_element_dimension(element);

			fprintf(data->ipelem_file, " The number of geometric Xj-coordinates is [3]: %d\n",
				dimension);
			for (i = 0 ; i < data->number_of_components ; i++)
			{
				return_code = FE_element_field_get_component_FE_basis(element, data->field, 
					/*component_number*/i, &fe_basis);
				
				basis_number = 0;
				while ((data->basis_array[basis_number] != fe_basis) && (basis_number < data->number_of_bases))
				{
					basis_number++;
				}
				fprintf(data->ipelem_file, " The basis function type for geometric variable %d is [1]:  %d\n",
					i + 1, basis_number + 1);
			}
			first_basis = 1;
			for (i = 0 ; i < data->number_of_bases ; i++)
			{
				FE_basis_get_dimension(data->basis_array[i], &basis_dimension);
				if (dimension == basis_dimension)
				{
					/* Use the first component as it has a basis, could try and match to the 
						which component actually used this basis above */
					if (get_FE_element_field_component(element, data->field, /*component*/0, &component))
					{
						FE_element_field_component_get_number_of_nodes(component,
							&number_of_element_field_nodes);

						if (first_basis)
						{
							fprintf(data->ipelem_file, " Enter the %d global numbers for basis %d:",
								number_of_element_field_nodes, i + 1);
						}
						else
						{
							fprintf(data->ipelem_file, " Enter the %d numbers for basis %d [prev]:",
								number_of_element_field_nodes, i + 1);
						}
						ALLOCATE(versions_array, int, number_of_element_field_nodes);
						ALLOCATE(node_number_array, int, number_of_element_field_nodes);
						for (j = 0 ; j < number_of_element_field_nodes ; j++)
						{
							FE_element_field_component_get_standard_node_map(
								component, j, &standard_node_map);
							Standard_node_to_element_map_get_node_index(
								standard_node_map, &node_index);
							Standard_node_to_element_map_get_number_of_nodal_values(
								standard_node_map, &number_of_nodal_values);

							get_FE_element_node(element, node_index, &node);
							node_number_array[j] = get_FE_node_identifier(node);
							versions_array[j] = get_FE_node_field_component_number_of_versions(node,
								data->field, /*component*/0);

							fprintf(data->ipelem_file, " %d", node_number_array[j]);
						}
						fprintf(data->ipelem_file, "\n");
						if (first_basis)
						{
							for (j = 0 ; j < number_of_element_field_nodes ; j++)
							{
								if (1 < versions_array[j])
								{
									FE_element_field_component_get_standard_node_map(
										component, j, &standard_node_map);
									Standard_node_to_element_map_get_node_index(
										standard_node_map, &node_index);
									Standard_node_to_element_map_get_number_of_nodal_values(
										standard_node_map, &number_of_nodal_values);

									get_FE_element_node(element, node_index, &node);
									derivatives_in_node = 
										get_FE_node_field_component_number_of_derivatives(node, data->field, /*component*/0);
									
									/* Get the nodal_value_index of the first value and use that to specify version,
										cannot support mixes or mismatches anyway, could check all values the same */
									Standard_node_to_element_map_get_nodal_value_index(
										standard_node_map, /*k*/0, &nodal_value_index);
									version = nodal_value_index / (derivatives_in_node + 1);

									occurences = 1;
									for (k = j - 1 ; k >= 0 ; k--)
									{
										if (node_number_array[k] == node_number_array[j])
										{
											occurences++;
										}
									}
									/* Required to specify version to use for each different coordinate,
										just specify the same versions for now. */
									for (k = 0 ; k < 3 ; k++)
									{
										fprintf(data->ipelem_file, 
											" The version number for occurrence  %d of node %5d"
											", njj=%d is [ 1]: %d\n", occurences, node_number_array[j],
											k + 1, version + 1);
									}
								}
							}
							first_basis = 0;
						}
						DEALLOCATE(versions_array);
						DEALLOCATE(node_number_array);
					}
				}
			}
			fprintf(data->ipelem_file, "\n");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_cm_FE_element.  Missing element_field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_cm_FE_element */

static int write_ipelem_file(FILE *ipelem_file, struct FE_region *region,
	struct FE_field *field, struct write_cm_files_data *data)
/*******************************************************************************
LAST MODIFIED : 22 March 2006

DESCRIPTION :
Writes text for an <ipbase_file> to support <field>.
==============================================================================*/
{
	int return_code;
	struct FE_element_write_cm_check_element_values_data cm_element_data;

	ENTER(write_ipelem_file);
	if (ipelem_file && region && field)
	{
		return_code = 1;

		fprintf(ipelem_file, " CMISS Version 1.21 ipelem File Version 2\n");
		fprintf(ipelem_file, " Heading: cmgui generated file\n\n"); 

		cm_element_data.number_of_elements = 0;
		cm_element_data.field = field;
		cm_element_data.number_of_components = get_FE_field_number_of_components(field);
		cm_element_data.number_of_bases = data->number_of_bases;
		cm_element_data.basis_array = data->basis_array;
		cm_element_data.ipelem_file = ipelem_file;
		if (return_code = FE_region_for_each_FE_element(region,
				FE_element_write_cm_check_element_values, (void *)&cm_element_data))
		{
			fprintf(ipelem_file, " The number of elements is [1]: %d\n\n", 
				cm_element_data.number_of_elements);

			return_code = FE_region_for_each_FE_element(region,
				write_cm_FE_element, (void *)&cm_element_data);
		}
		else
		{
			display_message(ERROR_MESSAGE, "write_ipelem_file.  "
				"Elements do not have the required consistency to write an ipelem file.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "write_ipelem_file.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_ipelem_file */

static int write_cm_FE_region(FILE *ipcoor_file, FILE *ipbase_file,
	FILE *ipnode_file, FILE *ipelem_file,
	struct FE_region *fe_region, struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 22 March 2006

DESCRIPTION :
Writes <field> of <fe_region> to the <output_file>.
==============================================================================*/
{
	int return_code;
	struct write_cm_files_data data;

	ENTER(write_cm_FE_region);
	if (ipcoor_file && ipbase_file && ipnode_file && ipelem_file && fe_region && field)
	{
		data.number_of_bases = 0;
		data.basis_array = (struct FE_basis **)NULL;

		return_code = write_ipcoor_file(ipcoor_file, field);
		if (return_code)
		{
			return_code = write_ipbase_file(ipbase_file, fe_region, field, &data);
		}
		if (return_code)
		{
			return_code = write_ipnode_file(ipnode_file, fe_region, field);
		}
		if (return_code)
		{
			return_code = write_ipelem_file(ipelem_file, fe_region, field, &data);
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE, "write_cm_FE_region.  Failed");
			return_code = 0;
		}
	  
		if (data.basis_array)
		{
			DEALLOCATE(data.basis_array);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_cm_FE_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_cm_FE_region */

/*
Global functions
----------------
*/

int write_cm_files(FILE *ipcoor_file, FILE *ipbase_file,
	FILE *ipnode_file, FILE *ipelem_file,
	struct Cmiss_region *root_region, char *write_path,
	struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 22 March 2006

DESCRIPTION :
Writes the set of <ipcoor_file>, <ipbase_file>, <ipnode_file> and <ipelem_file>
that defines elements of <field> in <write_path>.
==============================================================================*/
{
	int number_of_children, return_code;
	struct Cmiss_region *write_region;
	struct FE_region *fe_region;

	ENTER(write_exregion_file);
	write_region = (struct Cmiss_region *)NULL;
	if (ipcoor_file && ipbase_file && ipnode_file && ipelem_file && root_region && 
		Cmiss_region_get_number_of_child_regions(root_region, &number_of_children)
		&& write_path && Cmiss_region_get_region_from_path(root_region,
			write_path, &write_region) && write_region)
	{
		return_code = 1;
		if (fe_region = Cmiss_region_get_FE_region(write_region))
		{
			return_code = write_cm_FE_region(ipcoor_file, ipbase_file,
				ipnode_file, ipelem_file, fe_region, field);
		}
		else
		{
			display_message(ERROR_MESSAGE, "write_cm_files.  "
				"Specified region %s has no finite elements.", write_path);
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"write_cm_files.  Error writing region");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_cm_files.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_cm_files */
