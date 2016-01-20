/***************************************************************************//**
 * FILE : finite_element_helper.h
 *
 * Convenience functions for making simple finite element fields.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
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
