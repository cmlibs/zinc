/***************************************************************************//**
 * FILE : cmiss_stream_id.h
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

#ifndef CMISS_STREAM_ID_H

/***************************************************************************//**
 * A handle to cmiss stream information. Stream information maintain, create
 * and give details to stream_resources for reading in or writing onto
 * external resources.
 * User can get a handle to stream information from different objects that
 * provides read and write functionality.
 * Once stream is created, user can create different types of stream resources
 * and associate specific data/settings for reading and writing.
 * There are two derived types of this objects.
 *
 * #see Cmiss_stream_resource_id
 * #see Cmiss_stream_information_image_id
 * #see Cmiss_stream_information_region_id
 * #see Cmiss_field_image_create_stream_information
 * #see Cmiss_region_create_stream_information
 * #see Cmiss_stream_information_cast_image
 * #see Cmiss_stream_information_cast_region
 */
	struct Cmiss_stream_information;
	typedef struct Cmiss_stream_information *Cmiss_stream_information_id;

/***************************************************************************//**
 * A handle to cmiss stream resource. Stream resource give description of the
 * file or memory for read/write.
 * User can get a handle to stream resource through Cmiss_stream_information.
 * The stream is then add into the stream information and user can associate
 * type specific data to the resource for reading/writing.
 * There are two derived types of this object.
 *
 * #see Cmiss_stream_information_id
 * #see Cmiss_stream_resource_file_id
 * #see Cmiss_stream_resource_memory_id
 * #see Cmiss_stream_information_create_resource_file
 * #see Cmiss_stream_information_create_resource_memory
 * #see Cmiss_stream_information_create_resource_memory_buffer
 * #see Cmiss_stream_resource_cast_file
 * #see Cmiss_stream_resource_cast_memory
 */
	struct Cmiss_stream_resource;
	typedef struct Cmiss_stream_resource *Cmiss_stream_resource_id;

	/***************************************************************************//**
	 * A handle to cmiss stream resource file. Stream resource file give description
	 * of the file for reading/writing.
	 * User can get a handle to stream resource through Cmiss_stream_information.
	 * The stream is then add into the stream information and user can associate
	 * type specific data to the resource for reading/writing.
	 * This object is a derived object of Cmiss_stream_resource_id.
	 *
	 * #see Cmiss_stream_resource_id
   * #see Cmiss_stream_information_create_resource_file
   * #see Cmiss_stream_resource_cast_file
	 */
	struct Cmiss_stream_resource_file;
	typedef struct Cmiss_stream_resource_file *Cmiss_stream_resource_file_id;

	/***************************************************************************//**
	 * A handle to cmiss stream resource memory. Stream resource memory give
	 * description of the memory for reading/writing.
	 * User can get a handle to stream resource through Cmiss_stream_information.
	 * The stream is then add into the stream information and user can associate
	 * type specific data to the resource for reading/writing.
	 * This object is a derived object of Cmiss_stream_resource_id.
	 *
	 * #see Cmiss_stream_resource_id
   * #see Cmiss_stream_information_create_resource_memory
   * #see Cmiss_stream_resource_cast_memory
	 */
	struct Cmiss_stream_resource_memory;
	typedef struct Cmiss_stream_resource_memory *Cmiss_stream_resource_memory_id;

  #define CMISS_STREAM_ID_H
#endif /* CMISS_STREAM_ID_H */
