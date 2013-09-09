/*****************************************************************************//**
 * FILE : fieldmodule.h
 *
 * Public interface to the field module including its generic functions.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDMODULE_H__
#define CMZN_FIELDMODULE_H__

#include "types/regionid.h"
#include "types/fieldid.h"
#include "types/fieldmoduleid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Automatic scalar broadcast
 *
 * For field constructors (cmzn_field_module_create~ functions) which specify
 * the they apply automatic scalar broadcast for their source fields arguments,
 * if the one of the source fields has multiple components and the
 * other is a scalar, then the scalar will be automatically replicated so that
 * it matches the number of components in the multiple component field.
 * For example the result of
 * ADD(CONSTANT([1 2 3 4], CONSTANT([10]) is [11 12 13 14].
 */

/*
Global functions
----------------
*/

/***************************************************************************//**
 * Returns a new reference to the field module with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param field_module  The field module to obtain a new reference to.
 * @return  New field module reference with incremented reference count.
 */
ZINC_API cmzn_field_module_id cmzn_field_module_access(cmzn_field_module_id field_module);

/***************************************************************************//**
 * Destroys reference to the field module and sets pointer/handle to NULL.
 * Internally this just decrements the reference count.
 *
 * @param field_module_address  Address of field module reference.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_module_destroy(cmzn_field_module_id *field_module_address);

/***************************************************************************//**
 * Begin caching or increment cache level for this field module. Call this
 * function before making multiple changes to fields, nodes, elements etc. from
 * this field module to minimise number of change messages sent to clients.
 * Must call cmzn_field_module_end_change after making changes.
 * Note that field module changes are always cached when the region changes are
 * being cached.
 *
 * @see cmzn_region_begin_change
 * @param field_module  The field_module to begin change cache on.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_module_begin_change(cmzn_field_module_id field_module);

/***************************************************************************//**
 * Decrement cache level or end caching of changes for this field module.
 * Call cmzn_field_module_begin_change before making multiple changes
 * and call this afterwards. When change level is restored to zero,
 * cached change messages are sent out to clients.
 *
 * @param field_module  The field_module to end change cache on.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_module_end_change(cmzn_field_module_id field_module);

/***************************************************************************//**
 * Returns the field of the specified name from the field module.
 *
 * @param field_module  Region field module in which to find the field.
 * @param field_name  The name of the field to find.
 * @return  New reference to field of specified name, or NULL if not found.
 */
ZINC_API cmzn_field_id cmzn_field_module_find_field_by_name(
	cmzn_field_module_id field_module, const char *field_name);

/***************************************************************************//**
 * Create a field iterator object for iterating through the fields in the field
 * module, in alphabetical order of name. The iterator initially points at the
 * position before the first field, so the first call to
 * cmzn_field_iterator_next() returns the first field and advances the
 * iterator.
 * Iterator becomes invalid if fields are added, removed or renamed while in use.
 *
 * @param field_module  Handle to the field_module whose fields are to be
 * iterated over.
 * @return  Handle to field_iterator at position before first, or NULL if
 * error.
 */
ZINC_API cmzn_field_iterator_id cmzn_field_module_create_field_iterator(
	cmzn_field_module_id field_module);

/***************************************************************************//**
 * Defines, for all elements of all meshes in field module, face elements of
 * dimension one lower in the associated face mesh, and all their faces
 * recursively down to 1 dimensional lines.
 *
 * @param field_module  Handle to the field_module owning the meshes to define
 * faces for.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_module_define_all_faces(cmzn_field_module_id field_module);

/***************************************************************************//**
 * Gets the region this field module can create fields for.
 *
 * @param field_module  The field module to query.
 * @return  Accessed handle to owning region for field_module.
 */
ZINC_API cmzn_region_id cmzn_field_module_get_region(cmzn_field_module_id field_module);

#ifdef __cplusplus
}
#endif

#endif
