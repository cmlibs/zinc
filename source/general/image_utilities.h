/*******************************************************************************
FILE : image_utilities.h

LAST MODIFIED : 28 may 1999

DESCRIPTION :
Utilities for handling images.
==============================================================================*/
#if !defined (IMAGE_UTILITIES_H)
#define IMAGE_UTILITIES_H
#include "user_interface/printer.h"

#define TIFF_HI_LO 0x4D4D
#define TIFF_LO_HI 0x4949

/*
Global types
------------
*/
enum Image_file_format
{
	POSTSCRIPT_FILE_FORMAT,
	RGB_FILE_FORMAT,
	TIFF_FILE_FORMAT
}; /* enum Image_file_format */

enum Image_orientation
{
	LANDSCAPE_ORIENTATION,
	PORTRAIT_ORIENTATION
}; /* enum Image_orientation */

enum Tiff_image_compression
{
	TIFF_NO_COMPRESSION,
	TIFF_LZW_COMPRESSION,
	TIFF_PACK_BITS_COMPRESSION
}; /* enum Tiff_image_compression */

/*
Global functions
----------------
*/
int write_rgb_image_file(char *file_name,int number_of_components,
	int number_of_bytes_per_component,
	int number_of_rows,int number_of_columns,int row_padding,
	long unsigned *image);
/*******************************************************************************
LAST MODIFIED : 25 August 1999

DESCRIPTION :
Writes an image in SGI rgb file format.
<row_padding> indicates a number of bytes that is padding on each row of data 
(textures are required to be in multiples of two).
==============================================================================*/

int write_postscript_image_file(char *file_name,int number_of_components,
	int number_of_bytes_per_component,
	int number_of_rows,int number_of_columns,int row_padding,
	float pixel_aspect_ratio,long unsigned *image,
	enum Image_orientation image_orientation,struct Printer *printer);
/*******************************************************************************
LAST MODIFIED : 26 August 1999

DESCRIPTION :
Writes an image in Postscript file format.
<row_padding> indicates a number of bytes that is padding on each row of data 
(textures are required to be in multiples of two).
==============================================================================*/

int write_tiff_image_file(char *file_name,int number_of_components,
	int number_of_bytes_per_component,
	int number_of_rows,int number_of_columns, int row_padding,
	enum Tiff_image_compression compression,long unsigned *image);
/*******************************************************************************
LAST MODIFIED : 26 August 1999

DESCRIPTION :
Writes an <image> in TIFF file format using <compression>.
<row_padding> indicates a number of bytes that is padding on each row of data 
(textures are required to be in multiples of two).
==============================================================================*/

int read_rgb_image_file(char *file_name,int *number_of_components,
	int *number_of_bytes_per_component,
	long int *height,long int *width, long unsigned **image);
/*******************************************************************************
LAST MODIFIED : 25 August 1999

DESCRIPTION :
Reads an image from a SGI rgb file.
???DB.  Need to find out more about images.
==============================================================================*/

int read_tiff_image_file(char *file_name,int *number_of_components,
	int *number_of_bytes_per_component,
	long int *height,long int *width,
	long unsigned **image);
/*******************************************************************************
LAST MODIFIED : 26 August 1999

DESCRIPTION :
Reads an image from a TIFF file.
==============================================================================*/

int read_image_file(char *file_name, int *number_of_components,
	int *number_of_bytes_per_component, long int *height,long int *width,
	long unsigned **image);
/*******************************************************************************
LAST MODIFIED : 25 August 1999

DESCRIPTION :
Detects the image type from the file extension (rgb/tiff) and then reads.
==============================================================================*/

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

int undistort_image(unsigned long **image,
	int number_of_components,int number_of_bytes_per_component,
	int height,int width,
	double centre_x,double centre_y,double factor_k1);
/*******************************************************************************
LAST MODIFIED : 25 August 1999

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

unsigned long *copy_image(unsigned long *image,int number_of_components,
	int width,int height);
/*******************************************************************************
LAST MODIFIED : 27 April 1998

DESCRIPTION :
Allocates and returns a copy of <image>.
==============================================================================*/

int crop_image(unsigned long **image,int number_of_components,
	int number_of_bytes_per_component, int *width,int *height,
	int crop_left_margin,int crop_bottom_margin,int crop_width,int crop_height);
/*******************************************************************************
LAST MODIFIED : 25 August 1999

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
#endif /* !defined (IMAGE_UTILITIES_H) */
