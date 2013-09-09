/***************************************************************************//**
 * FILE : streamid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_STREAMID_H__
#define CMZN_STREAMID_H__

/***************************************************************************//**
 * A handle to zinc stream information. Stream information maintain, create
 * and give details to stream_resources for reading in or writing onto
 * external resources.
 * User can get a handle to stream information from different objects that
 * provides read and write functionality.
 * Once stream is created, user can create different types of stream resources
 * and associate specific data/settings for reading and writing.
 * There are two derived types of this objects.
 *
 * #see cmzn_stream_resource_id
 * #see cmzn_stream_information_image_id
 * #see cmzn_stream_information_region_id
 * #see cmzn_field_image_create_stream_information
 * #see cmzn_region_create_stream_information
 * #see cmzn_stream_information_cast_image
 * #see cmzn_stream_information_cast_region
 */
	struct cmzn_stream_information;
	typedef struct cmzn_stream_information *cmzn_stream_information_id;

/***************************************************************************//**
 * A handle to zinc stream resource. Stream resource give description of the
 * file or memory for read/write.
 * User can get a handle to stream resource through cmzn_stream_information.
 * The stream is then add into the stream information and user can associate
 * type specific data to the resource for reading/writing.
 * There are two derived types of this object.
 *
 * #see cmzn_stream_information_id
 * #see cmzn_stream_resource_file_id
 * #see cmzn_stream_resource_memory_id
 * #see cmzn_stream_information_create_resource_file
 * #see cmzn_stream_information_create_resource_memory
 * #see cmzn_stream_information_create_resource_memory_buffer
 * #see cmzn_stream_resource_cast_file
 * #see cmzn_stream_resource_cast_memory
 */
	struct cmzn_stream_resource;
	typedef struct cmzn_stream_resource *cmzn_stream_resource_id;

	/***************************************************************************//**
	 * A handle to zinc stream resource file. Stream resource file give description
	 * of the file for reading/writing.
	 * User can get a handle to stream resource through cmzn_stream_information.
	 * The stream is then add into the stream information and user can associate
	 * type specific data to the resource for reading/writing.
	 * This object is a derived object of cmzn_stream_resource_id.
	 *
	 * #see cmzn_stream_resource_id
   * #see cmzn_stream_information_create_resource_file
   * #see cmzn_stream_resource_cast_file
	 */
	struct cmzn_stream_resource_file;
	typedef struct cmzn_stream_resource_file *cmzn_stream_resource_file_id;

	/***************************************************************************//**
	 * A handle to zinc stream resource memory. Stream resource memory give
	 * description of the memory for reading/writing.
	 * User can get a handle to stream resource through cmzn_stream_information.
	 * The stream is then add into the stream information and user can associate
	 * type specific data to the resource for reading/writing.
	 * This object is a derived object of cmzn_stream_resource_id.
	 *
	 * #see cmzn_stream_resource_id
   * #see cmzn_stream_information_create_resource_memory
   * #see cmzn_stream_resource_cast_memory
	 */
	struct cmzn_stream_resource_memory;
	typedef struct cmzn_stream_resource_memory *cmzn_stream_resource_memory_id;

#endif
