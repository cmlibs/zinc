/***************************************************************************//**
 * FILE : render_to_finite_elements.c
 *
 * Renders gtObjects to finite elements
 */
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
#include <stdlib.h>
#include <string.h>
#include <math.h>
//-- extern "C" {
#include "api/cmiss_element.h"
#include "api/cmiss_field_group.h"
#include "api/cmiss_field_module.h"
#include "api/cmiss_field_subobject_group.h"
#include "api/cmiss_node.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "general/debug.h"
//-- }
#include "general/enumerator_private.hpp"
//-- extern "C" {
#include "general/list.h"
#include "general/list_private.h"
#include "general/object.h"
#include "general/mystring.h"
#include "general/random.h"
#include "graphics/graphics_object.h"
#include "graphics/material.h"
#include "graphics/scene.h"
//-- }
#include "general/statistics.h"
#include "graphics/scene.hpp"
//-- extern "C" {
#include "graphics/spectrum.h"
#include "graphics/texture.h"
#include "general/message.h"
#include "graphics/render_to_finite_elements.h"
//-- }
#include "graphics/graphics_object_private.hpp"

/*
Module types
------------
*/

struct Render_node
{
	struct FE_node *fe_node;
	FE_value_triple coordinates;
	float *data;
};

struct Render_to_finite_elements_data
{
	Cmiss_region_id region;
	Cmiss_field_group_id group;
	Cmiss_field_module_id field_module;
	Cmiss_field_cache_id field_cache;
	enum Render_to_finite_elements_mode render_mode;
	Cmiss_field_id coordinate_field;
	Cmiss_nodeset_id master_nodeset, nodeset;
	Cmiss_mesh_id master_mesh_1d, mesh_1d;
	Cmiss_mesh_id master_mesh_2d, mesh_2d;
	Cmiss_node_template_id node_template;
	Cmiss_element_template_id line_element_template, triangle_element_template, square_element_template;

	Render_to_finite_elements_data(Cmiss_region_id region,
			Cmiss_field_group_id group, enum Render_to_finite_elements_mode render_mode,
			Cmiss_field_id coordinate_field) :
		region(region),
		group(group),
		field_module(Cmiss_region_get_field_module(region)),
		field_cache(Cmiss_field_module_create_cache(field_module)),
		render_mode(render_mode),
		coordinate_field(coordinate_field),
		master_nodeset(Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes")),
		nodeset(0),
		master_mesh_1d(Cmiss_field_module_find_mesh_by_dimension(field_module, 1)),
		mesh_1d(0),
		master_mesh_2d(Cmiss_field_module_find_mesh_by_dimension(field_module, 2)),
		mesh_2d(0),
		node_template(0),
		line_element_template(0),
		triangle_element_template(0),
		square_element_template(0)
	{
		if (group)
		{
			Cmiss_field_node_group_id node_group = Cmiss_field_group_get_node_group(group, master_nodeset);
			if (!node_group)
				node_group = Cmiss_field_group_create_node_group(group, master_nodeset);
			nodeset = Cmiss_nodeset_group_base_cast(Cmiss_field_node_group_get_nodeset(node_group));
			Cmiss_field_node_group_destroy(&node_group);
			Cmiss_field_element_group_id element_group_1d = Cmiss_field_group_get_element_group(group, master_mesh_1d);
			if (!element_group_1d)
				element_group_1d = Cmiss_field_group_create_element_group(group, master_mesh_1d);
			mesh_1d = Cmiss_mesh_group_base_cast(Cmiss_field_element_group_get_mesh(element_group_1d));
			Cmiss_field_element_group_destroy(&element_group_1d);
			Cmiss_field_element_group_id element_group_2d = Cmiss_field_group_get_element_group(group, master_mesh_2d);
			if (!element_group_2d)
				element_group_2d = Cmiss_field_group_create_element_group(group, master_mesh_2d);
			mesh_2d = Cmiss_mesh_group_base_cast(Cmiss_field_element_group_get_mesh(element_group_2d));
			Cmiss_field_element_group_destroy(&element_group_2d);

		}
		else
		{
			nodeset = Cmiss_nodeset_access(master_nodeset);
			mesh_1d = Cmiss_mesh_access(master_mesh_1d);
			mesh_2d = Cmiss_mesh_access(master_mesh_2d);
		}
		if (render_mode == RENDER_TO_FINITE_ELEMENTS_SURFACE_NODE_CLOUD)
		{
			/* Set a random seed so that testing generates the same cloud
				each time */
			CMGUI_SEED_RANDOM(10000);
		}
		Cmiss_field_module_begin_change(field_module);
	}

	~Render_to_finite_elements_data()
	{
		Cmiss_element_template_destroy(&square_element_template);
		Cmiss_element_template_destroy(&triangle_element_template);
		Cmiss_element_template_destroy(&line_element_template);
		Cmiss_node_template_destroy(&node_template);
		Cmiss_mesh_destroy(&mesh_2d);
		Cmiss_mesh_destroy(&master_mesh_2d);
		Cmiss_mesh_destroy(&mesh_1d);
		Cmiss_mesh_destroy(&master_mesh_1d);
		Cmiss_nodeset_destroy(&nodeset);
		Cmiss_nodeset_destroy(&master_nodeset);
		Cmiss_field_cache_destroy(&field_cache);
		Cmiss_field_module_end_change(field_module);
		Cmiss_field_module_destroy(&field_module);
	}

	int checkValidCoordinateField()
	{
		int return_code = 1;
		if (3 != Computed_field_get_number_of_components(coordinate_field))
			return_code = 0;

		node_template = Cmiss_nodeset_create_node_template(nodeset);
		if (!Cmiss_node_template_define_field(node_template, coordinate_field))
			return_code = 0;
		const int local_node_indexes[] = { 1, 2, 3, 4 };

		line_element_template = Cmiss_mesh_create_element_template(mesh_1d);
		Cmiss_element_template_set_shape_type(line_element_template, CMISS_ELEMENT_SHAPE_LINE);
		Cmiss_element_template_set_number_of_nodes(line_element_template, 2);
		Cmiss_element_basis_id line_basis = Cmiss_field_module_create_element_basis(
			field_module, /*dimension*/1, CMISS_BASIS_FUNCTION_LINEAR_LAGRANGE);
		if (!Cmiss_element_template_define_field_simple_nodal(line_element_template,
			coordinate_field, /*component_number*/-1, line_basis, /*number_of_nodes*/2, local_node_indexes))
			return_code = 0;
		Cmiss_element_basis_destroy(&line_basis);

		triangle_element_template = Cmiss_mesh_create_element_template(mesh_2d);
		Cmiss_element_template_set_shape_type(triangle_element_template, CMISS_ELEMENT_SHAPE_TRIANGLE);
		Cmiss_element_template_set_number_of_nodes(triangle_element_template, 3);
		Cmiss_element_basis_id triangle_basis = Cmiss_field_module_create_element_basis(
			field_module, /*dimension*/2, CMISS_BASIS_FUNCTION_LINEAR_SIMPLEX);
		if (!Cmiss_element_template_define_field_simple_nodal(triangle_element_template,
			coordinate_field, /*component_number*/-1, triangle_basis, /*number_of_nodes*/3, local_node_indexes))
			return_code = 0;
		Cmiss_element_basis_destroy(&triangle_basis);

		square_element_template = Cmiss_mesh_create_element_template(mesh_2d);
		Cmiss_element_template_set_shape_type(square_element_template, CMISS_ELEMENT_SHAPE_SQUARE);
		Cmiss_element_template_set_number_of_nodes(square_element_template, 4);
		Cmiss_element_basis_id square_basis = Cmiss_field_module_create_element_basis(
			field_module, /*dimension*/2, CMISS_BASIS_FUNCTION_LINEAR_LAGRANGE);
		if (!Cmiss_element_template_define_field_simple_nodal(square_element_template,
			coordinate_field, /*component_number*/-1, square_basis, /*number_of_nodes*/4, local_node_indexes))
			return_code = 0;
		Cmiss_element_basis_destroy(&square_basis);

		return return_code;
	}

	void setTime(FE_value time)
	{
		Cmiss_field_cache_set_time(field_cache, time);
	}

	Cmiss_node_id createNode(FE_value *coordinates)
	{
		Cmiss_node_id node = Cmiss_nodeset_create_node(nodeset, /*identifier=automatic*/-1, node_template);
		Cmiss_field_cache_set_node(field_cache, node);
		Cmiss_field_assign_real(coordinate_field, field_cache, /*number_of_values*/3, coordinates);
		return node;
	}

	/** Generates a finite_element representation of the specified line according
	 * to the <render_mode>
	 */
	int addLine(int number_of_data_components, struct Render_node *node1, struct Render_node *node2)
	{
		int return_code = 0;
		switch (render_mode)
		{
			case RENDER_TO_FINITE_ELEMENTS_SURFACE_NODE_CLOUD:
			{
				double length, density, expected_number;
				FE_value position[3], side[3], xi1;
				int i, j, number_of_points;
				return_code = 1;
				for (i = 0 ; i < 3 ; i++)
				{
					side[i] = node2->coordinates[i] - node1->coordinates[i];
				}
				length = sqrt(side[0] * side[0]
					+ side[1] * side[1]
					+ side[2] * side[2]);
				if (number_of_data_components)
				{
					density = (node1->data[0] + node2->data[0]) / 3.0;
					if (density < 0.0)
					{
						density = 0.0;
					}
				}
				else
				{
					density = 1.0;
				}
				expected_number = length * density;
				/* get actual_number = sample from Poisson distribution with
					mean given by expected_number */
				number_of_points =
					sample_Poisson_distribution(expected_number);
				for (j = 0; (j < number_of_points) && return_code; j++)
				{
					xi1 = CMGUI_RANDOM(float);
					for (i = 0 ; i < 3 ; i++)
					{
						position[i] = node1->coordinates[i]
							+ xi1 * side[i];
					}
					Cmiss_node_id node = createNode(position);
					if (node)
					{
						Cmiss_node_destroy(&node);
					}
					else
					{
						return_code = 0;
					}
				}
			} break;
			case RENDER_TO_FINITE_ELEMENTS_LINEAR_PRODUCT:
			{
				if (node1 && node1->fe_node && node2 && node2->fe_node)
				{
					Cmiss_element_template_set_node(line_element_template, 1, node1->fe_node);
					Cmiss_element_template_set_node(line_element_template, 2, node2->fe_node);
					return_code = Cmiss_mesh_define_element(mesh_1d, /*identifier*/-1, line_element_template);
				}
				else
				{
					display_message(ERROR_MESSAGE,"Render_to_finite_elements_data::addLine.  "
						"Linear product render should have already created the nodes.");
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE, "Render_to_finite_elements_data::addLine.  "
					"Unknown render mode.");
				return_code = 0;
			} break;
		}
		return return_code;
	}

	/** Generates a finite_element representation of the specified triangle according
	 * to the <render_mode>
	 */
	int addTriangle(int number_of_data_components,
		struct Render_node *node1, struct Render_node *node2, struct Render_node *node3)
	{
		int return_code = 0;
		switch (render_mode)
		{
			case RENDER_TO_FINITE_ELEMENTS_SURFACE_NODE_CLOUD:
			{
				double area, coordinate_1, coordinate_2, coordinate_3, density, expected_number;
				FE_value side1[3], side2[3], side3[3], position[3], xi1, xi2;
				int i, j, number_of_points;

				return_code = 1;
				for (i = 0 ; i < 3 ; i++)
				{
					side1[i] = node2->coordinates[i] - node1->coordinates[i];
					side2[i] = node3->coordinates[i] - node1->coordinates[i];
					side3[i] = node3->coordinates[i] - node2->coordinates[i];
				}
				coordinate_1 = sqrt(side1[0] * side1[0]
					+ side1[1] * side1[1]
					+ side1[2] * side1[2]);
				coordinate_2 = sqrt(side2[0] * side2[0]
					+ side2[1] * side2[1]
					+ side2[2] * side2[2]);
				coordinate_3 = sqrt(side3[0] * side3[0]
					+ side3[1] * side3[1]
					+ side3[2] * side3[2]);
				area = 0.5 * ( coordinate_1 + coordinate_2 + coordinate_3 );
				area = sqrt ( area * (area - coordinate_1) *
					(area - coordinate_2) * (area - coordinate_3));
				if (number_of_data_components)
				{
					density = (node1->data[0] + node2->data[0] + node3->data[0]) / 3.0;
					if (density < 0.0)
					{
						density = 0.0;
					}
				}
				else
				{
					density = 1.0;
				}
				expected_number = area * density;
				/* get actual_number = sample from Poisson distribution with
					mean given by expected_number */
				number_of_points =
					sample_Poisson_distribution(expected_number);
				for (j = 0; (j < number_of_points) && return_code; j++)
				{
					xi1 = CMGUI_RANDOM(float);
					xi2 = CMGUI_RANDOM(float);
					if(xi1 + xi2 > 1.0)
					{
						xi1 = 1.0 - xi1;
						xi2 = 1.0 - xi2;
					}
					for (i = 0 ; i < 3 ; i++)
					{
						position[i] = node1->coordinates[i]
							+ xi1 * side1[i] + xi2 * side2[i];
					}
					Cmiss_node_id node = createNode(position);
					if (node)
					{
						Cmiss_node_destroy(&node);
					}
					else
					{
						return_code = 0;
					}
				}
			} break;
			case RENDER_TO_FINITE_ELEMENTS_LINEAR_PRODUCT:
			{
				if (node1 && node1->fe_node && node2 && node2->fe_node && node3 && node3->fe_node)
				{
					Cmiss_element_template_set_node(triangle_element_template, 1, node1->fe_node);
					Cmiss_element_template_set_node(triangle_element_template, 2, node2->fe_node);
					Cmiss_element_template_set_node(triangle_element_template, 3, node3->fe_node);
					return_code = Cmiss_mesh_define_element(mesh_2d, /*identifier*/-1, triangle_element_template);
				}
				else
				{
					display_message(ERROR_MESSAGE, "Render_to_finite_elements_data::addTriangle.  "
						"Linear product render should have already created the nodes.");
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,"Render_to_finite_elements_data::addTriangle.  "
					"Unknown render mode.");
				return_code = 0;
			} break;
		}
		return (return_code);
	}

	/** Generates a finite_element representation of the specified square according
	 * to the <render_mode>.  If the mode does not have a special representation then
	 * it will default to creating to triangles.
	 * The vertex ordering is the usual order in cmgui.
	 * xi1=0,xi2=0 ; xi1=1,xi2=0 ; xi1=0,xi2=1 ; xi1=1,xi2=1
	 */
	int addSquare(int number_of_data_components,
		struct Render_node *node1, struct Render_node *node2, struct Render_node *node3, struct Render_node *node4)
	{
		int return_code = 0;
		switch (render_mode)
		{
			case RENDER_TO_FINITE_ELEMENTS_LINEAR_PRODUCT:
			{
				if (node1 && node1->fe_node && node2 && node2->fe_node &&
					node3 && node3->fe_node && node4 && node4->fe_node)
				{
					Cmiss_element_template_set_node(square_element_template, 1, node1->fe_node);
					Cmiss_element_template_set_node(square_element_template, 2, node2->fe_node);
					Cmiss_element_template_set_node(square_element_template, 3, node3->fe_node);
					Cmiss_element_template_set_node(square_element_template, 4, node4->fe_node);
					return_code = Cmiss_mesh_define_element(mesh_2d, /*identifier*/-1, square_element_template);
				}
				else
				{
					display_message(ERROR_MESSAGE, "Render_to_finite_elements_data::addSquare.  "
						"Linear product render should have already created the nodes.");
				}
			} break;
			default:
			{
				return_code = addTriangle(number_of_data_components, node1, node2, node3) &&
					addTriangle(number_of_data_components, node2, node3, node4);
			} break;
		}
		return (return_code);
	}

};

/*
Module functions
----------------
*/

static int Render_node_create(struct Render_node *render_node,
	struct Render_to_finite_elements_data *data,
	FE_value *coordinates, int number_of_data_components, float *data_values)
/*******************************************************************************
LAST MODIFIED : 8 December 2005

DESCRIPTION :
Generates a finite_element node at the specified location
==============================================================================*/
{
	int return_code;
	render_node->fe_node = (struct FE_node *)NULL;
	render_node->data = (float *)NULL;
	return_code = 1;
	if (data)
	{
		switch (data->render_mode)
		{
			case RENDER_TO_FINITE_ELEMENTS_SURFACE_NODE_CLOUD:
			{
				/* Keep the coordinates for later */
				render_node->coordinates[0] = coordinates[0];
				render_node->coordinates[1] = coordinates[1];
				render_node->coordinates[2] = coordinates[2];
				if (number_of_data_components)
				{
					render_node->data = data_values;
				}
			} break;
			default:
			{
				Cmiss_node_id node = data->createNode(coordinates);
				render_node->fe_node = node;
				Cmiss_node_destroy(&node); // existing code did not access
			} break;
		}
	}
	return (return_code);
}

static int render_polyline_to_finite_elements(
	struct Render_to_finite_elements_data *data, Triple *point_list,
	int number_of_data_components,GTDATA *data_values,
	struct Graphical_material *material,struct Spectrum *spectrum,int n_pts,
	enum GT_polyline_type polyline_type)
/*******************************************************************************
LAST MODIFIED : 14 July 2006

DESCRIPTION :
Writes VRML code to the file handle which represents the given
continuous polyline. If data or spectrum are NULL they are ignored.  
==============================================================================*/
{
	GTDATA *data_ptr;
	int i,number_of_points,return_code;
	struct Render_node *nodes;
	FE_value_triple position;

	ENTER(render_polyline_to_finite_elements);
	if (point_list&&(1<n_pts)&&
		((g_NO_DATA==number_of_data_components)||(data&&material&&spectrum)))
	{
		return_code=1;
		nodes = (struct Render_node *)NULL;
		switch (polyline_type)
		{
			case g_PLAIN:
			{
				number_of_points=n_pts;
			} break;
			case g_PLAIN_DISCONTINUOUS:
			{
				/* n_pts = number of line segments in this case */
				number_of_points=2*n_pts;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"render_polyline_to_finite_elements.  Unsupported polyline_type");
				return_code=0;
			} break;
		}
		if (return_code)
		{
			data_ptr = data_values;
			if (ALLOCATE(nodes, struct Render_node, number_of_points))
			{
				for (i = 0 ; return_code && (i < number_of_points) ; i++)
				{
					CAST_TO_FE_VALUE(position, point_list[i], 3);
					if (!(Render_node_create(&nodes[i], data, position,
						 number_of_data_components, data_ptr)))
					{
						display_message(ERROR_MESSAGE,
								"render_voltex_to_finite_elements.  "
								"Could not set fields at node");
						return_code = 0;
					}
					if (number_of_data_components)
					{
						data_ptr += number_of_data_components;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"render_voltex_to_finite_elements.  "
					"Unable to allocate node array");
				return_code = 0;
			}
		}
		if (return_code)
		{
			switch (polyline_type)
			{
				case g_PLAIN:
				{
					for (i = 0 ; i < n_pts - 1 ; i++)
					{
						data->addLine(number_of_data_components, &nodes[i], &nodes[i+1]);
					}
				} break;
				case g_PLAIN_DISCONTINUOUS:
				{
					for (i=0;i<n_pts;i++)
					{
						data->addLine(number_of_data_components, &nodes[2*i], &nodes[2*i+1]);
					}
				} break;
				case g_POLYLINE_TYPE_BEFORE_FIRST:
				case g_NORMAL:
				case g_NORMAL_DISCONTINUOUS:
				case g_POLYLINE_TYPE_AFTER_LAST:
				case g_POLYLINE_TYPE_INVALID:
				{
					/* Do nothing */
				} break;
			}
		}
		if (nodes)
		{
			DEALLOCATE(nodes);
		}
	}
	else
	{
		if (1<n_pts)
		{
			display_message(ERROR_MESSAGE,"render_polyline_to_finite_elements.  Invalid argument(s)");
			return_code=0;
		}
		else
		{
			return_code=1;
		}
	}
	LEAVE;

	return (return_code);
} /* render_polyline_to_finite_elements */

static int render_surface_to_finite_elements(
	struct Render_to_finite_elements_data *data,
	Triple *surfpts, Triple *normalpts,
	Triple *texturepts, int number_of_data_components, GTDATA *data_values,
	int npts1,
	int npts2,enum GT_surface_type surface_type,gtPolygonType polygon_type)
/*******************************************************************************
LAST MODIFIED : 8 December 2005

DESCRIPTION :
==============================================================================*/
{
	GTDATA *data_ptr;
	int i,j,return_code;
	int index,index_1,index_2,number_of_points;
	struct Render_node *nodes;
	FE_value_triple position;

	ENTER(render_surface_to_finite_elements);
	return_code=0;
	USE_PARAMETER(normalpts);
	USE_PARAMETER(texturepts);
	if (surfpts&&(0<npts1)&&(1<npts2))
	{
		switch (surface_type)
		{
			case g_SHADED:
			case g_SHADED_TEXMAP:
			{
				switch (polygon_type)
				{
					case g_QUADRILATERAL:
					{
						number_of_points=npts1*npts2;
						return_code=1;
					} break;
					case g_TRIANGLE:
					{
						number_of_points=(npts1*(npts1+1))/2;
						return_code=1;
					} break;
					case g_GENERAL_POLYGON:
					{
						/* Do nothing */
					} break;
				}
			} break;
			case g_SH_DISCONTINUOUS:
			case g_SH_DISCONTINUOUS_TEXMAP:
			{
				number_of_points = npts1*npts2;
				return_code = 1;
			} break;
			case g_SURFACE_TYPE_BEFORE_FIRST:
			case g_SH_DISCONTINUOUS_STRIP:
			case g_SH_DISCONTINUOUS_STRIP_TEXMAP:
			case g_SURFACE_TYPE_AFTER_LAST:
			case g_SURFACE_TYPE_INVALID:
			{
				/* Do nothing */
			} break;
		}
		if (return_code)
		{
			nodes = (struct Render_node *)NULL;
			data_ptr = data_values;
			if (ALLOCATE(nodes, struct Render_node, number_of_points))
			{
				for (i = 0 ; return_code && (i < number_of_points) ; i++)
				{
					CAST_TO_FE_VALUE(position, surfpts[i], 3);
					if (!(Render_node_create(&nodes[i], data, position,
						 number_of_data_components, data_ptr)))
					{
						display_message(ERROR_MESSAGE,
								"render_voltex_to_finite_elements.  "
								"Could not set fields at node");
						return_code = 0;
					}
					if (number_of_data_components)
					{
						data_ptr += number_of_data_components;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"render_voltex_to_finite_elements.  "
					"Unable to allocate node array");
				return_code = 0;
			}
			if (return_code)
			{
				switch (surface_type)
				{
					case g_SHADED:
					case g_SHADED_TEXMAP:
					{
						switch (polygon_type)
						{
							case g_QUADRILATERAL:
							{
								index=0;
								for (j=0;j<npts2-1;j++)
								{
									for (i=0;i<npts1-1;i++)
									{
 										data->addSquare(number_of_data_components,
											&nodes[index], &nodes[index+1], &nodes[index+npts1], &nodes[index+npts1+1]);
										index++;
									}
									index++;
								}
							} break;
							case g_TRIANGLE:
							{
								/* triangle strip */
								index_1=0;
								index_2=index_1+npts1;
								for (i=npts1-1;i>0;i--)
								{
									data->addTriangle(number_of_data_components,
										&nodes[index_1], &nodes[index_1+1], &nodes[index_2]);
									index_1++;
									for (j=i-1;j>0;j--)
									{
										data->addTriangle(number_of_data_components,
											&nodes[index_1], &nodes[index_2+1], &nodes[index_2]);
										index_2++;
										data->addTriangle(number_of_data_components,
											&nodes[index_1], &nodes[index_1+1], &nodes[index_2]);
										index_1++;
									}
									index_1++;
									index_2++;
								}
							} break;
							case g_GENERAL_POLYGON:
							{
								/* Do nothing */
							} break;
						}
					} break;
					case g_SH_DISCONTINUOUS:
					case g_SH_DISCONTINUOUS_TEXMAP:
					{
						index = 0;
						switch (polygon_type)
						{
							case g_QUADRILATERAL:
							{
								for (i = 0; i<npts1; i++)
								{
									data->addSquare(number_of_data_components,
										&nodes[index], &nodes[index+1], &nodes[index+2], &nodes[index+3]);
									index += 4;
								}
							} break;
							case g_TRIANGLE:
							{
								for (i = 0; i < npts1; i++)
								{
									data->addTriangle(number_of_data_components,
										&nodes[index], &nodes[index + 1], &nodes[index + 2]);
									index += 3;
								}
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"render_surface_to_finite_elements.  Unsupported discontinuous polygon_type");
								return_code = 0;
							} break;
						}
					} break;
					case g_SURFACE_TYPE_BEFORE_FIRST:
					case g_SH_DISCONTINUOUS_STRIP:
					case g_SH_DISCONTINUOUS_STRIP_TEXMAP:
					case g_SURFACE_TYPE_AFTER_LAST:
					case g_SURFACE_TYPE_INVALID:
					{
						/* Do nothing */
					} break;
				}
			}
			if (nodes)
			{
				DEALLOCATE(nodes);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"render_surface_to_finite_elements.  Unsupported surface_type");
			return_code=0;
		}
	}
	else
	{
		if ((0<npts1)&&(1<npts2))
		{
			display_message(ERROR_MESSAGE,
				"render_surface_to_finite_elements.  Invalid argument(s)");
			return_code=0;
		}
		else
		{
			return_code=1;
		}
	}
	LEAVE;

	return (return_code);
} /* render_surface_to_finite_elements */

static int render_voltex_to_finite_elements(
	struct Render_to_finite_elements_data *data,
	int number_of_vertices, struct VT_iso_vertex **vertex_list, 
	int number_of_triangles, struct VT_iso_triangle **triangle_list,
	int number_of_data_components)
/*******************************************************************************
LAST MODIFIED : 8 December 2005

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;
	struct Render_node *nodes;
	FE_value_triple position;

	ENTER(render_voltex_to_finite_elements);

	return_code = 1;
	if (triangle_list && vertex_list && (0<number_of_triangles))
	{
		if (ALLOCATE(nodes, struct Render_node, number_of_vertices))
		{
			for (i = 0 ; return_code && (i < number_of_vertices) ; i++)
			{
				CAST_TO_FE_VALUE(position, vertex_list[i]->coordinates, 3);
				if (!(Render_node_create(&nodes[i], data, position,
					number_of_data_components, vertex_list[i]->data)))
				{
					display_message(ERROR_MESSAGE,
						"render_voltex_to_finite_elements.  "
						"Could not set fields at node");
					return_code = 0;
				}
			}
			if (return_code)
			{
				for (i = 0 ; return_code && (i < number_of_triangles) ; i++)
				{
					return_code = data->addTriangle(number_of_data_components,
						&nodes[triangle_list[i]->vertices[0]->index],
						&nodes[triangle_list[i]->vertices[1]->index],
						&nodes[triangle_list[i]->vertices[2]->index]);
				}
			}
			DEALLOCATE(nodes);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"render_voltex_to_finite_elements.  "
				"Unable to allocate node array");
			return_code = 0;
		}

	}
	LEAVE;

	return (return_code);
} /* render_voltex_to_finite_elements */

static int Graphics_object_render_to_finite_elements(
	struct GT_object *object, double time, 
	struct Render_to_finite_elements_data *data)
/*******************************************************************************
LAST MODIFIED : 8 December 2005

DESCRIPTION :
==============================================================================*/
{
	float proportion= 0,*times;
	int itime, number_of_times, return_code;
#if defined (NEW_CODE)
	struct GT_glyph_set *interpolate_glyph_set,*glyph_set,*glyph_set_2;
	struct GT_point *point;
	struct GT_pointset *interpolate_point_set,*point_set,*point_set_2;
#endif /* defined (NEW_CODE) */
	struct GT_polyline *interpolate_line,*line,*line_2;
	struct GT_surface *interpolate_surface,*surface,*surface_2;
	struct GT_voltex *voltex;
	union GT_primitive_list *primitive_list1 = NULL, *primitive_list2 = NULL;

	ENTER(Graphics_object_render_to_finite_elements);
	return_code=1;
	if (object && data)
	{
		number_of_times = object->number_of_times;
		if (0 < number_of_times)
		{
			itime = number_of_times;
			if ((itime>1)&&(times=object->times))
			{
				itime--;
				times += itime;
				if (time>= *times)
				{
					proportion=0;
				}
				else
				{
					while ((itime>0)&&(time< *times))
					{
						itime--;
						times--;
					}
					if (time< *times)
					{
						proportion=0;
					}
					else
					{
						proportion=times[1]-times[0];
						if (proportion>0)
						{
							proportion=time-times[0]/proportion;
						}
						else
						{
							proportion=0;
						}
					}
				}
			}
			else
			{
				itime=0;
				proportion=0;
			}
			if (object->primitive_lists &&
				(primitive_list1 = object->primitive_lists + itime))
			{
				if (proportion > 0)
				{
					if (!(primitive_list2 = object->primitive_lists + itime + 1))
					{
						display_message(ERROR_MESSAGE,
							"Graphics_object_render_to_finite_elements.  Invalid primitive_list");
						return_code = 0;
					}
				}
				else
				{
					primitive_list2 = (union GT_primitive_list *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Graphics_object_render_to_finite_elements.  Invalid primitive_lists");
				return_code = 0;
			}
		}
		if ((0 < number_of_times) && return_code)
		{
			switch (GT_object_get_type(object))
			{
#if defined (OLD_CODE)
				case g_GLYPH_SET:
				{
					if (glyph_set = primitive_list1->gt_glyph_set.first)
					{
						if (proportion>0)
						{
							glyph_set_2 = primitive_list2->gt_glyph_set.first;
							while (glyph_set&&glyph_set_2)
							{
								if (interpolate_glyph_set=morph_GT_glyph_set(proportion,
									glyph_set,glyph_set_2))
								{
									draw_glyph_set_vrml(vrml_file,
										interpolate_glyph_set->number_of_points,
										interpolate_glyph_set->point_list,
										interpolate_glyph_set->axis1_list,
										interpolate_glyph_set->axis2_list,
										interpolate_glyph_set->axis3_list,
										interpolate_glyph_set->scale_list,
										interpolate_glyph_set->glyph,
										interpolate_glyph_set->labels,
										interpolate_glyph_set->n_data_components,
										interpolate_glyph_set->data,
										object->default_material,object->spectrum,
										time,vrml_prototype_list);
									DESTROY(GT_glyph_set)(&interpolate_glyph_set);
								}
								glyph_set = glyph_set->ptrnext;
								glyph_set_2 = glyph_set_2->ptrnext;
							}
						}
						else
						{
							while (glyph_set)
							{
								draw_glyph_set_vrml(vrml_file,
									glyph_set->number_of_points,
									glyph_set->point_list, glyph_set->axis1_list,
									glyph_set->axis2_list, glyph_set->axis3_list,
									glyph_set->scale_list, glyph_set->glyph,
									glyph_set->labels,
									glyph_set->n_data_components, glyph_set->data,
									object->default_material, object->spectrum,
									time, vrml_prototype_list);
								glyph_set = glyph_set->ptrnext;
							}
						}
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"Graphics_object_render_to_finite_elements.  Missing glyph_set");
						return_code=0;
					}
				} break;
				case g_POINT:
				{
					if (point = primitive_list1->gt_point.first)
					{
						draw_point_set_vrml(vrml_file,
							1, point->position, &(point->text), point->marker_type,
							point->marker_size,point->n_data_components,point->data,
							object->default_material,object->spectrum);
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"Graphics_object_render_to_finite_elements.  Missing point");
						return_code=0;
					}
				} break;
				case g_POINTSET:
				{
					if (point_set = primitive_list1->gt_pointset.first)
					{
						if (proportion>0)
						{
							point_set_2 = primitive_list2->gt_pointset.first;
							while (point_set&&point_set_2)
							{
								if (interpolate_point_set=morph_GT_pointset(proportion,
									point_set, point_set_2))
								{
									draw_point_set_vrml(vrml_file,
										interpolate_point_set->n_pts,
										interpolate_point_set->pointlist,
										interpolate_point_set->text,
										interpolate_point_set->marker_type,
										interpolate_point_set->marker_size,
										interpolate_point_set->n_data_components,
										interpolate_point_set->data,
										object->default_material,object->spectrum);
									DESTROY(GT_pointset)(&interpolate_point_set);
								}
								point_set=point_set->ptrnext;
								point_set_2=point_set_2->ptrnext;
							}
						}
						else
						{
							draw_point_set_vrml(vrml_file, 
								point_set->n_pts,point_set->pointlist,
								point_set->text,point_set->marker_type,point_set->marker_size,
								point_set->n_data_components,point_set->data,
								object->default_material,object->spectrum);
						}
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"Graphics_object_render_to_finite_elements.  Missing point");
						return_code=0;
					}
				} break;
#endif /* defined (OLD_CODE) */
				case g_POLYLINE:
				{
					line = primitive_list1->gt_polyline.first;
					if (line != 0)
					{
						if (0<proportion)
						{
							line_2 = primitive_list2->gt_polyline.first;
							while (line&&line_2)
							{
								interpolate_line=
									morph_GT_polyline(proportion,line,line_2);
								if (interpolate_line)
								{
									render_polyline_to_finite_elements(data,
										interpolate_line->pointlist,
										interpolate_line->n_data_components,interpolate_line->data,
										object->default_material,object->spectrum,
										interpolate_line->n_pts,interpolate_line->polyline_type);
									DESTROY(GT_polyline)(&interpolate_line);
								}
								line=line->ptrnext;
								line_2=line_2->ptrnext;
							}
						}
						else
						{
							while (line)
							{
								render_polyline_to_finite_elements(data,
									line->pointlist,
									line->n_data_components,line->data,
									object->default_material,object->spectrum,
									line->n_pts,line->polyline_type);
								line=line->ptrnext;
							}
						}
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"Graphics_object_render_to_finite_elements.  Missing polyline");
						return_code=0;
					}
				} break;
				case g_SURFACE:
				{
					surface = primitive_list1->gt_surface.first;
					if (surface != 0)
					{
						if (0<proportion)
						{
							surface_2 = primitive_list2->gt_surface.first;
							while (surface&&surface_2)
							{
								interpolate_surface=morph_GT_surface(proportion,
									surface,surface_2);
								if (interpolate_surface != 0)
								{
									render_surface_to_finite_elements(data,
										interpolate_surface->pointlist,
										interpolate_surface->normallist,
										interpolate_surface->texturelist,
										interpolate_surface->n_data_components,
										interpolate_surface->data,interpolate_surface->n_pts1,
										interpolate_surface->n_pts2,surface->surface_type,
										surface->polygon);
									DESTROY(GT_surface)(&interpolate_surface);
								}
								surface=surface->ptrnext;
								surface_2=surface_2->ptrnext;
							}
						}
						else
						{
							while (surface)
							{
								render_surface_to_finite_elements(data,
									surface->pointlist,
									surface->normallist,surface->texturelist,
									surface->n_data_components,surface->data,
									surface->n_pts1,
									surface->n_pts2,surface->surface_type,surface->polygon);
								surface=surface->ptrnext;
							}
						}
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"Graphics_object_render_to_finite_elements.  Missing surface");
						return_code=0;
					}
				} break;
				case g_VOLTEX:
				{
					voltex = primitive_list1->gt_voltex.first;
					if (voltex != 0)
					{
						while (voltex)
						{
							render_voltex_to_finite_elements(data,
								voltex->number_of_vertices, voltex->vertex_list,
								voltex->number_of_triangles, voltex->triangle_list,
								voltex->n_data_components);
							voltex=voltex->ptrnext;
						}
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"Graphics_object_render_to_finite_elements.  Missing voltex");
						return_code=0;
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,"Graphics_object_render_to_finite_elements.  Invalid object type");
					return_code=0;
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_object_render_to_finite_elements.  Missing object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_object_render_to_finite_elements */

static int Graphics_object_render_to_finite_elements_iterator(
	struct GT_object *gt_object, double time, void *data_void)
/*******************************************************************************
LAST MODIFIED : 8 December 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code = 0;
	struct Render_to_finite_elements_data *data;

	ENTER(Graphics_object_render_to_finite_elements_iterator);
	if (gt_object && (data = (struct Render_to_finite_elements_data *)data_void))
	{
		data->setTime(time);
		switch(GT_object_get_type(gt_object))
		{
			case g_VOLTEX:
			case g_SURFACE:
			case g_POLYLINE:
			{
				return_code = Graphics_object_render_to_finite_elements(gt_object,
					time, data);
			} break;
			case g_POINT:
			case g_POINTSET:
			case g_GLYPH_SET:
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_object_render_to_finite_elements_iterator.  "
					"The graphics object %s is of a type not yet supported",
					gt_object->name);
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_object_render_to_finite_elements_iterator.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_object_render_to_finite_elements_iterator */

/*
Global functions
----------------
*/

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Render_to_finite_elements_mode)
{
	const char *enumerator_string;

	ENTER(ENUMERATOR_STRING(Render_to_finite_elements_mode));
	switch (enumerator_value)
	{
		case RENDER_TO_FINITE_ELEMENTS_LINEAR_PRODUCT:
		{

			enumerator_string = "render_linear_product_elements";
		} break;
		case RENDER_TO_FINITE_ELEMENTS_SURFACE_NODE_CLOUD:
		{
			enumerator_string = "render_surface_node_cloud";
		} break;
		default:
		{
			enumerator_string = (const char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Render_to_finite_elements_mode) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Render_to_finite_elements_mode)

int render_to_finite_elements(Cmiss_scene_id scene, Cmiss_region_id source_region,
	const char *graphic_name, enum Render_to_finite_elements_mode render_mode,
	Cmiss_region_id region, Cmiss_field_group_id group,
	Cmiss_field_id coordinate_field)
{
	int return_code;
	if (scene && region && coordinate_field)
	{
		return_code = build_Scene(scene);
		Render_to_finite_elements_data data(region, group, render_mode, coordinate_field);
		if (data.checkValidCoordinateField())
		{
			if (!source_region)
			{
				return_code=for_each_graphics_object_in_scene(scene,
					Graphics_object_render_to_finite_elements_iterator, (void *)&data);
			}
			else
			{
				return_code=Scene_export_region_graphics_object(scene, source_region,
					graphic_name, Graphics_object_render_to_finite_elements_iterator,
					(void *)&data);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"render_to_finite_elements.  "
				"Invalid or unsupported coordinate field");
			return_code = 0;
		}
	}
	else
	{
 		display_message(ERROR_MESSAGE,"render_to_finite_elements.  "
			"Invalid argument(s)");
		return_code=0;
	}
	return (return_code);
}
