/***************************************************************************//**
 * FILE : cmiss_stream_private.hpp
 *
 * The private interface to cmzn_stream.
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (CMZN_STREAM_PRIVATE_HPP)
#define CMZN_STREAM_PRIVATE_HPP

#include <list>
#include <stdio.h>
#include <string.h>
#include "zinc/stream.h"
#include "general/debug.h"
#include "general/mystring.h"

struct cmzn_stream_memory_block
{
	const void *memory_buffer;
	unsigned int memory_buffer_size;
	int to_be_deallocated;
};

struct cmzn_stream_resource
{
public:

	cmzn_stream_resource() :
		access_count(1)
	{
	}

	virtual ~cmzn_stream_resource()
	{
	}

	inline cmzn_stream_resource *access()
	{
		++access_count;
		return this;
	}

	inline int deaccess()
	{
		--access_count;
		if (1 > access_count)
		{
			delete this;
		}
		return 1;
	}

protected:
	int access_count;
}; /* struct cmzn_io_stream */

struct cmzn_stream_resource_file : public cmzn_stream_resource
{
public:

	cmzn_stream_resource_file(const char *file_name_in) : file_name(duplicate_string(file_name_in))
	{
	}

	virtual ~cmzn_stream_resource_file()
	{
		if (file_name)
			DEALLOCATE(file_name);
	}

	char *getFileName()
	{
		return duplicate_string(file_name);
	}

protected:
	char *file_name;
}; /* struct cmzn_input_stream */

struct cmzn_stream_resource_memory : public cmzn_stream_resource
{
public:

	cmzn_stream_resource_memory() : memory_block(new cmzn_stream_memory_block)
	{
		memory_block->to_be_deallocated = 0;
		memory_block->memory_buffer = NULL;
		memory_block->memory_buffer_size = 0;
	}

	cmzn_stream_resource_memory(const void *buffer, int buffer_size) :
		memory_block(new cmzn_stream_memory_block)
	{
		memory_block->to_be_deallocated = 0;
		memory_block->memory_buffer = buffer;
		memory_block->memory_buffer_size = buffer_size;
	}

	~cmzn_stream_resource_memory()
	{
		if (memory_block)
		{
			if (memory_block->to_be_deallocated && memory_block->memory_buffer)

			{
				DEALLOCATE(memory_block->memory_buffer);
			}
			delete (memory_block);
			memory_block = NULL;
		}
	}

	int getBuffer(void **memory_buffer_reference, unsigned int *memory_buffer_size)
	{
		*memory_buffer_reference = const_cast<void *>(memory_block->memory_buffer);
		*memory_buffer_size = memory_block->memory_buffer_size;
		return 1;
	}

	int getBufferCopy(void **memory_buffer_references, unsigned int *memory_buffer_sizes)
	{
		if (memory_block->memory_buffer && memory_block->memory_buffer_size)
		{
			void *memory_point = NULL;
			ALLOCATE(memory_point, char, memory_block->memory_buffer_size);
			memcpy(memory_point, memory_block->memory_buffer,
				memory_block->memory_buffer_size);
			*memory_buffer_references = memory_point;
			*memory_buffer_sizes = memory_block->memory_buffer_size;
			return 1;
		}
		else
		{
			*memory_buffer_references = NULL;
			*memory_buffer_sizes = 0;
		}
		return 0;
	}

	int setBuffer(void *memory_buffer_reference, unsigned int memory_buffer_sizes)
	{
		memory_block->memory_buffer = memory_buffer_reference;
		memory_block->memory_buffer_size = memory_buffer_sizes;
		memory_block->to_be_deallocated = 1;
		return 1;
	}

protected:

	cmzn_stream_memory_block *memory_block;
};

struct cmzn_resource_properties
{
public:

	cmzn_resource_properties(cmzn_stream_resource_id resource_in) : resource(
		cmzn_stream_resource_access(resource_in))
	{
	}

	~cmzn_resource_properties()
	{
		cmzn_stream_resource_destroy(&resource);
	}

	cmzn_stream_resource_id getResource()
	{
		if (resource)
			return resource;
		return NULL;
	}

protected:
	cmzn_stream_resource_id resource;
};

typedef std::list<cmzn_resource_properties *> cmzn_stream_properties_list;
typedef std::list<cmzn_resource_properties *>::const_iterator cmzn_stream_properties_list_const_iterator;

struct cmzn_stream_information
{
public:

	cmzn_stream_information() :
		access_count(1)
	{
	}

	virtual ~cmzn_stream_information()
	{
		clearResources();
	}

	inline cmzn_stream_information *access()
	{
		++access_count;
		return this;
	}

	inline int deaccess()
	{
		--access_count;
		if (1 > access_count)
		{
			delete this;
		}
		return 1;
	}

	int appendResourceProperties(cmzn_resource_properties *stream_properties)
	{
		if (stream_properties)
		{
			resources_list.push_back(stream_properties);
			return 1;
		}
		return 0;
	}

	int clearResources()
	{
		std::list<cmzn_resource_properties *>::iterator pos;
		for (pos = resources_list.begin(); pos != resources_list.end(); ++pos)
		{
			cmzn_resource_properties *temp = *pos;
			delete temp;
		}
		resources_list.clear();
		return 1;
	}

	virtual cmzn_resource_properties *createResourceProperties(cmzn_stream_resource_id stream)
	{
		if (stream)
			return new cmzn_resource_properties(stream);
		return NULL;
	}

	cmzn_stream_resource_id createStreamResourceFile(const char *file_name)
	{
		if (file_name)
		{
			cmzn_stream_resource_id stream = new cmzn_stream_resource_file(
				file_name);
			appendResourceProperties(createResourceProperties(stream));
			return stream;
		}
		return NULL;
	}

	cmzn_stream_resource_id createStreamResourceMemory()
	{
		cmzn_stream_resource_id stream = new cmzn_stream_resource_memory();
		appendResourceProperties(createResourceProperties(stream));
		return stream;
	}

	cmzn_stream_resource_id createStreamResourceMemoryBuffer(const void *memory_block,
		unsigned int block_length)
	{
		if (memory_block && block_length)
		{
			cmzn_stream_resource_id stream = new cmzn_stream_resource_memory(
				memory_block, block_length);
			appendResourceProperties(createResourceProperties(stream));
			return stream;
		}
		return NULL;
	}

	inline cmzn_resource_properties *findResourceInList(cmzn_stream_resource_id resource_in)
	{
		std::list<cmzn_resource_properties *>::iterator pos;
		for (pos = resources_list.begin(); pos != resources_list.end(); ++pos)
		{
			if ((*pos)->getResource() == resource_in)
			{
				return *pos;
			}
		}
		return NULL;
	}

	const cmzn_stream_properties_list& getResourcesList() const
	{
		return resources_list;
	}

	int removeResource(cmzn_stream_resource_id resource_in)
	{
		std::list<cmzn_resource_properties *>::iterator pos;
		for (pos = resources_list.begin(); pos != resources_list.end(); ++pos)
		{
			if ((*pos)->getResource() == resource_in)
			{
				cmzn_resource_properties *properties = *pos;
				resources_list.erase(pos);
				delete properties;
				return 1;
			}
		}
		return 0;
	}

protected:
	int access_count;
	cmzn_stream_properties_list resources_list;
}; /* struct cmzn_stream_information */

#endif /* CMZN_STREAM_PRIVATE_HPP */
