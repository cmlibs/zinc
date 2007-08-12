/*******************************************************************************
FILE : image_utilities.h

LAST MODIFIED : 11 March 2002

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
#if !defined (IMAGE_UTILITIES_H)
#define IMAGE_UTILITIES_H
#include "general/enumerator.h"
#include "general/object.h"

/*
Global types
------------
*/
enum Cmgui_image_format
/*******************************************************************************
LAST MODIFIED : 1 November 2006

DESCRIPTION :
Enumerator for specifying the image format.
Note: the first value will be 0 by the ANSI standard, with each subsequent entry
incremented by 1. This pattern is expected by the ENUMERATOR macros.
Must ensure the ENUMERATOR_STRING function returns a string for each value here.
==============================================================================*/
{
  CMGUI_IMAGE_I,
  CMGUI_IMAGE_IA,
  CMGUI_IMAGE_RGB,
  CMGUI_IMAGE_RGBA
}; /* enum Cmgui_image_format */

enum Image_file_format
/*******************************************************************************
LAST MODIFIED : 1 March 2002

DESCRIPTION :
Enumerator for specifying the image file format.
Note: the first value will be 0 by the ANSI standard, with each subsequent entry
incremented by 1. This pattern is expected by the ENUMERATOR macros.
Must ensure the ENUMERATOR_STRING function returns a string for each value here.
==============================================================================*/
{
	BMP_FILE_FORMAT,
	JPG_FILE_FORMAT,
	GIF_FILE_FORMAT,
	POSTSCRIPT_FILE_FORMAT,
	RAW_FILE_FORMAT,  /* otherwise known as the true RGB format */
	RGB_FILE_FORMAT,  /* denotes the SGI RGB format */
	TIFF_FILE_FORMAT,
	YUV_FILE_FORMAT,
	/* following used to indicate that the format has not been specified */
	UNKNOWN_IMAGE_FILE_FORMAT
}; /* enum Image_file_format */

enum Tiff_image_compression
{
	TIFF_NO_COMPRESSION,
	TIFF_LZW_COMPRESSION,
	TIFF_PACK_BITS_COMPRESSION
}; /* enum Tiff_image_compression */

enum Raw_image_storage
{
	RAW_INTERLEAVED_RGB,  /* RGBRGBRGBRGBRGBRGBRGBRGB */
	RAW_PLANAR_RGB        /* RRRRRR...GGGGGG...BBBBBB */
}; /* enum Raw_image_storage */

struct Cmgui_image_information;
/*******************************************************************************
LAST MODIFIED : 18 February 2002

DESCRIPTION :
Private structure for describing a Cmgui_image to read in.
Lists one or more file names to be read in, with hints for width, height and
interlace_type for raw rgb/yuv image formats.
If more than one file_name is included, they must have consistent width, height
and other attributes.
Must be created and passed to Cmgui_image_read.
==============================================================================*/

struct Cmgui_image;
/*******************************************************************************
LAST MODIFIED : 13 February 2002

DESCRIPTION :
Private structure for storing 2-D images.
==============================================================================*/

/*
Global functions
----------------
*/

int Open_image_environment(char *program_name);
/*******************************************************************************
LAST MODIFIED : 1 March 2002

DESCRIPTION :
Sets up the image library. Must be called before using any image I/O
functions, ie. at the start of the program.
==============================================================================*/

int Close_image_environment(void);
/*******************************************************************************
LAST MODIFIED : 31 March 2006

DESCRIPTION :
Called to finialise the use of the image environment
==============================================================================*/

PROTOTYPE_ENUMERATOR_FUNCTIONS(Image_file_format);

char *Image_file_format_extension(enum Image_file_format image_file_format);
/*******************************************************************************
LAST MODIFIED : 8 September 2000

DESCRIPTION :
Returns the expected file extension stem for a given <image_file_format>. By
stem it is meant that the given characters follow the final . in the file name,
but extra characters may follow. This is especially true for .tif/.tiff and
.yuv#### extensions.
==============================================================================*/

int Image_file_format_from_file_name(char *file_name,
	enum Image_file_format *image_file_format_address);
/*******************************************************************************
LAST MODIFIED : 8 September 2000

DESCRIPTION :
Returns the <Image_file_format> determined from the file_extension in
<file_name>, or UNKNOWN_IMAGE_FILE_FORMAT if none found or no match made.
==============================================================================*/

PROTOTYPE_ENUMERATOR_FUNCTIONS(Raw_image_storage);

int get_radial_distortion_corrected_coordinates(double dist_x,double dist_y,
	double dist_centre_x,double dist_centre_y,double dist_factor_k1,
	double *corr_x,double *corr_y);
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

int get_radial_distortion_distorted_coordinates(double corr_x,double corr_y,
	double dist_centre_x,double dist_centre_y,double dist_factor_k1,
	double tolerance,double *dist_x,double *dist_y);
/*******************************************************************************
LAST MODIFIED : 30 April 1998

DESCRIPTION :
Returns the position point <corr_x,corr_y> would be at in the radial distorted
lens system. Inverse of get_radial_distortion_corrected_coordinates.
Iterative routine requires a tolerance to be set on how little the distorted
radius shifts in an iteration to be an acceptable solution.
Allows up to around 10% distortion to be corrected. (This is a large value!)
==============================================================================*/

struct Cmgui_image_information *CREATE(Cmgui_image_information)(void);
/*******************************************************************************
LAST MODIFIED : 26 February 2002

DESCRIPTION :
Creates a blank Cmgui_image_information.
To read in an image must set at least one file name in this structure; for raw
or yuv image formats, width, height and raw_image_storage may need to be set.
To create an image need to specify most dimension arguments.
==============================================================================*/

int DESTROY(Cmgui_image_information)(
	struct Cmgui_image_information **cmgui_image_information_address);
/*******************************************************************************
LAST MODIFIED : 18 February 2002

DESCRIPTION :
Frees the memory use by the Cmgui_image_information and sets
<*cmgui_image_information_address> to NULL.
==============================================================================*/

int Cmgui_image_information_add_file_name(
	struct Cmgui_image_information *cmgui_image_information, char *file_name);
/*******************************************************************************
LAST MODIFIED : 27 February 2002

DESCRIPTION :
Adds the <file_name> to the end of the list in <cmgui_image_information>.
Clears 'valid' flag if fails.
==============================================================================*/

int Cmgui_image_information_set_file_name_series(
	struct Cmgui_image_information *cmgui_image_information,
	char *file_name_template, char *file_number_pattern, int start_file_number,
	int stop_file_number, int file_number_increment);
/*******************************************************************************
LAST MODIFIED : 18 February 2002

DESCRIPTION :
Adds a series of file names based on the <file_name_template> to the
<cmgui_image_information>. The numbers from <start_file_number> to
<stop_file_number> with <file_number_increment> are substituted for the first
instance of <file_number_pattern> in <file_name_template>.
The number appears with leading zeros up to the length of <file_number_pattern>.
Clears 'valid' flag if fails.
==============================================================================*/

int Cmgui_image_information_set_file_name(
	struct Cmgui_image_information *cmgui_image_information,
	int file_name_number, char *file_name);
/*******************************************************************************
LAST MODIFIED : 18 February 2002

DESCRIPTION :
Sets the <file_name> for <file_name_number> of <cmgui_image_information>.
Clears 'valid' flag if fails.
==============================================================================*/

int Cmgui_image_information_set_height(
	struct Cmgui_image_information *cmgui_image_information, int height);
/*******************************************************************************
LAST MODIFIED : 26 February 2002

DESCRIPTION :
Sets the <height> recorded with the <cmgui_image_information>.
Used to specify the height for raw file formats read with Cmgui_image_read.
Clears 'valid' flag of cmgui_image_information if not correctly set.
==============================================================================*/

int Cmgui_image_information_set_image_file_format(
	struct Cmgui_image_information *cmgui_image_information,
	enum Image_file_format image_file_format);
/*******************************************************************************
LAST MODIFIED : 1 March 2002

DESCRIPTION :
Sets the <image_file_format> of <cmgui_image>.
If set to UNKNOWN_IMAGE_FILE_FORMAT then the format is determined from the
filename, otherwise it is forced from the format listed here.
==============================================================================*/

int Cmgui_image_information_set_number_of_bytes_per_component(
	struct Cmgui_image_information *cmgui_image_information,
	int number_of_bytes_per_component);
/*******************************************************************************
LAST MODIFIED : 26 February 2002

DESCRIPTION :
Sets the <number_of_bytes_per_component> recorded with the
<cmgui_image_information>. Only valid values are 1 and 2.
Applied in Cmgui_image_write. Ignored by Cmgui_image_read.
Clears 'valid' flag of cmgui_image_information if not correctly set.
==============================================================================*/

int Cmgui_image_information_set_number_of_components(
	struct Cmgui_image_information *cmgui_image_information,
	int number_of_components);
/*******************************************************************************
LAST MODIFIED : 26 February 2002

DESCRIPTION :
Sets the <number_of_components> recorded with the <cmgui_image_information>.
Only valid values are 1=Intensity, 2=IntensityAlpha, 3=RGB, 4=RGBA.
Currently ignored.
Clears 'valid' flag of cmgui_image_information if not correctly set.
==============================================================================*/

int Cmgui_image_information_set_raw_image_storage(
	struct Cmgui_image_information *cmgui_image_information,
	enum Raw_image_storage raw_image_storage);
/*******************************************************************************
LAST MODIFIED : 20 February 2002

DESCRIPTION :
Sets the <raw_image_storage> of <cmgui_image>.
==============================================================================*/

int Cmgui_image_information_set_width(
	struct Cmgui_image_information *cmgui_image_information, int width);
/*******************************************************************************
LAST MODIFIED : 26 February 2002

DESCRIPTION :
Sets the <width> recorded with the <cmgui_image_information>.
Used to specify the width for raw file formats read with Cmgui_image_read.
Clears 'valid' flag of cmgui_image_information if not correctly set.
==============================================================================*/

struct IO_stream_package;

int Cmgui_image_information_set_io_stream_package(
	struct Cmgui_image_information *cmgui_image_information,
	struct IO_stream_package *io_stream_package);
/*******************************************************************************
LAST MODIFIED : 16 September 2004

DESCRIPTION :
Sets the <io_stream_package> recorded with the <cmgui_image_information>.
Used to specify the io_stream_package for raw file formats read with Cmgui_image_read.
Clears 'valid' flag of cmgui_image_information if not correctly set.
==============================================================================*/

int DESTROY(Cmgui_image)(struct Cmgui_image **cmgui_image_address);
/*******************************************************************************
LAST MODIFIED : 13 February 2002

DESCRIPTION :
Frees the memory use by the Cmgui_image and sets <*cmgui_image_address> to NULL.
==============================================================================*/

int Cmgui_image_append(struct Cmgui_image *cmgui_image,
	struct Cmgui_image **second_cmgui_image_address);
/*******************************************************************************
LAST MODIFIED : 27 February 2002

DESCRIPTION :
Appends the Cmgui_image pointer to by <second_cmgui_image_address> on to the
end of <cmgui_image>. Both images must be of the same size and colour depth.
Whether this function succeeds or fails, <second_cmgui_image> will bedestroyed.
==============================================================================*/

struct Cmgui_image *Cmgui_image_constitute(int width, int height,
	int number_of_components, int number_of_bytes_per_component,
	int source_width_bytes, unsigned char *source_pixels);
/*******************************************************************************
LAST MODIFIED : 27 February 2002

DESCRIPTION :
Creates a single Cmgui_image of the specified <width>, <height> with
<number_of_components> where 1=luminance, 2=LuminanceA, 3=RGB, 4=RGBA, and
<number_of_bytes_per_component> which may be 1 or 2.
Data for the image is taken from <source_pixels> which has <source_width_bytes>
of at least <width>*<number_of_components>*<number_of_bytes_per_component>.
The source_pixels are stored in rows from the bottom to top and from left to
right in each row. Pixel colours are interleaved, eg. RGBARGBARGBA...
==============================================================================*/

int Cmgui_image_dispatch(struct Cmgui_image *cmgui_image,
	int image_number, int left, int bottom, int width, int height,
	int padded_width_bytes, int number_of_fill_bytes, unsigned char *fill_bytes,
	int components, unsigned char *destination_pixels);
/*******************************************************************************
LAST MODIFIED : 27 February 2002

DESCRIPTION :
Fills <destination_pixels> with all or part of image <image_number> of
<cmgui_image>, where 0 is the first image.
The <left>, <bottom>, <width> and <height> specify the part of <cmgui_image>
output and must be wholly within its bounds.
Image data is ordered from the bottom row to the top, and within each row from
the left to the right.
If <components> is > 0, the specified components are output at each pixel, 
otherwise all the number_of_components components of the image are output at each pixel.
Pixel values relate to components by:
  1 -> I    = Intensity;
  2 -> IA   = Intensity Alpha;
  3 -> RGB  = Red Green Blue;
  4 -> RGBA = Red Green Blue Alpha;
  5 -> BGR  = Blue Green Red

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

int Cmgui_image_get_height(struct Cmgui_image *cmgui_image);
/*******************************************************************************
LAST MODIFIED : 20 February 2002

DESCRIPTION :
Returns the <height> of <cmgui_image>.
==============================================================================*/

int Cmgui_image_get_number_of_bytes_per_component(
	struct Cmgui_image *cmgui_image);
/*******************************************************************************
LAST MODIFIED : 20 February 2002

DESCRIPTION :
Returns the <number_of_bytes_per_component> of <cmgui_image>, currently either
1 or 2.
==============================================================================*/

int Cmgui_image_get_number_of_components(struct Cmgui_image *cmgui_image);
/*******************************************************************************
LAST MODIFIED : 15 February 2002

DESCRIPTION :
Returns the <number_of_components> - R, G, B, A, I etc. of <cmgui_image>.
==============================================================================*/

int Cmgui_image_get_number_of_images(struct Cmgui_image *cmgui_image);
/*******************************************************************************
LAST MODIFIED : 19 February 2002

DESCRIPTION :
Returns the <number_of_images> stored <cmgui_image>.
==============================================================================*/

int Cmgui_image_get_width(struct Cmgui_image *cmgui_image);
/*******************************************************************************
LAST MODIFIED : 20 February 2002

DESCRIPTION :
Returns the <width> <cmgui_image>.
==============================================================================*/

int Cmgui_image_convert_format(struct Cmgui_image *cmgui_image, enum Cmgui_image_format format);
/*******************************************************************************
LAST MODIFIED : 31 October 2006

DESCRIPTION :
Sets the magick image type and updates the cmgui_image to the format specified by 
the <format>
==============================================================================*/

struct Cmgui_image *Cmgui_image_read(
	struct Cmgui_image_information *cmgui_image_information);
/*******************************************************************************
LAST MODIFIED : 26 February 2002

DESCRIPTION :
Creates a Cmgui_image containing the images from the files listed in the
<cmgui_image_information>. If more than one file_name is listed, they are
checked to be of the same size and assembled together; note that the images
making up the series may not themselves have a third dimension, such as animated
GIF or dicom files, unless only a single filename is given.
The <cmgui_image_information> should be given appropriate width, height
and other parameters for formats that require them.
==============================================================================*/

int Cmgui_image_write(struct Cmgui_image *cmgui_image,
	struct Cmgui_image_information *cmgui_image_information);
/*******************************************************************************
LAST MODIFIED : 4 March 2002

DESCRIPTION :
Writes <cmgui_image> to the filename or filenames listed in the
<cmgui_image_information>. If set, the image_file_format in the information
overrides the file format determined from the extension.
There must be as many filenames supplied as images in <cmgui_image> with the
exception of certain formats which can support all images being written to a
single file, eg. animated GIF, DICOM, in which case a single filename requests
that the images be adjoined in the single file.
==============================================================================*/

#endif /* !defined (IMAGE_UTILITIES_H) */
