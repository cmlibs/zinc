/**
 * @file fieldderivatives.h
 *
 * The public interface to the Zinc fields that evaluate derivatives.
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDDERIVATIVES_H__
#define CMZN_FIELDDERIVATIVES_H__

#include "types/fieldid.h"
#include "types/fieldderivativesid.h"
#include "types/fieldmoduleid.h"

#include "opencmiss/zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates a field returning the derivative of the source field with respect to
 * the chosen element xi coordinate index as its primary value. The xi index is
 * relative to the element/face/line it is evaluated on, not the top-level
 * element. Returned field has same number of components as the source field.
 *
 * @param fieldmodule  Region field module which will own new field.
 * @param source_field  Source field to get derivative number.
 * @param xi_index  Element coordinate system index for derivative, from 1 to
 * element dimension.
 * @return  Handle to new field, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_derivative(
	cmzn_fieldmodule_id fieldmodule, cmzn_field_id source_field, int xi_index);

/**
 * Cast field to derivative type if valid.
 *
 * @param field  The field to cast.
 * @return  Handle to derived derivative field, or NULL/invalid handle if
 * wrong type or failed.
 */
ZINC_API cmzn_field_derivative_id cmzn_field_cast_derivative(cmzn_field_id field);

/**
 * Cast derivative field back to its base field and return the field.
 * IMPORTANT NOTE: Returned field does not have incremented reference count and
 * must not be destroyed. Use cmzn_field_access() to add a reference if
 * maintaining returned handle beyond the lifetime of the derived field.
 * Use this function to call base-class API, e.g.:
 * cmzn_field_set_name(cmzn_field_derived_base_cast(derived_field), "bob");
 *
 * @param derivative_field  Handle to the derivative field to cast.
 * @return  Non-accessed handle to the base field or NULL if failed.
 */
ZINC_C_INLINE cmzn_field_id cmzn_field_derivative_base_cast(
	cmzn_field_derivative_id derivative_field)
{
	return (cmzn_field_id)(derivative_field);
}

/**
 * Destroys handle to the derivative field (and sets it to NULL).
 * Internally this decrements the reference count.
 *
 * @param derivative_field_address  Address of handle to the field to
 * destroy.
 * @return  Result OK on success, otherwise ERROR_ARGUMENT.
 */
ZINC_API int cmzn_field_derivative_destroy(
	cmzn_field_derivative_id *derivative_field_address);

/**
 * Get the element xi coordinate index the derivative is with respect to.
 *
 * @param derivative_field  Handle to the derivative field to query.
 * @return  Xi index >= 1, or 0 if invalid field.
 */
ZINC_API int cmzn_field_derivative_get_xi_index(
	cmzn_field_derivative_id derivative_field);

/**
 * Set the element xi coordinate index the derivative is with respect to.
 *
 * @param derivative_field  Handle to the derivative field to modify.
 * @param xi_index  Element coordinate system index for derivative, from 1 to
 * element dimension.
 * @return  Result OK on success, otherwise ERROR_ARGUMENT.
 */
ZINC_API int cmzn_field_derivative_set_xi_index(
	cmzn_field_derivative_id derivative_field, int xi_index);

/**
 * Creates a field returning the curl of vector_field at location given by
 * coordinate_field. All fields including return field have 3 components.
 *
 * @param field_module  Region field module which will own new field.
 * @param vector_field  Vector field from which curl is evaluated. Must have
 * rectangular cartesian coordinate system.
 * @param coordinate_field  Field supplying location.
 * @return  Handle to new field, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_curl(
	cmzn_fieldmodule_id field_module,
	cmzn_field_id vector_field, cmzn_field_id coordinate_field);

/**
 * Creates a scalar field returning the divergence of vector field within
 * coordinate field.
 * The number of components of <vector_field> and <coordinate_field> must be the
 * same and less than or equal to 3.
 *
 * @param field_module  Region field module which will own new field.
 * @param vector_field  Vector field from which divergence is evaluated. Must
 * have rectangular cartesian coordinate system.
 * @param coordinate_field  Field supplying location.
 * @return  Handle to new field, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_divergence(
	cmzn_fieldmodule_id field_module,
	cmzn_field_id vector_field, cmzn_field_id coordinate_field);

/**
 * Creates a field returning the gradient of a source field with respect to
 * a given coordinate field.
 * Evaluation is only possible in elements of dimension equal or less than the
 * number of components in the coordinate field. If the element dimension is
 * lower, ficticious orthogonal xi axes are defined so that the derivative of
 * coordinates w.r.t. xi is square and hence invertable.
 * The number of components of the new field is the product of the numbers of
 * components in the source and coordinate fields.
 * If the source field has multiple components, all the derivatives of its
 * first component w.r.t. the coordinate components are given first, followed
 * by the second component, etc. Hence, this function can return the standard
 * gradient of a scalar source field, and the deformation gradient if a
 * deformed coordinate field is passed as the source field.
 * The gradient can also be calculated at nodes, albeit approximately using a
 * finite difference approach by perturbing the coordinate field.
 *
 * @param fieldmodule  Region field module which will own new field.
 * @param source_field  Field to calculate gradient of.
 * @param coordinate_field  Field supplying coordinate location over which
 * the source field is expected to vary, with up to 3 real components.
 * @return  Handle to new field, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_gradient(
	cmzn_fieldmodule_id fieldmodule,
	cmzn_field_id source_field, cmzn_field_id coordinate_field);

#ifdef __cplusplus
}
#endif

#endif /* CMZN_FIELDDERIVATIVES_H__ */
