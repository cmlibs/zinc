/**
 * @file fieldparameters.h
 *
 * Public interface to parameters of a field.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDPARAMETERS_H__
#define CMZN_FIELDPARAMETERS_H__

#include "types/fieldparametersid.h"
#include "types/fieldid.h"

#include "opencmiss/zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get field parameters object for field.
 * Currently only supported by finite element field type. 
 *
 * @param field  The field to get parameters object for.
 * @return  Handle to new field parameters object, or NULL/invalid handle on
 * failure or if not supported by field type.
 */
ZINC_API cmzn_fieldparameters_id cmzn_field_get_fieldparameters(
	cmzn_field_id field);

/**
 * Returns a new handle to the field parameters with reference count
 * incremented.
 *
 * @param fieldparameters  The fieldparameters to obtain a new handle to.
 * @return  New handle to field parameters object, or NULL/invalid handle on
 * failure.
 */
ZINC_API cmzn_fieldparameters_id cmzn_fieldparameters_access(
	cmzn_fieldparameters_id fieldparameters);

/**
 * Destroys handle to the field parameters (and sets it to NULL).
 * Internally this decrements the reference count.
 *
 * @param fieldparametersAddress  Address of handle to field parameters.
 * @return  Result OK on success, any other value on failure.
 */
ZINC_API int cmzn_fieldparameters_destroy(
	cmzn_fieldparameters_id *fieldparametersAddress);

/**
* Get the field these field parameters are for.
*
* @param fieldparameters  The field parameters object to query.
* @return  Handle to field, or NULL/invalid handle if invalid field
* parameters object.
*/
ZINC_API cmzn_field_id cmzn_fieldparameters_get_field(
	cmzn_fieldparameters_id fieldparameters);

/**
 * Return the number of parameters for the field.
 *
 * @param fieldparameters  The field parameters object to query.
 * @return  Number of field parameters or -1 on error.
 */
ZINC_API int cmzn_fieldparameters_get_number_of_parameters(
	cmzn_fieldparameters_id fieldparameters);

#ifdef __cplusplus
}
#endif

#endif /* CMZN_FIELDPARAMETERS_H__ */
