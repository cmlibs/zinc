/*******************************************************************************
FILE : element_operations.h

LAST MODIFIED : 3 March 2003

DESCRIPTION :
FE_element functions that utilise non finite element data structures and
therefore cannot reside in finite element modules.
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
#if !defined (ELEMENT_OPERATIONS_H)
#define ELEMENT_OPERATIONS_H

#include "zinc/fieldfiniteelement.h"
#include "computed_field/computed_field.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "general/multi_range.h"
#include "selection/element_point_ranges_selection.h"

/*
Global functions
----------------
*/

int FE_region_change_element_identifiers(struct FE_region *fe_region,
	int dimension,	int element_offset,
	struct Computed_field *sort_by_field, FE_value time);
/*******************************************************************************
LAST MODIFIED : 16 January 2003

DESCRIPTION :
Changes the identifiers of all elements of <cm_type> in <fe_region>.
If <sort_by_field> is NULL, adds <element_offset> to the identifiers.
If <sort_by_field> is specified, it is evaluated at the centre of all elements
in the group and the elements are sorted by it - changing fastest with the first
component and keeping the current order where the field has the same values.
Checks for and fails if attempting to give any of the elements in <fe_region> an
identifier already used by an element in the same master FE_region.
Calls to this function should be enclosed in FE_region_begin_change/end_change.
Note function avoids iterating through FE_region element lists as this is not
allowed during identifier changes.
==============================================================================*/

/***************************************************************************//**
 * Create an element list from the elements in mesh optionally restricted to
 * those within the element_ranges or where conditional_field is true in element
 * at time.
 *
 * @param mesh  Handle to the mesh.
 * @param element_ranges  Optional Multi_range of element identifiers.
 * @param conditional_field  Optional field interpreted as a boolean value which
 * must be true for an element from mesh to be included in list.
 * @param time  Time to evaluate the conditional_field at.
 * @return  The element list, or NULL on failure.
 */
struct LIST(FE_element) *Cmiss_mesh_get_selected_element_list(Cmiss_mesh_id mesh,
	struct Multi_range *element_ranges, struct Computed_field *conditional_field,
	FE_value time);

/***************************************************************************//**
 * Create an element list from those elements of dimension in the supplied
 * region, optionally restricted to any of the following conditions:
 * - element identifier is in the element_ranges;
 * - element is in the group_field;
 * - conditional field is true in the element;
 *
 * @param region  The pointer to a region
 * @param dimension  The dimension of elements to query about
 * @param element_ranges  Multi_range of elements.
 * @param group_field  Group field of the region
 * @param conditional_field  Optional field interpreted as a boolean value which
 * must be true for an element from mesh to be included in list.
 * @param time  Time to evaluate the conditional_field at.
 * @return  The element list, or NULL on failure.
 */
struct LIST(FE_element) *FE_element_list_from_region_and_selection_group(
	struct Cmiss_region *region, int dimension,
	struct Multi_range *element_ranges, struct Computed_field *group_field,
	struct Computed_field *conditional_field, FE_value time);

/***************************************************************************//**
 * Create points in gauss_points_nodeset with embedded locations and weights
 * matching the Gauss quadrature points in all elements of mesh.
 * Supports all main element shapes up to order 4.
 *
 * @param mesh  The mesh to create gauss points for.
 * @param order  The 1-D polynomial order from 1 to 4, which for line shapes
 * gives that number of Gauss points per element dimension.
 * @param gauss_points_nodeset  The nodeset to create gauss points in.
 * @param first_identifier  The minimum identifier to use for the gauss points.
 * @param gauss_location_field  Field to define at gauss points for storing
 * gauss point element_xi location.
 * @param gauss_weight_field  Scalar field to define at gauss points for storing
 * real Gauss point weight.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_mesh_create_gauss_points(Cmiss_mesh_id mesh, int order,
	Cmiss_nodeset_id gauss_points_nodeset, int first_identifier,
	Cmiss_field_stored_mesh_location_id gauss_location_field,
	Cmiss_field_finite_element_id gauss_weight_field);

#endif /* !defined (ELEMENT_OPERATIONS_H) */
