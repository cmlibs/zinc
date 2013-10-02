/*******************************************************************************
FILE : finite_element_discretization.h

LAST MODIFIED : 12 October 2001

DESCRIPTION :
Functions for discretizing finite elements into points and simple sub-domains.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (FINITE_ELEMENT_DISCRETIZATION_H)
#define FINITE_ELEMENT_DISCRETIZATION_H

#include "computed_field/computed_field.h"
#include "finite_element/finite_element.h"
#include "graphics/auxiliary_graphics_types.h"

int FE_element_shape_get_xi_points_cell_centres(
	struct FE_element_shape *element_shape, int *number_in_xi,
	int *number_of_xi_points_address, FE_value_triple **xi_points_address);
/*******************************************************************************
LAST MODIFIED : 20 April 2001

DESCRIPTION :
Calculates the <number_of_xi_points> in the centres of uniform cells across the
<element_shape> according to <number_in_xi>.
If <xi_points_address> is supplied an array containing the xi locations of these
points is allocated and put in this address. Xi positions are always returned as
triples with remaining xi coordinates 0 for 1-D and 2-D cases.
Note the actual number and layout is dependent on the <element_shape>; see
comments for simplex and polygons shapes for more details.
==============================================================================*/

int FE_element_shape_get_xi_points_cell_corners(
	struct FE_element_shape *element_shape, int *number_in_xi,
	int *number_of_xi_points_address, FE_value_triple **xi_points_address);
/*******************************************************************************
LAST MODIFIED : 20 April 2001

DESCRIPTION :
Calculates the <number_of_xi_points> in the corners of uniform cells across the
<element_shape> according to <number_in_xi>.
If <xi_points_address> is supplied an array containing the xi locations of these
points is allocated and put in this address. Xi positions are always returned as
triples with remaining xi coordinates 0 for 1-D and 2-D cases.
Note the actual number and layout is dependent on the <element_shape>; see
comments for simplex and polygons shapes for more details.
==============================================================================*/

int FE_element_shape_get_indices_for_xi_location_in_cell_corners(
	struct FE_element_shape *element_shape, int *number_in_xi,
	const FE_value *xi, int *indices);
/*******************************************************************************
LAST MODIFIED : 18 October 2005

DESCRIPTION :
Determines if <xi> cooresponds to a location on the corners of uniform cells
across the <element_shape> according to <number_in_xi>.
If so, returns 1 and sets the <indices> to match.
Otherwise the routine returns 0.
==============================================================================*/

int FE_element_get_xi_points(struct FE_element *element,
	cmzn_element_point_sample_mode sample_mode,
	int *number_in_xi, FE_value_triple exact_xi, cmzn_fieldcache_id field_cache,
	struct Computed_field *coordinate_field, struct Computed_field *density_field,
	int *number_of_xi_points_address, FE_value_triple **xi_points_address);
/*******************************************************************************
LAST MODIFIED : 20 April 2001

DESCRIPTION :
Calculates the <number_of_xi_points> across the <element_shape> according to
the <sample_mode> and some of <number_in_xi>, <exact_xi>,
<coordinate_field> and <density_field>, depending on the mode.
If <xi_points_address> is supplied an array containing the xi locations of these
points is allocated and put in this address. Xi positions are always returned as
triples with remaining xi coordinates 0 for 1-D and 2-D cases.
<exact_xi> should be supplied for sample mode SET_LOCATION - although it
is trivial, it is passed and used here to provide a consistent interface.]
@param field_cache  cmzn_fieldcache for evaluating coordinate and density
fields, required for DENSITY and POISSON modes. Time is expected to have been
set in the field_cache if needed.
==============================================================================*/

int FE_element_convert_xi_points_cell_corners_to_top_level(
	struct FE_element *element, struct FE_element *top_level_element,
	int *top_level_number_in_xi, int number_of_xi_points, FE_value_triple *xi_points,
	int **top_level_xi_point_numbers_address);
/*******************************************************************************
LAST MODIFIED : 23 April 2001

DESCRIPTION :
If the element is a face or line of a cube or square top-level element, converts
the <number_of_xi_points> <xi_points> to be locations in the top-level element.
Also allocates the *<top_level_xi_point_numbers_address> to contain the
appropriate xi_point_numbers relative to the top-level element.
Notes:
1. The xi_points put into this function must have been calculated with the
CMZN_ELEMENT_POINT_SAMPLE_CELL_CORNERS more and the number_in_xi determined from the
relation from <element> to <top_level_element> and its <top_level_number_in_xi>.
2. Sets *<top_level_xi_point_numbers_address> to NULL if not ALLOCATED; hence
a return value here indicates that the xi_points have been converted.
==============================================================================*/

int FE_element_get_numbered_xi_point(struct FE_element *element,
	cmzn_element_point_sample_mode sample_mode,
	int *number_in_xi, FE_value_triple exact_xi, cmzn_fieldcache_id field_cache,
	struct Computed_field *coordinate_field, struct Computed_field *density_field,
	int xi_point_number, FE_value *xi);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Returns in <xi> the location of the <xi_point_number> out of those that would
be calculated by FE_element_get_xi_points. The default_behaviour is to
Call the above function, extract the xi location and DEALLOCATE the xi_points.
This is quite expensive; for this reason cell_centres and cell_corners in line,
square and cube elements, as well as exact_xi, are handled separately since the
calculation is trivial.
@param field_cache  cmzn_fieldcache for evaluating coordinate and density
fields, required for DENSITY and POISSON modes. Time is expected to be set in
the field_cache if needed.
==============================================================================*/

int convert_xi_points_from_element_to_parent(int number_of_xi_points,
	FE_value_triple *xi_points,int element_dimension,int parent_dimension,
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

#endif /* !defined (FINITE_ELEMENT_DISCRETIZATION_H) */
