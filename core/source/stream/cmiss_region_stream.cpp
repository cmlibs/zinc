/***************************************************************************//**
 * FILE : cmiss_region_stream.cpp
 *
 * The definition to cmzn_region_stream.
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

#include "zinc/region.h"
#include "field_io/read_fieldml.h"
#include "finite_element/export_finite_element.h"
#include "finite_element/import_finite_element.h"
#include "general/debug.h"
#include "general/mystring.h"
//#include "finite_element/finite_element_region.h"
#include "region/cmiss_region.h"
#include "stream/cmiss_region_stream.hpp"

namespace {

int cmzn_region_read_from_memory(struct cmzn_region *region, const void *memory_buffer,
	const unsigned int memory_buffer_size, struct FE_import_time_index *time_index, int useData)
{
	const char block_name[] = "dataBlock";
	const char block_name_uri[] = "memory:dataBlock";
	int return_code;
	struct IO_stream_package *io_stream_package;
	struct IO_stream *input_stream;

	ENTER(cmzn_region_read_file);
	return_code = 0;
	if (region && memory_buffer && memory_buffer_size && (io_stream_package=CREATE(IO_stream_package)()))
	{
		//We should add a way to define a memory block without requiring specifying a name.
		IO_stream_package_define_memory_block(io_stream_package,
			block_name, memory_buffer, memory_buffer_size);
		input_stream = CREATE(IO_stream)(io_stream_package);
		IO_stream_open_for_read(input_stream, block_name_uri);
		if (!useData)
		{
			return_code = read_exregion_file(region, input_stream, time_index);
		}
		else
		{
			return_code = read_exdata_file(region, input_stream, time_index);
		}
		IO_stream_close(input_stream);
		DESTROY(IO_stream)(&input_stream);
		IO_stream_package_free_memory_block(io_stream_package,
			block_name);
		DESTROY(IO_stream_package)(&io_stream_package);
	}
	LEAVE;

	return(return_code);
}

/** attempts to determine type of file: EX or FieldML */
int cmzn_region_read_field_file_of_name(struct cmzn_region *region, const char *file_name,
	struct IO_stream_package *io_stream_package,
	struct FE_import_time_index *time_index, int useData)
{
	int return_code = 0;
	if (is_FieldML_file(file_name))
	{
		if (time_index)
		{
			display_message(WARNING_MESSAGE, "cmzn_region_read. Time not supported by FieldML reader");
		}
		return_code = parse_fieldml_file(region, file_name);
	}
	else
	{
		return_code = read_exregion_file_of_name(region, file_name, io_stream_package, time_index,
			useData);
	}
	return return_code;
}

}

int cmzn_region_read(cmzn_region_id region,
	cmzn_stream_information_id stream_information)
{
	struct FE_import_time_index time_index_value, *time_index = NULL,
		stream_time_index_value, *stream_time_index = NULL;
	int return_code = 0;
	cmzn_stream_information_region_id region_stream_information = NULL;
	if (stream_information)
	{
		region_stream_information = dynamic_cast<cmzn_stream_information_region *>(stream_information);
	}
	if (region && region_stream_information &&
		(cmzn_stream_information_region_get_region_private(region_stream_information) == region))
	{
		const cmzn_stream_properties_list streams_list = region_stream_information->getResourcesList();
		struct IO_stream_package *io_stream_package = CREATE(IO_stream_package)();
		struct cmzn_region *temp_region = cmzn_region_create_region(region);
		if (!(streams_list.empty()) && io_stream_package && temp_region)
		{
			cmzn_stream_properties_list_const_iterator iter;
			cmzn_resource_properties *stream_properties = NULL;
			cmzn_stream_resource_id stream = NULL;
			return_code = 1;
			if (cmzn_stream_information_region_has_attribute(region_stream_information,
				CMZN_STREAM_INFORMATION_REGION_ATTRIBUTE_TIME))
			{
				time_index_value.time = cmzn_stream_information_region_get_attribute_real(
					region_stream_information, CMZN_STREAM_INFORMATION_REGION_ATTRIBUTE_TIME);
				time_index = &time_index_value;
			}
			for (iter = streams_list.begin(); iter != streams_list.end(); ++iter)
			{
				stream_properties = *iter;
				stream = stream_properties->getResource();
				if (cmzn_stream_information_region_has_resource_attribute(
					region_stream_information, stream, CMZN_STREAM_INFORMATION_REGION_ATTRIBUTE_TIME))
				{
					stream_time_index_value.time = cmzn_stream_information_region_get_resource_attribute_real(
						region_stream_information, stream, CMZN_STREAM_INFORMATION_REGION_ATTRIBUTE_TIME);
					stream_time_index = &stream_time_index_value;
				}
				else
				{
					stream_time_index = time_index;
				}
				cmzn_stream_resource_file_id file_resource = cmzn_stream_resource_cast_file(stream);
				cmzn_stream_resource_memory_id memory_resource = NULL;
				void *memory_block = NULL;
				unsigned int buffer_size = 0;
				int readData = 0;
				int domain_type = cmzn_stream_information_region_get_resource_domain_type(
					region_stream_information, stream);
				if ((domain_type & CMZN_FIELD_DOMAIN_DATA) && (!(domain_type & CMZN_FIELD_DOMAIN_NODES)))
				{
					readData = 1;
				}
				if (file_resource)
				{
					char *file_name = file_resource->getFileName();
					if (file_name)
					{
						if (!cmzn_region_read_field_file_of_name(temp_region, file_name, io_stream_package, stream_time_index,
							readData))
						{
							return_code = 0;
							display_message(ERROR_MESSAGE, "cmzn_region_read. Cannot read file %s", file_name);
						}
						DEALLOCATE(file_name);
					}
					cmzn_stream_resource_file_destroy(&file_resource);
				}
				else if (NULL != (memory_resource = cmzn_stream_resource_cast_memory(stream)))
				{
					memory_resource->getBuffer(&memory_block, &buffer_size);
					if (memory_block)
					{
						if (!cmzn_region_read_from_memory(temp_region, memory_block, buffer_size, stream_time_index,
							readData))
						{
							return_code = 0;
							display_message(ERROR_MESSAGE, "cmzn_region_read. Cannot read memory");
						}
					}
					cmzn_stream_resource_memory_destroy(&memory_resource);
				}
				else
				{
					return_code = 0;
					display_message(ERROR_MESSAGE, "cmzn_region_read. Stream error");
				}
				if (!return_code)
				{
					break;
				}
			}
			if (return_code && cmzn_region_can_merge(region,temp_region))
			{
				return_code = cmzn_region_merge(region, temp_region);
			}
			DEACCESS(cmzn_region)(&temp_region);
			DESTROY(IO_stream_package)(&io_stream_package);
		}
	}
	return return_code;
}

int cmzn_region_read_file(cmzn_region_id region, const char *file_name)
{
	int return_code = 0;
	if (region && file_name)
	{
		cmzn_stream_information_id stream_information =
			cmzn_region_create_stream_information(region);
		cmzn_stream_resource_id resource = cmzn_stream_information_create_resource_file(
			stream_information, file_name);
		return_code = cmzn_region_read(region, stream_information);
		cmzn_stream_resource_destroy(&resource);
		cmzn_stream_information_destroy(&stream_information);
	}
	return return_code;
}

int cmzn_region_write(cmzn_region_id region,
	cmzn_stream_information_id stream_information)
{
	int return_code = 0;
	double time = 0.0, stream_time = 0.0;
	cmzn_stream_information_region_id region_stream_information = NULL;
	if (stream_information)
	{
		region_stream_information = dynamic_cast<cmzn_stream_information_region *>(stream_information);
	}
	if (region && region_stream_information &&
		(cmzn_stream_information_region_get_region_private(region_stream_information) == region))
	{
		const cmzn_stream_properties_list streams_list = region_stream_information->getResourcesList();
		if (!(streams_list.empty()))
		{
			cmzn_stream_properties_list_const_iterator iter;
			cmzn_resource_properties *stream_properties = NULL;
			cmzn_stream_resource_id stream = NULL;
			return_code = 1;
			if (cmzn_stream_information_region_has_attribute(region_stream_information,
				CMZN_STREAM_INFORMATION_REGION_ATTRIBUTE_TIME))
			{
				time = cmzn_stream_information_region_get_attribute_real(
					region_stream_information, CMZN_STREAM_INFORMATION_REGION_ATTRIBUTE_TIME);
			}
			for (iter = streams_list.begin(); iter != streams_list.end(); ++iter)
			{
				stream_properties = *iter;
				stream = stream_properties->getResource();
				if (cmzn_stream_information_region_has_resource_attribute(
					region_stream_information, stream, CMZN_STREAM_INFORMATION_REGION_ATTRIBUTE_TIME))
				{
					stream_time = cmzn_stream_information_region_get_resource_attribute_real(
						region_stream_information, stream, CMZN_STREAM_INFORMATION_REGION_ATTRIBUTE_TIME);
				}
				else
				{
					stream_time = time;
				}
				cmzn_stream_resource_file_id file_resource = cmzn_stream_resource_cast_file(stream);
				cmzn_stream_resource_memory_id memory_resource = NULL;
				void *memory_block = NULL;
				unsigned int buffer_size = 0;
				int writeElements = 0, writeData = 0, writeNodes = 0;
				int domain_type =	cmzn_stream_information_region_get_resource_domain_type(
					region_stream_information, stream);
				if (domain_type == CMZN_FIELD_DOMAIN_TYPE_INVALID)
				{
					writeElements = CMZN_FIELD_DOMAIN_MESH_1D|CMZN_FIELD_DOMAIN_MESH_2D|
						CMZN_FIELD_DOMAIN_MESH_3D|CMZN_FIELD_DOMAIN_MESH_HIGHEST_DIMENSION;
					writeData = 1;
					writeNodes = 1;
				}
				else
				{
					if (domain_type & CMZN_FIELD_DOMAIN_NODES)
					{
						writeNodes = 1;
					}
					if (domain_type & CMZN_FIELD_DOMAIN_DATA)
					{
						writeData = 1;
					}
					if (domain_type & CMZN_FIELD_DOMAIN_MESH_1D)
					{
						writeElements = CMZN_FIELD_DOMAIN_MESH_1D;
					}
					if (domain_type & CMZN_FIELD_DOMAIN_MESH_2D)
					{
						writeElements |= CMZN_FIELD_DOMAIN_MESH_2D;
					}
					if (domain_type & CMZN_FIELD_DOMAIN_MESH_3D)
					{
						writeElements |= CMZN_FIELD_DOMAIN_MESH_3D;
					}
					if (domain_type & CMZN_FIELD_DOMAIN_MESH_HIGHEST_DIMENSION)
					{
						writeElements |= CMZN_FIELD_DOMAIN_MESH_HIGHEST_DIMENSION;
					}
				}
				if (file_resource)
				{
					char *file_name = file_resource->getFileName();
					if (file_name)
					{
						if (!write_exregion_file_of_name(file_name, region, (cmzn_field_group_id)0,
							cmzn_stream_information_region_get_root_region(region_stream_information),
							writeElements,	writeNodes, writeData,
							FE_WRITE_ALL_FIELDS, /* number_of_field_names */0, /*field_names*/ NULL,
							stream_time,	FE_WRITE_COMPLETE_GROUP, FE_WRITE_RECURSIVE))
						{
							return_code = 0;
							display_message(ERROR_MESSAGE, "cmzn_region_write. Cannot write file %s", file_name);
						}
						DEALLOCATE(file_name);
					}
					cmzn_stream_resource_file_destroy(&file_resource);
				}
				else if (NULL != (memory_resource = cmzn_stream_resource_cast_memory(stream)))
				{
					if (!write_exregion_file_to_memory_block(region, (cmzn_field_group_id)0,
						cmzn_stream_information_region_get_root_region(region_stream_information),
						writeElements,	writeNodes, writeData,
						FE_WRITE_ALL_FIELDS, /* number_of_field_names */0, /*field_names*/ NULL,
						stream_time,	FE_WRITE_COMPLETE_GROUP, FE_WRITE_RECURSIVE, &memory_block, &buffer_size))
					{
						return_code = 0;
						display_message(ERROR_MESSAGE, "cmzn_region_write. Cannot write to memory block");
					}
					else
					{
						memory_resource->setBuffer(memory_block, buffer_size);
					}
					cmzn_stream_resource_memory_destroy(&memory_resource);
				}
				else
				{
					return_code = 0;
					display_message(ERROR_MESSAGE, "cmzn_region_write. Stream error");
				}
				if (!return_code)
				{
					break;
				}
			}
		}
	}
	return return_code;
}

int cmzn_region_write_file(cmzn_region_id region, const char *file_name)
{
	int return_code = 0;
	if (region && file_name)
	{
		cmzn_stream_information_id stream_information =
			cmzn_region_create_stream_information(region);
		cmzn_stream_resource_id resource = cmzn_stream_information_create_resource_file(
			stream_information, file_name);
		return_code = cmzn_region_write(region, stream_information);
		cmzn_stream_resource_destroy(&resource);
		cmzn_stream_information_destroy(&stream_information);
	}
	return return_code;
}

cmzn_stream_information_id cmzn_region_create_stream_information(struct cmzn_region *region)
{
	if (region)
	{
		return new cmzn_stream_information_region(region);
	}
	return NULL;
}

cmzn_stream_information_region_id cmzn_stream_information_cast_region(
	cmzn_stream_information_id stream_information)
{
	if (stream_information &&
		(dynamic_cast<cmzn_stream_information_region *>(stream_information)))
	{
		stream_information->access();
		return (reinterpret_cast<cmzn_stream_information_region *>(stream_information));
	}
	else
	{
		return (NULL);
	}
}

int cmzn_stream_information_region_destroy(cmzn_stream_information_region_id *stream_information_address)
{
	if (stream_information_address && *stream_information_address)
	{
		(*stream_information_address)->deaccess();
		*stream_information_address = NULL;
		return 1;
	}
	return 0;
}


bool cmzn_stream_information_region_has_attribute(
	cmzn_stream_information_region_id stream_information,
	enum cmzn_stream_information_region_attribute attribute)
{
	if (stream_information)
	{
		switch (attribute)
		{
			case CMZN_STREAM_INFORMATION_REGION_ATTRIBUTE_TIME:
			{
				return stream_information->isTimeEnabled();
			} break;
			default:
			{
			} break;
		}
	}
	return false;
}

double cmzn_stream_information_region_get_attribute_real(
	cmzn_stream_information_region_id stream_information,
	enum cmzn_stream_information_region_attribute attribute)
{
	double return_value = 0.0;
	if (stream_information)
	{
		switch (attribute)
		{
			case CMZN_STREAM_INFORMATION_REGION_ATTRIBUTE_TIME:
			{
				return_value = stream_information->getTime();
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"cmzn_stream_information_region_get_attribute_real.  Invalid attribute");
			} break;
		}
	}
	return return_value;
}

int cmzn_stream_information_region_set_attribute_real(
	cmzn_stream_information_region_id stream_information,
	enum cmzn_stream_information_region_attribute attribute,
	double value)
{
	int return_code = 0;
	if (stream_information)
	{
		switch (attribute)
		{
			case CMZN_STREAM_INFORMATION_REGION_ATTRIBUTE_TIME:
			{
				return_code = stream_information->setTime(value);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"cmzn_stream_information_region_set_attribute_real.  Invalid attribute");
			} break;
		}
	}
	return return_code;
}

bool cmzn_stream_information_region_has_resource_attribute(
	cmzn_stream_information_region_id stream_information,
	cmzn_stream_resource_id resource,
	enum cmzn_stream_information_region_attribute attribute)
{
	if (stream_information && resource)
	{
		switch (attribute)
		{
			case CMZN_STREAM_INFORMATION_REGION_ATTRIBUTE_TIME:
			{
				return stream_information->isResourceTimeEnabled(resource);
			} break;
			default:
			{
			} break;
		}
	}
	return false;
}

double cmzn_stream_information_region_get_resource_attribute_real(
	cmzn_stream_information_region_id stream_information,
	cmzn_stream_resource_id resource,
	enum cmzn_stream_information_region_attribute attribute)
{
	double return_value = 0.0;
	if (stream_information && resource)
	{
		switch (attribute)
		{
			case CMZN_STREAM_INFORMATION_REGION_ATTRIBUTE_TIME:
			{
				return_value = stream_information->getResourceTime(resource);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"cmzn_stream_information_region_get_resource_attribute_real.  Invalid attribute");
			} break;
		}
	}
	return return_value;
}

int cmzn_stream_information_region_set_resource_attribute_real(
	cmzn_stream_information_region_id stream_information,
	cmzn_stream_resource_id resource,
	enum cmzn_stream_information_region_attribute attribute,
	double value)
{
	int return_code = 0;
	if (stream_information && resource)
	{
		switch (attribute)
		{
			case CMZN_STREAM_INFORMATION_REGION_ATTRIBUTE_TIME:
			{
				return_code = stream_information->setResourceTime(resource, value);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"cmzn_stream_information_region_set_resource_attribute_real.  Invalid attribute");
			} break;
		}
	}
	return return_code;
}

int cmzn_stream_information_region_get_resource_domain_type(
	cmzn_stream_information_region_id stream_information,
	cmzn_stream_resource_id resource)
{
	if (stream_information && resource)
	{
		return stream_information->getResourceDomainType(resource);
	}
	return (int)CMZN_FIELD_DOMAIN_TYPE_INVALID;
}

int cmzn_stream_information_region_set_resource_domain_type(
	cmzn_stream_information_region_id stream_information,
	cmzn_stream_resource_id resource,
	int domain_type)
{
	if (stream_information && resource)
	{
		return stream_information->setResourceDomainType(resource, domain_type);
	}
	return CMZN_ERROR_ARGUMENT;
}

cmzn_region_id cmzn_stream_information_region_get_region_private(
	cmzn_stream_information_region_id stream_information)
{
	return stream_information->getRegion();
}

cmzn_region_id cmzn_stream_information_region_get_root_region(
	cmzn_stream_information_region_id stream_information)
{
	if (stream_information)
	{
		return stream_information->getRootRegion();
	}
	return NULL;
}

enum cmzn_stream_information_region_attribute
	cmzn_stream_information_region_attribute_enum_from_string(const char *string)
{
	enum cmzn_stream_information_region_attribute attribute =
		(cmzn_stream_information_region_attribute)0;
	if (string)
	{
		const char *str[] = {"TIME"};
		for (unsigned int i = 0; i < 1; i ++)
		{
			if (!strcmp(str[i], string))
			{
				attribute = (cmzn_stream_information_region_attribute)(i + 1);
				break;
			}
		}
	}
	return attribute;
}

char *cmzn_stream_information_region_attribute_enum_to_string(
	enum cmzn_stream_information_region_attribute attribute)
{
	char *string = NULL;
	if (0 < attribute && attribute <= 1)
	{
		const char *str[] = {"TIME"};
		string = duplicate_string(str[attribute - 1]);
	}
	return string;
}

