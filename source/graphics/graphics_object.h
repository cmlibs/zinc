/*******************************************************************************
FILE : graphics_object.h

LAST MODIFIED : 8 August 2002

DESCRIPTION :
Graphical object data structures.

HISTORY :
Used to be gtypes.h
7 June 1994
	Merged GTTEXT into GTPOINT and GTPOINTSET and added a marker type and a marker
	size.
4 February 1996
	Added time dependence for gtObject.
16 February 1996
	Added graphical finite element structure (code yet to be done).
24 February 1996
	Separated out user defined objects and put in userdef_object.h
4 June 1996
	Replaced gtObjectListItem by struct LIST(gtObject)
5 June 1996
	Changed gtObject to GT_object
11 January 1997
	Added pointers to the nodes in a GTPOINTSET.  This is a temporary measure to
	allow the graphical_node_editor to work (will be replaced by the graphical
	FE_node)
30 June 1997
	Added macros/functions for safer access to graphics objects. Should convert
	rest of code to use them, so that members of graphics_objects are private.
13 October 1998
	SAB Added a callback mechanism so that through GT_object_changed interested
	objects can automatically be notified of changes (i.e. the Scene_object and
	consequently the Scene).
==============================================================================*/
#if !defined (GRAPHICS_OBJECT_H)
#define GRAPHICS_OBJECT_H

#include "general/geometry.h"
#include "general/list.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/graphics_library.h"
#include "graphics/material.h"
#include "graphics/selected_graphic.h"
#include "graphics/spectrum.h"
#include "graphics/volume_texture.h"

/*
Global types
------------
*/
typedef enum
/*******************************************************************************
LAST MODIFIED : 16 February 1996

DESCRIPTION :
==============================================================================*/
{
	g_SIMPLE,
	g_LINKED
} gtObjectLinkType;

enum GT_object_type
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Must ensure all the types defined here are handled by function
get_GT_object_type_string.
Have members BEFORE_FIRST and AFTER_LAST to enable iterating through the list
without knowing which order the types are in.
==============================================================================*/
{
	g_OBJECT_TYPE_INVALID,
	g_OBJECT_TYPE_BEFORE_FIRST,
	g_GLYPH_SET,
	g_NURBS,
	g_POINT,
	g_POINTSET,
	g_POLYLINE,
	g_SURFACE,
	g_USERDEF,
	g_VOLTEX,
	g_OBJECT_TYPE_AFTER_LAST
};

enum GT_visibility_type
/*******************************************************************************
LAST MODIFIED : 22 September 1997

DESCRIPTION :
Flag for visibility of GT_objects when drawn on windows.
==============================================================================*/
{
	g_INVISIBLE,
	g_VISIBLE
}; /* enum GT_visibility_type */

typedef enum
/*******************************************************************************
LAST MODIFIED : 26 June 1998

DESCRIPTION :
==============================================================================*/
{
	g_GENERAL_POLYGON,
	g_QUADRILATERAL,
	g_TRIANGLE
} gtPolygonType;

enum GT_surface_type
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Must ensure all the types defined here are handled by functions:
  get_GT_surface_type_string      (in: graphics_object.c)
  get_GT_surface_type_from_string (in: graphics_object.c)
  makegtobj                       (in: makegtobj.c)
  file_read_graphics_objects      (in: import_graphics_object.c)
Have members BEFORE_FIRST and AFTER_LAST to enable iterating through the list
without knowing which order the types are in.
==============================================================================*/
{
	g_SURFACE_TYPE_INVALID,
	g_SURFACE_TYPE_BEFORE_FIRST,
	g_SHADED, /* old 0 */
	g_SH_DISCONTINUOUS, /* old 3 */
	g_SHADED_TEXMAP, /* old 6 */
	g_SH_DISCONTINUOUS_TEXMAP, /* old 7 */
	g_SH_DISCONTINUOUS_STRIP,
	g_SH_DISCONTINUOUS_STRIP_TEXMAP,
	g_WIREFRAME_SHADED_TEXMAP,
	g_SURFACE_TYPE_AFTER_LAST
};

enum GT_voltex_type
/*******************************************************************************
LAST MODIFIED : 3 May 2000

DESCRIPTION :
==============================================================================*/
{
	g_VOLTEX_TYPE_INVALID,
	g_VOLTEX_TYPE_BEFORE_FIRST,
	g_VOLTEX_SHADED_TEXMAP,
	g_VOLTEX_WIREFRAME_SHADED_TEXMAP,
	g_VOLTEX_TYPE_AFTER_LAST
};

enum GT_polyline_type
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Must ensure all the types defined here are handled by functions:
  get_GT_polyline_type_string      (in: graphics_object.c)
  get_GT_polyline_type_from_string (in: graphics_object.c)
  makegtobj                        (in: makegtobj.c)
  file_read_graphics_objects       (in: import_graphics_object.c)
Have members BEFORE_FIRST and AFTER_LAST to enable iterating through the list
without knowing which order the types are in.
==============================================================================*/
{
	g_POLYLINE_TYPE_INVALID,
	g_POLYLINE_TYPE_BEFORE_FIRST,
	g_PLAIN, /* old 0 */
	g_NORMAL, /* old 1 */
	g_PLAIN_DISCONTINUOUS, /* old 2 */
	g_NORMAL_DISCONTINUOUS, /* old 3 */
	g_POLYLINE_TYPE_AFTER_LAST
}; /* enum GT_polyline_type */

typedef enum
/*******************************************************************************
LAST MODIFIED : 16 February 1996

DESCRIPTION :
==============================================================================*/
{
	g_RGBCOLOUR,
	g_MATERIAL
} gtAttributeType;

typedef enum
/*******************************************************************************
LAST MODIFIED : 16 February 1996

DESCRIPTION :
==============================================================================*/
{
	g_NOT_CREATED,
	g_CREATED
} gtStatusType;

typedef enum
/*******************************************************************************
LAST MODIFIED : 6 July 1998

DESCRIPTION :
If the gtTransformType of a graphics object is g_NOT_ID it will use its
transformation matrix, otherwise no matrix manipulations are output with it.
Note that if the gtInheritanceType of the graphics object is g_CHILD, the
transformation step is bypassed anyway.
==============================================================================*/
{
	g_ID,
	g_NOT_ID
} gtTransformType;

typedef enum
/*******************************************************************************
LAST MODIFIED : 16 February 1996

DESCRIPTION :
==============================================================================*/
{
	g_PARENT,
	g_CHILD
} gtInheritanceType;

typedef enum
/*******************************************************************************
LAST MODIFIED : 16 February 1996

DESCRIPTION :
==============================================================================*/
{
	g_NO_DATA,
	g_SCALAR,
	g_TWO_COMPONENTS,
	g_VECTOR,
	g_VECTOR4
} gtDataType;

typedef float GTDATA;

typedef enum
/*******************************************************************************
LAST MODIFIED : 23 February 1998

DESCRIPTION :
==============================================================================*/
{
	g_NO_MARKER,
	g_POINT_MARKER,
	g_PLUS_MARKER,
	g_DERIVATIVE_MARKER
} gtMarkerType;

struct GT_glyph_set
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
Graphics primitive for positioning and scaling a glyph - a graphics object with
no materials that merely defines geometry. Glyphs should be designed to be
centred at 0,0,0 and oriented with the 3 primary axes 1, 2 and 3. The display
list of the glyph is called after setting up a model transformation matrix that
shifts the centre and axes 1, 2 and 3 of the glyph to vectors specified in the
<point_list>, <axis1_list>, <axis2_list> and <axis3_list>, respectively. Both
scaling and rotation - not necessarily maintaining orthogonality - are supplied
by the axis vectors. An additional scaling is supplied by the <size> member
which has one component per axis. Certain glyphs, eg. arrows and cones, are able
to take advantage of negative sizes to point inwards in a given axis, eg. to
show compressive strains as -><-, and tensile strains as <-->. Data may also be
stored with this object so that the entire glyph may be coloured by a spectrum.
Hence, glyphs should not have any data themselves. An optional label may also be
drawn beside each glyph. The optional <names> array allows individual items in
the glyph_set to be identified in picking for node position/vector editing.
==============================================================================*/
{
	int number_of_points;
	Triple *axis1_list, *axis2_list, *axis3_list, *point_list, *scale_list;
	char **labels;
	struct GT_object *glyph;
	int n_data_components;
	GTDATA *data;
	/* store integer object_name eg. element number from which this object came */
	int object_name;
	/* have auxiliary_object_name for marking glyph_set for editing purposes; this
		 is necessary since for some element_point types we put the
		 top_level_element in the object_name; the graphics name of the actual
		 face/line/element in use should go here: */
	int auxiliary_object_name;
	/* names recorded per point in the set, eg. node or grid point numbers */
	int *names;
	struct GT_glyph_set *ptrnext;
}; /* struct GT_glyph_set */

struct GT_nurbs
/*******************************************************************************
LAST MODIFIED : 25 February 2000

DESCRIPTION :
==============================================================================*/
{
	int nurbsprop;

	/* surface description */
	int sorder;
	int torder;
	int sknotcnt;
	int tknotcnt;
	int maxs;
	int maxt;
	double *sknots;
	double *tknots;
	double *controlpts;
	/* normals */
	double *normal_control_points;
	/* texture coordinates */
	double *texture_control_points;

	/* nurb trim curve */
	int corder;
	int cknotcnt;
	int ccount;
	double *cknots;
	double *trimarray;

	/* piecewise linear trim curve */
	int pwlcnt;
	double *pwlarray;

	/* store integer object_name eg. element number from which this object came */
	int object_name;

	struct GT_nurbs *ptrnext;
}; /* struct GT_nurbs */

struct GT_point
/*******************************************************************************
LAST MODIFIED : 16 February 1996

DESCRIPTION :
==============================================================================*/
{
	Triple *position;
	char *text;
	gtMarkerType marker_type;
	float marker_size;
	int n_data_components;
	GTDATA *data;
	struct GT_point *ptrnext;
}; /* struct GT_point */

struct GT_pointset
/*******************************************************************************
LAST MODIFIED : 16 April 1999

DESCRIPTION :
???RC.  integer names added for OpenGL picking.
==============================================================================*/
{
	int n_pts;
	Triple *pointlist;
	char **text;
	gtMarkerType marker_type;
	float marker_size;
	int n_data_components;
	GTDATA *data;
	struct GT_pointset *ptrnext;
	/* names are usually node numbers (they were accessed nodes, but this
		 prevented the nodes from being destroyed) */
	int *names;
}; /* struct GT_pointset */

struct GT_polyline
/*******************************************************************************
LAST MODIFIED : 16 April 1999

DESCRIPTION :
==============================================================================*/
{
	enum GT_polyline_type polyline_type;
	int n_pts;
	int n_data_components;
	Triple *pointlist;
	Triple *normallist;
	GTDATA *data;
	/* store integer object_name eg. element number from which this object came */
	int object_name;
	struct GT_polyline *ptrnext;
}; /* struct GT_polyline */

struct GT_surface
/*******************************************************************************
LAST MODIFIED : 16 April 1999

DESCRIPTION :
==============================================================================*/
{
	enum GT_surface_type surface_type;
	gtPolygonType polygon;
	int n_data_components;
	int n_pts1;
	int n_pts2;
	Triple *pointlist;
	Triple *normallist;
	Triple *texturelist;
	GTDATA *data;
	/* store integer object_name eg. element number from which this object came */
	int object_name;
	struct GT_surface *ptrnext;
}; /* struct GT_surface */

struct GT_userdef
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
User defined graphics object primitive type. Contains three parameters:
- void pointer to data for user defined graphics;
- pointer to function used to destroy user data when the GT_userdef is
	destroyed.
- pointer to function for rendering the user defined graphic passing data as
	a parameter.
???RC May wish to add time to render function.
???RC This structure could/should be made private.
==============================================================================*/
{
	void *data;
	int (*destroy_function)(void **);
	int (*render_function)(void *);
	/* for compatibility with the other GT_objects: */
	struct GT_userdef *ptrnext;
}; /* struct GT_userdef */

struct GT_voltex
/*******************************************************************************
LAST MODIFIED : 8 March 2002

DESCRIPTION :
???MS.  Initial simple structure - only deal with a fixed isosurface
==============================================================================*/
{
	/* # triangles */
	int n_iso_polys;
	/* # unique vertices */
	int n_vertices;
	/* # repetitions of texture */
	int n_rep;
	/* list of indices into vertex table */
	int *triangle_list;
	/* deformed vertex list */
	struct VT_iso_vertex *vertex_list;
	/* non-repeating list of per-vertex materials with access_count */
	int number_of_per_vertex_materials;
	struct Graphical_material **per_vertex_materials;
	/* materials assigned to vertices */
	int *iso_poly_material_index;
	/* non-repeating list of per-vertex environment_maps with access_count */
	int number_of_per_vertex_environment_maps;
	struct Environment_map **per_vertex_environment_maps;
	/* environment_maps assigned to vertices */
	int *iso_poly_environment_map_index;
	/* cop for cells */
	double *iso_poly_cop;
	float *texturemap_coord;
	int *texturemap_index;
	int n_data_components;
	/* This array stores all the data values,
		each vertex has an index pointing into this array */
	GTDATA *data;
	/* store integer object_name eg. element number from which this object came */
	int object_name;
	struct GT_voltex *ptrnext;
	/* voltex type */
	enum GT_voltex_type voltex_type;
}; /* struct GT_voltex */

/* The callback makes a circular reference in this file */
struct GT_object;

typedef int(*Graphics_object_callback)(struct GT_object *graphics_object,
	void *user_data);

struct Graphics_object_callback_data
{
	Graphics_object_callback callback;
	void *callback_user_data;
	struct Graphics_object_callback_data *next;
}; /* struct Graphics_object_callback_data */

typedef struct GT_object
/*******************************************************************************
LAST MODIFIED : 7 July 2000

DESCRIPTION :
Graphical object data structure.
==============================================================================*/
{
	char *name;
	enum GT_object_type object_type;
	/* for linked object */
	struct GT_object *nextobject;
	/* for inheritance */
	struct GT_object *parentobject;
	/* for selected primitives and subobjects */
	enum Graphics_select_mode select_mode;
	struct LIST(Selected_graphic) *selected_graphic_list;
	/* default attributes */
		/*???DB.  Default is a bit of a misnomer.  Here it means the unhighlighted
			colour */
	/* either colour or material */
	gtAttributeType default_att;
	int default_colourindex;
	struct Graphical_material *default_material,*selected_material;
	struct Graphics_object_callback_data *update_callback_list;
	/* spectrum */
	struct Spectrum *spectrum;
	/* the object can vary with time.  It is specified at <number_of_times>
		<times>.  The <times> are in increasing order.  The object varies linearly
		between <times> and is constant before the first and after the last */
	int number_of_times;
	float *times;
	union
	{
		struct GT_glyph_set **gt_glyph_set;
		struct GT_nurbs **gt_nurbs;
		struct GT_point **gt_point;
		struct GT_pointset **gt_pointset;
		struct GT_polyline **gt_polyline;
		struct GT_surface **gt_surface;
		struct GT_userdef **gt_userdef;
		struct GT_voltex **gt_voltex;
	} gu;
#if defined (OPENGL_API)
	GLuint display_list;
#endif /* defined (OPENGL_API) */
	/* enumeration indicates whether the graphics display list is up to date */
	enum Graphics_compile_status compile_status;
	/*???temporary*/
	int glyph_mirror_mode;
	int access_count;
} gtObject;

DECLARE_LIST_TYPES(GT_object);

struct Graphics_object_range_struct
/*******************************************************************************
LAST MODIFIED : 6 August 1997

DESCRIPTION :
Structure for storing range of time in one or several graphics objects.
Set first=1 before calling range routines. Only if first==0 afterwards is the
range valid.
==============================================================================*/
{
	int first;
	Triple maximum,minimum;
}; /* Graphics_object_range_struct */

struct Graphics_object_data_range_struct
/*******************************************************************************
LAST MODIFIED : 29 October 1997

DESCRIPTION :
Structure for storing range of data in one or several graphics objects.
Set first=1 before calling data range routines. Only if first==0 afterwards is
the data range valid.
???RC likely to change when multiple data values stored in graphics_objects
for combining several spectrums.
==============================================================================*/
{
	int first;
	float minimum,maximum;
}; /* Graphics_object_data_range_struct */

struct Graphics_object_time_range_struct
/*******************************************************************************
LAST MODIFIED : 6 August 1997

DESCRIPTION :
Structure for storing range of time in one or several graphics objects.
Set first=1 before calling time range routines. Only if first==0 afterwards is
the time range valid.
==============================================================================*/
{
	int first;
	float minimum,maximum;
}; /* Graphics_object_time_range_struct */

/*
Global variables
----------------
*/
/*???DB.  I'm not sure that this should be here */
extern float global_line_width,global_point_size;

/*
Global functions
----------------
*/

char *get_GT_object_type_string(enum GT_object_type object_type);
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Returns a string describing the object type, suitable for writing to file
(and reinterpreting it later).
==============================================================================*/

int get_GT_object_type_from_string(char *type_string,
	enum GT_object_type *object_type);
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Returns the object type from the string produced by function
get_GT_object_type_string. For compatibility, also supports converting old
enumerator numbers (as text) into the new enumerator values, with a warning.
==============================================================================*/

char *get_GT_polyline_type_string(enum GT_polyline_type polyline_type);
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Returns a string describing the polyline type, suitable for writing to file
(and reinterpreting it later).
==============================================================================*/

int get_GT_polyline_type_from_string(char *type_string,
	enum GT_polyline_type *polyline_type);
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Returns the polyline type from the string produced by function
get_GT_polyline_type_string. For compatibility, also supports converting old
enumerator numbers (as text) into the new enumerator values, with a warning.
==============================================================================*/

char *get_GT_surface_type_string(enum GT_surface_type surface_type);
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Returns a string describing the surface type, suitable for writing to file
(and reinterpreting it later).
==============================================================================*/

int get_GT_surface_type_from_string(char *type_string,
	enum GT_surface_type *surface_type);
/*******************************************************************************
LAST MODIFIED : 13 August 1998

DESCRIPTION :
Returns the surface type from the string produced by function
get_GT_surface_type_string. For compatibility, also supports converting old
enumerator numbers (as text) into the new enumerator values, with a warning.
==============================================================================*/

struct GT_glyph_set *morph_GT_glyph_set(float proportion,
	struct GT_glyph_set *initial,struct GT_glyph_set *final);
/*******************************************************************************
LAST MODIFIED : 7 July 1998

DESCRIPTION :
Creates a new GT_glyph_set which is the interpolation of two GT_glyph_sets.
The two glyph_sets must have the same glyph and data_type.
==============================================================================*/

struct GT_pointset *morph_GT_pointset(float proportion,
	struct GT_pointset *initial,struct GT_pointset *final);
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Creates a new GT_pointset which is the interpolation of two GT_pointsets.
==============================================================================*/

struct GT_polyline *morph_GT_polyline(float proportion,
	struct GT_polyline *initial,struct GT_polyline *final);
/*******************************************************************************
LAST MODIFIED : 6 February 1996

DESCRIPTION :
Creates a new GT_polyline which is the interpolation of two GT_polylines.
==============================================================================*/

struct GT_surface *morph_GT_surface(float proportion,
	struct GT_surface *initial,struct GT_surface *final);
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Creates a new GT_surface which is the interpolation of two GT_surfaces.
==============================================================================*/

gtObject *morph_gtObject(char *name,float proportion,gtObject *initial,
	gtObject *final);
/*******************************************************************************
LAST MODIFIED : 27 December 1995

DESCRIPTION :
Creates a new gtObject which is the interpolation of two gtObjects.
==============================================================================*/

struct GT_object *transform_GT_object(struct GT_object *object,
	float *transformation);
/*******************************************************************************
LAST MODIFIED : 8 July 1999

DESCRIPTION :
Creates a new GT_object which is the transformation of <object>.
Only surfaces are implemented at the moment.
Normals are not updated (wavefront export doesn't use normals anyway).
==============================================================================*/

Triple *surfalloc(int,int,int);
/*******************************************************************************
LAST MODIFIED : 21 March 1993

DESCRIPTION :
==============================================================================*/

Triple *linealloc(int,int);
/*******************************************************************************
LAST MODIFIED : 21 March 1993

DESCRIPTION :
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(GT_object);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(GT_object);

PROTOTYPE_LIST_FUNCTIONS(GT_object);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(GT_object,name,char *);

struct GT_glyph_set *CREATE(GT_glyph_set)(int number_of_points,
	Triple *point_list, Triple *axis1_list, Triple *axis2_list,
	Triple *axis3_list, Triple *scale_list, struct GT_object *glyph,
	char **labels, int n_data_components, GTDATA *data,
	int object_name, int *names);
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Allocates memory and assigns fields for a GT_glyph_set. The glyph set shows
the object <glyph> at the specified <number_of_points> with positions given in
<point_list>, and principal axes in <axis1_list>, <axis2_list> and <axis3_list>.
The magnitude of these axes control scaling of the glyph at each point, while
their orientations - which need not be orthogonal - effect rotations and skew.
There magnitudes also multiplied by the <scale_list> values, 1 value per axis,
which permit certain glyphs to reverse direction with negative values.
The optional <labels> parameter is an array of strings to be written beside each
glyph, while the optional <data> of number <n_data_components> per glyph allows
colouring of the glyphs by a spectrum.
The glyph_set will be marked as coming from the <object_name>, and integer
identifier, while the optional <names> contains an integer identifier per point.
Note: All arrays passed to this routine are owned by the new GT_glyph_set
and are deallocated by its DESTROY function.
==============================================================================*/

int DESTROY(GT_glyph_set)(struct GT_glyph_set **glyph_set_address);
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Frees the frees the memory for <**glyph_set_address> and sets
<*glyph_set_address> to NULL.
==============================================================================*/

struct GT_nurbs *CREATE(GT_nurbs)(void);
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Creates a default GT_nurbs object.
==============================================================================*/

int DESTROY(GT_nurbs)(struct GT_nurbs **nurbs);
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Frees the memory for <**nurbs> and its fields and sets <*nurbs> to NULL.
==============================================================================*/

int GT_nurbs_set_surface(struct GT_nurbs *nurbs,
	int sorder, int torder, int sknotcount, int tknotcount,
	double *sknots, double *tknots, 
	int scontrolcount, int tcontrolcount, double *control_points);
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Sets the surface in a GT_nurbs structure.
There must be sknotcount values in sknots,
tknotcount values in tknots and scontrolcount * tcontrolcount values
in the controlpoints, with the s direction varying more quickly.
The arrays are assigned directly to the object and not copied.
==============================================================================*/

int GT_nurbs_set_nurb_trim_curve(struct GT_nurbs *nurbs,
	int order, int knotcount, double *knots,
	int control_count, double *control_points);
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Sets a Nurb curve used to trim the Nurbs surface in a GT_nurbs structure.
The arrays are assigned directly to the object and not copied.
==============================================================================*/

int GT_nurbs_set_piecewise_linear_trim_curve(struct GT_nurbs *nurbs,
	int number_of_points, double *points);
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Sets a piecewise linear curve used to trim the Nurbs surface in a GT_nurbs structure.
The array is assigned directly to the object and not copied.
==============================================================================*/

int GT_nurbs_set_normal_control_points(struct GT_nurbs *nurbs,
	double *normal_control_points);
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Sets the control point array for the normals.  The array is assigned directly and
the number of points is assumed to be the same as the geometry control
points specified in set surface.  Each normal is assumed to have three components.
==============================================================================*/

int GT_nurbs_set_texture_control_points(struct GT_nurbs *nurbs,
	double *texture_control_points);
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Sets the control point array for the texture.  The array is assigned directly and
the number of points is assumed to be the same as the geometry control
points specified in set surface.  Each point is assumed to have three
texture coordinates.
==============================================================================*/

struct GT_point *CREATE(GT_point)(Triple *position,char *text,
	gtMarkerType marker_type,float marker_size,int n_data_components,GTDATA *data);
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Allocates memory and assigns fields for a GT_point.  When the <marker_type> is
g_DERIVATIVE_MARKER, there should be 4 points in <pointlist> - first point is
for the <node>, next point is the end point for the xi1 derivative axis, etc.
If the end point is the same as the node point it is assumed that there isn't a
derivative in that xi direction.
==============================================================================*/

int DESTROY(GT_point)(struct GT_point **point);
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Frees the frees the memory for <**point> and sets <*point> to NULL.
==============================================================================*/

struct GT_pointset *CREATE(GT_pointset)(int n_pts,Triple *pointlist,char **text,
	gtMarkerType marker_type,float marker_size,int n_data_components,GTDATA *data,
	int *names);
/*******************************************************************************
LAST MODIFIED : 1 June 1999

DESCRIPTION :
Allocates memory and assigns fields for a GT_pointset.  When the <marker_type>
is g_DERIVATIVE_MARKER, there should be 4*<n_pts> points in <pointlist> - in
each group of four points the first for the nodes, the next is the end points
for the xi1 derivative axis, etc.  If the end point is the same as the node
point it is assumed that there isn't a derivative in that xi direction.
==============================================================================*/

int DESTROY(GT_pointset)(struct GT_pointset **pointset);
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Frees the frees the memory for <**pointset> and sets <*pointset> to NULL.
==============================================================================*/

struct GT_polyline *CREATE(GT_polyline)(enum GT_polyline_type polyline_type,
	int n_pts,Triple *pointlist,Triple *normallist,int n_data_components,GTDATA *data);
/*******************************************************************************
LAST MODIFIED : 31 May 1999

DESCRIPTION :
Allocates memory and assigns fields for a graphics polyline.
==============================================================================*/

int DESTROY(GT_polyline)(struct GT_polyline **polyline);
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Frees the memory for <**polyline> and its fields and sets <*polyline> to NULL.
==============================================================================*/

struct GT_surface *CREATE(GT_surface)(enum GT_surface_type surface_type,
	gtPolygonType polytype,int n_pts1,int n_pts2,Triple *pointlist,
	Triple *normallist, Triple *texturelist,
	int n_data_components,GTDATA *data);
/*******************************************************************************
LAST MODIFIED : 31 May 1999

DESCRIPTION :
Allocates memory and assigns fields for a graphics surface.
==============================================================================*/

int DESTROY(GT_surface)(struct GT_surface **surface);
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Frees the memory for <**surface> and sets <*surface> to NULL.
==============================================================================*/

struct GT_userdef *CREATE(GT_userdef)(void *data,
	int (*destroy_function)(void **),int (*render_function)(void *));
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Allocates memory and assigns fields for a user-defined primitive.
Any data required for rendering the primitive should be passed in the
void *data parameter; a destroy_function should be given if dynamically
allocated data is passed. The render function is called with the data as a
parameter to render the user-defined primitive.
==============================================================================*/

int DESTROY(GT_userdef)(struct GT_userdef **userdef);
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Frees the memory for <**userdef> and its fields and sets <*userdef> to NULL.
==============================================================================*/

struct GT_voltex *CREATE(GT_voltex)(int n_iso_polys, int n_vertices,
	int *triangle_list, struct VT_iso_vertex *vertex_list, double *iso_poly_cop,
	float *texturemap_coord, int *texturemap_index,int n_rep,
	int n_data_components, GTDATA *data, enum GT_voltex_type voltex_type);
/*******************************************************************************
LAST MODIFIED : 8 August 2002

DESCRIPTION :
Allocates memory and assigns fields for a graphics volume texture.
==============================================================================*/

int DESTROY(GT_voltex)(struct GT_voltex **voltex);
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Frees the memory for <**voltex> and sets <*voltex> to NULL.
???DB.  Free memory for fields ?
==============================================================================*/

int GT_voltex_set_triangle_vertex_environment_map(struct GT_voltex *voltex,
	int triangle_number, int triangle_vertex_number,
	struct Environment_map *environment_map);
/*******************************************************************************
LAST MODIFIED : 8 August 2002

DESCRIPTION :
Sets the environment map used for vertex <vertex_number> in <voltex> to
<environment_map>. Handles conversion to an indexed look-up into a non-repeating
environment_map array.
==============================================================================*/

int GT_voltex_set_triangle_vertex_material(struct GT_voltex *voltex,
	int triangle_number, int triangle_vertex_number,
	struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 7 August 2002

DESCRIPTION :
Sets the material used for vertex <vertex_number> in <voltex> to <material>.
Handles conversion to an indexed look-up into a non-repeating material array.
==============================================================================*/

struct GT_object *CREATE(GT_object)(char *name,enum GT_object_type object_type,
	struct Graphical_material *default_material);
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Allocates memory and assigns fields for a graphics object.
==============================================================================*/

int DESTROY(GT_object)(struct GT_object **object_ptr);
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Frees the memory for the fields of <**object>, frees the memory for <**object>
and sets <*object> to NULL.
==============================================================================*/

int compile_GT_object(struct GT_object *graphics_object_list,void *time_void);
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Rebuilds the display list for each uncreated or morphing graphics object in the
<graphics_object_list>, a simple linked list. The object is compiled at the time
pointed to by <time_void>.
==============================================================================*/

int execute_GT_object(struct GT_object *graphics_object_list,void *time_void);
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Rebuilds the display list for each uncreated or morphing graphics object in the
<graphics_object_list>, a simple linked list.
If the linked list has more than one graphics_object in it, the number of the
graphics object, starting at zero for the first
==============================================================================*/

int GT_object_changed(struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 12 March 2002

DESCRIPTION :
External modules that change a GT_object should call this routine so that
objects interested in this GT_object will be notified that is has changed.
==============================================================================*/

int GT_object_Graphical_material_change(struct GT_object *graphics_object,
	struct LIST(Graphical_material) *changed_material_list);
/*******************************************************************************
LAST MODIFIED : 12 March 2002

DESCRIPTION :
Tells the <graphics_object> that the materials in the <changed_material_list>
have changed. If any of these materials are used in any graphics object,
changes the compile_status to CHILD_GRAPHICS_NOT_COMPILED and
informs clients of the need to recompile and redraw. Note that if a spectrum is
in use the more expensive GRAPHICS_NOT_COMPILED status is necessarily set.
Note: Passing a NULL <changed_material_list> indicates the equivalent of a
change to any material in use in the linked graphics objects.
==============================================================================*/

int GT_object_Spectrum_change(struct GT_object *graphics_object,
	struct LIST(Spectrum) *changed_spectrum_list);
/*******************************************************************************
LAST MODIFIED : 12 March 2002

DESCRIPTION :
Tells the <graphics_object> that the spectrums in the <changed_spectrum_list>
have changed. If any of these spectrums are used in any graphics object,
changes the compile_status to GRAPHICS_NOT_COMPILED and
informs clients of the need to recompile and redraw.
Note: Passing a NULL <changed_spectrum_list> indicates the equivalent of a
change to any spectrum in use in the linked graphics objects.
==============================================================================*/

int GT_object_add_callback(struct GT_object *graphics_object, 
	Graphics_object_callback callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 13 October 1998

DESCRIPTION :
Adds a callback routine which is called whenever a GT_object is aware of
changes.  As the GT_object is not private, this relies on modules that change a
GT_object calling GT_object_changed.
==============================================================================*/

int GT_object_remove_callback(struct GT_object *graphics_object,
	Graphics_object_callback callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 13 October 1998

DESCRIPTION :
Removes a callback which was added previously
==============================================================================*/

int GT_object_has_time(struct GT_object *graphics_object,float time);
/*******************************************************************************
LAST MODIFIED : 26 June 1997

DESCRIPTION :
Returns 1 if the time parameter is used by the graphics_object.
==============================================================================*/

int GT_object_get_number_of_times(struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 18 June 1998

DESCRIPTION :
Returns the number of times/primitive lists in the graphics_object.
==============================================================================*/

float GT_object_get_time(struct GT_object *graphics_object,int time_no);
/*******************************************************************************
LAST MODIFIED : 18 June 1998

DESCRIPTION :
Returns the time at <time_no> from the graphics_object.
Note that time numbers range from 1 to number_of_times.
==============================================================================*/

float GT_object_get_nearest_time(struct GT_object *graphics_object,float time);
/*******************************************************************************
LAST MODIFIED : 7 August 1997

DESCRIPTION :
Returns the nearest time to <time> in <graphics_object> at which graphics
primitives are called.
NOTE: presently finds the nearest time that is *lower* than <time>. When all
routines updated to use this, may be changed to get actual nearest time.
==============================================================================*/

int get_graphics_object_range(struct GT_object *graphics_object,
	void *graphics_object_range_void);
/*******************************************************************************
LAST MODIFIED : 8 August 1997

DESCRIPTION :
Returns the range of the coordinates in <graphics_object> or 0 if object empty
or error occurs. First should be set to 1 outside routine. Several calls to
this routine for differing graphics objects (without settings first in between)
will produce the range of all the graphics objects.
???RC only does some object types.
==============================================================================*/

int get_graphics_object_data_range(struct GT_object *graphics_object,
	void *graphics_object_data_range_void);
/*******************************************************************************
LAST MODIFIED : 29 October 1997

DESCRIPTION :
Returns the range of the data values stored in the graphics object.
Returned range generally used to set or enlarge spectrum ranges.
???RC likely to change when multiple data values stored in graphics_objects
for combining several spectrums.
==============================================================================*/

int get_graphics_object_time_range(struct GT_object *graphics_object,
	void *graphics_object_time_range_void);
/*******************************************************************************
LAST MODIFIED : 8 August 1997

DESCRIPTION :
Enlarges the minimum and maximum time range by that of the graphics_object.
==============================================================================*/

#if defined (FULL_NAMES)
#define GT_OBJECT_ADD_(primitive_type) GT_object_add_ ## primitive_type
#else
#define GT_OBJECT_ADD_(primitive_type) goa_ ## primitive_type
#endif
#define GT_OBJECT_ADD(primitive_type) GT_OBJECT_ADD_(primitive_type)

#define PROTOTYPE_GT_OBJECT_ADD_FUNCTION(primitive_type) \
int GT_OBJECT_ADD(primitive_type)( \
	struct GT_object *graphics_object, \
	float time,struct primitive_type *primitive) \
/***************************************************************************** \
LAST MODIFIED : 19 June 1998 \
\
DESCRIPTION : \
Adds <primitive> to <graphics_object> at <time>, creating the new time if it \
does not already exist. \
============================================================================*/ \

PROTOTYPE_GT_OBJECT_ADD_FUNCTION(GT_glyph_set);
PROTOTYPE_GT_OBJECT_ADD_FUNCTION(GT_nurbs);
PROTOTYPE_GT_OBJECT_ADD_FUNCTION(GT_point);
PROTOTYPE_GT_OBJECT_ADD_FUNCTION(GT_pointset);
PROTOTYPE_GT_OBJECT_ADD_FUNCTION(GT_polyline);
PROTOTYPE_GT_OBJECT_ADD_FUNCTION(GT_surface);
PROTOTYPE_GT_OBJECT_ADD_FUNCTION(GT_userdef);
PROTOTYPE_GT_OBJECT_ADD_FUNCTION(GT_voltex);

#if defined (FULL_NAMES)
#define GT_OBJECT_GET_(primitive_type) GT_object_get_ ## primitive_type
#else
#define GT_OBJECT_GET_(primitive_type) gog_ ## primitive_type
#endif
#define GT_OBJECT_GET(primitive_type) GT_OBJECT_GET_(primitive_type)

#define PROTOTYPE_GT_OBJECT_GET_FUNCTION(primitive_type) \
struct primitive_type *GT_OBJECT_GET(primitive_type)( \
	struct GT_object *graphics_object,float time) \
/***************************************************************************** \
LAST MODIFIED : 19 June 1997 \
\
DESCRIPTION : \
Returns pointer to the primitive at the given time in graphics_object. \
============================================================================*/

PROTOTYPE_GT_OBJECT_GET_FUNCTION(GT_glyph_set);
PROTOTYPE_GT_OBJECT_GET_FUNCTION(GT_nurbs);
PROTOTYPE_GT_OBJECT_GET_FUNCTION(GT_point);
PROTOTYPE_GT_OBJECT_GET_FUNCTION(GT_pointset);
PROTOTYPE_GT_OBJECT_GET_FUNCTION(GT_polyline);
PROTOTYPE_GT_OBJECT_GET_FUNCTION(GT_surface);
PROTOTYPE_GT_OBJECT_GET_FUNCTION(GT_userdef);
PROTOTYPE_GT_OBJECT_GET_FUNCTION(GT_voltex);

int GT_object_delete_time(struct GT_object *graphics_object,float time);
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Removes all primitive at <time> from <graphics_object>.
==============================================================================*/

#if defined (FULL_NAMES)
#define GT_OBJECT_REMOVE_PRIMITIVES_WITH_OBJECT_NAME_(primitive_type) \
	GT_object_remove_primitives_with_object_name_ ## primitive_type
#else
#define GT_OBJECT_REMOVE_PRIMITIVES_WITH_OBJECT_NAME_(primitive_type) \
	gorpwon_ ## primitive_type
#endif
#define GT_OBJECT_REMOVE_PRIMITIVES_WITH_OBJECT_NAME(primitive_type) \
	GT_OBJECT_REMOVE_PRIMITIVES_WITH_OBJECT_NAME_(primitive_type)

#define PROTOTYPE_GT_OBJECT_REMOVE_PRIMITIVES_WITH_OBJECT_NAME_FUNCTION( \
	primitive_type) \
int GT_OBJECT_REMOVE_PRIMITIVES_WITH_OBJECT_NAME(primitive_type)( \
	struct GT_object *graphics_object,float time,int object_name) \
/***************************************************************************** \
LAST MODIFIED : 20 April 1999 \
\
DESCRIPTION : \
Removes all primitives from <graphics_object> at <time> for which the \
object_name member matches the given <object_name>. \
============================================================================*/

PROTOTYPE_GT_OBJECT_REMOVE_PRIMITIVES_WITH_OBJECT_NAME_FUNCTION(GT_glyph_set);
PROTOTYPE_GT_OBJECT_REMOVE_PRIMITIVES_WITH_OBJECT_NAME_FUNCTION(GT_polyline);
PROTOTYPE_GT_OBJECT_REMOVE_PRIMITIVES_WITH_OBJECT_NAME_FUNCTION(GT_surface);
PROTOTYPE_GT_OBJECT_REMOVE_PRIMITIVES_WITH_OBJECT_NAME_FUNCTION(GT_voltex);

#if defined (FULL_NAMES)
#define GT_OBJECT_REMOVE_PRIMITIVES_WITH_AUXILIARY_OBJECT_NAME_(primitive_type) \
	GT_object_remove_primitives_with_auxiliary_object_name_ ## primitive_type
#else
#define GT_OBJECT_REMOVE_PRIMITIVES_WITH_AUXILIARY_OBJECT_NAME_(primitive_type) \
	gorpwaon_ ## primitive_type
#endif
#define GT_OBJECT_REMOVE_PRIMITIVES_WITH_AUXILIARY_OBJECT_NAME(primitive_type) \
	GT_OBJECT_REMOVE_PRIMITIVES_WITH_AUXILIARY_OBJECT_NAME_(primitive_type)

#define PROTOTYPE_GT_OBJECT_REMOVE_PRIMITIVES_WITH_AUXILIARY_OBJECT_NAME_FUNCTION( primitive_type ) \
int GT_OBJECT_REMOVE_PRIMITIVES_WITH_AUXILIARY_OBJECT_NAME(primitive_type)( \
	struct GT_object *graphics_object, float time, int auxiliary_object_name) \
/***************************************************************************** \
LAST MODIFIED : 7 June 2001 \
\
DESCRIPTION : \
Removes all primitives from <graphics_object> at <time> for which the \
auxiliary_object_name member matches the given <auxiliary_object_name>. \
============================================================================*/

PROTOTYPE_GT_OBJECT_REMOVE_PRIMITIVES_WITH_AUXILIARY_OBJECT_NAME_FUNCTION( \
	GT_glyph_set);

enum Graphics_select_mode GT_object_get_select_mode(
	struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 7 July 2000

DESCRIPTION :
Gets the default_select_mode of a GT_object.
==============================================================================*/

int GT_object_set_select_mode(struct GT_object *graphics_object,
	enum Graphics_select_mode select_mode);
/*******************************************************************************
LAST MODIFIED : 7 July 2000

DESCRIPTION :
Sets the select_mode of the <graphics_object>.
==============================================================================*/

int GT_object_get_glyph_mirror_mode(struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Gets the glyph_mirror_mode of a GT_object -- true or false.
???RC temporary until we have a separate struct Glyph.
==============================================================================*/

int GT_object_set_glyph_mirror_mode(struct GT_object *graphics_object,
	int glyph_mirror_mode);
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Sets the glyph_mirror_mode of the <graphics_object> to true or false.
???RC temporary until we have a separate struct Glyph.
==============================================================================*/

int GT_object_clear_selected_graphic_list(struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Clears the list of selected primitives and subobjects in <graphics_object>.
==============================================================================*/

int GT_object_select_graphic(struct GT_object *graphics_object,int number,
	struct Multi_range *subranges);
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Selects graphics with the object_name <number> in <graphics_object>, with
optional <subranges> narrowing down the selection. Selected objects will be
rendered with the selected/highlight material.
Replaces any current selection with the same <number>.
Iff the function is successful the <subranges> will be owned by the
graphics_object.
==============================================================================*/

int GT_object_is_graphic_selected(struct GT_object *graphics_object,int number,
	struct Multi_range **subranges);
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Returns true if graphics with the given <number> are selected in
<graphics_object>, and if so, its selected subranges are returned with it. The
returned subranges (which can be NULL if there are none) are for short-term
viewing only as they belong the the Selected_graphic containing them.
==============================================================================*/

struct Graphical_material *get_GT_object_default_material(
	struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 4 June 1999

DESCRIPTION :
Gets the default_material of a GT_object.
==============================================================================*/

int set_GT_object_default_material(struct GT_object *graphics_object,
	struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 11 June 1998

DESCRIPTION :
Sets the default_material of a GT_object.
==============================================================================*/

struct Graphical_material *get_GT_object_selected_material(
	struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Gets the selected_material of a GT_object.
==============================================================================*/

int set_GT_object_selected_material(struct GT_object *graphics_object,
	struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Sets the selected_material of a GT_object.
==============================================================================*/

int set_GT_object_Spectrum(struct GT_object *graphics_object,
	void *spectrum_void);
/*******************************************************************************
LAST MODIFIED : 20 October 1997

DESCRIPTION :
Sets the spectrum of a GT_object.
==============================================================================*/

struct Spectrum *get_GT_object_spectrum(struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 4 June 1999

DESCRIPTION :
Gets the spectrum of a GT_object.
==============================================================================*/

int set_GT_object_list_Spectrum(struct LIST(GT_object) *graphics_object_list,
	struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 18 October 1997

DESCRIPTION :
Sets the spectrum of all the GT_objects in a list.
==============================================================================*/

int GT_object_list_contents(struct GT_object *graphics_object,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 5 January 1998

DESCRIPTION :
Writes out information contained in <graphics_object> including its name and
type.
==============================================================================*/

int expand_spectrum_range_with_graphics_object(
	struct GT_object *graphics_object,void *spectrum_void);
/*******************************************************************************
LAST MODIFIED : 24 November 1997

DESCRIPTION :
Ensures the <spectrum> maximum and minimum is at least large enough to include
the range of data values in <graphics_object>.
==============================================================================*/

int set_Graphics_object(struct Parse_state *state,
	void *graphics_object_address_void,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 15 May 1998

DESCRIPTION :
Modifier function to set the graphics_object from a command.
==============================================================================*/

int resolve_glyph_axes(Triple point, Triple axis1, Triple axis2,
	Triple axis3, Triple scale, int mirror, int reverse, Triple final_point,
	Triple final_axis1, Triple final_axis2, Triple final_axis3);
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Multiplies the three axes by their <scale> to give the final axes, reversing
<final_axis3> if necessary to produce a right handed coordinate system.
If <mirror> is true, then the axes are pointed in the opposite direction.
If <reverse> is true, then the point is shifted to the end of each axis if the
scale is negative for that axis.
==============================================================================*/
#endif /* !defined (GRAPHICS_OBJECT_H) */
