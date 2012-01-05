/***************************************************************************//**
 * FILE : cmiss_stream.h
 *
 * The public interface to Cmiss_stream.
 *
 */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005-2011
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __CMISS_STREAM_H__
#define __CMISS_STREAM_H__

#include "types/cmiss_c_inline.h"
#include "types/cmiss_stream_id.h"

/***************************************************************************//**
* Returns a new reference to the stream_information with reference count
* incremented.
* Caller is responsible for destroying the new reference.
*
* @param stream_information  The stream_information to obtain a new reference to.
* @return  New stream_information reference with incremented reference count.
*/
Cmiss_stream_information_id Cmiss_stream_information_access(
	Cmiss_stream_information_id stream_information);

/***************************************************************************//**
 * Destroys this reference to the stream_information (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param stream_information_address  Address of handle to the stream_information.
 * @return  status CMISS_OK if successfully destroyed the stream_information,
 * any other value on failure.
 */
int Cmiss_stream_information_destroy(
	Cmiss_stream_information_id *stream_information_address);

/***************************************************************************//**
 * Creates a stream_resource of file type with provided file_name.
 * Corresponding read/write functions with the stream_information will attempt to
 * read/write file with the same name.
 *
 * #see Cmiss_field_image_write
 * #see Cmiss_region_write
 * #see Cmiss_field_image_read
 * #see Cmiss_region_read
 * #see Cmiss_stream_resource_cast_file
 *
 * @param stream_information  stream_information which will contains the new
 * 	stream_resource.
 *  @param file_name  name of a file.
 * @return  Handle to newly created stream_resource.
 */
Cmiss_stream_resource_id Cmiss_stream_information_create_resource_file(
	Cmiss_stream_information_id stream_information, const char *file_name);

/***************************************************************************//**
 * Creates a stream_resource of memory type with no memory buffer, the memory
 * buffer storage in this object is reserved for write or export function calls
 * with the stream_information in the future. To input an memory buffer for
 * reading please see Cmiss_stream_information_create_resource_memory_buffer.
 *
 * #see Cmiss_field_image_write
 * #see Cmiss_region_write
 * #see Cmiss_stream_resource_cast_memory
 *
 * @param stream_information  stream_information which will contains the new
 * 	stream_resource.
 * @return  Handle to newly created stream_resource.
 */
Cmiss_stream_resource_id Cmiss_stream_information_create_resource_memory(
	Cmiss_stream_information_id stream_information);

/***************************************************************************//**
 * Creates a stream_resource of memory type and store the pointer to the buffer,
 * the pointer can then be read into a Cmiss object. This function does not
 * copy the buffer, user is responsible for the life time of the buffer. Please
 * make sure the buffer is valid when reading the stream information.
 * #see Cmiss_field_image_read
 * #see Cmiss_region_read
 * #see Cmiss_stream_resource_cast_memory
 *
 * @param stream_information  stream_information which will contains the new
 * 	stream_resource.
 * @param buffer  pointer to the a memory buffer
 * @param buffer_length  length of the buffer
 * @return  Handle to newly created stream_resource.
 */
Cmiss_stream_resource_id Cmiss_stream_information_create_resource_memory_buffer(
	Cmiss_stream_information_id stream_information, const void *buffer,
	unsigned int buffer_length);

/***************************************************************************//**
* Returns a new reference to the stream_resource with reference count incremented.
* Caller is responsible for destroying the new reference.
*
* @param stream  The stream_resource to obtain a new reference to.
* @return  New stream_resource reference with incremented reference count.
*/
Cmiss_stream_resource_id Cmiss_stream_resource_access(Cmiss_stream_resource_id resource);

/***************************************************************************//**
 * Destroys this reference to the stream (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param resource_address  Address of handle to the io stream.
 * @return  status CMISS_OK if successfully destroyed the output stream handle,
 * any other value on failure.
 */
int Cmiss_stream_resource_destroy(Cmiss_stream_resource_id *resource_address);

/***************************************************************************//**
 * If the stream_resource is of file type, then this function returns
 * the file specific representation, otherwise it returns NULL.
 * Caller is responsible for destroying the returned derived reference.
 *
 * @param resource  The generic stream_resource to be cast.
 * @return  stream_resource_file specific representation if the input
 * stream_resource is of this type, otherwise NULL.
 */
Cmiss_stream_resource_file_id Cmiss_stream_resource_cast_file(
	Cmiss_stream_resource_id resource);

/*****************************************************************************//**
 * Destroys a Cmiss_stream_resource_file object.
 * @param resource_address  Pointer to a stream_resource_file object, which
 * is destroyed and the pointer is set to NULL.
 * @return  status CMISS_OK if the operation is successful, any other value on failure.
 */
int Cmiss_stream_resource_file_destroy(
	Cmiss_stream_resource_file_id *resource_address);

/***************************************************************************//**
 * Cast stream_resource_file back to its base stream_resource and
 * return it.
 * IMPORTANT NOTE: Returned stream_resource does not have incremented
 * reference count and must not be destroyed. Use Cmiss_stream_resource_access()
 * to add a reference if maintaining returned handle beyond the lifetime of the
 * stream_information_image argument.
 *
 * @param stream  Handle to the stream_resource_file to cast.
 * @return  Non-accessed handle to the base stream information or NULL if failed.
 */
CMISS_C_INLINE Cmiss_stream_resource_id Cmiss_stream_resource_file_base_cast(
	Cmiss_stream_resource_file_id resource)
{
	return (Cmiss_stream_resource_id)(resource);
}

/***************************************************************************//**
 * Return the name set on the file resource.
 *
 * @param resource  The resource whose file name is requested.
 * @return  On success: allocated string containing field name. Up to caller to
 * free using Cmiss_deallocate().
 */
char *Cmiss_stream_resource_file_get_name(Cmiss_stream_resource_file_id resource);

/***************************************************************************//**
 * If the stream_resource is of memory type, then this function returns
 * the file specific representation, otherwise it returns NULL.
 * Caller is responsible for destroying the returned derived reference.
 *
 * #see Cmiss_stream_resource_memory_get_buffer
 * #see Cmiss_stream_resource_memory_get_buffer_copy
 *
 * @param resource  The generic stream_resource to be cast.
 * @return  stream_resource_memory specific representation if the input
 * stream_resource is of this type, otherwise NULL.
 */
Cmiss_stream_resource_memory_id Cmiss_stream_resource_cast_memory(
	Cmiss_stream_resource_id resource);

/*****************************************************************************//**
 * Destroys a Cmiss_stream_resource_memory object.
 *
 * @param resource_address  Pointer to a stream_resource_memory object, which
 * is destroyed and the pointer is set to NULL.
 * @return  status CMISS_OK if the operation is successful, any other value on failure.
 */
int Cmiss_stream_resource_memory_destroy(
	Cmiss_stream_resource_memory_id *resource_address);

/***************************************************************************//**
 * Cast stream_resource_memory back to its base stream_resource and
 * return it.
 * IMPORTANT NOTE: Returned stream_resource does not have incremented
 * reference count and must not be destroyed. Use Cmiss_stream_resource_access()
 * to add a reference if maintaining returned handle beyond the lifetime of the
 * stream_information_image argument.
 *
 * @param stream  Handle to the stream_resource_memory to cast.
 * @return  Non-accessed handle to the base stream resource or NULL if failed.
 */
CMISS_C_INLINE Cmiss_stream_resource_id Cmiss_stream_resource_memory_base_cast(
	Cmiss_stream_resource_memory_id stream_resource)
{
	return (Cmiss_stream_resource_id)(stream_resource);
}

/*****************************************************************************//**
 * Return the memory block currently in the stream resource object.
 *
 * #see Cmiss_region_write
 * #see Cmiss_field_image_write
 *
 * @param resource  The Cmiss_stream_resource_memory object.
 * @param memory_buffer_reference  Will be set to point to the allocated
 * 	memory block.
 * @param memory_buffer_size  Will be set to the length of
 * 	the returned memory block.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
int Cmiss_stream_resource_memory_get_buffer(Cmiss_stream_resource_memory_id resource,
	void **memory_buffer_references, unsigned int *memory_buffer_sizes);

/*****************************************************************************//**
 * Similar to Cmiss_stream_resource_memory_get_buffer but this function will make
 * a copy and return the memory block currently in the stream resource object.
 * User must call Cmiss_deallocate to release memory from the returned buffer
 * when they are no longer needed.
 *
 * #see Cmiss_region_write
 * #see Cmiss_field_image_write
 * #see Cmiss_deallocate
 *
 * @param resource  The Cmiss_stream_resource_memory object.
 * @param memory_buffer_reference  Will be set to point to the allocated
 * 	memory block.
 * @param memory_buffer_size  Will be set to the length of
 * 	the returned memory block.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
int Cmiss_stream_resource_memory_get_buffer_copy(
	Cmiss_stream_resource_memory_id resource, void **memory_buffer_references,
	unsigned int *memory_buffer_sizes);

#endif
