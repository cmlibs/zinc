/**
 * FILE : finite_element_shape.hpp
 *
 * Finite element shape bounds.
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (FINITE_ELEMENT_SHAPE_HPP)
#define FINITE_ELEMENT_SHAPE_HPP

#include "opencmiss/zinc/types/elementid.h"
#include <opencmiss/zinc/zincconfigure.h>
#include "general/list.h"

/*
Global types
------------
*/

struct FE_region;

/** The different shape types available.
 * Note that UNSPECIFIED_SHAPE is only used to establish elements where the
 * dimension alone is known. */
enum FE_element_shape_type
{
	UNSPECIFIED_SHAPE,
	LINE_SHAPE,
	POLYGON_SHAPE,
	SIMPLEX_SHAPE
};

/**
 * A description of the shape of an element in Xi space.  It includes how to
 * calculate face coordinates from element coordinates and how to calculate
 * element coordinates from face coordinates. */
struct FE_element_shape;

DECLARE_LIST_TYPES(FE_element_shape);

/*
Global functions
----------------
*/

struct FE_element_shape *CREATE(FE_element_shape)(int dimension,
	const int *type, struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 8 July 2003

DESCRIPTION :
Requests from the <fe_region> for a shape with the specified <dimension> and
<type>.  If one is not found, a shape is created (with <type> duplicated) and
added to the region.  The shape is returned.
<type> is analogous to the basis type array, except that the entries are 0 or 1.
If <type> is omitted an "unspecified" shape of the given <dimension> is
returned. An element with such a shape may not have fields defined on it until
it is given a proper shape.
==============================================================================*/

int DESTROY(FE_element_shape)(struct FE_element_shape **element_shape_address);
/*******************************************************************************
LAST MODIFIED : 23 September 1995

DESCRIPTION :
Remove the shape from the list of all shapes.  Free the memory for the shape and
sets <*element_shape_address> to NULL.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(FE_element_shape);

PROTOTYPE_LIST_FUNCTIONS(FE_element_shape);

/***************************************************************************//**
 * Creates an element shape object given just a cmzn_element_shape_type.
 *
 * @return  Accessed shape object or NULL on error.
 */
struct FE_element_shape *FE_element_shape_create_simple_type(
	struct FE_region *fe_region, enum cmzn_element_shape_type shape_type);

/**
 * Returns the number of faces of the element shape.
 */
int FE_element_shape_get_number_of_faces(const FE_element_shape *element_shape);

/***************************************************************************//**
 * Returns a cmzn_element_shape_type describing the shape if possible.
 *
 * @param element_shape   The shape object to query.
 * @return  The shape type, or unknown if not able to be described by enum.
 */
enum cmzn_element_shape_type FE_element_shape_get_simple_type(
	struct FE_element_shape *element_shape);

/***************************************************************************//**
 * Returns an allocated string with the EX file description of the shape, e.g.
 * - line*line*line
 * - simplex(3)*line*simplex
 * - simplex(2;3)*simplex*simplex
 * - polygon(5;2)*polygon*line
 */
char *FE_element_shape_get_EX_description(struct FE_element_shape *element_shape);

/***************************************************************************//**
 * Creates an unspecified element shape of the supplied dimension.
 *
 * @return  Accessed shape object or NULL on error.
 */
struct FE_element_shape *FE_element_shape_create_unspecified(
	struct FE_region *fe_region, int dimension);

/** Returns true if the <element_shape> has only LINE_SHAPE in each dimension. */
bool FE_element_shape_is_line(struct FE_element_shape *element_shape);

/** Returns true if the element_shape is 2d simplex */
bool FE_element_shape_is_triangle(struct FE_element_shape *element_shape);

struct FE_element_shape *get_FE_element_shape_of_face(
	const FE_element_shape *shape,int face_number, struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 7 July 2003

DESCRIPTION :
From the parent <shape> returns the FE_element_shape for its face <face_number>.
The <shape> must be of dimension 2 or 3. Faces of 2-D elements are always lines.
==============================================================================*/

/**
 * @return  The dimension of the element shape, or 0 if error.
 */
int get_FE_element_shape_dimension(struct FE_element_shape *element_shape);

/**
 * Return face_to_element map, a square matrix b + A xi for the face_number of
 * shape.
 *
 * @return  Address of internal face_to_element map, or 0 if invalid arguments.
 * Note: do not deallocate!
 */
const FE_value *get_FE_element_shape_face_to_element(
	struct FE_element_shape *element_shape, int face_number);

/**
 * Finds face number corresponding to xi location in element, if any.
 * Element coordinates must be within tolerance of the face equation.
 * Only supports simplex and line shapes, not polygon shape.
 * @param shape  Element shape to query.
 * @param xi  Element local coordinates in shape to check.
 * @param tolerance  Closeness of difference from face equation >= 0.0.
 * @param lastFaceNumber  Optional last face number to only try face numbers
 * higher than this. Use -1 for no last face number, otherwise must be from
 * 0 to number of faces - 1.
 * @return  Face number for shape >= 0, or -1 if not on a face.
 */
int FE_element_shape_find_face_number_for_xi(struct FE_element_shape *shape,
	FE_value *xi, FE_value tolerance, int lastFaceNumber = -1);

int get_FE_element_shape_xi_linkage_number(
	struct FE_element_shape *element_shape, int xi_number1, int xi_number2,
	int *xi_linkage_number_address);
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Returns a number indicating how the dimension <xi_number1> and <xi_number2> are
linked in <element_shape>.
If they are linked in a simplex, a non-zero return indicates they are linked.
If they are linked in a polygon, the returned number is the number of sides in
the polygon.
A value of zero indicates the dimensions are not linked.
Note the first xi_number is 0.
==============================================================================*/

int get_FE_element_shape_xi_shape_type(struct FE_element_shape *element_shape,
	int xi_number, enum FE_element_shape_type *shape_type_address);
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Returns the shape type of <element_shape> on <xi_number> -- on main diagonal of
type array. The first xi_number is 0.
==============================================================================*/

int get_FE_element_shape_next_linked_xi_number(
	struct FE_element_shape *element_shape, int xi_number,
	int *next_xi_number_address, int *xi_link_number_address);
/*******************************************************************************
LAST MODIFIED : 6 November 2002

DESCRIPTION :
Returns in <next_xi_number_address> the next xi number higher than <xi_number>
which is linked in shape with it, plus in <xi_link_number_address> the number
denoting how it is linked; currently used only for polygon shapes to denote the
number of polygon sides.
If there is no remaining linked dimension, 0 is returned in both addresses.
<xi_number> is from 0 to one less than the shape dimension.
Also checks that the linked xi numbers have the same shape type.
==============================================================================*/

/**
 * Adjust xi to be within the bounds of the element shape, moving it as needed
 * to the nearest point on its boundary.
 * @param shape  Element shape to limit to.
 * @param xi  Element local coordinates to adjust if needed.
 * @param tolerance  Amount that xi is allowed to be outside bounds.
 * @return  True if xi was limited to bounds, false if not or invalid arguments.
 */
bool FE_element_shape_limit_xi_to_element(FE_element_shape *shape,
	FE_value *xi, FE_value tolerance);

/**
 * Calculate the geometric unit surface normal for a face of the given shape from
 * the coordinate derivatives with respect to the shape.
 * @param shape  Element shape to query.
 * @param faceNumber  The face number >= 0.
 * @param coordinatesCount  The number of coordinate components, at least shape
 * dimension, at most 3.
 * @param coordinateDerivatives  Derivatives of the coordinates with respect to
 * xi for the shape.
 * @param faceNormal  Array of length coordinatesCount to receive outward unit
 * normal.
 * @return  True on success, false if failed.
 */
bool FE_element_shape_get_face_outward_normal(FE_element_shape *shape, int faceNumber,
	int coordinatesCount, const FE_value *coordinateDerivatives, FE_value *faceNormal);

/** Calculates the <normal>, in xi space, of the specified face.
 * Cannot guarantee that <normal> is inward. */
int FE_element_shape_calculate_face_xi_normal(struct FE_element_shape *shape,
	int face_number, FE_value *normal);

/**
 * Return whether the shape-face mapping for the supplied face number gives the
 * face an inward normal.
 *
 * @param shape  The shape - must be 2 or 3 dimensional. For 2-D the normal is
 * inward if the primary xi of the face is anticlockwise around the element
 * such that the cross product of the parent normal (d1xd2) and the face xi is
 * inward.
 * @param face_number  Face index from 0 to (shape->number_of_faces - 1).
 * @return  True if face has inward normal, otherwise false.
 */
bool FE_element_shape_face_has_inward_normal(struct FE_element_shape *shape,
	int face_number);

/**
 * Adds the <increment> to <xi>.  If this moves <xi> outside of the shape, then
 * the step is limited to take <xi> to the boundary, <face_number> is set to be
 * the limiting face, <fraction> is updated with the fraction of the <increment>
 * actually used, the <increment> is updated to contain the part not used,
 * the <xi_face> are calculated for that face and the <xi> are changed to be
 * on the boundary of the shape.
 */
int FE_element_shape_xi_increment(struct FE_element_shape *shape,
	FE_value *xi, FE_value *increment, FE_value *step_size,
	int *face_number_address, FE_value *xi_face);

#endif /* !defined (FINITE_ELEMENT_SHAPE_HPP) */
