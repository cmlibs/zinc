/*******************************************************************************
FILE : rendergl.h

LAST MODIFIED : 16 November 2000

DESCRIPTION :
Header file for rendergl.c, GL rendering calls (API specific)
==============================================================================*/
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

int draw_surfaceGL(Triple *surfpts, Triple *normalpoints,
	Triple *texturepoints, int npts1, int npts2, gtPolygonType polygon_type,
	int number_of_data_components, GTDATA *data, 
	struct Graphical_material *material, struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 1 June 1999

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
	struct Graphical_material **iso_poly_material, struct Environment_map **iso_env_map,
	float *texturemap_coord,int *texturemap_index,int number_of_data_components,
	GTDATA *data,struct Graphical_material *material,struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
==============================================================================*/
#endif /* !defined (RENDERGL_H) */
