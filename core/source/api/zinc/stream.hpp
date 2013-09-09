/***************************************************************************//**
 * FILE : stream.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_STREAM_HPP__
#define CMZN_STREAM_HPP__

#include "zinc/stream.h"

namespace OpenCMISS
{
namespace Zinc
{

class StreamResource
{
protected:

	cmzn_stream_resource_id id;

public:

	StreamResource() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit StreamResource(cmzn_stream_resource_id in_stream_resource_id) : id(in_stream_resource_id)
	{  }

	StreamResource(const StreamResource& streamResource) : id(cmzn_stream_resource_access(streamResource.id))
	{  }

	StreamResource& operator=(const StreamResource& streamInformation)
	{
		cmzn_stream_resource_id temp_id = cmzn_stream_resource_access(streamInformation.id);
		if (0 != id)
		{
			cmzn_stream_resource_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~StreamResource()
	{
		if (0 != id)
		{
			cmzn_stream_resource_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_stream_resource_id getId()
	{
		return id;
	}

};

class StreamResourceFile : public StreamResource
{

public:

	StreamResourceFile(StreamResource& streamResource) :
		StreamResource(reinterpret_cast<cmzn_stream_resource_id>(
			cmzn_stream_resource_cast_file(streamResource.getId())))
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit StreamResourceFile(cmzn_stream_resource_file_id stream_resource_file_id) :
		StreamResource(reinterpret_cast<cmzn_stream_resource_id>(stream_resource_file_id))
	{ }

	char *getName()
	{
		return cmzn_stream_resource_file_get_name(
			reinterpret_cast<cmzn_stream_resource_file_id>(id));
	}

};

class StreamResourceMemory : public StreamResource
{

public:

	StreamResourceMemory(StreamResource& streamResource) :
		StreamResource(reinterpret_cast<cmzn_stream_resource_id>(
			cmzn_stream_resource_cast_memory(streamResource.getId())))
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit StreamResourceMemory(cmzn_stream_resource_memory_id stream_resource_memory_id) :
		StreamResource(reinterpret_cast<cmzn_stream_resource_id>(stream_resource_memory_id))
	{ }

};

class StreamInformation
{
protected:

	cmzn_stream_information_id id;

public:

	StreamInformation() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit StreamInformation(cmzn_stream_information_id in_stream_information_id) : id(in_stream_information_id)
	{  }

	StreamInformation(const StreamInformation& streamInformation) :
		id(cmzn_stream_information_access(streamInformation.id))
	{  }

	StreamInformation& operator=(const StreamInformation& streamInformation)
	{
		cmzn_stream_information_id temp_id = cmzn_stream_information_access(streamInformation.id);
		if (0 != id)
		{
			cmzn_stream_information_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~StreamInformation()
	{
		if (0 != id)
		{
			cmzn_stream_information_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_stream_information_id getId()
	{
		return id;
	}

	StreamResourceFile createResourceFile(const char *file_name)
	{
		return StreamResourceFile(reinterpret_cast<cmzn_stream_resource_file_id>(
			cmzn_stream_information_create_resource_file(id, file_name)));
	}

	StreamResourceMemory createResourceMemory()
	{
		return StreamResourceMemory(reinterpret_cast<cmzn_stream_resource_memory_id>(
			cmzn_stream_information_create_resource_memory(id)));
	}

	StreamResourceMemory createResourceMemoryBuffer(const void *buffer,
		unsigned int buffer_length)
	{
		return StreamResourceMemory(reinterpret_cast<cmzn_stream_resource_memory_id>(
			cmzn_stream_information_create_resource_memory_buffer(id, buffer, buffer_length)));
	}

};

}  // namespace Zinc
}


#endif /* CMZN_STREAM_HPP__ */
