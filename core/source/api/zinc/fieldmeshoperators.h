/**
 * @file fieldmeshoperators.h
 *
 * Implements field operators that sum or process fields over a mesh.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDMESHOPERATORS_H__
#define CMZN_FIELDMESHOPERATORS_H__

#include "types/fieldid.h"
#include "types/fieldmoduleid.h"
#include "types/elementid.h"
#include "types/fieldmeshoperatorsid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates a field which calculates the integral over the mesh of 
 * integrand.dV for volume in 3-D, dA for area in 2-D, dL for length in 1-D.
 * Returned field has same number of components as the integrand field,
 * with each component integrated separately.
 * The integral is calculated by Gauss Quadrature with the order i.e. the
 * number of Gauss points in each element dimension settable by type-specific
 * API. The default order of 1 is adequate only for linear fields.
 * Note: assumes all elements of the mesh have a right-handed coordinate
 * system; if this is not the case the integral will be incorrect.
 *
 * @param field_module  Region field module which will own the new field.
 * @param integrand_field  Field to integrate. An integrand of constant value 1
 * gives the volume/area/length of the mesh, i.e. sum for each element.
 * @param coordinate_field  Field supplying coordinates; must have at least as
 * many components as the mesh, up to a maximum of 3. It is up to the user to
 * supply a field giving values in a Rectangular Cartesian coordinate system,
 * using a coordinate transformation field conversion if needed.
 * @param mesh  The mesh to integrate over.
 * @return  Handle to new field, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_mesh_integral(
	cmzn_fieldmodule_id field_module, cmzn_field_id integrand_field,
	cmzn_field_id coordinate_field, cmzn_mesh_id mesh);

/**
 * If the field is a mesh integral type field, return the derived field handle.
 *
 * @param field  The field to be cast.
 * @return  Handle to derived mesh integral field, or NULL/invalid handle if
 * wrong type or failed.
 */
ZINC_API cmzn_field_mesh_integral_id cmzn_field_cast_mesh_integral(cmzn_field_id field);

/**
 * Cast mesh integral field back to its base field and return the field.
 * IMPORTANT NOTE: Returned field does not have incremented reference count and
 * must not be destroyed. Use cmzn_field_access() to add a reference if
 * maintaining returned handle beyond the lifetime of the derived field.
 * Use this function to call base-class API, e.g.:
 * cmzn_field_set_name(cmzn_field_derived_base_cast(derived_field), "bob");
 *
 * @param mesh_integral_field  Handle to the mesh integral field to cast.
 * @return  Non-accessed handle to the base field or NULL if failed.
 */
ZINC_C_INLINE cmzn_field_id cmzn_field_mesh_integral_base_cast(
	cmzn_field_mesh_integral_id mesh_integral_field)
{
	return (cmzn_field_id)(mesh_integral_field);
}

/**
 * Destroys handle to the mesh integral field (and sets it to NULL).
 * Internally this decrements the reference count.
 *
 * @param mesh_integral_field_address  Address of handle to the field to
 * destroy.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_mesh_integral_destroy(
	cmzn_field_mesh_integral_id *mesh_integral_field_address);

/**
 * Set the order of integration i.e. number of Gauss points in each element
 * axis. Typically this needs to be at least equal to the polynomial order
 * of the basis function: 1 for linear, 2 for quadratic etc.
 *
 * @param mesh_integral_field  Handle to the mesh integral field to cast.
 * @param order  The order of integration or number of points, from 1 to 4.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_field_mesh_integral_set_order(
	cmzn_field_mesh_integral_id mesh_integral_field, int order);

#ifdef __cplusplus
}
#endif

#endif
