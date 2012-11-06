/***************************************************************************//**
 * FILE : cmiss_stream_private.cpp
 *
 * The definition to cmiss_stream.
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

#include "zinc/stream.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "stream/cmiss_stream_private.hpp"

Cmiss_stream_resource_id Cmiss_stream_resource_access(Cmiss_stream_resource_id resource)
{
	if (resource)
	{
		return resource->access();
	}
	return NULL;
}

int Cmiss_stream_resource_destroy(Cmiss_stream_resource_id *resource_address)
{
	if (resource_address && *resource_address)
	{
		(*resource_address)->deaccess();
		*resource_address = NULL;
		return 1;
	}
	return 0;
}

Cmiss_stream_resource_file_id Cmiss_stream_resource_cast_file(
	Cmiss_stream_resource_id resource)
{
	if (resource &&
		(dynamic_cast<Cmiss_stream_resource_file *>(resource)))
	{
		resource->access();
		return (reinterpret_cast<Cmiss_stream_resource_file *>(resource));
	}
	else
	{
		return (NULL);
	}
}

int Cmiss_stream_resource_file_destroy(Cmiss_stream_resource_file_id *resource_address)
{
	if (resource_address && *resource_address)
	{
		(*resource_address)->deaccess();
		*resource_address = NULL;
		return 1;
	}
	return 0;
}

Cmiss_stream_resource_memory_id Cmiss_stream_resource_cast_memory(
	Cmiss_stream_resource_id resource)
{
	if (resource &&
		(dynamic_cast<Cmiss_stream_resource_memory *>(resource)))
	{
		resource->access();
		return (reinterpret_cast<Cmiss_stream_resource_memory *>(resource));
	}
	else
	{
		return (NULL);
	}
}

int Cmiss_stream_resource_memory_destroy(Cmiss_stream_resource_memory_id *resource_address)
{
	if (resource_address && *resource_address)
	{
		(*resource_address)->deaccess();
		*resource_address = NULL;
		return 1;
	}
	return 0;
}

int Cmiss_stream_resource_memory_get_buffer(Cmiss_stream_resource_memory_id resource,
	void **memory_buffer_references, unsigned int *memory_buffer_sizes)
{
	if (resource)
	{
		return resource->getBuffer(memory_buffer_references, memory_buffer_sizes);
	}
	return 0;
}

int Cmiss_stream_resource_memory_get_buffer_copy(Cmiss_stream_resource_memory_id resource,
	void **memory_buffer_references, unsigned int *memory_buffer_sizes)
{
	if (resource)
	{
		return resource->getBufferCopy(memory_buffer_references, memory_buffer_sizes);
	}
	return 0;
}

char *Cmiss_stream_resource_file_get_name(Cmiss_stream_resource_file_id resource)
{
	if (resource)
	{
		return resource->getFileName();
	}
	return 0;
}

Cmiss_stream_information_id Cmiss_stream_information_access(
	Cmiss_stream_information_id stream_information)
{
	if (stream_information)
	{
		return stream_information->access();
	}
	return NULL;
}

int Cmiss_stream_information_destroy(Cmiss_stream_information_id *stream_information_address)
{
	if (stream_information_address && *stream_information_address)
	{
		(*stream_information_address)->deaccess();
		*stream_information_address = NULL;
		return 1;
	}
	return 0;
}

Cmiss_stream_resource_id Cmiss_stream_information_create_resource_file(
	Cmiss_stream_information_id stream_information, const char *file_name)
{
	if (stream_information)
	{
		return stream_information->createStreamResourceFile(file_name);
	}
	return NULL;
}

Cmiss_stream_resource_id Cmiss_stream_information_create_resource_memory(
	Cmiss_stream_information_id stream_information)
{
	if (stream_information)
	{
		return stream_information->createStreamResourceMemory();
	}
	return NULL;
}

Cmiss_stream_resource_id Cmiss_stream_information_create_resource_memory_buffer(
	Cmiss_stream_information_id stream_information,
	const void *buffer, unsigned int buffer_length)
{
	if (stream_information)
	{
		return stream_information->createStreamResourceMemoryBuffer(buffer,
			buffer_length);
	}
	return NULL;
}
