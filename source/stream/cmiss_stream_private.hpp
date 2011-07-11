/***************************************************************************//**
 * FILE : cmiss_stream_private.hpp
 *
 * The private interface to Cmiss_stream.
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

#if !defined (CMISS_STREAM_PRIVATE_HPP)
#define CMISS_STREAM_PRIVATE_HPP

#include <list>
#include <stdio.h>
#include <string.h>
extern "C" {
#include "api/cmiss_stream.h"
#include "general/debug.h"
#include "general/mystring.h"
}

struct Cmiss_stream_memory_block
{
	const void *memory_buffer;
	unsigned int memory_buffer_size;
	int to_be_deallocated;
};

struct Cmiss_stream_resource
{
public:

	Cmiss_stream_resource() :
		access_count(1)
	{
	}

	virtual ~Cmiss_stream_resource()
	{
	}

	inline Cmiss_stream_resource *access()
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
}; /* struct Cmiss_io_stream */

struct Cmiss_stream_resource_file : public Cmiss_stream_resource
{
public:

	Cmiss_stream_resource_file(const char *file_name_in) : file_name(duplicate_string(file_name_in))
	{
	}

	virtual ~Cmiss_stream_resource_file()
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
}; /* struct Cmiss_input_stream */

struct Cmiss_stream_resource_memory : public Cmiss_stream_resource
{
public:

	Cmiss_stream_resource_memory() : memory_block(new Cmiss_stream_memory_block)
	{
		memory_block->to_be_deallocated = 0;
		memory_block->memory_buffer = NULL;
		memory_block->memory_buffer_size = 0;
	}

	Cmiss_stream_resource_memory(const void *buffer, int buffer_size) :
		memory_block(new Cmiss_stream_memory_block)
	{
		memory_block->to_be_deallocated = 0;
		memory_block->memory_buffer = buffer;
		memory_block->memory_buffer_size = buffer_size;
	}

	~Cmiss_stream_resource_memory()
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

	Cmiss_stream_memory_block *memory_block;
};

struct Cmiss_resource_properties
{
public:

	Cmiss_resource_properties(Cmiss_stream_resource_id resource_in) : resource(
		Cmiss_stream_resource_access(resource_in))
	{
	}

	~Cmiss_resource_properties()
	{
		Cmiss_stream_resource_destroy(&resource);
	}

	Cmiss_stream_resource_id getResource()
	{
		if (resource)
			return resource;
		return NULL;
	}

protected:
	Cmiss_stream_resource_id resource;
};

typedef std::list<Cmiss_resource_properties *> Cmiss_stream_properties_list;
typedef std::list<Cmiss_resource_properties *>::const_iterator Cmiss_stream_properties_list_const_iterator;

struct Cmiss_stream_information
{
public:

	Cmiss_stream_information() :
		access_count(1)
	{
	}

	virtual ~Cmiss_stream_information()
	{
		clearResources();
	}

	inline Cmiss_stream_information *access()
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

	int appendResourceProperties(Cmiss_resource_properties *stream_properties)
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
		std::list<Cmiss_resource_properties *>::iterator pos;
		for (pos = resources_list.begin(); pos != resources_list.end(); ++pos)
		{
			Cmiss_resource_properties *temp = *pos;
			delete temp;
		}
		resources_list.clear();
		return 1;
	}

	virtual Cmiss_resource_properties *createResourceProperties(Cmiss_stream_resource_id stream)
	{
		if (stream)
			return new Cmiss_resource_properties(stream);
		return NULL;
	}

	Cmiss_stream_resource_id createStreamResourceFile(const char *file_name)
	{
		if (file_name)
		{
			Cmiss_stream_resource_id stream = new Cmiss_stream_resource_file(
				file_name);
			appendResourceProperties(createResourceProperties(stream));
			return stream;
		}
		return NULL;
	}

	Cmiss_stream_resource_id createStreamResourceMemory()
	{
		Cmiss_stream_resource_id stream = new Cmiss_stream_resource_memory();
		appendResourceProperties(createResourceProperties(stream));
		return stream;
	}

	Cmiss_stream_resource_id createStreamResourceMemoryBuffer(const void *memory_block,
		unsigned int block_length)
	{
		if (memory_block && block_length)
		{
			Cmiss_stream_resource_id stream = new Cmiss_stream_resource_memory(
				memory_block, block_length);
			appendResourceProperties(createResourceProperties(stream));
			return stream;
		}
		return NULL;
	}

	inline Cmiss_resource_properties *findResourceInList(Cmiss_stream_resource_id resource_in)
	{
		std::list<Cmiss_resource_properties *>::iterator pos;
		for (pos = resources_list.begin(); pos != resources_list.end(); ++pos)
		{
			if ((*pos)->getResource() == resource_in)
			{
				return *pos;
			}
		}
		return NULL;
	}

	const Cmiss_stream_properties_list& getResourcesList() const
	{
		return resources_list;
	}

	int removeResource(Cmiss_stream_resource_id resource_in)
	{
		std::list<Cmiss_resource_properties *>::iterator pos;
		for (pos = resources_list.begin(); pos != resources_list.end(); ++pos)
		{
			if ((*pos)->getResource() == resource_in)
			{
				Cmiss_resource_properties *properties = *pos;
				resources_list.erase(pos);
				delete properties;
				return 1;
			}
		}
		return 0;
	}

protected:
	int access_count;
	Cmiss_stream_properties_list resources_list;
}; /* struct Cmiss_stream_information */

#endif /* CMISS_STREAM_PRIVATE_HPP */
