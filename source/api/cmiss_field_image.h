/*******************************************************************************
FILE : cmiss_field_image.h

LAST MODIFIED : 24 June 2008

DESCRIPTION :
Implements cmiss fields which wrap images, structured grid data.
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
#if !defined (CMISS_FIELD_IMAGE_H)
#define CMISS_FIELD_IMAGE_H

#include "api/cmiss_field.h"

/*****************************************************************************//**
 * The image field specific handle to a image Cmiss_field.
 */
struct Cmiss_field_image;

typedef struct Cmiss_field_image *Cmiss_field_image_id;

enum Cmiss_field_image_storage_pixel_format
/*******************************************************************************
 */
{
	CMISS_FIELD_IMAGE_STORAGE_PIXEL_FORMAT_LUMINANCE,
	CMISS_FIELD_IMAGE_STORAGE_PIXEL_FORMAT_LUMINANCE_ALPHA,
	CMISS_FIELD_IMAGE_STORAGE_PIXEL_FORMAT_RGB,
	CMISS_FIELD_IMAGE_STORAGE_PIXEL_FORMAT_RGBA,
	CMISS_FIELD_IMAGE_STORAGE_PIXEL_FORMAT_ABGR,
	CMISS_FIELD_IMAGE_STORAGE_PIXEL_FORMAT_BGR
}; /* enum Cmiss_field_image_storage_pixel_format */

/*****************************************************************************//**
 * Describes the format for storage.
 * Whether a particular format is actually available depends on whether
 * it is compatible with a particular format type when used with 
 * #Cmiss_field_image_get_formatted_image_data and whether support for that combination
 * has been included when the program was built.
 * This is a small subset of formats available, more can be selected by specifying
 * the appropriate format_string for a Cmiss_field_image_storage_information.
 */
enum Cmiss_field_image_storage_format
{
	CMISS_FIELD_IMAGE_STORAGE_FORMAT_BMP,
	CMISS_FIELD_IMAGE_STORAGE_FORMAT_DICOM,
	CMISS_FIELD_IMAGE_STORAGE_FORMAT_JPG,
	CMISS_FIELD_IMAGE_STORAGE_FORMAT_GIF,
	CMISS_FIELD_IMAGE_STORAGE_FORMAT_PNG,
	CMISS_FIELD_IMAGE_STORAGE_FORMAT_SGI,
	CMISS_FIELD_IMAGE_STORAGE_FORMAT_TIFF
};

/*****************************************************************************//**
 * Describes the type of image compression for storage.
 * Whether a particular image compression is actually available depends on whether
 * it is compatible with a particular format type when used with 
 * #Cmiss_field_image_get_formatted_image_data and whether support for that combination
 * has been included when the program was built.
 */
enum Cmiss_field_image_storage_compression
{
	CMISS_FIELD_IMAGE_STORAGE_COMPRESSION_UNSPECIFIED,
	CMISS_FIELD_IMAGE_STORAGE_COMPRESSION_NONE,
	CMISS_FIELD_IMAGE_STORAGE_COMPRESSION_BZIP,
	CMISS_FIELD_IMAGE_STORAGE_COMPRESSION_FAX,
	CMISS_FIELD_IMAGE_STORAGE_COMPRESSION_JPEG,
	CMISS_FIELD_IMAGE_STORAGE_COMPRESSION_JPEG2000,
	CMISS_FIELD_IMAGE_STORAGE_COMPRESSION_LOSSLESS_JPEG,
	CMISS_FIELD_IMAGE_STORAGE_COMPRESSION_LZW,
	CMISS_FIELD_IMAGE_STORAGE_COMPRESSION_RLE,
	CMISS_FIELD_IMAGE_STORAGE_COMPRESSION_ZIP
};

/*****************************************************************************//**
 * Optional information used to describe the binary data supplied with
 * Cmiss_field_image_set_formatted_image_data and used to specify how to format
 * the binary data retrieved with Cmiss_texture_get_formatted_image_data.
 */
struct Cmiss_field_image_storage_information;

typedef struct Cmiss_field_image_storage_information *Cmiss_field_image_storage_information_id;

/*****************************************************************************//**
 * Creates a new image based field.  This constructor does not define the
 * actual image data, which should then be set using a Cmiss_field_image_set_*
 * function.
 * 
 * @param domain_field  The field in which the image data will be embedded.
 * @return Newly created field
*/
Cmiss_field_id Cmiss_field_create_image(Cmiss_field_id domain_field);

/*****************************************************************************//**
 * If the image_field is of type image field then this function returns
 * the image_field specific representation, otherwise returns NULL.
 * 
 * @param image_field  The image field to be cast.
 * @return  Image field specific representation if the input is the correct
 * field type, otherwise returns NULL.
 */
Cmiss_field_image_id Cmiss_field_image_cast(Cmiss_field_id image_field);

/*****************************************************************************//**
 * Reads image data into the field.
 * The storage_information may specify a filename, series of filenames or
 * a memory block reference to read from.
 * If the format specified in the storage_information 
 * is a "raw" format (such as rgb or gray) which does not embed
 * information about the pixel storage then the data size is expected to be
 * supplied in the storage_information parameter.
 * 
 * @param image_field The image field.
 * @param storage_information  Information about the supplied formatted image data.
 * At a minimum it should specify either a filename or a memory block
 * reference.
 * @return Returns 1 if the operation is successful, 0 if it is not.
 */
int Cmiss_field_image_read(Cmiss_field_image_id image_field,
	Cmiss_field_image_storage_information_id storage_information);

/*****************************************************************************//**
 * Writes a formatted representation of the image data.
 * The storage_information is used to control the formatted output.
 * If a memory block reference has been specified to the storage_information
 * then this will be allocated and set and the corresponding memory block
 * length set.
 * Otherwise the routine will try to write to the filename set on the 
 * storage information.
 * The routine should fail if the values specified in the storage_information
 * cannot be respected.
 * If one or two of the size parameters are set on the storage_information
 * then other dimensions will be adjusted to maintain aspect ratio and then the image is
 * resized just for this output.
 * 
 * @param image_field The image field.
 * @param storage_information  Information specifying the required format
 * for the returned formatted image data.
 * At a minimum it should specify either a filename or a memory block
 * reference.
 * @return Returns 1 if the operation is successful, 0 if it is not.
 */
int Cmiss_field_image_write(Cmiss_field_image_id image_field,
	Cmiss_field_image_storage_information_id storage_information);

/*****************************************************************************//**
 * A simple image read function that reads from a single filename.
 * 
 * @param image_field The image field.
 * @param storage_information  The file to read from.  The format is normally
 * determined from the extension but can be specified with a colon separated prefix.
 * @return Returns 1 if the operation is successful, 0 if it is not.
 */
int Cmiss_field_image_read_file(Cmiss_field_image_id image_field,
	const char *file_name);

/*****************************************************************************//**
 * A simple image write function that writes to a single filename.
 * 
 * @param image_field The image field.
 * @param file_name  The file to write out to.  The format is normally
 * determined from the extension but can be specified with a colon separated prefix.
 * @return Returns 1 if the operation is successful, 0 if it is not.
 */
int Cmiss_field_image_write_file(Cmiss_field_image_id image_field,
	const char *file_name);

/*****************************************************************************//**
 * Creates a Cmiss_field_image_storage_information object.
 * @return The created object.
 */
Cmiss_field_image_storage_information_id Cmiss_field_image_storage_information_create(void);

/*****************************************************************************//**
 * Destroys a Cmiss_field_image_storage_information object.
 * @param storage_information_address  Pointer to a storage_information object, which
 * is destroyed and the pointer is set to NULL.
 * @return Returns 1 if the operation is successful, 0 if it is not.
 */
int Cmiss_field_image_storage_information_destroy(
	Cmiss_field_image_storage_information_id *storage_information_address);

/*****************************************************************************//**
 * Adds a file name to the list that will be read from or written to when
 * this storage_information is used with #Cmiss_field_image_read and
 * #Cmiss_field_image_write.
 * 
 * @param storage_information  The storage information object.
 * @param file_name  A file name for reading from or writing to.
 * If the format_string has a prefix, like "jpg:fred" then this prefix is used to
 * describe a format, otherwise the format_string suffix is used, "fred.jpg".
 * If #Cmiss_field_image_storage_information_set_format is
 * also used then it overrides this string (by prepending the appropriate prefix
 * internally).
 * If #Cmiss_field_image_storage_information_set_memory_block or
 * #Cmiss_field_image_storage_information_set_write_to_memory_block are also
 * called then this file_name will only be used to help specify the format
 * of the memory data and no file access will be made.
 * @return Returns 1 if the operation is successful, 0 if it is not.
 */
int Cmiss_field_image_storage_information_add_file_name(
	Cmiss_field_image_storage_information_id storage_information,
	const char *file_name);

/*****************************************************************************//**
 * Specifies the format for binary data with this storage information using a 
 * enumerated type.  Only a subset of available types can be specified with this
 * function, more are available using #Cmiss_field_image_storage_information_set_format_string.
 * 
 * @param storage_information  The storage information object.
 * @param format  The image format.
 * @return Returns 1 if the operation is successful, 0 if it is not.
 */
int Cmiss_field_image_storage_information_set_format(
	Cmiss_field_image_storage_information_id storage_information,
	enum Cmiss_field_image_storage_format format);

/*****************************************************************************//**
 * Specifies the pixel width for binary data using this storage_information.
 * 
 * @param storage_information  The storage information object.
 * @param width  The width of the formatted data in pixels.
 * @return Returns 1 if the operation is successful, 0 if it is not.
 */
int Cmiss_field_image_storage_information_set_width(
	Cmiss_field_image_storage_information_id storage_information,
	unsigned int width);

/*****************************************************************************//**
 * Specifies the pixel height for binary data using this storage_information.
 * 
 * @param storage_information  The storage information object.
 * @param height  The height of the formatted data in pixels.
 * @return Returns 1 if the operation is successful, 0 if it is not.
 */
int Cmiss_field_image_storage_information_set_height(
	Cmiss_field_image_storage_information_id storage_information,
	unsigned int height);

/*****************************************************************************//**
 * Specifies the pixel depth (the 3D size in pixels) for binary data using this 
 * storage_information.
 * 
 * @param storage_information  The storage information object.
 * @param depth  The depth of the formatted data in pixels.
 * @return Returns 1 if the operation is successful, 0 if it is not.
 */
/*int Cmiss_field_image_storage_information_set_depth(
	Cmiss_field_image_storage_information_id storage_information,
	unsigned int depth);*/

/*****************************************************************************//**
 * Specifies the pixel format for binary data using this storage_information.
 * 
 * @param storage_information  The storage information object.
 * @param pixel_format  The pixel_format of the formatted data.
 * @return Returns 1 if the operation is successful, 0 if it is not.
 */
int Cmiss_field_image_storage_information_set_pixel_format(
	Cmiss_field_image_storage_information_id storage_information,
	enum Cmiss_field_image_storage_pixel_format pixel_format);

/*****************************************************************************//**
 * Specifies the number of bytes per component for binary data using this
 * storage_information.
 * 
 * @param storage_information  The storage information object.
 * @param number_of_bytes_per_component  The number of bytes per pixel component
 * of the formatted data.
 * @return Returns 1 if the operation is successful, 0 if it is not.
 */
int Cmiss_field_image_storage_information_set_number_of_bytes_per_component(
	Cmiss_field_image_storage_information_id storage_information,
	unsigned int number_of_bytes_per_component);

/*****************************************************************************//**
 * Specifies the compression type for binary data using this storage_information.
 * 
 * @param storage_information  The storage information object.
 * @param compression  The type of compression applied.  Various combinations of image
 * format, compression and quality may or may not work together.
 * @return Returns 1 if the operation is successful, 0 if it is not.
 */
int Cmiss_field_image_storage_information_set_compression(
	Cmiss_field_image_storage_information_id storage_information,
	enum Cmiss_field_image_storage_compression compression);
	
/*****************************************************************************//**
 * Specifies the quality for binary data using this storage_information.
 * 
 * @param storage_information  The storage information object.
 * @param quality  This parameter controls compression for compressed lossy formats,
 * where a quality of 1.0 specifies the least lossy output for a given format and a
 * quality of 0.0 specifies the most compression.
 * @return Returns 1 if the operation is successful, 0 if it is not.
 */
int Cmiss_field_image_storage_information_set_quality(
	Cmiss_field_image_storage_information_id storage_information,
	double quality);

/*****************************************************************************//**
 * Specifies that this storage_information will read from a memory_block
 * instead of reading from a file.
 * 
 * @param storage_information  The storage information object.
 * @param memory_block  A pointer to memory_block information.
 * @param memory_block_length  The length of this memory_block.
 * @return Returns 1 if the operation is successful, 0 if it is not.
 */
int Cmiss_field_image_storage_information_set_memory_block(
	Cmiss_field_image_storage_information_id storage_information,
	void *memory_block, unsigned int memory_block_length);

/*****************************************************************************//**
 * Specifies that this storage_information will write to a memory_block
 * instead of writing to file.  Once read the new memory block can be
 * retrieved with #Cmiss_field_image_storage_information_set_memory_block.
 * 
 * @param storage_information  The storage information object.
 * @return Returns 1 if the operation is successful, 0 if it is not.
 */
int Cmiss_field_image_storage_information_set_write_to_memory_block(
	Cmiss_field_image_storage_information_id storage_information);

/*****************************************************************************//**
 * Retrieve a memory block that has been written to when the storage_information
 * specified #Cmiss_field_image_storage_information_set_write_to_memory_block.
 * 
 * @param storage_information  The storage information object.
 * @param memory_block_reference  Will be set to point to the allocated
 * memory block.  When no longer required the memory block should be
 * released with #Cmiss_deallocate.
 * @param memory_block_length_reference  Will be set to the length of
 * the returned memory block.
 * @return Returns 1 if the operation is successful, 0 if it is not.
 */
int Cmiss_field_image_storage_information_get_memory_block(
	Cmiss_field_image_storage_information_id storage_information,
	void **memory_block, unsigned int *memory_block_length);

#endif /* !defined (CMISS_FIELD_IMAGE_H) */
