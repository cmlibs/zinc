/**
 * @file region.h
 *
 * The public interface to region objects used to managed models and submodels
 * in a tree-like structure. Each region owns its own fields and subregions,
 * and may have a scene attached to it which holds its graphics.
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_REGION_H__
#define CMZN_REGION_H__

#include "types/contextid.h"
#include "types/elementid.h"
#include "types/fieldid.h"
#include "types/fieldmoduleid.h"
#include "types/nodeid.h"
#include "types/sceneid.h"
#include "types/streamid.h"
#include "types/regionid.h"

#include "cmlibs/zinc/zincsharedobject.h"

/*
Global functions
----------------
*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns a new handle to the region with reference count incremented.
 *
 * @param region  Handle to a region.
 * @return  New handle to region, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_region_id cmzn_region_access(cmzn_region_id region);

/**
 * Destroys this handle to the region, and sets it to NULL.
 * Internally this decrements the reference count.
 *
 * @param region_address  The address to the handle of the region
 *    to be destroyed.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_region_destroy(cmzn_region_id *region_address);

/**
 * Begin caching or increment cache level for this region only. Call this
 * function before making multiple changes to the region or its fields and
 * objects via its field_module to minimise number of change messages sent to
 * clients. Must call region end change method after making changes.
 * Important: Do not pair with region end hierarchical change method!
 * Note: region change caching encompasses field_module change caching so there
 * is no need to call fieldmodule begin/end change methods as well.
 * Can be nested.
 * @see cmzn_region_end_change
 *
 * @param region  The region to begin change cache on.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_region_begin_change(cmzn_region_id region);

/**
 * Decrement cache level or end caching of changes for this region only.
 * Call region begin change method before making multiple field or region changes
 * and call this afterwards. When change level is restored to zero in region,
 * cached change messages are sent out to clients.
 * Important: Do not pair with region begin hierarchical change method!
 * @see cmzn_region_begin_change
 *
 * @param region  The region to end change cache on.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_region_end_change(cmzn_region_id region);

/**
 * Begin caching or increment cache level for all regions in a tree, used to
 * efficiently and safely make hierarchical field changes or modify the tree.
 * Must call region end hierarchical_change method after modifications made.
 * Can be nested.
 * Important: Do not pair with non-hierarchical region end change method!
 * @see cmzn_region_end_hierarchical_change
 *
 * @param region  The root of the region tree to begin change cache on.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_region_begin_hierarchical_change(cmzn_region_id region);

/**
 * Decrement cache level or end caching of changes for all regions in a tree.
 * Call region begin hierarchical change method before making hierarchical field
 * changes or modifying the region tree, and call this afterwards. When change
 * level is restored to zero in any region, cached change messages are sent out.
 * Important: Do not pair with non-hierarchical region begin change method!
 * @see cmzn_region_begin_hierarchical_change
 *
 * @param region  The root of the region tree to end change cache on.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_region_end_hierarchical_change(cmzn_region_id region);

/**
 * Get the owning context for the region.
 *
 * @param region  The region to query.
 * @return  Handle to context, or NULL/invalid handle if none or failed.
 */
ZINC_API cmzn_context_id cmzn_region_get_context(cmzn_region_id region);

/**
 * Returns the name of the region.
 *
 * @param region  The region whose name is requested.
 * @return  On success: allocated string containing region name. Up to caller to
 * free using cmzn_deallocate().
 */
ZINC_API char *cmzn_region_get_name(cmzn_region_id region);

/**
 * Sets the name of the region. Any name is valid as long as it is unique in
 * the parent region, however avoid using forward slash characters '/' in names
 * as this is used as the region path separator, and avoid name ".." which is
 * used to identify the parent region in region paths.
 *
 * @param region  The region to be named.
 * @param name  The new name for the region.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_region_set_name(cmzn_region_id region, const char *name);

/**
 * Returns a handle to the parent region of this region.
 *
 * @param region  The child region.
 * @return  Handle to parent region, or NULL/invalid handle if none or failed.
 */
ZINC_API cmzn_region_id cmzn_region_get_parent(cmzn_region_id region);

/**
 * Returns the full path string from the root to this region, consisting of
 * child region names separated by forward slash characters '/'.
 *
 * @param region  The region whose path is requested.
 * @return  On success: allocated string containing full region path.
 */
ZINC_API char *cmzn_region_get_path(cmzn_region_id region);

/**
 * Returns the relative path to this region from base region, a string
 * consisting of child region names separated by forward slash characters '/'.
 * Relative path may start with parent region names ".." to get back to the
 * common ancestor of this and base regions.
 *
 * @param region  The region whose path is requested.
 * @param base_region  The region the path is relative to.
 * @return  Allocated string containing relative region path, or null string
 * if regions are invalid or not in a common tree.
 */
ZINC_API char *cmzn_region_get_relative_path(cmzn_region_id region,
	cmzn_region_id base_region);

/**
 * Get the root or top parent region for this region, which may be itself.
 *
 * @param region  Region to query.
 * @return  Handle to root region which will be the region itself if it has no
 * parents, or NULL/invalid handle if invalid argument.
 */
ZINC_API cmzn_region_id cmzn_region_get_root(cmzn_region_id region);

/**
 * Returns a handle to the first child region of this region.
 *
 * @param region  The region whose first child is requested.
 * @return  Handle to first child region, or NULL/invalid handle if none or failed.
 */
ZINC_API cmzn_region_id cmzn_region_get_first_child(cmzn_region_id region);

/**
 * Returns a reference to this region's next sibling region.
 *
 * @param region  The region whose next sibling is requested.
 * @return  Handle to next sibling region, or NULL/invalid handle if none or failed.
 */
ZINC_API cmzn_region_id cmzn_region_get_next_sibling(cmzn_region_id region);

/**
 * Returns a reference to this region's previous sibling region.
 *
 * @param region  The region whose previous sibling is requested.
 * @return  Handle to previous sibling region, or NULL/invalid handle if none or failed.
 */
ZINC_API cmzn_region_id cmzn_region_get_previous_sibling(cmzn_region_id region);

/**
 * Replaces the region reference with a reference to its next sibling.
 * Convenient for iterating through a child list, equivalent to:
 * {
 *   cmzn_region_id temp = cmzn_region_get_next_sibling(*region_address);
 *   cmzn_region_destroy(region_address);
 *   *region_address = temp;
 * }
 *
 * @param region_address  The address of the region reference to replace.
 */
ZINC_API void cmzn_region_reaccess_next_sibling(cmzn_region_id *region_address);

/**
 * Adds new_child to the end of the list of child regions of this region.
 * If the new_child is already in a region tree, it is first removed.
 * Fails if new_child contains this region.
 * Fails if new_child is unnamed or the name is already used by another child of
 * this region.
 *
 * @param region  The intended parent region of new_child.
 * @param new_child  The child to add.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_region_append_child(cmzn_region_id region, cmzn_region_id new_child);

/**
 * Inserts new_child before the existing ref_child in the list of child regions
 * of this region. If ref_child is NULL new_child is added at the end of the
 * list. If the new_child is already in the region tree, it is first removed.
 * Fails if new_child contains this region.
 * Fails if new_child is unnamed or the name is already used by another child of
 * this region.
 *
 * @param region  The intended parent region of new_child.
 * @param new_child  The child to append.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_region_insert_child_before(cmzn_region_id region,
	cmzn_region_id new_child, cmzn_region_id ref_child);

/**
 * Removes old_child from the list of child regions of this region.
 * Fails if old_child is not a child of this region.
 *
 * @param region  The current parent region of old_child.
 * @param old_child  The child to remove.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_region_remove_child(cmzn_region_id region,
	cmzn_region_id old_child);

/**
 * Returns true if region is or contains the subregion.
 *
 * @param region  The region being tested as container.
 * @param subregion  The region being tested for containment.
 * @return  Boolean true if this region is or contains subregion, otherwise false.
 */
ZINC_API bool cmzn_region_contains_subregion(cmzn_region_id region,
	cmzn_region_id subregion);

/**
 * Finds child region with supplied name, if any.
 *
 * @param region  Handle to region to search.
 * @param name  The name of the child.
 * @return  Handle to child region, or NULL/invalid handle if not found or failed.
 */
ZINC_API cmzn_region_id cmzn_region_find_child_by_name(
	cmzn_region_id region, const char *name);

/**
 * Returns a handle to the subregion at the path relative to this region.
 * This is able to find child, parent, sibling or cousin regions, or the
 * region itself.
 *
 * @param region  Handle to the region the path is relative to.
 * @param path  Region path, a series of valid region names separated by
 * forward slashes '/'. Leading and trailing separators are optional.
 * Name ".." identifies a parent region.
 * @return  Handle to subregion, or NULL/invalid handle if not found or failed.
 */
ZINC_API cmzn_region_id cmzn_region_find_subregion_at_path(cmzn_region_id region,
	const char *path);

/**
 * Get field module which manages this region's fields, which must be passed
 * to field factory create methods.
 *
 * @param region  Handle to region.
 * @return  Handle to field module, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_fieldmodule_id cmzn_region_get_fieldmodule(cmzn_region_id region);

/**
 * Creates and returns a reference to a region compatible with base_region,
 * i.e. able to exist in the same region tree.
 *
 * @see cmzn_context_create_region
 * @param base_region  An existing region.
 * @return  Handle to new region, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_region_id cmzn_region_create_region(cmzn_region_id base_region);

/**
 * Create a child region with provided name in region.
 * Fails if a child of that name exists already.
 *
 * @see cmzn_region_set_name
 * @param region  Handle to region to create child in.
 * @param name  The name for the new child region.
 * @return  Handle to new child region, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_region_id cmzn_region_create_child(cmzn_region_id region,
	const char *name);

/**
 * Create a region at the specified relative path, creating any intermediary
 * regions if required.
 * This is able to create child, sibling or cousin regions.
 * Fails if a subregion exists at that path already, or if the relative path
 * goes above the root region.
 *
 * @param region  The region the path is relative to.
 * @param path  Region path, a series of valid region names separated by
 * forward slashes '/'. Leading and trailing separators are optional.
 * Name ".." identifies a parent region.
 * @return  Handle to new subregion, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_region_id cmzn_region_create_subregion(cmzn_region_id region,
	const char *path);

/**
 * Reads region data using stream resource objects provided in the
 * stream information object.
 * @see cmzn_streaminformation_id
 *
 * @param region  The region to read the resources in streaminformation into.
 * @param streaminformation  Handle to the stream information region containing
 * 		information about resources to read from.
 * @return  Status CMZN_OK if data successfully read and merged into specified
 * region, any other value on failure.
 */
ZINC_API int cmzn_region_read(cmzn_region_id region,
	cmzn_streaminformation_region_id streaminformation_region);

/**
 * Convenient function to read a file with the provided name into a region
 * directly.
 *
 * @param region  The region to be read into
 * @param file_name  name of the file to read from.
 * @return  Status CMZN_OK if data successfully read and merged into specified
 * region, any other value on failure.
 */
ZINC_API int cmzn_region_read_file(cmzn_region_id region, const char *file_name);

/**
 * Writes region data to stream resource objects described in the
 * stream information object.
 *
 * @param region  The region to be written out.
 * @param streaminformation  Handle to the stream information region containing
 * 		information about resources to write to.
 * @return  Status CMZN_OK if data is successfully written out, any other value
 * on failure.
 */
ZINC_API int cmzn_region_write(cmzn_region_id region,
	cmzn_streaminformation_region_id streaminformation_region);

/**
 * Convenient function to write the region into a file with the provided name.
 *
 * @param region  The region to be written out.
 * @param file_name  name of the file to write to..
 *
 * @return  Status CMZN_OK if data is successfully written out, any other value
 * otherwise.
 */
ZINC_API int cmzn_region_write_file(cmzn_region_id region, const char *file_name);

/**
 * Return handle to the scene for this region, which contains
 * graphics for visualising fields in the region.
 *
 * @param region  The region to query.
 * @return  Handle to scene, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_scene_id cmzn_region_get_scene(cmzn_region_id region);

/**
 * Get range of times present in time sequences owned by this region. Includes
 * time sequences parameters are mapped to plus those held by the client.
 *
 * @param region  The region to query.
 * @param minimumValueOut  Location to store minimum time on success.
 * @param maximumValueOut  Location to store maximum time on success.
 * @return  Result OK on success, ERROR_NOT_FOUND if no time-varying
 * parameters, otherwise ERROR_ARGUMENT.
 */
ZINC_API int cmzn_region_get_time_range(cmzn_region_id region,
	double *minimumValueOut, double *maximumValueOut);

/**
 * Get range of times present in time sequences owned by this region and all of
 * its descendents. Includes time sequences parameters are mapped to plus those
 * held by the client.
 *
 * @param region  The root region of tree to query.
 * @param minimumValueOut  Location to store minimum time on success.
 * @param maximumValueOut  Location to store maximum time on success.
 * @return  Result OK on success, ERROR_NOT_FOUND if no time-varying
 * parameters, otherwise ERROR_ARGUMENT.
 */
ZINC_API int cmzn_region_get_hierarchical_time_range(cmzn_region_id region,
	double *minimumValueOut, double *maximumValueOut);

/**
 * Create a notifier for getting callbacks for changes to the region tree
 * structure.
 *
 * @param region  Handle to the region to get notifications for.
 * @return  Handle to new region notifier, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_regionnotifier_id cmzn_region_create_regionnotifier(
	cmzn_region_id region);

/**
 * Returns a new handle to the region event with reference count
 * incremented.
 *
 * @param event  The region event to obtain a new handle to.
 * @return  New handle to event, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_regionevent_id cmzn_regionevent_access(
	cmzn_regionevent_id event);

/**
 * Destroys this handle to the region event and sets it to NULL.
 * Internally this decrements the reference count.
 * Note: Do not destroy the event passed to the user callback function.
 *
 * @param event_address  Address of region event handle to destroy.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_regionevent_destroy(cmzn_regionevent_id *event_address);

/**
 * Returns a new handle to the region notifier with reference count
 * incremented.
 *
 * @param notifier  The region notifier to obtain a new handle to.
 * @return  New handle to region notifier, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_regionnotifier_id cmzn_regionnotifier_access(
	cmzn_regionnotifier_id notifier);

/**
 * Destroys handle to the region notifier and sets it to NULL.
 * Internally this decrements the reference count.
 *
 * @param notifier_address  Address of region notifier handle to destroy.
 * @return  Result OK on success, otherwise ERROR_ARGUMENT.
 */
ZINC_API int cmzn_regionnotifier_destroy(
	cmzn_regionnotifier_id *notifier_address);

/**
 * Stop callbacks and remove the callback function from the region notifier.
 *
 * @param notifier  Handle to the region notifier.
 * @return  Result OK on success, any other error on failure.
 */
ZINC_API int cmzn_regionnotifier_clear_callback(
	cmzn_regionnotifier_id notifier);

/**
 * Assign the callback function and user data for the region notifier.
 * This function also starts the callback.
 *
 * @see cmzn_regionnotifier_callback_function
 * @param notifier  Handle to the region notifier.
 * @param function  function to be called when event is triggered.
 * @param user_data_in  Void pointer to user object. User must ensure this
 * object's lifetime exceeds the duration for which callbacks are active.
 * @return  Result OK on success, any other error on failure.
 */
ZINC_API int cmzn_regionnotifier_set_callback(
	cmzn_regionnotifier_id notifier,
	cmzn_regionnotifier_callback_function function, void *user_data_in);

/**
 * Get the user data set when establishing the callback.
 *
 * @see cmzn_regionnotifier_set_callback
 * @param notifier  Handle to the region notifier.
 * @return  User data or NULL on failure or not set.
 */
ZINC_API void *cmzn_regionnotifier_get_callback_user_data(
	cmzn_regionnotifier_id notifier);

#ifdef __cplusplus
}
#endif

#endif
