/***************************************************************************//**
 * FILE : stream.hpp
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
 * The Original Code is libZinc.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2012
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

#ifndef __ZN_STREAM_HPP__
#define __ZN_STREAM_HPP__

#include "api/cmiss_stream.h"

namespace Zn
{

class StreamResource
{
protected:

	Cmiss_stream_resource_id id;

public:

	StreamResource() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit StreamResource(Cmiss_stream_resource_id in_stream_resource_id) : id(in_stream_resource_id)
	{  }

	StreamResource(const StreamResource& streamResource) : id(Cmiss_stream_resource_access(streamResource.id))
	{  }

	StreamResource& operator=(const StreamResource& streamInformation)
	{
		Cmiss_stream_resource_id temp_id = Cmiss_stream_resource_access(streamInformation.id);
		if (0 != id)
		{
			Cmiss_stream_resource_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~StreamResource()
	{
		if (0 != id)
		{
			Cmiss_stream_resource_destroy(&id);
		}
	}

	Cmiss_stream_resource_id getId()
	{
		return id;
	}

};

class StreamResourceFile : public StreamResource
{

public:

	// takes ownership of C-style region reference
	StreamResourceFile(StreamResource& streamResource) :
		StreamResource(streamResource)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit StreamResourceFile(Cmiss_stream_resource_file_id stream_resource_file_id) :
		StreamResource(reinterpret_cast<Cmiss_stream_resource_id>(stream_resource_file_id))
	{ }

	char *getName()
	{
		return Cmiss_stream_resource_file_get_name(
			reinterpret_cast<Cmiss_stream_resource_file_id>(id));
	}

};

class StreamResourceMemory : public StreamResource
{

public:

	// takes ownership of C-style region reference
	StreamResourceMemory(StreamResource& streamResource) :
		StreamResource(streamResource)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit StreamResourceMemory(Cmiss_stream_resource_memory_id stream_resource_memory_id) :
		StreamResource(reinterpret_cast<Cmiss_stream_resource_id>(stream_resource_memory_id))
	{ }

};

class StreamInformation
{
protected:

	Cmiss_stream_information_id id;

public:

	StreamInformation() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit StreamInformation(Cmiss_stream_information_id in_stream_information_id) : id(in_stream_information_id)
	{  }

	StreamInformation(const StreamInformation& streamInformation) :
		id(Cmiss_stream_information_access(streamInformation.id))
	{  }

	StreamInformation& operator=(const StreamInformation& streamInformation)
	{
		Cmiss_stream_information_id temp_id = Cmiss_stream_information_access(streamInformation.id);
		if (0 != id)
		{
			Cmiss_stream_information_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~StreamInformation()
	{
		if (0 != id)
		{
			Cmiss_stream_information_destroy(&id);
		}
	}

	Cmiss_stream_information_id getId()
	{
		return id;
	}

	StreamResourceFile createResourceFile(const char *file_name)
	{
		return StreamResourceFile(reinterpret_cast<Cmiss_stream_resource_file_id>(
			Cmiss_stream_information_create_resource_file(id, file_name)));
	}

	StreamResourceMemory createResourceMemory()
	{
		return StreamResourceMemory(reinterpret_cast<Cmiss_stream_resource_memory_id>(
			Cmiss_stream_information_create_resource_memory(id)));
	}

	StreamResourceMemory createResourceMemoryBuffer(const void *buffer,
		unsigned int buffer_length)
	{
		return StreamResourceMemory(reinterpret_cast<Cmiss_stream_resource_memory_id>(
			Cmiss_stream_information_create_resource_memory_buffer(id, buffer, buffer_length)));
	}

};

}  // namespace Zn


#endif /* __ZN_STREAM_HPP__ */
