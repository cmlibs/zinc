/**
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

/**
 * A handle to zinc stream information. Stream information maintain, create
 * and give details to streamresources for reading in or writing onto
 * external resources.
 * User can get a handle to stream information from different objects that
 * provides read and write functionality.
 * Once stream is created, user can create different types of stream resources
 * and associate specific data/settings for reading and writing.
 * There are two derived types of this objects.
 *
 * #see cmzn_streamresource_id
 * #see cmzn_streaminformation_image_id
 * #see cmzn_streaminformation_region_id
 * #see cmzn_field_image_create_streaminformation
 * #see cmzn_region_create_streaminformation
 * #see cmzn_streaminformation_cast_image
 * #see cmzn_streaminformation_cast_region
 */
	struct cmzn_streaminformation;
	typedef struct cmzn_streaminformation *cmzn_streaminformation_id;

/**
 * A handle to zinc stream resource. Stream resource give description of the
 * file or memory for read/write.
 * User can get a handle to stream resource through cmzn_streaminformation.
 * The stream is then add into the stream information and user can associate
 * type specific data to the resource for reading/writing.
 * There are two derived types of this object.
 *
 * #see cmzn_streaminformation_id
 * #see cmzn_streamresource_file_id
 * #see cmzn_streamresource_memory_id
 * #see cmzn_streaminformation_create_streamresource_file
 * #see cmzn_streaminformation_create_streamresource_memory
 * #see cmzn_streaminformation_create_streamresource_memory_buffer
 * #see cmzn_streamresource_cast_file
 * #see cmzn_streamresource_cast_memory
 */
	struct cmzn_streamresource;
	typedef struct cmzn_streamresource *cmzn_streamresource_id;

/**
 * A handle to zinc stream resource file. Stream resource file give description
 * of the file for reading/writing.
 * User can get a handle to stream resource through cmzn_streaminformation.
 * The stream is then add into the stream information and user can associate
 * type specific data to the resource for reading/writing.
 * This object is a derived object of cmzn_streamresource_id.
 *
 * #see cmzn_streamresource_id
 * #see cmzn_streaminformation_create_streamresource_file
 * #see cmzn_streamresource_cast_file
 */
struct cmzn_streamresource_file;
typedef struct cmzn_streamresource_file *cmzn_streamresource_file_id;

/**
 * A handle to zinc stream resource memory. Stream resource memory give
 * description of the memory for reading/writing.
 * User can get a handle to stream resource through cmzn_streaminformation.
 * The stream is then add into the stream information and user can associate
 * type specific data to the resource for reading/writing.
 * This object is a derived object of cmzn_streamresource_id.
 *
 * #see cmzn_streamresource_id
 * #see cmzn_streaminformation_create_streamresource_memory
 * #see cmzn_streamresource_cast_memory
 */
struct cmzn_streamresource_memory;
typedef struct cmzn_streamresource_memory *cmzn_streamresource_memory_id;

#endif
