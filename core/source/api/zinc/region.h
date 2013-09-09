/*******************************************************************************
FILE : region.h

LAST MODIFIED : 03 March 2005

DESCRIPTION :
The public interface to the cmzn_regions.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_REGION_H__
#define CMZN_REGION_H__

#include "types/elementid.h"
#include "types/fieldid.h"
#include "types/fieldmoduleid.h"
#include "types/nodeid.h"
#include "types/streamid.h"
#include "types/regionid.h"

#include "zinc/zincsharedobject.h"

/*
Global functions
----------------
*/

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Returns a new reference to the region with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param region  The region to obtain a new reference to.
 * @return  New region reference with incremented reference count.
 */
ZINC_API cmzn_region_id cmzn_region_access(cmzn_region_id region);

/*******************************************************************************
 * Destroys this handle to the region, and sets it to NULL.
 * Internally this just decrements the reference count.
 *
 * @param region_address  The address to the handle of the region
 *    to be destroyed.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_region_destroy(cmzn_region_id *region_address);

/***************************************************************************//**
 * Begin caching or increment cache level for this region only. Call this
 * function before making multiple changes to the region or its fields and
 * objects via its field_module to minimise number of change messages sent to
 * clients. Must call cmzn_region_end_change after making changes.
 * Important: Do not pair with cmzn_region_end_hierarchical_change.
 * Note: region change caching encompasses field_module change caching so there
 * is no need to call cmzn_field_module_begin_change/end_change as well.
 *
 * @param region  The region to begin change cache on.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_region_begin_change(cmzn_region_id region);

/***************************************************************************//**
 * Decrement cache level or end caching of changes for this region only.
 * Call cmzn_region_begin_change before making multiple field or region changes
 * and call this afterwards. When change level is restored to zero in region,
 * cached change messages are sent out to clients.
 * Important: Do not pair with cmzn_region_begin_hierarchical_change.
 *
 * @param region  The region to end change cache on.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_region_end_change(cmzn_region_id region);

/***************************************************************************//**
 * Begin caching or increment cache level for all regions in a tree, used to
 * efficiently and safely make hierarchical field changes or modify the tree.
 * Must call cmzn_region_begin_hierarchical_change after modifications made.
 * Important: Do not pair with non-hierarchical cmzn_region_end_change.
 *
 * @param region  The root of the region tree to begin change cache on.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_region_begin_hierarchical_change(cmzn_region_id region);

/***************************************************************************//**
 * Decrement cache level or end caching of changes for all regions in a tree.
 * Call cmzn_region_begin_hierarchical_change before making hierarchical field
 * changes or modifying the region tree, and call this afterwards. When change
 * level is restored to zero in any region, cached change messages are sent out.
 * Important: Do not pair with non-hierarchical cmzn_region_begin_change.
 *
 * @param region  The root of the region tree to end change cache on.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_region_end_hierarchical_change(cmzn_region_id region);

/***************************************************************************//**
 * Returns the name of the region.
 *
 * @param region  The region whose name is requested.
 * @return  On success: allocated string containing region name. Up to caller to
 * free using cmzn_deallocate().
 */
ZINC_API char *cmzn_region_get_name(cmzn_region_id region);

/***************************************************************************//**
 * Sets the name of the region.
 * A valid region name must start with an alphanumeric character, contain only
 * alphanumeric characters, spaces ' ', dots '.', colons ':' or underscores '_',
 * and may not finish with a space.
 * Fails if the new name is already in use by another region in the same parent.
 *
 * @param region  The region to be named.
 * @param name  The new name for the region.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_region_set_name(cmzn_region_id region, const char *name);

/***************************************************************************//**
 * Returns a reference to the parent region of this region.
 *
 * @param region  The child region.
 * @return  Accessed reference to parent region, or NULL if none.
 */
ZINC_API cmzn_region_id cmzn_region_get_parent(cmzn_region_id region);

/***************************************************************************//**
 * Returns a reference to the first child region of this region.
 *
 * @param region  The region whose first child is requested.
 * @return  Accessed reference to first child region, or NULL if none.
 */
ZINC_API cmzn_region_id cmzn_region_get_first_child(cmzn_region_id region);

/***************************************************************************//**
 * Returns a reference to this region's next sibling region.
 *
 * @param region  The region whose next sibling is requested.
 * @return  Accessed reference to next sibling region, or NULL if none.
 */
ZINC_API cmzn_region_id cmzn_region_get_next_sibling(cmzn_region_id region);

/***************************************************************************//**
 * Returns a reference to this region's previous sibling region.
 *
 * @param region  The region whose previous sibling is requested.
 * @return  Accessed reference to previous sibling region, or NULL if none.
 */
ZINC_API cmzn_region_id cmzn_region_get_previous_sibling(cmzn_region_id region);

/***************************************************************************//**
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

/***************************************************************************//**
 * Adds new_child to the end of the list of child regions of this region.
 * If the new_child is already in the region tree, it is first removed.
 * Fails if new_child contains this region.
 * Fails if new_child is unnamed or the name is already used by another child of
 * this region.
 *
 * @param region  The intended parent region of new_child.
 * @param new_child  The child to add.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_region_append_child(cmzn_region_id region, cmzn_region_id new_child);

/***************************************************************************//**
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

/***************************************************************************//**
 * Removes old_child from the list of child regions of this region.
 * Fails if old_child is not a child of this region.
 *
 * @param region  The current parent region of old_child.
 * @param old_child  The child to remove.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_region_remove_child(cmzn_region_id region,
	cmzn_region_id old_child);

/***************************************************************************//**
 * Returns true if region is or contains the subregion.
 *
 * @param region  The region being tested as container.
 * @param subregion  The region being tested for containment.
 * @return  1 if this region is or contains subregion, 0 if not.
 */
ZINC_API int cmzn_region_contains_subregion(cmzn_region_id region,
	cmzn_region_id subregion);

/***************************************************************************//**
 * Returns a reference to the child region with supplied name, if any.
 *
 * @param region  The region to search.
 * @param name  The name of the child.
 * @return  Accessed reference to the named child, or NULL if no match.
 */
ZINC_API cmzn_region_id cmzn_region_find_child_by_name(
	cmzn_region_id region, const char *name);

/***************************************************************************//**
 * Returns a reference to the subregion at the path relative to this region.
 * The format of the path string is CHILD_NAME/CHILD_NAME/...
 * i.e. forward slash characters '/' are used as parent/child name separators.
 * Single leading and trailing separator characters are ignored.
 * Hence, both name="" and name="/" find the region itself.
 *
 * @param region  The region to search.
 * @param path  The directory-style path to the subregion.
 * @return  Accessed reference to subregion, or NULL no match.
 */
ZINC_API cmzn_region_id cmzn_region_find_subregion_at_path(cmzn_region_id region,
	const char *path);

/***************************************************************************//**
 * Returns field module container for this region's fields, which must be passed
 * to field factory create methods.
 *
 * @param region  The region from which to obtain the field module.
 * @return  Field module object.
 */
ZINC_API cmzn_field_module_id cmzn_region_get_field_module(cmzn_region_id region);

/***************************************************************************//**
 * Creates and returns a reference to a region compatible with base_region,
 * i.e. able to exist in the same region tree.
 *
 * @see cmzn_context_create_region
 * @param base_region  An existing region.
 * @return  Accessed reference to the newly created region, or NULL if none.
 */
ZINC_API cmzn_region_id cmzn_region_create_region(cmzn_region_id base_region);

/***************************************************************************//**
 * Create a child region with provided name in parent region.
 * Fails if a child of that name exists already.
 *
 * @see cmzn_region_set_name
 * @param parent_region  The parent region for the new region.
 * @param name  The name for the newly created region
 * @return  Accessed reference to the new child region, or NULL if failed.
 */
ZINC_API cmzn_region_id cmzn_region_create_child(cmzn_region_id parent_region,
	const char *name);

/***************************************************************************//**
 * Create a region at the specified relative path, creating any intermediary
 * regions if required.
 * Fails if a subregion exists at that path already.
 *
 * @param top_region  The region the path is relative to.
 * @param path  Region path, a series of valid region names separated by a
 * forward slash "/". Leading and trailing separator slashes are optional.
 * @return  Accessed reference to the new subregion, or NULL if failed.
 */
ZINC_API cmzn_region_id cmzn_region_create_subregion(cmzn_region_id top_region,
	const char *path);

/***************************************************************************//**
 * Reads region data using the cmzn_stream_resource obejcts provided in the
 * cmzn_stream_information object.
 * @see cmzn_stream_information_id
 *
 * @param region  The region to read the resources in stream_information into.
 * @param stream_information Handle to the cmzn_stream_information containing
 * 		information to read file into.
 * @return  Status CMZN_OK if data successfully read and merged into specified
 * region, any other value on failure.
 */
ZINC_API int cmzn_region_read(cmzn_region_id region,
	cmzn_stream_information_id stream_information);

/***************************************************************************//**
 * Convenient function to read a file with the provided name into a region
 * directly.
 *
 * @param region  The region to be read into
 * @param file_name  name of the file to read from.
 *
 * @return  Status CMZN_OK if data successfully read and merged into specified
 * region, any other value on failure.
 */
ZINC_API int cmzn_region_read_file(cmzn_region_id region, const char *file_name);

/***************************************************************************//**
 * Write region data using the data provided in the cmzn_io_stream object.
 *
 * @param region  The region to be written out.
 * @param stream_information Handle to the cmzn_stream_information containing
 * 		information to read file into.
 *
 * @return  Status CMZN_OK if data is successfully written out, any other value
 * on failure.
 */
ZINC_API int cmzn_region_write(cmzn_region_id region,
	cmzn_stream_information_id stream_information);

/***************************************************************************//**
 * Convenient function to write the region into a file with the provided name.
 *
 * @param region  The region to be written out.
 * @param file_name  name of the file to write to..
 *
 * @return  Status CMZN_OK if data is successfully written out, any other value
 * otherwise.
 */
ZINC_API int cmzn_region_write_file(cmzn_region_id region, const char *file_name);

enum cmzn_stream_information_region_attribute
{
	CMZN_STREAM_INFORMATION_REGION_ATTRIBUTE_INVALID = 0,
	CMZN_STREAM_INFORMATION_REGION_ATTRIBUTE_TIME = 1
};


/***************************************************************************//**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum cmzn_stream_information_region_attribute
	cmzn_stream_information_region_attribute_enum_from_string(const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call cmzn_deallocate to destroy the successfully returned string.
 *
 * @param attribute  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *cmzn_stream_information_region_attribute_enum_to_string(
	enum cmzn_stream_information_region_attribute attribute);

/*****************************************************************************//**
 * Creates a cmzn_stream_resource object.
 *
 * #see cmzn_stream_information_id
 *
 * @return The created object.
 */
ZINC_API cmzn_stream_information_id cmzn_region_create_stream_information(
	cmzn_region_id region);

/***************************************************************************//**
 * If the stream_information is of region type, then this function returns
 * the region specific representation, otherwise it returns NULL.
 * Caller is responsible for destroying the returned derived reference.
 *
 * @param stream_information  The generic stream_information to be cast.
 * @param stream_information  Handle to the cmzn_stream_information.
 * @return  region specific representation if the input stream_information is
 * of this type, otherwise NULL.
 */
ZINC_API cmzn_stream_information_region_id cmzn_stream_information_cast_region(
	cmzn_stream_information_id stream_information);

/***************************************************************************//**
 * Cast cmzn_stream_information_region back to its base stream_information and
 * return the stream_information.
 * IMPORTANT NOTE: Returned stream_information does not have incremented
 * reference count and must not be destroyed. Use cmzn_stream_information_access()
 * to add a reference if maintaining returned handle beyond the lifetime of the
 * stream_information_image argument.
 *
 * @param stream_information  Handle to the stream_information_image_ to cast.
 * @return  Non-accessed handle to the base stream information or NULL if failed.
 */
ZINC_C_INLINE cmzn_stream_information_id cmzn_stream_information_region_base_cast(
	cmzn_stream_information_region_id stream_information)
{
	return (cmzn_stream_information_id)(stream_information);
}

/*****************************************************************************//**
 * Destroys a cmzn_stream_information_region object.
 *
 * @param stream_information_address  Pointer to a stream_information object, which
 * is destroyed and the pointer is set to NULL.
 * @return  Status CMZN_OK if the operation is successful, any other value on
 * failure.
 */
ZINC_API int cmzn_stream_information_region_destroy(
	cmzn_stream_information_region_id *stream_information_address);

/***************************************************************************//**
 * Check either an attribute of stream_information has been set or
 * not.
 *
 * @param stream_information  Handle to the cmzn_stream_information_region.
 * @param attribute  The identifier of the real attribute to get.
 * @return  1 if attribute has been set.
 */
ZINC_API bool cmzn_stream_information_region_has_attribute(
	cmzn_stream_information_region_id stream_information,
	enum cmzn_stream_information_region_attribute attribute);

/***************************************************************************//**
 * Get a real value of an attribute of stream_information.
 *
 * @param stream_information  Handle to the cmzn_stream_information_region.
 * @param attribute  The identifier of the real attribute to get.
 * @return  Value of the attribute.
 */
ZINC_API double cmzn_stream_information_region_get_attribute_real(
	cmzn_stream_information_region_id stream_information,
	enum cmzn_stream_information_region_attribute attribute);

/***************************************************************************//**
 * Set a double attribute of the cmzn_stream_information_region.
 *
 * @param stream_information  Handle to the cmzn_stream_information_region.
 * @param attribute  The identifier of the double attribute to set.
 * @param value  The new value for the attribute.
 *
 * @return  status CMZN_OK if attribute successfully set, any other value if
 * failed or attribute not valid or unable to be set for this
 * cmzn_stream_information_region.
 */
ZINC_API int cmzn_stream_information_region_set_attribute_real(
	cmzn_stream_information_region_id stream_information,
	enum cmzn_stream_information_region_attribute attribute,
	double value);

/***************************************************************************//**
 * Check either an attribute of a stream in stream_information has been set or
 * not.
 *
 * @param stream_information  Handle to the cmzn_stream_information_region.
 * @param resource  Handle to the cmzn_stream_resource.
 * @param attribute  The identifier of the real attribute to get.
 * @return  1 if attribute has been set.
 */
ZINC_API bool cmzn_stream_information_region_has_resource_attribute(
	cmzn_stream_information_region_id stream_information,
	cmzn_stream_resource_id resource,
	enum cmzn_stream_information_region_attribute attribute);

/***************************************************************************//**
 * Get a real value of an attribute of a stream in stream_information.
 *
 * @param stream_information  Handle to the cmzn_stream_information_region.
 * @param resource  Handle to the cmzn_stream_resource.
 * @param attribute  The identifier of the real attribute to get.
 * @return  Value of the attribute.
 */
ZINC_API double cmzn_stream_information_region_get_resource_attribute_real(
	cmzn_stream_information_region_id stream_information,
	cmzn_stream_resource_id resource,
	enum cmzn_stream_information_region_attribute attribute);

/**
 * Set a double attribute of the cmzn_stream_information_region.
 *
 * @param stream_information  Handle to the cmzn_stream_information_region.
 * @param resource  Handle to the cmzn_stream_resource.
 * @param attribute  The identifier of the double attribute to set.
 * @param value  The new value for the attribute.
 *
 * @return   status CMZN_OK if attribute successfully set, any other value if
 * failed or attribute not valid or unable to be set for this
 * cmzn_stream_information_region.
 */
ZINC_API int cmzn_stream_information_region_set_resource_attribute_real(
	cmzn_stream_information_region_id stream_information,
	cmzn_stream_resource_id resource,
	enum cmzn_stream_information_region_attribute attribute,
	double value);

/**
 * Get the specified domain_type for a stream resource in stream_information.
 *
 * @param stream_information  Handle to the cmzn_stream_information_region.
 * @param resource  Handle to the cmzn_stream_resource.
 * @return  Bitmasks for specified domain types for stream resource,
 * 	CMZN_FIELD_DOMAIN_TYPE_INVALID if failed or unset
 */
ZINC_API int cmzn_stream_information_region_get_resource_domain_type(
	cmzn_stream_information_region_id stream_information, cmzn_stream_resource_id resource);

/***************************************************************************//**
 * Set the domain_type to be export from the region this stream information is
 * created in. The current default setting will output all domains in region.
 * The domain type also specifies nodesets without a specified domain_type in
 * exformat file to be imported as nodes or datapoints domain.
 * If leave unset, unspecified nodesets will be import as nodes.
 *
 * @param stream_information  Handle to the cmzn_stream_information_region.
 * @param resource  Handle to the cmzn_stream_resource.
 * @param domain_type  Bitmasks for the domain type to be set for output. It currently supports
 *   the following domains:
 *   CMZN_FIELD_DOMAIN_POINT - Only output the region name if this is the only bit set
 *   CMZN_FIELD_DOMAIN_NODES - Enable output of nodes
 *   CMZN_FIELD_DOMAIN_DATA - Enable output of datapoints
 *   CMZN_FIELD_DOMAIN_MESH_1D - Enable output of 1D mesh
 *   CMZN_FIELD_DOMAIN_MESH_2D - Enable output of 2D mesh
 *   CMZN_FIELD_DOMAIN_MESH_3D - Enable output of 3D mesh
 *   CMZN_FIELD_DOMAIN_MESH_HIGHEST_DIMENSION - Enable output of mesh with highest
 *   dimension possible
 *
 * @return   status CMZN_OK if domain_type is successfully set, any other value if
 *   failed or domain_type not valid or unable to be set for this
 * cmzn_stream_information_region.
 */
ZINC_API int cmzn_stream_information_region_set_resource_domain_type(
	cmzn_stream_information_region_id stream_information,
	cmzn_stream_resource_id resource,
	int domain_type);

#ifdef __cplusplus
}
#endif

#endif
