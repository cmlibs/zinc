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
#include "api/cmiss_field_module.h"

#ifndef CMISS_FIELD_IMAGE_ID_DEFINED
/*****************************************************************************//**
 * The image field specific handle to a image Cmiss_field.
 */
	struct Cmiss_field_image;
	typedef struct Cmiss_field_image *Cmiss_field_image_id;
	#define CMISS_FIELD_IMAGE_ID_DEFINED
#endif /* CMISS_FIELD_IMAGE_ID_DEFINED */

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
enum Cmiss_field_image_storage_file_format
{
	CMISS_FIELD_IMAGE_STORAGE_FILE_FORMAT_BMP,
	CMISS_FIELD_IMAGE_STORAGE_FILE_FORMAT_DICOM,
	CMISS_FIELD_IMAGE_STORAGE_FILE_FORMAT_JPG,
	CMISS_FIELD_IMAGE_STORAGE_FILE_FORMAT_GIF,
	CMISS_FIELD_IMAGE_STORAGE_FILE_FORMAT_PNG,
	CMISS_FIELD_IMAGE_STORAGE_FILE_FORMAT_SGI,
	CMISS_FIELD_IMAGE_STORAGE_FILE_FORMAT_TIFF
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

/***************************************************************************//**
 * Describes the blending of the texture with the texture constant colour and
 * the underlying fragment colour
 */
enum Cmiss_field_image_combine_mode
{
	CMISS_FIELD_IMAGE_COMBINE_BLEND = 0,
	CMISS_FIELD_IMAGE_COMBINE_DECAL = 1,
	CMISS_FIELD_IMAGE_COMBINE_MODULATE = 2,
	CMISS_FIELD_IMAGE_COMBINE_ADD = 3,
	CMISS_FIELD_IMAGE_COMBINE_ADD_SIGNED = 4,  /*!< Add the value and subtract 0.5 so the texture value
								 effectively ranges from -0.5 to 0.5 */
	CMISS_FIELD_IMAGE_COMBINE_MODULATE_SCALE_4 = 5,  /*!< Multiply and then scale by 4, so that we can
										 scale down or up */
	CMISS_FIELD_IMAGE_COMBINE_BLEND_SCALE_4 = 6,  /*!< Same as blend with a 4 * scaling */
	CMISS_FIELD_IMAGE_COMBINE_SUBTRACT = 7,
	CMISS_FIELD_IMAGE_COMBINE_ADD_SCALE_4 = 8,
	CMISS_FIELD_IMAGE_COMBINE_SUBTRACT_SCALE_4 = 9,
	CMISS_FIELD_IMAGE_COMBINE_INVERT_ADD_SCALE_4 = 10,
	CMISS_FIELD_IMAGE_COMBINE_INVERT_SUBTRACT_SCALE_4 = 11
};

/***************************************************************************//**
 * Whether the texture is compressed.  Could add specific compression formats that
 * are explictly requested from the hardware.
 */
enum Cmiss_field_image_compression_mode
{
	CMISS_FIELD_IMAGE_COMPRESSION_UNCOMPRESSED = 0,
	CMISS_FIELD_IMAGE_COMPRESSION_COMPRESSED_UNSPECIFIED = 1/*!< Allow the hardware to choose the compression */
};

/***************************************************************************//**
 * Specfiy how the graphics hardware rasterises the texture onto the screen.
 */
enum Cmiss_field_image_filter_mode
{
	CMISS_FIELD_IMAGE_FILTER_NEAREST = 0,
	CMISS_FIELD_IMAGE_FILTER_LINEAR = 1,
	CMISS_FIELD_IMAGE_FILTER_NEAREST_MIPMAP_NEAREST = 2,
	CMISS_FIELD_IMAGE_FILTER_LINEAR_MIPMAP_NEAREST = 3,
	CMISS_FIELD_IMAGE_FILTER_LINEAR_MIPMAP_LINEAR = 4
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
 * actual image data if no source_field is provided, which should then be set using 
 * a Cmiss_field_image_set_* function.
 * If a source_field is provided, an internal texture will be evaluated if it has 
 * sizes and dimension defined. If domain field is not provided by the user, 
 * this function will try to take the texture coordinates field from the source 
 * field and if it is not available, this field will automatically
 * create a xi field or get the xi field from the source field region as its domain
 * field. 
 * It is not mandatory to provide domain_field, source_field or both.
 * Texture format will depend on the number of components of the source field.
 * i.e "1 component field creates a LUMINANCE texture, "
 *		 "2 component field creates a LUMINANCE_ALPHA texture, "
 *		 "3 component field creates a RGB texture, "
 *		 "4 component field creates a RGBA texture. "
 * @param field_module  Region field module which will own new field.
 * @param domain_field  The field in which the image data will be embedded.
 * @param source_field  Optional source field to automatically provides pixel
 * values to the image.
 * @return Newly created field
*/
Cmiss_field_id Cmiss_field_module_create_image(Cmiss_field_module_id field_module,
	Cmiss_field_id domain_field, Cmiss_field_id source_field);

/*****************************************************************************//**
 * If the image_field is of type image field then this function returns
 * the image_field specific representation, otherwise returns NULL.
 * Caller is responsible for destroying the new image filter reference.
 *
 * @param image_field  The image field to be cast.
 * @return  Image field specific representation if the input is the correct
 * field type, otherwise returns NULL.
 */
Cmiss_field_image_id Cmiss_field_cast_image(Cmiss_field_id image_field);

/***************************************************************************//**
 * Cast image field back to its base field and return the field.
 * IMPORTANT NOTE: Returned field does not have incremented reference count and
 * must not be destroyed. Use Cmiss_field_access() to add a reference if
 * maintaining returned handle beyond the lifetime of the image argument.
 * Use this function to call base-class API, e.g.:
 * Cmiss_field_set_name(Cmiss_field_iamge_base_cast(image_field), "bob");
 *
 * @param image  Handle to the image field to cast.
 * @return  Non-accessed handle to the base field or NULL if failed.
 */
CMISS_C_INLINE Cmiss_field_id Cmiss_field_image_base_cast(Cmiss_field_image_id image)
{
	return (Cmiss_field_id)(image);
}

/***************************************************************************//**
 * Destroys this reference to the image field (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param image_address  Address of handle to the image field.
 * @return  1 if successfully destroyed the image handle, otherwise 0.
 */
int Cmiss_field_image_destroy(Cmiss_field_image_id *image_address);

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
 * Returns how the image is combined with the material: blend, decal or modulate.
 *
 * @param  image_field  The image field.
 * @return  Returns enum describing how the image is combined with the material.
 */
enum Cmiss_field_image_combine_mode Cmiss_field_image_get_combine_mode(
   Cmiss_field_image_id image_field);

/*****************************************************************************//**
 * Sets how the image is combined with the material: blend, decal or modulate.
 *
 * @param image_field  The image field.
 * @param combine_mode  Enumerator describing how the image is combined with the
 * 		material.
 * @return  Returns 1 if successfully set the combine mode.
 */
int Cmiss_field_image_set_combine_mode(Cmiss_field_image_id image_field,
   enum Cmiss_field_image_combine_mode combine_mode);

/*****************************************************************************//**
 * Returns how the image is stored in graphics memory.
 *
 * @param image_field  The image field.
 * @return  Returns enum describing how the image is stored in graphics memory.
 */
enum Cmiss_field_image_compression_mode Cmiss_field_image_get_compression_mode(
   Cmiss_field_image_id image_field);

/*****************************************************************************//**
 * Indicate to the graphics hardware how you would like the texture stored in
 * graphics memory.
 *
 * @param image_field  The image field.
 * @param compression_mode  Enumerator describing how the image is combined with the
 * 		material.
 * @return  Returns 1 if successfully set the compression mode.
 */
int Cmiss_field_image_set_compression_mode(Cmiss_field_image_id image_field,
   enum Cmiss_field_image_compression_mode compression_mode);

/*****************************************************************************//**
 * Returns how the image is rasterised onto the screen.
 *
 * @param image_field  The image field.
 * @return  Returns enum describing how the image is rasterised onto the screen.
 */
enum Cmiss_field_image_filter_mode Cmiss_field_image_get_filter_mode(
   Cmiss_field_image_id image_field);

/*****************************************************************************//**
 * Indicate to the graphics hardware how you would like the image rasterised
 * onto the screen.
 *
 * @param image_field  The image field.
 * @param filter_mode  Enumerator describing how the graphics hardware rasterises
 *   the texture onto the screen.
 * @return  Returns 1 if successfully set the filter_mode.
 */
int Cmiss_field_image_set_filter_mode(Cmiss_field_image_id image_field,
   enum Cmiss_field_image_filter_mode filter_mode);

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
 * @param file_format  The image file format.
 * @return Returns 1 if the operation is successful, 0 if it is not.
 */
int Cmiss_field_image_storage_information_set_file_format(
	Cmiss_field_image_storage_information_id storage_information,
	enum Cmiss_field_image_storage_file_format file_format);

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
