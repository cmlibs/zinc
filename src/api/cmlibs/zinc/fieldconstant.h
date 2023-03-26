/**
 * @file fieldconstant.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDCONSTANT_H__
#define CMZN_FIELDCONSTANT_H__

#include "types/fieldid.h"
#include "types/fieldconstantid.h"
#include "types/fieldmoduleid.h"

#include "opencmiss/zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates a field with the components specified in the array values.
 * Internally this a composite field.
 *
 * @param field_module  Region field module which will own new field.
 * @param number_of_values  The number of values in the array.
 * @param values The array of constant values
 * @return  Handle to new field, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_constant(cmzn_fieldmodule_id field_module,
	int number_of_values, const double *values);

/**
 * If the field is constant type, return type-specific handle to it.
 *
 * @param field  The field to be cast.
 * @return  Handle to derived constant field, or NULL/invalid handle if wrong type or failed.
 */
ZINC_API cmzn_field_constant_id cmzn_field_cast_constant(cmzn_field_id field);

/**
 * Cast constant field back to its base field and return the field.
 * IMPORTANT NOTE: Returned field does not have incremented reference count and
 * must not be destroyed. Use cmzn_field_access() to add a reference if
 * maintaining returned handle beyond the lifetime of the derived field.
 * Use this function to call base-class API, e.g.:
 * cmzn_field_set_name(cmzn_field_derived_base_cast(derived_field), "bob");
 *
 * @param constant_field  Handle to the constant field to cast.
 * @return  Non-accessed handle to the base field or NULL if failed.
 */
ZINC_C_INLINE cmzn_field_id cmzn_field_constant_base_cast(
	cmzn_field_constant_id constant_field)
{
	return (cmzn_field_id)(constant_field);
}

/**
 * Creates a string constant field with the supplied
 * string value in <string_constant>.
 *
 * @param field_module  Region field module which will own new field.
 * @param string_constant The constant char string.
 * @return  Handle to new field, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_string_constant(cmzn_fieldmodule_id field_module,
	const char *string_constant);

/**
 * If the field is string constant type, return type-specific handle to it.
 *
 * @param field  The field to be cast.
 * @return  Handle to derived string constant field, or NULL/invalid handle if wrong type or failed.
 */
ZINC_API cmzn_field_string_constant_id cmzn_field_cast_string_constant(cmzn_field_id field);

/**
 * Cast string constant field back to its base field and return the field.
 * IMPORTANT NOTE: Returned field does not have incremented reference count and
 * must not be destroyed. Use cmzn_field_access() to add a reference if
 * maintaining returned handle beyond the lifetime of the derived field.
 * Use this function to call base-class API, e.g.:
 * cmzn_field_set_name(cmzn_field_derived_base_cast(derived_field), "bob");
 *
 * @param string_constant_field  Handle to the string constant field to cast.
 * @return  Non-accessed handle to the base field or NULL if failed.
 */
ZINC_C_INLINE cmzn_field_id cmzn_field_string_constant_base_cast(
	cmzn_field_string_constant_id string_constant_field)
{
	return (cmzn_field_id)(string_constant_field);
}

#ifdef __cplusplus
}
#endif

#endif // CMZN_FIELDCONSTANT_H
