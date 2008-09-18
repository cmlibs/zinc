/*******************************************************************************
FILE : io_stream.h

LAST MODIFIED : 6 December 2004

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
#if !defined (IO_STREAM_H)
#define IO_STREAM_H

#include "general/object.h"

/*
Global types
------------
*/

struct IO_stream_package;

struct IO_stream;

/*
Global functions
----------------
*/

struct IO_stream_package *CREATE(IO_stream_package)(void);
/*******************************************************************************
LAST MODIFIED : 4 August 2004

DESCRIPTION :
==============================================================================*/

int IO_stream_package_define_memory_block(struct IO_stream_package *stream_class,
	char *block_name, void *memory_block, int memory_block_length);
/*******************************************************************************
LAST MODIFIED : 16 September 2004

DESCRIPTION :
==============================================================================*/

int IO_stream_package_free_memory_block(struct IO_stream_package *stream_class,
	char *block_name);
/*******************************************************************************
LAST MODIFIED : 17 September 2004

DESCRIPTION :
==============================================================================*/

int DESTROY(IO_stream_package)(struct IO_stream_package **stream_class_address);
/*******************************************************************************
LAST MODIFIED : 4 August 2004

DESCRIPTION :
==============================================================================*/

int IO_stream_uri_is_native_imagemagick(char *stream_uri);
/*******************************************************************************
LAST MODIFIED : 3 September 2004

DESCRIPTION :
Returns <true> if imagemagick can handle the image pointed to by <stream_uri>
directly which will probably be most efficient.
If not then image_utilities will read it all into a memory buffer and pass that
along to imagemagick.
==============================================================================*/

struct IO_stream *CREATE(IO_stream)(struct IO_stream_package *stream_class);
/*******************************************************************************
LAST MODIFIED : 4 August 2004

DESCRIPTION :
==============================================================================*/

int IO_stream_open_for_read(struct IO_stream *stream, char *stream_uri);
/*******************************************************************************
LAST MODIFIED : 4 August 2004

DESCRIPTION :
==============================================================================*/

int IO_stream_end_of_stream(struct IO_stream *stream);
/*******************************************************************************
LAST MODIFIED : 23 August 2004

DESCRIPTION :
==============================================================================*/

int IO_stream_scan(struct IO_stream *stream, const char *format, ...);
/*******************************************************************************
LAST MODIFIED : 4 August 2004

DESCRIPTION :
Equivalent to a standard C fscanf or sscanf on the stream.
==============================================================================*/

int IO_stream_fread(struct IO_stream *stream, void *ptr, size_t size, size_t nmemb);
/*******************************************************************************
LAST MODIFIED : 28 March 2007

DESCRIPTION :
Equivalent to a standard C fread on the stream (although I have reordered the
parameters so the stream is first).
==============================================================================*/

int IO_stream_getc(struct IO_stream *stream);
/*******************************************************************************
LAST MODIFIED : 23 August 2004

DESCRIPTION :
Equivalent to a standard C fgetc on the stream.
==============================================================================*/

int IO_stream_read_string(struct IO_stream *stream,const char *format,char **string_read);
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

char *IO_stream_get_location_string(struct IO_stream *stream);
/*******************************************************************************
LAST MODIFIED : 23 August 2004

DESCRIPTION :
Returns an allocated string specifying where the stream is currently positioned
suitable for use in diagnostic messages.
==============================================================================*/

int IO_stream_read_to_memory(struct IO_stream *stream, void **stream_data,
	int *stream_data_length);
/*******************************************************************************
LAST MODIFIED : 13 September 2004

DESCRIPTION :
==============================================================================*/

int IO_stream_seek(struct IO_stream *stream, long offset, int whence);
/*******************************************************************************
LAST MODIFIED : 6 December 2004

DESCRIPTION :
Implements the stdio function fseek on stream where possible.
==============================================================================*/

int IO_stream_deallocate_read_to_memory(struct IO_stream *stream);
/*******************************************************************************
LAST MODIFIED : 13 September 2004

DESCRIPTION :
==============================================================================*/

int IO_stream_close(struct IO_stream *stream);
/*******************************************************************************
LAST MODIFIED : 4 August 2004

DESCRIPTION :
==============================================================================*/

int DESTROY(IO_stream)(struct IO_stream **stream_address);
/*******************************************************************************
LAST MODIFIED : 4 August 2004

DESCRIPTION :
==============================================================================*/
#endif /* !defined (IO_STREAM_H) */
