/**
 * @file fieldrange.h
 *
 * The public interface to the zinc field range.
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDRANGE_H__
#define CMZN_FIELDRANGE_H__

#include "types/elementid.h"
#include "types/fieldrangeid.h"
#include "types/fieldid.h"

#include "opencmiss/zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns a new handle to the field range with reference count incremented.
 *
 * @param fieldrange  The field range to obtain a new handle to.
 * @return  New handle to field range, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_fieldrange_id cmzn_fieldrange_access(cmzn_fieldrange_id fieldrange);

/**
 * Destroys handle to the field range, and sets it to NULL.
 * Internally this decrements the reference count.
 *
 * @param fieldrange_address  Address of handle to field range to destroy.
 * @return  Result OK on success, otherwise ERROR_ARGUMENT.
 */
ZINC_API int cmzn_fieldrange_destroy(cmzn_fieldrange_id *fieldrange_address);

/**
 * Get the element and element local coordinates of the point where the
 * minimum value for the component was found, if found on a mesh.
 *
 * @param fieldrange  The field range to query.
 * @param componentNumber  The component of the field starting at 1.
 * @param coordinatesCount  The size of the coordinatesOut array, must be at
 * least element dimension, or 3 for any supported dimension.
 * @param coordinatesOut  Array to receive element local xi coordinates.
 * @return  Handle to element on success with coordinatesOut filled, or
 * NULL/invalid element handle if no range or not found at a mesh location.
 */
ZINC_API cmzn_element_id cmzn_fieldrange_get_component_minimum_mesh_location(
	cmzn_fieldrange_id fieldrange, int componentNumber,
	int coordinatesCount, double *coordinatesOut);

/**
 * Get the element and element local coordinates of the point where the
 * maximum value for the component was found, if found on a mesh.
 *
 * @param fieldrange  The field range to query.
 * @param componentNumber  The component of the field starting at 1.
 * @param coordinatesCount  The size of the coordinatesOut array, must be at
 * least element dimension, or 3 for any supported dimension.
 * @param coordinatesOut  Array to receive element local xi coordinates.
 * @return  Handle to element on success with coordinatesOut filled, or
 * NULL/invalid element handle if no range or not found at a mesh location.
 */
ZINC_API cmzn_element_id cmzn_fieldrange_get_component_maximum_mesh_location(
	cmzn_fieldrange_id fieldrange, int componentNumber,
	int coordinatesCount, double *coordinatesOut);

/**
 * Get the field values at the point where the minimum value of the component
 * of the field was found.
 *
 * @param fieldrange  The field range to query.
 * @param componentNumber  The component of the field starting at 1.
 * @param valuesCount  The size of the valuesOut array, must be at
 * least the number of components of the field the range is for.
 * @param valuesOut  Array to receive field values.
 * @return  Result OK on success, ERROR_NOT_FOUND if no valid range,
 * ERROR_ARGUMENT if invalid arguments supplied.
 */
ZINC_API int cmzn_fieldrange_get_component_minimum_values_real(
	cmzn_fieldrange_id fieldrange, int componentNumber,
	int valuesCount, double *valuesOut);

/**
 * Get the field values at the point where the maximum value of the component
 * of the field was found.
 *
 * @param fieldrange  The field range to query.
 * @param componentNumber  The component of the field starting at 1.
 * @param valuesCount  The size of the valuesOut array, must be at
 * least the number of components of the field the range is for.
 * @param valuesOut  Array to receive field values.
 * @return  Result OK on success, ERROR_NOT_FOUND if no valid range,
 * ERROR_ARGUMENT if invalid arguments supplied.
 */
ZINC_API int cmzn_fieldrange_get_component_maximum_values_real(
	cmzn_fieldrange_id fieldrange, int componentNumber,
	int valuesCount, double *valuesOut);

/**
 * Return the field the range was calculated for.
 *
 * @param fieldrange  The field range to query.
 * @return  Handle to field the range is for, or NULL/invalid handle if no
 * range has been calculated or the supplied range is NULL/invalid.
 */
ZINC_API cmzn_field_id cmzn_fieldrange_get_field(cmzn_fieldrange_id fieldrange);

/**
 * Get the minimum and maximum values of the field on the domain it was
 * evaluated over.
 *
 * @param fieldrange  The field range to query.
 * @param valuesCount  The sizes of the values arrays to fill >= the number
 * of components of the field.
 * @param minimumValuesOut  Array to receive minimum values.
 * @param maximumValuesOut  Array to receive maximum values.
 * @return  Result OK on success, ERROR_NOT_FOUND if no valid range,
 * ERROR_ARGUMENT if invalid arguments supplied.
 */
ZINC_API int cmzn_fieldrange_get_range_real(cmzn_fieldrange_id fieldrange,
	int valuesCount, double *minimumValuesOut, double *maximumValuesOut);

/**
 * Return whether a valid range is held.
 *
 * @param fieldrange  The field range to query.
 * @return  Boolean true if range is valid otherwise false.
 */
ZINC_API bool cmzn_fieldrange_has_valid_range(cmzn_fieldrange_id fieldrange);

#ifdef __cplusplus
}
#endif

#endif /* CMZN_FIELDRANGE_H__ */
