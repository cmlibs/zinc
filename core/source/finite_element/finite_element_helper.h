/***************************************************************************//**
 * FILE : finite_element_helper.h
 *
 * Convenience functions for making simple finite element fields.
 */
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
 * Portions created by the Initial Developer are Copyright (C) 2009
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
#if !defined (FINITE_ELEMENT_HELPER_H)
#define FINITE_ELEMENT_HELPER_H

/***************************************************************************//**
 * Convenience function for finding or creating a 3-component coordinate
 * FE_field with rectangular cartesian coordinate system and component names
 * "x", "y" and "z", suitable for nodal interpolation.
 *
 * @param fe_region  The region this field is found or created in.
 * @param name  The name of the field. Suggestion: "coordinates".
 * @return  The field with incremented access count, or NULL if incompatible
 * field of same name already exists in fe_region.
 */
FE_field *FE_field_create_coordinate_3d(struct FE_region* fe_region,
	const char *name);

/***************************************************************************//**
 * Creates an element with a simplex shape: triangle or tetrahedron.
 * The returned element is for fe_region but not yet merged into it, suitable
 * for use as a template for creating further elements in the region.
 * Caller generally needs to establish the number of nodes and define fields
 * before using it as a template. Use DEACCESS to clean up reference to element.
 * 
 * @param fe_region  The region the element is created for use with.
 * @param dimension  Dimension: Must be 2 for triangle, 3 for tetrahedron.
 * @return  The element with access count of 1, or NULL if failed.
 */
struct FE_element *FE_element_create_with_simplex_shape(
	struct FE_region *fe_region, int dimension);

/***************************************************************************//**
 * Defines a nodal interpolated field over element using the supplied basis
 * function for all field components on each element coordinate dimension.
 * Local nodes are assumed to be listed by the element in the order expected by
 * the element basis function, i.e. increasing in xi1 fastest, followed by xi2,
 * etc.
 * Number of nodes must have been set and sufficient for the chosen basis.
 * Scale factors are not used by parameter maps set up with this function; all
 * default to unit value == 1.0.
 * Warning: currently cannot support mixing bases with different node numbers
 * or arrangements.
 * 
 * @param element to define the field on.
 * @param field  The field to define.
 * @param basis_type  The basis function type used in each element coordinate
 * dimension, one of LINEAR_LAGRANGE, QUADRATIC_LAGRANGE, CUBIC_LAGRANGE,
 * LINEAR_SIMPLEX or QUADRATIC_SIMPLEX. SIMPLEX bases are only supported for
 * elements of dimension 2 or 3.
 * @return  1 on success, 0 on failure.
 */
int FE_element_define_field_simple(struct FE_element *element,
	struct FE_field *field, enum FE_basis_type basis_type);

#endif /* FINITE_ELEMENT_HELPER_H */
