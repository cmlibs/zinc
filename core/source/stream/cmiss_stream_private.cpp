/***************************************************************************//**
 * FILE : cmiss_stream_private.cpp
 *
 * The definition to cmiss_stream.
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "zinc/stream.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "stream/cmiss_stream_private.hpp"

cmzn_stream_resource_id cmzn_stream_resource_access(cmzn_stream_resource_id resource)
{
	if (resource)
	{
		return resource->access();
	}
	return NULL;
}

int cmzn_stream_resource_destroy(cmzn_stream_resource_id *resource_address)
{
	if (resource_address && *resource_address)
	{
		(*resource_address)->deaccess();
		*resource_address = NULL;
		return 1;
	}
	return 0;
}

cmzn_stream_resource_file_id cmzn_stream_resource_cast_file(
	cmzn_stream_resource_id resource)
{
	if (resource &&
		(dynamic_cast<cmzn_stream_resource_file *>(resource)))
	{
		resource->access();
		return (reinterpret_cast<cmzn_stream_resource_file *>(resource));
	}
	else
	{
		return (NULL);
	}
}

int cmzn_stream_resource_file_destroy(cmzn_stream_resource_file_id *resource_address)
{
	if (resource_address && *resource_address)
	{
		(*resource_address)->deaccess();
		*resource_address = NULL;
		return 1;
	}
	return 0;
}

cmzn_stream_resource_memory_id cmzn_stream_resource_cast_memory(
	cmzn_stream_resource_id resource)
{
	if (resource &&
		(dynamic_cast<cmzn_stream_resource_memory *>(resource)))
	{
		resource->access();
		return (reinterpret_cast<cmzn_stream_resource_memory *>(resource));
	}
	else
	{
		return (NULL);
	}
}

int cmzn_stream_resource_memory_destroy(cmzn_stream_resource_memory_id *resource_address)
{
	if (resource_address && *resource_address)
	{
		(*resource_address)->deaccess();
		*resource_address = NULL;
		return 1;
	}
	return 0;
}

int cmzn_stream_resource_memory_get_buffer(cmzn_stream_resource_memory_id resource,
	void **memory_buffer_references, unsigned int *memory_buffer_sizes)
{
	if (resource)
	{
		return resource->getBuffer(memory_buffer_references, memory_buffer_sizes);
	}
	return 0;
}

int cmzn_stream_resource_memory_get_buffer_copy(cmzn_stream_resource_memory_id resource,
	void **memory_buffer_references, unsigned int *memory_buffer_sizes)
{
	if (resource)
	{
		return resource->getBufferCopy(memory_buffer_references, memory_buffer_sizes);
	}
	return 0;
}

char *cmzn_stream_resource_file_get_name(cmzn_stream_resource_file_id resource)
{
	if (resource)
	{
		return resource->getFileName();
	}
	return 0;
}

cmzn_stream_information_id cmzn_stream_information_access(
	cmzn_stream_information_id stream_information)
{
	if (stream_information)
	{
		return stream_information->access();
	}
	return NULL;
}

int cmzn_stream_information_destroy(cmzn_stream_information_id *stream_information_address)
{
	if (stream_information_address && *stream_information_address)
	{
		(*stream_information_address)->deaccess();
		*stream_information_address = NULL;
		return 1;
	}
	return 0;
}

cmzn_stream_resource_id cmzn_stream_information_create_resource_file(
	cmzn_stream_information_id stream_information, const char *file_name)
{
	if (stream_information)
	{
		return stream_information->createStreamResourceFile(file_name);
	}
	return NULL;
}

cmzn_stream_resource_id cmzn_stream_information_create_resource_memory(
	cmzn_stream_information_id stream_information)
{
	if (stream_information)
	{
		return stream_information->createStreamResourceMemory();
	}
	return NULL;
}

cmzn_stream_resource_id cmzn_stream_information_create_resource_memory_buffer(
	cmzn_stream_information_id stream_information,
	const void *buffer, unsigned int buffer_length)
{
	if (stream_information)
	{
		return stream_information->createStreamResourceMemoryBuffer(buffer,
			buffer_length);
	}
	return NULL;
}
