/*******************************************************************************
FILE : render_to_finite_elements.c

LAST MODIFIED : 8 December 2005

DESCRIPTION :
Renders gtObjects to VRML file
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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "general/debug.h"
#include "general/enumerator_private.h"
#include "general/list.h"
#include "general/list_private.h"
#include "general/object.h"
#include "general/mystring.h"
#include "general/random.h"
#include "general/statistics.h"
#include "graphics/graphics_object.h"
#include "graphics/material.h"
#include "graphics/scene.h"
#include "graphics/spectrum.h"
#include "graphics/texture.h"
#include "user_interface/message.h"
#include "graphics/graphics_object_private.h"
#include "graphics/render_to_finite_elements.h"

/*
Module types
------------
*/

struct Render_node
{
	struct FE_node *fe_node;
	float *coordinates;
	float *data;
};

struct Render_to_finite_elements_data
{
	enum Render_to_finite_elements_mode render_mode;
	int node_offset;
	int element_offset;
	struct FE_region *fe_region;
 	struct FE_node *template_node;
	struct Computed_field *coordinate_field;
	struct FE_field *fe_coordinate_field;
}; /* struct Render_to_finite_elements_data */

/*
Module functions
----------------
*/

static struct FE_node *FE_region_add_node(struct Render_to_finite_elements_data *data, 
	FE_value time, float *coordinates)
/*******************************************************************************
LAST MODIFIED : 8 December 2005

DESCRIPTION :
Generates a finite_element node at the specified location
==============================================================================*/
{
	int node_number;
	struct FE_node *node, *return_node;

	ENTER(FE_region_add_node);

	return_node = (struct FE_node *)NULL;
	if (data->fe_region)
	{
		node_number = FE_region_get_next_FE_node_identifier(data->fe_region, data->node_offset);
		data->node_offset = node_number + 1;

		if (node = CREATE(FE_node)(node_number,
				(struct FE_region *)NULL, data->template_node))
		{	
			ACCESS(FE_node)(node);
			if (Computed_field_set_values_at_node(data->coordinate_field,
					node, time, coordinates))
			{
				if (FE_region_merge_FE_node(data->fe_region, node))
				{
					return_node = node;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"FE_region_add_node.  "
						"Could not merge node into region");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_region_add_node.  "
					"Could not set fields at node");
			}
			DEACCESS(FE_node)(&node);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_region_add_node.  "
				"Could not set fields at node");
		}
	}
	LEAVE;

	return (return_node);
} /* FE_region_add_node */

static int Render_node_create(struct Render_node *render_node,
	struct Render_to_finite_elements_data *data, FE_value time,
	float *coordinates, int number_of_data_components, float *data_values)
/*******************************************************************************
LAST MODIFIED : 8 December 2005

DESCRIPTION :
Generates a finite_element node at the specified location
==============================================================================*/
{
	int return_code;

	ENTER(create_Render_node);

	render_node->fe_node = (struct FE_node *)NULL;
	render_node->coordinates = (float *)NULL;
	render_node->data = (float *)NULL;
	return_code = 1;
	if (data->fe_region)
	{
		switch (data->render_mode)
		{	
			case RENDER_TO_FINITE_ELEMENTS_SURFACE_NODE_CLOUD:
			{
				/* Keep the coordinate pointer for later */
				render_node->coordinates = coordinates;
				if (number_of_data_components)
				{
					render_node->data = data_values;
				}
			} break;
			default:
			{
				render_node->fe_node = FE_region_add_node(data, time, coordinates);
			} break;
		}
	}
	LEAVE;

	return (return_code);
} /* Render_node_create */

static int FE_region_add_triangle(struct Render_to_finite_elements_data *data,
	FE_value time, int number_of_data_components,
	struct Render_node *node1, struct Render_node *node2, struct Render_node *node3)
/*******************************************************************************
LAST MODIFIED : 8 December 2005

DESCRIPTION :
Generates a finite_element representation of the specified triangle according
to the <render_mode>
==============================================================================*/
{
	double area, coordinate_1, coordinate_2, coordinate_3, density, expected_number;
	FE_value side1[3], side2[3], side3[3], position[3], xi1, xi2;
	int element_number, i, j, number_of_points, return_code;
	struct FE_element *element;

	ENTER(FE_region_add_triangle);

	return_code = 0;
	if (data->fe_region)
	{
		switch (data->render_mode)
		{
			case RENDER_TO_FINITE_ELEMENTS_SURFACE_NODE_CLOUD:
			{
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
					if (!FE_region_add_node(data, time, position))
					{
						return_code = 0;
					}
				}
			} break;
			case RENDER_TO_FINITE_ELEMENTS_LINEAR_PRODUCT:
			{
				if (node1 && node1->fe_node && node2 && node2->fe_node && node3 && node3->fe_node)
				{
					element_number = FE_region_get_next_FE_element_identifier(data->fe_region, CM_ELEMENT,
						data->element_offset);
					data->element_offset = element_number + 1;

					element = create_FE_element_with_line_shape(element_number, data->fe_region, /*dimension*/2);

					FE_element_define_tensor_product_basis(element, /*dimension*/2, LINEAR_LAGRANGE,
						data->fe_coordinate_field);

					set_FE_element_node(element, /*Node index*/0, node1->fe_node);
					set_FE_element_node(element, /*Node index*/1, node1->fe_node);
					set_FE_element_node(element, /*Node index*/2, node2->fe_node);
					set_FE_element_node(element, /*Node index*/3, node3->fe_node);
							
					if (!(return_code = FE_region_merge_FE_element_and_faces_and_nodes(data->fe_region, element)))
					{
						display_message(ERROR_MESSAGE,"FE_region_add_triangle.  "
							"Unable to merge triangle into fe_region.");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"FE_region_add_triangle.  "
						"Linear product render should have already created the nodes.");
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,"FE_region_add_triangle.  "
					"Unknown render mode.");
				return_code = 0;
			}
		}
	}
	LEAVE;

	return (return_code);
} /* FE_region_add_triangle */

static int FE_region_add_square(struct Render_to_finite_elements_data *data,
	FE_value time, int number_of_data_components,
	struct Render_node *node1, struct Render_node *node2, struct Render_node *node3, struct Render_node *node4)
/*******************************************************************************
LAST MODIFIED : 8 December 2005

DESCRIPTION :
Generates a finite_element representation of the specified square according
to the <render_mode>.  If the mode does not have a special representation then
it will default to creating to triangles.
The vertex ordering is the ususal order in cmgui.  
xi1=0,xi2=0 ; xi1=1,xi2=0 ; xi1=0,xi2=1 ; xi1=1,xi2=1
==============================================================================*/
{
	int return_code;

	ENTER(FE_region_add_square);

	return_code=0;
	if (data->fe_region)
	{
		switch (data->render_mode)
		{
			default:
			{
				return_code = FE_region_add_triangle(data, time, number_of_data_components,
					node1, node2, node3) &&
					FE_region_add_triangle(data, time, number_of_data_components,
					node2, node3, node4);
			} break;
		}
	}
	LEAVE;

	return (return_code);
} /* FE_region_add_square */

static int FE_region_add_line(struct Render_to_finite_elements_data *data,
	FE_value time, int number_of_data_components,
	struct Render_node *node1, struct Render_node *node2)
/*******************************************************************************
LAST MODIFIED : 14 July 2006

DESCRIPTION :
Generates a finite_element representation of the specified line according
to the <render_mode>
==============================================================================*/
{
	double length, density, expected_number;
	FE_value position[3], side[3], xi1;
	int element_number, i, j, number_of_points, return_code;
	struct FE_element *element;

	ENTER(FE_region_add_line);

	return_code = 0;
	if (data->fe_region)
	{
		switch (data->render_mode)
		{
			case RENDER_TO_FINITE_ELEMENTS_SURFACE_NODE_CLOUD:
			{
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
					if (!FE_region_add_node(data, time, position))
					{
						return_code = 0;
					}
				}
			} break;
			case RENDER_TO_FINITE_ELEMENTS_LINEAR_PRODUCT:
			{
				if (node1 && node1->fe_node && node2 && node2->fe_node)
				{
					element_number = FE_region_get_next_FE_element_identifier(data->fe_region, CM_ELEMENT,
						data->element_offset);
					data->element_offset = element_number + 1;

					element = create_FE_element_with_line_shape(element_number, data->fe_region,
						/*dimension*/1);

					FE_element_define_tensor_product_basis(element, /*dimension*/1,
						LINEAR_LAGRANGE, data->fe_coordinate_field);

					set_FE_element_node(element, /*Node index*/0, node1->fe_node);
					set_FE_element_node(element, /*Node index*/1, node2->fe_node);
							
					if (!(return_code = FE_region_merge_FE_element_and_faces_and_nodes(data->fe_region, element)))
					{
						display_message(ERROR_MESSAGE,"FE_region_add_line.  "
							"Unable to merge line into fe_region.");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"FE_region_add_line.  "
						"Linear product render should have already created the nodes.");
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,"FE_region_add_line.  "
					"Unknown render mode.");
				return_code = 0;
			}
		}
	}
	LEAVE;

	return (return_code);
} /* FE_region_add_line */

static int render_polyline_to_finite_elements(
	struct Render_to_finite_elements_data *data, FE_value time, 
	Triple *point_list,
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
	Triple *triple;

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
			}
		}
		if (return_code)
		{
			triple = point_list;
			data_ptr = data_values;
			if (ALLOCATE(nodes, struct Render_node, number_of_points))
			{
				for (i = 0 ; return_code && (i < number_of_points) ; i++)
				{
					if (!(Render_node_create(&nodes[i], data, time, *triple, 
						 number_of_data_components, data_ptr)))
					{
						display_message(ERROR_MESSAGE,
								"render_voltex_to_finite_elements.  "
								"Could not set fields at node");
						return_code = 0;
					}
					triple++;
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
						FE_region_add_line(data, time, number_of_data_components,
							&nodes[i], &nodes[i+1]);
					}
				} break;
				case g_PLAIN_DISCONTINUOUS:
				{
					for (i=0;i<n_pts;i++)
					{
						FE_region_add_line(data, time, number_of_data_components,
							&nodes[2*i], &nodes[2*i+1]);
					}
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
	struct Render_to_finite_elements_data *data, FE_value time,
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
	Triple *triple;

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
			case g_WIREFRAME_SHADED_TEXMAP:
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
				}
			} break;
		}
		if (return_code)
		{
			nodes = (struct Render_node *)NULL;
			triple = surfpts;
			data_ptr = data_values;
			if (ALLOCATE(nodes, struct Render_node, number_of_points))
			{
				for (i = 0 ; return_code && (i < number_of_points) ; i++)
				{
					if (!(Render_node_create(&nodes[i], data, time, *triple, 
						 number_of_data_components, data_ptr)))
					{
						display_message(ERROR_MESSAGE,
								"render_voltex_to_finite_elements.  "
								"Could not set fields at node");
						return_code = 0;
					}
					triple++;
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
					case g_WIREFRAME_SHADED_TEXMAP:
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
 										FE_region_add_square(data, time, number_of_data_components,
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
									FE_region_add_triangle(data, time, number_of_data_components,
										&nodes[index_1], &nodes[index_1+1], &nodes[index_2]);
									index_1++;
									for (j=i-1;j>0;j--)
									{
										FE_region_add_triangle(data, time, number_of_data_components, 
											&nodes[index_1], &nodes[index_2+1], &nodes[index_2]);
										index_2++;
										FE_region_add_triangle(data, time, number_of_data_components,
											&nodes[index_1], &nodes[index_1+1], &nodes[index_2]);
										index_1++;
									}
									index_1++;
									index_2++;
								}
							} break;
						}
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
	struct Render_to_finite_elements_data *data, FE_value time,
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

	ENTER(render_voltex_to_finite_elements);

	return_code = 1;
	if (triangle_list && vertex_list && (0<number_of_triangles))
	{
		if (ALLOCATE(nodes, struct Render_node, number_of_vertices))
		{
			for (i = 0 ; return_code && (i < number_of_vertices) ; i++)
			{
				if (!(Render_node_create(&nodes[i], data, time, 
					vertex_list[i]->coordinates,
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
					return_code = FE_region_add_triangle(data, time, number_of_data_components,
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
	float proportion,*times;
	int itime, number_of_times, return_code;
#if defined (NEW_CODE)
	struct GT_glyph_set *interpolate_glyph_set,*glyph_set,*glyph_set_2;
	struct GT_point *point;
	struct GT_pointset *interpolate_point_set,*point_set,*point_set_2;
#endif /* defined (NEW_CODE) */
	struct GT_polyline *interpolate_line,*line,*line_2;
	struct GT_surface *interpolate_surface,*surface,*surface_2;
	struct GT_voltex *voltex;
	union GT_primitive_list *primitive_list1, *primitive_list2;

	ENTER(Graphics_object_render_to_finite_elements);
	return_code=1;
	if (object && data->fe_region)
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
					if (line = primitive_list1->gt_polyline.first)
					{
						if (0<proportion)
						{
							line_2 = primitive_list2->gt_polyline.first;
							while (line&&line_2)
							{
								if (interpolate_line=
									morph_GT_polyline(proportion,line,line_2))
								{
									render_polyline_to_finite_elements(data, time,
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
								render_polyline_to_finite_elements(data, time,
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
					if (surface = primitive_list1->gt_surface.first)
					{
						if (0<proportion)
						{
							surface_2 = primitive_list2->gt_surface.first;
							while (surface&&surface_2)
							{
								if (interpolate_surface=morph_GT_surface(proportion,
									surface,surface_2))
								{
									render_surface_to_finite_elements(data, time,
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
								render_surface_to_finite_elements(data, time,
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
					if (voltex = primitive_list1->gt_voltex.first)
					{
						while (voltex)
						{
							render_voltex_to_finite_elements(data, time,
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
	int return_code;
	struct Render_to_finite_elements_data *data;

	ENTER(Graphics_object_render_to_finite_elements_iterator);
	if (gt_object && (data = (struct Render_to_finite_elements_data *)data_void))
	{
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
	char *enumerator_string;

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
			enumerator_string = (char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Render_to_finite_elements_mode) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Render_to_finite_elements_mode)

int render_to_finite_elements(struct Scene *scene, struct FE_region *fe_region,
	enum Render_to_finite_elements_mode render_mode, 
	struct Computed_field *coordinate_field)
/******************************************************************************
LAST MODIFIED : 8 December 2005

DESCRIPTION :
Renders the visible objects as finite elements into the specified <fe_region>.
==============================================================================*/
{
	int return_code;
	struct FE_node_field_creator *coordinate_node_field_creator;
	struct LIST(FE_field) *fe_coordinate_field_list;
	struct Render_to_finite_elements_data data;

	ENTER(render_to_finite_elements);
	if (scene && fe_region)
	{
		return_code = build_Scene(scene);

		data.render_mode = render_mode;
		data.element_offset = 1;
		data.node_offset = 1;
		data.fe_region = fe_region;
		data.coordinate_field = coordinate_field;
		data.fe_coordinate_field = (struct FE_field *)NULL;
		data.template_node = (struct FE_node *)NULL;

		if (render_mode == RENDER_TO_FINITE_ELEMENTS_SURFACE_NODE_CLOUD)
		{
			/* Set a random seed so that testing generates the same cloud
				each time */
			CMGUI_SEED_RANDOM(10000);
		}

		FE_region_begin_change(fe_region);
		FE_region_begin_define_faces(fe_region);

		if ((3 == Computed_field_get_number_of_components(coordinate_field)) &&
			(fe_coordinate_field_list =
				Computed_field_get_defining_FE_field_list(coordinate_field)) &&
			(1 == NUMBER_IN_LIST(FE_field)(fe_coordinate_field_list)) &&
			(data.fe_coordinate_field = FIRST_OBJECT_IN_LIST_THAT(FE_field)(
				(LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL,(void *)NULL,
				fe_coordinate_field_list)))
		{
			if ((coordinate_node_field_creator = CREATE(FE_node_field_creator)
					(/*number_of_components*/3)))
			{
				/* create the node */
				if ((data.template_node = CREATE(FE_node)(0, fe_region,
							(struct FE_node *)NULL)) &&
					define_FE_field_at_node(data.template_node, data.fe_coordinate_field,
						(struct FE_time_sequence *)NULL,
						coordinate_node_field_creator))
				{
					return_code=1;	
				}
				DESTROY(FE_node_field_creator)(
					&coordinate_node_field_creator);
			}
			else
			{
				return_code = 0;
			}
			DESTROY_LIST(FE_field)(&fe_coordinate_field_list);
		}
		else
		{
			display_message(ERROR_MESSAGE,"render_to_finite_elements.  "
				"Invalid or unsupported coordinate field");
			return_code = 0;
		}

		if (return_code)
		{
			return_code=for_each_graphics_object_in_scene(scene,
				Graphics_object_render_to_finite_elements_iterator, (void *)&data);
		}
		if (data.template_node)
		{
			DESTROY(FE_node)(&data.template_node);
		}
		Computed_field_clear_cache(coordinate_field);
		FE_region_end_define_faces(fe_region);
		FE_region_end_change(fe_region);
	}
	else
	{
 		display_message(ERROR_MESSAGE,"render_to_finite_elements.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* render_to_finite_elements */
