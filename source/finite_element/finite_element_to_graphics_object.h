/*******************************************************************************
FILE : finite_element_to_graphics_object.h

LAST MODIFIED : 20 August 1999

DESCRIPTION :
The function prototypes for creating graphical objects from finite elements.
==============================================================================*/
#if !defined (FINITE_ELEMENT_TO_GRAPHICAL_OBJECT_H)
#define FINITE_ELEMENT_TO_GRAPHICAL_OBJECT_H

#include "finite_element/computed_field.h"
#include "finite_element/finite_element.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/graphics_object.h"
#include "graphics/volume_texture.h"

/*
Global types
------------
*/
struct Element_to_cylinder_data
/*******************************************************************************
LAST MODIFIED : 20 August 1999

DESCRIPTION :
Data for converting a 1-D element into a cylinder.
==============================================================================*/
{
	char exterior;
	int face_number;
	/* radius = constant_radius + scale_factor*radius_field */
	float constant_radius,scale_factor,time;
	int number_of_segments_along,number_of_segments_around;
	struct Computed_field *coordinate_field,*data_field,*radius_field;
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
LAST MODIFIED : 21 October 1998

DESCRIPTION :
Data for converting a 3-D element into a volume.
==============================================================================*/
{
	struct Clipping *clipping;
	struct Computed_field *blur_field;
	struct Computed_field *coordinate_field;
	struct Computed_field *data_field;
	struct Computed_field *surface_data_density_field;
	struct Computed_field *displacement_map_field;
	int displacement_map_xi_direction;
	struct GROUP(FE_node) *surface_data_group;
	struct GT_object *graphics_object;
	struct MANAGER(FE_node) *data_manager;
	struct MANAGER(FE_field) *fe_field_manager;
	float time;
	struct VT_volume_texture *volume_texture;
}; /* struct Element_to_volume_data */

struct Element_to_iso_surface_data
/*******************************************************************************
LAST MODIFIED : 15 February 1999

DESCRIPTION :
Data for converting a 3-D element into an iso_surface (via a volume_texture).
???RC Only converted iso_field_scalar to Computed_fields yet, since haven't
converted voltex code.
==============================================================================*/
{
	char *graphics_object_name;
	double iso_value;
	int field_component_number;
	float time;
	gtObject *graphics_object;
	struct Clipping *clipping;
	struct Computed_field *coordinate_field, *data_field, *iso_scalar_field;
	struct Computed_field *surface_data_density_field;
	struct Element_discretization discretization;
	struct Graphical_material *material;
	struct LIST(GT_object) *graphics_object_list;
	struct GROUP(FE_node) *surface_data_group;
	struct MANAGER(FE_node) *data_manager;
	struct MANAGER(FE_field) *fe_field_manager;
}; /* struct Element_to_iso_surface_data */

struct Element_to_glyph_set_data
/*******************************************************************************
LAST MODIFIED : 20 August 1999

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
The <dimension> value limits the elements in the following way:
1 = CM_LINE or dimension 1;
2 = CM_FACE or dimension 2;
3 = CM_ELEMENT or dimension 3.
If the dimension is less than 3, <exterior> and <face_number> may be used.
==============================================================================*/
{
	char exterior;
	enum Glyph_edit_mode glyph_edit_mode;
	float time;
	enum Xi_discretization_mode xi_discretization_mode;
	int dimension,face_number,number_of_cells_in_xi1,number_of_cells_in_xi2,
		number_of_cells_in_xi3;
	struct Computed_field *coordinate_field,*data_field,*label_field,
		*orientation_scale_field;
	struct FE_field *native_discretization_field;
	struct GROUP(FE_element) *element_group;
	struct GT_object *glyph,*graphics_object;
	Triple glyph_centre,glyph_size,glyph_scale_factors;
}; /* struct Element_to_glyph_set_data */

/*
Global functions
----------------
*/

struct GT_glyph_set *create_GT_glyph_set_from_FE_element(
	struct FE_element *element,struct FE_element *top_level_element,
	struct Computed_field *coordinate_field,
	int number_of_xi_points,Triple *xi_points,
	struct GT_object *glyph,Triple glyph_centre,Triple glyph_size,
	struct Computed_field *orientation_scale_field,Triple glyph_scale_factors,
	struct Computed_field *data_field,struct Computed_field *label_field,
	enum Glyph_edit_mode glyph_edit_mode);
/*******************************************************************************
LAST MODIFIED : 13 July 1999

DESCRIPTION :
Converts a finite element into a set of glyphs displaying information
about fields defined over it.
At each of the <number_of_xi_points> <xi_points> the <glyph> of <glyph_size>
with its centre located at <glyph_centre> is displayed.
The optional <orientation_scale_field> can be used to orient and scale the
glyph in a manner depending on the number of components in the field (see
function make_glyph_orientation_scale_axes). The three <glyph_scale_factors>
multiply the scaling effect in each axis taken from the
<orientation_scale_field>.
The optional <data_field> (currently only a scalar) is calculated as data over
the glyph_set, for later colouration by a spectrum.
The optional <label_field> is written beside each glyph in string form.
The optional <top_level_element> may be provided as a clue to Computed_fields
to say which parent element they should be evaluated on as necessary.
<glyph_edit_mode> is not used yet and should be set to GLYPH_EDIT_NOTHING.
Note:
- the coordinate and orientation fields are assumed to be rectangular cartesian.
==============================================================================*/

struct GT_glyph_set *create_GT_glyph_set_from_FE_node_group(
	struct GROUP(FE_node) *node_group,struct MANAGER(FE_node) *node_manager,
	struct Computed_field *coordinate_field,struct GT_object *glyph,
	Triple glyph_centre,Triple glyph_size,
	struct Computed_field *orientation_scale_field,Triple glyph_scale_factors,
	struct Computed_field *data_field,struct Computed_field *label_field,
	enum Glyph_edit_mode glyph_edit_mode);
/*******************************************************************************
LAST MODIFIED : 13 July 1999

DESCRIPTION :
Creates a GT_glyph_set displaying a <glyph> of size <glyph_size>, with centre
at <glyph_centre>, at each node in <node_group> (or <node_manager> if the former
is NULL).
The optional <orientation_scale_field> can be used to orient and scale the
glyph in a manner depending on the number of components in the field (see
function make_glyph_orientation_scale_axes). The three <glyph_scale_factors>
multiply the scaling effect in each axis taken from the
<orientation_scale_field>.
The optional <data_field> is calculated as data over the glyph_set, for later
colouration by a spectrum.
The optional <label_field> is written beside each glyph in string form.
The <glyph_edit_mode> controls whether node cmiss numbers are output as integer
names with the glyph_set, and how they are to be interpreted when editing.

Notes:
- the coordinate and orientation fields are assumed to be rectangular cartesian.
==============================================================================*/

struct GT_polyline *create_GT_polyline_from_FE_element(
	struct FE_element *element,struct Computed_field *coordinate_field,
	struct Computed_field *data_field,int number_of_segments,
	struct FE_element *top_level_element);
/*******************************************************************************
LAST MODIFIED : 2 July 1999

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
	struct FE_element *top_level_element);
/*******************************************************************************
LAST MODIFIED : 2 July 1999

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
Notes:
- the coordinate field is assumed to be rectangular cartesian.
==============================================================================*/

struct GT_nurbs *create_GT_nurb_from_FE_element(struct FE_element *element,
	struct Computed_field *coordinate_field,
	struct Computed_field *texture_coordinate_field,
	struct FE_element *top_level_element);
/*******************************************************************************
LAST MODIFIED : 6 July 1999

DESCRIPTION :
The optional <top_level_element> may be provided as a clue to Computed_fields
to say which parent element they should be evaluated on as necessary.
==============================================================================*/

struct GT_surface *create_GT_surface_from_FE_element(
	struct FE_element *element,struct Computed_field *coordinate_field,
	struct Computed_field *texture_coordinate_field,
	struct Computed_field *data_field,int number_of_segments_in_xi1_requested,
	int number_of_segments_in_xi2_requested,char reverse_normals,
	struct FE_element *top_level_element);
/*******************************************************************************
LAST MODIFIED : 2 July 1999

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
	struct VT_volume_texture *vtexture,
	struct Computed_field *displacement_field, int displacement_map_xi_direction,
	struct Computed_field *blur_field);
/*******************************************************************************
LAST MODIFIED : 2 July 1999

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
	struct MANAGER(FE_field) *fe_field_manager, struct Computed_field *data_density_field);
/*******************************************************************************
LAST MODIFIED : 28 April 1999

DESCRIPTION :
==============================================================================*/

int warp_GT_voltex_with_FE_element(struct GT_voltex *voltex1,
	struct GT_voltex *voltex2,struct FE_element *element,
	struct FE_field *warp_field,double ximax[3],float *warp_values,int xi_order);
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Replaces voltex2 vertices with voltex1 vertices warped over an element block
Vertices are normalized by their x, y, z range values.
???DB.  xi_order should be an enumerated type
==============================================================================*/

int warp_FE_node_group_with_FE_element(struct GROUP(FE_node) *node_group,
	struct MANAGER(FE_node) *node_manager,struct FE_field *coordinate_field,
	struct FE_field *to_coordinate_field,struct FE_element *element,
	struct FE_field *warp_field,double ximax[3],float *warp_values,int xi_order);
/*******************************************************************************
LAST MODIFIED : 4 March 1998

DESCRIPTION :
Replaces voltex2 vertices with voltex1 vertices warped over an element block
Vertices are normalized by their x, y, z range values.
???DB.  xi_order should be an enumerated type
==============================================================================*/

Triple *get_xi_points_at_cell_centres(int number_of_cells_in_xi1,
	int number_of_cells_in_xi2,int number_of_cells_in_xi3,
	int *number_of_xi_points);
/*******************************************************************************
LAST MODIFIED : 15 September 1998

DESCRIPTION :
Allocates and returns an array of xi locations at the centres of
<number_of_cells_in_xi1>*<number_of_cells_in_xi2>*<number_of_cells_in_xi3>
cells of equal size in xi over a 3-D element. The function also returns the
<number_of_xi_points> calculated.
This function also handles 1-D and 2-D cells in the following way:
* If <number_of_cells_in_xi2> is zero, then only <number_of_cells_in_xi1> points
  are calculated and only in 1 dimension, ie. xi[1]=xi[2]=0.0.
* If <number_of_cells_in_xi3> is zero, then only <number_of_cells_in_xi1>*
  <number_of_cells_in_xi2> points are calculated and only in 2 dimension, ie.
  xi[2]=0.0.
Note: xi changes from 0 to 1 over each element direction.
==============================================================================*/

Triple *get_xi_points_at_cell_corners(int number_of_cells_in_xi1,
	int number_of_cells_in_xi2,int number_of_cells_in_xi3,
	int *number_of_xi_points);
/*******************************************************************************
LAST MODIFIED : 3 November 1998

DESCRIPTION :
Allocates and returns an array of xi locations at the corners of
<number_of_cells_in_xi1>*<number_of_cells_in_xi2>*<number_of_cells_in_xi3>
cells of equal size in xi over a 3-D element. The function also returns the
<number_of_xi_points> calculated (remember: there is one more in each direction
than the number of cells).
This function also handles 1-D and 2-D cells in the following way:
* If <number_of_cells_in_xi2> is zero, then only <number_of_cells_in_xi1> cells
  are calculated and only in 1 dimension, ie. xi[1]=xi[2]=0.0.
* If <number_of_cells_in_xi3> is zero, then only <number_of_cells_in_xi1>*
  <number_of_cells_in_xi2> cells are calculated and only in 2 dimension, ie.
  xi[2]=0.0.
Note: xi changes from 0 to 1 over each element direction.
==============================================================================*/

Triple *get_xi_points_in_cells_random(int number_of_cells_in_xi1,
	int number_of_cells_in_xi2,int number_of_cells_in_xi3,
	int *number_of_xi_points);
/*******************************************************************************
LAST MODIFIED : 2 March 1999

DESCRIPTION :
Allocates and returns an array of xi locations each at random locations in
<number_of_cells_in_xi1>*<number_of_cells_in_xi2>*<number_of_cells_in_xi3>
cells of equal size in xi over a 3-D element. The function also returns the
<number_of_xi_points> calculated.
This function also handles 1-D and 2-D cells in the following way:
* If <number_of_cells_in_xi2> is zero, then only <number_of_cells_in_xi1> points
  are calculated and only in 1 dimension, ie. xi[1]=xi[2]=0.0.
* If <number_of_cells_in_xi3> is zero, then only <number_of_cells_in_xi1>*
  <number_of_cells_in_xi2> points are calculated and only in 2 dimension, ie.
  xi[2]=0.0.
Note: xi changes from 0 to 1 over each element direction.
==============================================================================*/

int convert_xi_points_from_element_to_parent(int number_of_xi_points,
	Triple *xi_points,int element_dimension,int parent_dimension,
	FE_value *element_to_parent);
/*******************************************************************************
LAST MODIFIED : 17 September 1998

DESCRIPTION :
Converts the list of <number_of_xi_points> xi coordinates in the <xi_points>
array from the <element_dimension> (=number of xi coordinates) to the
<parent_dimension>, which must be greater. Only the first <element_dimension>
components of the <xi_points> will therefore have useful values - the remaining
of the 3 components should be zero.

Matrix <element_to_parent> is (parent_dimension) rows X (element_dimension+1)
columns in size, with the first column being the xi offset vector b, such that,
xi(parent) = b + A.xi(element)
while A is the remainder of the matrix. (Appropriate matrices are given by the
face_to_element member of struct FE_element_shape, and by function
FE_element_get_top_level_element_conversion.)
==============================================================================*/

int make_glyph_orientation_scale_axes(int number_of_values,
	FE_value *orientation_scale_values,FE_value *axis1,FE_value *axis2,
	FE_value *axis3,FE_value *size);
/*******************************************************************************
LAST MODIFIED : 14 February 1999

DESCRIPTION :
Computes the three glyph orientation axes from the <orientation_scale_values>.

The orientation is understood from the number_of_values as:
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

struct VT_vector_field *interpolate_vector_field_on_FE_element(double ximax[3],
	struct FE_element *element,struct Computed_field *coordinate_field,
	struct VT_vector_field *vector_field);
/*******************************************************************************
LAST MODIFIED : 13 November 1994

DESCRIPTION :
Interpolates xi points (triples in vector field) over the finite <element>
<block>.  Outputs the updated field.
==============================================================================*/

struct GT_voltex *generate_clipped_GT_voltex_from_FE_element(
	struct Clipping *clipping,struct FE_element *element,
	struct Computed_field *coordinate_field,struct Computed_field *field_scalar,
	struct VT_volume_texture *texture,
	struct Computed_field *displacement_map_field, int displacement_map_xi_direction,
	struct Computed_field *blur_field);
/*******************************************************************************
LAST MODIFIED : 2 July 1999

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

int element_to_iso_surface(struct FE_element *element,
	void *void_element_to_iso_surface_data);
/*******************************************************************************
LAST MODIFIED : 15 February 1999

DESCRIPTION :
Converts a 3-D element into an iso_surface (via a volume_texture).
==============================================================================*/

int set_Texture_image_from_field(struct Texture *texture, struct Computed_field *field,
	struct Computed_field *texture_coordinate_field, struct Spectrum *spectrum,
	enum Texture_storage_type storage,int image_width,int image_height,
	char *image_file_name,int crop_left_margin,int crop_bottom_margin,
	int crop_width,int crop_height);
/*******************************************************************************
LAST MODIFIED : 26 May 1999

DESCRIPTION :
Creates the image in the format given by sampling the <field> according to the
reverse mapping of the <texture_coordinate_field>.  The values returned by
field are converted to "colours" by applying the <spectrum>
==============================================================================*/
#endif /* !defined (FINITE_ELEMENT_TO_GRAPHICAL_OBJECT_H) */
