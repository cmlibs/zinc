/*******************************************************************************
FILE : finite_element_to_graphics_object.h

LAST MODIFIED : 11 October 2002

DESCRIPTION :
The function prototypes for creating graphical objects from finite elements.
==============================================================================*/
#if !defined (FINITE_ELEMENT_TO_GRAPHICAL_OBJECT_H)
#define FINITE_ELEMENT_TO_GRAPHICAL_OBJECT_H

#include "computed_field/computed_field.h"
#include "finite_element/finite_element.h"
#include "general/enumerator.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/graphics_object.h"
#include "graphics/volume_texture.h"

/*
Global types
------------
*/
enum Use_element_type
/*******************************************************************************
LAST MODIFIED : 20 March 2001

DESCRIPTION :
For glyph sets - determines whether they are generated from:
USE_ELEMENTS = CM_ELEMENT or dimension 3
USE_FACES    = CM_FACE or dimension 2
USE_LINES    = CM_LINE or dimension 1
==============================================================================*/
{
  USE_ELEMENTS,
  USE_FACES,
  USE_LINES
}; /* enum Use_element_type */

struct Element_to_cylinder_data
/*******************************************************************************
LAST MODIFIED : 11 October 2002

DESCRIPTION :
Data for converting a 1-D element into a cylinder.
==============================================================================*/
{
	char exterior;
	int face_number;
	/* radius = constant_radius + scale_factor*radius_field */
	float constant_radius,scale_factor,time;
	int number_of_segments_along,number_of_segments_around;
	struct Computed_field *coordinate_field, *data_field, *radius_field,
		*texture_coordinate_field;
	struct Graphical_material *material;
	struct GROUP(FE_element) *element_group;
	struct GT_object *graphics_object;
}; /* struct Element_to_cylinder_data */

struct Element_to_polyline_data
/*******************************************************************************
LAST MODIFIED : 20 August 1999

DESCRIPTION :
Data for converting a 1-D element into a polyline.
==============================================================================*/
{
	char exterior;
	float time;
	int face_number,number_of_segments_in_xi1;
	struct Computed_field *coordinate_field,*data_field;
	struct GROUP(FE_element) *element_group;
	struct GT_object *graphics_object;
}; /* struct Element_to_line_data */

struct Element_to_surface_data
/*******************************************************************************
LAST MODIFIED : 20 August 1999

DESCRIPTION :
Data for converts a 2-D element into a surface.
==============================================================================*/
{
	char exterior,reverse_normals;
	/* Can be either g_SURFACE or g_NURBS */
	enum GT_object_type object_type;
	enum Render_type render_type;
	float time;
	int face_number,number_of_segments_in_xi1,number_of_segments_in_xi2;
	struct Computed_field *coordinate_field,*data_field,*texture_coordinate_field;
	struct GROUP(FE_element) *element_group;
	struct GT_object *graphics_object;
}; /* struct Element_to_surface_data */

struct Displacement_map
/*******************************************************************************
LAST MODIFIED : 27 April 1998

DESCRIPTION :
Used for displaying voltexes.
???RC Move to texture? volume_texture?
==============================================================================*/
{
	double scale;
	int xi_direction;
	struct Texture *texture;
}; /* struct Displacement_map */

struct Surface_pointset
/*******************************************************************************
LAST MODIFIED : 21 December 1998

DESCRIPTION :
==============================================================================*/
{
	float scale;
	struct GT_pointset *surface_points;
	struct Texture *texture;
}; /* struct Surface_pointset */

struct Warp_values
/*******************************************************************************
LAST MODIFIED : 27 May 1998

DESCRIPTION :
Used for warping voltexes.
???RC Move to volume_texture?
==============================================================================*/
{
	float value[6];
}; /* struct Warp_values */

#if defined (OLD_CODE)
struct Warp_data
/*******************************************************************************
LAST MODIFIED : 27 May 1998

DESCRIPTION :
Data for warping a volume.
==============================================================================*/
{
	struct GT_object *graphics_object1,*graphics_object2;
	struct Element_discretization *extent;
	struct FE_field *coordinate_field;
	struct Warp_values warp_values;
	int xi_order;
}; /* struct Warp_data */
#endif /* defined (OLD_CODE) */

struct Element_to_volume_data
/*******************************************************************************
LAST MODIFIED : 4 December 2000

DESCRIPTION :
Data for converting a 3-D element into a volume.
==============================================================================*/
{
	struct Clipping *clipping;
	struct Computed_field *blur_field;
	struct Computed_field *coordinate_field;
	struct Computed_field *data_field;
	struct Computed_field *surface_data_density_field;
	struct Computed_field *surface_data_coordinate_field;
	struct Computed_field *texture_coordinate_field;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Computed_field *displacement_map_field;
	int displacement_map_xi_direction;
	struct FE_time *fe_time;
	struct GROUP(FE_node) *surface_data_group;
	struct GT_object *graphics_object;
	struct MANAGER(FE_node) *data_manager;
	struct MANAGER(FE_field) *fe_field_manager;
	enum Render_type render_type;
	float time;
	struct VT_volume_texture *volume_texture;
}; /* struct Element_to_volume_data */

struct Element_to_iso_scalar_data
/*******************************************************************************
LAST MODIFIED : 4 December 2000

DESCRIPTION :
Data for converting a 3-D element into an iso_surface (via a volume_texture).
==============================================================================*/
{
	char exterior;
	double iso_value;
	enum Use_element_type use_element_type;
	float time;
	int face_number;
	struct Clipping *clipping;
	struct Computed_field *coordinate_field, *data_field, *scalar_field;
	struct Computed_field *surface_data_coordinate_field;
	struct Computed_field *texture_coordinate_field;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Computed_field *surface_data_density_field;
	int number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct FE_field *native_discretization_field;
	struct GT_object *graphics_object;
	enum Render_type render_type;
	struct GROUP(FE_element) *element_group;
	struct GROUP(FE_node) *surface_data_group;
	struct MANAGER(FE_node) *data_manager;
	struct MANAGER(FE_field) *fe_field_manager;
	struct FE_time *fe_time;
}; /* struct Element_to_iso_surface_data */

struct Element_to_glyph_set_data
/*******************************************************************************
LAST MODIFIED : 8 May 2001

DESCRIPTION :
Data for converting a finite element into a set of glyphs displaying information
about fields defined over it.
If the <native_discretization_field> is given and uses an element based field in
any element, the native discretization is taken as the number of regular-sized
cells in each xi direction of the element, otherwise, the three values of
number_of_cells_in_xi* are used.
If the <cell_corners_flag> is not set, glyphs are displayed at the cell centres,
otherwise they are displayed at the cell corners and there will be one extra
point in each xi-direction as there are cells.
At each of these points the <glyph> of <glyph_size> with its centre located at
<glyph_centre> is displayed. The optional <orientation_scale_field> can be used
to orient and scale the glyph in a manner depending on the number of components
in the field (see function make_glyph_orientation_scale_axes). The three
<glyph_scale_factors> multiply the scaling effect in each axis taken from the
<orientation_scale_field>.
The optional <data_field> (currently only a scalar) is calculated as data over
the glyph_set, for later colouration by a spectrum.
The optional <label_field> is written beside each glyph in string form.
The <use_element_type> determines the type/dimension of elements in use.
If the dimension is less than 3, <exterior> and <face_number> may be used.
==============================================================================*/
{
	char exterior;
	enum Graphics_select_mode select_mode;
	enum Use_element_type use_element_type;
	enum Xi_discretization_mode xi_discretization_mode;
	FE_value base_size[3], centre[3], scale_factors[3];
	float time;
	int face_number,number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct Computed_field *coordinate_field, *data_field, *variable_scale_field,
		*label_field, *orientation_scale_field, *xi_point_density_field;
	struct FE_field *native_discretization_field;
	struct GROUP(FE_element) *element_group;
	struct GT_object *glyph,*graphics_object;
	Triple exact_xi;
}; /* struct Element_to_glyph_set_data */

enum Collapsed_element_type
/*******************************************************************************
LAST MODIFIED : 25 July 2001

DESCRIPTION :
How an element is collapsed.
???DB.  Currently only 2D
==============================================================================*/
{
	ELEMENT_NOT_COLLAPSED,
	ELEMENT_COLLAPSED_XI1_0,
	ELEMENT_COLLAPSED_XI1_1,
	ELEMENT_COLLAPSED_XI2_0,
	ELEMENT_COLLAPSED_XI2_1
}; /* enum Collapsed_element_type */

/*
Global functions
----------------
*/
int make_glyph_orientation_scale_axes(
	int number_of_orientation_scale_values, FE_value *orientation_scale_values,
	FE_value *axis1,FE_value *axis2, FE_value *axis3, FE_value *size);
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Computes the three glyph orientation axes from the <orientation_scale_values>.

The orientation is understood from the number_of_orientation_scale_values as:
0 = zero scalar (no vector/default orientation);
1 = scalar (no vector/default orientation);
2 = 1 2-D vector (2nd glyph axis is normal in plane, 3rd is out of 2-D plane);
3 = 1 3-D vector (orthogonal 2nd and 3rd glyph axes are arbitrarily chosen);
4 = 2 2-D vectors (3rd glyph axis taken as out of 2-D plane);
6 = 2 3-D vectors (3rd glyph axis found from cross product);
9 = 3 3-D vectors = complete definition of glyph axes;

The scaling behaviour depends on the number of vectors interpreted above, where:
0 = isotropic scaling on all three axes by scalar;
1 = isotropic scaling on all three axes by magnitude of vector;
2 = scaling in direction of 2 vectors, ie. they keep their current length, unit
    vector in 3rd axis;
3 = scaling in direction of 3 vectors - ie. they keep their current length.

Function returns the axes as unit vectors with their magnitudes in the <size>
array. This is always possible if there is a scalar (or zero scalar), but where
zero vectors are either read or calculated from the <orientation_scale_values>,
these are simply returned, since no valid direction can be produced.
==============================================================================*/

PROTOTYPE_ENUMERATOR_FUNCTIONS(Use_element_type);

enum CM_element_type Use_element_type_CM_element_type(
	enum Use_element_type use_element_type);
/*******************************************************************************
LAST MODIFIED : 22 December 1999

DESCRIPTION :
Returns the CM_element_type expected for the <use_element_type>. Note that a
match is found if either the dimension or the CM_element_type matches the
element.
==============================================================================*/

int Use_element_type_dimension(enum Use_element_type use_element_type);
/*******************************************************************************
LAST MODIFIED : 22 December 1999

DESCRIPTION :
Returns the dimension expected for the <use_element_type>. Note that a match is
found if either the dimension or the CM_element_type matches the element.
==============================================================================*/

struct FE_element *FE_element_group_get_element_with_Use_element_type(
	struct GROUP(FE_element) *element_group,
	enum Use_element_type use_element_type,int element_number);
/*******************************************************************************
LAST MODIFIED : 1 March 2000

DESCRIPTION :
Because USE_FACES can refer to either a 2-D CM_FACE or a 2-D CM_ELEMENT, and
USE_LINES can refer to a 1-D CM_LINE or a 1-D CM_ELEMENT, this function handles
the logic for getting the most appropriate element from <element_group> with
the given the <use_element_type> and <element_number>.
==============================================================================*/

int CM_element_information_to_graphics_name(struct CM_element_information *cm);
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Encodes <cm> as a single integer that can be converted back to the element with
CM_element_information_from_graphics_name. cm->number must be non-negative,
and for CM_ELEMENT and CM_FACE must be less than INT_MAX/2, since they share the
positive integers. Note: keeps element numbers contiguous for a CM_type.
Used for selection and highlighting of elements.
==============================================================================*/

int CM_element_information_from_graphics_name(struct CM_element_information *cm,
	int graphics_name);
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
Fills <cm> with the CM_type and number determined from the the integer
<graphics_name>, encoded with CM_element_information_to_graphics_name.
Used for selection and highlighting of elements.
==============================================================================*/

struct GT_glyph_set *create_GT_glyph_set_from_FE_element(
	struct FE_element *element, struct FE_element *top_level_element,
	struct Computed_field *coordinate_field,
	int number_of_xi_points, Triple *xi_points, struct GT_object *glyph,
	FE_value *base_size, FE_value *centre, FE_value *scale_factors,
	struct Computed_field *orientation_scale_field,
	struct Computed_field *variable_scale_field,
	struct Computed_field *data_field, struct Computed_field *label_field,
	enum Graphics_select_mode select_mode, int element_selected,
	struct Multi_range *selected_ranges, int *point_numbers, FE_value time);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Converts a finite element into a set of glyphs displaying information
about fields defined over it.
At each of the <number_of_xi_points> <xi_points> the <glyph> of at least
<base_size> with the given glyph <centre> is displayed.
The optional <orientation_scale_field> can be used to orient and scale the
glyph in a manner depending on the number of components in the field. The
optional <variable_scale_field> can provide signed scaling independently of the
glyph axis directions. See function make_glyph_orientation_scale_axes for
details. The combined scale from the above 2 fields is multiplied in each axis
by the <scale_factors> then added to the base_size.
The optional <data_field> (currently only a scalar) is calculated as data over
the glyph_set, for later colouration by a spectrum.
The optional <label_field> is written beside each glyph in string form.
The optional <top_level_element> may be provided as a clue to Computed_fields
to say which parent element they should be evaluated on as necessary.
<select_mode> is used in combination with the <element_selected> and
<selected_ranges> to draw only those points with numbers in or out of the given
ranges when given value GRAPHICS_DRAW_SELECTED or GRAPHICS_DRAW_UNSELECTED.
If <element_selected> is true, all points are selected, otherwise selection is
determined from the <selected_ranges>, and if <selected_ranges> is NULL, no
numbers are selected.
If <point_numbers> are supplied then points numbers for OpenGL picking are taken
from this array, otherwise they are sequential, starting at 0.
Note:
- the coordinate and orientation fields are assumed to be rectangular cartesian.
- the coordinate system of the variable_scale_field is ignored/not used.
==============================================================================*/

struct GT_glyph_set *create_GT_glyph_set_from_FE_node_group(
	struct GROUP(FE_node) *node_group, struct MANAGER(FE_node) *node_manager,
	struct Computed_field *coordinate_field, struct GT_object *glyph,
	FE_value *base_size, FE_value *centre, FE_value *scale_factors,
	FE_value time, struct Computed_field *orientation_scale_field,
	struct Computed_field *variable_scale_field,
	struct Computed_field *data_field, struct Computed_field *label_field,
	enum Graphics_select_mode select_mode,
	struct LIST(FE_node) *selected_node_list);
/*******************************************************************************
LAST MODIFIED : 22 November 2001

DESCRIPTION :
Creates a GT_glyph_set displaying a <glyph> of at least <base_size>, with the
given glyph <centre> at each node in <node_group> (or <node_manager> if the
former is NULL).
The optional <orientation_scale_field> can be used to orient and scale the
glyph in a manner depending on the number of components in the field. The
optional <variable_scale_field> can provide signed scaling independently of the
glyph axis directions. See function make_glyph_orientation_scale_axes for
details. The combined scale from the above 2 fields is multiplied in each axis
by the <scale_factors> then added to the base_size.
The optional <data_field> is calculated as data over the glyph_set, for later
colouration by a spectrum.
The optional <label_field> is written beside each glyph in string form.
The <select_mode> controls whether node cmiss numbers are output as integer
names with the glyph_set. If <select_mode> is DRAW_SELECTED or DRAW_UNSELECTED,
the nodes (not) in the <selected_node_list> are rendered only. This
functionality is only supported if <node_group> is supplied.
Notes:
- the coordinate and orientation fields are assumed to be rectangular cartesian.
- the coordinate system of the variable_scale_field is ignored/not used.
==============================================================================*/

struct GT_polyline *create_GT_polyline_from_FE_element(
	struct FE_element *element,struct Computed_field *coordinate_field,
	struct Computed_field *data_field,int number_of_segments,
	struct FE_element *top_level_element, FE_value time);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Creates a <GT_polyline> from the <coordinate_field> for the 1-D finite <element>
using <number_of_segments> spaced equally in xi.
The optional <data_field> (currently only a scalar) is calculated as data over
the polyline, for later colouration by a spectrum.
The optional <top_level_element> may be provided as a clue to Computed_fields
to say which parent element they should be evaluated on as necessary.
Notes:
- the coordinate field is assumed to be rectangular cartesian.
==============================================================================*/

struct GT_surface *create_cylinder_from_FE_element(struct FE_element *element,
	struct Computed_field *coordinate_field,struct Computed_field *data_field,
	float constant_radius,float scale_factor,struct Computed_field *radius_field,
	int number_of_segments_along,int number_of_segments_around,
	struct Computed_field *texture_coordinate_field,
	struct FE_element *top_level_element,FE_value time);
/*******************************************************************************
LAST MODIFIED : 11 October 2002

DESCRIPTION :
Creates a <GT_surface> from the <coordinate_field> and the radius for the 1-D
finite <element> using a grid of points.  The cylinder is built out of an array
of rectangles <number_of_segments_along> by <number_of_segments_around> the
cylinder.The actual radius is calculated as:
radius = constant_radius + scale_factor*radius_field(a scalar field)
The optional <data_field> (currently only a scalar) is calculated as data over
the length of the cylinder, for later colouration by a spectrum.
The optional <top_level_element> may be provided as a clue to Computed_fields
to say which parent element they should be evaluated on as necessary.
The first component of <texture_coordinate_field> is used to control the
texture coordinates along the element. If not supplied it will match Xi. The
texture coordinate around the cylinders is always from 0 to 1.
Notes:
- the coordinate field is assumed to be rectangular cartesian.
==============================================================================*/

struct GT_nurbs *create_GT_nurb_from_FE_element(struct FE_element *element,
	struct Computed_field *coordinate_field,
	struct Computed_field *texture_coordinate_field,
	struct FE_element *top_level_element, FE_value time);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
The optional <top_level_element> may be provided as a clue to Computed_fields
to say which parent element they should be evaluated on as necessary.
==============================================================================*/

int get_surface_element_segmentation(struct FE_element *element,
	int number_of_segments_in_xi1_requested,
	int number_of_segments_in_xi2_requested,int reverse_normals,
	int *number_of_points_in_xi1,int *number_of_points_in_xi2,
	int *number_of_points,int *number_of_polygon_vertices,
	gtPolygonType *polygon_type,enum Collapsed_element_type *collapsed_element,
	char *modified_reverse_normals);
/*******************************************************************************
LAST MODIFIED : 30 July 2001

DESCRIPTION :
Sorts out how standard, polygon and simplex elements are segmented, based on
numbers of segments requested for "square" elements.
==============================================================================*/

struct GT_surface *create_GT_surface_from_FE_element(
	struct FE_element *element,struct Computed_field *coordinate_field,
	struct Computed_field *texture_coordinate_field,
	struct Computed_field *data_field,int number_of_segments_in_xi1_requested,
	int number_of_segments_in_xi2_requested,char reverse_normals,
	struct FE_element *top_level_element, enum Render_type render_type,
	FE_value time);
/*******************************************************************************
LAST MODIFIED : 2 May 2000

DESCRIPTION :
Creates a <GT_surface> from the <coordinate_field> for the 2-D finite <element>
using an array of <number_of_segments_in_xi1> by <number_of_segments_in_xi2>
rectangles in xi space.  The spacing is constant in each of xi1 and xi2.
The optional <texture_coordinate_field> is evaluated at each vertex for the
corresonding texture coordinates.  If none is supplied then a length based
algorithm is used instead.
The optional <data_field> (currently only a scalar) is calculated as data over
the surface, for later colouration by a spectrum.
The optional <top_level_element> may be provided as a clue to Computed_fields
to say which parent element they should be evaluated on as necessary.
Notes:
- the coordinate field is assumed to be rectangular cartesian.
???DB.  Check for collapsed elements (repeated nodes) and treat accordingly -
	discretise with triangles ?  Have done for one special case - bi-quadratic
	with collapse along xi2=0.  Extend ?
???DB.  Always calculates texture coordinates.  Calculate as required ?  Finite
	element graphical object ?
???DB.  18 September 1993.  Only the side with the normal pointing out is
	textured correctly (even in TWOSIDE mode).  For 2-D finite elements which
	are faces of 3-D finite elements make the normals point out of the element.
==============================================================================*/

struct GT_voltex *create_GT_voltex_from_FE_element(struct FE_element *element,
	struct Computed_field *coordinate_field,struct Computed_field *data_field,
	struct VT_volume_texture *vtexture, enum Render_type render_type,
	struct Computed_field *displacement_field, int displacement_map_xi_direction,
	struct Computed_field *blur_field,
	struct Computed_field *texture_coordinate_field, FE_value time);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Creates a <GT_voltex> from a 3-D finite <element> <block> and volume texture
The volume texture contains a list of coloured triangular polygons representing
an isosurface calculated from the volumetric data. These polygons are stored in
local xi1,2,3 space and undergo a free form deformation (FFD) when mapped onto
the finite <element> <block>. The output contains the deformed polygon & normal
list.  Only the vertices are transformed. The polygons are then generated from a
list of pointers to the vertices.  Normals are calculated from the resulting
deformed vertices by taking the average of the cross products of the surrounding
faces.
Added a surface_points set and a controlling texture_map and scale which list
seed points distributed on the surface randomly but with density due to the 
texture_map.
==============================================================================*/

int create_surface_data_points_from_GT_voltex(struct GT_voltex *voltex,
	struct FE_element *element, struct Computed_field *coordinate_field,
	struct VT_volume_texture *vtexture,
	struct MANAGER(FE_node) *data_manager, struct GROUP(FE_node) *data_group,
	struct MANAGER(Computed_field) *computed_field_manager,
	struct MANAGER(FE_field) *fe_field_manager, struct FE_time *fe_time,
	struct Computed_field *data_density_field,
	struct Computed_field *data_coordinate_field, FE_value time);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
This function takes a <voltex> and the corresponding <vtexture> and creates
a randomly placed set of data_points over the surface the <voltex> describes.
The vtexture is used to define an element_xi field at each of the data_points.
These can then be used to draw axes or gradient vectors over the surface of an
isosurface or to create hair streamlines starting from the surface of a volume
texture. If the <data_coordinate_field> is supplied, it - ie. the FE_field it is
calculated from -  will be defined at the data points, and the field is given
the position of the point, with appropriate coordinate conversion.
==============================================================================*/

int warp_GT_voltex_with_FE_element(struct GT_voltex *voltex1,
	struct GT_voltex *voltex2,struct FE_element *element,
	struct FE_field *warp_field,double ximax[3],float *warp_values,int xi_order,
	FE_value time);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Replaces voltex2 vertices with voltex1 vertices warped over an element block
Vertices are normalized by their x, y, z range values.
???DB.  xi_order should be an enumerated type
==============================================================================*/

int warp_FE_node_group_with_FE_element(struct GROUP(FE_node) *node_group,
	struct MANAGER(FE_node) *node_manager,struct FE_field *coordinate_field,
	struct FE_field *to_coordinate_field,struct FE_element *element,
	struct FE_field *warp_field,double ximax[3],float *warp_values,int xi_order,
	FE_value time);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Replaces voltex2 vertices with voltex1 vertices warped over an element block
Vertices are normalized by their x, y, z range values.
???DB.  xi_order should be an enumerated type
==============================================================================*/

struct VT_vector_field *interpolate_vector_field_on_FE_element(double ximax[3],
	struct FE_element *element,struct Computed_field *coordinate_field,
	struct VT_vector_field *vector_field, FE_value time);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Interpolates xi points (triples in vector field) over the finite <element>
<block>.  Outputs the updated field.
==============================================================================*/

struct GT_voltex *generate_clipped_GT_voltex_from_FE_element(
	struct Clipping *clipping,struct FE_element *element,
	struct Computed_field *coordinate_field,struct Computed_field *field_scalar,
	struct VT_volume_texture *texture, enum Render_type render_type,
	struct Computed_field *displacement_map_field, int displacement_map_xi_direction,
	struct Computed_field *blur_field, 
	struct Computed_field *texture_coordinate_field, FE_value time);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Generates clipped voltex from <volume texture> and <clip_function> over
<element><block>
==============================================================================*/

int write_FE_element_layout(double ximax[3],
	struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 20 March 1998

DESCRIPTION :
Prints out element block layout (given seed element) for use in external
programs
==============================================================================*/

int FE_element_can_be_displayed(struct FE_element *element,
	int dimension,enum CM_element_type cm_element_type,int exterior,
	int face_number,struct GROUP(FE_element) *element_group);
/*******************************************************************************
LAST MODIFIED : 22 December 1999

DESCRIPTION :
Returns true if the element is <exterior>, if set, and on the given
<face_number>, if non-negative, and that the parent element identifying this is
in the <element_group> in the latter case, again if specified. Tests are assumed
to succeed for all unspecified parameters.
Also tests whether the <dimension> or <cm_element_type> matches.
==============================================================================*/

int element_to_cylinder(struct FE_element *element,
	void *void_element_to_cylinder_data);
/*******************************************************************************
LAST MODIFIED : 14 February 1999

DESCRIPTION :
Converts a finite element into a cylinder.
==============================================================================*/

int element_to_polyline(struct FE_element *element,
	void *element_to_polyline_data_void);
/*******************************************************************************
LAST MODIFIED : 12 March 1999

DESCRIPTION :
Converts a finite element into a polyline and adds it to a graphics_object.
==============================================================================*/

int element_to_nurbs(struct FE_element *element,
	void *void_element_to_nurbs_data);
/*******************************************************************************
LAST MODIFIED : 12 March 1999

DESCRIPTION :
Converts a finite element into a nurbs surface.
==============================================================================*/

int element_to_surface(struct FE_element *element,
	void *void_element_to_surface_data);
/*******************************************************************************
LAST MODIFIED : 12 March 1999

DESCRIPTION :
Converts a finite element into a surface.
==============================================================================*/

int set_Displacement_map(struct Parse_state *state,
	void *displacement_map_void,void *texture_manager_void);
/*******************************************************************************
LAST MODIFIED : 27 April 1998

DESCRIPTION :
A modifier function for setting the displacement map.
???RC Move to texture? volume_texture?
==============================================================================*/

int set_Warp_values(struct Parse_state *state,
	void *warp_values_void,void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 6 February 1998

DESCRIPTION :
???RC Move to volume_texture?
==============================================================================*/

#if defined (OLD_CODE)
int warp_volume(struct FE_element *element,void *warp_data_void);
/*******************************************************************************
LAST MODIFIED : 15 September 1997

DESCRIPTION :
Warps a volume.
==============================================================================*/
#endif /* defined (OLD_CODE) */

int element_to_glyph_set(struct FE_element *element,
	void *new_element_to_glyph_set_data_void);
/*******************************************************************************
LAST MODIFIED : 4 January 1999

DESCRIPTION :
Converts a finite element into a set of glyphs displaying information about the
fields defined over it.
==============================================================================*/

int element_to_volume(struct FE_element *element,
	void *void_element_to_volume_data);
/*******************************************************************************
LAST MODIFIED : 16 May 1998

DESCRIPTION :
Converts a 3-D element into a volume.
==============================================================================*/

int element_to_iso_scalar(struct FE_element *element,
	void *element_to_iso_scalar_data_void);
/*******************************************************************************
LAST MODIFIED : 28 January 2000

DESCRIPTION :
Computes iso-surfaces/lines/points graphics from <element>.
==============================================================================*/

int create_iso_surfaces_from_FE_element(struct FE_element *element,
	double iso_value, FE_value time,struct Clipping *clipping,
	struct Computed_field *coordinate_field,
	struct Computed_field *data_field,struct Computed_field *scalar_field,
	struct Computed_field *surface_data_density_field,
	struct Computed_field *surface_data_coordinate_field,
	struct Computed_field *texture_coordinate_field,
	int *number_in_xi,
	struct GT_object *graphics_object,enum Render_type render_type,
	struct GROUP(FE_node) *surface_data_group,
	struct MANAGER(FE_node) *data_manager,
	struct MANAGER(FE_field) *fe_field_manager, struct FE_time *fe_time,
	struct MANAGER(Computed_field) *computed_field_manager);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Converts a 3-D element into an iso_surface (via a volume_texture).
==============================================================================*/

#endif /* !defined (FINITE_ELEMENT_TO_GRAPHICAL_OBJECT_H) */
