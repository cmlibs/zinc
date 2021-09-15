/**
 * @file fieldapply.h
 *
 * Defines fields for applying the function of other fields, including from
 * other regions, with argument binding.
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDAPPLY_H__
#define CMZN_FIELDAPPLY_H__

#include "types/fieldid.h"
#include "types/fieldapplyid.h"
#include "types/fieldmoduleid.h"

#include "opencmiss/zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates a zinc field which applies the function of the supplied field,
 * optionally binding argument fields it depends on to other source fields.
 * This is the main mechanism for reusing field definitions from other
 * regions.
 *
 * @param fieldmodule  Region field module which will own new field.
 * @param evaluate_field  The field which this field will evaluate. Can be from
 * a different region.
 * @return  Handle to new field, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_apply(
	cmzn_fieldmodule_id fieldmodule, cmzn_field_id evaluate_field);

/**
 * If the field is apply type, return type-specific handle to it.
 *
 * @param field  The field to be cast.
 * @return  Handle to derived constant field, or NULL/invalid handle if wrong
 * type or failed.
 */
ZINC_API cmzn_field_apply_id cmzn_field_cast_apply(cmzn_field_id field);

/**
 * Destroys handle to the apply field (and sets it to NULL).
 * Internally this decrements the reference count.
 *
 * @param apply_address  Address of handle to the apply field.
 * @return  Result OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_apply_destroy(cmzn_field_apply_id *apply_address);

/**
 * Get the argument field at bind index.
 *
 * @param apply_field  The apply field to query.
 * @param bind_index  The index from 1 to number of bindings.
 * @return  Handle to argument field, or null/invalid handle if invalid
 * arguments.
 */
ZINC_API cmzn_field_id cmzn_field_apply_get_bind_argument_field(
	cmzn_field_apply_id apply_field, int bind_index);

/**
 * Get the source field bound to an argument field.
 *
 * @param apply_field  The apply field to query.
 * @param argument_field  The argument field to get the bound source field for.
 * Must be a real Argument field from the same region as the Apply field's
 * evaluate field.
 * @return  Handle to source field if set, or null/invalid handle if none or
 * invalid argument.
 */
ZINC_API cmzn_field_id cmzn_field_apply_get_bind_argument_source_field(
	cmzn_field_apply_id apply_field, cmzn_field_id argument_field);

/**
 * Set the source field bound to an argument field.
 *
 * @param apply_field  The apply field to modify.
 * @param argument_field  The argument field to set the bound source field for.
 * Must be a real Argument field from the same region as the Apply field's
 * evaluate field.
 * @param source_field  The source field to bind to the argument field within
 * this apply field, or pass null/invalid field to clear. Must be from the same
 * region as the apply field and match the argument in value type and number of
 * components. The source field cannot depend on the apply field, its evaluate
 * field or the argument field, directly or indirectly including through other
 * argument bindings, to avoid infinite evaluation cycles.
 * @return  Result OK on success, ERROR_ARGUMENT if invalid apply, argument or
 * source field, otherwise some other error.
 */
ZINC_API int cmzn_field_apply_set_bind_argument_source_field(
	cmzn_field_apply_id apply_field, cmzn_field_id argument_field,
	cmzn_field_id source_field);

/**
 * Get number of argument-source field bindings applied.
 *
 * @param apply_field  The apply field to query.
 * @return  Number of bindings or -1 if invalid arguments.
 */
ZINC_API int cmzn_field_apply_get_number_of_bindings(cmzn_field_apply_id apply_field);

/**
 * Creates a field representing a multi-component real-valued argument to other
 * fields defining reusable operators acting on it. Works exclusively with an
 * Apply field which can evaluate the operator field by binding all arguments.
 * The Argument field type delegates its evaluation to the field currently
 * bound to it in the field cache.
 *
 * @param fieldmodule  Region field module which will own new field.
 * @param number_of_components  The number of real components of the field.
 * @return  Handle to new field, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_argument_real(
	cmzn_fieldmodule_id fieldmodule, int number_of_components);

/**
 * If the field is argument real type, return type-specific handle to it.
 *
 * @param field  The field to be cast.
 * @return  Handle to derived constant field, or NULL/invalid handle if wrong
 * type or failed.
 */
ZINC_API cmzn_field_argument_real_id cmzn_field_cast_argument_real(
	cmzn_field_id field);

/**
 * Destroys handle to the argument_real field (and sets it to NULL).
 * Internally this decrements the reference count.
 *
 * @param argument_real_address  Address of handle to the argument_real field.
 * @return  Result OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_argument_real_destroy(
	cmzn_field_argument_real_id *argument_real_address);



#ifdef __cplusplus
}
#endif

#endif /* #ifndef CMZN_FIELDAPPLY_H */
