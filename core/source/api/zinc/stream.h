/**
 * FILE : stream.h
 *
 * The public interface to zinc stream.
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_STREAM_H__
#define CMZN_STREAM_H__

#include "types/streamid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns a new reference to the stream information with reference count
 * incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param streaminformation  The stream information to obtain a new reference to.
 * @return  New stream information reference with incremented reference count.
 */
ZINC_API cmzn_streaminformation_id cmzn_streaminformation_access(
	cmzn_streaminformation_id streaminformation);

/**
 * Destroys this reference to the stream information (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param streaminformation_address  Address of handle to the stream information.
 * @return  status CMZN_OK if successfully destroyed the stream information,
 * any other value on failure.
 */
ZINC_API int cmzn_streaminformation_destroy(
	cmzn_streaminformation_id *streaminformation_address);

/**
 * Creates a stream resource of file type with provided file_name.
 * Corresponding read/write functions with the stream information will attempt to
 * read/write file with the same name.
 *
 * @see cmzn_field_image_write
 * @see cmzn_region_write
 * @see cmzn_field_image_read
 * @see cmzn_region_read
 * @see cmzn_streamresource_cast_file
 *
 * @param streaminformation  Stream information which will contains the new
 * stream resource.
 * @param file_name  name of a file.
 * @return  Handle to newly created stream resource.
 */
ZINC_API cmzn_streamresource_id cmzn_streaminformation_create_streamresource_file(
	cmzn_streaminformation_id streaminformation, const char *file_name);

/**
 * Creates a stream resource of memory type with no memory buffer, the memory
 * buffer storage in this object is reserved for write or export function calls
 * with the stream information in the future. To input an memory buffer for
 * reading please see cmzn_streaminformation_create_streamresource_memory_buffer.
 *
 * @see cmzn_field_image_write
 * @see cmzn_region_write
 * @see cmzn_field_image_read
 * @see cmzn_region_read
 * @see cmzn_streamresource_cast_memory
 *
 * @param streaminformation  Stream information which will contains the new
 * stream resource.
 * @return  Handle to newly created stream resource.
 */
ZINC_API cmzn_streamresource_id cmzn_streaminformation_create_streamresource_memory(
	cmzn_streaminformation_id streaminformation);

/**
 * Creates a stream resource of memory type and store the pointer to the buffer,
 * the pointer can then be read into a zinc object. This function does not
 * copy the buffer, user is responsible for the life time of the buffer. Please
 * make sure the buffer is valid when reading the stream information.
 * @see cmzn_field_image_write
 * @see cmzn_region_write
 * @see cmzn_field_image_read
 * @see cmzn_region_read
 * @see cmzn_streamresource_cast_memory
 *
 * @param streaminformation  Stream information which will contains the new
 * stream resource.
 * @param buffer  pointer to the a memory buffer
 * @param buffer_length  length of the buffer
 * @return  Handle to newly created stream resource.
 */
ZINC_API cmzn_streamresource_id cmzn_streaminformation_create_streamresource_memory_buffer(
	cmzn_streaminformation_id streaminformation, const void *buffer,
	unsigned int buffer_length);

/**
* Returns a new reference to the stream resource with reference count incremented.
* Caller is responsible for destroying the new reference.
*
* @param resource  The stream resource to obtain a new reference to.
* @return  New stream resource reference with incremented reference count.
*/
ZINC_API cmzn_streamresource_id cmzn_streamresource_access(cmzn_streamresource_id resource);

/**
 * Destroys this reference to the stream (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param resource_address  Address of handle to the io stream.
 * @return  status CMZN_OK if successfully destroyed the output stream handle,
 * any other value on failure.
 */
ZINC_API int cmzn_streamresource_destroy(cmzn_streamresource_id *resource_address);

/**
 * If the stream resource is of file type, then this function returns
 * the file specific representation, otherwise it returns NULL.
 * Caller is responsible for destroying the returned derived reference.
 *
 * @param resource  The generic stream resource to be cast.
 * @return  streamresource_file specific representation if the input
 * streamresource is of this type, otherwise NULL.
 */
ZINC_API cmzn_streamresource_file_id cmzn_streamresource_cast_file(
	cmzn_streamresource_id resource);

/**
 * Destroys a cmzn_streamresource_file object.
 * @param resource_address  Pointer to a streamresource_file object, which
 * is destroyed and the pointer is set to NULL.
 * @return  status CMZN_OK if the operation is successful, any other value on failure.
 */
ZINC_API int cmzn_streamresource_file_destroy(
	cmzn_streamresource_file_id *resource_address);

/**
 * Cast streamresource_file back to its base streamresource and return it.
 * IMPORTANT NOTE: Returned stream resource does not have incremented
 * reference count and must not be destroyed. Use cmzn_streamresource_access()
 * to add a reference if maintaining returned handle beyond the lifetime of the
 * resource argument.
 *
 * @param resource  Handle to the streamresource_file to cast.
 * @return  Non-accessed handle to the base stream information or NULL if failed.
 */
ZINC_C_INLINE cmzn_streamresource_id cmzn_streamresource_file_base_cast(
	cmzn_streamresource_file_id resource)
{
	return (cmzn_streamresource_id)(resource);
}

/**
 * Return the name set on the file resource.
 *
 * @param resource  The resource whose file name is requested.
 * @return  On success: allocated string containing field name. Up to caller to
 * free using cmzn_deallocate().
 */
ZINC_API char *cmzn_streamresource_file_get_name(cmzn_streamresource_file_id resource);

/**
 * If the stream resource is of memory type, then this function returns
 * the file specific representation, otherwise it returns NULL.
 * Caller is responsible for destroying the returned derived reference.
 *
 * @see cmzn_streamresource_memory_get_buffer
 * @see cmzn_streamresource_memory_get_buffer_copy
 *
 * @param resource  The generic streamresource to be cast.
 * @return  streamresource_memory specific representation if the input
 * streamresource is of this type, otherwise NULL.
 */
ZINC_API cmzn_streamresource_memory_id cmzn_streamresource_cast_memory(
	cmzn_streamresource_id resource);

/**
 * Destroys a cmzn_streamresource_memory object.
 *
 * @param resource_address  Pointer to a streamresource_memory object, which
 * is destroyed and the pointer is set to NULL.
 * @return  status CMZN_OK if the operation is successful, any other value on failure.
 */
ZINC_API int cmzn_streamresource_memory_destroy(
	cmzn_streamresource_memory_id *resource_address);

/**
 * Cast streamresource_memory back to its base streamresource and
 * return it.
 * IMPORTANT NOTE: Returned streamresource does not have incremented
 * reference count and must not be destroyed. Use cmzn_streamresource_access()
 * to add a reference if maintaining returned handle beyond the lifetime of the
 * streaminformation_image argument.
 *
 * @param resource  Handle to the streamresource_memory to cast.
 * @return  Non-accessed handle to the base stream resource or NULL if failed.
 */
ZINC_C_INLINE cmzn_streamresource_id cmzn_streamresource_memory_base_cast(
	cmzn_streamresource_memory_id resource)
{
	return (cmzn_streamresource_id)(resource);
}

/**
 * Return the memory block currently in the stream resource object.
 *
 * @see cmzn_region_write
 * @see cmzn_field_image_write
 *
 * @param resource  The cmzn_streamresource_memory object.
 * @param memory_buffer_reference  Will be set to point to the allocated
 * 	memory block.
 * @param memory_buffer_size  Will be set to the length of
 * 	the returned memory block.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_streamresource_memory_get_buffer(cmzn_streamresource_memory_id resource,
	void **memory_buffer_references, unsigned int *memory_buffer_sizes);

/**
 * Similar to cmzn_streamresource_memory_get_buffer but this function will make
 * a copy and return the memory block currently in the stream resource object.
 * User must call cmzn_deallocate to release memory from the returned buffer
 * when they are no longer needed.
 *
 * @see cmzn_region_write
 * @see cmzn_field_image_write
 * @see cmzn_deallocate
 *
 * @param resource  The cmzn_streamresource_memory object.
 * @param memory_buffer_reference  Will be set to point to the allocated
 * 	memory block.
 * @param memory_buffer_size  Will be set to the length of
 * 	the returned memory block.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_streamresource_memory_get_buffer_copy(
	cmzn_streamresource_memory_id resource, void **memory_buffer_references,
	unsigned int *memory_buffer_sizes);

/**
 * Get the specified data compression for a specified streamresource in
 * streaminformation.
 *
 * @param streaminformation  Handle to the cmzn_streaminformation.
 * @param resource  Handle to the cmzn_streamresource.
 * @return  enum for specified data_compression_type for stream resource,
 * 	CMZN_STREAMINFORMATION_DATA_COMPRESSION_TYPE_INVALID if failed or unset
 */
ZINC_API enum cmzn_streaminformation_data_compression_type
	cmzn_streaminformation_get_resource_data_compression_type(
	cmzn_streaminformation_id streaminformation, cmzn_streamresource_id resource);

/**
 * Specify the data compression of this streamresource and it will override the one
 * specified in the owning streaminformation.
 *
 * @param streaminformation  Handle to the cmzn_streaminformation.
 * @param resource  Handle to the cmzn_streamresource.
 * @param data_compression_type  enum to indicate the compression used in the resources
 *	in the streaminformation
 *
 * @return   status CMZN_OK if domain types successfully set, any other value if
 *   failed or domain data_compression_type not valid or unable to be set for this
 * 	cmzn_streaminformation_resource.
 */
ZINC_API int cmzn_streaminformation_set_resource_data_compression_type(
	cmzn_streaminformation_id streaminformation,	cmzn_streamresource_id resource,
	enum cmzn_streaminformation_data_compression_type data_compression_type);

/**
 * Get the specified data compression for the stream resources in streaminformation.
 *
 * @param streaminformation  Handle to the cmzn_streaminformation.
 * @return  enum for specified data_compression_type for stream resource,
 * 	CMZN_STREAMINFORMATION_DATA_COMPRESSION_TYPE_INVALID if failed or unset
 */
ZINC_API enum cmzn_streaminformation_data_compression_type cmzn_streaminformation_get_data_compression_type(
	cmzn_streaminformation_id streaminformation);

/**
 * Specify the data compression of the streamresource in the streaminformation.
 *
 * @param streaminformation  Handle to the cmzn_streaminformation.
 * @param data_compression_type  enum to indicate the compression used in the resources
 *	in the streaminformation
 * @return   status CMZN_OK if data compression successfully set, any other value if
 *   failed or data_compression_type type is not valid or unable to be set for this
 *   cmzn_streaminformation.
 */
ZINC_API int cmzn_streaminformation_set_data_compression_type(
	cmzn_streaminformation_id streaminformation,
	enum cmzn_streaminformation_data_compression_type data_compression_type);

#ifdef __cplusplus
}
#endif

#endif
