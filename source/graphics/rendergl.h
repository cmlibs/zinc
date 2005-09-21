/*******************************************************************************
FILE : rendergl.h

LAST MODIFIED : 28 November 2003

DESCRIPTION :
Header file for rendergl.c, GL rendering calls (API specific)
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
#if !defined (RENDERGL_H)
#define RENDERGL_H

#include "graphics/graphics_object.h"
#include "general/multi_range.h"

/*
Global functions
----------------
*/

int draw_glyphsetGL(int number_of_points,Triple *point_list, Triple *axis1_list,
	Triple *axis2_list, Triple *axis3_list, Triple *scale_list,
	struct GT_object *glyph, char **labels,
	int number_of_data_components, GTDATA *data, int *names,
	int label_bounds_dimension, int label_bounds_components, float *label_bounds,
	struct Graphical_material *material, struct Spectrum *spectrum,
	int draw_selected, int some_selected,
	struct Multi_range *selected_name_ranges);
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Draws graphics object <glyph> at <number_of_points> points given by the
positions in <point_list> and oriented and scaled by <axis1_list>, <axis2_list>
and <axis3_list>, each axis additionally scaled by its value in <scale_list>.
If the glyph is part of a linked list through its nextobject
member, these attached glyphs are also executed.
Writes the <labels> array strings, if supplied, beside each glyph point.
If <names> are supplied these identify each point/glyph for OpenGL picking.
If <draw_selected> is set, then only those <names> in <selected_name_ranges>
are drawn, otherwise only those names not there are drawn.
If <some_selected> is true, <selected_name_ranges> indicates the points that
are selected, or all points if <selected_name_ranges> is NULL.
==============================================================================*/

int draw_pointsetGL(int n_pts,Triple *point_list,char **text,
	gtMarkerType marker_type,float marker_size,int *names,
	int number_of_data_components, GTDATA *data,
	struct Graphical_material *material, struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
???RC.  21/12/97 The optional names are supplied to allow identification of the
object with OpenGL picking. (used to use a accessed pointer to an FE_node which
did not allow other objects to be identified, nor nodes to be destroyed). Since
can't change picking names between glBegin and glEnd, must have a separate
begin/end bracket for each point. This may reduce rendering performance a
little, but on the upside, spectrum handling and text output are done
simultaneously.
==============================================================================*/

int draw_polylineGL(Triple *point_list, Triple *normal_list, int n_pts,
	int number_of_data_components, GTDATA *data,
	struct Graphical_material *material,struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 1 June 1999

DESCRIPTION :
Put all the different drawpolylineGL routines together.
==============================================================================*/

int draw_dc_polylineGL(Triple *point_list,Triple *normal_list, int n_pts, 
	int number_of_data_components, GTDATA *data,
	struct Graphical_material *material,struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
==============================================================================*/

int draw_surfaceGL(Triple *surfpts, Triple *normalpoints, Triple *tangentpoints, 
	Triple *texturepoints, int npts1, int npts2, gtPolygonType polygon_type,
	int number_of_data_components, GTDATA *data, 
	struct Graphical_material *material, struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 28 November 2003

DESCRIPTION :
==============================================================================*/

int draw_dc_surfaceGL(Triple *surfpts, Triple *normal_points, 
	Triple *texture_points, int npolys,int npp,gtPolygonType polygon_type,int strip,
	int number_of_data_components, GTDATA *data,
	struct Graphical_material *material,struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
If the <polygon_type> is g_TRIANGLE or g_QUADRILATERAL, the discontinuous
sets of vertices are drawn as separate triangles or quadrilaterals - or as
strips of the respective types if the <strip> flag is set. Otherwise a single
polygon is drawn for each of the <npolys>.
==============================================================================*/

int draw_nurbsGL(struct GT_nurbs *nurbptr);
/*******************************************************************************
LAST MODIFIED : 21 July 1997

DESCRIPTION :
==============================================================================*/

int draw_voltexGL(int n_iso_polys,int *triangle_list,
	struct VT_iso_vertex *vertex_list,int n_vertices,int n_rep,
	struct Graphical_material **per_vertex_materials,
	int *iso_poly_material_index,
	struct Environment_map **per_vertex_environment_maps,
	int *iso_poly_environment_map_index,
	float *texturemap_coord,int *texturemap_index,int number_of_data_components,
	GTDATA *data, struct Graphical_material *default_material,
	struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 8 August 2002

DESCRIPTION :
Numbers in <iso_poly_material_index> are indices into the materials in the
<per_vertex_materials>. A zero value denotes use of the default material;
and index of 1 means the first material in the per_vertex_materials. Not
supplying the <iso_poly_material_index> gives the default material to all
vertices.
Use of <iso_poly_environment_map_index> and <per_vertex_environment_maps> is
exactly the same as for materials. Note environment map materials are used in
preference to normal materials.
==============================================================================*/

#endif /* !defined (RENDERGL_H) */
