/*******************************************************************************
FILE : mcubes.h

LAST MODIFIED : 11 April 2005

DESCRIPTION :
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
#if !defined (MCUBES_H)
#define MCUBES_H

#include "graphics/volume_texture.h"

/*
Global constants
----------------
*/
/* max # of polys to be put out from clip for one cell */
#define MAX_INTERSECTION 64  /* 36 */

/*
Global types
------------
*/
struct Triangle
{
	/* vertex */
	double v[3][3];
	/* normal */
	double n[3];
	/* clipping plane eqn parameter */
	double d;
	/* material at vertex */
	int mat[3];
	int clip_history;
	int clip_history2;
	/* store trilinear fn at vertices for deciding non_intersecting clips */
	double trilinear_int[3];
}; /* struct Triangle */

/* structure to represent certain cell values for use in interpolating */
struct Cell_fn
{
	double *xc;
	double *yc;
	double *zc;
	double *fn;
	double isovalue;
	/* the direction of the isosurface 1 out, -1 in */
	double dir;
}; /* struct Cell_fn */

/*
Global functions
----------------
*/

int normalized_cross_product(float vector_1[3],float vector_2[3],
	float result[3]);
/*******************************************************************************
LAST MODIFIED : 18 February 1998

DESCRIPTION :
Calculates the normalized cross product of <vector_1> and <vector_2> and puts
it in <result>.
==============================================================================*/

int cross_product(double vector_1[3],double vector_2[3],
	double result[3]);
/*******************************************************************************
LAST MODIFIED : 18 February 1998

DESCRIPTION :
Calculates the cross product of <vector_1> and <vector_2> and puts
it in <result>.
==============================================================================*/

#if defined (OLD_CODE)
int load_mc_tables(void);
/*******************************************************************************
LAST MODIFIED : 25 November 1994

DESCRIPTION :
Loads the pntr & data tables
==============================================================================*/
#endif /* defined (OLD_CODE) */

int marching_cubes(struct VT_scalar_field **scalar_field,int n_scalar_fields,
	struct VT_vector_field *coordinate_field,
	struct MC_iso_surface *mc_iso_surface,
	double *isovalue,int closed_surface,int cutting_plane_on,
	double decimation_tolerance);
/*******************************************************************************
LAST MODIFIED : 11 April 2005

DESCRIPTION :
The modified marching cubes algorithm for constructing isosurfaces.  This
constructs an isosurface for specified value from a filtered volume texture.
The nodal values are obtained by a weighting of the surrounding cells. These are
used to create an isosurface, and a polygon list is created.  Normals are
calculated using central or one sided differences at each node.  If
closed_surface, the iso surface generated contains closed surfaces along
intersections with the boundary.
==============================================================================*/

int update_scalars(struct VT_volume_texture *t,double *cutting_plane);
/*******************************************************************************
LAST MODIFIED : 5 October 1996

DESCRIPTION :
Calculates scalar values at nodes from the average of the surrounding cell
values. Also calculates the gradient (using central differences) of the scalar
field. These values are then updated or stored as part of the volume_texture
structure.

???MS.  I think i should put in a provision to edit node values - ie so this
routine cant touch the set nodes.
==============================================================================*/

void print_texture(struct VT_volume_texture *texture);
/*******************************************************************************
LAST MODIFIED : 18 February 1998

DESCRIPTION :
Prints volume_texure scalar info into a file
==============================================================================*/

void destroy_mc_triangle_list(struct MC_cell *mc_cell,
	struct MC_triangle ***triangle_list,int *n_triangles,int n_scalar_fields, 
	struct MC_iso_surface *mc_iso_surface);
/*******************************************************************************
LAST MODIFIED : 24 February 1997

DESCRIPTION :
Destroy triangle list for a mc_cell, which includes (n_scalar_fields + 6)
independent triangle lists
==============================================================================*/

int calculate_mc_material(struct VT_volume_texture *texture,
	struct MC_iso_surface *mc_iso_surface);
/*******************************************************************************
LAST MODIFIED : 18 February 1998

DESCRIPTION :
Calculates the material/colour field to accompany the isosurface (ie allocates
colours to the polygons)
==============================================================================*/

void cube_map_function(int *index,float *texturemap_coord,
	struct VT_iso_vertex *vertex,double cop[3],double ximin[3],double ximax[3]);
/*******************************************************************************
LAST MODIFIED : 7 January 1998

DESCRIPTION :
==============================================================================*/

void face_cube_map_function(int *index,float texturemap_coord[3],
	struct VT_iso_vertex *vertex,double cop[3],int face_index,
	double ximin[3],double ximax[3]);
/*******************************************************************************
LAST MODIFIED : 7 January 1998

DESCRIPTION :
==============================================================================*/

void mc_cube_map_function(int *index,float *texturemap_coord,
	struct MC_vertex *vertex,double cop[3],double ximin[3],double ximax[3]);
/*******************************************************************************
LAST MODIFIED : 7 January 1998

DESCRIPTION :
==============================================================================*/

void mc_face_cube_map_function(int *index,float texturemap_coord[3],
	struct MC_vertex *vertex,double cop[3],int face_index,double ximin[3],
	double ximax[3]);
/*******************************************************************************
LAST MODIFIED : 7 January 1998

DESCRIPTION :
==============================================================================*/

void general_cube_map_function(int *index,float *texturemap_coord,
	float *vertex_coords,double cop[3],double ximin[3],double ximax[3]);
/*******************************************************************************
LAST MODIFIED : 7 January 1998

DESCRIPTION :
Calculates u,v texture map values for a vertex. Third texture coordinate set to
zero (in anticipation of SGI 3d texture mapping potential combination).

Calculates intersection from a centre of projection through a vertex onto the
surface of a cube - returns u,v coords on surface, and index (1-6) of
intersected surface.
==============================================================================*/

void general_face_cube_map_function(int *index,float texturemap_coord[3],
	float *vertex_coords,double cop[3],int face_index,double ximin[3],
	double ximax[3]);
/*******************************************************************************
LAST MODIFIED : 7 January 1998

DESCRIPTION :
Calculates u,v texture map values for a vertex on a face.  This varies from the
general case in that face values are fixed as the projection is made onto them.
==============================================================================*/

void clean_mc_iso_surface(int n_scalar_fields, 
    struct MC_iso_surface *mc_iso_surface);
/*******************************************************************************
LAST MODIFIED : 24 February 1997

DESCRIPTION : 
==============================================================================*/

int calculate_detail_map(struct VT_volume_texture *texture,
	struct MC_iso_surface *mc_iso_surface);
/*******************************************************************************
LAST MODIFIED : 17 July 1997

DESCRIPTION :
Fills mcubes detail map 0: leave same 1+: increase trianglular resolution 
==============================================================================*/
#endif /* !defined (MCUBES_H) */
