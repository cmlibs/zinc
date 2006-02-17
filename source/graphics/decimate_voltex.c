/*******************************************************************************
FILE : decimate_voltex.c

LAST MODIFIED : 11 November 2005

DESCRIPTION :
Decimate the triangles in the GT_voltex to satisfy a curvature threshold.
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
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "general/compare.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/matrix_vector.h"
#include "general/mystring.h"
#include "general/object.h"
#include "general/octree.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/graphics_object.h"
#include "graphics/material.h"
#include "graphics/rendergl.h"
#include "graphics/spectrum.h"
#include "graphics/volume_texture.h"
#include "user_interface/message.h"
#include "graphics/graphics_object_private.h"

/* Predeclare the Decimation_cost as we have a circular dependency */
struct Decimation_cost;
DECLARE_LIST_TYPES(Decimation_cost);
PROTOTYPE_OBJECT_FUNCTIONS(Decimation_cost);
PROTOTYPE_LIST_FUNCTIONS(Decimation_cost);

struct Decimation_quadric
/*******************************************************************************
LAST MODIFIED : 23 February 2005

DESCRIPTION :
Stores an VT_iso_vertex and its Decimation_quadric
==============================================================================*/
{
	struct VT_iso_vertex *vertex;
	double matrix[10];
	struct LIST(Decimation_cost) *dependent_cost_list;
	int access_count;
};

static struct Decimation_quadric *CREATE(Decimation_quadric)(void)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
==============================================================================*/
{
	struct Decimation_quadric *quadric;

	ENTER(CREATE(Decimation_quadric));
	if (ALLOCATE(quadric, struct Decimation_quadric, 1))
	{
		quadric->vertex = (struct VT_iso_vertex *)NULL;
		quadric->matrix[0] = 0.0;
		quadric->matrix[1] = 0.0;
		quadric->matrix[2] = 0.0;
		quadric->matrix[3] = 0.0;
		quadric->matrix[4] = 0.0;
		quadric->matrix[5] = 0.0;
		quadric->matrix[6] = 0.0;
		quadric->matrix[7] = 0.0;
		quadric->matrix[8] = 0.0;
		quadric->matrix[9] = 0.0;
		quadric->dependent_cost_list = CREATE(LIST(Decimation_cost))();
		quadric->access_count = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Decimation_quadric).  "
			"Unable to allocate memory for memory_quadric list structure");
		quadric = (struct Decimation_quadric *)NULL;
	}
	LEAVE;
	
	return (quadric);
} /* CREATE(Decimation_quadric) */

int DESTROY(Decimation_quadric)(struct Decimation_quadric **quadric_address)
/*******************************************************************************
LAST MODIFIED : 28 February 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Decimation_quadric *quadric;

	ENTER(DESTROY(Decimation_quadric));
	if (quadric_address && (quadric = *quadric_address))
	{
		if (quadric->access_count <= 0)
		{
			DESTROY(LIST(Decimation_cost))(&quadric->dependent_cost_list);
			DEALLOCATE(*quadric_address);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Decimation_quadric).  Destroy called when access count > 0.");
			*quadric_address = (struct Decimation_quadric *)NULL;
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Decimation_quadric).  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Decimation_quadric) */

DECLARE_LIST_TYPES(Decimation_quadric);

PROTOTYPE_OBJECT_FUNCTIONS(Decimation_quadric);
PROTOTYPE_LIST_FUNCTIONS(Decimation_quadric);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Decimation_quadric,vertex,struct VT_iso_vertex *);

DECLARE_OBJECT_FUNCTIONS(Decimation_quadric)
FULL_DECLARE_INDEXED_LIST_TYPE(Decimation_quadric);
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Decimation_quadric,vertex,struct VT_iso_vertex *,compare_pointer)
DECLARE_INDEXED_LIST_FUNCTIONS(Decimation_quadric)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Decimation_quadric,vertex,
	struct VT_iso_vertex *, compare_pointer)

struct Decimation_cost
/*******************************************************************************
LAST MODIFIED : 23 February 2005

DESCRIPTION :
Stores an VT_iso_vertex and its Decimation_quadric
==============================================================================*/
{
	struct Decimation_quadric *quadric1;
	struct Decimation_quadric *quadric2;
	double coordinates[3];
	double cost;
	struct Decimation_cost *self;  /* Need a self pointer so the compare can use
												 additional fields as well as the cost */
	int invalid_cost_counter; /* Count the number of times this cost has been 
										  selected as optimal but the cost is rejected due
										  to inverting triangles */
	/* Add an index for sorting these costs without using the pointer
		so that the decimation is repeatable for testing */
	int index;
	int access_count;
};

int compare_decimation_cost(struct Decimation_cost *cost1,
	struct Decimation_cost *cost2)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Sorts decimation_cost objects, first by the cost and then by an index which is
bit shifted so that it doesn't number objects created sequentially close to each
other.
==============================================================================*/
{
	int return_code;

	ENTER(compare_decimation_cost);
	if (cost1->cost < cost2->cost)
	{
		return_code = -1;
	}
	else
	{
		if (cost1->cost > cost2->cost)
		{
			return_code = 1;
		}
		else
		{
			if (cost1->index < cost2->index)
			{
				return_code = -1;
			}
			else
			{
				if (cost1->index > cost2->index)
				{
					return_code = 1;
				}
				else
				{
					return_code = 0;
				}
			}
		}
	}
	LEAVE;

	return (return_code);
} /* compare_decimation_cost */

static struct Decimation_cost *CREATE(Decimation_cost)(int index)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
==============================================================================*/
{
	struct Decimation_cost *cost;

	ENTER(CREATE(Decimation_cost));
	if (ALLOCATE(cost, struct Decimation_cost, 1))
	{
		cost->quadric1 = (struct Decimation_quadric *)NULL;
		cost->quadric2 = (struct Decimation_quadric *)NULL;
		cost->coordinates[0] = 0.0;
		cost->coordinates[1] = 0.0;
		cost->coordinates[2] = 0.0;
		cost->cost = 0.0;
		cost->self = cost;
		cost->invalid_cost_counter = 0;
		cost->index = index;
		cost->access_count = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Decimation_cost).  "
			"Unable to allocate memory for memory_cost list structure");
		cost = (struct Decimation_cost *)NULL;
	}
	LEAVE;
	
	return (cost);
} /* CREATE(Decimation_cost) */

int DESTROY(Decimation_cost)(struct Decimation_cost **cost_address)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Decimation_cost *cost;

	ENTER(DESTROY(Decimation_cost));
	if (cost_address && (cost = *cost_address))
	{
		if (cost->access_count <= 0)
		{
			if (cost->quadric1)
			{
				DEACCESS(Decimation_quadric)(&cost->quadric1);
			}
			if (cost->quadric2)
			{
				DEACCESS(Decimation_quadric)(&cost->quadric2);
			}
			DEALLOCATE(*cost_address);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Decimation_cost).  Destroy called when access count > 0.");
			*cost_address = (struct Decimation_cost *)NULL;
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Decimation_cost).  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Decimation_cost) */

PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Decimation_cost,self,
	struct Decimation_cost *);

DECLARE_OBJECT_FUNCTIONS(Decimation_cost)
FULL_DECLARE_INDEXED_LIST_TYPE(Decimation_cost);
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Decimation_cost,self,
	struct Decimation_cost *,compare_decimation_cost)
DECLARE_INDEXED_LIST_FUNCTIONS(Decimation_cost)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Decimation_cost,self,
	struct Decimation_cost *,compare_decimation_cost)

static int Decimation_cost_has_quadric(struct Decimation_cost *cost,
	void *quadric_void)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Returns 1 if the <cost> structure references the specified <quadric_void>.
==============================================================================*/
{
	int return_code;

	if (cost)
	{
		if ((cost->quadric1 == (struct Decimation_quadric *)quadric_void) ||
			(cost->quadric2 == (struct Decimation_quadric *)quadric_void))
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
			"Decimation_cost_has_quadric.  Invalid arguments.");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* calculate_decimation_cost */

static int check_triangles_are_not_inverted(struct VT_iso_vertex *vertex,
   struct VT_iso_vertex *other_vertex, double *new_coord)
/*******************************************************************************
LAST MODIFIED : 18 March 2005

DESCRIPTION :
Checks to see if collapsing <vertex> and <other_vertex> to location <new_coord>
will cause any of the triangles involving <vertex> to invert.  Returns true
if no triangles are inverted and false if any of them are.  If a triangle has
both <vertex> and <other_vertex> then it is expected it will collapse and be
removed, so it is not checked.  Test again with the vertices in the other order
if you want to check the triangle lists of both vertices.
==============================================================================*/
{
	double a[3], b[3], v1[3], v2[3], v3[4];
	int collapsing_triangle, j, k, number_of_triangles, return_code;
	struct VT_iso_vertex *vertexb, *vertexc;

	return_code = 1;
	number_of_triangles = vertex->number_of_triangles;
	for (j = 0 ; return_code && (j < number_of_triangles) ; j++)
	{
		collapsing_triangle = 0;
		if (vertex == vertex->triangles[j]->vertices[0])
		{
			if ((other_vertex == vertex->triangles[j]->vertices[1])
				|| (other_vertex == vertex->triangles[j]->vertices[2]))
			{
				collapsing_triangle = 1;
			}
			else
			{
				vertexb = vertex->triangles[j]->vertices[1];
				vertexc = vertex->triangles[j]->vertices[2];
			}
		}
		else if (vertex == vertex->triangles[j]->vertices[1])
		{
			if ((other_vertex == vertex->triangles[j]->vertices[0])
				|| (other_vertex == vertex->triangles[j]->vertices[2]))
			{
				collapsing_triangle = 1;
			}
			else
			{
				vertexb = vertex->triangles[j]->vertices[0];
				vertexc = vertex->triangles[j]->vertices[2];
			}
		}
		else
		{
			if ((other_vertex == vertex->triangles[j]->vertices[0])
				|| (other_vertex == vertex->triangles[j]->vertices[1]))
			{
				collapsing_triangle = 1;
			}
			else
			{
				vertexb = vertex->triangles[j]->vertices[0];
				vertexc = vertex->triangles[j]->vertices[1];
			}
		}
		if (!collapsing_triangle)
		{
			/* Calculate current normal and proposed normal */
			for (k = 0 ; k < 3 ; k++)
			{
				v1[k] = vertexb->coordinates[k] - vertexc->coordinates[k];
				v2[k] = vertex->coordinates[k] - vertexc->coordinates[k];
				v3[k] = new_coord[k] - vertexc->coordinates[k];
			}
			cross_product3(v1, v2, a);
			cross_product3(v1, v3, b);

			if (dot_product3(a, b) < -1.0e-12)
			{
				return_code = 0;
			}
		}
	}

	return (return_code);
}

static int Decimation_cost_calculate_cost(struct Decimation_cost *cost,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 28 February 2005

DESCRIPTION :
Calculates the cost of a collapse based on the quadrics.
==============================================================================*/
{
	double a, b, c, d, edge_matrix[16], edge_matrix_lu[16], length, rhs[4], vTq[4];
	int i, lu_index[4], return_code;
	struct Decimation_quadric *quadric, *quadric2;

	USE_PARAMETER(dummy_void);

	quadric = cost->quadric1;
	quadric2 = cost->quadric2;

	length = (quadric->vertex->coordinates[0] - quadric2->vertex->coordinates[0]) *
		(quadric->vertex->coordinates[0] - quadric2->vertex->coordinates[0]) +
		(quadric->vertex->coordinates[1] - quadric2->vertex->coordinates[1]) *
		(quadric->vertex->coordinates[1] - quadric2->vertex->coordinates[1]) +
		(quadric->vertex->coordinates[2] - quadric2->vertex->coordinates[2]) *
		(quadric->vertex->coordinates[2] - quadric2->vertex->coordinates[2]) ;
	if (length > 0.0)
	{
		return_code = 1;

		length = sqrt(length);
		/* Expand out the quadric to its full matrix replacing the
			bottom row with an identity row and perform an
			LU decompose and backsub with a [0 0 0 1] rhs. */
		edge_matrix[0] = quadric->matrix[0] + quadric2->matrix[0];
		edge_matrix[1] = quadric->matrix[1] + quadric2->matrix[1];
		edge_matrix[2] = quadric->matrix[2] + quadric2->matrix[2];
		edge_matrix[3] = quadric->matrix[3] + quadric2->matrix[3];
	
		edge_matrix[4] = edge_matrix[1];
		edge_matrix[5] = quadric->matrix[4] + quadric2->matrix[4];
		edge_matrix[6] = quadric->matrix[5] + quadric2->matrix[5];
		edge_matrix[7] = quadric->matrix[6] + quadric2->matrix[6];
	
		edge_matrix[8] = edge_matrix[2];
		edge_matrix[9] = edge_matrix[6];
		edge_matrix[10] = quadric->matrix[7] + quadric2->matrix[7];
		edge_matrix[11] = quadric->matrix[8] + quadric2->matrix[8];
	
		edge_matrix[12] = edge_matrix[3];
		edge_matrix[13] = edge_matrix[7];
		edge_matrix[14] = edge_matrix[11];
		edge_matrix[15] = quadric->matrix[9] + quadric2->matrix[9];
	
		for (i = 0 ; i < 12 ; i++)
		{
			edge_matrix_lu[i] = edge_matrix[i];
		}
		edge_matrix_lu[12] = 0.0;
		edge_matrix_lu[13] = 0.0;
		edge_matrix_lu[14] = 0.0;
		edge_matrix_lu[15] = 1.0;

		rhs[0] = 0.0;
		rhs[1] = 0.0;
		rhs[2] = 0.0;
		rhs[3] = 1.0;

		/* Use a low tolerance to singular matrices */
		if ((cost->invalid_cost_counter == 0) && 
			LU_decompose(/*n*/4, edge_matrix_lu, lu_index, &d,/*singular_tolerance*/1.0e-5))
		{
			LU_backsubstitute(/*n*/4, edge_matrix_lu, lu_index, rhs);
								
			cost->coordinates[0] = rhs[0];
			cost->coordinates[1] = rhs[1];
			cost->coordinates[2] = rhs[2];
								
			/* error = vT Q v; */
			multiply_matrix(/*m*/1, /*s*/4, /*n*/4, rhs, edge_matrix, vTq);
			/* Actually a dot product really */
			multiply_matrix(/*m*/1, /*s*/4, /*4*/1, vTq, rhs, &cost->cost);

		}
		else
		{
			if (cost->invalid_cost_counter == 0)
			{
				cost->invalid_cost_counter = 1;
			}

			/* Try positions of vertex1 and vertex2 and (vertex1+vertex2)/2 */
			rhs[0] = (quadric->vertex->coordinates[0] + quadric2->vertex->coordinates[0]) / 2.0;
			rhs[1] = (quadric->vertex->coordinates[1] + quadric2->vertex->coordinates[1]) / 2.0;
			rhs[2] = (quadric->vertex->coordinates[2] + quadric2->vertex->coordinates[2]) / 2.0;
			rhs[3] = 1.0;
			cost->coordinates[0] = rhs[0];
			cost->coordinates[1] = rhs[1];
			cost->coordinates[2] = rhs[2];
			/* error = vT Q v; */
			multiply_matrix(/*m*/1, /*s*/4, /*n*/4, rhs, edge_matrix, vTq);
			/* Actually a dot product really */
			multiply_matrix(/*m*/1, /*s*/4, /*4*/1, vTq, rhs, &a);

			rhs[0] = quadric->vertex->coordinates[0];
			rhs[1] = quadric->vertex->coordinates[1];
			rhs[2] = quadric->vertex->coordinates[2];
			rhs[3] = 1.0;
			/* error = vT Q v; */
			multiply_matrix(/*m*/1, /*s*/4, /*n*/4, rhs, edge_matrix, vTq);
			/* Actually a dot product really */
			multiply_matrix(/*m*/1, /*s*/4, /*4*/1, vTq, rhs, &b);

			rhs[0] = quadric2->vertex->coordinates[0];
			rhs[1] = quadric2->vertex->coordinates[1];
			rhs[2] = quadric2->vertex->coordinates[2];
			rhs[3] = 1.0;
			/* error = vT Q v; */
			multiply_matrix(/*m*/1, /*s*/4, /*n*/4, rhs, edge_matrix, vTq);
			/* Actually a dot product really */
			multiply_matrix(/*m*/1, /*s*/4, /*4*/1, vTq, rhs, &c);

			switch (cost->invalid_cost_counter)
			{
				case 1:
				{
					/* Find the least value */
					cost->cost = a;
					if (b < cost->cost)
					{
						cost->cost = b;
						cost->coordinates[0] = quadric->vertex->coordinates[0];
						cost->coordinates[1] = quadric->vertex->coordinates[1];
						cost->coordinates[2] = quadric->vertex->coordinates[2];
					}
					if (c < cost->cost)
					{
						cost->cost = c;
						cost->coordinates[0] = quadric2->vertex->coordinates[0];
						cost->coordinates[1] = quadric2->vertex->coordinates[1];
						cost->coordinates[2] = quadric2->vertex->coordinates[2];
					}
				} break;
				case 2:
				{
					/* Find the middle value */
					if (((a <= b) && (b < c)) || (c <= b) && (b < a)) 
					{
						cost->cost = b;
						cost->coordinates[0] = quadric->vertex->coordinates[0];
						cost->coordinates[1] = quadric->vertex->coordinates[1];
						cost->coordinates[2] = quadric->vertex->coordinates[2];
					}
					else if (((a <= c) && (c < b)) || (b <= c) && (c < a)) 
					{
						cost->cost = c;
						cost->coordinates[0] = quadric2->vertex->coordinates[0];
						cost->coordinates[1] = quadric2->vertex->coordinates[1];
						cost->coordinates[2] = quadric2->vertex->coordinates[2];
					}
					else
					{
						cost->cost = a;
					}
				} break;
				case 3:
				{
					/* Find the maximum value */
					cost->cost = a;
					if (b >= cost->cost)
					{
						cost->cost = b;
						cost->coordinates[0] = quadric->vertex->coordinates[0];
						cost->coordinates[1] = quadric->vertex->coordinates[1];
						cost->coordinates[2] = quadric->vertex->coordinates[2];
					}
					if (c >= cost->cost)
					{
						cost->cost = c;
						cost->coordinates[0] = quadric2->vertex->coordinates[0];
						cost->coordinates[1] = quadric2->vertex->coordinates[1];
						cost->coordinates[2] = quadric2->vertex->coordinates[2];
					}
				} break;
				default:
				{
					/* Not an error, just no more options */
					cost->cost = 0;
					return_code = 0;
				}
			}
		}

		/* Non dimensionalise the cost by normalising it against the edge length */
		cost->cost /= length;

	}
	else
	{
		display_message(ERROR_MESSAGE,"Decimation_cost_calculate_cost.  "
			"Vertices are at the same location.");
		return_code = 0;
	}
	
	LEAVE;
	
	return (return_code);
} /* Decimation_cost_calculate_cost */

int GT_voltex_decimate_triangles(struct GT_voltex *voltex,
	double threshold_distance)
/*******************************************************************************
LAST MODIFIED : 11 November 2005

DESCRIPTION :
Decimates triangle mesh
Implementing edge collapses following Garland and Heckbert 
"Surface Simplification Using Quadric Error Metrics" SIGGRAPH 97

Currently this routine will allow collapsed triangles which have hanging nodes.
The storage of the could be generalised to allow it to work on triangular meshes
other than just MC_iso_surfaces, maybe graphics objects.  The normals probably
are not being updated properly either (if the collapsed triangles were fixed then
the normals should be calclated after this step rather than before).  The data
structures, Decimation_quadrics (vertices) and Decimation_cost (edges) should be
better merged with the rest of the storage and so lists could be used instead of
the current arrays of triangle pointers.
The decimation_cost could include a penalty according to the rate at which the 
data variables vary along the edge, so that where a display data value is varying
fastest the edge is less likely to collapse.
==============================================================================*/
{
	double a[3], boundary_weight, d, q[3], v1[3], v2[3];
	int add_triangle, allocated_edges, compacting_triangles, compacting_vertices,
		cost_index, edge_count, *edges_counts, found, i, j, k, l,
		*new_edges_counts, number_of_edges, number_of_planes,
		number_of_triangles, remove_triangle, return_code, triangle_index,
		triangles_removed, vertex_index;
	struct Decimation_cost *cost, *update_cost;
	struct Decimation_quadric *independent_quadric, *quadric, *quadric2;
	struct LIST(Decimation_cost) *cost_list, *pre_cost_list;
	struct LIST(Decimation_quadric) *quadric_list;
	struct VT_iso_vertex **edges_list, **new_edges_list, **new_vertex_list_ptr,
		*vertex, *vertex2, *vertex3, **vertex_list_ptr;
	struct VT_iso_triangle **edges_triangles, **new_edges_triangles,
		**new_triangle_list_ptr, **triangle_list_ptr;

	ENTER(GT_voltex_decimate_triangles);
	return_code = 1;

	/* 1. Computed Q quadric matrices for each vertex */
	quadric_list = CREATE(LIST(Decimation_quadric))();

	/* For each vertex calculate Q, by summing Kp for each plane */
	for (i = 0 ; i < voltex->number_of_vertices ; i++)
	{
		if (quadric = CREATE(Decimation_quadric)())
		{
			vertex = voltex->vertex_list[i];
			quadric->vertex = vertex;

			number_of_planes = vertex->number_of_triangles;
			for (j = 0 ; j < number_of_planes ; j++)
			{
				/* Calculate the plane equation for this triangle */
				for (k = 0 ; k < 3 ; k++)
				{
					/* Just calculate the plane using the vertices in the triangle list */ 
					v1[k] = vertex->triangles[j]->vertices[1]->coordinates[k] -
						vertex->triangles[j]->vertices[0]->coordinates[k];
					v2[k] = vertex->triangles[j]->vertices[2]->coordinates[k] -
						vertex->triangles[j]->vertices[0]->coordinates[k];
					q[k] = vertex->coordinates[k];
				}
				cross_product3(v1, v2, a);
				if (1.0e-8 > norm3(a))
				{
					display_message(ERROR_MESSAGE,"decimate_triangles.  "
						"Triangle has no area.");
				}
				else
				{
					normalize3(a);
					d = - dot_product3(a, q);

					quadric->matrix[0] += a[0] * a[0];
					quadric->matrix[1] += a[0] * a[1];
					quadric->matrix[2] += a[0] * a[2];
					quadric->matrix[3] += a[0] * d;
					quadric->matrix[4] += a[1] * a[1];
					quadric->matrix[5] += a[1] * a[2];
					quadric->matrix[6] += a[1] * d;
					quadric->matrix[7] += a[2] * a[2];
					quadric->matrix[8] += a[2] * d;
					quadric->matrix[9] += d * d;

				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"decimate_triangles.  "
				"Unable to allocate quadric storage memory.");
		}
		ADD_OBJECT_TO_LIST(Decimation_quadric)(quadric, quadric_list);
	}

	/* 2. Find the edges eligible for contraction and for each boundary
	 edge add the plane perpendicular to the plane of triangle which runs along
	 the edge into the quadrics for each vertex */
	pre_cost_list = CREATE(LIST(Decimation_cost))();
	cost_index = 1;
	allocated_edges = 40;
	ALLOCATE(edges_list, struct VT_iso_vertex *, allocated_edges);
	ALLOCATE(edges_counts, int, allocated_edges);
	ALLOCATE(edges_triangles, struct VT_iso_triangle *, allocated_edges);
	for (i = 0 ; i < voltex->number_of_vertices ; i++)
	{
		vertex = voltex->vertex_list[i];
		if (quadric = FIND_BY_IDENTIFIER_IN_LIST(Decimation_quadric,
			vertex)(vertex, quadric_list))
		{
			number_of_edges = vertex->number_of_triangles;
			if (number_of_edges > allocated_edges)
			{
				if (REALLOCATE(new_edges_list, edges_list, struct VT_iso_vertex *, allocated_edges + 20) &&
					REALLOCATE(new_edges_counts, edges_counts, int, allocated_edges + 20) &&
					REALLOCATE(new_edges_triangles, edges_triangles, struct VT_iso_triangle *, allocated_edges + 20))
				{
					edges_list = new_edges_list;
					edges_counts = new_edges_counts;
					edges_triangles = new_edges_triangles;
					allocated_edges += 20;
				}
			}
			edge_count = 0;
			for (j = 0 ; j < number_of_edges ; j++)
			{
				/* Check the vertices for previous triangles so that we don't
					add each edge for possibly two trianges to the same vertex,
					by counting our encounters for each vertex we can then find the
					boundary edges */
			
				for (k = 0 ; k < 3 ; k++)
				{
					if (vertex != vertex->triangles[j]->vertices[k])
					{
						vertex2 = vertex->triangles[j]->vertices[k];
						/* A simple search seems OK as this list should be < 15 */
						found = 0;
						l = 0;
						while (!found && (l < edge_count))
						{
							if (vertex2 == edges_list[l])
							{
								found = 1;
								edges_counts[l]++;
							}
							l++;
						}
						if (!found)
						{
							if (edge_count >= allocated_edges)
							{
								if (REALLOCATE(new_edges_list, edges_list, struct VT_iso_vertex *, allocated_edges + 20) &&
									REALLOCATE(new_edges_counts, edges_counts, int, allocated_edges + 20) &&
									REALLOCATE(new_edges_triangles, edges_triangles, struct VT_iso_triangle *, allocated_edges + 20))
								{
									edges_list = new_edges_list;
									edges_counts = new_edges_counts;
									edges_triangles = new_edges_triangles;
									allocated_edges += 20;
								}
							}
							
							edges_list[edge_count] = vertex2;
							edges_counts[edge_count] = 1;
							edges_triangles[edge_count] = vertex->triangles[j];
							edge_count++;
								
							if (vertex->index < vertex2->index)
							{
								/* Only create costs for the vertices which have a larger
									pointer so that we don't get each edge from both ends */
								/* Reorder the bits in the index so that we don't favour work
									in the order they are created, but haven't swapped every bit */
								if (cost = CREATE(Decimation_cost)(((cost_index & 0xf) << 28) +
										((cost_index & 0xf0) << 20) + ((cost_index & 0xff00) << 8) 
										+ ((cost_index & 0xff0000) >> 8) + ((cost_index & 0xff000000) >> 24)))
								{
									cost_index++;
									cost->quadric1 = ACCESS(Decimation_quadric)(quadric);
									if (quadric2 = FIND_BY_IDENTIFIER_IN_LIST(Decimation_quadric,
											vertex)(vertex2, quadric_list))
									{
										cost->quadric2 = ACCESS(Decimation_quadric)(quadric2);

										/* Put this into a temporary list as we are still updating
											the quadrics with the edge penalties and we should not
											have any objects in a list when we update its cost as
											this is the identifier for the list. */
										cost->cost = 0.0;
										ADD_OBJECT_TO_LIST(Decimation_cost)(cost, pre_cost_list);
									}
									else
									{
										display_message(ERROR_MESSAGE,"decimate_triangles.  "
											"Unable to find quadric for second vertex.");
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,"decimate_triangles.  "
										"Unable to allocate cost storage memory.");
								}
							}
						}
					}
				}
			}
			for (j = 0 ; j < edge_count ; j++)
			{
				/* Only if only 1 triangle has this vertex and only when the 
					vertex2 pointer is less than the quadric1 ptr so that we only
					do this from one end. */
				if ((edges_counts[j] == 1) && (vertex->index < edges_list[j]->index))
				{
					/* This is a boundary edge, add a new plane into the quadrics of both
						vertices */
					vertex2 = edges_list[j];
					quadric2 = FIND_BY_IDENTIFIER_IN_LIST(Decimation_quadric,
						vertex)(vertex2, quadric_list);
					if (vertex == edges_triangles[j]->vertices[0])
					{
						if (vertex2 == edges_triangles[j]->vertices[1])
						{
							vertex3 = edges_triangles[j]->vertices[2];
						}
						else
						{
							vertex3 = edges_triangles[j]->vertices[1];
						}
					}
					else if (vertex == edges_triangles[j]->vertices[1])
					{
						if (vertex2 == edges_triangles[j]->vertices[0])
						{
							vertex3 = edges_triangles[j]->vertices[2];
						}
						else
						{
							vertex3 = edges_triangles[j]->vertices[0];
						}
					}
					else
					{
						if (vertex2 == edges_triangles[j]->vertices[0])
						{
							vertex3 = edges_triangles[j]->vertices[1];
						}
						else
						{
							vertex3 = edges_triangles[j]->vertices[0];
						}
					}

					/* Calculate the plane equation for this triangle */
					for (k = 0 ; k < 3 ; k++)
					{
						v1[k] = vertex3->coordinates[k] - vertex->coordinates[k];
						v2[k] = vertex2->coordinates[k] - vertex->coordinates[k];
						q[k] = vertex->coordinates[k];
					}
					d = dot_product3(v1, v2);
					d /= norm3(v2);
					d /= norm3(v2);
					for (k = 0 ; k < 3 ; k++)
					{
						a[k] = v1[k] - d * v2[k];
					}
					if (1.0e-8 > norm3(a))
					{
						display_message(ERROR_MESSAGE,"decimate_triangles.  "
							"Triangle has no area.");
					}
					else
					{
						normalize3(a);
						d = - dot_product3(a, q);
						boundary_weight = 1e6;

						quadric->matrix[0] += boundary_weight * a[0] * a[0];
						quadric->matrix[1] += boundary_weight * a[0] * a[1];
						quadric->matrix[2] += boundary_weight * a[0] * a[2];
						quadric->matrix[3] += boundary_weight * a[0] * d;
						quadric->matrix[4] += boundary_weight * a[1] * a[1];
						quadric->matrix[5] += boundary_weight * a[1] * a[2];
						quadric->matrix[6] += boundary_weight * a[1] * d;
						quadric->matrix[7] += boundary_weight * a[2] * a[2];
						quadric->matrix[8] += boundary_weight * a[2] * d;
						quadric->matrix[9] += boundary_weight * d * d;

						quadric2->matrix[0] += boundary_weight * a[0] * a[0];
						quadric2->matrix[1] += boundary_weight * a[0] * a[1];
						quadric2->matrix[2] += boundary_weight * a[0] * a[2];
						quadric2->matrix[3] += boundary_weight * a[0] * d;
						quadric2->matrix[4] += boundary_weight * a[1] * a[1];
						quadric2->matrix[5] += boundary_weight * a[1] * a[2];
						quadric2->matrix[6] += boundary_weight * a[1] * d;
						quadric2->matrix[7] += boundary_weight * a[2] * a[2];
						quadric2->matrix[8] += boundary_weight * a[2] * d;
						quadric2->matrix[9] += boundary_weight * d * d;
					}
				
					
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"decimate_triangles.  "
				"Unable to find quadric for first vertex.");
		}
	}
	DEALLOCATE(edges_list);
	DEALLOCATE(edges_counts);
	DEALLOCATE(edges_triangles);

	/* 3. For each edge compute optimal contraction target v and
		therefore the cost of contracting that pair */
	cost_list = CREATE(LIST(Decimation_cost))();
	while (cost = FIRST_OBJECT_IN_LIST_THAT(Decimation_cost)(
		(LIST_CONDITIONAL_FUNCTION(Decimation_cost) *)NULL, (void *)NULL,
		pre_cost_list))
	{
		ACCESS(Decimation_cost)(cost);

		REMOVE_OBJECT_FROM_LIST(Decimation_cost)(cost, pre_cost_list);

		if (Decimation_cost_calculate_cost(cost, (void *)NULL))
		{
			/* Add this cost into the list of each vertex */
			ADD_OBJECT_TO_LIST(Decimation_cost)(cost, 
				cost->quadric1->dependent_cost_list);
			ADD_OBJECT_TO_LIST(Decimation_cost)(cost, 
				cost->quadric2->dependent_cost_list);
			
			ADD_OBJECT_TO_LIST(Decimation_cost)(cost, cost_list);
		}
		DEACCESS(Decimation_cost)(&cost);
	}
	DESTROY(LIST(Decimation_cost))(&pre_cost_list);

	/* 4. Remove the minimum cost pair and collapse the edge */
	triangles_removed = 0;
	while ((cost = FIRST_OBJECT_IN_LIST_THAT(Decimation_cost)(
		(LIST_CONDITIONAL_FUNCTION(Decimation_cost) *)NULL, (void *)NULL,
		cost_list)) && (cost->cost < threshold_distance))
	{
		quadric = cost->quadric1;
		vertex = quadric->vertex;
		quadric2 = cost->quadric2;
		vertex2 = quadric2->vertex;

		if (check_triangles_are_not_inverted(vertex, vertex2, cost->coordinates) &&
			 check_triangles_are_not_inverted(vertex2, vertex, cost->coordinates))
		{
			/* Move vertex 1 to the optimal contraction */
			for (i = 0 ; i < 3 ; i++)
			{
				quadric->vertex->coordinates[i] = cost->coordinates[i];
			}
			/* We approximate the new quadric by the sum of the old ones, the
				same quadric we used for the edge */
			for (i = 0 ; i < 10 ; i++)
			{
				quadric->matrix[i] += quadric2->matrix[i];
			}
			/* Replace all references to quadric2 with quadric1, or if quadric1
				is already in the triangle then this triangle has collapsed so
				it can be removed */
			number_of_triangles = vertex2->number_of_triangles;
			for (i = 0 ; i < number_of_triangles ; i++)
			{
				remove_triangle = 0;
				for (j = 0 ; j < 3 ; j++)
				{
					if (vertex2->triangles[i]->vertices[j] == vertex)
					{
						remove_triangle = 1;
					}
				}
				if (remove_triangle)
				{
					/* We can defer all the rest of the update to the clean
						up at the bottom because we are using the Decimation_quadric
						and Decimation_cost structures to store our mesh for this purpose */
					vertex2->triangles[i]->index = -1;
				}
				else
				{
					add_triangle = 0;
					for (j = 0 ; j < 3 ; j++)
					{
						if (vertex2->triangles[i]->vertices[j] == vertex2)
						{
							vertex2->triangles[i]->vertices[j] = vertex;
							add_triangle++;
						}
					}
#if defined (DEBUG)
					if (add_triangle != 1)
					{
						display_message(ERROR_MESSAGE,"decimate_triangles.  "
							"Repeated vertex or valid vertex not found in triangle.");					
					}
#endif /* defined (DEBUG) */
					/* Add this into the triangle pointers of the first vertex */
					REALLOCATE(vertex->triangles, vertex->triangles,
						struct VT_iso_triangle *, vertex->number_of_triangles + 1);
					vertex->triangles[vertex->number_of_triangles] = 
						vertex2->triangles[i];
					vertex->number_of_triangles++;	
				}
			}

			/* Now we must find each edge cost that involves either vertex and
				update its cost. */
			REMOVE_OBJECT_FROM_LIST(Decimation_cost)(cost, quadric->dependent_cost_list);
			REMOVE_OBJECT_FROM_LIST(Decimation_cost)(cost, quadric2->dependent_cost_list);

			/* Shuffle all the costs into the second list */
			while (update_cost = FIRST_OBJECT_IN_LIST_THAT(Decimation_cost)(
						 (LIST_CONDITIONAL_FUNCTION(Decimation_cost) *)NULL, (void *)NULL,
						 quadric->dependent_cost_list))
			{
				ADD_OBJECT_TO_LIST(Decimation_cost)(update_cost, quadric2->dependent_cost_list);
				REMOVE_OBJECT_FROM_LIST(Decimation_cost)(update_cost, quadric->dependent_cost_list);
			}
			/* Now for each cost in the second list we remove it from the main list,
				recalculate it's cost and then add it back in to the second list */
			while (update_cost = FIRST_OBJECT_IN_LIST_THAT(Decimation_cost)(
						 (LIST_CONDITIONAL_FUNCTION(Decimation_cost) *)NULL, (void *)NULL,
						 quadric2->dependent_cost_list))
			{
				if (update_cost->quadric1 == cost->quadric1)
				{
					/* Other vertex should be independent vertex */
					independent_quadric = update_cost->quadric2;
				}
				else if (update_cost->quadric1 == cost->quadric2)
				{
					/* Change the quadric1 in the update cost to the new
						vertex and the other vertex is the independent one */
					REACCESS(Decimation_quadric)(&update_cost->quadric1, cost->quadric1);
					independent_quadric = update_cost->quadric2;
				}
				else if (update_cost->quadric2 == cost->quadric1)
				{
					/* Other vertex should be independent vertex */
					independent_quadric = update_cost->quadric1;
				}
				else if (update_cost->quadric2 == cost->quadric2)
				{
					/* Change the quadric1 in the update cost to the new
						vertex and the other vertex is the independent one */
					REACCESS(Decimation_quadric)(&update_cost->quadric2, cost->quadric1);
					independent_quadric = update_cost->quadric1;
				}
#if defined (DEBUG)
				else
				{
					display_message(ERROR_MESSAGE,"decimate_triangles.  "
						"Unable to find expected vertex in dependent_cost_list.");
				}
#endif /* defined (DEBUG) */

				ACCESS(Decimation_cost)(update_cost);
				REMOVE_OBJECT_FROM_LIST(Decimation_cost)(update_cost, cost_list);
				REMOVE_OBJECT_FROM_LIST(Decimation_cost)(update_cost, quadric2->dependent_cost_list);
				REMOVE_OBJECT_FROM_LIST(Decimation_cost)(update_cost,
					independent_quadric->dependent_cost_list);

#if defined (DEBUG)
				if (update_cost->access_count > 1)
				{
					display_message(ERROR_MESSAGE,"decimate_triangles.  "
						"Update cost is still acessed by something.");
				}
#endif /* defined (DEBUG) */

				if (!(FIRST_OBJECT_IN_LIST_THAT(Decimation_cost)(Decimation_cost_has_quadric,
							(void *)independent_quadric, quadric->dependent_cost_list)))
				{
					/* Reset the invalid counter */
					update_cost->invalid_cost_counter = 0;
					/* Here we can change the cost as no lists are referencing it,
						if this is not a duplicate, update it and put it back in the lists */
					Decimation_cost_calculate_cost(update_cost, (void *)NULL);
				
					ADD_OBJECT_TO_LIST(Decimation_cost)(update_cost, cost_list);
				
					ADD_OBJECT_TO_LIST(Decimation_cost)(update_cost, quadric->dependent_cost_list);
					ADD_OBJECT_TO_LIST(Decimation_cost)(update_cost,
						independent_quadric->dependent_cost_list);
				}

				DEACCESS(Decimation_cost)(&update_cost);
			}

			vertex2->index = -1;
			triangles_removed += 2;
			
			REMOVE_OBJECT_FROM_LIST(Decimation_cost)(cost, cost_list);
		}
		else
		{
			cost->invalid_cost_counter++;

			ACCESS(Decimation_cost)(cost);
			REMOVE_OBJECT_FROM_LIST(Decimation_cost)(cost, cost_list);
			REMOVE_OBJECT_FROM_LIST(Decimation_cost)(cost, quadric->dependent_cost_list);
			REMOVE_OBJECT_FROM_LIST(Decimation_cost)(cost, quadric2->dependent_cost_list);
			
			if (Decimation_cost_calculate_cost(cost, (void *)NULL))
			{
				ADD_OBJECT_TO_LIST(Decimation_cost)(cost, cost_list);
				ADD_OBJECT_TO_LIST(Decimation_cost)(cost, quadric->dependent_cost_list);
				ADD_OBJECT_TO_LIST(Decimation_cost)(cost, quadric2->dependent_cost_list);
			}
			DEACCESS(Decimation_cost)(&cost);
		}
	}

	/* Need to undo the circular references before destroying the list to enable
		everything to clean up */
	while (cost = FIRST_OBJECT_IN_LIST_THAT(Decimation_cost)(
		(LIST_CONDITIONAL_FUNCTION(Decimation_cost) *)NULL, (void *)NULL,
		cost_list))
	{
		REMOVE_OBJECT_FROM_LIST(Decimation_cost)(cost, cost->quadric1->dependent_cost_list);
		REMOVE_OBJECT_FROM_LIST(Decimation_cost)(cost, cost->quadric2->dependent_cost_list);
		REMOVE_OBJECT_FROM_LIST(Decimation_cost)(cost, cost_list);
	}
	DESTROY(LIST(Decimation_cost))(&cost_list);
	DESTROY(LIST(Decimation_quadric))(&quadric_list);

	/* Recreate the vertex_list and compiled_triangle_list */
	vertex_list_ptr = voltex->vertex_list;
	new_vertex_list_ptr = voltex->vertex_list;
	vertex_index = 0;
	compacting_vertices = 0;
	for (i = 0 ; i < voltex->number_of_vertices ; i++)
	{
		if (-1 != (*vertex_list_ptr)->index)
		{
			if (compacting_vertices)
			{
				*new_vertex_list_ptr = *vertex_list_ptr;
				(*vertex_list_ptr)->index = vertex_index;
			}

			triangle_list_ptr = (*vertex_list_ptr)->triangles;
			new_triangle_list_ptr = (*vertex_list_ptr)->triangles;
			triangle_index = 0;
			compacting_triangles = 0;
			for (j = 0 ; j < (*vertex_list_ptr)->number_of_triangles ; j++)
			{
				if (-1 != (*triangle_list_ptr)->index)
				{
					if (compacting_triangles)
					{
						*new_triangle_list_ptr = *triangle_list_ptr;
					}
					triangle_index++;
					new_triangle_list_ptr++;
				}
				else
				{
					compacting_triangles = 1;
				}
				triangle_list_ptr++;
			}
			(*vertex_list_ptr)->number_of_triangles = triangle_index;

			vertex_index++;
			new_vertex_list_ptr++;
		}
		else
		{
			/* Delete vertex */
			DESTROY(VT_iso_vertex)(vertex_list_ptr);
			compacting_vertices = 1;
		}
		vertex_list_ptr++;
	}
	voltex->number_of_vertices = vertex_index;
	REALLOCATE(voltex->vertex_list, voltex->vertex_list,
		struct VT_iso_vertex *, voltex->number_of_vertices);

	triangle_list_ptr = voltex->triangle_list;
	new_triangle_list_ptr = voltex->triangle_list;
	triangle_index = 0;
	compacting_triangles = 0;
	for (i = 0 ; i < voltex->number_of_triangles ; i++)
	{
		if (-1 != (*triangle_list_ptr)->index)
		{
			if (compacting_triangles)
			{
				*new_triangle_list_ptr = *triangle_list_ptr;
				(*triangle_list_ptr)->index = triangle_index;
			}
			triangle_index++;
			new_triangle_list_ptr++;
		}
		else
		{
			compacting_triangles = 1;

			/* Free memory */
			DEALLOCATE (*triangle_list_ptr);
		}
		triangle_list_ptr++;
	}
	voltex->number_of_triangles = triangle_index;
	REALLOCATE(voltex->triangle_list, voltex->triangle_list,
		struct VT_iso_triangle *, voltex->number_of_triangles);

	LEAVE;
	return(return_code);

} /* decimate_triangles */
