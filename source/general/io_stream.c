/*******************************************************************************
FILE : io_stream.c

LAST MODIFIED : 23 March 2007

DESCRIPTION :
A class to provide a consistent IO interface to files, memory, gzip and bzip
streams.
==============================================================================*/
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
 * Portions created by the Initial Developer are Copyright (C) 2005
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
#if defined (GENERIC_PC) && defined (UNIX)
/* SAB 10 December 2004 Specifying -std=gnu99 on the command line doesn't seem
	to be sufficient for the cross compiler so I am specifying it here too. */
#  define _ISOC99_SOURCE
#endif /* defined (GENERIC_PC) && defined (UNIX) */
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <zlib.h>
#include <bzlib.h>
#include "general/debug.h"
#include "general/mystring.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "user_interface/message.h"
#include "general/io_stream.h"

/* SAB 16 Sept 2004
	Unfortunately sscanf does a strlen on the buffer.  If the buffer is large
	this can be prohibitively slow, so we need to set a reduced lookahead and
	modify the buffer so that the strlen doesn't go very far */
/* SAB 2 December 2004
	Turning this on all the time, cause if you are given a memory stream
	it may not be NULL terminated at all so there is no delimiter to stop
	running into memory that shouldn't be accessed, so copying in chunks 
	guarantees a NULL delimiter. */
#define IO_STREAM_SPEED_UP_SSCANF

/*
Module types
------------
*/

enum IO_stream_type 
/*******************************************************************************
LAST MODIFIED : 23 August 2004

DESCRIPTION :
==============================================================================*/
{
	IO_STREAM_UNKNOWN_TYPE,
	IO_STREAM_FILE_TYPE,
	IO_STREAM_GZIP_FILE_TYPE,
	IO_STREAM_BZ2_FILE_TYPE,
	IO_STREAM_MEMORY_TYPE,
	IO_STREAM_GZIP_MEMORY_TYPE,
	IO_STREAM_BZ2_MEMORY_TYPE
}; /*  enum IO_stream_type */

struct IO_memory_block
/*******************************************************************************
LAST MODIFIED : 23 August 2004

DESCRIPTION :
==============================================================================*/
{
	char *name;
	void *memory_ptr;
	int data_length;
	int access_count;
}; /* struct IO_memory_block */

DECLARE_LIST_TYPES(IO_memory_block);

FULL_DECLARE_INDEXED_LIST_TYPE(IO_memory_block);

struct IO_stream_package
/*******************************************************************************
LAST MODIFIED : 23 August 2004

DESCRIPTION :
==============================================================================*/
{
	struct LIST(IO_memory_block) *memory_block_list;
}; /* struct IO_stream_package */

struct IO_stream
/*******************************************************************************
LAST MODIFIED : 23 March 2007

DESCRIPTION :
==============================================================================*/
{
	struct IO_stream_package *class;
	enum IO_stream_type type;
	char *uri;

	/* When using a chunk memory buffer */
	char *buffer;
	int buffer_index;
	int buffer_valid_index;
	int buffer_chunk_size;
	int buffer_chunks;
#if defined IO_STREAM_SPEED_UP_SSCANF
	int buffer_lookahead;
#endif /* defined IO_STREAM_SPEED_UP_SSCANF */

	/* When using a whole file memory buffer */
	char *data;
	int data_length;

	/* IO_STREAM_FILE_TYPE */
	FILE *file_handle;

	/* IO_STREAM_GZIP_FILE_TYPE */
	gzFile *gzip_file_handle;

	/* IO_STREAM_BZ2_FILE_TYPE */
	BZFILE *bz2_file_handle;

	/* IO_STREAM_MEMORY_TYPE */
	struct IO_memory_block *memory_block;
	int memory_block_index;

	/* IO_STREAM_BZ2_FILE_TYPE */
	bz_stream *bz2_memory_stream;
	int last_bz2_return;

}; /* struct IO_stream */


/*
Global functions
----------------
*/

struct IO_memory_block *CREATE(IO_memory_block)(char *name, void *memory_ptr,
	int data_length)
/*******************************************************************************
LAST MODIFIED : 23 August 2004

DESCRIPTION :
==============================================================================*/
{
	struct IO_memory_block *io_memory_block;

	ENTER(CREATE(IO_memory_block));

	if (ALLOCATE(io_memory_block, struct IO_memory_block, 1) &&
		ALLOCATE(io_memory_block->name, char, strlen(name) + 1))
	{
		strcpy(io_memory_block->name, name);
		io_memory_block->memory_ptr = memory_ptr;
		io_memory_block->data_length = data_length;
		io_memory_block->access_count = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(IO_memory_block).  Could not allocate memory for IO_memory_block");
		io_memory_block = (struct IO_memory_block *)NULL;
	}

	LEAVE;

	return (io_memory_block);
} /* CREATE(IO_memory_block) */

int DESTROY(IO_memory_block)(struct IO_memory_block **memory_block_address)
/*******************************************************************************
LAST MODIFIED : 23 August 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct IO_memory_block *memory_block;

	ENTER(DESTROY(IO_memory_block));

	if (memory_block_address && (memory_block = *memory_block_address))
	{
		DEALLOCATE(memory_block->name);
		DEALLOCATE(*memory_block_address);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(IO_memory_block). Invalid arguments.");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* DESTROY(IO_memory_block) */

DECLARE_OBJECT_FUNCTIONS(IO_memory_block)

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(IO_memory_block,name,char *,strcmp)

DECLARE_INDEXED_LIST_FUNCTIONS(IO_memory_block)

DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(IO_memory_block,name,
	char *,strcmp)

struct IO_stream_package *CREATE(IO_stream_package)(void)
/*******************************************************************************
LAST MODIFIED : 23 August 2004

DESCRIPTION :
==============================================================================*/
{
	struct IO_stream_package *io_stream_package;

	ENTER(CREATE(IO_stream_package));

	if (ALLOCATE(io_stream_package, struct IO_stream_package, 1))
	{
		io_stream_package->memory_block_list = CREATE(LIST(IO_memory_block))();
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(IO_stream_package).  Could not allocate memory for IO_stream_package");
		io_stream_package = (struct IO_stream_package *)NULL;
	}

	LEAVE;

	return (io_stream_package);
} /* CREATE(IO_stream_package) */

int IO_stream_package_define_memory_block(struct IO_stream_package *stream_class,
	char *block_name, void *memory_block, int memory_block_length)
/*******************************************************************************
LAST MODIFIED : 16 September 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(IO_stream_package_define_memory_block);

	if (stream_class && block_name && memory_block)
	{
		/* Add object to list */
		if (memory_block = CREATE(IO_memory_block)(block_name, memory_block,
				memory_block_length))
		{
			return_code = ADD_OBJECT_TO_LIST(IO_memory_block)(memory_block,
				stream_class->memory_block_list);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"IO_stream_package_define_memory_block. Unable to define block.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"IO_stream_package_define_memory_block. Invalid arguments.");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* IO_stream_package_define_memory_block */

int IO_stream_package_free_memory_block(struct IO_stream_package *stream_class,
	char *block_name)
/*******************************************************************************
LAST MODIFIED : 16 September 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct IO_memory_block *memory_block;

	ENTER(IO_stream_package_define_memory_block);

	if (stream_class && block_name)
	{
		/* Add object to list */
		if (memory_block = FIND_BY_IDENTIFIER_IN_LIST(IO_memory_block,name)(block_name,
				stream_class->memory_block_list))
		{
			return_code = REMOVE_OBJECT_FROM_LIST(IO_memory_block)(memory_block,
				stream_class->memory_block_list);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"IO_stream_package_define_memory_block. Unable to define block.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"IO_stream_package_define_memory_block. Invalid arguments.");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* IO_stream_package_define_memory_block */

int DESTROY(IO_stream_package)(struct IO_stream_package **stream_class_address)
/*******************************************************************************
LAST MODIFIED : 23 August 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct IO_stream_package *stream_class;

	ENTER(DESTROY(IO_stream_package));

	if (stream_class_address && (stream_class = *stream_class_address))
	{
		DESTROY(LIST(IO_memory_block))
			(&stream_class->memory_block_list);
		DEALLOCATE(*stream_class_address);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(IO_stream_package). Invalid arguments.");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* DESTROY(IO_stream_package) */

int IO_stream_uri_is_native_imagemagick(char *stream_uri)
/*******************************************************************************
LAST MODIFIED : 3 September 2004

DESCRIPTION :
Returns <true> if imagemagick can handle the image pointed to by <stream_uri>
directly which will probably be most efficient.
If not then image_utilities will read it all into a memory buffer and pass that
along to imagemagick.
==============================================================================*/
{
	int return_code;

	ENTER(IO_stream_uri_is_native_imagemagick);

	if (stream_uri)
	{
		if (!strncmp("memory:", stream_uri, 7))
		{
			return_code = 0;
		}
	   else
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"IO_stream_uri_is_native_imagemagick. Invalid arguments.");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* IO_stream_uri_is_native_imagemagick */

struct IO_stream *CREATE(IO_stream)(struct IO_stream_package *stream_class)
/*******************************************************************************
LAST MODIFIED : 23 March 2007

DESCRIPTION :
==============================================================================*/
{
	struct IO_stream *io_stream;

	ENTER(CREATE(IO_stream));

	if (stream_class)
	{
		if (ALLOCATE(io_stream, struct IO_stream, 1))
		{
			io_stream->class = stream_class;
			io_stream->type = IO_STREAM_UNKNOWN_TYPE;
			io_stream->uri = (char *)NULL;

			/* When using a memory buffer */
			io_stream->buffer = (char *)NULL;
			io_stream->buffer_index = 0;
			io_stream->buffer_valid_index = 0;
			io_stream->buffer_chunk_size = 0;
			io_stream->buffer_chunks = 0;
#if defined IO_STREAM_SPEED_UP_SSCANF
			io_stream->buffer_lookahead = 0;
#endif /* defined IO_STREAM_SPEED_UP_SSCANF */

			io_stream->data = (char *)NULL;
			io_stream->data_length = 0;

			/* IO_STREAM_FILE_TYPE */
			io_stream->file_handle = (FILE *)NULL;
			
			/* IO_STREAM_GZIP_FILE_TYPE */
			io_stream->gzip_file_handle = (gzFile *)NULL;

			/* IO_STREAM_BZ2_FILE_TYPE */
			io_stream->bz2_file_handle = (BZFILE *)NULL;

			/* IO_STREAM_MEMORY_TYPE */
			io_stream->memory_block = (struct IO_memory_block *)NULL;
			io_stream->memory_block_index = 0;

			/* IO_STREAM_BZ2_MEMORY_TYPE */
			io_stream->bz2_memory_stream = (bz_stream *)NULL;
			io_stream->last_bz2_return = BZ_OK;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(IO_stream).  Could not allocate memory for IO_stream");
			io_stream = (struct IO_stream *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(IO_stream). Invalid arguments.");
		io_stream = (struct IO_stream *)NULL;
	}

	LEAVE;

	return (io_stream);
} /* CREATE(IO_stream) */

int IO_stream_open_for_read(struct IO_stream *stream, char *stream_uri)
/*******************************************************************************
LAST MODIFIED : 23 March 2007

DESCRIPTION :
==============================================================================*/
{
	char *colon, *filename, *uri_type;
	int file_uri_specifier, return_code;

	ENTER(IO_stream_open);

	return_code = 0;
	if (stream && stream_uri)
	{
		if (!strncmp("memory:", stream_uri, 7))
		{
			if (stream->memory_block = FIND_BY_IDENTIFIER_IN_LIST(IO_memory_block,name)(stream_uri + 7,
					stream->class->memory_block_list))
			{
				return_code = 1;
				ACCESS(IO_memory_block)(stream->memory_block);
				if (!strncmp(".gz", stream_uri + strlen(stream_uri) - 3, 3))
				{
					stream->type = IO_STREAM_GZIP_MEMORY_TYPE;
				}
				else if (!strncmp(".bz2", stream_uri + strlen(stream_uri) - 4, 4))
				{
					stream->type = IO_STREAM_BZ2_MEMORY_TYPE;
					ALLOCATE(stream->bz2_memory_stream, bz_stream, 1);
					stream->bz2_memory_stream->next_in = (char *)NULL;
					stream->bz2_memory_stream->avail_in = 0;
					stream->bz2_memory_stream->total_in_lo32 = 0;
					stream->bz2_memory_stream->total_in_hi32 = 0;
					stream->bz2_memory_stream->next_out = (char *)NULL;
					stream->bz2_memory_stream->avail_out = 0;
					stream->bz2_memory_stream->total_out_lo32 = 0;
					stream->bz2_memory_stream->total_out_hi32 = 0;
					stream->bz2_memory_stream->state = NULL;
					stream->bz2_memory_stream->bzalloc = NULL;
					stream->bz2_memory_stream->bzfree = NULL;
					stream->bz2_memory_stream->opaque = NULL;
					if (BZ_OK != BZ2_bzDecompressInit(stream->bz2_memory_stream,
							/*debug*/0, /*small_memory*/0))
					{
						display_message(ERROR_MESSAGE,
							"IO_stream_open. Error initialiseing bz2 memory stream.");
						return_code = 0;
					}
				}
				else
				{
					stream->type = IO_STREAM_MEMORY_TYPE;
				}
#if defined IO_STREAM_SPEED_UP_SSCANF
				/* If we are going to modify the buffer for the workaround then
					we will need to copy in buffer chunks */
				stream->buffer_chunk_size = 131072;
				stream->buffer_chunks = 10;
				stream->buffer_lookahead = 100;
#endif /* defined IO_STREAM_SPEED_UP_SSCANF */
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"IO_stream_open. Unable to find memory block named \"%s\".",
					stream_uri + 7);
				return_code = 0;
			}
		}
	   else
		{
			/* Look for a colon operator, fail if not file: */
			file_uri_specifier = !strncmp("file:", stream_uri, 5);
			if (file_uri_specifier || (!(colon = strchr(stream_uri, ':'))))
			{
				if (file_uri_specifier)
				{
					filename = stream_uri + 6;
				}
				else
				{
					filename = stream_uri;
				}
				if (!strncmp(".gz", filename + strlen(filename) - 3, 3))
				{
					if (stream->gzip_file_handle = gzopen(filename, "rb"))
					{
						stream->type = IO_STREAM_GZIP_FILE_TYPE;
						stream->buffer_chunk_size = 131072;
						stream->buffer_chunks = 10;
#if defined IO_STREAM_SPEED_UP_SSCANF
						stream->buffer_lookahead = 100;
#endif /* defined IO_STREAM_SPEED_UP_SSCANF */
						return_code = 1;
					}
				}
				else if (!strncmp(".bz2", stream_uri + strlen(stream_uri) - 4, 4))
				{
					if (stream->bz2_file_handle = BZ2_bzopen(filename, "rb"))
					{
						stream->type = IO_STREAM_BZ2_FILE_TYPE;
						stream->buffer_chunk_size = 131072;
						stream->buffer_chunks = 10;
#if defined IO_STREAM_SPEED_UP_SSCANF
						stream->buffer_lookahead = 100;
#endif /* defined IO_STREAM_SPEED_UP_SSCANF */
						return_code = 1;
					}
				}
				else
				{
					if (stream->file_handle = fopen(filename, "r"))
					{
						stream->type = IO_STREAM_FILE_TYPE;
						return_code = 1;
					}
				}
			}
			else
			{
				uri_type = duplicate_string(stream_uri);
				uri_type[colon - stream_uri + 1] = 0;
				display_message(ERROR_MESSAGE,
					"IO_stream_open. uri type \"%s\" not understood.",
					uri_type);
				return_code = 0;
			}
		}
		if (IO_STREAM_UNKNOWN_TYPE != stream->type)
		{
			stream->uri = duplicate_string(stream_uri);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"IO_stream_open. Invalid arguments.");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* IO_stream_open */

static int IO_stream_read_to_internal_buffer(struct IO_stream *stream)
/*******************************************************************************
LAST MODIFIED : 23 March 2007

DESCRIPTION :
==============================================================================*/
{
	int read_characters, return_code;
#if defined (IO_STREAM_SPEED_UP_SSCANF)
	int copy_size;
#endif /* defined (IO_STREAM_SPEED_UP_SSCANF) */

	return_code = 1;
	switch (stream->type)
	{
		case IO_STREAM_GZIP_FILE_TYPE:
		case IO_STREAM_BZ2_FILE_TYPE:
#if defined (IO_STREAM_SPEED_UP_SSCANF)
		case IO_STREAM_MEMORY_TYPE:
			/* Unfortunately if we are going to modify the buffer then we need to
				copy it */
#endif /* defined (IO_STREAM_SPEED_UP_SSCANF) */
		case IO_STREAM_BZ2_MEMORY_TYPE:
		{
			if (!stream->buffer)
			{
				if (ALLOCATE(stream->buffer, char, stream->buffer_chunk_size *
						stream->buffer_chunks + 10))
				{
					stream->buffer_index = 0;
					stream->buffer_valid_index = 0;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"IO_stream_scan.  Unable to allocate internal buffer.");
					return_code = 0;
				}
			}
			if (stream->buffer_index + stream->buffer_chunk_size
				> stream->buffer_valid_index)
			{
				if (stream->buffer_valid_index + stream->buffer_chunk_size
					> stream->buffer_chunk_size * stream->buffer_chunks)
				{
					if (stream->buffer_valid_index - stream->buffer_index > stream->buffer_index)
					{
						display_message(ERROR_MESSAGE,
							"IO_stream_scan. memcpy with overlapping memory.");
						return_code = 0;
					}
					memcpy(stream->buffer, stream->buffer + stream->buffer_index, 
						stream->buffer_valid_index - stream->buffer_index);
					stream->buffer_valid_index -= stream->buffer_index;
					stream->buffer_index = 0;
				}

				switch (stream->type)
				{		
					case IO_STREAM_GZIP_FILE_TYPE:
					{
						read_characters = gzread(stream->gzip_file_handle, stream->buffer + stream->buffer_valid_index,
							stream->buffer_chunk_size);
					} break;
					case IO_STREAM_BZ2_FILE_TYPE:
					{
						read_characters = BZ2_bzread(stream->bz2_file_handle, stream->buffer + stream->buffer_valid_index,
							stream->buffer_chunk_size);
				
					} break;
#if defined (IO_STREAM_SPEED_UP_SSCANF)
					case IO_STREAM_MEMORY_TYPE:
					{
						if (stream->memory_block_index + stream->buffer_chunk_size <=
							stream->memory_block->data_length)
						{
							copy_size = stream->buffer_chunk_size;
						}
						else
						{
							copy_size = stream->memory_block->data_length - stream->memory_block_index;
						}
						if (copy_size)
						{
							memcpy(stream->buffer + stream->buffer_valid_index,
								((char *)stream->memory_block->memory_ptr) 
								+ stream->memory_block_index,
								copy_size);
						}
						read_characters = copy_size;
						stream->memory_block_index += read_characters;
					} break;
#endif /* defined (IO_STREAM_SPEED_UP_SSCANF) */
					case IO_STREAM_BZ2_MEMORY_TYPE:
					{
						if (stream->last_bz2_return == BZ_STREAM_END)
						{
							read_characters = 0;
						}
						else
						{
							stream->bz2_memory_stream->next_in = 
								((char *)stream->memory_block->memory_ptr) + stream->memory_block_index;
							stream->bz2_memory_stream->avail_in = stream->memory_block->data_length
								- stream->memory_block_index;
							stream->bz2_memory_stream->next_out = 
								stream->buffer + stream->buffer_valid_index;
							stream->bz2_memory_stream->avail_out = stream->buffer_chunk_size;

							stream->last_bz2_return = BZ2_bzDecompress(stream->bz2_memory_stream);

							read_characters = stream->buffer_chunk_size -
								stream->bz2_memory_stream->avail_out;
							stream->memory_block_index = stream->memory_block->data_length;
							if ((stream->last_bz2_return != BZ_STREAM_END) &&
								(stream->last_bz2_return != BZ_OK))
							{
								display_message(ERROR_MESSAGE,
									"IO_stream_read_to_internal_buffer.  "
									"Error uncompressing bzip2 memory buffer.");
								return_code = 0;
								read_characters = 0;
							}
						}
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"IO_stream_read_to_internal_buffer.  Invalid memory buffered read type.");
						return_code = 0;
					} break;
				}
				stream->buffer_valid_index += read_characters;
				stream->buffer[stream->buffer_valid_index] = 0;
			}
		} break;
#if ! defined (IO_STREAM_SPEED_UP_SSCANF)
		case IO_STREAM_MEMORY_TYPE:
		{
			if (!stream->buffer)
			{
				stream->buffer = stream->memory_block->memory_ptr;
				stream->buffer_index = 0;
				stream->buffer_valid_index = stream->memory_block->data_length;
			}
		} break;
#endif /* ! defined (IO_STREAM_SPEED_UP_SSCANF) */
		default:
		{
			display_message(ERROR_MESSAGE,
				"IO_stream_read_to_internal_buffer.  Invalid memory read type.");
			return_code = 0;
		} break;
	}

	return (return_code);
}

int IO_stream_end_of_stream(struct IO_stream *stream)
/*******************************************************************************
LAST MODIFIED : 23 March 2007

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(IO_stream_end_of_stream);

	if (stream)
	{
		switch (stream->type)
		{
			case IO_STREAM_FILE_TYPE:
			{
				return_code = feof(stream->file_handle);
			} break;
			case IO_STREAM_GZIP_FILE_TYPE:
			case IO_STREAM_BZ2_FILE_TYPE:
			{
				/* Check buffer actually */
				return_code = 0;
			} break;
			case IO_STREAM_MEMORY_TYPE:
			case IO_STREAM_BZ2_MEMORY_TYPE:
			{
				IO_stream_read_to_internal_buffer(stream);
				return_code = (stream->buffer_index >= stream->buffer_valid_index);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"IO_stream_end_of_stream. IO stream invalid or type not implemented.");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"IO_stream_end_of_stream. Invalid arguments.");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* IO_stream_end_of_stream */

int IO_stream_scan(struct IO_stream *stream, char *format, ...)
/*******************************************************************************
LAST MODIFIED : 23 August 2004

DESCRIPTION :
Equivalent to a standard C fscanf or sscanf on the stream.
==============================================================================*/
{
	char *index1, *index2, local_buffer[1000];
	int count, keep_scanning, length, local_counter, return_code;
	va_list arguments;
	void *va_pointer;
#if defined IO_STREAM_SPEED_UP_SSCANF
	char temp;
	int scan, temp_offset;
#endif /* defined IO_STREAM_SPEED_UP_SSCANF */

	ENTER(IO_stream_scan);

	if (stream && format)
	{
		switch (stream->type)
		{
			case IO_STREAM_FILE_TYPE:
			{
				va_start(arguments, format);
				return_code = vfscanf(stream->file_handle, format, arguments);
				va_end(arguments);
			} break;
			case IO_STREAM_GZIP_FILE_TYPE:
			case IO_STREAM_BZ2_FILE_TYPE:
			case IO_STREAM_MEMORY_TYPE:
			case IO_STREAM_BZ2_MEMORY_TYPE:
			{
				IO_stream_read_to_internal_buffer(stream);
				/* Start at 0 and increment for each sucessful value read to be
					compatible with fscanf */
				return_code = 0;
				keep_scanning = 1;

				va_start(arguments, format);
				index1 = format;
				index2 = strchr(index1, '%');
				local_counter = 0;
				if (index2)
				{
					length = index2 - index1;
				}
				else
				{
					length = strlen (index1);
				}
				if (length)
				{
					strncpy(local_buffer, index1, length);
					sprintf(local_buffer + length, "%%n");
					count = -1;

#if defined IO_STREAM_SPEED_UP_SSCANF
					scan = 1;
					while (scan)
					{
						scan = 0;

						temp_offset = stream->buffer_index + stream->buffer_lookahead;
						temp = stream->buffer[temp_offset];
						stream->buffer[temp_offset] = 0;
#endif /* defined IO_STREAM_SPEED_UP_SSCANF */
					
						if ((0 <= sscanf(stream->buffer + stream->buffer_index, 
									local_buffer, &count)) && (count != -1))
						{
							stream->buffer_index += count;
							local_counter += count;
						}
						else
						{
							keep_scanning = 0;
						}
					
#if defined IO_STREAM_SPEED_UP_SSCANF
						stream->buffer[temp_offset] = temp;

						if (count == stream->buffer_lookahead)
						{
							stream->buffer_index -= count;
							local_counter -= count;
							stream->buffer_lookahead += 10;
							scan = 1;
						}
					}
#endif /* defined IO_STREAM_SPEED_UP_SSCANF */

				}
				index1 = index2;
				if (index1)
				{
					index2 = strchr(index1 + 1, '%');
					if (index2 && (index2 - index1 < 2))
					{
						/* Special case for %%, could keep looking but this is rare */
						index2 = index1 + 2;
					}
				}
				while (keep_scanning && index1)
				{
					if (index2)
					{
						length = index2 - index1;
						strncpy(local_buffer, index1, length);
						sprintf(local_buffer + length, "%%n");
					}
					else
					{
						/* scan to the end */
						strcpy(local_buffer, index1);
						sprintf(local_buffer + strlen(local_buffer), "%%n");
					}
					count = -1;
					if (local_buffer[1] == '*')
					{
#if defined IO_STREAM_SPEED_UP_SSCANF
						scan = 1;
						while (scan)
						{
							scan = 0;

							temp_offset = stream->buffer_index + stream->buffer_lookahead;
							temp = stream->buffer[temp_offset];
							stream->buffer[temp_offset] = 0;			
#endif /* defined IO_STREAM_SPEED_UP_SSCANF */
							if ((0 <= sscanf(stream->buffer + stream->buffer_index, 
										local_buffer, &count)) && (count != -1))
							{
								stream->buffer_index += count;
								local_counter += count;
							}
							else
							{
								keep_scanning = 0;
							}
#if defined IO_STREAM_SPEED_UP_SSCANF
							stream->buffer[temp_offset] = temp;

							if (count == stream->buffer_lookahead)
							{
								stream->buffer_index -= count;
								local_counter -= count;
								stream->buffer_lookahead += 10;
								scan = 1;
							}
						}
#endif /* defined IO_STREAM_SPEED_UP_SSCANF */
					}
					else
					{
						va_pointer = va_arg(arguments, void *);
						/* This cannot test for 2 as the string could be %n%n which could
							return 0, instead check if count has been written to */
#if defined IO_STREAM_SPEED_UP_SSCANF
						scan = 1;
						while (scan)
						{
							scan = 0;

							temp_offset = stream->buffer_index + stream->buffer_lookahead;
							temp = stream->buffer[temp_offset];
							stream->buffer[temp_offset] = 0;			
#endif /* defined IO_STREAM_SPEED_UP_SSCANF */
							if ((0 <= sscanf(stream->buffer + stream->buffer_index, 
										local_buffer, va_pointer, &count)) && (count != -1))
							{
								if (local_buffer[1] == 'n')
								{
									*(int *)va_pointer += local_counter;
								}
								else
								{
									return_code++;
								}
								stream->buffer_index += count;
								local_counter += count;
							}
							else
							{
								/* consume white space */
								if (0 <= sscanf(stream->buffer + stream->buffer_index, 
										" %n", &count))
								{
									stream->buffer_index += count;
									local_counter += count;								
								}
								keep_scanning = 0;
							}
#if defined IO_STREAM_SPEED_UP_SSCANF
							stream->buffer[temp_offset] = temp;

							if (count == stream->buffer_lookahead)
							{
								stream->buffer_index -= count;
								local_counter -= count;
								stream->buffer_lookahead += 10;
								scan = 1;
							}
						}
#endif /* defined IO_STREAM_SPEED_UP_SSCANF */
					}
					index1 = index2;
					if (index1)
					{
						index2 = strchr(index1 + 1, '%');
						if (index2 && (index2 - index1 < 2))
						{
							/* Special case for %%, could keep looking but this is rare */
							index2 = index1 + 2;
						}
					}
				}
				va_end(arguments);

			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"IO_stream_scan. IO stream invalid or type not implemented.");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"IO_stream_scan. Invalid arguments.");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* IO_stream_scan */

int IO_stream_getc(struct IO_stream *stream)
/*******************************************************************************
LAST MODIFIED : 23 August 2004

DESCRIPTION :
Equivalent to a standard C fgetc on the stream.
==============================================================================*/
{
	int return_code;

	ENTER(IO_stream_getc);

	if (stream)
	{
		switch (stream->type)
		{
			case IO_STREAM_FILE_TYPE:
			{
				return_code = fgetc(stream->file_handle);
			} break;
			case IO_STREAM_MEMORY_TYPE:
			case IO_STREAM_GZIP_FILE_TYPE:
 			case IO_STREAM_BZ2_FILE_TYPE:
 			case IO_STREAM_BZ2_MEMORY_TYPE:
			{
				IO_stream_read_to_internal_buffer(stream);
				return_code = stream->buffer[stream->buffer_index];
				stream->buffer_index++;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"IO_stream_getc. IO stream invalid or type not implemented.");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"IO_stream_getc. Invalid arguments.");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* IO_stream_getc */

int IO_stream_fread(struct IO_stream *stream, void *ptr, size_t size, size_t nmemb)
/*******************************************************************************
LAST MODIFIED : 28 March 2007

DESCRIPTION :
Equivalent to a standard C fread on the stream (although I have reordered the
parameters so the stream is first).
==============================================================================*/
{
	char *memptr;
	int bytes_this_copy, eof, items_to_read, items_this_copy, return_code;

	ENTER(IO_stream_fread);

	if (stream)
	{
		switch (stream->type)
		{
			case IO_STREAM_FILE_TYPE:
			{
				return_code = fread(ptr, size, nmemb, stream->file_handle);
			} break;
			case IO_STREAM_MEMORY_TYPE:
			case IO_STREAM_GZIP_FILE_TYPE:
			case IO_STREAM_BZ2_FILE_TYPE:
 			case IO_STREAM_BZ2_MEMORY_TYPE:
			{
				eof = 0;
				items_to_read = nmemb;
				memptr = (char *)ptr;
				while (items_to_read && !eof)
				{
					IO_stream_read_to_internal_buffer(stream);
					if (stream->buffer_valid_index > stream->buffer_index)
					{
					
						if ((unsigned)(stream->buffer_valid_index - stream->buffer_index) >=
							(size * items_to_read))
						{
							items_this_copy = items_to_read;
						}
						else
						{
							items_this_copy = 
								(stream->buffer_valid_index - stream->buffer_index) / size;
						}
						bytes_this_copy = items_this_copy * size;
						memcpy(memptr, stream->buffer + stream->buffer_index,
							bytes_this_copy);
						stream->buffer_index += bytes_this_copy;
						memptr += bytes_this_copy;
						items_to_read -= items_this_copy;
					}
					else
					{
						eof = 1;
					}
				}
				return_code = nmemb - items_to_read;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"IO_stream_fread. IO stream invalid or type not implemented.");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"IO_stream_fread. Invalid arguments.");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* IO_stream_fread */

int IO_stream_read_string(struct IO_stream *stream,char *format,char **string_read)
/******************************************************************************
LAST MODIFIED : 23 August 2004

DESCRIPTION :
	A routine for reading in a single string.  It allocates the memory for the
string.  It uses fscanf:
1. the format string follows the format string for scanf, but is for only one %
	item and does not have the %.
2. if field width is not specified, 256 is used.
3. it reads from the stream <stream>.
???DB.  What should be the return code if no characters are read (EOF) ?
=============================================================================*/
{
	int characters_read,format_len,return_code,working_string_len;
	char *working_format,*working_string;

	ENTER(read_string);
	/* check for valid parameters */
	if (stream&&format&&string_read)
	{
		switch (stream->type)
		{
			case IO_STREAM_FILE_TYPE:
			case IO_STREAM_GZIP_FILE_TYPE:
			case IO_STREAM_BZ2_FILE_TYPE:
			case IO_STREAM_MEMORY_TYPE:
			case IO_STREAM_BZ2_MEMORY_TYPE:
			{
				format_len=strlen(format);
				if (!strcmp(format,"s"))
					/* format is s */
				{
					return_code=1;
				}
				else
				{
					if ((format[0]=='[')&&(format[format_len-1]==']'))
						/* format is [ */
					{
						return_code=1;
					}
					else
					{
						/*        print_message(1,"read_string.  Format is not s or [");*/
						display_message(WARNING_MESSAGE,"IO_stream_read_string.  Format is not s or [");
						return_code=0;
					}
				}
				if (return_code)
				{
					/* construct a format for reading the string and counting the characters
						read */
					if (ALLOCATE(working_format,char,format_len+6)&&
						ALLOCATE(working_string,char,1))
					{
						strcpy(working_format,"%80");
						strcat(working_format,format);
						strcat(working_format,"%n");
						/* read in string, allocating extra memory as required */
						working_string_len=0;
						working_string[0]='\0';
						characters_read=80;
						while (return_code&&!IO_stream_end_of_stream(stream)&&(characters_read==80))
						{
							working_string_len += 80;
							if (REALLOCATE(working_string,working_string,char,
									working_string_len+1))
							{
								characters_read=0;
								IO_stream_scan(stream,working_format,working_string+working_string_len-80,
									&characters_read);
							}
							else
							{
								/*            print_message(1,*/
								display_message(WARNING_MESSAGE,
									"read_string.  Could not allocate memory for string");
								return_code=0;
							}
						}
						if (return_code)
						{
							if (working_string)
							{
								REALLOCATE(*string_read,working_string,char,
									strlen(working_string)+1);
							}
							else
							{
								*string_read=(char *)NULL;
							}
						}
						else
						{
							if (working_string)
							{
								DEALLOCATE(working_string);
							}
						}
						DEALLOCATE(working_format);
					}
					else
					{
						/*        print_message(1,*/
						display_message(WARNING_MESSAGE,
							"IO_stream_read_string.  Could not allocate memory for working format");
						return_code=0;
					}
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"IO_stream_read_string. IO stream invalid or type not implemented.");
				return_code = 0;
			} break;
		}
	}
	else
	{
		/*    print_message(1,"read_string.  Invalid argument(s)");*/
		display_message(WARNING_MESSAGE,"IO_stream_read_string.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return(return_code);
} /* IO_stream_read_string */

char *IO_stream_get_location_string(struct IO_stream *stream)
/*******************************************************************************
LAST MODIFIED : 23 August 2004

DESCRIPTION :
Returns an allocated string specifying where the stream is currently positioned
suitable for use in diagnostic messages.
==============================================================================*/
{
	char *string;
	int c,line_number;
	long location,temp_location;

	ENTER(IO_stream_get_location_string);
	string = (char *)NULL;
	line_number = 0;
	if (stream)
	{
		switch (stream->type)
		{
			case IO_STREAM_FILE_TYPE:
			{
				location = ftell(stream->file_handle);
				rewind(stream->file_handle);
				temp_location = ftell(stream->file_handle);
				while (temp_location<location)
				{
					do
					{
						c=fgetc(stream->file_handle);
					} while (('\n'!=c)&&(EOF!=c));
					temp_location = ftell(stream->file_handle);
					line_number++;
				}
				/* Re-set the position in the stream to the original location */
				fseek(stream->file_handle,location,SEEK_SET);
				if (ALLOCATE(string, char, strlen(stream->uri) + 30))
				{
					sprintf(string, "%s line %d", stream->uri, line_number);
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"IO_stream_get_location_string. IO stream invalid or type not implemented.");
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"IO_stream_close. Invalid arguments.");
		string = (char *)NULL;
	}
	LEAVE;

	return (string);
} /* IO_stream_get_location_string */

int IO_stream_read_to_memory(struct IO_stream *stream, void **stream_data,
	int *stream_data_length)
/*******************************************************************************
LAST MODIFIED : 23 March 2007

DESCRIPTION :
==============================================================================*/
{
	char *new_data;
	int bytes_read, read_to_memory_chunk = 10000, return_code, total_read;

	ENTER(IO_stream_read_to_memory);

	if (stream)
	{
		return_code = 1;
		if (!stream->data)
		{
			if (!(ALLOCATE(stream->data, char, read_to_memory_chunk)))
			{
				display_message(ERROR_MESSAGE,
					"IO_stream_read_to_memory. Unable to allocate stream memory data.");
			}
			stream->data_length = read_to_memory_chunk;
		}
		switch (stream->type)
		{
			case IO_STREAM_FILE_TYPE:
			case IO_STREAM_GZIP_FILE_TYPE:
 			case IO_STREAM_BZ2_FILE_TYPE:
 			case IO_STREAM_BZ2_MEMORY_TYPE:
			{
				total_read = 0;
				while (return_code && !IO_stream_end_of_stream(stream))
				{
					if (total_read + read_to_memory_chunk > stream->data_length)
					{
						if (REALLOCATE(new_data, stream->data, char, stream->data_length + read_to_memory_chunk))
						{
							stream->data = new_data;
							stream->data_length += read_to_memory_chunk;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"IO_stream_read_to_memory. Unable to reallocate stream memory data.");
							return_code = 0;
						}
					}
					if (return_code)
					{
						switch (stream->type)
						{
							case IO_STREAM_FILE_TYPE:
							{
								bytes_read = fread(stream->data + total_read, 1, read_to_memory_chunk,
									stream->file_handle);
							} break;
							case IO_STREAM_GZIP_FILE_TYPE:
							{
								bytes_read = gzread(stream->gzip_file_handle, stream->data + total_read,
									read_to_memory_chunk);
							} break;
							case IO_STREAM_BZ2_FILE_TYPE:
							{
								bytes_read = BZ2_bzread(stream->bz2_file_handle, stream->data + total_read,
									read_to_memory_chunk);
							} break;
							case IO_STREAM_BZ2_MEMORY_TYPE:
							{
								stream->bz2_memory_stream->next_in = 
									((char *)stream->memory_block->memory_ptr) + stream->memory_block_index;
								stream->bz2_memory_stream->avail_in = stream->memory_block->data_length
									- stream->memory_block_index;

								stream->bz2_memory_stream->next_out = 
									stream->data + total_read;
								stream->bz2_memory_stream->avail_out = read_to_memory_chunk;

								BZ2_bzDecompress(stream->bz2_memory_stream);

								bytes_read = read_to_memory_chunk -
									stream->bz2_memory_stream->avail_out;
								stream->memory_block_index += stream->memory_block->data_length - 
									stream->bz2_memory_stream->avail_in;
							} break;
						}
						total_read += bytes_read;
					}
				}
				if (stream->data_length != total_read)
				{
					REALLOCATE(stream->data, stream->data, char, total_read);
					stream->data_length = total_read;
				}
				if (return_code)
				{
					*stream_data = stream->data;
					*stream_data_length = stream->data_length;
				}
			} break;
			case IO_STREAM_MEMORY_TYPE:
			{
				*stream_data = stream->memory_block->memory_ptr;
				*stream_data_length = stream->memory_block->data_length;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"IO_stream_read_to_memory. IO stream invalid or type not implemented.");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"IO_stream_read_to_memory. Invalid arguments.");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* IO_stream_read_to_memory */

int IO_stream_deallocate_read_to_memory(struct IO_stream *stream)
/*******************************************************************************
LAST MODIFIED : 13 September 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(IO_stream_deallocate_read_to_memory);

	if (stream)
	{
		return_code = 1;
		switch (stream->type)
		{
			case IO_STREAM_FILE_TYPE:
			case IO_STREAM_GZIP_FILE_TYPE:
			case IO_STREAM_BZ2_FILE_TYPE:
			case IO_STREAM_BZ2_MEMORY_TYPE:
			{
				if (stream->data)
				{
					DEALLOCATE(stream->data);
					stream->data_length = 0;
				}
			} break;
			case IO_STREAM_MEMORY_TYPE:
			{
				/* Memory is allocated by memory block, don't free until the 
					memory block is removed or the IO_stream_package is DESTROYed */
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"IO_stream_deallocate_read_to_memory. IO stream invalid or type not implemented.");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"IO_stream_deallocate_read_to_memory. Invalid arguments.");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* IO_stream_deallocate_read_to_memory */

int IO_stream_seek(struct IO_stream *stream, long offset, int whence)
/*******************************************************************************
LAST MODIFIED : 6 December 2004

DESCRIPTION :
Implements the stdio function fseek on stream where possible.
==============================================================================*/
{
	int return_code;
	long location;

	ENTER(IO_stream_seek);

	if (stream)
	{
		return_code = 1;
		switch (stream->type)
		{
			case IO_STREAM_FILE_TYPE:
			{
				return_code = !fseek(stream->file_handle, offset, whence);
				stream->buffer_valid_index = 0;
			} break;
			case IO_STREAM_GZIP_FILE_TYPE:
			{
				return_code = !gzseek(stream->file_handle, offset, whence);
				stream->buffer_valid_index = 0;
			} break;
			case IO_STREAM_BZ2_FILE_TYPE:
			{
				display_message(ERROR_MESSAGE, "IO_stream_seek. "
					"Unable to seek on bz2 compressed files currently.");
				return_code = 0;
			} break;
			case IO_STREAM_MEMORY_TYPE:
			{
				switch (whence)
				{
					case SEEK_SET:
					{
						location = offset;
					} break;
					case SEEK_CUR:
					{
						location = stream->memory_block_index + offset;
					} break;
					case SEEK_END:
					{
						location = stream->memory_block->data_length + offset;
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"IO_stream_seek. Unknown seek type.");
						return_code = 0;
					}
				}
				if (return_code)
				{
					if ((location >= 0) && (location < stream->memory_block->data_length))
					{
						stream->memory_block_index = location;
						stream->buffer_valid_index = 0;
						stream->buffer_index = 0;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"IO_stream_seek. Attempt to seek out of memory block.");
						return_code = 0;	
					}
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"IO_stream_seek. IO stream invalid or type not implemented.");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"IO_stream_seek. Invalid arguments.");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* IO_stream_seek */

int IO_stream_close(struct IO_stream *stream)
/*******************************************************************************
LAST MODIFIED : 23 August 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(IO_stream_close);

	if (stream)
	{
		IO_stream_deallocate_read_to_memory(stream);
		switch (stream->type)
		{
			case IO_STREAM_FILE_TYPE:
			{
				fclose(stream->file_handle);
				stream->type = IO_STREAM_UNKNOWN_TYPE;
			} break;
			case IO_STREAM_GZIP_FILE_TYPE:
			{
				gzclose(stream->gzip_file_handle);
				stream->type = IO_STREAM_UNKNOWN_TYPE;
			} break;
			case IO_STREAM_BZ2_FILE_TYPE:
			{
				BZ2_bzclose(stream->bz2_file_handle);
				stream->type = IO_STREAM_UNKNOWN_TYPE;
			} break;
			case IO_STREAM_MEMORY_TYPE:
			{
				if (stream->memory_block)
				{
					DEACCESS(IO_memory_block)(&stream->memory_block);
				}
				stream->type = IO_STREAM_UNKNOWN_TYPE;
			} break;
			case IO_STREAM_BZ2_MEMORY_TYPE:
			{
				if (stream->memory_block)
				{
					DEACCESS(IO_memory_block)(&stream->memory_block);
				}
				if (stream->bz2_memory_stream)
				{
					BZ2_bzDecompressEnd(stream->bz2_memory_stream);
					DEALLOCATE(stream->bz2_memory_stream);
				}
				stream->type = IO_STREAM_UNKNOWN_TYPE;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"IO_stream_close. IO stream invalid or type not implemented.");
				return_code = 0;
			} break;
		}
		if (stream->uri)
		{
			DEALLOCATE(stream->uri);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"IO_stream_close. Invalid arguments.");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* IO_stream_close */

int DESTROY(IO_stream)(struct IO_stream **stream_address)
/*******************************************************************************
LAST MODIFIED : 23 August 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct IO_stream *stream;

	ENTER(DESTROY(IO_stream));

	if (stream_address && (stream = *stream_address))
	{
		if (IO_STREAM_UNKNOWN_TYPE != stream->type)
		{
			IO_stream_close(stream);
			IO_stream_deallocate_read_to_memory(stream);
		}
		if (stream->uri)
		{
			DEALLOCATE(stream->uri);
		}
		if (stream->buffer)
		{
			DEALLOCATE(stream->buffer);
		}
		DEALLOCATE(*stream_address);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(IO_stream). Invalid arguments.");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* DESTROY(IO_stream) */

