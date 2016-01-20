/***************************************************************************//**
 * FILE : cmiss_region.h
 *
 * Definition of cmzn_region, container of fields for representing model data,
 * and child regions for building hierarchical models.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (CMZN_REGION_H)
#define CMZN_REGION_H

#include "zinc/fieldgroup.h"
#include "zinc/region.h"
#include "computed_field/computed_field.h"
#include "general/callback.h"
#include "general/object.h"


/*
Global constants
----------------
*/

/* separator character for cmzn_region in path strings */
#define CMZN_REGION_PATH_SEPARATOR_CHAR '/'
#define CMZN_REGION_PATH_SEPARATOR_STRING "/"

/*
Global types
------------
*/

struct cmzn_context;

struct cmzn_region;
/*******************************************************************************
LAST MODIFIED : 30 September 2002

DESCRIPTION :
Object responsible for building directed acyclic graph hierarchies in cmzn.
Implements hierarchies by contains a list of cmzn_region_child objects, each
with their own name as seen by this parent.
Implements advanced hierarchical functionality through context objects stored
within it. Type and role of context objects are not known to the cmzn_region.
==============================================================================*/

struct cmzn_region_changes
/*******************************************************************************
LAST MODIFIED : 11 March 2010

DESCRIPTION :
Data broadcast with callbacks from <cmzn_region> describing the changes.
==============================================================================*/
{
	/* true if the name of this region has changed */
	int name_changed;
	/* true if children added, removed or reordered in cmzn_region */
	int children_changed;
	/* if a single child has been added (and none removed) it is indicated here */
	struct cmzn_region *child_added;
	/* if a single child has been removed (and none added) it is indicated here */
	struct cmzn_region *child_removed;
}; /* struct cmzn_region_changes */

DECLARE_CMZN_CALLBACK_TYPES(cmzn_region_change, \
	struct cmzn_region *, struct cmzn_region_changes *, void);

/*
Global functions
----------------
*/

PROTOTYPE_OBJECT_FUNCTIONS(cmzn_region);

/**
 * Private constructor for a cmzn_region.
 * Region is created with an access_count of 1; DEACCESS to destroy.
 * @param base_region  Optional region to share element shape and basis data
 * with. If NULL, creates independent lists of shape and basis information.
 *
 * Note: This is an internal function and should not be exposed to the API.
 *
 * @return  Accessed reference to the newly created region, or NULL if failed.
 */
cmzn_region *cmzn_region_create_internal(cmzn_region *base_region);

/***************************************************************************//**
 * Remove all nodes, elements, data and finite element fields from this region.
 *
 * @param region  The region to clear the fields from. Must not be a group.
 * @return  1 on success, 0 if no region supplied.
 */
int cmzn_region_clear_finite_elements(struct cmzn_region *region);

/***************************************************************************//**
 * Returns FE_region for this cmzn_region.
 */
struct FE_region *cmzn_region_get_FE_region(struct cmzn_region *region);

/***************************************************************************//**
 * Returns the field manager for this region.
 */
struct MANAGER(Computed_field) *cmzn_region_get_Computed_field_manager(
	struct cmzn_region *region);

/***************************************************************************//**
 * Returns the size a field cache array needs to be to fit the assigned field
 * cache indexes.
 */
int cmzn_region_get_field_cache_size(cmzn_region_id region);

/***************************************************************************//**
 * Adds cache to the list of caches for this region. Region needs this list to
 * add new value caches for any fields created while the cache exists.
 */
void cmzn_region_add_field_cache(cmzn_region_id region, cmzn_fieldcache_id cache);

/***************************************************************************//**
 * Removes cache from the list of caches for this region.
 */
void cmzn_region_remove_field_cache(cmzn_region_id region,
	cmzn_fieldcache_id cache);

int cmzn_region_add_callback(struct cmzn_region *region,
	CMZN_CALLBACK_FUNCTION(cmzn_region_change) *function, void *user_data);
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Adds a callback to <region> so that when it changes <function> is called with
<user_data>. <function> has 3 arguments, a struct cmzn_region *, a
struct cmzn_region_changes * and the void *user_data.
==============================================================================*/

int cmzn_region_remove_callback(struct cmzn_region *region,
	CMZN_CALLBACK_FUNCTION(cmzn_region_change) *function, void *user_data);
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Removes the callback calling <function> with <user_data> from <region>.
==============================================================================*/

/***************************************************************************//**
 * Allocates and returns the path to the root_region ("/").
 *
 * @return  Allocated string "/".
 */
char *cmzn_region_get_root_region_path(void);

/***************************************************************************//**
 * Returns the full path name from the root region to this region. Path name
 * always begins and ends with the CMZN_REGION_PATH_SEPARATOR_CHAR '/'.
 *
 * @param region  The region whose path is requested.
 * @return  On success: allocated string containing full region path.
 */
char *cmzn_region_get_path(struct cmzn_region *region);

/***************************************************************************//**
 * Returns the relative path name to this region from other_region. Path name
 * always begins and ends with the CMZN_REGION_PATH_SEPARATOR_CHAR '/'.
 *
 * @param region  The region whose path is requested.
 * @param other_region  The region the path is relative to.
 * @return  On success: allocated string containing relative region path; on
 * failure: NULL, including case when region is not within other_region.
 */
char *cmzn_region_get_relative_path(struct cmzn_region *region,
	struct cmzn_region *other_region);

/***************************************************************************//**
 * Returns a non-accessed pointer to parent region of this region, if any.
 * Internal only.
 *
 * @param region  The child region.
 * @return  Non-accessed reference to parent region, or NULL if none.
 */
struct cmzn_region *cmzn_region_get_parent_internal(struct cmzn_region *region);

/*******************************************************************************
 * Internal only. External API is cmzn_fieldmodule_find_field_by_name.
 * @return  Accessed handle to field of given name, or NULL if none.
 */
cmzn_field_id cmzn_region_find_field_by_name(cmzn_region_id region,
	const char *field_name);

/***************************************************************************//**
 * Deprecated legacy version of cmzn_region_find_subregion_at_path returning
 * non-ACCESSed region as final argument.
 *
 * @param region  The region to search.
 * @param path  The directory-style path to the subregion.
 * @param subregion_address  Address to put region at path. Set to NULL if no
 * region is identified.
 * @return  1 if region found, 0 otherwise.
 */
int cmzn_region_get_region_from_path_deprecated(struct cmzn_region *region,
	const char *path, struct cmzn_region **subregion_address);

/**
 * Returns a reference to the root region of this region.
 *
 * @param region  The region.
 * @return  Accessed reference to root region, or NULL if none.
 */
struct cmzn_region *cmzn_region_get_root(struct cmzn_region *region);

/**
 * Returns true if region has no parent.
 */
bool cmzn_region_is_root(struct cmzn_region *region);

/***************************************************************************//**
 * Separates a region/path/name into the region plus region-path and remainder
 * string containing text from the first unrecognized child region name.
 *
 * Examples:
 * "fibres" or "/fibres" -> root_region, "" and "fibres" if fibres was not a
 *     child region of the root region
 * "heart/fibres" or "/heart/fibres" -> heart region, "heart" and "fibres" if
 *     heart region has no child region called fibres
 * "body/heart" -> heart region, "body/heart" and NULL name if root region
 *     contains body region contains heart region
 * "heart/bob/fred/" -> region heart, "heart" and "bob/fred" if region heart
 *     has no child region called bob
 *
 * @param root_region the starting region for path
 * @path string the input path
 * @param region_address on success, points to region partially matched by path
 * @param region_path_address on success, returns allocated string path to the
 *   returned region, stripped of leading and trailing region path separators
 * @param remainder_address on success, returns pointer to allocated remainder
 *   of path stripped of leading and trailing region path separators, or NULL
 *   if all of path was resolved
 * @return 1 on success, 0 on failure
 */
int cmzn_region_get_partial_region_path(struct cmzn_region *root_region,
	const char *path, struct cmzn_region **region_address,
	char **region_path_address,	char **remainder_address);

int cmzn_region_list(struct cmzn_region *region,
	int indent, int indent_increment);
/*******************************************************************************
LAST MODIFIED : 5 March 2003

DESCRIPTION :
Lists the cmzn_region hierarchy starting from <region>. Contents are listed
indented from the left margin by <indent> spaces; this is incremented by
<indent_increment> for each child level.
==============================================================================*/

/**
 * Check that fields and other object definitions in source region are properly
 * defined and compatible with definitions in target region.
 * Converts legacy field representations e.g. read from older EX files, hence
 * source region can be modified, and function fails if conversion is not
 * possible.
 * Successful from this function is prerequisite for calling cmzn_region_merge.
 * @see cmzn_region_merge
 * @param target_region  Optional target/global region to check compatibility
 * with. Omit to confirm conversion of legacy field representations only.
 * Not modified.
 * @param source_region  Source region to check. Can be modified.
 * @return  True if compatible and conversions successful, false if failed or
 * source region is missing.
 */
bool cmzn_region_can_merge(cmzn_region_id target_region,
	cmzn_region_id source_region);

/**
 * Merge fields and other objects from source region tree into target region,
 * transferring objects in some cases for efficiency.
 * @see cmzn_region_can_merge
 * @param target_region  Target / global region to merge into.
 * @param source_region  Source region to merge from. Modified and left in an
 * in an unusable state by this function as some of its objects, hence must be
 * destroyed after calling.
 * @return  1 on success, 0 on failure.
 */
int cmzn_region_merge(cmzn_region_id target_region, cmzn_region_id source_region);

#endif /* !defined (CMZN_REGION_H) */
