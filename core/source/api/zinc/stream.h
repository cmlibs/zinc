/***************************************************************************//**
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

/***************************************************************************//**
* Returns a new reference to the stream_information with reference count
* incremented.
* Caller is responsible for destroying the new reference.
*
* @param stream_information  The stream_information to obtain a new reference to.
* @return  New stream_information reference with incremented reference count.
*/
ZINC_API cmzn_stream_information_id cmzn_stream_information_access(
	cmzn_stream_information_id stream_information);

/***************************************************************************//**
 * Destroys this reference to the stream_information (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param stream_information_address  Address of handle to the stream_information.
 * @return  status CMZN_OK if successfully destroyed the stream_information,
 * any other value on failure.
 */
ZINC_API int cmzn_stream_information_destroy(
	cmzn_stream_information_id *stream_information_address);

/***************************************************************************//**
 * Creates a stream_resource of file type with provided file_name.
 * Corresponding read/write functions with the stream_information will attempt to
 * read/write file with the same name.
 *
 * #see cmzn_field_image_write
 * #see cmzn_region_write
 * #see cmzn_field_image_read
 * #see cmzn_region_read
 * #see cmzn_stream_resource_cast_file
 *
 * @param stream_information  stream_information which will contains the new
 * 	stream_resource.
 *  @param file_name  name of a file.
 * @return  Handle to newly created stream_resource.
 */
ZINC_API cmzn_stream_resource_id cmzn_stream_information_create_resource_file(
	cmzn_stream_information_id stream_information, const char *file_name);

/***************************************************************************//**
 * Creates a stream_resource of memory type with no memory buffer, the memory
 * buffer storage in this object is reserved for write or export function calls
 * with the stream_information in the future. To input an memory buffer for
 * reading please see cmzn_stream_information_create_resource_memory_buffer.
 *
 * #see cmzn_field_image_write
 * #see cmzn_region_write
 * #see cmzn_stream_resource_cast_memory
 *
 * @param stream_information  stream_information which will contains the new
 * 	stream_resource.
 * @return  Handle to newly created stream_resource.
 */
ZINC_API cmzn_stream_resource_id cmzn_stream_information_create_resource_memory(
	cmzn_stream_information_id stream_information);

/***************************************************************************//**
 * Creates a stream_resource of memory type and store the pointer to the buffer,
 * the pointer can then be read into a zinc object. This function does not
 * copy the buffer, user is responsible for the life time of the buffer. Please
 * make sure the buffer is valid when reading the stream information.
 * #see cmzn_field_image_read
 * #see cmzn_region_read
 * #see cmzn_stream_resource_cast_memory
 *
 * @param stream_information  stream_information which will contains the new
 * 	stream_resource.
 * @param buffer  pointer to the a memory buffer
 * @param buffer_length  length of the buffer
 * @return  Handle to newly created stream_resource.
 */
ZINC_API cmzn_stream_resource_id cmzn_stream_information_create_resource_memory_buffer(
	cmzn_stream_information_id stream_information, const void *buffer,
	unsigned int buffer_length);

/***************************************************************************//**
* Returns a new reference to the stream_resource with reference count incremented.
* Caller is responsible for destroying the new reference.
*
* @param stream  The stream_resource to obtain a new reference to.
* @return  New stream_resource reference with incremented reference count.
*/
ZINC_API cmzn_stream_resource_id cmzn_stream_resource_access(cmzn_stream_resource_id resource);

/***************************************************************************//**
 * Destroys this reference to the stream (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param resource_address  Address of handle to the io stream.
 * @return  status CMZN_OK if successfully destroyed the output stream handle,
 * any other value on failure.
 */
ZINC_API int cmzn_stream_resource_destroy(cmzn_stream_resource_id *resource_address);

/***************************************************************************//**
 * If the stream_resource is of file type, then this function returns
 * the file specific representation, otherwise it returns NULL.
 * Caller is responsible for destroying the returned derived reference.
 *
 * @param resource  The generic stream_resource to be cast.
 * @return  stream_resource_file specific representation if the input
 * stream_resource is of this type, otherwise NULL.
 */
ZINC_API cmzn_stream_resource_file_id cmzn_stream_resource_cast_file(
	cmzn_stream_resource_id resource);

/*****************************************************************************//**
 * Destroys a cmzn_stream_resource_file object.
 * @param resource_address  Pointer to a stream_resource_file object, which
 * is destroyed and the pointer is set to NULL.
 * @return  status CMZN_OK if the operation is successful, any other value on failure.
 */
ZINC_API int cmzn_stream_resource_file_destroy(
	cmzn_stream_resource_file_id *resource_address);

/***************************************************************************//**
 * Cast stream_resource_file back to its base stream_resource and
 * return it.
 * IMPORTANT NOTE: Returned stream_resource does not have incremented
 * reference count and must not be destroyed. Use cmzn_stream_resource_access()
 * to add a reference if maintaining returned handle beyond the lifetime of the
 * stream_information_image argument.
 *
 * @param stream  Handle to the stream_resource_file to cast.
 * @return  Non-accessed handle to the base stream information or NULL if failed.
 */
ZINC_C_INLINE cmzn_stream_resource_id cmzn_stream_resource_file_base_cast(
	cmzn_stream_resource_file_id resource)
{
	return (cmzn_stream_resource_id)(resource);
}

/***************************************************************************//**
 * Return the name set on the file resource.
 *
 * @param resource  The resource whose file name is requested.
 * @return  On success: allocated string containing field name. Up to caller to
 * free using cmzn_deallocate().
 */
ZINC_API char *cmzn_stream_resource_file_get_name(cmzn_stream_resource_file_id resource);

/***************************************************************************//**
 * If the stream_resource is of memory type, then this function returns
 * the file specific representation, otherwise it returns NULL.
 * Caller is responsible for destroying the returned derived reference.
 *
 * #see cmzn_stream_resource_memory_get_buffer
 * #see cmzn_stream_resource_memory_get_buffer_copy
 *
 * @param resource  The generic stream_resource to be cast.
 * @return  stream_resource_memory specific representation if the input
 * stream_resource is of this type, otherwise NULL.
 */
ZINC_API cmzn_stream_resource_memory_id cmzn_stream_resource_cast_memory(
	cmzn_stream_resource_id resource);

/*****************************************************************************//**
 * Destroys a cmzn_stream_resource_memory object.
 *
 * @param resource_address  Pointer to a stream_resource_memory object, which
 * is destroyed and the pointer is set to NULL.
 * @return  status CMZN_OK if the operation is successful, any other value on failure.
 */
ZINC_API int cmzn_stream_resource_memory_destroy(
	cmzn_stream_resource_memory_id *resource_address);

/***************************************************************************//**
 * Cast stream_resource_memory back to its base stream_resource and
 * return it.
 * IMPORTANT NOTE: Returned stream_resource does not have incremented
 * reference count and must not be destroyed. Use cmzn_stream_resource_access()
 * to add a reference if maintaining returned handle beyond the lifetime of the
 * stream_information_image argument.
 *
 * @param stream  Handle to the stream_resource_memory to cast.
 * @return  Non-accessed handle to the base stream resource or NULL if failed.
 */
ZINC_C_INLINE cmzn_stream_resource_id cmzn_stream_resource_memory_base_cast(
	cmzn_stream_resource_memory_id stream_resource)
{
	return (cmzn_stream_resource_id)(stream_resource);
}

/*****************************************************************************//**
 * Return the memory block currently in the stream resource object.
 *
 * #see cmzn_region_write
 * #see cmzn_field_image_write
 *
 * @param resource  The cmzn_stream_resource_memory object.
 * @param memory_buffer_reference  Will be set to point to the allocated
 * 	memory block.
 * @param memory_buffer_size  Will be set to the length of
 * 	the returned memory block.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_stream_resource_memory_get_buffer(cmzn_stream_resource_memory_id resource,
	void **memory_buffer_references, unsigned int *memory_buffer_sizes);

/*****************************************************************************//**
 * Similar to cmzn_stream_resource_memory_get_buffer but this function will make
 * a copy and return the memory block currently in the stream resource object.
 * User must call cmzn_deallocate to release memory from the returned buffer
 * when they are no longer needed.
 *
 * #see cmzn_region_write
 * #see cmzn_field_image_write
 * #see cmzn_deallocate
 *
 * @param resource  The cmzn_stream_resource_memory object.
 * @param memory_buffer_reference  Will be set to point to the allocated
 * 	memory block.
 * @param memory_buffer_size  Will be set to the length of
 * 	the returned memory block.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_stream_resource_memory_get_buffer_copy(
	cmzn_stream_resource_memory_id resource, void **memory_buffer_references,
	unsigned int *memory_buffer_sizes);

#ifdef __cplusplus
}
#endif

#endif
