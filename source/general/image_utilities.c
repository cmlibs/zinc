/*******************************************************************************
FILE : image_utilities.c

LAST MODIFIED : 20 December 2004

DESCRIPTION :
Utilities for handling images.
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
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "general/debug.h"
#include "general/enumerator_private.h"
#include "general/image_utilities.h"
#include "general/indexed_list_private.h"
#include "general/io_stream.h"
#include "general/myio.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#if defined (IMAGEMAGICK)
/* image magick interfaces */
#include "magick/api.h"
#endif /* defined (IMAGEMAGICK) */

/* #define DEBUG 1 */

/*
Module constants
----------------
*/

#if ! defined (IMAGEMAGICK)

#define TIFF_HI_LO 0x4D4D
#define TIFF_LO_HI 0x4949

/* field types */
#define TIFF_BYTE_FIELD 1
#define TIFF_ASCII_FIELD 2
#define TIFF_SHORT_FIELD 3
#define TIFF_LONG_FIELD 4
#define TIFF_RATIONAL_FIELD 5
#define TIFF_SBYTE_FIELD 6
#define TIFF_UNDEFINED_FIELD 7
#define TIFF_SSHORT_FIELD 8
#define TIFF_SLONG_FIELD 9
#define TIFF_SRATIONAL_FIELD 10
#define TIFF_FLOAT_FIELD 11
#define TIFF_DOUBLE_FIELD 12
/* photometric interpretation */
#define TIFF_WHITE_IS_ZERO 0
#define TIFF_BLACK_IS_ZERO 1
#define TIFF_RGB 2
#define TIFF_PALETTE_COLOUR 3
/* resolution */
#define TIFF_NO_ABSOLUTE_UNIT 1
#define TIFF_INCH 2
#define TIFF_CENTIMETER 3
/* compression */
#define TIFF_NO_COMPRESSION_VALUE 1
#define TIFF_HUFFMAN_COMPRESSION_VALUE 2
#define TIFF_LZW_COMPRESSION_VALUE 5
#define TIFF_PACK_BITS_COMPRESSION_VALUE 32773
/* planar configuration */
#define TIFF_CHUNKY_PLANAR_CONFIGURATION 1
#define TIFF_PLANAR_PLANAR_CONFIGURATION 2
#endif /* ! defined (IMAGEMAGICK) */

/*
Module types
------------
*/

#if !defined (IMAGEMAGICK)
enum Image_orientation
{
	LANDSCAPE_ORIENTATION,
	PORTRAIT_ORIENTATION
}; /* enum Image_orientation */
#endif /* ! defined (IMAGEMAGICK) */

struct Cmgui_image_information
/*******************************************************************************
LAST MODIFIED : 11 March 2002

DESCRIPTION :
Private structure for describing information needed to read or create a
Cmgui_image. Note not all members are needed for each task; file names are
needed for reading files, but only certain raw file types need width, height
and other information to be specified before they can be read.
If more than one file_name is included, they must have consistent width, height
and other attributes.
For creating a blank Cmgui_image, file names are ignored but most other
parameters are used to set the image dimensions and colour depth.
==============================================================================*/
{
	int valid; /* will be set to zero if not set up properly */
	int number_of_file_names;
	char **file_names;
	/* following can be used to override format inferred from file extension */
	enum Image_file_format image_file_format;
	int height, number_of_bytes_per_component, number_of_components, width;
	enum Raw_image_storage raw_image_storage;
	int background_number_of_fill_bytes;
	unsigned char *background_fill_bytes;
	struct IO_stream_package *io_stream_package;
};

struct Cmgui_image
/*******************************************************************************
LAST MODIFIED : 26 February 2002

DESCRIPTION :
Private structure for storing 2-D images.
==============================================================================*/
{
#if defined (IMAGEMAGICK)
	/* Image magick images are stored in bottom-to-top format */
	Image *magick_image;
#else /* defined (IMAGEMAGICK) */
	/* simple image_array storage is from top-to-bottom */
	unsigned char **image_arrays;
#endif /* defined (IMAGEMAGICK) */
	int width, height;
	int number_of_components;
	int number_of_bytes_per_component;
	int number_of_images;
};

#if ! defined (IMAGEMAGICK)
struct Colour_map_entry
/*******************************************************************************
LAST MODIFIED : 20 May 1998

DESCRIPTION :
For creating a colour map and index image in write_tiff_image_file.
==============================================================================*/
{
	int index;
	/* identifier points to itself */
	struct Colour_map_entry *identifier;
	unsigned char blue,green,red;
	int access_count;
}; /* struct Colour_map_entry */

DECLARE_LIST_TYPES(Colour_map_entry);

FULL_DECLARE_INDEXED_LIST_TYPE(Colour_map_entry);
#endif /* ! defined (IMAGEMAGICK) */

/*
Module functions
----------------
*/
#if ! defined (IMAGEMAGICK)
DECLARE_DEFAULT_DESTROY_OBJECT_FUNCTION(Colour_map_entry)

DECLARE_OBJECT_FUNCTIONS(Colour_map_entry)

static int compare_Colour_map_entry(struct Colour_map_entry *entry_1,
	struct Colour_map_entry *entry_2)
{
	int return_code;

	ENTER(compare_Colour_map_entry);
	if (entry_1&&entry_2)
	{
		if (entry_1->red==entry_2->red)
		{
			if (entry_1->green==entry_2->green)
			{
				if (entry_1->blue==entry_2->blue)
				{
					return_code=0;
				}
				else
				{
					if (entry_1->blue<entry_2->blue)
					{
						return_code= -1;
					}
					else
					{
						return_code=1;
					}
				}
			}
			else
			{
				if (entry_1->green<entry_2->green)
				{
					return_code= -1;
				}
				else
				{
					return_code=1;
				}
			}
		}
		else
		{
			if (entry_1->red<entry_2->red)
			{
				return_code= -1;
			}
			else
			{
				return_code=1;
			}
		}
	}
	else
	{
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* compare_Colour_map_entry */

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Colour_map_entry,identifier, \
	struct Colour_map_entry *,compare_Colour_map_entry)

DECLARE_INDEXED_LIST_FUNCTIONS(Colour_map_entry)

DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Colour_map_entry, \
	identifier,struct Colour_map_entry *,compare_Colour_map_entry)

static int byte_swap(unsigned char *byte_array,int value_size,
	int number_of_values,int least_to_most)
/*******************************************************************************
LAST MODIFIED : 16 April 2000

DESCRIPTION :
To take care of little/big endian.
==============================================================================*/
{
	int return_code;
#if defined (BYTE_ORDER)
	int i,j;
	unsigned char *bottom_byte,byte,*element,*top_byte;
#endif /* defined (BYTE_ORDER) */

	ENTER(byte_swap);
	USE_PARAMETER(byte_array);
	USE_PARAMETER(value_size);
	USE_PARAMETER(number_of_values);
	USE_PARAMETER(least_to_most);
	return_code=1;
#if defined (BYTE_ORDER)
#if (1234==BYTE_ORDER)
	if (!least_to_most)
#else /* (1234==BYTE_ORDER) */
	if (least_to_most)
#endif /* (1234==BYTE_ORDER) */
	{
		element=byte_array;
		for (j=number_of_values;j>0;j--)
		{
			bottom_byte=element;
			top_byte=element+value_size;
			for (i=value_size/2;i>0;i--)
			{
				top_byte--;
				byte= *bottom_byte;
				*bottom_byte= *top_byte;
				*top_byte=byte;
				bottom_byte++;
			}
			element += value_size;
		}
	}
#endif /* defined (BYTE_ORDER) */
	LEAVE;

	return (return_code);
} /* byte_swap */

static int byte_swap_and_write(unsigned char *byte_array,int value_size,
	int number_of_values,int least_to_most,FILE *output_file)
/*******************************************************************************
LAST MODIFIED : 31 August 2000

DESCRIPTION :
Performs the byte swap and write, copying if necessary so that
the original values are not modified.
???DB.  Should be combined with functions in general/myio
==============================================================================*/
{
	int return_code;
#if defined (BYTE_ORDER)
	int i,j;
	unsigned char *bottom_byte,byte,*element,*temp_byte_array,*top_byte;
#endif /* defined (BYTE_ORDER) */

	ENTER(byte_swap_and_write);
	return_code=0;
#if defined (BYTE_ORDER)
#if (1234==BYTE_ORDER)
	if (!least_to_most)
#else /* (1234==BYTE_ORDER) */
	if (least_to_most)
#endif /* (1234==BYTE_ORDER) */
	{
		/* we must copy the bytes before reordering so as not to mess up the 
			original data */
		if (ALLOCATE(temp_byte_array,unsigned char,value_size*number_of_values))
		{
			memcpy(temp_byte_array,byte_array,value_size*number_of_values);
			element=temp_byte_array;
			for (j=number_of_values;j>0;j--)
			{
				bottom_byte=element;
				top_byte=element+value_size;
				for (i=value_size/2;i>0;i--)
				{
					top_byte--;
					byte= *bottom_byte;
					*bottom_byte= *top_byte;
					*top_byte=byte;
					bottom_byte++;
				}
				element += value_size;
			}
			return_code =
				fwrite(temp_byte_array,value_size,number_of_values,output_file);
			DEALLOCATE(temp_byte_array);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"byte_swap_and_write.  Unable to allocate memory for temp byte array");
			return_code = 0;
		}
	}
	else
	{
		return_code=fwrite(byte_array,value_size,number_of_values,output_file);
	}
#else /* defined (BYTE_ORDER) */
	USE_PARAMETER(least_to_most);
	return_code=fwrite(byte_array,value_size,number_of_values,output_file);
#endif /* defined (BYTE_ORDER) */
	LEAVE;

	return (return_code);
} /* byte_swap_and_write */

static int read_and_byte_swap(unsigned char *byte_array,int value_size,
	int number_of_values,int least_to_most,FILE *input_file)
/*******************************************************************************
LAST MODIFIED : 31 August 2000

DESCRIPTION :
Performs the read and byte.
???DB.  Should be combined with functions in general/myio
==============================================================================*/
{
	int return_code;
#if defined (BYTE_ORDER)
	int i,j;
	unsigned char *bottom_byte,byte,*element,*top_byte;
#endif /* defined (BYTE_ORDER) */

	ENTER(read_and_byte_swap);
	return_code=0;
#if defined (BYTE_ORDER)
#if (1234==BYTE_ORDER)
	if (!least_to_most)
#else /* (1234==BYTE_ORDER) */
	if (least_to_most)
#endif /* (1234==BYTE_ORDER) */
	{
		if (number_of_values==(return_code=fread(byte_array,value_size,
			number_of_values,input_file)))
		{
			element=byte_array;
			for (j=number_of_values;j>0;j--)
			{
				bottom_byte=element;
				top_byte=element+value_size;
				for (i=value_size/2;i>0;i--)
				{
					top_byte--;
					byte= *bottom_byte;
					*bottom_byte= *top_byte;
					*top_byte=byte;
					bottom_byte++;
				}
				element += value_size;
			}
		}
	}
	else
	{
		return_code=fread(byte_array,value_size,number_of_values,input_file);
	}
#else /* defined (BYTE_ORDER) */
	USE_PARAMETER(least_to_most);
	return_code=fread(byte_array,value_size,number_of_values,input_file);
#endif /* defined (BYTE_ORDER) */
	LEAVE;

	return (return_code);
} /* read_and_byte_swap */

/* define following macros to guarantee that unsigned integers stored in byte
	 arrays with most significant byte first are correctly read into the given
	 unsigned integer types, the sizeof which differs between machines, hance we
	 cannot simply cast it. */

/* following work for 32-bit versions only */
#define UNSIGNED_LONG_INT_FROM_4_BYTES(byte_array) *((unsigned long int *)(byte_array))
#define UNSIGNED_SHORT_INT_FROM_2_BYTES(byte_array) *((unsigned short int *)(byte_array))

/* following clashes with the BYTE_ORDER #define which assumes casting will
	 be done. Casting fails on the 64-bit version */
/*
#define UNSIGNED_LONG_INT_FROM_4_BYTES(byte_array) \
	(((unsigned long int)(*(byte_array)) << 24) + \
	 ((unsigned long int)(*(byte_array + 1)) << 16) + \
	 ((unsigned long int)(*(byte_array + 2)) << 8) + \
	 ((unsigned long int)(*(byte_array + 3))))

#define UNSIGNED_SHORT_INT_FROM_2_BYTES(byte_array) \
	(((unsigned short int)(*(byte_array)) << 8) + \
	 ((unsigned short int)(*(byte_array + 1))))
*/

static int read_tiff_field(unsigned short int *tag, unsigned short int *type,
	unsigned long int *count, unsigned char **field_values_address,
	FILE *tiff_file, int least_to_most)
/*******************************************************************************
LAST MODIFIED : 30 March 2001

DESCRIPTION :
For reading a field in an image file directory.
==============================================================================*/
{
	int i,number_of_bytes,number_of_value_components,return_code,
		value_component_size;
	long int current_file_position;
	unsigned char byte_array[4],*value;
	unsigned long int file_offset;

	ENTER(read_tiff_field);
	/* read field information */
	return_code=1;
#if defined (DEBUG)
	/*???debug */
	printf("Field");
#endif /* defined (DEBUG) */
	/* read tag */
	if (1==read_and_byte_swap(byte_array,2,1,least_to_most,tiff_file))
	{
		*tag = UNSIGNED_SHORT_INT_FROM_2_BYTES(byte_array);
#if defined (DEBUG)
		/*???debug */
		printf(".  tag=%d",*tag);
#endif /* defined (DEBUG) */
		/* read field type: short, long, rational ... */
		if (1==read_and_byte_swap(byte_array,2,1,least_to_most,
			tiff_file))
		{
			*type = UNSIGNED_SHORT_INT_FROM_2_BYTES(byte_array);
#if defined (DEBUG)
			/*???debug */
			printf(", type=");
#endif /* defined (DEBUG) */
			switch (*type)
			{
				case TIFF_BYTE_FIELD:
				{
#if defined (DEBUG)
					/*???debug */
					printf("BYTE");
#endif /* defined (DEBUG) */
					number_of_value_components=1;
					value_component_size=1;
				} break;
				case TIFF_ASCII_FIELD:
				{
#if defined (DEBUG)
					/*???debug */
					printf("ASCII");
#endif /* defined (DEBUG) */
					number_of_value_components=1;
					value_component_size=1;
				} break;
				case TIFF_SHORT_FIELD:
				{
#if defined (DEBUG)
					/*???debug */
					printf("SHORT");
#endif /* defined (DEBUG) */
					number_of_value_components=1;
					value_component_size=2;
				} break;
				case TIFF_LONG_FIELD:
				{
#if defined (DEBUG)
					/*???debug */
					printf("LONG");
#endif /* defined (DEBUG) */
					number_of_value_components=1;
					value_component_size=4;
				} break;
				case TIFF_RATIONAL_FIELD:
				{
#if defined (DEBUG)
					/*???debug */
					printf("RATIONAL");
#endif /* defined (DEBUG) */
					number_of_value_components=2;
					value_component_size=4;
				} break;
				case TIFF_SBYTE_FIELD:
				{
#if defined (DEBUG)
					/*???debug */
					printf("SBYTE");
#endif /* defined (DEBUG) */
					number_of_value_components=1;
					value_component_size=1;
				} break;
				case TIFF_UNDEFINED_FIELD:
				{
#if defined (DEBUG)
					/*???debug */
					printf("UNDEFINED");
#endif /* defined (DEBUG) */
					number_of_value_components=1;
					value_component_size=1;
				} break;
				case TIFF_SSHORT_FIELD:
				{
#if defined (DEBUG)
					/*???debug */
					printf("SSHORT");
#endif /* defined (DEBUG) */
					number_of_value_components=1;
					value_component_size=2;
				} break;
				case TIFF_SLONG_FIELD:
				{
#if defined (DEBUG)
					/*???debug */
					printf("SLONG");
#endif /* defined (DEBUG) */
					number_of_value_components=1;
					value_component_size=4;
				} break;
				case TIFF_SRATIONAL_FIELD:
				{
#if defined (DEBUG)
					/*???debug */
					printf("SRATIONAL");
#endif /* defined (DEBUG) */
					number_of_value_components=2;
					value_component_size=4;
				} break;
				case TIFF_FLOAT_FIELD:
				{
#if defined (DEBUG)
					/*???debug */
					printf("FLOAT");
#endif /* defined (DEBUG) */
					number_of_value_components=1;
					value_component_size=4;
				} break;
				case TIFF_DOUBLE_FIELD:
				{
#if defined (DEBUG)
					/*???debug */
					printf("DOUBLE");
#endif /* defined (DEBUG) */
					number_of_value_components=1;
					value_component_size=8;
				} break;
				default:
				{
#if defined (DEBUG)
					/*???debug */
					printf("UNKNOWN");
#endif /* defined (DEBUG) */
					number_of_value_components=0;
					value_component_size=0;
				} break;
			}
			/* read field count: (how many values are present) */
			if (1==read_and_byte_swap(byte_array,4,1,least_to_most,
				tiff_file))
			{
				*count = UNSIGNED_LONG_INT_FROM_4_BYTES(byte_array);
#if defined (DEBUG)
				/*???debug */
				printf(", count=%ld",*count);
#endif /* defined (DEBUG) */
				/* read value/offset */
				if ((4==fread(byte_array,sizeof(char),4,tiff_file)))
				{
#if defined (DEBUG)
				/*???debug */
					printf(", byte_array=%ld",UNSIGNED_LONG_INT_FROM_4_BYTES(byte_array));
#endif /* defined (DEBUG) */
					number_of_bytes=(int)
						((*count)*number_of_value_components*value_component_size);
					if (0<number_of_bytes)
					{
						if (ALLOCATE(value, unsigned char, number_of_bytes))
						{
							if (4<number_of_bytes)
							{
								byte_swap(byte_array,4,1,least_to_most);
								current_file_position=ftell(tiff_file);
								file_offset = UNSIGNED_LONG_INT_FROM_4_BYTES(byte_array);
								if (0 == fseek(tiff_file, (signed long int)file_offset, SEEK_SET))
								{
									if (number_of_bytes==(int)fread(value,1,number_of_bytes,tiff_file))
									{
										if (0!=fseek(tiff_file,current_file_position,SEEK_SET))
										{
											display_message(ERROR_MESSAGE,
							"read_tiff_field.  Repositioning file after reading field value");
											return_code=0;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"read_tiff_field.  Reading field value %d %ld",
											number_of_bytes, file_offset);
										return_code = 0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"read_tiff_field.  Could not find field value");
									return_code=0;
								}
							}
							else
							{
								byte_swap(byte_array,value_component_size,
								  (int)(*count)*number_of_value_components,least_to_most);
#if defined (DEBUG)
								/*???debug */
								printf(", swapped_byte_array=%lx",
									UNSIGNED_LONG_INT_FROM_4_BYTES(byte_array));
#endif /* defined (DEBUG) */
								for (i=0;i<number_of_bytes;i++)
								{
									value[i]=byte_array[i];
								}
							}
							if (return_code)
							{
#if defined (DEBUG)
								/*???debug */
								printf(", value_address=%p",value);
								printf(", value[0]=%x", *value);
#endif /* defined (DEBUG) */
								*field_values_address = value;
								/* If 4 >= number_of_bytes then the byte order is
									already swapped */
								if (4<number_of_bytes)
								{
									return_code=byte_swap(value,value_component_size,
										(int)(*count)*number_of_value_components,least_to_most);
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_tiff_field.  Insufficient memory for field value");
							return_code=0;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"read_tiff_field.  Could not read the field offset/value");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_tiff_field.  Could not read field count");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_tiff_field.  Could not read field type");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_tiff_field.  Could not read field tag");
		return_code=0;
	}
#if defined (DEBUG)
	/*???debug */
	printf("\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* read_tiff_field */
#endif /* ! defined (IMAGEMAGICK) */

/*
Global functions
----------------
*/

int Open_image_environment(char *program_name)
/*******************************************************************************
LAST MODIFIED : 1 March 2002

DESCRIPTION :
Sets up the image library. Must be called before using any image I/O
functions, ie. at the start of the program.
==============================================================================*/
{
	int return_code;

	ENTER(Open_image_environment);
	if (program_name)
	{
#if defined (IMAGEMAGICK)
#if ! defined (SGI)
		/* Not calling this on IRIX for the time being as this routine messes with the LOCALE
			and installs some signal handlers.  These signal handlers caused problems
			on IRIX when checking for floating point exceptions.
			Not doing this could mean that imagemagick does not close itself down correctly,
			especially if it has made temporary files. */
		InitializeMagick(program_name);
#endif /* ! defined (SGI) */
#endif /* defined (IMAGEMAGICK) */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Open_image_environment.  Missing program name.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Open_image_environment */

int Close_image_environment(void)
/*******************************************************************************
LAST MODIFIED : 31 March 2006

DESCRIPTION :
Called to finialise the use of the image environment
==============================================================================*/
{
	int return_code;

	ENTER(Close_image_environment);

#if defined (IMAGEMAGICK)
#if ! defined (SGI)
	DestroyMagick();
#endif /* ! defined (SGI) */
#endif /* defined (IMAGEMAGICK) */
	return_code = 1;

	LEAVE;

	return (return_code);
} /* Close_image_environment */

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Image_file_format)
{
	char *enumerator_string;

	ENTER(ENUMERATOR_STRING(Image_file_format));
	switch (enumerator_value)
	{
		case BMP_FILE_FORMAT:
		{
			enumerator_string = "bmp";
		} break;
		case JPG_FILE_FORMAT:
		{
			enumerator_string = "jpg";
		} break;
		case GIF_FILE_FORMAT:
		{
			enumerator_string = "gif";
		} break;
		case POSTSCRIPT_FILE_FORMAT:
		{
			enumerator_string = "postscript";
		} break;
		case RAW_FILE_FORMAT:
		{
			enumerator_string = "raw";
		} break;
		case RGB_FILE_FORMAT:
		{
			enumerator_string = "rgb";
		} break;
		case TIFF_FILE_FORMAT:
		{
			enumerator_string = "tiff";
		} break;
		case YUV_FILE_FORMAT:
		{
			enumerator_string = "yuv";
		} break;
		default:
		{
			enumerator_string = (char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Image_file_format) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Image_file_format)

char *Image_file_format_extension(enum Image_file_format image_file_format)
/*******************************************************************************
LAST MODIFIED : 1 March 2002

DESCRIPTION :
Returns the expected file extension stem for a given <image_file_format>. By
stem it is meant that the given characters follow the final . in the file name,
but extra characters may follow. This is especially true for .tif/.tiff and
.yuv#### extensions.
==============================================================================*/
{
	char *file_format_extension;

	ENTER(Image_file_format_extension);
	switch (image_file_format)
	{	
		case BMP_FILE_FORMAT:
		{
			file_format_extension = "bmp";
		} break;
		case JPG_FILE_FORMAT:
		{
			file_format_extension = "jpg";
		} break;
		case GIF_FILE_FORMAT:
		{
			file_format_extension = "gif";
		} break;
		case POSTSCRIPT_FILE_FORMAT:
		{
			file_format_extension = "ps";
		} break;
		case RAW_FILE_FORMAT:
		{
			file_format_extension = "raw";
		} break;
		case RGB_FILE_FORMAT:
		{
			file_format_extension = "rgb";
		} break;
		case TIFF_FILE_FORMAT:
		{
			file_format_extension = "tif";
		} break;
		case YUV_FILE_FORMAT:
		{
			file_format_extension = "yuv";
		} break;
		default:
		{
			file_format_extension = (char *)NULL;
		} break;
	}
	LEAVE;

	return (file_format_extension);
} /* Image_file_format_extension */

int Image_file_format_from_file_name(char *file_name,
	enum Image_file_format *image_file_format_address)
/*******************************************************************************
LAST MODIFIED : 3 May 2001

DESCRIPTION :
Returns the <Image_file_format> determined from the file_extension in
<file_name>, or UNKNOWN_IMAGE_FILE_FORMAT if none found or no match made.
==============================================================================*/
{
	char *file_extension, *other_file_extension;
	enum Image_file_format image_file_format;
	int return_code;

	ENTER(Image_file_format_from_file_name);
	if (file_name && image_file_format_address)
	{
		if (file_extension = strrchr(file_name, '.'))
		{
			file_extension++;
			image_file_format = (enum Image_file_format)0;
			while ((other_file_extension =
				Image_file_format_extension(image_file_format)) &&
				(!fuzzy_string_compare(other_file_extension, file_extension)))
			{
				image_file_format++;
			}
			if (other_file_extension)
			{
				*image_file_format_address = image_file_format;
				return_code = 1;
			}
			else
			{
				return_code = 0;
			}
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Image_file_format_from_file_name.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Image_file_format_from_file_name */

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Raw_image_storage)
{
	char *enumerator_string;

	ENTER(ENUMERATOR_STRING(Raw_image_storage));
	switch (enumerator_value)
	{
		case RAW_INTERLEAVED_RGB:
		{
			enumerator_string = "raw_interleaved_rgb";
		} break;
		case RAW_PLANAR_RGB:
		{
			enumerator_string = "raw_planar_rgb";
		} break;
		default:
		{
			enumerator_string = (char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Raw_image_storage) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Raw_image_storage)

#if defined (OLD_CODE)
int write_image_file(char *file_name,
	int number_of_components, int number_of_bytes_per_component,
	int width, int height, int row_padding,
	long unsigned *image)
/*******************************************************************************
LAST MODIFIED : 4 March 2002

DESCRIPTION :
<row_padding> indicates a number of bytes that is padding on each row of data 
(textures are required to be in multiples of two).
==============================================================================*/
{
	int return_code;

	ENTER(write_image_file);

	if (file_name&&(0<number_of_components)&&(0<width)&&
		(0<height)&&image)
	{
#if defined (IMAGEMAGICK)
		/* SAB.  Note that the data returned from this routine is
			stored bottom to top and must be flipped to conform
			with the top to bottom storage normally used by Cmgui */

		char *magick_pixel_storage, RGBA[] = "RGBA", RGB[] = "RGB";
		int i, row_size, source_size;
		Image *magick_image;
		ImageInfo *magick_image_info;
		ExceptionInfo magick_exception;
		StorageType magick_storage_type;
		unsigned char *destination, *image_char, *image_data, *source;

		return_code = 1;
		GetExceptionInfo(&magick_exception);

		/* We are double handling the data so that I can put it into
			ImageMagick without modifying ImageMagick to understand our
			row padding and I can reverse the top to bottom order */
		if (ALLOCATE(image_data, unsigned char,
			width * height * number_of_components *
			number_of_bytes_per_component))
		{
			row_size = width * number_of_components *
				number_of_bytes_per_component;
			source_size = (width + row_padding) * number_of_components *
				number_of_bytes_per_component;
			source = (unsigned char *)image + (height - 1) * source_size;
			destination = image_data;
			for (i = 0 ; i < height ; i++)
			{
				memcpy((void *)destination,(void *)source,row_size);
				destination += row_size;
				source -= source_size;
			}
			switch (number_of_components)
			{
				case 3:
				{
					magick_pixel_storage = RGB;
				} break;
				case 4:
				{
					magick_pixel_storage = RGBA;
					/* Invert the alpha channel */
					image_char = (unsigned char *)image_data + 3;
					for (i = 0 ; i < width * height ; i++)
					{
						*image_char = 0xff - *image_char;
						image_char += 4;
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"write_image_file.  Unknown/unimplemented pixel storage");
					return_code = 0;
				} break;
			}
			switch (number_of_bytes_per_component)
			{
				case 1:
				{
					magick_storage_type = CharPixel;
				} break;
				case 2:
				{
					magick_storage_type = ShortPixel;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"write_image_file.  Unknown/unimplemented bytes per pixel");
					return_code = 0;
				} break;
			}
			if (return_code)
			{
				if (magick_image_info=CloneImageInfo((ImageInfo *) NULL))
				{
					if(magick_image=ConstituteImage(width, height, magick_pixel_storage, 
						magick_storage_type, image_data, &magick_exception))
					{
						strcpy(magick_image->filename,file_name);
						if(WriteImage (magick_image_info, magick_image))
						{
							return_code = 1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Could not write image \"%s\"\n", file_name);
							return_code = 0;
						}
						DestroyImage(magick_image);					
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"write_image_file.  Unable to create magick image");
						return_code = 0;
					}
					DestroyImageInfo(magick_image_info);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"write_image_file.  Could not create image information");
					return_code = 0;
				}
			}
			DEALLOCATE(image_data);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_image_file.  Could not allocate memory for image data");
			return_code = 0;
		}
#else /* defined (IMAGEMAGICK) */
		USE_PARAMETER(number_of_bytes_per_component);
		USE_PARAMETER(row_padding);
#endif /* defined (IMAGEMAGICK) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"write_image_file.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_image_file */
#endif /* defined (OLD_CODE) */

static int get_yuv_resolution_from_file_size(int file_size,
	int *width, int *height)
/*******************************************************************************
LAST MODIFIED : 16 February 2002

DESCRIPTION :
Determines the <width> and <height> of a yuv image file of <file_size> if
that file size is that of a common video standard.
Returns 1 if a size is successfully determined, and zero without error if not.
==============================================================================*/
{
	int return_code;

	ENTER(get_yuv_resolution_from_file_size);
	if (width && height)
	{
		return_code = 1;
		if ((1920*1080*2) == file_size)
		{
			/* North American HDTV standard, 2 interlaced fields; put fields
				 side by side in double wide image */
			*width = 3840;
			*height = 540;
		}
		else if ((1920*540*2) == file_size)
		{
			/* North American HDTV standard, single field */
			*width = 1920;
			*height = 540;
		}
		else if ((720*486*2) == file_size)
		{
			/* CCIR-601 NSTC format, 2 interlaced fields; put fields
				 side by side in double wide image */
			*width = 1440;
			*height = 243;
		}
		else if ((720*243*2) == file_size)
		{
			/* CCIR-601 NSTC format, single field */
			*width = 720;
			*height = 243;
		}
		else if ((720*576*2) == file_size)
		{
			/* CCIR-601 PAL format, 2 interlaced fields; put fields
				 side by side in double wide image */
			*width = 1440;
			*height = 288;
		}
		else if ((720*288*2) == file_size)
		{
			/* CCIR-601 PAL format, single field */
			*width = 720;
			*height = 288;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_yuv_resolution_from_file_size.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* get_yuv_resolution_from_file_size */

#if ! defined (IMAGEMAGICK)
int write_rgb_image_file(char *file_name,
	int number_of_components, int number_of_bytes_per_component,
	int number_of_columns, int number_of_rows, int row_padding,
	long unsigned *image)
/*******************************************************************************
LAST MODIFIED : 4 March 2002

DESCRIPTION :
Writes an image in SGI rgb file format.
<row_padding> indicates a number of bytes that is padding on each row of data 
(textures are required to be in multiples of two).
==============================================================================*/
{
	char bytes_per_component,dummy[404],image_name[80],storage;
	FILE *output_file;
	int bytes_per_pixel,i,k,l,least_to_most,return_code;
	long colour_map,maximum_pixel_value,minimum_pixel_value,
	   row_size,row_sizes,row_start,row_starts;
	short magic_number;
	unsigned char last_pixel,maximum_pixel_value_char[4],minimum_pixel_value_char[4],
	   *pixel,run[0x7f],run_length,row_size_char[4],row_start_char[4];
	unsigned short components,dimension,height,width;

	ENTER(write_rgb_image_file);
	least_to_most = 0;
	if (file_name && (0 < number_of_components) && (0 < number_of_columns) &&
		(0 < number_of_rows) && image)
	{
		if (number_of_bytes_per_component == 1)
		{
			/* open the output file */
			if (output_file=fopen(file_name,"wb"))
			{
				/* write the output file header */
				/* found in /etc/magic */
				magic_number=474;
				/* use run length encoding */
				storage=1;
				bytes_per_component=1;
				bytes_per_pixel = 4;
				dimension=3;
				width=number_of_columns;
				height=number_of_rows;
				components=number_of_components;
				minimum_pixel_value=0;
				maximum_pixel_value=255;
				for (i=0;i<404;i++)
				{
					dummy[i]='\0';
				}
				for (i=0;i<80;i++)
				{
					image_name[i]='\0';
				}
				/* normal interpretation (B/W for 1 component, RGB for 3 components
					and RGBA for 4 components) of image file */
				colour_map=0;
				/* using pixel shift operations instead of byte_swap_and_write as this works
					for 64bit versions as well as different 32bit versions */
				minimum_pixel_value_char[0] = (unsigned char)(((0xff000000 & minimum_pixel_value)) >> 24);
				minimum_pixel_value_char[1] = (unsigned char)(((0x00ff0000 & minimum_pixel_value)) >> 16);
				minimum_pixel_value_char[2] = (unsigned char)(((0x0000ff00 & minimum_pixel_value)) >> 8);
				minimum_pixel_value_char[3] = (unsigned char)(((0x000000ff & minimum_pixel_value)));
				maximum_pixel_value_char[0] = (unsigned char)(((0xff000000 & maximum_pixel_value)) >> 24);
				maximum_pixel_value_char[1] = (unsigned char)(((0x00ff0000 & maximum_pixel_value)) >> 16);
				maximum_pixel_value_char[2] = (unsigned char)(((0x0000ff00 & maximum_pixel_value)) >> 8);
				maximum_pixel_value_char[3] = (unsigned char)(((0x000000ff & maximum_pixel_value)));
				if ((1==byte_swap_and_write((unsigned char *)&magic_number,2,1,
					least_to_most,output_file))&&
					(1==byte_swap_and_write((unsigned char *)&storage,1,1,least_to_most,
					output_file))&&
					(1==byte_swap_and_write((unsigned char *)&bytes_per_component,1,1,
					least_to_most,output_file))&&
					(1==byte_swap_and_write((unsigned char *)&dimension,2,1,least_to_most,
					output_file))&&
					(1==byte_swap_and_write((unsigned char *)&width,2,1,least_to_most,
					output_file))&&
					(1==byte_swap_and_write((unsigned char *)&height,2,1,least_to_most,
					output_file))&&
					(1==byte_swap_and_write((unsigned char *)&components,2,1,
					least_to_most,output_file))&&
					(1==fwrite(minimum_pixel_value_char,4,1,output_file))&&
					(1==fwrite(maximum_pixel_value_char,4,1,output_file))&&
					(1==byte_swap_and_write((unsigned char *)dummy,4,1,least_to_most,
					output_file))&&
					(80==byte_swap_and_write((unsigned char *)image_name,1,80,
					least_to_most,output_file))&&
					(1==byte_swap_and_write((unsigned char *)&colour_map,4,1,
					least_to_most,output_file))&&
					(404==byte_swap_and_write((unsigned char *)dummy,1,404,least_to_most,
					output_file)))
				{
					/* write the image header */
				   /* I want to avoid platform dependencies such as sizeof (long) */
					row_starts=(long)(512+number_of_rows*bytes_per_pixel);
					row_sizes=(long)(512+(number_of_components+1)*number_of_rows*
						bytes_per_pixel);
					row_start=(long)(512+2*number_of_components*number_of_rows*
						bytes_per_pixel);
					for (i=number_of_rows-1;i>=0;i--)
					{
						row_sizes -= bytes_per_pixel;
						row_starts -= bytes_per_pixel;
						for (l=0;l<number_of_components;l++)
						{
							fseek(output_file,row_starts+(long)(l*number_of_rows*
								bytes_per_pixel),SEEK_SET);
							row_start_char[0] = (unsigned char)(((0xff000000 & row_start)) >> 24);
							row_start_char[1] = (unsigned char)(((0x00ff0000 & row_start)) >> 16);
							row_start_char[2] = (unsigned char)(((0x0000ff00 & row_start)) >> 8);
							row_start_char[3] = (unsigned char)(((0x000000ff & row_start)));
							fwrite(row_start_char,4,1,output_file);
							fseek(output_file,row_start,SEEK_SET);
							row_size=0;
							pixel=((unsigned char *)image)+number_of_components*
							   i*(number_of_columns+row_padding)+l;
							k=number_of_columns;
							while (k>0)
							{
								run_length=1;
								last_pixel= *pixel;
								k--;
								if (k>0)
								{
									pixel += number_of_components;
									if (*pixel==last_pixel)
									{
										do
										{
											last_pixel= *pixel;
											run_length++;
											k--;
											pixel += number_of_components;
										} while ((k>0)&&(run_length<0x7f)&&(*pixel==last_pixel));
										byte_swap_and_write((unsigned char *)&run_length,
											sizeof(unsigned char),1,least_to_most,output_file);
										byte_swap_and_write((unsigned char *)&last_pixel,
											sizeof(unsigned char),1,least_to_most,output_file);
										row_size += 2;
									}
									else
									{
										run[0]=last_pixel;
										do
										{
											last_pixel= *pixel;
											run[run_length]=last_pixel;
											run_length++;
											k--;
											pixel += number_of_components;
										} while ((k>0)&&(run_length<0x7f)&&(*pixel!=last_pixel));
										row_size += run_length+1;
										run_length |= 0x80;
										byte_swap_and_write((unsigned char *)&run_length,
											sizeof(unsigned char),1,least_to_most,output_file);
										run_length &= 0x7f;
										byte_swap_and_write((unsigned char *)run,
											sizeof(unsigned char),(size_t)run_length,least_to_most,
											output_file);
									}
								}
								else
								{
									byte_swap_and_write((unsigned char *)&run_length,
										sizeof(unsigned char),1,least_to_most,output_file);
									byte_swap_and_write((unsigned char *)&last_pixel,
										sizeof(unsigned char),1,least_to_most,output_file);
									row_size += 2;
								}
							}
							run_length=0;
							byte_swap_and_write((unsigned char *)&run_length,
								sizeof(unsigned char),1,least_to_most,output_file);
							row_size++;
							fseek(output_file,row_sizes+(long)(l*number_of_rows*
								bytes_per_pixel),SEEK_SET);
							row_size_char[0] = (unsigned char)(((0xff000000 & row_size)) >> 24);
							row_size_char[1] = (unsigned char)(((0x00ff0000 & row_size)) >> 16);
							row_size_char[2] = (unsigned char)(((0x0000ff00 & row_size)) >> 8);
							row_size_char[3] = (unsigned char)(((0x000000ff & row_size)));
							fwrite(row_size_char,4,1,output_file);
							row_start += row_size;
						}
					}
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"write_rgb_image_file.  Could not write file heading");
					return_code=0;
				}
				(void)fclose(output_file);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"write_rgb_image_file.  Could not open file");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_rgb_image_file.  Only implemented for one byte per component");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"write_rgb_image_file.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_rgb_image_file */

int write_postscript_image_file(char *file_name,
	int number_of_components, int number_of_bytes_per_component,
	int number_of_columns, int number_of_rows, int row_padding,
	float pixel_aspect_ratio,
	enum Image_orientation image_orientation,
	long unsigned *image)
/*******************************************************************************
LAST MODIFIED : 4 March 2002

DESCRIPTION :
Writes an image in Postscript file format.
???DB.  Currently only outputs RGBA images
<row_padding> indicates a number of bytes that is padding on each row of data 
since textures are required to be in powers of two.
==============================================================================*/
{
	FILE *output_file;
	float image_height,image_width,printer_page_height,printer_page_width,
		scale_mm_to_default;
	int bounding_box_bottom,bounding_box_left,bounding_box_right,bounding_box_top,
		count,i,j, page_bottom_margin_mm, page_height_mm, page_left_margin_mm,
		page_right_margin_mm, page_top_margin_mm, page_width_mm,
		printer_page_height_mm,printer_page_width_mm,return_code;
	unsigned char *pixel;

	ENTER(write_postscript_image_file);
	/*???debug */
	printf("enter write_postscript_image_file %s %d %d %d %p\n",file_name,
		number_of_components,number_of_rows,number_of_columns,image);
	/* check arguments */
	return_code=0;
	if (file_name && (4 == number_of_components) && (0 < number_of_columns) &&
		(0 < number_of_rows) && (0 < pixel_aspect_ratio) && image)
	{
		if (number_of_bytes_per_component == 1)
		{
			/* open the output file */
			if (output_file=fopen(file_name,"wt"))
			{
				/* write the output file header */
				switch (image_orientation)
				{
					case LANDSCAPE_ORIENTATION:
					{
						image_height=(float)number_of_columns*pixel_aspect_ratio;
						image_width=(float)number_of_rows;
					} break;
					default:
					{
						image_height=(float)number_of_rows;
						image_width=(float)number_of_columns*pixel_aspect_ratio;
					} break;
				}
				/* following six constants used to be read in from defaults in 
					 user_interface/printer.c */
				page_width_mm = 210;
				page_height_mm = 297;
				page_left_margin_mm = 10;
				page_right_margin_mm = 10;
				page_bottom_margin_mm = 25;
				page_top_margin_mm = 25;

				printer_page_height_mm = page_height_mm -
					page_top_margin_mm - page_bottom_margin_mm;
				printer_page_width_mm = page_width_mm -
					page_left_margin_mm - page_right_margin_mm;
				/* the default postscript unit is a 72nd of an inch */
				scale_mm_to_default = 72./25.4;
				if (image_height*(float)printer_page_width_mm <
					image_width*(float)printer_page_height_mm)
				{
					bounding_box_left = (int)(scale_mm_to_default*
						(float)(page_left_margin_mm));
					bounding_box_right = (int)(scale_mm_to_default*
						(float)(page_width_mm - page_right_margin_mm));
					bounding_box_top = (int)(scale_mm_to_default*
						(float)(page_height_mm - page_top_margin_mm));
					bounding_box_bottom = (int)(scale_mm_to_default*
						((float)(page_height_mm - page_top_margin_mm) -
							printer_page_width_mm*image_height/image_width));
				}
				else
				{
					bounding_box_left = (int)(scale_mm_to_default*
						(float)(page_left_margin_mm));
					bounding_box_right = (int)(scale_mm_to_default*
						((float)(page_left_margin_mm) +
							printer_page_height_mm*image_width/image_height));
					bounding_box_top = (int)(scale_mm_to_default*
						(float)(page_height_mm - page_top_margin_mm));
					bounding_box_bottom = (int)(scale_mm_to_default*
						(float)(page_bottom_margin_mm));
				}
				/* output encapsulated postscript header */
				(void)fprintf(output_file,"%%!PS-Adobe-3.0 EPSF-3.0\n");
				(void)fprintf(output_file,"%%%%BoundingBox: %d %d %d %d\n",
					bounding_box_left,bounding_box_bottom,bounding_box_right,
					bounding_box_top);
				(void)fprintf(output_file,"%%%%Title: %s\n",file_name);
				(void)fprintf(output_file,"%%%%Creator: cmgui\n");
				(void)fprintf(output_file,"%%%%EndComments\n");
				(void)fprintf(output_file,"%%\n");
				printer_page_width=(float)(bounding_box_right-bounding_box_left);
				printer_page_height=(float)(bounding_box_top-bounding_box_bottom);
				switch (image_orientation)
				{
					case LANDSCAPE_ORIENTATION:
					{
						/* page height is x and page width is y */
						(void)fprintf(output_file,
							"%% page height is x and page width is y\n");
						/* change the origin to the bottom right of the bounding box */
						(void)fprintf(output_file,
							"%% change the origin to the bottom right of the bounding box\n");
						(void)fprintf(output_file,"%d %d translate\n",
							bounding_box_right,bounding_box_bottom);
						/* rotate the axes by 90 degrees counterclockwise */
						(void)fprintf(output_file,
							"%% rotate the axes by 90 degrees counterclockwise\n");
						(void)fprintf(output_file,"90 rotate\n");
						if (image_width*printer_page_width>
							image_height*printer_page_height)
						{
							(void)fprintf(output_file,"%.5g %.5g scale\n",
								printer_page_height/((float)number_of_rows*pixel_aspect_ratio),
								printer_page_height/(float)number_of_rows);
							(void)fprintf(output_file,"0 %.5g translate\n",
								((printer_page_width*(float)number_of_rows)/printer_page_height-
									(float)number_of_rows)/2);
						}
						else
						{
							(void)fprintf(output_file,"%.5g %.5g scale\n",
								printer_page_width/(float)number_of_columns,
								printer_page_width/(float)number_of_columns*pixel_aspect_ratio);
							(void)fprintf(output_file,"%.5g 0 translate\n",
								((printer_page_height*(float)number_of_columns)/
									printer_page_width-(float)number_of_columns)/2);
						}
					} break;
					default:
					{
						/* page height is y and page width is x */
						(void)fprintf(output_file,
							"%% page height is y and page width is x\n");
						/* change the origin to the bottom left of the bounding box */
						(void)fprintf(output_file,
							"%% change the origin to the bottom left of the bounding box\n");
						(void)fprintf(output_file,"%d %d translate\n",bounding_box_left,
							bounding_box_bottom);
						if (image_width*printer_page_height>
							image_height*printer_page_width)
						{
							(void)fprintf(output_file,"%.5g %.5g scale\n",
								printer_page_width/(float)number_of_columns,
								printer_page_width/(float)number_of_columns*pixel_aspect_ratio);
							(void)fprintf(output_file,"0 %.5g translate\n",
								((printer_page_height*(float)number_of_columns)/
									(printer_page_width*pixel_aspect_ratio)-(float)number_of_rows)/2);
						}
						else
						{
							(void)fprintf(output_file,"%.5g %.5g scale\n",
								printer_page_height/((float)number_of_rows*pixel_aspect_ratio),
								printer_page_height/(float)number_of_rows);
							(void)fprintf(output_file,"%.5g 0 translate\n",
								((printer_page_width*(float)number_of_rows*pixel_aspect_ratio)/
									printer_page_height-(float)number_of_columns)/2);
						}
					} break;
				}
				(void)fprintf(output_file,"/scan_line %d string def\n",
					3*number_of_columns);
				/* write the image header */
				/* write the postscript for displaying the image */
				(void)fprintf(output_file,"%d %d\n",number_of_columns,number_of_rows);
				(void)fprintf(output_file,"8\n");
				(void)fprintf(output_file,"[1 0 0 -1 0 %d]\n",number_of_rows);
				(void)fprintf(output_file,"{\n");
				(void)fprintf(output_file,
					"  currentfile scan_line readhexstring pop\n");
				(void)fprintf(output_file,"}\n");
				(void)fprintf(output_file,"false\n");
				(void)fprintf(output_file,"3\n");
				(void)fprintf(output_file,"colorimage\n");
				count=0;
				/* write the image to the file */
				for (i=number_of_rows-1;i>=0;i--)
				{
					pixel=((unsigned char *)image)+i*number_of_components*
					(number_of_columns+row_padding);
					for (j=number_of_columns;j>0;j--)
					{
						(void)fprintf(output_file,"%02x%02x%02x",pixel[0],
							pixel[1],pixel[2]);
						count++;
						if (count>=13)
						{
							count=0;
							(void)fprintf(output_file,"\n");
						}
						pixel += number_of_components;
					}
				}
				if (count!=0)
				{
					(void)fprintf(output_file,"\n");
				}
				(void)fprintf(output_file,"showpage\n");
				(void)fprintf(output_file,"%%%%EOF\n");
				(void)fclose(output_file);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"write_postscript_image_file.  Could not open file");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_postscript_image_file.  Only implemented for one byte per component");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_postscript_image_file.  Missing argument(s)");
		return_code=0;
	}
	/*???debug */
	printf("leave write_postscript_image_file\n");
	LEAVE;

	return (return_code);
} /* write_postscript_image_file */

#define WRITE_LZW_CODE(out_code) \
/*???debug */ \
/*printf("out_code %4x\n",out_code);*/ \
high_byte=(unsigned char)(out_code/256); \
low_byte=(unsigned char)(out_code%256); \
if (bit_count<out_bit_count) \
{ \
	out_bit_count -= bit_count; \
	out_byte |= (high_byte<<out_bit_count); \
} \
else \
{ \
	out_byte |= (high_byte>>(bit_count-out_bit_count)); \
	fwrite(&out_byte,1,1,output_file); \
	/*???debug */ \
	/*printf("out_byte %02x\n",out_byte);*/ \
	number_of_compressed_bytes++; \
	out_bit_count += 8-bit_count; \
	out_byte=high_byte<<out_bit_count; \
} \
out_byte |= (low_byte>>(8-out_bit_count)); \
fwrite(&out_byte,1,1,output_file); \
/*???debug */ \
/*printf("out_byte %02x\n",out_byte);*/ \
number_of_compressed_bytes++; \
out_byte=low_byte<<out_bit_count;

static int LZW_compress_to_file(FILE *output_file,
	unsigned char *uncompressed_bytes,unsigned short number_of_uncompressed_bytes,
	unsigned short *number_of_compressed_bytes_address)
/*******************************************************************************
LAST MODIFIED : 31 July 1998

DESCRIPTION :
Uses LZW compression to compress a stream of bytes to a file.
==============================================================================*/
{
	int bit_count,found,i,j,matched_string_length,out_bit_count,return_code,
		search_length;
	/* limited to 12-bit codes */
	unsigned char high_byte,low_byte,*matched_string,*next_byte,
		**next_string_table_entry,out_byte,*search_string,*string_table[4096];
	unsigned short clear_code,end_of_information_code,matched_code,
		*next_string_table_length_entry,number_of_compressed_bytes,number_of_codes,
		search_code,string_table_length[4096];

	ENTER(LZW_compress_to_file);
#if defined (DEBUG)
	/*???debug */
	/*printf("enter LZW_compress_to_file\n");*/
#endif /* defined (DEBUG) */
	/* check arguments */
	if (output_file&&uncompressed_bytes&&number_of_compressed_bytes_address)
	{
		return_code=1;
		/* initialize the string table */
		next_string_table_entry=string_table;
		next_string_table_length_entry=string_table_length;
		i=0;
		while (return_code&&(i<256))
		{
			if (ALLOCATE(*next_string_table_entry,unsigned char,1))
			{
				(*next_string_table_entry)[0]=(unsigned char)i;
				*next_string_table_length_entry=1;
				next_string_table_entry++;
				next_string_table_length_entry++;
				i++;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"LZW_compress_to_file.  Error initializing string table");
				return_code=0;
			}
		}
		if (return_code)
		{
			/* special codes */
			clear_code=256;
			*next_string_table_entry=(unsigned char *)NULL;
			*next_string_table_length_entry=1;
			next_string_table_entry++;
			next_string_table_length_entry++;
			end_of_information_code=257;
			*next_string_table_entry=(unsigned char *)NULL;
			*next_string_table_length_entry=1;
			next_string_table_entry++;
			next_string_table_length_entry++;
			number_of_codes=258;
			bit_count=1;
			out_bit_count=8;
			out_byte='\0';
			number_of_compressed_bytes=0;
			/* write clear_code */
			WRITE_LZW_CODE(clear_code);
			next_byte=uncompressed_bytes;
			matched_code=(unsigned short)(*next_byte);
#if defined (DEBUG)
			/*???debug */
			/*printf("in_byte %02x\n",*next_byte);*/
#endif /* defined (DEBUG) */
			matched_string_length=1;
			matched_string=string_table[matched_code];
			i=number_of_uncompressed_bytes-1;
			while (return_code&&(i>0))
			{
				next_byte++;
#if defined (DEBUG)
				/*???debug */
				/*printf("in_byte %02x\n",*next_byte);*/
#endif /* defined (DEBUG) */
				search_code=matched_code+1;
				found=0;
				while (!found&&(search_code<number_of_codes))
				{
					search_string=string_table[search_code];
					search_length=string_table_length[search_code];
					if (search_length==matched_string_length+1)
					{
						j=0;
						while ((j<matched_string_length)&&
							(matched_string[j]==search_string[j]))
						{
							j++;
						}
						if ((j==matched_string_length)&&
							(*next_byte==search_string[matched_string_length]))
						{
							found=1;
						}
						else
						{
							search_code++;
						}
					}
					else
					{
						search_code++;
					}
				}
				if (found)
				{
					matched_code=search_code;
					matched_string_length++;
					matched_string=search_string;
				}
				else
				{
					WRITE_LZW_CODE(matched_code);
					if (ALLOCATE(*next_string_table_entry,unsigned char,
						matched_string_length+1))
					{
						search_string= *next_string_table_entry;
						for (j=0;j<matched_string_length;j++)
						{
							search_string[j]=matched_string[j];
						}
						search_string[matched_string_length]= *next_byte;
						*next_string_table_length_entry=matched_string_length+1;
						next_string_table_entry++;
						next_string_table_length_entry++;
						number_of_codes++;
						switch (bit_count)
						{
							case 1:
							{
								if (number_of_codes>=512)
								{
									bit_count=2;
								}
							} break;
							case 2:
							{
								if (number_of_codes>=1024)
								{
									bit_count=3;
								}
							} break;
							case 3:
							{
								if (number_of_codes>=2048)
								{
									bit_count=4;
								}
							} break;
							case 4:
							{
								if (number_of_codes>4095)
								{
									WRITE_LZW_CODE(clear_code);
									bit_count=1;
									/* free the string table */
									for (j=(int)(next_string_table_entry-string_table)-258;j>0;
										j--)
									{
										next_string_table_entry--;
										DEALLOCATE(*next_string_table_entry);
									}
									next_string_table_length_entry=string_table_length+258;
									number_of_codes=258;
								}
							} break;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"LZW_compress_to_file.  Error adding to string table");
						return_code=0;
					}
					matched_code=(unsigned short)(*next_byte);
					matched_string_length=1;
					matched_string=string_table[matched_code];
				}
				i--;
			}
			WRITE_LZW_CODE(matched_code);
			/* write end_of_information_code */
			WRITE_LZW_CODE(end_of_information_code);
			/* finish off last byte */
			if (out_bit_count<8)
			{
				fwrite(&out_byte,1,1,output_file);
#if defined (DEBUG)
				/*???debug */
				/*printf("out_byte %02x\n",out_byte);*/
#endif /* defined (DEBUG) */
				number_of_compressed_bytes++;
			}
			*number_of_compressed_bytes_address=number_of_compressed_bytes;
#if defined (DEBUG)
			/*???debug */
			printf("number_of_codes=%d\n",number_of_codes);
			printf("number_of_compressed_bytes=%d\n",number_of_compressed_bytes);
/*
			printf("table\n");
			for (i=258;i<number_of_codes;i++)
			{
				printf("%02x ",i);
				for (j=0;j<string_table_length[i];j++)
				{
					printf("%02x",(string_table[i])[j]);
				}
				printf("\n");
			}
*/
#endif /* defined (DEBUG) */
		}
		/* clear string table */
		while (next_string_table_entry>string_table)
		{
			next_string_table_entry--;
			DEALLOCATE(*next_string_table_entry);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"LZW_compress_to_file.  Invalid argument(s)");
		return_code=0;
	}
#if defined (DEBUG)
	/*???debug */
	/*printf("leave LZW_compress_to_file\n");*/
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* LZW_compress_to_file */

static int write_tiff_field(unsigned short tag,unsigned short type,
	unsigned long count,unsigned long value_offset,FILE *tiff_file)
/*******************************************************************************
LAST MODIFIED : 31 July 1998

DESCRIPTION :
For writing a field in an image file directory.
==============================================================================*/
{
	int return_code;
	unsigned long value_offset_temp;

	ENTER(write_tiff_field);
	return_code=1;
	fwrite(&tag,sizeof(unsigned short),1,tiff_file);
	fwrite(&type,sizeof(unsigned short),1,tiff_file);
	fwrite(&count,sizeof(unsigned long),1,tiff_file);
#if defined (BYTE_ORDER) && (1234==BYTE_ORDER)
	value_offset_temp=value_offset;
#else /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */
	if (1==count)
	{
		switch (type)
		{
			case TIFF_ASCII_FIELD:
			case TIFF_BYTE_FIELD:
			case TIFF_SBYTE_FIELD:
			{
				value_offset_temp=value_offset<<24;
			} break;
			case TIFF_SHORT_FIELD:
			case TIFF_SSHORT_FIELD:
			{
				value_offset_temp=value_offset<<16;
			} break;
			default:
			{
				value_offset_temp=value_offset;
			} break;
		}
	}
	else
	{
		value_offset_temp=value_offset;
	}
#endif /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */
	fwrite(&value_offset_temp,sizeof(unsigned long),1,tiff_file);
	LEAVE;

	return (return_code);
} /* write_tiff_field */

static int pack_bits_compress_to_file(FILE *output_file,
	unsigned char *uncompressed_bytes,unsigned short number_of_uncompressed_bytes,
	int number_of_columns,unsigned short *number_of_compressed_bytes_address)
/*******************************************************************************
LAST MODIFIED : 7 August 1998

DESCRIPTION :
Uses pack bits compression to compress a stream of bytes to a file.
==============================================================================*/
{
	char out_run_length;
	int i,j,return_code;
	unsigned char last_pixel,*pixel,run[128],run_length;
	unsigned short number_of_compressed_bytes;

	ENTER(pack_bits_compress_to_file);
	return_code=1;
	number_of_compressed_bytes=0;
	pixel=uncompressed_bytes;
	for (i=number_of_uncompressed_bytes;i>0;i -= number_of_columns)
	{
		j=number_of_columns;
		while (j>0)
		{
			run_length=1;
			last_pixel= *pixel;
			j--;
			if (j>0)
			{
				pixel++;
				if (*pixel==last_pixel)
				{
					do
					{
						last_pixel= *pixel;
						run_length++;
						j--;
						pixel++;
					} while ((j>0)&&(run_length<128)&&(*pixel==last_pixel));
					out_run_length=1-run_length;
					fwrite(&out_run_length,sizeof(char),1,output_file);
					fwrite(&last_pixel,sizeof(unsigned char),1,output_file);
					number_of_compressed_bytes += 2;
				}
				else
				{
					run[0]=last_pixel;
					do
					{
						last_pixel= *pixel;
						run[run_length]=last_pixel;
						run_length++;
						j--;
						pixel++;
					} while ((j>0)&&(run_length<128)&&(*pixel!=last_pixel));
					number_of_compressed_bytes += run_length+1;
					out_run_length=run_length-1;
					fwrite(&out_run_length,sizeof(char),1,output_file);
					fwrite(run,sizeof(unsigned char),(size_t)run_length,output_file);
				}
			}
			else
			{
				out_run_length=run_length-1;
				fwrite(&out_run_length,sizeof(char),1,output_file);
				fwrite(&last_pixel,sizeof(unsigned char),1,output_file);
				number_of_compressed_bytes += 2;
			}
		}
	}
	*number_of_compressed_bytes_address=number_of_compressed_bytes;
	LEAVE;

	return (return_code);
} /* pack_bits_compress_to_file */

int write_tiff_image_file(char *file_name,
	int number_of_components, int number_of_bytes_per_component,
	int number_of_columns, int number_of_rows, int row_padding,
	enum Tiff_image_compression compression, long unsigned *image)
/*******************************************************************************
LAST MODIFIED : 4 March 2002

DESCRIPTION :
Writes an <image> in TIFF file format using <compression>.
<row_padding> indicates a number of bytes that is padding on each row of data 
(textures are required to be in multiples of two).

Not working for 64 bit as assumes a long is 4 bytes!
==============================================================================*/
{
	FILE *output_file;
	int i,j,k,number_in_colour_map,return_code,row_number,samples_per_pixel;
	struct Colour_map_entry *colour_map_entry,*colour_map_index;
	struct LIST(Colour_map_entry) *colour_map_list;
	unsigned char byte,*palette_image,*palette_image_pixel,*pixel,
		*uncompressed_byte,*uncompressed_bytes;
	unsigned long compression_value,image_file_directory_offset,
		next_image_file_directory_offset,photometric_interpretation_value,
		planar_configuration_value,strip_byte_counts_start,*strip_offset,
		*strip_offsets,strip_offsets_start,value_offset;
	unsigned short bits_per_sample[8],byte_order,colour_map[3*256],
		image_file_directory_entry_count,number_of_strips,
		number_of_uncompressed_bytes,rows_per_strip,rows_per_strip_save,
		*strip_byte_count,*strip_byte_counts,version;

	ENTER(write_tiff_image_file);
#if defined (DEBUG)
	/*???debug */
	printf("enter write_tiff_image_file\n");
#endif /* defined (DEBUG) */
	/* check arguments */
	return_code=0;
	if (file_name && (0 < number_of_components) && (0 < number_of_columns) &&
		(0 < number_of_rows) && image)
	{
		if (number_of_bytes_per_component == 1)
		{
			/* try to convert to a palette image */
			if (ALLOCATE(palette_image,unsigned char,number_of_rows*number_of_columns))
			{
				pixel=(unsigned char *)image;
				palette_image_pixel=palette_image;
				number_in_colour_map=0;
				if (colour_map_list=CREATE(LIST(Colour_map_entry))())
				{
					if (ALLOCATE(colour_map_entry,struct Colour_map_entry,1))
					{
						colour_map_entry->access_count=0;
						colour_map_entry->identifier=colour_map_entry;
						colour_map_entry->index=number_in_colour_map;
					}
					else
					{
						DESTROY(LIST(Colour_map_entry))(&colour_map_list);
					}
				}
				for (i=0;i<256*3;i++)
				{
					colour_map[i]=(unsigned short)0;
				}
				i=number_of_rows;
				while (colour_map_list&&(number_in_colour_map<=256)&&(i>0))
				{
					i--;
					pixel=((unsigned char *)image)+
				   number_of_components*i*(number_of_columns+row_padding);
					j=number_of_columns;
					while (colour_map_list&&(number_in_colour_map<=256)&&(j>0))
					{
						switch (number_of_components)
						{
							case 1:
							{
								colour_map_entry->red= *pixel;
								colour_map_entry->green= *pixel;
								colour_map_entry->blue= *pixel;
								pixel++;
							} break;
							case 2:
							{
								pixel++;
								colour_map_entry->red= *pixel;
								colour_map_entry->green= *pixel;
								colour_map_entry->blue= *pixel;
								pixel++;
							} break;
							case 3:
							{
								colour_map_entry->red= *pixel;
								pixel++;
								colour_map_entry->green= *pixel;
								pixel++;
								colour_map_entry->blue= *pixel;
								pixel++;
							} break;
							case 4:
							{
								colour_map_entry->red= *pixel;
								pixel++;
								colour_map_entry->green= *pixel;
								pixel++;
								colour_map_entry->blue= *pixel;
								pixel++;
								pixel++;
							} break;
						}
						if (colour_map_index=FIND_BY_IDENTIFIER_IN_LIST(Colour_map_entry,
							identifier)(colour_map_entry,colour_map_list))
						{
							*palette_image_pixel=(unsigned char)(colour_map_index->index);
						}
						else
						{
							if (number_in_colour_map<256)
							{
								if (ADD_OBJECT_TO_LIST(Colour_map_entry)(colour_map_entry,
									colour_map_list))
								{
									colour_map[0+number_in_colour_map]=
									257*(unsigned short)(colour_map_entry->red);
									colour_map[256+number_in_colour_map]=
									257*(unsigned short)(colour_map_entry->green);
									colour_map[512+number_in_colour_map]=
									257*(unsigned short)(colour_map_entry->blue);
									if (ALLOCATE(colour_map_entry,struct Colour_map_entry,1))
									{
										colour_map_entry->access_count=0;
										colour_map_entry->identifier=colour_map_entry;
										colour_map_entry->index=number_in_colour_map+1;
										*palette_image_pixel=(unsigned char)(colour_map_entry->index);
									}
									else
									{
										DESTROY(LIST(Colour_map_entry))(&colour_map_list);
									}
								}
								else
								{
									DESTROY(LIST(Colour_map_entry))(&colour_map_list);
								}
							}
							number_in_colour_map++;
						}
						palette_image_pixel++;
						j--;
					}
				}
#if defined (DEBUG)
				/*???debug */
				printf("number_in_colour_map %d\n",number_in_colour_map);
				printf("i %d\n",i);
#endif /* defined (DEBUG) */
				if (colour_map_list)
				{
					if (number_in_colour_map>256)
					{
						DEALLOCATE(palette_image);
					}
					if (colour_map_entry)
					{
						DEALLOCATE(colour_map_entry);
					}
					DESTROY(LIST(Colour_map_entry))(&colour_map_list);
				}
				else
				{
					DEALLOCATE(palette_image);
				}
			}
			/* aim to have 8K per strip */
			if (palette_image)
			{
				photometric_interpretation_value=TIFF_PALETTE_COLOUR;
				planar_configuration_value=TIFF_CHUNKY_PLANAR_CONFIGURATION;
				samples_per_pixel=1;
			}
			else
			{
				photometric_interpretation_value=TIFF_RGB;
				planar_configuration_value=TIFF_PLANAR_PLANAR_CONFIGURATION;
				samples_per_pixel=3;
			}
			rows_per_strip=1+(8*1024)/(samples_per_pixel*number_of_columns);
			if ((1<rows_per_strip)&&
				(rows_per_strip*samples_per_pixel*number_of_columns-(8*1024)>
					(8*1024)-((rows_per_strip-1)*samples_per_pixel*number_of_columns)))
			{
				rows_per_strip--;
			}
			number_of_strips=(number_of_rows+rows_per_strip-1)/rows_per_strip;
			/* allocate memory for strip byte counts and offsets */
			if (TIFF_PLANAR_PLANAR_CONFIGURATION==planar_configuration_value)
			{
				number_of_strips *= samples_per_pixel;
			}
#if defined (DEBUG)
			/*???debug */
			printf("number_of_rows %d\n",number_of_rows);
			printf("number_of_columns %d\n",number_of_columns);
			printf("rows_per_strip %d\n",rows_per_strip);
			printf("number_of_strips %d\n",number_of_strips);
#endif /* defined (DEBUG) */
			ALLOCATE(strip_byte_counts,unsigned short,number_of_strips);
			ALLOCATE(strip_offsets,unsigned long,number_of_strips);
			if (TIFF_PLANAR_PLANAR_CONFIGURATION==planar_configuration_value)
			{
				number_of_uncompressed_bytes=rows_per_strip*number_of_columns;
			}
			else
			{
				number_of_uncompressed_bytes=rows_per_strip*samples_per_pixel*
				number_of_columns;
			}
			if (!palette_image)
			{
				ALLOCATE(uncompressed_bytes,unsigned char,number_of_uncompressed_bytes);
			}
			if (strip_byte_counts&&strip_offsets&&(palette_image||uncompressed_bytes))
			{
				for (i=0;i<number_of_strips;i++)
				{
					strip_byte_counts[i]=0;
					strip_offsets[i]=0;
				}
				/* open the output file */
				if (output_file=fopen(file_name,"wb"))
				{
					/* write the image file header */
#if defined (BYTE_ORDER) && (1234==BYTE_ORDER)
					byte_order=TIFF_LO_HI;
#else /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */
					byte_order=TIFF_HI_LO;
#endif /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */
					image_file_directory_offset=sizeof(unsigned short);
					fwrite(&byte_order,sizeof(unsigned short),1,output_file);
					version=42;
					image_file_directory_offset += sizeof(unsigned short);
					fwrite(&version,sizeof(unsigned short),1,output_file);
					image_file_directory_offset += sizeof(unsigned long);
					fwrite(&image_file_directory_offset,sizeof(unsigned long),1,
						output_file);
#if defined (DEBUG)
					/*???debug */
					printf("location %d %d\n",ftell(output_file),
						(int)image_file_directory_offset);
#endif /* defined (DEBUG) */
					/* write the image file directory */
					if (palette_image)
					{
						image_file_directory_entry_count=11;
					}
					else
					{
						image_file_directory_entry_count=10;
					}
					/* value offsets should be word aligned (4 ?) */
					value_offset=image_file_directory_offset+sizeof(unsigned short)+
					image_file_directory_entry_count*12+sizeof(unsigned long);
					fwrite(&image_file_directory_entry_count,sizeof(unsigned short),1,
						output_file);
					/* write the fields in ascending tag order */
					/* image width */
					write_tiff_field((unsigned short)256,(unsigned short)TIFF_SHORT_FIELD,
						(unsigned long)1,(unsigned long)number_of_columns,output_file);
					/* image length */
					write_tiff_field((unsigned short)257,(unsigned short)TIFF_SHORT_FIELD,
						(unsigned long)1,(unsigned long)number_of_rows,output_file);
					/* bits per sample */
					if (palette_image)
					{
						write_tiff_field((unsigned short)258,(unsigned short)TIFF_SHORT_FIELD,
							(unsigned long)1,(unsigned long)8,output_file);
					}
					else
					{
						write_tiff_field((unsigned short)258,(unsigned short)TIFF_SHORT_FIELD,
							(unsigned long)3,(unsigned long)value_offset,output_file);
						bits_per_sample[0]=8;
						bits_per_sample[1]=8;
						bits_per_sample[2]=8;
						value_offset += 3*sizeof(unsigned short);
					}
					/* compression */
					switch (compression)
					{
						case TIFF_NO_COMPRESSION:
						{
							compression_value=TIFF_NO_COMPRESSION_VALUE;
						} break;
						case TIFF_LZW_COMPRESSION:
						{
							compression_value=TIFF_LZW_COMPRESSION_VALUE;
						} break;
						case TIFF_PACK_BITS_COMPRESSION:
						{
							compression_value=TIFF_PACK_BITS_COMPRESSION_VALUE;
						} break;
						default:
						{
							compression_value=TIFF_NO_COMPRESSION_VALUE;
						} break;
					}
					write_tiff_field((unsigned short)259,(unsigned short)TIFF_SHORT_FIELD,
						(unsigned long)1,compression_value,output_file);
					/* photometric interpretation */
					write_tiff_field((unsigned short)262,(unsigned short)TIFF_SHORT_FIELD,
						(unsigned long)1,photometric_interpretation_value,output_file);
					/* strip offsets */
					if (1<number_of_strips)
					{
						strip_offsets_start=value_offset;
						write_tiff_field((unsigned short)273,
							(unsigned short)TIFF_LONG_FIELD,(unsigned long)number_of_strips,
							(unsigned long)value_offset,output_file);
						value_offset += number_of_strips*sizeof(unsigned long);
					}
					else
					{
						strip_offsets_start=ftell(output_file)+2*sizeof(unsigned short)+
						sizeof(unsigned long);
						write_tiff_field((unsigned short)273,
							(unsigned short)TIFF_LONG_FIELD,(unsigned long)number_of_strips,
							(unsigned long)value_offset,output_file);
					}
					/* samples per pixel */
					write_tiff_field((unsigned short)277,(unsigned short)TIFF_SHORT_FIELD,
						(unsigned long)1,(unsigned long)samples_per_pixel,output_file);
					/* rows per strip */
					write_tiff_field((unsigned short)278,(unsigned short)TIFF_LONG_FIELD,
						(unsigned long)1,(unsigned long)rows_per_strip,output_file);
					/* strip byte counts */
					if (2<number_of_strips)
					{
						strip_byte_counts_start=value_offset;
						write_tiff_field((unsigned short)279,(unsigned short)TIFF_SHORT_FIELD,
							(unsigned long)number_of_strips,(unsigned long)value_offset,
							output_file);
						value_offset += number_of_strips*sizeof(unsigned short);
					}
					else
					{
						strip_byte_counts_start=ftell(output_file)+2*sizeof(unsigned short)+
						sizeof(unsigned long);
						write_tiff_field((unsigned short)279,(unsigned short)TIFF_SHORT_FIELD,
							(unsigned long)number_of_strips,(unsigned long)value_offset,
							output_file);
					}
					/* planar configuration */
					write_tiff_field((unsigned short)284,(unsigned short)TIFF_SHORT_FIELD,
						(unsigned long)1,planar_configuration_value,output_file);
					if (palette_image)
					{
						/* colour map */
						write_tiff_field((unsigned short)320,(unsigned short)TIFF_SHORT_FIELD,
							(unsigned long)(3*256),(unsigned long)value_offset,output_file);
						value_offset += 3*256*sizeof(unsigned short);
					}
					next_image_file_directory_offset=0;
					fwrite(&next_image_file_directory_offset,sizeof(unsigned long),1,
						output_file);
#if defined (DEBUG)
					/*???debug */
					printf("written image_function_definitions\n");
					printf("location %d %d\n",ftell(output_file),
						(int)image_file_directory_offset+sizeof(unsigned short)+
						sizeof(unsigned long)+image_file_directory_entry_count*12);
#endif /* defined (DEBUG) */
					/* write bits_per_sample */
					if (!palette_image)
					{
						/* write bits_per_sample */
						fwrite(bits_per_sample,sizeof(unsigned short),3,output_file);
					}
#if defined (DEBUG)
					/*???debug */
					printf("written bits_per_sample\n");
					printf("strip offsets start %d %d\n",ftell(output_file),
						(int)image_file_directory_offset+sizeof(unsigned short)+
						image_file_directory_entry_count*12+sizeof(unsigned long)+
						3*sizeof(unsigned short));
#endif /* defined (DEBUG) */
					if (1<number_of_strips)
					{
						/* write the strip offsets (currently 0 will need to be overwritten
							later) */
						fwrite(strip_offsets,sizeof(unsigned long),number_of_strips,
							output_file);
					}
					if (2<number_of_strips)
					{
						/* write the strip byte counts (currently 0 will need to be
							overwritten later) */
						fwrite(strip_byte_counts,sizeof(unsigned short),number_of_strips,
							output_file);
					}
					if (palette_image)
					{
						/* write the colour map */
						fwrite(colour_map,sizeof(unsigned short),3*256,output_file);
					}
#if defined (DEBUG)
					/*???debug */
					printf("written strip_offsets and strip_byte_counts\n");
#endif /* defined (DEBUG) */
					/* write the image to the file */
					row_number=number_of_rows;
					strip_offset=strip_offsets;
					strip_byte_count=strip_byte_counts;
					strip_offset[0]=value_offset;
#if defined (DEBUG)
					/*???debug */
					printf("strip_offset[0] %d %d\n",strip_offset[0],ftell(output_file));
#endif /* defined (DEBUG) */
					if (palette_image)
					{
						uncompressed_bytes=palette_image;
					}
					rows_per_strip_save=rows_per_strip;
					for (k=number_of_strips;k>0;k--)
					{
						if (palette_image)
						{
							if (row_number<rows_per_strip)
							{
								if (TIFF_PLANAR_PLANAR_CONFIGURATION==planar_configuration_value)
								{
									number_of_uncompressed_bytes=row_number*number_of_columns;
								}
								else
								{
									number_of_uncompressed_bytes=row_number*number_of_columns*
									samples_per_pixel;
								}
							}
							row_number -= rows_per_strip;
						}
						else
						{
							/* extract the strip */
							if (row_number<rows_per_strip)
							{
								rows_per_strip=row_number;
								if (TIFF_PLANAR_PLANAR_CONFIGURATION==planar_configuration_value)
								{
									number_of_uncompressed_bytes=rows_per_strip*number_of_columns;
								}
								else
								{
									number_of_uncompressed_bytes=rows_per_strip*number_of_columns*
									samples_per_pixel;
								}
							}
#if defined (DEBUG)
							/*???debug */
							printf(
								"k=%d,row_number=%d,number_of_strips=%d,rows_per_strip=%d,%d\n",k,
								row_number,number_of_strips,rows_per_strip,
								(2-(k-1)/(number_of_strips/samples_per_pixel)));
#endif /* defined (DEBUG) */
							uncompressed_byte=uncompressed_bytes;
							for (i=rows_per_strip;i>0;i--)
							{
								row_number--;
								if (TIFF_PLANAR_PLANAR_CONFIGURATION==planar_configuration_value)
								{
									pixel=((unsigned char *)image)+number_of_components*row_number*
								   (number_of_columns+row_padding)+
							      (2-(k-1)/(number_of_strips/samples_per_pixel));
									for (j=number_of_columns;j>0;j--)
									{
										*uncompressed_byte= *pixel;
										uncompressed_byte++;
										pixel += number_of_components;
									}
								}
								else
								{
									pixel=((unsigned char *)image)+number_of_components*row_number*
									(number_of_columns+row_padding);
									for (j=number_of_columns;j>0;j--)
									{
										switch (number_of_components)
										{
											case 1:
											{
												*uncompressed_byte= *pixel;
												uncompressed_byte++;
												*uncompressed_byte= *pixel;
												uncompressed_byte++;
												*uncompressed_byte= *pixel;
												uncompressed_byte++;
												pixel++;
											} break;
											case 2:
											{
												pixel++;
												*uncompressed_byte= *pixel;
												uncompressed_byte++;
												*uncompressed_byte= *pixel;
												uncompressed_byte++;
												*uncompressed_byte= *pixel;
												uncompressed_byte++;
												pixel++;
											} break;
											case 3:
											{
												*uncompressed_byte= *pixel;
												uncompressed_byte++;
												pixel++;
												*uncompressed_byte= *pixel;
												uncompressed_byte++;
												pixel++;
												*uncompressed_byte= *pixel;
												uncompressed_byte++;
												pixel++;
											} break;
											case 4:
											{
												*uncompressed_byte= *pixel;
												uncompressed_byte++;
												pixel++;
												*uncompressed_byte= *pixel;
												uncompressed_byte++;
												pixel++;
												*uncompressed_byte= *pixel;
												uncompressed_byte++;
												pixel++;
												pixel++;
											} break;
										}
									}
								}
							}
#if defined (DEBUG)
							/*???debug */
							printf("extracted strip %d\n",number_of_strips-k+1);
#endif /* defined (DEBUG) */
						}
						switch (compression)
						{
							case TIFF_NO_COMPRESSION:
							{
								fwrite(uncompressed_bytes,sizeof(unsigned char),
									number_of_uncompressed_bytes,output_file);
								*strip_byte_count=number_of_uncompressed_bytes;
							} break;
							case TIFF_LZW_COMPRESSION:
							{
								LZW_compress_to_file(output_file,uncompressed_bytes,
									number_of_uncompressed_bytes,strip_byte_count);
							} break;
							case TIFF_PACK_BITS_COMPRESSION:
							{
								pack_bits_compress_to_file(output_file,uncompressed_bytes,
									number_of_uncompressed_bytes,number_of_columns,
									strip_byte_count);
							} break;
						}
#if defined (DEBUG)
						/*???debug */
						printf("strip_offset[%d] %d %d\n",number_of_strips-k+1,
							strip_offset[0]+strip_byte_count[0],ftell(output_file));
#endif /* defined (DEBUG) */
						if (k>1)
						{
							strip_offset[1]=strip_offset[0]+strip_byte_count[0];
							/* value offsets must be word aligned */
							if (0!=strip_offset[1]%2)
							{
								strip_offset[1]++;
								byte=(unsigned char)1;
								fwrite(&byte,sizeof(unsigned char),1,output_file);
							}
						}
						strip_offset++;
						strip_byte_count++;
						if (palette_image)
						{
							uncompressed_bytes += rows_per_strip*number_of_columns;
						}
						else
						{
							if ((TIFF_PLANAR_PLANAR_CONFIGURATION==planar_configuration_value)&&
								(row_number<=0))
							{
								rows_per_strip=rows_per_strip_save;
								row_number=number_of_rows;
								number_of_uncompressed_bytes=rows_per_strip*number_of_columns;
							}
						}
					}
#if defined (DEBUG)
					/*???debug */
					printf("written strips\n");
					for (k=0;k<number_of_strips;k++)
					{
						printf("%d %d %d\n",k,strip_offsets[k],strip_byte_counts[k]);
					}
#endif /* defined (DEBUG) */
					/* write the strip offsets */
					fseek(output_file,(long int)strip_offsets_start,SEEK_SET);
#if defined (DEBUG)
					printf("strip offsets start %d %d\n",ftell(output_file),
						strip_offsets_start);
#endif /* defined (DEBUG) */
					fwrite(strip_offsets,sizeof(unsigned long),number_of_strips,
						output_file);
#if defined (DEBUG)
					printf("strip byte counts start %d %d\n",ftell(output_file),
						strip_byte_counts_start);
#endif /* defined (DEBUG) */
					/* write the strip byte counts */
					fseek(output_file,(long int)strip_byte_counts_start,SEEK_SET);
#if defined (DEBUG)
					printf("strip byte counts start %d %d\n",ftell(output_file),
						strip_byte_counts_start);
#endif /* defined (DEBUG) */
					fwrite(strip_byte_counts,sizeof(unsigned short),number_of_strips,
						output_file);
					(void)fclose(output_file);
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"write_tiff_image_file.  Could not open file");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"write_tiff_image_file.  Could not allocate memory for strip offsets/byte counts");
				return_code=0;
			}
			DEALLOCATE(strip_byte_counts);
			DEALLOCATE(strip_offsets);
			if (palette_image)
			{
				DEALLOCATE(palette_image);
			}
			else
			{
				DEALLOCATE(uncompressed_bytes);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_tiff_image_file.  Only implemented for one byte per component");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_tiff_image_file.  Missing argument(s)");
		return_code=0;
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave write_tiff_image_file\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* write_tiff_image_file */

#if defined (OLD_CODE)
int write_tiff_image_file(char *file_name, int number_of_components,
	int number_of_columns, int number_of_rows, long unsigned *image)
/*******************************************************************************
LAST MODIFIED : 4 March 2002

DESCRIPTION :
Writes an image in TIFF file format.
==============================================================================*/
{
	FILE *output_file;
	int i,j,return_code;
	struct Image_function_definitions
	{
		struct Tiff_image_function_definition_entry image_width;
		struct Tiff_image_function_definition_entry image_length;
		struct Tiff_image_function_definition_entry bits_per_sample;
		struct Tiff_image_function_definition_entry photometric_int;
		struct Tiff_image_function_definition_entry strip_offsets;
		struct Tiff_image_function_definition_entry samples_per_pixel;
		struct Tiff_image_function_definition_entry rows_per_strip;
		struct Tiff_image_function_definition_entry strip_byte_counts;
		unsigned long next_image_function_definition_offset;
	} image_function_definitions;
	unsigned char *pixel;
	unsigned long image_function_definition_offset;
	unsigned short byte_order,image_function_definition_entry_count,version;

	ENTER(write_tiff_image_file);
	return_code=0;
	if (file_name && (0 < number_of_components) && (0 < number_of_columns) &&
		(0 < number_of_rows) && image)
	{
		/* open the output file */
		if (output_file=fopen(file_name,"wb"))
		{
			/* write the tiff file header */
#if defined (BYTE_ORDER) && (1234==BYTE_ORDER)
			byte_order=TIFF_LO_HI;
#else /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */
			byte_order=TIFF_HI_LO;
#endif /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */
			image_function_definition_offset=sizeof(unsigned short);
			fwrite(&byte_order,sizeof(unsigned short),1,output_file);
			version=42;
			image_function_definition_offset += sizeof(unsigned short);
			fwrite(&version,sizeof(unsigned short),1,output_file);
			image_function_definition_offset += sizeof(unsigned long);
			fwrite(&image_function_definition_offset,sizeof(unsigned long),1,
				output_file);
			image_function_definition_entry_count=8;
			image_function_definition_offset += sizeof(unsigned short);
			fwrite(&image_function_definition_entry_count,sizeof(unsigned short),1,
				output_file);
			/* write the image header */
			/* fill image_function_definition entries and write to file */
			image_function_definitions.image_width.tag=256;
			image_function_definitions.image_width.type=TIFF_SHORT_FIELD;
			image_function_definitions.image_width.length=1;
#if defined (BYTE_ORDER) && (1234==BYTE_ORDER)
			image_function_definitions.image_width.value_offset=number_of_columns;
#else /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */
			image_function_definitions.image_width.value_offset=number_of_columns<<16;
#endif /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */
			image_function_definitions.image_length.tag=257;
			image_function_definitions.image_length.type=TIFF_SHORT_FIELD;
			image_function_definitions.image_length.length=1;
#if defined (BYTE_ORDER) && (1234==BYTE_ORDER)
			image_function_definitions.image_length.value_offset=number_of_rows;
#else /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */
			image_function_definitions.image_length.value_offset=number_of_rows<<16;
#endif /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */
			image_function_definitions.bits_per_sample.tag=258;
			image_function_definitions.bits_per_sample.type=TIFF_SHORT_FIELD;
			image_function_definitions.bits_per_sample.length=1;
#if defined (BYTE_ORDER) && (1234==BYTE_ORDER)
			image_function_definitions.bits_per_sample.value_offset=8;
#else /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */
			image_function_definitions.bits_per_sample.value_offset=8<<16;
#endif /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */
			image_function_definitions.photometric_int.tag=262;
			image_function_definitions.photometric_int.type=TIFF_SHORT_FIELD;
			image_function_definitions.photometric_int.length=1;
#if defined (BYTE_ORDER) && (1234==BYTE_ORDER)
			image_function_definitions.photometric_int.value_offset=TIFF_RGB;
#else /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */
			image_function_definitions.photometric_int.value_offset=TIFF_RGB<<16;
#endif /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */
			image_function_definitions.strip_offsets.tag=273;
			image_function_definitions.strip_offsets.type=TIFF_LONG_FIELD;
			image_function_definitions.strip_offsets.length=1;
			image_function_definitions.strip_offsets.value_offset=
				(int)image_function_definition_offset+
				sizeof(struct Image_function_definitions);
			image_function_definitions.samples_per_pixel.tag=277;
			image_function_definitions.samples_per_pixel.type=TIFF_SHORT_FIELD;
			image_function_definitions.samples_per_pixel.length=1;
#if defined (BYTE_ORDER) && (1234==BYTE_ORDER)
			image_function_definitions.samples_per_pixel.value_offset=3;
#else /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */
			image_function_definitions.samples_per_pixel.value_offset=3<<16;
#endif /* defined (BYTE_ORDER) && (1234==BYTE_ORDER) */
			image_function_definitions.rows_per_strip.tag=278;
			image_function_definitions.rows_per_strip.type=TIFF_LONG_FIELD;
			image_function_definitions.rows_per_strip.length=1;
			image_function_definitions.rows_per_strip.value_offset=number_of_rows;
			image_function_definitions.strip_byte_counts.tag=279;
			image_function_definitions.strip_byte_counts.type=TIFF_LONG_FIELD;
			image_function_definitions.strip_byte_counts.length=1;
			image_function_definitions.strip_byte_counts.value_offset=
				3*number_of_rows*number_of_columns;
			image_function_definitions.next_image_function_definition_offset=0;
			/* write IFD blocks */
			fwrite(&image_function_definitions,
				sizeof(struct Image_function_definitions),1,output_file);
			/* write the image to the file */
			for (i=number_of_rows-1;i>=0;i--)
			{
				pixel=(unsigned char *)(image+(i*number_of_columns));
				for (j=number_of_columns;j>0;j--)
				{
					fwrite(pixel,sizeof(char),1,output_file);
					fwrite(pixel+1,sizeof(char),1,output_file);
					fwrite(pixel+2,sizeof(char),1,output_file);
					pixel += sizeof(long unsigned);
				}
			}
			(void)fclose(output_file);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_tiff_image_file.  Could not open file");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_tiff_image_file.  Missing argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_tiff_image_file */
#endif /* defined (OLD_CODE) */

int read_raw_image_file(char *file_name,
	enum Raw_image_storage raw_image_storage,
	int *width_address, int *height_address, int *number_of_components_address,
	int *number_of_bytes_per_component_address, unsigned char **image_address)
/*******************************************************************************
LAST MODIFIED : 14 February 2002

DESCRIPTION :
Reads an image from a RAW file, returning it as an RGB image. RAW images have
no header, and hence the <raw_image_storage> and width at <*width_address> must
be specified in order to read it. If a positive height is specified, an
image of that height will be returned, otherwise the height will be calculated
from the file size. Note indexed RAW files are not supported.
==============================================================================*/
{
	unsigned char *byte_row, *image, *image_ptr;
	FILE *image_file;
	int aligned_row_size, bytes_read, comp, file_size, height, i, return_code,
		row, row_size, width;
	struct stat buf;

	ENTER(read_raw_image_file);
	if (file_name && width_address && height_address &&
		number_of_components_address && number_of_bytes_per_component_address &&
		image_address)
	{
		if (0 < (width = *width_address))
		{
			return_code = 1;
			if (image_file = fopen(file_name,"rb"))
			{
				/* use stat to get size of file to determine format from */
				if ((0 == stat(file_name, &buf)) &&
					(0 < (file_size = (int)(buf.st_size))))
				{
					if (0 >= (height = *height_address))
					{
						/* set height to include all data, padding rest of last line */
						height = (file_size + width*3 - 1) / (width*3);
					}
					/* rows are 4-byte aligned */
					row_size = width*3;
					aligned_row_size = (int)(((width*3 + 3)/4)*4);
					if (ALLOCATE(image, unsigned char, aligned_row_size*height))
					{
						switch (raw_image_storage)
						{
							case RAW_INTERLEAVED_RGB:
							{
								/* values are in RGB triples */
								/* convert image from top-to-bottom to bottom-to-top for
									 OpenGL image/texture storage */
								for (row=1;(row <= height)&&return_code;row++)
								{
									image_ptr = image + (height-row)*aligned_row_size;
									if (aligned_row_size != (bytes_read=fread(image_ptr,
										/*value_size*/1,row_size,image_file)))
									{
										/* fill rest of line with zeros */
										memset(image_ptr+bytes_read,0,aligned_row_size-bytes_read);
									}
								}
							} break;
							case RAW_PLANAR_RGB:
							{
								/* red image then green image then blue image */
								if (ALLOCATE(byte_row,unsigned char,width))
								{
									for (comp=0;(comp<3)&&return_code;comp++)
									{
										/* convert image from top-to-bottom to bottom-to-top for
											 OpenGL image/texture storage */
										for (row=1;(row <= height)&&return_code;row++)
										{
											image_ptr = image + (height-row)*aligned_row_size + comp;
											if (width != (bytes_read=fread(byte_row,
												/*value_size*/1,width,image_file)))
											{
												/* fill rest of line with zeros */
												memset(byte_row+bytes_read,0,width-bytes_read);
											}
											for (i=0;i<width;i++)
											{
												*image_ptr = byte_row[i];
												image_ptr += 3;
											}
											if ((0==comp)&&(row_size < aligned_row_size))
											{
												/* fill rest of line with zeros */
												memset(image_ptr,0,aligned_row_size-row_size);
											}
										}
									}
									DEALLOCATE(byte_row);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"read_raw_image_file.  Not enough memory");
									return_code=0;
								}
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"read_raw_image_file.  Unknown raw image storage");
								return_code=0;
							}
						}
						if (return_code)
						{
							*number_of_components_address = 3;
							*number_of_bytes_per_component_address = 1;
							*height_address = height;
							*width_address = width;
							*image_address = image;
						}
						else
						{
							DEALLOCATE(image);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_raw_image_file.  Not enough memory");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"read_raw_image_file.  Could not get size of file '%s'",file_name);
					return_code=0;
				}
				fclose(image_file);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_raw_image_file.  Could not open image file '%s'",file_name);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_raw_image_file.  No width specified for file '%s'",file_name);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"read_raw_image_file.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* read_raw_image_file */

int read_rgb_image_file(char *file_name,
	int *width_address, int *height_address, int *number_of_components_address,
	int *number_of_bytes_per_component_address, unsigned char **image_address)
/*******************************************************************************
LAST MODIFIED : 14 February 2002

DESCRIPTION :
Reads an image from a SGI rgb file.
number_of_components=1, I
number_of_components=2, IA
number_of_components=3, RGB
number_of_components=4, RGBA
???DB.  Need to find out more about images.  See 4Dgifts/iristools.
==============================================================================*/
{
	FILE *image_file;
	int i, j, k, least_to_most, max_row_size, number_of_bytes,
		number_of_bytes_per_component, number_of_rows,
		return_code,run_length;
	long row_size, row_start;
	unsigned char *image,*image_ptr,pixel,*row,*row_ptr;
	unsigned char *row_size_char,*row_sizes_char,*row_start_char,*row_starts_char;
	unsigned short dimension,height,image_file_type,magic_number,
		number_of_components,pixel2,width;

	ENTER(read_rgb_image_file);
	least_to_most = 0;
	if (file_name && width_address && height_address &&
		number_of_components_address && number_of_bytes_per_component_address &&
		image_address)
	{
		if (image_file=fopen(file_name,"rb"))
		{
			if ((1==read_and_byte_swap((unsigned char *)&magic_number,2,1,
				least_to_most,image_file))&&
				(1==read_and_byte_swap((unsigned char *)&image_file_type,2,1,
				least_to_most,image_file))&&
				(1==read_and_byte_swap((unsigned char *)&dimension,2,1,least_to_most,
				image_file))&&
				(1==read_and_byte_swap((unsigned char *)&width,2,1,least_to_most,
				image_file))&&
				(1==read_and_byte_swap((unsigned char *)&height,2,1,least_to_most,
				image_file))&&
				(1==read_and_byte_swap((unsigned char *)&number_of_components,2,1,
				least_to_most,image_file)))
			{
#if defined (DEBUG)
				/*???debug */
				printf("magic_number %x\n",magic_number);
				printf("image_file_type %x\n",image_file_type);
				printf("dimension %d\n",dimension);
				printf("width %d\n",width);
				printf("height %d\n",height);
				printf("number_of_components %d\n",number_of_components);
#endif /* defined (DEBUG) */
				if ((0<width)&&(0<height)&&(1<=number_of_components)&&
					(number_of_components<=4))
				{
					number_of_rows=height*number_of_components;
					return_code=1;
					image=(unsigned char *)NULL;
					number_of_bytes_per_component = (image_file_type & 0x000000ff);
					number_of_bytes = number_of_components*number_of_bytes_per_component;
					if (0x00000100==(image_file_type&0x0000ff00))
					{
						/* run length encoded */
						row_starts_char=(unsigned char *)NULL;
						row_sizes_char=(unsigned char *)NULL;
						/* SAB All this stuff needs to avoid the use of platform
							dependencies, like sizeof (long) etc. so I have used
							unsigned char throughout */
						if (ALLOCATE(row_starts_char,unsigned char,4*number_of_rows)&&
							ALLOCATE(row_sizes_char,unsigned char,4*number_of_rows)&&
							ALLOCATE(image,unsigned char,
							((width*height*number_of_bytes+3)/4)*4))
						{
							/* No longer doing a byte_swap as the use of << to add the
								bytes of the chars together will sort that out */
							if ((0==fseek(image_file,512,SEEK_SET))&&
							    (number_of_rows==(int)fread(row_starts_char,4,
								 number_of_rows,image_file))&&
								 (number_of_rows==(int)fread(row_sizes_char,4,
							  	 number_of_rows,image_file)))
							{
								/* find the maximum row size */
								row_size_char=row_sizes_char;
								row_size = ((long)row_size_char[3]) + 
									(((long)row_size_char[2]) << 8) +
									(((long)row_size_char[1]) << 16) +
									(((long)row_size_char[0]) << 24);
								max_row_size = row_size;
								for (i=number_of_rows-1;i>0;i--)
								{
									row_size_char += 4;
									row_size = ((long)row_size_char[3]) + 
										(((long)row_size_char[2]) << 8) +
										(((long)row_size_char[1]) << 16) +
										(((long)row_size_char[0]) << 24);
									if (row_size>max_row_size)
									{
										max_row_size = row_size;
									}
								}
								max_row_size *= number_of_bytes_per_component;
								if (ALLOCATE(row,unsigned char,max_row_size))
								{
									row_start_char=row_starts_char;
									row_size_char=row_sizes_char;
									image_ptr=image;
									i=0;
									while (return_code&&(i<number_of_components))
									{
										image_ptr = image + i*number_of_bytes_per_component;
										j=height;
										while (return_code&&(j>0))
										{
											row_start = ((long)row_start_char[3]) + 
												(((long)row_start_char[2]) << 8) +
												(((long)row_start_char[1]) << 16) +
												(((long)row_start_char[0]) << 24);
											row_size = ((long)row_size_char[3]) + 
												(((long)row_size_char[2]) << 8) +
												(((long)row_size_char[1]) << 16) +
												(((long)row_size_char[0]) << 24);
											switch(number_of_bytes_per_component)
											{
												case 1:
												{
													if ((0==fseek(image_file,row_start,SEEK_SET))&&
														(row_size==(int)fread(row,1,row_size,image_file)))
													{
														row_ptr=row;
														pixel= *row_ptr;
														run_length=(int)(pixel&0x0000007F);
														while (run_length)
														{
															row_ptr++;
															if (pixel&0x00000080)
															{
																while (run_length>0)
																{
																	*image_ptr= *row_ptr;
																	image_ptr += number_of_components;
																	row_ptr++;
																	run_length--;
																}
															}
															else
															{
																pixel= *row_ptr;
																row_ptr++;
																while (run_length>0)
																{
																	*image_ptr=pixel;
																	image_ptr += number_of_components;
																	run_length--;
																}
															}
															pixel= *row_ptr;
															run_length=(int)(pixel&0x0000007F);
														}
														row_start_char+=4;
														row_size_char+=4;
														j--;
													}
													else
													{
														display_message(ERROR_MESSAGE,
															"read_rgb_image_file.  Error reading row component");
														return_code=0;
													}
												} break;
												case 2:
												{
													if ((0==fseek(image_file,row_start,SEEK_SET))&&
														(row_size==(int)fread(row,1,row_size,image_file)))
													{
														row_ptr=row;
														pixel2 = (((unsigned short)*row_ptr) << 8) +
															((unsigned short)*(row_ptr + 1));
														run_length=(int)(pixel2&0x0000007F);
														while (run_length)
														{
															row_ptr+=2;
															if (pixel2&0x00000080)
															{
																while (run_length>0)
																{
																	*image_ptr= *row_ptr;
																	*(image_ptr + 1)= *(row_ptr + 1);
																	image_ptr += number_of_components * 2;
																	row_ptr+=2;
																	run_length--;
																}
															}
															else
															{
																pixel2= (((unsigned short)*row_ptr) << 8) +
																	((unsigned short)*(row_ptr + 1));
																row_ptr+=2;
																while (run_length>0)
																{
																	*image_ptr=(pixel2 & 0xff00)>>8;
																	*(image_ptr + 1)= pixel2 & 0x00ff;
																	image_ptr += number_of_components * 2;
																	run_length--;
																}
															}
															pixel2= (((unsigned short)*row_ptr) << 8) +
																((unsigned short)*(row_ptr + 1));
															run_length=(int)(pixel2&0x0000007F);
														}
														row_start_char+=4;
														row_size_char+=4;
														j--;
													}
													else
													{
														display_message(ERROR_MESSAGE,
															"read_rgb_image_file.  Error reading row component");
														return_code=0;
													}
												} break;
												default:
												{
													display_message(ERROR_MESSAGE,
														"read_rgb_image_file.  Unsupported bytes per component");
													return_code=0;
												} break;
											}
													
										}
										i++;
									}
									DEALLOCATE(row);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"read_rgb_image_file.  Could not allocate row array");
									return_code=0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"read_rgb_image_file.  Error reading row start/size");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_rgb_image_file.  Could not allocate image arrays");
							return_code=0;
						}
						DEALLOCATE(row_starts_char);
						DEALLOCATE(row_sizes_char);
					}
					else
					{
						/* verbatim */
						if (ALLOCATE(row,unsigned char,
							width*number_of_bytes_per_component)&&
							ALLOCATE(image,unsigned char,width*height*number_of_bytes))
						{
							if (0==fseek(image_file,512,SEEK_SET))
							{
								i=0;
								while (return_code&&(i<number_of_components))
								{
									image_ptr=image+i;
									j=height;
									while (return_code&&(j>0))
									{
										if (width==read_and_byte_swap(row,
											number_of_bytes_per_component,width,least_to_most,
											image_file))
										{
											row_ptr=row;
											switch(number_of_bytes_per_component)
											{
												case 1:
												{
													for (k=width;k>0;k--)
													{
														*image_ptr= *row_ptr;
														row_ptr++;
														image_ptr += number_of_components;
													}
												} break;
												case 2:
												{
													for (k=width;k>0;k--)
													{
														*image_ptr= *row_ptr;
														*(image_ptr + 1)= *(row_ptr + 1);
														row_ptr+=2;
														image_ptr += number_of_components * 2;
													}
												} break;
												default:
												{
													display_message(ERROR_MESSAGE,
														"read_rgb_image_file.  Unsupported bytes per component");
													return_code=0;
												} break;
											}
											j--;
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"read_rgb_image_file.  Error reading row component");
											return_code=0;
										}
									}
									i++;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"read_rgb_image_file.  Error positioning file");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_rgb_image_file.  Could not allocate image arrays");
							return_code=0;
						}
						DEALLOCATE(row);
					}
					if (return_code)
					{
						*width_address = (int)width;
						*height_address = (int)height;
						*number_of_components_address = (int)number_of_components;
						*number_of_bytes_per_component_address =
							number_of_bytes_per_component;
						*image_address = image;
					}
					else
					{
						DEALLOCATE(image);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"read_rgb_image_file.  Invalid image size");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_rgb_image_file.  Error reading file information");
				return_code=0;
			}
			fclose(image_file);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_rgb_image_file.  Could not open image file");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"read_rgb_image_file.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* read_rgb_image_file */

int read_tiff_image_file(char *file_name,
	int *width_address, int *height_address, int *number_of_components_address,
	int *number_of_bytes_per_component_address, unsigned char **image_address)
/*******************************************************************************
LAST MODIFIED : 14 February 2002

DESCRIPTION :
Reads an image from a TIFF file.
number_of_components=1, I
number_of_components=2, IA
number_of_components=3, RGB
number_of_components=4, RGBA
???DB.  Need to find out more about images.  See 4Dgifts/iristools.

A BRIEF EXPLANATION OF STRUCTURE OF A TIFF FILE
===============================================
(DETAILS AVAILABLE FROM TEXT FILE - DAVID BULLIVANT HAS A COPY)

In TIFF, individual fields are identified with a unique tag.	This allows
particular fields to be present or absent from the file as required by the
application.

A TIFF file begins with an 8-byte "image file header" that points to one
or more image file directories.	The image file directories contain information
about the images, as well as pointers to the actual image data.

STRUCTURES IN DETAIL
====================

"Image File Header" - contains the following information:

BYTES 0 - 1:	The first word of the file specifies the byte order used within
the file.	Legal values are:
	"II" - byte order from least to most significant
	"MM" - byte order from most to least significant
In both formats, character strings are stored into sequential byte locations.

BYTES 2 - 3:	The second word of the file is the TIFF version number.	This
number, 42, will never change and if it does, it means that TIFF has changed
in some way so radical that a TIFF reader should give up immediately.

BYTES 4 - 7:	This long word contains the offset (in bytes) of the first
Image File Directory.	The directory may be at any location in the file after
the header but must begin on a word boundary (it may follow the image data
it describes!).

(The term "byte offset" is always used to refer to a location with respect to
the beginning of the file.	The first byte of the file has a offset of 0.)

													----------------

"Image File Directory" (IFD) - consists of a 2-byte count of the number of
entries (ie. the number of fields) followed by a sequence of 12-byte field
entries, followed by a 4-byte offset of the next IFD (or 0 if none).

Each 12-byte IFD entry has the following format:
BYTES 0 - 1:	contain the Tag for the field
BYTES 2 - 3:	contain the Field Type
BYTES 4 - 7:	contain the Length of the Field
BYTES 8 - 11:	contain the Value Offset, the file offset (in bytes) of the
Value for the field.	The Value is expected to begin on a word boundary.

In order to save time and space, the Value Offset is interpreted to contain
the Value instead of pointing to the Value if the Value fits into 4 bytes.
If the value is less than 4 bytes, it is left-justified within the 4-byte
Value Offset, ie. stored in the lower-numbered bytes.	Whether the Value
fits within 4 bytes is determined by looking at the Type and Length of the
field.

The Length is specified in terms of the data type, not the total number of
bytes.	A single 16-bit word (SHORT) has a Length of 3, not 2, for example.
The data types and their lengths are described below:
1 = BYTE
2 = ASCII			8-bit bytes that store ASCII codes; the last byte must be null
3 = SHORT			16-bit (2-byte) unsigned integer
4 = LONG			32-bit (4-byte) unsigned integer
5 = RATIONAL	Two LONG''s - the first represents the numerator of a fraction,
the second the denominator.
==============================================================================*/
{
	FILE *tiff_file;
	float x_resolution, y_resolution;
	int i,least_to_most,return_code;
  long image_length,image_width,*strip_offset,*strip_offsets;
	unsigned char bit,byte,byte_array[4],colour_0,colour_1,*image,*image_ptr,
		*new_strip,*strip,*temp_unsigned_char_ptr, *field_values;
	unsigned long byte_count,column_number,field_count,ifd_offset,
      number_of_fields,number_of_strips,rows_per_strip,
		*strip_byte_count,*strip_byte_counts;
	unsigned short bits_per_sample,compression,field_tag,field_type,file_type,
		photometric_interpretation,planar_configuration,resolution_unit,
		samples_per_pixel;

	ENTER(read_tiff_image_file);
#if defined (DEBUG)
	/*???debug */
	printf("enter read_tiff_image_file\n");
#endif /* defined (DEBUG) */
	return_code=0;
	if (file_name && width_address && height_address &&
		number_of_components_address && number_of_bytes_per_component_address &&
		image_address)
	{
		*number_of_bytes_per_component_address = 1;
		if (tiff_file=fopen(file_name,"rb"))
		{
			return_code=1;
#if defined (DEBUG)
			/*???debug */
			printf("DETAILS OF DATA STORAGE FOR TIFF FILE %s\n",file_name);
#endif /* defined (DEBUG) */
			/* read tiff file header */
			/* find out byte_order of file (most/least to least/most significant) */
			fseek(tiff_file,0,0);
			if (2==fread(byte_array,sizeof(char),2,tiff_file))
			{
				if (('I'==byte_array[0])&&('I'==byte_array[1]))
				{
					least_to_most=1;
				}
				else
				{
					if (('M'==byte_array[0])&&('M'==byte_array[1]))
					{
						least_to_most=0;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_tiff_image_file.  Invalid byte order");
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_tiff_image_file.  Error reading byte order");
				return_code=0;
			}
			if (return_code)
			{
#if defined (DEBUG)
				/*???debug */
				printf("A) Byte Order = %d\n", least_to_most);
				printf("     where 0 = most to least significant\n");
				printf("           1 = least to most significant\n\n");
#endif /* defined (DEBUG) */
				/* check file number */
				if (1==read_and_byte_swap(byte_array,2,1,least_to_most,
					tiff_file))
				{
					file_type = UNSIGNED_SHORT_INT_FROM_2_BYTES(byte_array);
#if defined (DEBUG)
					/*???debug */
					printf("B) TIFF Version number = %d\n",file_type);
					printf("     where 42 is the only valid version.\n\n");
#endif /* defined (DEBUG) */
					if (42!=file_type)
					{
						display_message(ERROR_MESSAGE,
							"read_tiff_image_file.  Invalid TIFF file type %d", file_type);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"read_tiff_image_file.  Error reading file type");
					return_code=0;
				}
				if (return_code)
				{
					/* find offset of first image file directory */
						/*???DB.  Only reading first */
					if (1==read_and_byte_swap(byte_array,4,1,least_to_most,
						tiff_file))
					{
						ifd_offset = UNSIGNED_LONG_INT_FROM_4_BYTES(byte_array);
#if defined (DEBUG)
						/*???debug */
						printf("C) Address of Image File Directory = %ld\n\n",ifd_offset);
#endif /* defined (DEBUG) */
						if (0>=ifd_offset)
						{
							display_message(ERROR_MESSAGE,
								"read_tiff_image_file.  Invalid image file directory offset");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
						"read_tiff_image_file.  Error reading image file directory offset");
						return_code=0;
					}
					if (return_code)
					{
						/* go to image file directory and find out the number of fields */
						if (0==fseek(tiff_file, (signed long int)ifd_offset, 0))
						{
							if (1==read_and_byte_swap(byte_array,2,1,
								least_to_most,tiff_file))
							{
								number_of_fields = UNSIGNED_SHORT_INT_FROM_2_BYTES(byte_array);
#if defined (DEBUG)
								/*???debug */
								printf("D) Image file directory\n");
								printf("     Number of Fields = %d\n",number_of_fields);
#endif /* defined (DEBUG) */
								if (0>=number_of_fields)
								{
									display_message(ERROR_MESSAGE,
										"read_tiff_image_file.  Invalid number of fields");
									return_code=0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"read_tiff_image_file.  Error reading number of fields");
								return_code=0;
							}
							if (return_code)
							{
								/* set field defaults */
								photometric_interpretation=0;
								compression=TIFF_NO_COMPRESSION_VALUE;
								image_length=0;
								image_width=0;
								resolution_unit=TIFF_INCH;
								x_resolution= -1;
								y_resolution= -1;
								rows_per_strip=0;
								strip_offsets=(long int *)NULL;
								strip_byte_counts=(unsigned long int *)NULL;
								bits_per_sample=0;
								samples_per_pixel=0;
								number_of_strips=0;
								planar_configuration=TIFF_CHUNKY_PLANAR_CONFIGURATION;
								/* search through tags to find relevant fields */
								while (return_code&&(number_of_fields>0))
								{
									/* read field information */
									if (return_code = read_tiff_field(&field_tag, &field_type,
										&field_count, &field_values, tiff_file, least_to_most))
									{
										/* allocate values specific to tag numbers */
										switch (field_tag)
										{
											case 256: /* image width */
											{
#if defined (DEBUG)
												/*???debug */
												printf("image width");
#endif /* defined (DEBUG) */
												if (1==field_count)
												{
													switch (field_type)
													{
														case TIFF_SHORT_FIELD:
														{
															image_width = (signed long)
																UNSIGNED_SHORT_INT_FROM_2_BYTES(field_values);
														} break;
														case TIFF_LONG_FIELD:
														{
															image_width = (signed long)
																UNSIGNED_LONG_INT_FROM_4_BYTES(field_values);
														} break;
														default:
														{
															return_code=0;
														}
													}
												}
												else
												{
													return_code=0;
												}
												DEALLOCATE(field_values);
#if defined (DEBUG)
												/*???debug */
												if (return_code)
												{
													printf("=%ld\n",image_width);
												}
												else
												{
													printf(" invalid\n");
												}
#endif /* defined (DEBUG) */
											} break;
											case 257: /* image length */
											{
#if defined (DEBUG)
												/*???debug */
												printf("image length");
#endif /* defined (DEBUG) */
												if (1==field_count)
												{
													switch (field_type)
													{
														case TIFF_SHORT_FIELD:
														{
															image_length = (signed long)
																UNSIGNED_SHORT_INT_FROM_2_BYTES(field_values);
														} break;
														case TIFF_LONG_FIELD:
														{
															image_length = (signed long)
																UNSIGNED_LONG_INT_FROM_4_BYTES(field_values);
														} break;
														default:
														{
															return_code=0;
														}
													}
												}
												else
												{
													return_code=0;
												}
												DEALLOCATE(field_values);
#if defined (DEBUG)
												/*???debug */
												if (return_code)
												{
													printf("=%ld\n",image_length);
												}
												else
												{
													printf(" invalid\n");
												}
#endif /* defined (DEBUG) */
											} break;
											case 258: /* bits per sample */
											{
#if defined (DEBUG)
												/*???debug */
												printf("bits per sample");
#endif /* defined (DEBUG) */
												if ((1 == field_count) &&
													(TIFF_SHORT_FIELD == field_type))
												{
													bits_per_sample =
														UNSIGNED_SHORT_INT_FROM_2_BYTES(field_values);
#if defined (DEBUG)
													/*???debug */
													printf("=%d\n",bits_per_sample);
#endif /* defined (DEBUG) */
												}
												else
												{
#if defined (DEBUG)
													/*???debug */
													printf("=%d %d %d",
														UNSIGNED_SHORT_INT_FROM_2_BYTES(field_values),
														UNSIGNED_SHORT_INT_FROM_2_BYTES(field_values + 2),
														UNSIGNED_SHORT_INT_FROM_2_BYTES(field_values + 4));
#endif /* defined (DEBUG) */
													if ((3==field_count)&&(TIFF_SHORT_FIELD==field_type))
													{
														if ((8 == UNSIGNED_SHORT_INT_FROM_2_BYTES(field_values)) &&
															(8 == UNSIGNED_SHORT_INT_FROM_2_BYTES(field_values + 2)) &&
															(8 == UNSIGNED_SHORT_INT_FROM_2_BYTES(field_values + 4)))
														{
															bits_per_sample=8;
#if defined (DEBUG)
															/*???debug */
															printf("\n");
#endif /* defined (DEBUG) */
														}
														else
														{
#if defined (DEBUG)
															/*???debug */
															printf(" not supported\n");
#endif /* defined (DEBUG) */
															return_code=0;
														}
													}
													else
													{
#if defined (DEBUG)
														/*???debug */
														printf(" invalid\n");
#endif /* defined (DEBUG) */
														return_code=0;
													}
												}
												DEALLOCATE(field_values);
											} break;
											case 259: /* compression */
											{
#if defined (DEBUG)
												/*???debug */
												printf("compression");
#endif /* defined (DEBUG) */
												if ((1==field_count)&&(TIFF_SHORT_FIELD==field_type))
												{
													compression =
														UNSIGNED_SHORT_INT_FROM_2_BYTES(field_values);
#if defined (DEBUG)
													/*???debug */
													printf("=%d\n",compression);
#endif /* defined (DEBUG) */
												}
												else
												{
#if defined (DEBUG)
													/*???debug */
													printf(" invalid\n");
#endif /* defined (DEBUG) */
													return_code=0;
												}
												DEALLOCATE(field_values);
											} break;
											case 262: /* photometric interpretation */
											{
#if defined (DEBUG)
												/*???debug */
												printf("photometric interpretation");
#endif /* defined (DEBUG) */
												if ((1==field_count)&&(TIFF_SHORT_FIELD==field_type))
												{
													photometric_interpretation =
														UNSIGNED_SHORT_INT_FROM_2_BYTES(field_values);
#if defined (DEBUG)
													/*???debug */
													printf("=%d\n",photometric_interpretation);
#endif /* defined (DEBUG) */
												}
												else
												{
#if defined (DEBUG)
													/*???debug */
													printf(" invalid\n");
#endif /* defined (DEBUG) */
													return_code=0;
												}
												DEALLOCATE(field_values);
											} break;
											case 273: /* strip offsets */
											{
#if defined (DEBUG)
												/*???debug */
												printf("strip offsets");
#endif /* defined (DEBUG) */
												if (0 < field_count)
												{
													number_of_strips = field_count;
													if (ALLOCATE(strip_offsets, signed long, field_count))
													{
														strip_offset = strip_offsets;
														temp_unsigned_char_ptr = field_values;
														while ((0 < field_count) && strip_offsets)
														{
															switch (field_type)
															{
																case TIFF_SHORT_FIELD:
																{
																	*strip_offset = (signed long)
																		UNSIGNED_SHORT_INT_FROM_2_BYTES(temp_unsigned_char_ptr);
																	temp_unsigned_char_ptr += 2;
																} break;
																case TIFF_LONG_FIELD:
																{
																	*strip_offset = (signed long)
																		UNSIGNED_LONG_INT_FROM_4_BYTES(temp_unsigned_char_ptr);
																	temp_unsigned_char_ptr += 4;
																} break;
																default:
																{
																	DEALLOCATE(strip_offsets);
																	return_code = 0;
																} break;
															}
#if defined (DEBUG)
															/*???debug */
															printf(" %ld", *strip_offset);
#endif /* defined (DEBUG) */
															strip_offset++;
															field_count--;
														}
													}
													else
													{
														return_code = 0;
													}
													DEALLOCATE(field_values);
												}
												else
												{
													return_code=0;
												}
#if defined (DEBUG)
												/*???debug */
												if (return_code)
												{
													printf("\n");
												}
												else
												{
													printf(" invalid\n");
												}
#endif /* defined (DEBUG) */
											} break;
											case 277: /* samples/pixel */
											{
#if defined (DEBUG)
												/*???debug */
												printf("samples per pixel");
#endif /* defined (DEBUG) */
												if ((1==field_count)&&(TIFF_SHORT_FIELD==field_type))
												{
													samples_per_pixel =
														UNSIGNED_SHORT_INT_FROM_2_BYTES(field_values);
#if defined (DEBUG)
													/*???debug */
													printf("=%d\n",samples_per_pixel);
#endif /* defined (DEBUG) */
												}
												else
												{
#if defined (DEBUG)
													/*???debug */
													printf(" invalid\n");
#endif /* defined (DEBUG) */
													return_code=0;
												}
												DEALLOCATE(field_values);
											} break;
											case 278: /* rows/strip */
											{
#if defined (DEBUG)
												/*???debug */
												printf("rows per strip");
#endif /* defined (DEBUG) */
												if (1==field_count)
												{
													switch (field_type)
													{
														case TIFF_SHORT_FIELD:
														{
															rows_per_strip =
																UNSIGNED_SHORT_INT_FROM_2_BYTES(field_values);
														} break;
														case TIFF_LONG_FIELD:
														{
															rows_per_strip =
																UNSIGNED_LONG_INT_FROM_4_BYTES(field_values);
														} break;
														default:
														{
															return_code=0;
														}
													}
												}
												else
												{
													return_code=0;
												}
												DEALLOCATE(field_values);
#if defined (DEBUG)
												/*???debug */
												if (return_code)
												{
													printf("=%ld\n",rows_per_strip);
												}
												else
												{
													printf(" invalid\n");
												}
#endif /* defined (DEBUG) */
											} break;
											case 279: /* byte count/strip */
											{
#if defined (DEBUG)
												/*???debug */
												printf("strip byte counts");
#endif /* defined (DEBUG) */
												if ((0 < number_of_strips) &&
													(field_count == number_of_strips))
												{
													if (ALLOCATE(strip_byte_counts, unsigned long int,
														field_count))
													{
														strip_byte_count = strip_byte_counts;
														temp_unsigned_char_ptr = field_values;
														while ((0 < field_count) && strip_byte_counts)
														{
															switch (field_type)
															{
																case TIFF_SHORT_FIELD:
																{
																	*strip_byte_count =
																		UNSIGNED_SHORT_INT_FROM_2_BYTES(temp_unsigned_char_ptr);
																	temp_unsigned_char_ptr += 2;
																} break;
																case TIFF_LONG_FIELD:
																{
																	*strip_byte_count =
																		UNSIGNED_LONG_INT_FROM_4_BYTES(temp_unsigned_char_ptr);
																	temp_unsigned_char_ptr += 4;
																} break;
																default:
																{
																	DEALLOCATE(strip_byte_counts);
																	return_code = 0;
																} break;
															}
#if defined (DEBUG)
															/*???debug */
															printf(" %ld", *strip_byte_count);
#endif /* defined (DEBUG) */
															strip_byte_count++;
															field_count--;
														}
													}
													else
													{
														return_code=0;
													}
													DEALLOCATE(field_values);
												}
												else
												{
													return_code=0;
												}
#if defined (DEBUG)
												/*???debug */
												if (return_code)
												{
													printf("\n");
												}
												else
												{
													printf(" invalid\n");
												}
#endif /* defined (DEBUG) */
											} break;
											case 282: /* x resolution */
											{
#if defined (DEBUG)
												/*???debug */
												printf("x_resolution");
#endif /* defined (DEBUG) */
												if ((1 == field_count) &&
													(TIFF_RATIONAL_FIELD == field_type) &&
													(0 < UNSIGNED_LONG_INT_FROM_4_BYTES(field_values + 4)))
												{
													x_resolution =
														(float)UNSIGNED_LONG_INT_FROM_4_BYTES(field_values) /
														(float)UNSIGNED_LONG_INT_FROM_4_BYTES(field_values + 4);
#if defined (DEBUG)
													/*???debug */
													printf("=%g\n",x_resolution);
#endif /* defined (DEBUG) */
												}
												else
												{
#if defined (DEBUG)
													/*???debug */
													printf(" invalid\n");
#endif /* defined (DEBUG) */
													return_code=0;
												}
												DEALLOCATE(field_values);
											} break;
											case 283: /* y resolution */
											{
#if defined (DEBUG)
												/*???debug */
												printf("y_resolution");
#endif /* defined (DEBUG) */
												if ((1 == field_count)&&
													(TIFF_RATIONAL_FIELD == field_type)&&
													(0 < UNSIGNED_LONG_INT_FROM_4_BYTES(field_values + 4)))
												{
													y_resolution =
														(float)UNSIGNED_LONG_INT_FROM_4_BYTES(field_values) /
														(float)UNSIGNED_LONG_INT_FROM_4_BYTES(field_values + 4);
#if defined (DEBUG)
													/*???debug */
													printf("=%g\n",y_resolution);
#endif /* defined (DEBUG) */
												}
												else
												{
#if defined (DEBUG)
													/*???debug */
													printf(" invalid\n");
#endif /* defined (DEBUG) */
													return_code=0;
												}
												DEALLOCATE(field_values);
											} break;
											case 284: /* planar configuration */
											{
#if defined (DEBUG)
												/*???debug */
												printf("planar configuration");
#endif /* defined (DEBUG) */
												if ((1==field_count)&&(TIFF_SHORT_FIELD==field_type))
												{
													planar_configuration =
														UNSIGNED_SHORT_INT_FROM_2_BYTES(field_values);
#if defined (DEBUG)
													/*???debug */
													printf("=%d\n",planar_configuration);
#endif /* defined (DEBUG) */
												}
												else
												{
#if defined (DEBUG)
													/*???debug */
													printf(" invalid\n");
#endif /* defined (DEBUG) */
													return_code=0;
												}
												DEALLOCATE(field_values);
											} break;
											case 296: /* resolution unit */
											{
#if defined (DEBUG)
												/*???debug */
												printf("resolution unit");
#endif /* defined (DEBUG) */
												if ((1==field_count)&&(TIFF_SHORT_FIELD==field_type))
												{
													resolution_unit =
														UNSIGNED_SHORT_INT_FROM_2_BYTES(field_values);
#if defined (DEBUG)
													/*???debug */
													printf("=%d\n",resolution_unit);
#endif /* defined (DEBUG) */
												}
												else
												{
#if defined (DEBUG)
													/*???debug */
													printf(" invalid\n");
#endif /* defined (DEBUG) */
													return_code=0;
												}
												DEALLOCATE(field_values);
											} break;
											default:
											{
												DEALLOCATE(field_values);
											} break;
										}
#if defined (DEBUG)
										/*???debug */
										printf("\n");
#endif /* defined (DEBUG) */
									}
									number_of_fields--;
								}
								if (return_code)
								{
#if defined (DEBUG)
									/*???debug */
									for (i=0;i<number_of_strips;i++)
									{
										printf("strip offset %d: %d %d\n",i,strip_offsets[i],
											strip_byte_counts[i]);
									}
/*
									printf("strip 1\n");
									fseek(tiff_file,*strip_offsets,SEEK_SET);
									for (i= *strip_byte_counts;i>0;i--)
									{
										fread(&byte,sizeof(unsigned char),1,tiff_file);
										printf("%02x ",byte);
									}
									printf("\n");
*/
#endif /* defined (DEBUG) */
									/* classify image */
									switch (photometric_interpretation)
									{
										case TIFF_WHITE_IS_ZERO:
										case TIFF_BLACK_IS_ZERO:
										{
											/* bi-level or grey scale image */
											switch (bits_per_sample)
											{
												case 0: case 1:
												{
#if defined (DEBUG)
													/*???debug */
													printf("bi-level image\n");
#endif /* defined (DEBUG) */
													/* bi-level image */
													/* check fields */
													if ((image_width>0)&&(image_length>0)&&
														(1==samples_per_pixel)&&
														((TIFF_NO_COMPRESSION_VALUE==compression)||
														(TIFF_HUFFMAN_COMPRESSION_VALUE==compression)||
														(TIFF_PACK_BITS_COMPRESSION_VALUE==compression))&&
														strip_offsets&&strip_byte_counts&&
														(number_of_strips>0)&&(rows_per_strip>0)&&
/*???DB.  Standard says that they must be in file, but not used here and not
	present in some files */
/*														(x_resolution>=0)&&(y_resolution>=0)&& */
														((TIFF_NO_ABSOLUTE_UNIT==resolution_unit)||
														(TIFF_INCH==resolution_unit)||
														(TIFF_CENTIMETER==resolution_unit)))
													{
														if (TIFF_WHITE_IS_ZERO==photometric_interpretation)
														{
															colour_0=0x00;
															colour_1=0x01;
														}
														else
														{
															colour_0=0x01;
															colour_1=0x00;
														}
														if (ALLOCATE(image,unsigned char,
															((image_width*image_length+3)/4)*4))
														{
															switch (compression)
															{
																case TIFF_NO_COMPRESSION_VALUE:
																{
#if defined (DEBUG)
																	/*???debug */
																	printf("no compression\n");
#endif /* defined (DEBUG) */
																	image_ptr=image;
																	strip_offset=strip_offsets;
																	strip_byte_count=strip_byte_counts;
																	strip=(unsigned char *)NULL;
																	column_number=image_width;
																	while (return_code&&(number_of_strips>0))
																	{
																		byte_count= *strip_byte_count;
																		if ((0==fseek(tiff_file,*strip_offset,
																			SEEK_SET))&&(REALLOCATE(new_strip,strip,
																			unsigned char,byte_count))&&
																			(byte_count==fread(new_strip,
																			sizeof(unsigned char),byte_count,
																			tiff_file)))
																		{
																			strip=new_strip;
																			while (byte_count>0)
																			{
																				byte= *new_strip;
																				bit=0x80;
																				while (bit)
																				{
																					if (bit&byte)
																					{
																						*image_ptr=colour_1;
																					}
																					else
																					{
																						*image_ptr=colour_0;
																					}
																					image_ptr++;
																					column_number--;
																					if (0==column_number)
																					{
																						bit=0x00;
																						column_number=image_width;
																					}
																					else
																					{
																						bit=bit>>1;
																					}
																				}
																				new_strip++;
																				byte_count--;
																			}
																		}
																		else
																		{
																			display_message(ERROR_MESSAGE,
																				"read_tiff_image_file.  Reading strip");
																			return_code=0;
																		}
																		number_of_strips--;
																		strip_offset++;
																		strip_byte_count++;
																	}
																	DEALLOCATE(strip);
																	if (return_code)
																	{
																		*width_address = (int)image_width;
																		*height_address = (int)image_length;
																		*image_address = image;
																		*number_of_components_address = 1;
																	}
																	else
																	{
																		DEALLOCATE(image);
																	}
																} break;
																case TIFF_HUFFMAN_COMPRESSION_VALUE:
																{
																	display_message(ERROR_MESSAGE,
					"read_tiff_image_file.  Huffman compression not currently supported");
																	DEALLOCATE(image);
																	return_code=0;
																} break;
																case TIFF_PACK_BITS_COMPRESSION_VALUE:
																{
																	display_message(ERROR_MESSAGE,
				"read_tiff_image_file.  Pack bits compression not currently supported");
																	DEALLOCATE(image);
																	return_code=0;
																} break;
															}
														}
														else
														{
															display_message(ERROR_MESSAGE,
									"read_tiff_image_file.  Insufficient memory for image array");
															return_code=0;
														}
													}
													else
													{
														display_message(ERROR_MESSAGE,
									"read_tiff_image_file.  Invalid field(s) for bi-level image");
														return_code=0;
													}
												} break;
												case 8:
												{
													/* grey scale image */
#if defined (DEBUG)
													/*???debug */
													printf("grey scale %ld %ld %d %d %d %p %p %ld %ld %d %d %d %d\n",
														image_width,image_length,samples_per_pixel,
														compression,TIFF_NO_COMPRESSION_VALUE,strip_offsets,
														strip_byte_counts,number_of_strips,rows_per_strip,
														resolution_unit,TIFF_NO_ABSOLUTE_UNIT,TIFF_INCH,
														TIFF_CENTIMETER);
#endif /* defined (DEBUG) */
													/* check fields */
													if ((image_width>0)&&(image_length>0)&&
														(1==samples_per_pixel)&&
														(TIFF_NO_COMPRESSION_VALUE==compression)&&
														strip_offsets&&strip_byte_counts&&
														(number_of_strips>0)&&(rows_per_strip>0)&&
/*???DB.  Standard says that they must be in file, but not used here and not
	present in some files */
/*														(x_resolution>=0)&&(y_resolution>=0)&& */
														((TIFF_NO_ABSOLUTE_UNIT==resolution_unit)||
														(TIFF_INCH==resolution_unit)||
														(TIFF_CENTIMETER==resolution_unit)))
													{
#if defined (DEBUG)
														/*???debug */
														printf("image_size=%ld\n",
															((image_width*image_length-1)/4+1)*4);
#endif /* defined (DEBUG) */
														if (ALLOCATE(image,unsigned char,
															((image_width*image_length+3)/4)*4))
														{
															switch (compression)
															{
																case TIFF_NO_COMPRESSION_VALUE:
																{
#if defined (DEBUG)
																	/*???debug */
																	printf("no compression\n");
#endif /* defined (DEBUG) */
																	image_ptr=image;
																	strip_offset=strip_offsets;
																	strip_byte_count=strip_byte_counts;
																	while (return_code&&(number_of_strips>0))
																	{
																		byte_count= *strip_byte_count;
#if defined (DEBUG)
																		/*???debug */
																		printf("%ld %ld %ld\n",number_of_strips,
																			byte_count,*strip_offset);
#endif /* defined (DEBUG) */
																		if ((0==fseek(tiff_file,*strip_offset,
																			SEEK_SET))&&(byte_count==fread(image_ptr,
																			sizeof(unsigned char),byte_count,
																			tiff_file)))
																		{
																			image_ptr += byte_count;
																		}
																		else
																		{
																			display_message(ERROR_MESSAGE,
																				"read_tiff_image_file.  Reading strip");
																			return_code=0;
																		}
																		number_of_strips--;
																		strip_offset++;
																		strip_byte_count++;
																	}
																	if (return_code)
																	{
																		*width_address = (int)image_width;
																		*height_address = (int)image_length;
																		*image_address = image;
																		/* reorder the rows */
																		image_ptr=image+
																			(image_width*(image_length-1));
																		while (image<image_ptr)
																		{
																			for (i=0;i<image_width;i++)
																			{
																				byte=image_ptr[i];
																				image_ptr[i]=image[i];
																				image[i]=byte;
																			}
																			image_ptr -= image_width;
																			image += image_width;
																		}
																		*number_of_components_address = 1;
																	}
																	else
																	{
																		DEALLOCATE(image);
																	}
																} break;
															}
														}
														else
														{
															display_message(ERROR_MESSAGE,
									"read_tiff_image_file.  Insufficient memory for image array");
															return_code=0;
														}
													}
													else
													{
														display_message(ERROR_MESSAGE,
								"read_tiff_image_file.  Invalid field(s) for grey scale image");
														return_code=0;
													}
												} break;
												default:
												{
													display_message(ERROR_MESSAGE,
										"read_tiff_image_file.  Unknown number of bits per sample");
													return_code=0;
												} break;
											}
										} break;
										case TIFF_RGB:
										{
											/* RGB full colour image */
#if defined (DEBUG)
											/*???debug */
											printf(
												"rgb %ld %ld %d %d %d %d %p %p %ld %ld %d %d %d %d\n",
												image_width,image_length,bits_per_sample,
												samples_per_pixel,compression,TIFF_NO_COMPRESSION_VALUE,
												strip_offsets,strip_byte_counts,number_of_strips,
												rows_per_strip,resolution_unit,TIFF_NO_ABSOLUTE_UNIT,
												TIFF_INCH,TIFF_CENTIMETER);
#endif /* defined (DEBUG) */
											/* check fields */
											if ((image_width>0)&&(image_length>0)&&
												(8==bits_per_sample)&&(3==samples_per_pixel)&&
												(TIFF_NO_COMPRESSION_VALUE==compression)&&
												strip_offsets&&strip_byte_counts&&
												(number_of_strips>0)&&(rows_per_strip>0)&&
/*???DB.  Standard says that they must be in file, but not used here and not
present in some files */
/*														(x_resolution>=0)&&(y_resolution>=0)&& */
												((TIFF_NO_ABSOLUTE_UNIT==resolution_unit)||
												(TIFF_INCH==resolution_unit)||
												(TIFF_CENTIMETER==resolution_unit)))
											{
#if defined (DEBUG)
												/*???debug */
												printf("image_size=%ld\n",
													((image_width*image_length*samples_per_pixel+3)/4)*4);
#endif /* defined (DEBUG) */
												if (ALLOCATE(image,unsigned char,
													((image_width*image_length*samples_per_pixel+3)/4)*4))
												{
													switch (compression)
													{
														case TIFF_NO_COMPRESSION_VALUE:
														{
#if defined (DEBUG)
															/*???debug */
															printf("no compression\n");
#endif /* defined (DEBUG) */
															image_ptr=image;
															strip_offset=strip_offsets;
															strip_byte_count=strip_byte_counts;
															while (return_code&&(number_of_strips>0))
															{
																byte_count= *strip_byte_count;
#if defined (DEBUG)
																/*???debug */
																printf("%ld %ld %ld\n",number_of_strips,
																	byte_count,*strip_offset);
#endif /* defined (DEBUG) */
																if ((0==fseek(tiff_file,*strip_offset,
																	SEEK_SET))&&(byte_count==fread(image_ptr,
																	sizeof(unsigned char),byte_count,
																	tiff_file)))
																{
																	image_ptr += byte_count;
																}
																else
																{
																	display_message(ERROR_MESSAGE,
																		"read_tiff_image_file.  Reading strip");
																	return_code=0;
																}
																number_of_strips--;
																strip_offset++;
																strip_byte_count++;
															}
															if (return_code)
															{
																*width_address = (int)image_width;
																*height_address = (int)image_length;
																*image_address = image;
																/* reorder the rows */
																image_ptr=image+(image_width*samples_per_pixel*
																	(image_length-1));
																while (image<image_ptr)
																{
																	for (i=0;i<image_width*samples_per_pixel;i++)
																	{
																		byte=image_ptr[i];
																		image_ptr[i]=image[i];
																		image[i]=byte;
																	}
																	image_ptr -= image_width*samples_per_pixel;
																	image += image_width*samples_per_pixel;
																}
																*number_of_components_address=samples_per_pixel;
															}
															else
															{
																DEALLOCATE(image);
															}
														} break;
													}
												}
												else
												{
													display_message(ERROR_MESSAGE,
														"read_tiff_image_file.  "
														"Insufficient memory for image array");
													return_code=0;
												}
											}
											else
											{
												if (TIFF_NO_COMPRESSION_VALUE==compression)
												{
													display_message(ERROR_MESSAGE,
														"read_tiff_image_file.  "
														"Invalid field(s) for rgb image");
												}
												else
												{
													display_message(ERROR_MESSAGE,
														"read_tiff_image_file.  "
														"Compression in tiff not supported for rgb image");
												}
												return_code=0;
											}
										} break;
										case TIFF_PALETTE_COLOUR:
										{
											/* palette colour image */
											display_message(ERROR_MESSAGE, "read_tiff_image_file.  "
												"Palette colour images not currently supported");
											return_code=0;
										} break;
										default:
										{
											display_message(ERROR_MESSAGE, "read_tiff_image_file.  "
												"Unknown photometric interpretation");
											return_code=0;
										} break;
									}
								}
								if (strip_byte_counts)
								{
									DEALLOCATE(strip_byte_counts);
								}
								if (strip_offsets)
								{
									DEALLOCATE(strip_offsets);
								}
							} /* number of fields */
						} /* image file directory */
					}
				} /* file number */
			} /* byte order */
			fclose(tiff_file);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_tiff_image_file.  Could not open image file '%s'",file_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"read_tiff_image_file.  Invalid argument(s)");
	}
#if defined (DEBUG)
	/*???debug */
	printf("leave read_tiff_image_file\n");
#endif /* defined (DEBUG) */
	/* Keep these in case we want to use them sometime */
	USE_PARAMETER(planar_configuration);
	USE_PARAMETER(x_resolution);
	USE_PARAMETER(y_resolution);
	LEAVE;

	return (return_code);
} /* read_tiff_image_file */

int read_yuv_image_file(char *file_name,
	int *width_address, int *height_address, int *number_of_components_address,
	int *number_of_bytes_per_component_address, unsigned char **image_address)
/*******************************************************************************
LAST MODIFIED : 14 February 2002

DESCRIPTION :
Reads an image from a YUV file, returning it as an RGB image.
YUV images are commonly used for video output since they store just 4 bytes for
every 2 pixels in the order u y1 v y2. y1 and y2 effectively store the intensity
for each pixel. u and v, combined with the appropriate y then give the RGB
colour of the pixels, hence the colour resolution is somewhat reduced.
Refer to the code for the conversion to RGB.
YUV files have no header; the image dimensions are computed from the file size.
Several standard video sizes are supported as described in the code. Support for
other resolutions is given by specifying the width, and optionally the height
in the <width_address> and <height_address> locations.
If just the width is specified, the height is computed from the file size.
==============================================================================*/
{
#define YUV_LIMIT(x) \
((((x)>0xffffff)?0xff0000:(((x)<=0xffff)?0:(x)&0xff0000))>>16)
	unsigned char *image,*image_ptr,*yuv;
	FILE *image_file;
	int b, bytes_read, g, height, j, num_yyuv, r, rest_of_line, return_code, row,
		row_size, u, v,width, y1, y2;
	long int file_size;
	struct stat buf;

	ENTER(read_yuv_image_file);
	if (file_name && width_address && height_address &&
		number_of_components_address && number_of_bytes_per_component_address &&
		image_address)
	{
		return_code=1;
		if (image_file=fopen(file_name,"rb"))
		{
			/* use stat to get size of file to determine format from */
			if ((0==stat(file_name,&buf))&&(0<(file_size=(long int)(buf.st_size))))
			{
				if (0 < (width = *width_address))
				{
					/* Limit width to multiple of 4 so that final RGB is 4 byte aligned
						 and there are an even number of pixels for the u y1 v y2 quad */
					if (0 == (width % 4))
					{
						if (0 >= (height = *height_address))
						{
							/* set height to include all data, padding rest of last line */
							height = (int)((file_size + width*2 - 1) / (width*2));
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"read_yuv_image_file.  "
							"Width must be a multiple of 4 pixels");
						return_code = 0;
					}
				}
				else if (0 < (height = *height_address))
				{
					display_message(ERROR_MESSAGE,"read_yuv_image_file.  "
						"Must specify width if height is to specified");
					return_code=0;
				}
				else if (!get_yuv_resolution_from_file_size(file_size, &width, &height))
				{
					display_message(ERROR_MESSAGE, "read_yuv_image_file.  "
						"Size of YUV/UYVY image %s needs to be user-specified", file_name);
					return_code = 0;
				}
				if (return_code)
				{
					/* YUV format uses 2 bytes per pixel,
						 final image will be 3 bytes per pixel RGB */
					if (ALLOCATE(image, unsigned char, width*height*3))
					{
						/* convert image from top-to-bottom to bottom-to-top for
							 OpenGL image/texture storage */
						row_size=width*3;
						for (row=1;(row <= height)&&return_code;row++)
						{
							image_ptr = image + (height-row)*row_size;
							/* since yuv file has 2 bytes for every 3 in final rgb, read a
								 scanline into last 2/3 of rgb scanline and convert across */
							yuv = image_ptr + width;
							if (0<(bytes_read=fread(yuv,/*value_size*/1,width*2,image_file)))
							{
								num_yyuv = bytes_read/4;
								for (j=0;j<num_yyuv;j++)
								{
									u  = *yuv++ - 128;
									y1 = *yuv++ - 16;
									if (0>y1)
									{
										y1=0;
									}
									v  = *yuv++ - 128;
									y2 = *yuv++ - 16;
									if (0>y2)
									{
										y2=0;
									}
									r = 104635*v;
									g = -25690*u - 53294*v;
									b = 132278*u;
									
									y1 *= 76310;
									y2 *= 76310;
#if defined (SAVE_CODE)
#define YUV_OFFSET 16.0
#define YUV_RANGE  332.0
									/* Paul Charette's "adjustment" of YUV so that minimal
										 clipping occurs... this is down before the final
										 clipping to 0..255 range, ie with R meaning r+y1 etc. */
									R = (R + YUV_OFFSET)*(255.0/YUV_RANGE);
									G = (G + YUV_OFFSET)*(255.0/YUV_RANGE);
									B = (B + YUV_OFFSET)*(255.0/YUV_RANGE);
#endif /* defined (SAVE_CODE) */
									*image_ptr++ = YUV_LIMIT(r+y1);
									*image_ptr++ = YUV_LIMIT(g+y1);
									*image_ptr++ = YUV_LIMIT(b+y1);
									*image_ptr++ = YUV_LIMIT(r+y2);
									*image_ptr++ = YUV_LIMIT(g+y2);
									*image_ptr++ = YUV_LIMIT(b+y2);
								}
								if (num_yyuv < (width/2))
								{
									/* fill rest of line with zeros */
									rest_of_line=(width - num_yyuv*2)*3;
									memset(image_ptr,0,rest_of_line);
									image_ptr += rest_of_line;
								}
							}
							else
							{
								/* fill line with zeros */
								rest_of_line=width*3;
								memset(image_ptr,0,rest_of_line);
								image_ptr += rest_of_line;
							}
						}
						if (return_code)
						{
							*width_address = width;
							*height_address = height;
							*number_of_components_address = 3;
							*number_of_bytes_per_component_address = 1;
							*image_address = image;
						}
						else
						{
							DEALLOCATE(image);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_yuv_image_file.  Not enough memory");
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_yuv_image_file.  Could not get size of file '%s'",file_name);
				return_code=0;
			}
			fclose(image_file);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_yuv_image_file.  Could not open image file '%s'",file_name);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"read_yuv_image_file.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* read_yuv_image_file */
#endif /* ! defined (IMAGEMAGICK) */

#if defined (SAVE_CODE)

#if defined (IMAGEMAGICK)
int read_image_file(char *filename,int *number_of_components,
	int *number_of_bytes_per_component,long int *height,
	long int *width, long unsigned **image)
/*******************************************************************************
LAST MODIFIED : 10 May 2001

DESCRIPTION :
Detects the image type from the file extension (rgb/tiff) and then reads it.
For formats that do not have a header, eg. RAW and YUV, a width and height may
be specified in the <width> and <height> arguments.
==============================================================================*/
{
	char RGBA[] = "RGBA", RGB[] = "RGB", *pixel_storage;
	int i, length, return_code;
	Image *magick_image;
	ImageInfo *magick_image_info;
	ExceptionInfo magick_exception;
	long int file_size;
	struct stat buf;
	unsigned char *image_char;
	void *image_data;
	
	ENTER(read_image_file);
	if (filename && number_of_components && height && width && image)
	{
		/* SAB.  Note that the data returned from this routine is
			stored bottom to top and must be flipped to conform
			with the top to bottom storage normally used by Cmgui */
		GetExceptionInfo(&magick_exception);
		if (magick_image_info=CloneImageInfo((ImageInfo *) NULL))
		{
			/* Copy it now but we may overwrite if a special case */
			strcpy(magick_image_info->filename,filename);
			if (!strchr(filename, ':'))
			{
				/* Only add prefixes if there isn't one already and we want to
				 do something tricky */
				length = strlen(filename);
				/* Test to see if a file with suffix rgb is an sgi rgb file */
				if ((length > 4) && 
					fuzzy_string_compare_same_length((filename + length - 4), ".rgb"))
				{
					sprintf(magick_image_info->filename, "sgi:%s", filename);
				}
				/* Test to see if a file with suffix yuv is a standard size uyvy file */
				else if (!*width && !*height && (((length > 4) &&
					fuzzy_string_compare_same_length((filename + length - 4), ".yuv"))
					|| ((length > 5) &&
					fuzzy_string_compare_same_length((filename + length - 5), ".uyvy"))))
				{
					/* try to get width and height from the file_size */
					if ((0 == stat(filename, &buf)) &&
						(0 < (file_size = (long int)(buf.st_size))) &&
						get_yuv_resolution_from_file_size(file_size, width, height))
					{
						sprintf(magick_image_info->filename, "uyvy:%s", filename);
					}					
				}
			}
			if (*width && *height)
			{
				if(ALLOCATE(magick_image_info->size, char, 25))
				{
					sprintf(magick_image_info->size, "%ldx%ld", *width, *height);
				}
			}
			/* make the default depth 8 bits for raw files */
			magick_image_info->depth = 8;
#if defined (DEBUG)
			magick_image_info->verbose = 1;
#endif /* defined (DEBUG) */
			magick_image=ReadImage(magick_image_info,&magick_exception);
			if (magick_image)
			{
				if (magick_image->matte)
				{
					*number_of_components = 4;
					pixel_storage = RGBA;
				}
				else
				{
					*number_of_components = 3;
					pixel_storage = RGB;
				}
				*number_of_bytes_per_component = 1;
				/* Casting to suppress the warnings about loss of precision */
				*width = (long int)magick_image->columns;
				*height = (long int)magick_image->rows;
				if (ALLOCATE(image_data, unsigned char,
					(*width) * (*height) * (*number_of_components) *
					(*number_of_bytes_per_component)))
				{
					DispatchImage(magick_image, 0, 0, *width, *height,
						pixel_storage, CharPixel, image_data);
					*image = (long unsigned *)image_data;
					if (RGBA == pixel_storage)
					{
						/* Invert the alpha channel */
						image_char = (unsigned char *)image_data + 3;
						for (i = 0 ; i < *width * *height ; i++)
						{
							*image_char = 0xff - *image_char;
							image_char += 4;
						}
					}
					return_code = 1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"read_image_file.  Could not allocate memory for image data");
					return_code = 0;
				}
				DestroyImage(magick_image);

			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Could not read image %s", filename);
				return_code = 0;
			}
			DestroyImageInfo(magick_image_info);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_image_file.  Could not create image information");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "read_image_file.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* read_image_file */
#else /* defined (IMAGEMAGICK) */
int read_image_file(char *file_name,int *number_of_components,
	int *number_of_bytes_per_component,long int *height,long int *width,
	enum Raw_image_storage raw_image_storage, long unsigned **image)
/*******************************************************************************
LAST MODIFIED : 12 October 2000

DESCRIPTION :
Detects the image type from the file extension (rgb/tiff) and then reads it.
For formats that do not have a header, eg. RAW and YUV, a width and height may
be specified in the <width> and <height> arguments. For the RAW format, the
<raw_image_storage> is needed.
==============================================================================*/
{
	enum Image_file_format image_file_format;
	int return_code;

	ENTER(read_image_file);
	if (file_name && number_of_components && height && width && image)
	{
		if (Image_file_format_from_file_name(file_name, &image_file_format))
		{
			switch (image_file_format)
			{
				case POSTSCRIPT_FILE_FORMAT:
				{
					display_message(ERROR_MESSAGE,
						"Cannot read postscript image file '%s'", file_name);
					return_code = 0;
				} break;
				case RAW_FILE_FORMAT:
				{
					return_code = read_raw_image_file(file_name,number_of_components,
						number_of_bytes_per_component,height,width,raw_image_storage,image);
				} break;
				case RGB_FILE_FORMAT:
				{
					return_code = read_rgb_image_file(file_name,number_of_components,
						number_of_bytes_per_component,height,width,image);
				} break;
				case TIFF_FILE_FORMAT:
				{
					return_code = read_tiff_image_file(file_name,number_of_components,
						number_of_bytes_per_component,height,width,image);
				} break;
				case YUV_FILE_FORMAT:
				{
					return_code = read_yuv_image_file(file_name,number_of_components,
						number_of_bytes_per_component,height,width,image);
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"read_image_file.  Unknown image file format");
					return_code = 0;
				} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_image_file.  Could not determine image format of file '%s'",
				file_name);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "read_image_file.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* read_image_file */
#endif /* defined (IMAGEMAGICK) */

int undistort_image(unsigned long **image,
	int number_of_components,int number_of_bytes_per_component,
	int width,int height,
	double centre_x,double centre_y,double factor_k1)
/*******************************************************************************
LAST MODIFIED : 26 April 1998

DESCRIPTION :
Removes radial distortion centred at <centre_x,centre_y> from the image.
Distortion factor k1 works to correct distortion according to:
x(corrected)=x(distorted) + k1*x(distorted)*r*r
where:
1. Coordinates x (and similarly y) are measured from the distortion centre
2. r*r=x(distorted)*x(distorted)+y(distorted)*y(distorted)
This routine performs an approximation for the inverse mapping of the above. It
scans through the corrected coordinates building up a corrected copy of the
original image. Each point on the corrected image is replaced by a blending of
the four pixels on the original image at the equivalent distorted position.
On successful return, the image at <image> is replaced by the
corrected version.
==============================================================================*/
{
	double x_corr,y_corr,x_dist,y_dist,radius_squared,blend_value[4],
		blend_factor,radius_squared_dist,k1_dist;
	int return_code,i,j,k,i_dist,j_dist;
	unsigned long *distorted_image,*corrected_image;
	unsigned char *pixel,*texture_pixel;

	ENTER(undistort_image);
	return_code=0;
	/* check arguments */
	if (image&&(distorted_image = *image))
	{
		if (number_of_bytes_per_component > 1)
		{
			if (0.0==factor_k1)
			{
				/* no correction necessary */
				return_code=1;
			}
			else
			{
#if defined (DEBUG)
				/*???debug */
				printf("Correcting radial distortion centre=%f,%f k1=%f\n",
					centre_x,centre_y,factor_k1);
#endif /* defined (DEBUG) */
				if (ALLOCATE(corrected_image,unsigned long,
					(width*height*number_of_components+3)/4))
				{
					pixel=(unsigned char *)corrected_image;
					for (j=0;j<height;j++)
					{
						for (i=0;i<width;i++)
						{
							/* make coordinates originate at centre of distortion */
							x_corr=(double)i-centre_x;
							y_corr=(double)j-centre_y;
							radius_squared=x_corr*x_corr+y_corr*y_corr;
							/* get distorted position to take pixel values from */
							x_dist = x_corr*(1.0+factor_k1*radius_squared);
							y_dist = y_corr*(1.0+factor_k1*radius_squared);
							radius_squared_dist=x_dist*x_dist+y_dist*y_dist;
							if (0.0<radius_squared_dist)
							{
								k1_dist=factor_k1*radius_squared/radius_squared_dist;
							}
							else
							{
								k1_dist=0.0;
							}
							x_dist=x_corr/(1.0+k1_dist*radius_squared);
							y_dist=y_corr/(1.0+k1_dist*radius_squared);
							/* add the centre back in */
							x_dist += centre_x;
							y_dist += centre_y;
							/* get integer location of distorted position */
							i_dist=(int)x_dist;
							j_dist=(int)y_dist;
							for (k=0;k<number_of_components;k++)
							{
								blend_value[k]=0.0;
							}
							/* left pixels */
							if ((0<=i_dist)&&(width>i_dist))
							{
								/* lower left pixel */
								if ((0<=j_dist)&&((height>j_dist)))
								{
									texture_pixel=(unsigned char *)distorted_image+
										(number_of_components*(j_dist*width+i_dist));
									blend_factor=((double)i_dist+1.0-x_dist)*
										((double)j_dist+1.0-y_dist);
									for (k=0;k<number_of_components;k++)
									{
										blend_value[k] += blend_factor*(*texture_pixel);
										texture_pixel++;
									}
								}
								/* upper left pixel */
								if ((0<=(j_dist+1))&&(height>(j_dist+1)))
								{
									texture_pixel=(unsigned char *)distorted_image+
										(number_of_components*((j_dist+1)*width+i_dist));
									blend_factor=((double)i_dist+1.0-x_dist)*
										(y_dist-(double)j_dist);
									for (k=0;k<number_of_components;k++)
									{
										blend_value[k] += blend_factor*(*texture_pixel);
										texture_pixel++;
									}
								}
							}
							/* right pixels */
							if ((0<=(i_dist+1))&&(width>(i_dist+1)))
							{
								/* lower right pixel */
								if ((0<=j_dist)&&((height>j_dist)))
								{
									texture_pixel=(unsigned char *)distorted_image+
										(number_of_components*(j_dist*width+i_dist+1));
									blend_factor=(x_dist-(double)i_dist)*
										((double)j_dist+1.0-y_dist);
									for (k=0;k<number_of_components;k++)
									{
										blend_value[k] += blend_factor*(*texture_pixel);
										texture_pixel++;
									}
								}
								/* upper right pixel */
								if ((0<=(j_dist+1))&&(height>(j_dist+1)))
								{
									texture_pixel=(unsigned char *)distorted_image+
										(number_of_components*((j_dist+1)*width+i_dist+1));
									blend_factor=(x_dist-(double)i_dist)*
										(y_dist-(double)j_dist);
									for (k=0;k<number_of_components;k++)
									{
										blend_value[k] += blend_factor*(*texture_pixel);
										texture_pixel++;
									}
								}
							}
							/* now put the values just found in the corrected image */
							for (k=0;k<number_of_components;k++)
							{
								*pixel = (unsigned char)blend_value[k];
								pixel++;
							}
						}
					}
					DEALLOCATE(distorted_image);
					*image=corrected_image;
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"undistort_image.  Could not create corrected image");
					return_code=0;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"undistort_image"
				"  Not implemented for more than 1 byte per component");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"undistort_image.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* undistort_image */

unsigned long *copy_image(unsigned long *image,int number_of_components,
	int width,int height)
/*******************************************************************************
LAST MODIFIED : 27 April 1998

DESCRIPTION :
Allocates and returns a copy of <image>.
==============================================================================*/
{
	long unsigned *return_image,image_size;

	ENTER(copy_image);
	if (image&&(0<number_of_components)&&(0<=width)&&(0<=height))
	{
		image_size=(width*height*number_of_components+3)/4;
		if (ALLOCATE(return_image,unsigned long,image_size))
		{
			memcpy(return_image,image,image_size*4);
		}
		else
		{
			display_message(ERROR_MESSAGE,"copy_image.  Invalid argument(s)");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"copy_image.  Invalid argument(s)");
		return_image=(long unsigned *)NULL;
	}
	LEAVE;

	return (return_image);
} /* copy_image */

int crop_image(unsigned long **image,int number_of_components,
	int number_of_bytes_per_component,
	int *width,int *height,int crop_left_margin,
	int crop_bottom_margin,int crop_width,int crop_height)
/*******************************************************************************
LAST MODIFIED : 26 April 1998

DESCRIPTION :
Crops <image> (at *image) from its present <width>,<height> to be
<crop_width>,<crop_height> in size offset from the bottom left of the image
by <crop_left_margin>,<crop_bottom_margin>.
The <image> is reallocated to its new size and its new dimensions are put in
<*width>,<*height>.
If <crop_width> and <crop_height> are not both positive or if
<left_margin_texels> and <bottom_margin_texels> are not both non-negative or
if the cropping region is not contained in the image then no cropping is
performed and the original image is returned.
==============================================================================*/
{
	int i,return_code,destination_row_width_bytes,number_of_bytes,original_width,
		original_height,source_row_width_bytes;
	long unsigned *source_image,*cropped_image;
	unsigned char *source,*destination;

	ENTER(crop_image);
	if (image&&(source_image= *image)&&width&&height)
	{
		number_of_bytes = number_of_components * number_of_bytes_per_component;
		original_width= *width;
		original_height= *height;
		if ((0<=crop_left_margin)&&(0<=crop_bottom_margin)&&
			(0<crop_width)&&(0<crop_height)&&
			(crop_left_margin+crop_width<=original_width)&&
			(crop_bottom_margin+crop_height<=original_height))
		{
			destination_row_width_bytes=crop_width*number_of_bytes;
			source_row_width_bytes=original_width*number_of_bytes;
			destination=source=(unsigned char *)source_image;
			source += ((crop_bottom_margin*original_width+crop_left_margin)*
				number_of_bytes);
			for (i=crop_height;i>0;i--)
			{
				memcpy((void *)destination,(void *)source,destination_row_width_bytes);
				source += source_row_width_bytes;
				destination += destination_row_width_bytes;
			}
			if (REALLOCATE(cropped_image,source_image,unsigned long,
				(crop_width*crop_height*number_of_bytes+3)/4))
			{
				source_image=cropped_image;
				*width=crop_width;
				*height=crop_height;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,"crop_image.  Could not crop image");
				return_code=0;
			}
		}
		else
		{
			/* no cropping necessary */
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"crop_image.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* crop_image */

#endif /* defined (SAVE_CODE) */

int get_radial_distortion_corrected_coordinates(double dist_x,double dist_y,
	double dist_centre_x,double dist_centre_y,double dist_factor_k1,
	double *corr_x,double *corr_y)
/*******************************************************************************
LAST MODIFIED : 30 April 1998

DESCRIPTION :
Returns the position point <dist_x,dist_y> would be at if there was no radial
distortion in the lens used to record them.
Distortion factor k1 works to correct distortion according to:
corrected_x = distorted_x + k1*distorted_x*r*r
where:
1. Coordinates x (and similarly y) are measured from the centre of distortion.
2. r*r = distorted_x*distorted_x + distorted_y*distorted_y
==============================================================================*/
{
	int return_code;
	double xd,yd,rr;

	ENTER(get_radial_distortion_corrected_coordinates);
	if (corr_x&&corr_y)
	{
		xd=dist_x-dist_centre_x;
		yd=dist_y-dist_centre_y;
		rr=xd*xd+yd*yd;
		*corr_x = dist_centre_x + xd*(1+dist_factor_k1*rr);
		*corr_y = dist_centre_y + yd*(1+dist_factor_k1*rr);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_radial_distortion_corrected_coordinates.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_radial_distortion_corrected_coordinates */

int get_radial_distortion_distorted_coordinates(double corr_x,double corr_y,
	double dist_centre_x,double dist_centre_y,double dist_factor_k1,
	double tolerance,double *dist_x,double *dist_y)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Returns the position point <corr_x,corr_y> would be at in the radial distorted
lens system. Inverse of get_radial_distortion_corrected_coordinates.
Iterative routine requires a tolerance to be set on how little the distorted
radius shifts in an iteration to be an acceptable solution.
Allows up to around 10% distortion to be corrected. (This is a large value!)
==============================================================================*/
{
	int return_code,converged,iters;
	double xc,yc,xd,yd,last_xd,last_yd,max_shift,rr,tol_tol;

	ENTER(get_radial_distortion_distorted_coordinates);
	if (dist_x&&dist_y&&(0<tolerance))
	{
		tol_tol=tolerance*tolerance;
		last_xd=xd=xc=corr_x-dist_centre_x;
		last_yd=yd=yc=corr_y-dist_centre_y;
		rr=xd*xd+yd*yd;
		max_shift=dist_factor_k1*rr;
		if ((-0.1<max_shift)&&(1.0>max_shift))
		{
			converged=iters=0;
			while (!converged)
			{
				iters++;
				xd=xc/(1.0+dist_factor_k1*rr);
				yd=yc/(1.0+dist_factor_k1*rr);
				if (!(converged=((100==iters)||
					(((xd-last_xd)*(xd-last_xd)+(yd-last_yd)*(yd-last_yd))<tol_tol))))
				{
					rr=xd*xd+yd*yd;
					last_xd=xd;
					last_yd=yd;
				}
			}
			*dist_x=dist_centre_x+xd;
			*dist_y=dist_centre_y+yd;
			return_code=1;
		}
		else
		{
#if defined (OLD_CODE)
			/* now up to calling function to report error, if necessary */
			display_message(ERROR_MESSAGE,
				"get_radial_distortion_distorted_coordinates.  "
				"Maximum shift of %.1f is too great",max_shift);
#endif /* defined (OLD_CODE) */
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_radial_distortion_distorted_coordinates.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_radial_distortion_distorted_coordinates */

struct Cmgui_image_information *CREATE(Cmgui_image_information)(void)
/*******************************************************************************
LAST MODIFIED : 1 March 2002

DESCRIPTION :
Creates a blank Cmgui_image_information.
To read in an image must set at least one file name in this structure; for raw
or yuv image formats, width, height and raw_image_storage may need to be set.
To create an image need to specify most dimension arguments.
==============================================================================*/
{
	struct Cmgui_image_information *cmgui_image_information;

	ENTER(CREATE(Cmgui_image_information));
	if (ALLOCATE(cmgui_image_information, struct Cmgui_image_information, 1))
	{
		cmgui_image_information->valid = 1;
		cmgui_image_information->number_of_file_names = 0;
		cmgui_image_information->file_names = (char **)NULL;
		cmgui_image_information->height = 0;
		cmgui_image_information->image_file_format = UNKNOWN_IMAGE_FILE_FORMAT;
		cmgui_image_information->number_of_bytes_per_component = 1;
		cmgui_image_information->number_of_components = 3;
		cmgui_image_information->width = 0;
		cmgui_image_information->raw_image_storage = RAW_INTERLEAVED_RGB;
		cmgui_image_information->background_number_of_fill_bytes = 0;
		cmgui_image_information->background_fill_bytes = (unsigned char *)NULL;
		cmgui_image_information->io_stream_package = (struct IO_stream_package *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Cmgui_image_information).  Could not allocate structure");
	}
	LEAVE;

	return (cmgui_image_information);
} /* CREATE(Cmgui_image_information) */

int DESTROY(Cmgui_image_information)(
	struct Cmgui_image_information **cmgui_image_information_address)
/*******************************************************************************
LAST MODIFIED : 26 February 2002

DESCRIPTION :
Frees the memory use by the Cmgui_image_information and sets
<*cmgui_image_information_address> to NULL.
==============================================================================*/
{
	int i, return_code;
	struct Cmgui_image_information *cmgui_image_information;

	ENTER(DESTROY(Cmgui_image_information));
	if (cmgui_image_information_address &&
		(cmgui_image_information = *cmgui_image_information_address))
	{
		if (cmgui_image_information->file_names)
		{
			for (i = 0; i < cmgui_image_information->number_of_file_names; i++)
			{
				DEALLOCATE(cmgui_image_information->file_names[i]);
			}
			DEALLOCATE(cmgui_image_information->file_names);
		}
		if (cmgui_image_information->background_fill_bytes)
		{
			DEALLOCATE(cmgui_image_information->background_fill_bytes);
		}
		DEALLOCATE(*cmgui_image_information_address);
		*cmgui_image_information_address = (struct Cmgui_image_information *)NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Cmgui_image_information).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Cmgui_image_information) */

int Cmgui_image_information_add_file_name(
	struct Cmgui_image_information *cmgui_image_information, char *file_name)
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Adds the <file_name> to the end of the list in <cmgui_image_information>.
Clears 'valid' flag if fails.
==============================================================================*/
{
	char *temp_file_name, **temp_file_names;
	int return_code;

	ENTER(Cmgui_image_information_add_file_name);
	if (cmgui_image_information && file_name)
	{
		if (temp_file_name = duplicate_string(file_name))
		{
			if (REALLOCATE(temp_file_names, cmgui_image_information->file_names,
				char *, cmgui_image_information->number_of_file_names + 1))
			{
				/* add another file_name */
				temp_file_names[cmgui_image_information->number_of_file_names] =
					temp_file_name;
				cmgui_image_information->file_names = temp_file_names;
				cmgui_image_information->number_of_file_names++;
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmgui_image_information_add_file_name.  "
					"Could not enlarge file_names list");
				DEALLOCATE(temp_file_name);
				cmgui_image_information->valid = 0;
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmgui_image_information_add_file_name.  Could not copy file_name");
			cmgui_image_information->valid = 0;
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmgui_image_information_add_file_name.  Invalid argument(s)");
		if (cmgui_image_information)
		{
			cmgui_image_information->valid = 0;
		}
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmgui_image_information_add_file_name */

int Cmgui_image_information_set_file_name_series(
	struct Cmgui_image_information *cmgui_image_information,
	char *file_name_template, char *file_number_pattern, int start_file_number,
	int stop_file_number, int file_number_increment)
/*******************************************************************************
LAST MODIFIED : 19 March 2002

DESCRIPTION :
Adds a series of file names based on the <file_name_template> to the
<cmgui_image_information>. The numbers from <start_file_number> to
<stop_file_number> with <file_number_increment> are substituted for the first
instance of <file_number_pattern> in <file_name_template>.
The number appears with leading zeros up to the length of <file_number_pattern>.
The <keep_current> flag determines whether the new file list is appended or
replaces andy existing file list in the <cmgui_image_information>.
Clears 'valid' flag if fails.
==============================================================================*/
{
	char *file_number_string, format[20], *pattern_position, *prefix,
		*temp_file_name, **temp_file_names, *suffix;
	int error, file_number, i, new_number_of_file_names, pattern_width, return_code;

	ENTER(Cmgui_image_information_set_file_name_series);
	if (cmgui_image_information && file_name_template && file_number_pattern &&
		(0 < (pattern_width = strlen(file_number_pattern))) && (
		((start_file_number <= stop_file_number) && (0 < file_number_increment) &&
			(0 == (stop_file_number - start_file_number) % file_number_increment)) ||
		((start_file_number >= stop_file_number) && (0 > file_number_increment) &&
			(0 == (start_file_number - stop_file_number) % -file_number_increment)))
		&& (pattern_position = strstr(file_name_template, file_number_pattern)))
	{
		new_number_of_file_names = 1 + ((stop_file_number - start_file_number) /
			file_number_increment);
		if (ALLOCATE(temp_file_names, char *, new_number_of_file_names))
		{
			sprintf(format, "%%0%dd", pattern_width);
			prefix = file_name_template;
			*pattern_position = '\0';
			if ('\0' == *prefix)
			{
				prefix = (char *)NULL;
			}
			suffix = pattern_position + pattern_width;
			if ('\0' == *suffix)
			{
				suffix = (char *)NULL;
			}
			error = 0;
			if (ALLOCATE(file_number_string, char, 20 + pattern_width))
			{
				file_number = start_file_number;
				for (i = 0; (i < new_number_of_file_names) && !error; i++)
				{
					temp_file_name = (char *)NULL;
					if (prefix)
					{
						append_string(&temp_file_name, prefix, &error);
					}
					sprintf(file_number_string, format, file_number);
					append_string(&temp_file_name, file_number_string, &error);
					if (suffix)
					{
						append_string(&temp_file_name, suffix, &error);
					}
					if (!error)
					{
						temp_file_names[i] = temp_file_name;
					}
					file_number += file_number_increment;
				}
				if (error)
				{
					for (i = i - 1; i >= 0; i--)
					{
						DEALLOCATE(temp_file_names[i]);
					}
				}
				DEALLOCATE(file_number_string);
			}
			else
			{
				error = 1;
			}
			if (error)
			{
				DEALLOCATE(temp_file_names);
				display_message(ERROR_MESSAGE,
					"Cmgui_image_information_add_file_name_series.  "
					"Could not create file name series");
				cmgui_image_information->valid = 0;
				return_code = 0;
			}
			else
			{
				if (cmgui_image_information->number_of_file_names)
				{
					for (i = 0; i < cmgui_image_information->number_of_file_names ; i++)
					{
						DEALLOCATE(cmgui_image_information->file_names[i]);
					}
				}
				DEALLOCATE(cmgui_image_information->file_names);
				cmgui_image_information->file_names = temp_file_names;
				cmgui_image_information->number_of_file_names =
					new_number_of_file_names;
				return_code = 1;
			}
			/* leave file_number_template as it was */
			*pattern_position = file_number_pattern[0];
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmgui_image_information_add_file_name_series.  "
				"Could not enlarge file_names list");
			cmgui_image_information->valid = 0;
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmgui_image_information_add_file_name_series.  Invalid argument(s)");
		if (cmgui_image_information)
		{
			cmgui_image_information->valid = 0;
		}
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmgui_image_information_add_file_name_series */

int Cmgui_image_information_set_file_name(
	struct Cmgui_image_information *cmgui_image_information,
	int file_name_number, char *file_name)
/*******************************************************************************
LAST MODIFIED : 18 February 2002

DESCRIPTION :
Sets the <file_name> for <file_name_number> of <cmgui_image_information>.
Clears 'valid' flag if fails.
==============================================================================*/
{
	char *temp_file_name;
	int return_code;

	ENTER(Cmgui_image_information_set_file_name);
	if (cmgui_image_information && (0 <= file_name_number) &&
		(file_name_number < cmgui_image_information->number_of_file_names) &&
		file_name)
	{
		if (temp_file_name = duplicate_string(file_name))
		{
			/* change_existing file_name */
			DEALLOCATE(cmgui_image_information->file_names[file_name_number]);
			cmgui_image_information->file_names[file_name_number] = temp_file_name;
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmgui_image_information_set_file_name.  Could not copy file_name");
			cmgui_image_information->valid = 0;
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmgui_image_information_set_file_name.  Invalid argument(s)");
		if (cmgui_image_information)
		{
			cmgui_image_information->valid = 0;
		}
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmgui_image_information_set_file_name */

int Cmgui_image_information_set_height(
	struct Cmgui_image_information *cmgui_image_information, int height)
/*******************************************************************************
LAST MODIFIED : 26 February 2002

DESCRIPTION :
Sets the <height> recorded with the <cmgui_image_information>.
Used to specify the height for raw file formats read with Cmgui_image_read.
Clears 'valid' flag of cmgui_image_information if not correctly set.
==============================================================================*/
{
	int return_code;

	ENTER(Cmgui_image_information_set_height);
	if (cmgui_image_information)
	{
		if (0 <= height)
		{
			cmgui_image_information->height = height;
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmgui_image_information_set_height.  Negative height");
			cmgui_image_information->valid = 0;
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmgui_image_information_set_height.  Missing information");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmgui_image_information_set_height */

int Cmgui_image_information_set_image_file_format(
	struct Cmgui_image_information *cmgui_image_information,
	enum Image_file_format image_file_format)
/*******************************************************************************
LAST MODIFIED : 1 March 2002

DESCRIPTION :
Sets the <image_file_format> of <cmgui_image>.
If set to UNKNOWN_IMAGE_FILE_FORMAT then the format is determined from the
filename, otherwise it is forced from the format listed here.
==============================================================================*/
{
	int return_code;

	ENTER(Cmgui_image_information_set_image_file_format);
	if (cmgui_image_information)
	{
		cmgui_image_information->image_file_format = image_file_format;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmgui_image_information_set_image_file_format.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmgui_image_information_set_image_file_format */

int Cmgui_image_information_set_number_of_bytes_per_component(
	struct Cmgui_image_information *cmgui_image_information,
	int number_of_bytes_per_component)
/*******************************************************************************
LAST MODIFIED : 11 March 2002

DESCRIPTION :
Sets the <number_of_bytes_per_component> recorded with the
<cmgui_image_information>. Only valid values are 1 and 2.
Applied in Cmgui_image_write.
Clears 'valid' flag of cmgui_image_information if not correctly set.
==============================================================================*/
{
	int return_code;

	ENTER(Cmgui_image_information_set_number_of_bytes_per_component);
	if (cmgui_image_information)
	{
		if ((1 == number_of_bytes_per_component) ||
			(2 == number_of_bytes_per_component))
		{
			cmgui_image_information->number_of_bytes_per_component =
				number_of_bytes_per_component;
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmgui_image_information_set_number_of_bytes_per_component.  "
				"Number of bytes per component must be 1 or 2");
			cmgui_image_information->valid = 0;
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmgui_image_information_set_number_of_bytes_per_component.  "
			"Missing information");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmgui_image_information_set_number_of_bytes_per_component */

int Cmgui_image_information_set_number_of_components(
	struct Cmgui_image_information *cmgui_image_information,
	int number_of_components)
/*******************************************************************************
LAST MODIFIED : 11 March 2002

DESCRIPTION :
Sets the <number_of_components> recorded with the <cmgui_image_information>.
Only valid values are 1=Intensity, 2=IntensityAlpha, 3=RGB, 4=RGBA.
Currently ignored.
Clears 'valid' flag of cmgui_image_information if not correctly set.
==============================================================================*/
{
	int return_code;

	ENTER(Cmgui_image_information_set_number_of_components);
	if (cmgui_image_information)
	{
		if ((1 <= number_of_components) && (number_of_components <= 4))
		{
			cmgui_image_information->number_of_components = number_of_components;
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmgui_image_information_set_number_of_components.  "
				"Number of components must be from 1 to 4");
			cmgui_image_information->valid = 0;
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmgui_image_information_set_number_of_components.  Missing information");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmgui_image_information_set_number_of_components */

int Cmgui_image_information_set_raw_image_storage(
	struct Cmgui_image_information *cmgui_image_information,
	enum Raw_image_storage raw_image_storage)
/*******************************************************************************
LAST MODIFIED : 19 February 2002

DESCRIPTION :
Sets the <raw_image_storage> of <cmgui_image>.
==============================================================================*/
{
	int return_code;

	ENTER(Cmgui_image_information_set_raw_image_storage);
	if (cmgui_image_information)
	{
		cmgui_image_information->raw_image_storage = raw_image_storage;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmgui_image_information_set_raw_image_storage.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmgui_image_information_set_raw_image_storage */

int Cmgui_image_information_set_width(
	struct Cmgui_image_information *cmgui_image_information, int width)
/*******************************************************************************
LAST MODIFIED : 26 February 2002

DESCRIPTION :
Sets the <width> recorded with the <cmgui_image_information>.
Used to specify the width for raw file formats read with Cmgui_image_read.
Clears 'valid' flag of cmgui_image_information if not correctly set.
==============================================================================*/
{
	int return_code;

	ENTER(Cmgui_image_information_set_width);
	if (cmgui_image_information)
	{
		if (0 <= width)
		{
			cmgui_image_information->width = width;
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmgui_image_information_set_width.  Negative width");
			cmgui_image_information->valid = 0;
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmgui_image_information_set_width.  Missing information");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmgui_image_information_set_width */

int Cmgui_image_information_set_io_stream_package(
	struct Cmgui_image_information *cmgui_image_information,
	struct IO_stream_package *io_stream_package)
/*******************************************************************************
LAST MODIFIED : 16 September 2004

DESCRIPTION :
Sets the <io_stream_package> recorded with the <cmgui_image_information>.
Used to specify the io_stream_package for raw file formats read with Cmgui_image_read.
Clears 'valid' flag of cmgui_image_information if not correctly set.
==============================================================================*/
{
	int return_code;

	ENTER(Cmgui_image_information_set_io_stream_package);
	if (cmgui_image_information && io_stream_package)
	{
		cmgui_image_information->io_stream_package = io_stream_package;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmgui_image_information_set_io_stream_package.  Missing information");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmgui_image_information_set_io_stream_package */

#if defined (IMAGEMAGICK)
static int get_magick_image_parameters(Image *magick_image, int *width,
	int *height, int *number_of_components, int *number_of_bytes_per_component)
/*******************************************************************************
LAST MODIFIED : 19 February 2002

DESCRIPTION :
Extracts parameters from <magick_image> that matter for a Cmgui_image
==============================================================================*/
{
	int return_code;

	ENTER(get_magick_image_parameters);
	if (magick_image && width && height && number_of_components &&
		number_of_bytes_per_component)
	{
		*width = (int)magick_image->columns;
		*height = (int)magick_image->rows;
		if (magick_image->matte)
		{
			if (magick_image->colorspace == GRAYColorspace)
			{
				/* monochrome intensity with alpha */
				*number_of_components = 2;
			}
			else
			{
				/* RGBA */
				*number_of_components = 4;
			}
		}
		else
		{
			if (magick_image->colorspace == GRAYColorspace)
			{
				/* monochrome intensity only */
				*number_of_components = 1;
			}
			else
			{
				/* RGB */
				*number_of_components = 3;
			}
		}
		if (16 == magick_image->depth)
		{
			*number_of_bytes_per_component = 2;
		}
		else
		{
			*number_of_bytes_per_component = 1;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_magick_image_parameters.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* get_magick_image_parameters */
#endif /* defined (IMAGEMAGICK) */

#if defined (IMAGEMAGICK)
static int get_magick_image_number_of_consistent_images(Image *magick_image)
/*******************************************************************************
LAST MODIFIED : 19 February 2002

DESCRIPTION :
If all images in <magick_image> have the same width, height,
number_of_components and number_of_bytes_per_component, the number_of_images
is returned, otherwise zero.
Only these parameters matter to Cmgui_image for consistency.
==============================================================================*/
{
	Image *temp_magick_image;
	int number_of_images, return_code;
	
	ENTER(get_magick_image_number_of_consistent_images);
	if (magick_image)
	{
		number_of_images = 1;
		return_code = 1;
		temp_magick_image = magick_image;
		while (return_code && (temp_magick_image = temp_magick_image->next))
		{
			if ((temp_magick_image->columns == magick_image->columns) &&
				(temp_magick_image->rows == magick_image->rows) &&
				(temp_magick_image->matte == magick_image->matte) &&
				(temp_magick_image->colorspace == magick_image->colorspace) &&
				(temp_magick_image->depth == magick_image->depth))
			{
				number_of_images++;
			}
			else
			{
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_magick_image_number_of_consistent_images.  Missing magick_image");
		number_of_images = 0;
	}
	LEAVE;

	return (number_of_images);
} /* get_magick_image_number_of_consistent_images */
#endif /* defined (IMAGEMAGICK) */

static struct Cmgui_image *CREATE(Cmgui_image)(void)
/*******************************************************************************
LAST MODIFIED : 11 March 2002

DESCRIPTION :
Private constructor for Cmgui_image type.
Must be called by specific constructor and left with valid entries.
==============================================================================*/
{
	struct Cmgui_image *cmgui_image;

	ENTER(CREATE(Cmgui_image));
	if (ALLOCATE(cmgui_image, struct Cmgui_image, 1))
	{
#if defined (IMAGEMAGICK)
		cmgui_image->magick_image = (Image *)NULL;
#else /* defined (IMAGEMAGICK) */
		cmgui_image->image_arrays = (unsigned char **)NULL;
#endif /* defined (IMAGEMAGICK) */
		cmgui_image->width = 0;
		cmgui_image->height = 0;
		cmgui_image->number_of_components = 0;
		cmgui_image->number_of_bytes_per_component = 0;
		cmgui_image->number_of_images = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Cmgui_image).  Could not allocate structure");
	}
	LEAVE;

	return (cmgui_image);
} /* CREATE(Cmgui_image) */

int DESTROY(Cmgui_image)(struct Cmgui_image **cmgui_image_address)
/*******************************************************************************
LAST MODIFIED : 27 February 2002

DESCRIPTION :
Frees the memory use by the Cmgui_image and sets <*cmgui_image_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct Cmgui_image *cmgui_image;
#if defined (IMAGEMAGICK)
#else /* defined (IMAGEMAGICK) */
	int i;
#endif /* defined (IMAGEMAGICK) */

	ENTER(DESTROY(Cmgui_image));
	if (cmgui_image_address && (cmgui_image = *cmgui_image_address))
	{
#if defined (IMAGEMAGICK)
		if (cmgui_image->magick_image)
		{
			/* destroy linked-list of images */
			DestroyImages(cmgui_image->magick_image);
		}
#else /* defined (IMAGEMAGICK) */
		if (cmgui_image->image_arrays)
		{
			for (i = 0; i < cmgui_image->number_of_images; i++)
			{
				if (cmgui_image->image_arrays[i])
				{
					DEALLOCATE(cmgui_image->image_arrays[i]);
				}
			}
			DEALLOCATE(cmgui_image->image_arrays);
		}
#endif /* defined (IMAGEMAGICK) */
		DEALLOCATE(*cmgui_image_address);
		*cmgui_image_address = NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Cmgui_image).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Cmgui_image) */

int Cmgui_image_append(struct Cmgui_image *cmgui_image,
	struct Cmgui_image **second_cmgui_image_address)
/*******************************************************************************
LAST MODIFIED : 12 March 2002

DESCRIPTION :
Appends the Cmgui_image pointer to by <second_cmgui_image_address> on to the
end of <cmgui_image>. Both images must be of the same size and colour depth.
Whether this function succeeds or fails, <second_cmgui_image> will bedestroyed.
==============================================================================*/
{
	int return_code;
	struct Cmgui_image *second_cmgui_image;
#if defined (IMAGEMAGICK)
	Image *temp_magick_image;
#else /* defined (IMAGEMAGICK) */
	unsigned char **temp_image_arrays;
	int i;
#endif /* defined (IMAGEMAGICK) */

	ENTER(Cmgui_image_append);
	if (cmgui_image && second_cmgui_image_address &&
		(second_cmgui_image = *second_cmgui_image_address))
	{
		if ((cmgui_image->width == second_cmgui_image->width) &&
			(cmgui_image->height == second_cmgui_image->height) &&
			(cmgui_image->number_of_components ==
				second_cmgui_image->number_of_components) &&
			(cmgui_image->number_of_bytes_per_component ==
				second_cmgui_image->number_of_bytes_per_component))
		{
#if defined (IMAGEMAGICK)
			/* add this image to the end of the linked-list of images */
			temp_magick_image = cmgui_image->magick_image;
			while (temp_magick_image->next)
			{
				temp_magick_image = temp_magick_image->next;
			}
			/* set next/previous pointers */
			temp_magick_image->next = second_cmgui_image->magick_image;
			second_cmgui_image->magick_image->previous = temp_magick_image;
			/* remove the image from second_cmgui_image so not destroyed below */
			second_cmgui_image->magick_image = (Image *)NULL;
			/* increase the number of images */
			cmgui_image->number_of_images += second_cmgui_image->number_of_images;
			return_code = 1;
#else /* defined (IMAGEMAGICK) */
			if (REALLOCATE(temp_image_arrays, cmgui_image->image_arrays,
				unsigned char *, cmgui_image->number_of_images +
				second_cmgui_image->number_of_images))
			{
				for (i = 0; i < second_cmgui_image->number_of_images; i++)
				{
					temp_image_arrays[cmgui_image->number_of_images + i] =
						second_cmgui_image->image_arrays[i];
					/* clear pointer so not destroyed below */
					second_cmgui_image->image_arrays[i] = (unsigned char *)NULL;
				}
				cmgui_image->image_arrays = temp_image_arrays;
				cmgui_image->number_of_images += second_cmgui_image->number_of_images;
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmgui_image_append.  Could not enlarge image arrays");
				return_code = 0;
			}
#endif /* defined (IMAGEMAGICK) */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmgui_image_append.  Images are of different size");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmgui_image_append.  Invalid argument(s)");
		return_code = 0;
	}
	if (second_cmgui_image_address)
	{
		DESTROY(Cmgui_image)(second_cmgui_image_address);
	}
	LEAVE;

	return (return_code);
} /* Cmgui_image_append */

struct Cmgui_image *Cmgui_image_constitute(int width, int height,
	int number_of_components, int number_of_bytes_per_component,
	int source_width_bytes, unsigned char *source_pixels)
/*******************************************************************************
LAST MODIFIED : 18 April 2002

DESCRIPTION :
Creates a single Cmgui_image of the specified <width>, <height> with
<number_of_components> where 1=luminance, 2=LuminanceA, 3=RGB, 4=RGBA, and
<number_of_bytes_per_component> which may be 1 or 2.
Data for the image is taken from <source_pixels> which has <source_width_bytes>
of at least <width>*<number_of_components>*<number_of_bytes_per_component>.
The source_pixels are stored in rows from the bottom to top and from left to
right in each row. Pixel colours are interleaved, eg. RGBARGBARGBA...
==============================================================================*/
{
	int i, return_code;
	struct Cmgui_image *cmgui_image;
#if defined (IMAGEMAGICK)
	unsigned char *p, *start_p;
	Image *magick_image;
	int x, y;
  PixelPacket *q;
	unsigned short short_value[4];
#else /* defined (IMAGEMAGICK) */
	unsigned char *destination, *image, *source;
	int width_bytes;
#endif /* defined (IMAGEMAGICK) */

	ENTER(Cmgui_image_constitute);
	cmgui_image = (struct Cmgui_image *)NULL;
	if ((0 < width) && (0 < height) &&
		(1 <= number_of_components) && (number_of_components <= 4) &&
		((1 == number_of_bytes_per_component) ||
			(2 == number_of_bytes_per_component)) &&
		(width*number_of_components*number_of_bytes_per_component <=
			source_width_bytes) && source_pixels)
	{
		if (cmgui_image = CREATE(Cmgui_image)())
		{
			return_code = 1;
#if defined (IMAGEMAGICK)
			/* following code adapted from ImageMagick's ConstituteImage function
				 with does not support monochrome images */
			if (magick_image = AllocateImage((ImageInfo *) NULL))
			{
				magick_image->columns = width;
				magick_image->rows = height;
				switch (number_of_components)
				{
					case 1:
					{
						magick_image->colorspace = GRAYColorspace;
						magick_image->matte = MagickFalse;
					} break;
					case 2:
					{
						magick_image->colorspace = GRAYColorspace;
						magick_image->matte = MagickTrue;
					} break;
					case 3:
					{
						magick_image->colorspace = RGBColorspace;
						magick_image->matte = MagickFalse;
					} break;
					case 4:
					{
						magick_image->colorspace = RGBColorspace;
						magick_image->matte = MagickTrue;
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Cmgui_image_constitute.  Invalid number_of_components");
						return_code = 0;
					} break;
				}
				magick_image->depth = 8*number_of_bytes_per_component;
				/* need to reverse order of rows */
				start_p = source_pixels + height*source_width_bytes;
				for (y = 0; (y < height) && return_code; y++)
				{
					start_p -= source_width_bytes;
					if (q = SetImagePixels(magick_image, 0, y, width, 1))
					{
						p = start_p;
						switch (number_of_bytes_per_component)
						{
							case 1:
							{
								for (x = 0; x < width; x++)
								{
									q->red = Upscale(*p++);
									if (GRAYColorspace == magick_image->colorspace)
									{
										q->blue = q->green = q->red;
									}
									else
									{
										q->green = Upscale(*p++);
										q->blue = Upscale(*p++);
									}
									if (magick_image->matte)
									{
										/* reverse alpha/opacity */
										q->opacity = Upscale(0xFF - (*p++));
									}
									q++;
								}
							} break;
							case 2:
							{
								for (x = 0; x < width; x++)
								{
									for (i = 0; i < number_of_components; i++)
									{
#if (1234==BYTE_ORDER)
										short_value[i] = (((unsigned short)(*(p + 1))) << 8) - (*p);
#else /* (1234==BYTE_ORDER) */
										short_value[i] = (((unsigned short)(*p)) << 8) - (*(p + 1));
#endif /* (1234==BYTE_ORDER) */
										p += 2;
									}
									q->red = short_value[0];
									if (GRAYColorspace == magick_image->colorspace)
									{
										q->blue = q->green = q->red;
									}
									else
									{
										q->green = short_value[1];
										q->blue = short_value[2];
									}
									if (magick_image->matte)
									{
										/* reverse alpha/opacity */
										q->opacity = 0xFFFF - short_value[number_of_components - 1];
									}
									q++;
								}
							} break;
							default:
							{
								display_message(ERROR_MESSAGE, "Cmgui_image_constitute.  "
									"Invalid number_of_bytes_per_component");
								return_code = 0;
							} break;
						}
						if (!SyncImagePixels(magick_image))
						{
							display_message(ERROR_MESSAGE,
								"Cmgui_image_constitute.  Could not sync pixels");
							return_code = 0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Cmgui_image_constitute.  Could not set pixels");
						return_code = 0;
					}
				}
				if (return_code)
				{
					cmgui_image->magick_image = magick_image;
					cmgui_image->width = width;
					cmgui_image->height = height;
					cmgui_image->number_of_components = number_of_components;
					cmgui_image->number_of_bytes_per_component =
						number_of_bytes_per_component;
					cmgui_image->number_of_images = 1;
				}
				else
				{
					DestroyImage(magick_image);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmgui_image_constitute.  Allocate image failed");
				return_code = 0;
			}
#else /* defined (IMAGEMAGICK) */
			if (ALLOCATE(cmgui_image->image_arrays, unsigned char *, 1))
			{
				cmgui_image->image_arrays[0] = (unsigned char *)NULL;
				/* make image in our unsigned char format */
				width_bytes = width*number_of_components*number_of_bytes_per_component;
				if (ALLOCATE(image, unsigned char, height*width_bytes))
				{
					/* fill the image from bottom to top */
					source = source_pixels;
					destination = image;
					for (i = 0; i < height; i++)
					{
						memcpy(destination, source, width_bytes);
						source += source_width_bytes;
						destination += width_bytes;
					}
					cmgui_image->image_arrays[0] = image;
					cmgui_image->width = width;
					cmgui_image->height = height;
					cmgui_image->number_of_components = number_of_components;
					cmgui_image->number_of_bytes_per_component =
						number_of_bytes_per_component;
					cmgui_image->number_of_images = 1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Cmgui_image_constitute.  Could not allocate image");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmgui_image_constitute.  Could not allocate image arrays");
				return_code = 0;
			}
#endif /* defined (IMAGEMAGICK) */
			if (!return_code)
			{
				DESTROY(Cmgui_image)(&cmgui_image);
				cmgui_image = (struct Cmgui_image *)NULL;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmgui_image_constitute.  Invalid argument(s)");
	}
	LEAVE;

	return (cmgui_image);
} /* Cmgui_image_constitute */

int Cmgui_image_dispatch(struct Cmgui_image *cmgui_image,
	int image_number, int left, int bottom, int width, int height,
	int padded_width_bytes, int number_of_fill_bytes, unsigned char *fill_bytes,
	unsigned char *destination_pixels)
/*******************************************************************************
LAST MODIFIED : 12 March 2002

DESCRIPTION :
Fills <destination_pixels> with all or part of image <image_number> of
<cmgui_image>, where 0 is the first image.
The <left>, <bottom>, <width> and <height> specify the part of <cmgui_image>
output and must be wholly within its bounds.
Image data is ordered from the bottom row to the top, and within each row from
the left to the right.
All number_of_components components of the image are output at each pixel, and
pixel values relate to number_of_components by:
  1 -> I    = Intensity;
  2 -> IA   = Intensity Alpha;
  3 -> RGB  = Red Green Blue;
  4 -> RGBA = Red Green Blue Alpha;

If <padded_width_bytes> is zero, image data for subsequent rows follows exactly
after the right-most pixel of the row below. If a positive number is specified,
which must be greater than <width>*number_of_components*
number_of_bytes_per_component in <cmgui_image>, each
row of the output image will take up the specified number of bytes, with
pixels beyond the extracted image <width> undefined.
If <number_of_fill_bytes> is positive, the <fill_bytes> are repeatedly output
to fill the padded row; the cycle of outputting <fill_bytes> starts at the
left of the image to make a more consitent output if more than one colour is
specified in them -- it makes no difference if <number_of_fill_bytes> is 1 or
equal to the number_of_components.
<destination_pixels> must be large enough to take the greater of
<padded_width_bytes> or
<width>*number_of_components*number_of_bytes_per_component in the image.

???RC May wish to expand capabilities of this function in future to handle:
- choosing a different output_number_of_bytes_per_component
- different colour spaces output, currently fixed for the number_of_components;
==============================================================================*/
{
	int bytes_per_pixel, fill_byte_number, i, padding_bytes, return_code,
		width_bytes, y, y_limit;
	unsigned char *destination;
#if defined (IMAGEMAGICK)
	char *magick_image_storage;
	ExceptionInfo magick_exception;
	Image *magick_image;
	int image_height_minus_1, reverse_alpha;
	StorageType magick_storage_type;
	unsigned char *short_source, *temp_dest;
	unsigned short short_value;
#else /* defined (IMAGEMAGICK) */
	unsigned char *source;
	int source_width_bytes;
#endif /* defined (IMAGEMAGICK) */

	ENTER(Cmgui_image_dispatch);
	if (cmgui_image &&
		(0 <= image_number) && (image_number < cmgui_image->number_of_images) &&
		(0 <= left) && (0 < width) && (left + width <= cmgui_image->width) &&
		(0 <= bottom) && (0 < height) && (bottom + height <= cmgui_image->height) &&
		(bytes_per_pixel = cmgui_image->number_of_components *
			cmgui_image->number_of_bytes_per_component) &&
		(width_bytes = width * bytes_per_pixel) &&
		((0 == padded_width_bytes) ||
			((padded_width_bytes >= width_bytes) &&
				((0 == number_of_fill_bytes) ||
					((0 < number_of_fill_bytes) && fill_bytes)))) &&
		destination_pixels)
	{
		return_code = 1;
		destination = destination_pixels;
		if (0 < padded_width_bytes)
		{
			padding_bytes = padded_width_bytes - width_bytes;
		}
		else
		{
			padding_bytes = 0;
		}
#if defined (IMAGEMAGICK)
		GetExceptionInfo(&magick_exception);
		/* get the magick_image for this <image_number> */
		magick_image = cmgui_image->magick_image;
		for (i = 0; (i < image_number) && magick_image; i++)
		{
			magick_image = magick_image->next;
		}
		if (!magick_image)
		{
			display_message(ERROR_MESSAGE,
				"Cmgui_image_dispatch.  No image at image number %d",image_number);
			return_code = 0;
		}
		switch (cmgui_image->number_of_components)
		{
			case 1:
			{
				magick_image_storage = "R";
				reverse_alpha = 0;
			} break;
			case 2:
			{
				magick_image_storage = "RA";
				reverse_alpha = 0;
			} break;
			case 3:
			{
				magick_image_storage = "RGB";
				reverse_alpha = 0;
			} break;
			case 4:
			{
				magick_image_storage = "RGBA";
				reverse_alpha = 0;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmgui_image_dispatch.  Invalid number_of_components");
				return_code = 0;
			} break;
		}
		switch (cmgui_image->number_of_bytes_per_component)
		{
			case 1:
			{
				magick_storage_type = CharPixel;
			} break;
			case 2:
			{
				magick_storage_type = ShortPixel;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmgui_image_dispatch.  Invalid number_of_bytes_per_component");
				return_code = 0;
			} break;
		}
		image_height_minus_1 = cmgui_image->height - 1;
#else /* defined (IMAGEMAGICK) */
		source_width_bytes = cmgui_image->width*bytes_per_pixel;
		source = cmgui_image->image_arrays[image_number] +
			bottom*source_width_bytes + left*bytes_per_pixel;
#endif /* defined (IMAGEMAGICK) */
		if (return_code)
		{
			y_limit = bottom + height;
			for (y = bottom; y < y_limit; y++)
			{
#if defined (IMAGEMAGICK)
				/* y is 0 in the top scanline of ImageMagick images, hence reverse */
				DispatchImage(magick_image, left, image_height_minus_1 - y,
					width, 1, magick_image_storage, magick_storage_type,
					(void *)destination, &magick_exception);
				if (reverse_alpha)
				{
					temp_dest = destination +
						bytes_per_pixel - cmgui_image->number_of_bytes_per_component;
					short_source = (unsigned char *)(&short_value);
					for (i = 0; i < width; i++)
					{
						if (2 == cmgui_image->number_of_bytes_per_component)
						{
#if (1234==BYTE_ORDER)
							short_value = 0xFFFF -
								(((unsigned short)(*(temp_dest + 1))) << 8) - (*temp_dest);
#else /* (1234==BYTE_ORDER) */
							short_value = 0xFFFF -
								(((unsigned short)(*temp_dest)) << 8) - (*(temp_dest + 1));
#endif /* (1234==BYTE_ORDER) */
							*temp_dest = *short_source;
							*(temp_dest + 1) = *(short_source + 1);
						}
						else
						{
							*temp_dest = 0xFF - (*temp_dest);
						}
						temp_dest += bytes_per_pixel;
					}
				}
#else /* defined (IMAGEMAGICK) */
				memcpy(destination, source, width_bytes);
				source += source_width_bytes;
#endif /* defined (IMAGEMAGICK) */
				destination += width_bytes;
				if (0 < padding_bytes)
				{
					if (1 == number_of_fill_bytes)
					{
						memset(destination, *fill_bytes, padding_bytes);
						destination += padding_bytes;
					}
					else if (1 < number_of_fill_bytes)
					{
						if (y == bottom)
						{
							fill_byte_number = width_bytes % number_of_fill_bytes;
							for (i = 0; i < padding_bytes; i++)
							{
								*destination = fill_bytes[fill_byte_number];
								fill_byte_number++;
								if (fill_byte_number >= number_of_fill_bytes)
								{
									fill_byte_number = 0;
								}
								destination++;
							}
						}
						else
						{
							/* copy the pattern made on the first row */
							memcpy(destination, destination - padded_width_bytes,
								padding_bytes);
							destination += padding_bytes;
						}
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmgui_image_dispatch.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmgui_image_dispatch */

int Cmgui_image_get_height(struct Cmgui_image *cmgui_image)
/*******************************************************************************
LAST MODIFIED : 27 February 2002

DESCRIPTION :
Returns the <height> of <cmgui_image>.
==============================================================================*/
{
	int height;

	ENTER(Cmgui_image_get_height);
	if (cmgui_image)
	{
		height = cmgui_image->height;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmgui_image_get_height.  Invalid argument(s)");
		height = 0;
	}
	LEAVE;

	return (height);
} /* Cmgui_image_get_height */

int Cmgui_image_get_number_of_bytes_per_component(
	struct Cmgui_image *cmgui_image)
/*******************************************************************************
LAST MODIFIED : 20 February 2002

DESCRIPTION :
Returns the <number_of_bytes_per_component> of <cmgui_image>, currently either
1 or 2.
==============================================================================*/
{
	int number_of_bytes_per_component;

	ENTER(Cmgui_image_get_number_of_bytes_per_component);
	if (cmgui_image)
	{
		number_of_bytes_per_component = cmgui_image->number_of_bytes_per_component;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmgui_image_get_number_of_bytes_per_component.  Invalid argument(s)");
		number_of_bytes_per_component = 0;
	}
	LEAVE;

	return (number_of_bytes_per_component);
} /* Cmgui_image_get_number_of_bytes_per_component */

int Cmgui_image_get_number_of_components(struct Cmgui_image *cmgui_image)
/*******************************************************************************
LAST MODIFIED : 16 February 2002

DESCRIPTION :
Returns the <number_of_components> - R, G, B, A, I etc. of <cmgui_image>.
==============================================================================*/
{
	int number_of_components;

	ENTER(Cmgui_image_get_number_of_components);
	if (cmgui_image)
	{
		number_of_components = cmgui_image->number_of_components;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmgui_image_get_number_of_components.  Invalid argument(s)");
		number_of_components = 0;
	}
	LEAVE;

	return (number_of_components);
} /* Cmgui_image_get_number_of_components */

int Cmgui_image_get_number_of_images(struct Cmgui_image *cmgui_image)
/*******************************************************************************
LAST MODIFIED : 19 February 2002

DESCRIPTION :
Returns the <number_of_images> stored <cmgui_image>.
==============================================================================*/
{
	int number_of_images;

	ENTER(Cmgui_image_get_number_of_images);
	if (cmgui_image)
	{
		number_of_images = cmgui_image->number_of_images;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmgui_image_get_number_of_images.  Invalid argument(s)");
		number_of_images = 0;
	}
	LEAVE;

	return (number_of_images);
} /* Cmgui_image_get_number_of_images */

int Cmgui_image_get_width(struct Cmgui_image *cmgui_image)
/*******************************************************************************
LAST MODIFIED : 27 February 2002

DESCRIPTION :
Returns the <width> <cmgui_image>.
==============================================================================*/
{
	int width;

	ENTER(Cmgui_image_get_width);
	if (cmgui_image)
	{
		width = cmgui_image->width;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmgui_image_get_width.  Invalid argument(s)");
		width = 0;
	}
	LEAVE;

	return (width);
} /* Cmgui_image_get_width */

struct Cmgui_image *Cmgui_image_read(
	struct Cmgui_image_information *cmgui_image_information)
/*******************************************************************************
LAST MODIFIED : 12 March 2002

DESCRIPTION :
Creates a Cmgui_image containing the images from the files listed in the
<cmgui_image_information>. If more than one file_name is listed, they are
checked to be of the same size and assembled together; note that the images
making up the series may not themselves have a third dimension, such as animated
GIF or dicom files, unless only a single filename is given.
The <cmgui_image_information> should be given appropriate width, height
and other parameters for formats that require them.
==============================================================================*/
{
	char *file_name;
	int height, i, return_code, width;
	struct Cmgui_image *cmgui_image;
#if defined (IMAGEMAGICK)
	char *file_name_prefix, *old_magick_size, magick_size[41];
	int image_data_length, length;
	Image *magick_image, *temp_magick_image;
	ImageInfo *magick_image_info;
	ExceptionInfo magick_exception;
	long int file_size;
	struct stat buf;
	struct IO_stream *image_file;
	void *image_data;
#else /* defined (IMAGEMAGICK) */
	enum Image_file_format image_file_format;
	int number_of_bytes_per_component, number_of_components;
#endif /* defined (IMAGEMAGICK) */

	ENTER(Cmgui_image_read);
	cmgui_image = (struct Cmgui_image *)NULL;
	if (cmgui_image_information && cmgui_image_information->valid &&
		cmgui_image_information->file_names)
	{
		if (cmgui_image = CREATE(Cmgui_image)())
		{
			return_code = 1;
#if defined (IMAGEMAGICK)
			GetExceptionInfo(&magick_exception);
			if (magick_image_info = CloneImageInfo((ImageInfo *) NULL))
			{
				old_magick_size = magick_image_info->size;
				magick_image_info->size = (char *)NULL;
#if defined (DEBUG)
				magick_image_info->verbose = 1;
#endif /* defined (DEBUG) */
				for (i = 0; (i < cmgui_image_information->number_of_file_names) &&
					return_code; i++)
				{
					width = cmgui_image_information->width;
					height = cmgui_image_information->height;
					file_name = cmgui_image_information->file_names[i];
					length = strlen(file_name);
					file_name_prefix = "";
					if (!strchr(file_name, ':'))
					{
						/* Only add prefixes if there isn't one already and we want to
							 do something tricky */
						/* test to see if a file with suffix rgb is an sgi rgb file */
						if ((length > 4) &&  fuzzy_string_compare_same_length(
							(file_name + length - 4), ".rgb"))
						{
							file_name_prefix = "sgi:";
						}
						/* if width and height are not specified and the file is of yuv or
							 uyuv type, try to get standard video resolution from file size */
						else if ((0 != width) && (0 != height) && (
							((length > 4) && fuzzy_string_compare_same_length(
								(file_name + length - 4), ".yuv")) ||
							((length > 5) && fuzzy_string_compare_same_length(
								(file_name + length - 5), ".uyvy"))))
						{
							file_name_prefix = "uyvy:";
							/* try to get width and height from the file_size */
							if ((0 == stat(file_name, &buf)) &&
								(0 < (file_size = (long int)(buf.st_size))))
							{
								get_yuv_resolution_from_file_size(file_size, &width, &height);
							}
						}
					}
					sprintf(magick_image_info->filename, "%s%s",
						file_name_prefix, file_name);
					if ((0 < width) && (0 < height))
					{
						sprintf(magick_size, "%dx%d", width, height);
						magick_image_info->size = magick_size;
					}
					if ((1 == cmgui_image_information->number_of_components) || 
						(2 == cmgui_image_information->number_of_components))
					{
						magick_image_info->colorspace = GRAYColorspace;
					}
					if (2 == cmgui_image_information->number_of_bytes_per_component)
					{
						magick_image_info->depth = 16;
					}
					else
					{
						magick_image_info->depth = 8;
					}
					/* set interlace type for raw RGB files */
					switch (cmgui_image_information->raw_image_storage)
					{
						case RAW_INTERLEAVED_RGB:
						{
							magick_image_info->interlace = NoInterlace;
						} break;
						case RAW_PLANAR_RGB:
						{
							magick_image_info->interlace = PlaneInterlace;
						} break;
					}
					if (IO_stream_uri_is_native_imagemagick(magick_image_info->filename))
					{
						magick_image = ReadImage(magick_image_info, &magick_exception);
					}
					else
					{
						if (image_file = CREATE(IO_stream)(cmgui_image_information->io_stream_package))
						{
							if (IO_stream_open_for_read(image_file, magick_image_info->filename))
							{
								if (IO_stream_read_to_memory(image_file, &image_data, &image_data_length))
								{
									SetImageInfo(magick_image_info, MagickFalse, &magick_exception);
									magick_image = BlobToImage(magick_image_info,
										image_data, image_data_length, &magick_exception);
									IO_stream_deallocate_read_to_memory(image_file);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Cmgui_image_read.  Unable to read image uri into memory buffer.");
									return_code = 0;
								}
								IO_stream_close(image_file);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Cmgui_image_read.  Unable to open image uri for reading.");
								return_code = 0;
							}
							DESTROY(IO_stream)(&image_file);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Cmgui_image_read.  Unable to create IO_stream.");
							return_code = 0;
						}
					}
					if (magick_image)
					{
						if (cmgui_image->magick_image)
						{
							/* add image to the end of the doubly linked-list of images */
							temp_magick_image = cmgui_image->magick_image;
							while (temp_magick_image->next)
							{
								temp_magick_image = temp_magick_image->next;
							}
							temp_magick_image->next = magick_image;
							magick_image->previous = temp_magick_image;
						}
						else
						{
							cmgui_image->magick_image = magick_image;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Could not read image: %s", file_name);
						return_code = 0;
					}
					magick_image_info->size = (char *)NULL;
				}
				/* restore original size for ImageMagick to clean up */
				magick_image_info->size = old_magick_size;
				if (return_code && cmgui_image->magick_image)
				{
					if (cmgui_image->number_of_images =
						get_magick_image_number_of_consistent_images(
							cmgui_image->magick_image))
					{
						get_magick_image_parameters(cmgui_image->magick_image,
							&cmgui_image->width, &cmgui_image->height,
							&cmgui_image->number_of_components,
							&cmgui_image->number_of_bytes_per_component);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Cmgui_image_read.  Image size not constant over series");
						return_code = 0;
					}
				}
				DestroyImageInfo(magick_image_info);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmgui_image_read.  Could not create image information");
				return_code = 0;
			}
#else /* defined (IMAGEMAGICK) */
			if (ALLOCATE(cmgui_image->image_arrays, unsigned char *,
				cmgui_image_information->number_of_file_names))
			{
				/* clear image_arrays so destroy function can clean up if this fails */
				for (i = 0; (i < cmgui_image_information->number_of_file_names); i++)
				{
					cmgui_image->image_arrays[i] = (unsigned char *)NULL;
				}
				for (i = 0; (i < cmgui_image_information->number_of_file_names) &&
					return_code; i++)
				{
					file_name = cmgui_image_information->file_names[i];
					width = cmgui_image_information->width;
					height = cmgui_image_information->height;
					if (Image_file_format_from_file_name(file_name, &image_file_format))
					{
						switch (image_file_format)
						{
							case POSTSCRIPT_FILE_FORMAT:
							{
								display_message(ERROR_MESSAGE,
									"Cannot read postscript image file '%s'", file_name);
								return_code = 0;
							} break;
							case RAW_FILE_FORMAT:
							{
								return_code = read_raw_image_file(file_name,
									cmgui_image_information->raw_image_storage,
									&width, &height, &number_of_components,
									&number_of_bytes_per_component,
									&(cmgui_image->image_arrays[i]));
							} break;
							case RGB_FILE_FORMAT:
							{
								return_code = read_rgb_image_file(file_name,
									&width, &height, &number_of_components,
									&number_of_bytes_per_component,
									&(cmgui_image->image_arrays[i]));
							} break;
							case TIFF_FILE_FORMAT:
							{
								return_code = read_tiff_image_file(file_name,
									&width, &height, &number_of_components,
									&number_of_bytes_per_component,
									&(cmgui_image->image_arrays[i]));
							} break;
							case YUV_FILE_FORMAT:
							{
								return_code = read_yuv_image_file(file_name,
									&width, &height, &number_of_components,
									&number_of_bytes_per_component,
									&(cmgui_image->image_arrays[i]));
							} break;
							default:
							{
								display_message(ERROR_MESSAGE, "Cmgui_image_read.  "
									"This version of CMGUI cannot read images of .%s format",
									Image_file_format_extension(image_file_format));
								return_code = 0;
							} break;
						}
						if (return_code)
						{
							if (0 == i)
							{
								/* save size/format of first image */
								cmgui_image->width = width;
								cmgui_image->height = height;
								cmgui_image->number_of_components = number_of_components;
								cmgui_image->number_of_bytes_per_component =
									number_of_bytes_per_component;
								cmgui_image->number_of_images =
									cmgui_image_information->number_of_file_names;
							}
							else
							{
								/* make sure following images are the same size/format */
								if ((width != cmgui_image->width) ||
									(height != cmgui_image->height) ||
									(number_of_components != cmgui_image->number_of_components) ||
									(number_of_bytes_per_component !=
										cmgui_image->number_of_bytes_per_component))
								{
									display_message(ERROR_MESSAGE,
										"Cmgui_image_read.  Image size not constant over series");
									return_code = 0;
								}
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE, "Cmgui_image_read.  "
							"Could not determine image format of file '%s'", file_name);
						return_code = 0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmgui_image_read.  Could not allocate image arrays");
				return_code = 0;
			}
#endif /* defined (IMAGEMAGICK) */
			if (!return_code)
			{
				DESTROY(Cmgui_image)(&cmgui_image);
				cmgui_image = (struct Cmgui_image *)NULL;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmgui_image_read.  Invalid image information");
	}
	LEAVE;

	return (cmgui_image);
} /* Cmgui_image_read */

int Cmgui_image_write(struct Cmgui_image *cmgui_image,
	struct Cmgui_image_information *cmgui_image_information)
/*******************************************************************************
LAST MODIFIED : 23 March 2002

DESCRIPTION :
Writes <cmgui_image> to the filename or filenames listed in the
<cmgui_image_information>. If set, the image_file_format in the information
overrides the file format determined from the extension.
There must be as many filenames supplied as images in <cmgui_image> with the
exception of certain formats which can support all images being written to a
single file, eg. animated GIF, DICOM, in which case a single filename requests
that the images be adjoined in the single file.
==============================================================================*/
{
	char *file_name;
	int i, number_of_file_names, return_code;
#if defined (IMAGEMAGICK)
	Image *magick_image, *temp_next, *temp_previous;
	ImageInfo *magick_image_info;
#else /* defined (IMAGEMAGICK) */
	enum Image_file_format image_file_format;
#endif /* defined (IMAGEMAGICK) */

	ENTER(Cmgui_image_write);
	if (cmgui_image && cmgui_image_information &&
		cmgui_image_information->valid && cmgui_image_information->file_names &&
		((0 < (number_of_file_names =
			cmgui_image_information->number_of_file_names))) &&
		((1 == number_of_file_names) ||
			(number_of_file_names == cmgui_image->number_of_images)))
	{
		return_code = 1;
#if defined (IMAGEMAGICK)
		if (magick_image_info = CloneImageInfo((ImageInfo *) NULL))
		{
			magick_image = cmgui_image->magick_image;
			for (i = 0; (i < number_of_file_names) && return_code; i++)
			{
				file_name = cmgui_image_information->file_names[i];
				if ((!strchr(file_name, ':')) && (UNKNOWN_IMAGE_FILE_FORMAT !=
					cmgui_image_information->image_file_format))
				{
					if (RGB_FILE_FORMAT == cmgui_image_information->image_file_format)
					{
						/* rgb is used in cmgui to denote the sgi rgb format */
						sprintf(magick_image->filename, "sgi:%s", file_name);
					}
					else
					{
						sprintf(magick_image->filename, "%s:%s",
							Image_file_format_extension(
								cmgui_image_information->image_file_format), file_name);
					}
				}
				else
				{
					strcpy(magick_image->filename, file_name);
				}
				/* image magick seems to use 16-bit components internally so assume
					 we have to force output to be 8 bits if only 8-bits used */
				if (2 == cmgui_image_information->number_of_bytes_per_component)
				{
					magick_image_info->depth = 16;
				}
				else
				{
					magick_image_info->depth = 8;
				}
				/* set interlace type for raw RGB files */
				switch (cmgui_image_information->raw_image_storage)
				{
					case RAW_INTERLEAVED_RGB:
					{
						magick_image_info->interlace = NoInterlace;
					} break;
					case RAW_PLANAR_RGB:
					{
						magick_image_info->interlace = PlaneInterlace;
					} break;
				}
				if (3 <= cmgui_image->number_of_components)
				{
					/* image magick will reduce RGB to monochrome if all R, G and B
						 values are the same. The resulting images cannot be re-read!
						 If the image info type is set to TrueColorType, this is avoided */
					magick_image_info->type = TrueColorType;
				}
#if defined (DEBUG)
				magick_image_info->verbose = 1;
#endif /* defined (DEBUG) */
				if (number_of_file_names < cmgui_image->number_of_images)
				{
					/* ask ImageMagick to adjoin the images into one */
					magick_image_info->adjoin = MagickTrue;
					temp_previous = (Image *)NULL;
					temp_next = (Image *)NULL;
				}
				else
				{
					/* stop ImageMagick from adjoining images */
					magick_image_info->adjoin = MagickFalse;
					temp_previous = magick_image->previous;
					magick_image->previous = (Image *)NULL;
					temp_next = magick_image->next;
					magick_image->next = (Image *)NULL;
				}
				if (!WriteImage(magick_image_info, magick_image))
				{
					display_message(ERROR_MESSAGE,
						"Could not write image \"%s\"", file_name);
					return_code = 0;
				}
				if (temp_previous || temp_next)
				{
					/* restore place of image in linked list */
					magick_image->previous = temp_previous;
					magick_image->next = temp_next;
				}
				magick_image = magick_image->next;
			}
			DestroyImageInfo(magick_image_info);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmgui_image_write.  Could not create image information");
			return_code = 0;
		}
#else /* defined (IMAGEMAGICK) */
		/* must have same number of filenames as images */
		if (number_of_file_names == cmgui_image->number_of_images)
		{
			for (i = 0; (i < number_of_file_names) && return_code; i++)
			{
				file_name = cmgui_image_information->file_names[i];
				image_file_format = cmgui_image_information->image_file_format;
				if ((UNKNOWN_IMAGE_FILE_FORMAT != image_file_format) ||
					Image_file_format_from_file_name(file_name, &image_file_format))
				{
					switch (image_file_format)
					{
						case RGB_FILE_FORMAT:
						{
							return_code = write_rgb_image_file(file_name,
								cmgui_image->number_of_components,
								cmgui_image->number_of_bytes_per_component,
								cmgui_image->width, 
								cmgui_image->height,
								/*row_padding*/0,
								(unsigned long *)cmgui_image->image_arrays[i]);
						} break;
						case TIFF_FILE_FORMAT:
						{
							return_code = write_tiff_image_file(file_name, 
								cmgui_image->number_of_components,
								cmgui_image->number_of_bytes_per_component,
								cmgui_image->width, 
								cmgui_image->height,
								/*row_padding*/0,
								TIFF_PACK_BITS_COMPRESSION,
								(unsigned long *)cmgui_image->image_arrays[i]);
						} break;
						case POSTSCRIPT_FILE_FORMAT:
						{
							return_code = write_postscript_image_file(file_name,
								cmgui_image->number_of_components,
								cmgui_image->number_of_bytes_per_component,
								cmgui_image->width, 
								cmgui_image->height,
								/*row_padding*/0,
								/*pixel_aspect_ratio*/1.0,
								PORTRAIT_ORIENTATION,
								(unsigned long *)cmgui_image->image_arrays[i]);
						} break;
						default:
						{
							display_message(ERROR_MESSAGE, "Cmgui_image_write.  "
								"This version of CMGUI cannot write images of .%s format",
								Image_file_format_extension(image_file_format));
							return_code = 0;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "Cmgui_image_write.  "
						"Image file format of %s must be specified", file_name);
					return_code = 0;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Cmgui_image_write.  "
				"%d filename(s) supplied for %d image(s)",
				number_of_file_names, cmgui_image->number_of_images);
			return_code = 0;
		}
#endif /* defined (IMAGEMAGICK) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmgui_image_write.  Invalid image information");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmgui_image_write */
