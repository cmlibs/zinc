/**
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
#include "types/fieldcacheid.h"
#include "types/fieldid.h"
#include "types/fieldmoduleid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Automatic scalar broadcast
 *
 * For field constructors (cmzn_fieldmodule_create~ functions) which specify
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

/**
 * Returns a new reference to the field module with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param fieldmodule  The field module to obtain a new reference to.
 * @return  New field module reference with incremented reference count.
 */
ZINC_API cmzn_fieldmodule_id cmzn_fieldmodule_access(cmzn_fieldmodule_id fieldmodule);

/**
 * Destroys reference to the field module and sets pointer/handle to NULL.
 * Internally this just decrements the reference count.
 *
 * @param fieldmodule_address  Address of field module reference.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_fieldmodule_destroy(cmzn_fieldmodule_id *fieldmodule_address);

/**
 * Begin caching or increment cache level for this field module. Call this
 * function before making multiple changes to fields, nodes, elements etc. from
 * this field module to minimise number of change messages sent to clients.
 * Must call cmzn_fieldmodule_end_change after making changes.
 * Note that field module changes are always cached when the region changes are
 * being cached.
 *
 * @see cmzn_region_begin_change
 * @param fieldmodule  The field module to begin change cache on.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_fieldmodule_begin_change(cmzn_fieldmodule_id fieldmodule);

/**
 * Decrement cache level or end caching of changes for this field module.
 * Call cmzn_fieldmodule_begin_change before making multiple changes
 * and call this afterwards. When change level is restored to zero,
 * cached change messages are sent out to clients.
 *
 * @param fieldmodule  The field module to end change cache on.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_fieldmodule_end_change(cmzn_fieldmodule_id fieldmodule);

/**
 * Returns the field of the specified name from the field module.
 *
 * @param fieldmodule  Region field module in which to find the field.
 * @param field_name  The name of the field to find.
 * @return  New reference to field of specified name, or NULL if not found.
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_find_field_by_name(
	cmzn_fieldmodule_id fieldmodule, const char *field_name);

/**
 * Creates a field cache for storing a known location and field values and
 * derivatives at that location. Required to evaluate and assign field values.
 *
 * @param fieldmodule  The field module to create a field cache for.
 * @return  New field cache, or NULL if failed.
 */
ZINC_API cmzn_fieldcache_id cmzn_fieldmodule_create_fieldcache(
	cmzn_fieldmodule_id fieldmodule);

/**
 * Create a field iterator object for iterating through the fields in the field
 * module, in alphabetical order of name. The iterator initially points at the
 * position before the first field, so the first call to
 * cmzn_fielditerator_next() returns the first field and advances the
 * iterator.
 * Iterator becomes invalid if fields are added, removed or renamed while in use.
 *
 * @param fieldmodule  Handle to the field module whose fields are to be
 * iterated over.
 * @return  Handle to field_iterator at position before first, or NULL if
 * error.
 */
ZINC_API cmzn_fielditerator_id cmzn_fieldmodule_create_fielditerator(
	cmzn_fieldmodule_id fieldmodule);

/**
 * Create a notifier for getting callbacks for changes to the fields and related
 * objects in the field module.
 *
 * @param fieldmodule  Handle to the field module to get notifications for.
 * @return  On success, handle to field module notifier, otherwise NULL.
 * Up to caller to destroy handle.
 */
ZINC_API cmzn_fieldmodulenotifier_id cmzn_fieldmodule_create_fieldmodulenotifier(
	cmzn_fieldmodule_id fieldmodule);

/**
 * Defines, for all elements of all meshes in field module, face elements of
 * dimension one lower in the associated face mesh, and all their faces
 * recursively down to 1 dimensional lines.
 *
 * @param fieldmodule  Handle to the field module owning the meshes to define
 * faces for.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_fieldmodule_define_all_faces(cmzn_fieldmodule_id fieldmodule);

/**
 * Gets the region this field module can create fields for.
 *
 * @param fieldmodule  The field module to query.
 * @return  Accessed handle to owning region for field module.
 */
ZINC_API cmzn_region_id cmzn_fieldmodule_get_region(cmzn_fieldmodule_id fieldmodule);

/**
 * Returns a new reference to the field module notifier with reference count
 * incremented. Caller is responsible for destroying the new reference.
 *
 * @param notifier  The field module notifier to obtain a new reference to.
 * @return  Handle to field module notifier with incremented reference count.
 */
ZINC_API cmzn_fieldmodulenotifier_id cmzn_fieldmodulenotifier_access(
	cmzn_fieldmodulenotifier_id notifier);

/**
 * Destroys reference to the field module notifier and sets it to NULL.
 * Internally this just decrements the reference count.
 *
 * @param notifier_address  Address of field module notifier handle to destroy.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_fieldmodulenotifier_destroy(cmzn_fieldmodulenotifier_id *notifier_address);

/**
 * Stop and clear field module callback. This will stop the callback and also
 * remove the callback function from the fieldmodule notifier.
 *
 * @param notifier  Handle to the fieldmodule notifier.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_fieldmodulenotifier_clear_callback(cmzn_fieldmodulenotifier_id notifier);

/**
 * Assign the callback function and user data for the fieldmodule notifier.
 * This function also starts the callback.
 *
 * @see cmzn_fieldmodulenotifier_callback_function
 * @param notifier  Handle to the fieldmodule notifier.
 * @param function  function to be called when event is triggered.
 * @param user_data_in  Void pointer to user object. User must ensure this
 * object's lifetime exceeds the duration for which callbacks are active.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_fieldmodulenotifier_set_callback(cmzn_fieldmodulenotifier_id notifier,
	cmzn_fieldmodulenotifier_callback_function function, void *user_data_in);

/** 
 * Return the user data set by user when calling cmzn_fieldmodulenotifier_set_callback 
 * 
 * @see cmzn_fieldmodulenotifier_set_callback 
 * @param notifier  Handle to the field module notifier. 
 * @return  user data or NULL on failure or not set. 
 */ 
ZINC_API void *cmzn_fieldmodulenotifier_get_callback_user_data( 
 cmzn_fieldmodulenotifier_id notifier); 

/**
* Returns a new reference to the fieldmodule event with reference count incremented.
* Caller is responsible for destroying the new reference.
*
* @param event  The field module event to obtain a new reference to.
* @return  New field module notifier handle with incremented reference count.
*/
ZINC_API cmzn_fieldmoduleevent_id cmzn_fieldmoduleevent_access(
	cmzn_fieldmoduleevent_id event);

/**
 * Destroys this handle to the fieldmodule event and sets it to NULL.
 * Internally this just decrements the reference count.
 * Note: Do not destroy the event argument passed to the user callback function.
 *
 * @param event_address  Address of field module event handle to destroy.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_fieldmoduleevent_destroy(cmzn_fieldmoduleevent_id *event_address);

/**
 * Get logical OR of flags indicating how fields in the field module have changed.
 * @see cmzn_fieldmoduleevent_change_flag
 *
 * @param event  Handle to the field module event.
 * @return  The change flags summarising the change: logical OR of
 * enum cmzn_fieldmoduleevent_change_flag values.
 */
ZINC_API cmzn_fieldmoduleevent_change_flags cmzn_fieldmoduleevent_get_change_flags(
	cmzn_fieldmoduleevent_id event);

/**
 * Get logical OR of flags indicating how the field has changed.
 * @see cmzn_fieldmoduleevent_change_flag
 *
 * @param event  Handle to the field module event to query.
 * @param field  The field to query about.
 * @return  The change flags summarising the change: logical OR of
 * enum cmzn_fieldmoduleevent_change_flag values. Returns
 * CMZN_FIELDMODULEEVENT_CHANGE_FLAG_NONE in case of invalid arguments.
 */
ZINC_API cmzn_fieldmoduleevent_change_flags cmzn_fieldmoduleevent_get_field_change_flags(
	cmzn_fieldmoduleevent_id event, cmzn_field_id field);

#ifdef __cplusplus
}
#endif

#endif
