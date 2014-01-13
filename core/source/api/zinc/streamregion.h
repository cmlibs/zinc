/**
 * FILE : streamregion.h
 *
 * Region-specific file stream objects.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_STREAMREGION_H__
#define CMZN_STREAMREGION_H__

#include "types/fieldid.h"
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

/**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum cmzn_streaminformation_region_attribute
	cmzn_streaminformation_region_attribute_enum_from_string(const char *string);

/**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call cmzn_deallocate to destroy the successfully returned string.
 *
 * @param attribute  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *cmzn_streaminformation_region_attribute_enum_to_string(
	enum cmzn_streaminformation_region_attribute attribute);

/**
 * Creates a stream information object for specifying files/resources and options
 * for reading and writing field data to/from this region and child regions.
 * @see cmzn_streaminformation_id
 *
 * @return The created object.
 */
ZINC_API cmzn_streaminformation_id cmzn_region_create_streaminformation_region(
	cmzn_region_id region);

/**
 * If the streaminformation is of region type, then this function returns
 * the region specific representation, otherwise it returns NULL.
 * Caller is responsible for destroying the returned derived reference.
 *
 * @param streaminformation  The generic streaminformation to be cast.
 * @return  region specific representation if the input streaminformation is
 * of this type, otherwise NULL.
 */
ZINC_API cmzn_streaminformation_region_id cmzn_streaminformation_cast_region(
	cmzn_streaminformation_id streaminformation);

/**
 * Cast cmzn_streaminformation_region back to its base streaminformation and
 * return the streaminformation.
 * IMPORTANT NOTE: Returned streaminformation does not have incremented
 * reference count and must not be destroyed. Use cmzn_streaminformation_access()
 * to add a reference if maintaining returned handle beyond the lifetime of the
 * streaminformation_region argument.
 *
 * @param streaminformation  Handle to the streaminformation_region to cast.
 * @return  Non-accessed handle to the base stream information or NULL if failed.
 */
ZINC_C_INLINE cmzn_streaminformation_id cmzn_streaminformation_region_base_cast(
	cmzn_streaminformation_region_id streaminformation_region)
{
	return (cmzn_streaminformation_id)(streaminformation_region);
}

/**
 * Destroys a cmzn_streaminformation_region object.
 *
 * @param streaminformation_address  Pointer to a streaminformation object, which
 * is destroyed and the pointer is set to NULL.
 * @return  Status CMZN_OK if the operation is successful, any other value on
 * failure.
 */
ZINC_API int cmzn_streaminformation_region_destroy(
	cmzn_streaminformation_region_id *streaminformation_address);

/**
 * Check either an attribute of streaminformation has been set or
 * not.
 *
 * @param streaminformation  Handle to the cmzn_streaminformation_region.
 * @param attribute  The identifier of the real attribute to get.
 * @return  1 if attribute has been set.
 */
ZINC_API bool cmzn_streaminformation_region_has_attribute(
	cmzn_streaminformation_region_id streaminformation,
	enum cmzn_streaminformation_region_attribute attribute);

/**
 * Get a real value of an attribute of streaminformation.
 *
 * @param streaminformation  Handle to the cmzn_streaminformation_region.
 * @param attribute  The identifier of the real attribute to get.
 * @return  Value of the attribute.
 */
ZINC_API double cmzn_streaminformation_region_get_attribute_real(
	cmzn_streaminformation_region_id streaminformation,
	enum cmzn_streaminformation_region_attribute attribute);

/**
 * Set a double attribute of the cmzn_streaminformation_region.
 *
 * @param streaminformation  Handle to the cmzn_streaminformation_region.
 * @param attribute  The identifier of the double attribute to set.
 * @param value  The new value for the attribute.
 *
 * @return  status CMZN_OK if attribute successfully set, any other value if
 * failed or attribute not valid or unable to be set for this
 * cmzn_streaminformation_region.
 */
ZINC_API int cmzn_streaminformation_region_set_attribute_real(
	cmzn_streaminformation_region_id streaminformation,
	enum cmzn_streaminformation_region_attribute attribute,
	double value);

/**
 * Check either an attribute of a stream in streaminformation has been set or
 * not.
 *
 * @param streaminformation  Handle to the cmzn_streaminformation_region.
 * @param resource  Handle to the cmzn_streamresource.
 * @param attribute  The identifier of the real attribute to get.
 * @return  1 if attribute has been set.
 */
ZINC_API bool cmzn_streaminformation_region_has_resource_attribute(
	cmzn_streaminformation_region_id streaminformation,
	cmzn_streamresource_id resource,
	enum cmzn_streaminformation_region_attribute attribute);

/**
 * Get a real value of an attribute of a stream in streaminformation.
 *
 * @param streaminformation  Handle to the cmzn_streaminformation_region.
 * @param resource  Handle to the cmzn_streamresource.
 * @param attribute  The identifier of the real attribute to get.
 * @return  Value of the attribute.
 */
ZINC_API double cmzn_streaminformation_region_get_resource_attribute_real(
	cmzn_streaminformation_region_id streaminformation,
	cmzn_streamresource_id resource,
	enum cmzn_streaminformation_region_attribute attribute);

/**
 * Set a double attribute of the cmzn_streaminformation_region.
 *
 * @param streaminformation  Handle to the cmzn_streaminformation_region.
 * @param resource  Handle to the cmzn_streamresource.
 * @param attribute  The identifier of the double attribute to set.
 * @param value  The new value for the attribute.
 *
 * @return   status CMZN_OK if attribute successfully set, any other value if
 * failed or attribute not valid or unable to be set for this
 * cmzn_streaminformation_region.
 */
ZINC_API int cmzn_streaminformation_region_set_resource_attribute_real(
	cmzn_streaminformation_region_id streaminformation,
	cmzn_streamresource_id resource,
	enum cmzn_streaminformation_region_attribute attribute,
	double value);

/**
 * Get the specified domain types for a stream resource in streaminformation.
 *
 * @param streaminformation  Handle to the cmzn_streaminformation_region.
 * @param resource  Handle to the cmzn_streamresource.
 * @return  Bitmasks for specified domain types for stream resource,
 * 	CMZN_FIELD_DOMAIN_TYPE_INVALID if failed or unset
 */
ZINC_API cmzn_field_domain_types cmzn_streaminformation_region_get_resource_domain_types(
	cmzn_streaminformation_region_id streaminformation, cmzn_streamresource_id resource);

/**
 * Set the domain types to be exported from the region this stream information is
 * created in. The current default setting will output all domain types in region.
 * On import, the domain type also specifies nodesets without a specified domain
 * type in exformat file to be imported as nodes or datapoints domain. If unset,
 * unspecified nodesets will be imported as nodes.
 *
 * @param streaminformation  Handle to the cmzn_streaminformation_region.
 * @param resource  Handle to the cmzn_streamresource.
 * @param domain_types  Bitmasks for the domain type to be set for output. It currently supports
 *   the following domains:
 *   CMZN_FIELD_DOMAIN_TYPE_POINT - Only output the region name if this is the only bit set
 *   CMZN_FIELD_DOMAIN_TYPE_NODES - Enable output of nodes
 *   CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS - Enable output of datapoints
 *   CMZN_FIELD_DOMAIN_TYPE_MESH1D - Enable output of 1D mesh
 *   CMZN_FIELD_DOMAIN_TYPE_MESH2D - Enable output of 2D mesh
 *   CMZN_FIELD_DOMAIN_TYPE_MESH3D - Enable output of 3D mesh
 *   CMZN_FIELD_DOMAIN_TYPE_MESH_HIGHEST_DIMENSION - Enable output of mesh with highest
 *   dimension possible
 *
 * @return   status CMZN_OK if domain types successfully set, any other value if
 *   failed or domain type not valid or unable to be set for this
 * cmzn_streaminformation_region.
 */
ZINC_API int cmzn_streaminformation_region_set_resource_domain_types(
	cmzn_streaminformation_region_id streaminformation,
	cmzn_streamresource_id resource, cmzn_field_domain_types domain_types);

#ifdef __cplusplus
}
#endif

#endif
