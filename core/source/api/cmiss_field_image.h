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

#include "types/cmiss_c_inline.h"
#include "types/cmiss_field_id.h"
#include "types/cmiss_field_image_id.h"
#include "types/cmiss_field_module_id.h"
#include "types/cmiss_stream_id.h"

/*****************************************************************************//**
 * Describes the format for storage.
 * Whether a particular format is actually available depends on whether
 * it is compatible with a particular format type when used with
 * #Cmiss_field_image_get_formatted_image_data and whether support for that combination
 * has been included when the program was built.
 * This is a small subset of formats available, more can be selected by specifying
 * the appropriate format_string for a Cmiss_stream_information_image.
 */
enum Cmiss_stream_information_image_file_format
{
	CMISS_STREAM_INFORMATION_IMAGE_FILE_FORMAT_INVALID = 0,
	CMISS_STREAM_INFORMATION_IMAGE_FILE_FORMAT_BMP = 1,
	CMISS_STREAM_INFORMATION_IMAGE_FILE_FORMAT_DICOM = 2,
	CMISS_STREAM_INFORMATION_IMAGE_FILE_FORMAT_JPG = 3,
	CMISS_STREAM_INFORMATION_IMAGE_FILE_FORMAT_GIF = 4,
	CMISS_STREAM_INFORMATION_IMAGE_FILE_FORMAT_PNG = 5,
	CMISS_STREAM_INFORMATION_IMAGE_FILE_FORMAT_SGI = 6,
	CMISS_STREAM_INFORMATION_IMAGE_FILE_FORMAT_TIFF = 7
};

/***************************************************************************//**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
enum Cmiss_stream_information_image_file_format
	Cmiss_stream_information_image_file_format_enum_from_string(const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param format  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
char *Cmiss_stream_information_image_file_format_enum_to_string(
	enum Cmiss_stream_information_image_file_format format);

/***************************************************************************//**
 * Describes the blending of the texture with the texture constant colour and
 * the underlying fragment colour
 */
enum Cmiss_field_image_combine_mode
{
	CMISS_FIELD_IMAGE_COMBINE_INVALID = 0,
	CMISS_FIELD_IMAGE_COMBINE_BLEND = 1,
	CMISS_FIELD_IMAGE_COMBINE_DECAL = 2,
	CMISS_FIELD_IMAGE_COMBINE_MODULATE = 3,
	CMISS_FIELD_IMAGE_COMBINE_ADD = 4,
	CMISS_FIELD_IMAGE_COMBINE_ADD_SIGNED = 5,  /*!< Add the value and subtract 0.5 so the texture value
								 effectively ranges from -0.5 to 0.5 */
	CMISS_FIELD_IMAGE_COMBINE_MODULATE_SCALE_4 = 6,  /*!< Multiply and then scale by 4, so that we can
										 scale down or up */
	CMISS_FIELD_IMAGE_COMBINE_BLEND_SCALE_4 = 7,  /*!< Same as blend with a 4 * scaling */
	CMISS_FIELD_IMAGE_COMBINE_SUBTRACT = 8,
	CMISS_FIELD_IMAGE_COMBINE_ADD_SCALE_4 = 9,
	CMISS_FIELD_IMAGE_COMBINE_SUBTRACT_SCALE_4 = 10,
	CMISS_FIELD_IMAGE_COMBINE_INVERT_ADD_SCALE_4 = 11,
	CMISS_FIELD_IMAGE_COMBINE_INVERT_SUBTRACT_SCALE_4 = 12
};

/***************************************************************************//**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
enum Cmiss_field_image_combine_mode
	Cmiss_field_image_combine_mode_enum_from_string(const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param mode  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
char *Cmiss_field_image_combine_mode_enum_to_string(
	enum Cmiss_field_image_combine_mode mode);

/***************************************************************************//**
 * Whether the texture is compressed.  Could add specific compression formats that
 * are explictly requested from the hardware.
 */
enum Cmiss_field_image_hardware_compression_mode
{
	CMISS_FIELD_IMAGE_HARDWARE_COMPRESSION_MODE_INVALID = 0,
	CMISS_FIELD_IMAGE_HARDWARE_COMPRESSION_MODE_UNCOMPRESSED = 1,
	CMISS_FIELD_IMAGE_HARDWARE_COMPRESSION_MODE_AUTOMATIC = 2
	/*!< Allow the hardware to choose the compression */
};

/***************************************************************************//**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
enum Cmiss_field_image_hardware_compression_mode
	Cmiss_field_image_hardware_compression_mode_enum_from_string(const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param mode  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
char *Cmiss_field_image_hardware_compression_mode_enum_to_string(
	enum Cmiss_field_image_hardware_compression_mode mode);

/***************************************************************************//**
 * Specfiy how the graphics hardware rasterises the texture onto the screen.
 */
enum Cmiss_field_image_filter_mode
{
	CMISS_FIELD_IMAGE_FILTER_INVALID = 0,
	CMISS_FIELD_IMAGE_FILTER_NEAREST = 1,
	CMISS_FIELD_IMAGE_FILTER_LINEAR = 2,
	CMISS_FIELD_IMAGE_FILTER_NEAREST_MIPMAP_NEAREST = 3,
	CMISS_FIELD_IMAGE_FILTER_LINEAR_MIPMAP_NEAREST = 4,
	CMISS_FIELD_IMAGE_FILTER_LINEAR_MIPMAP_LINEAR = 5
};

/***************************************************************************//**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
enum Cmiss_field_image_filter_mode Cmiss_field_image_filter_mode_enum_from_string(
		const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param mode  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
char *Cmiss_field_image_filter_mode_enum_to_string(
	enum Cmiss_field_image_filter_mode mode);

enum Cmiss_field_image_attribute
{
	CMISS_FIELD_IMAGE_ATTRIBUTE_INVALID = 0,
	CMISS_FIELD_IMAGE_ATTRIBUTE_RAW_WIDTH_PIXELS = 1,
	CMISS_FIELD_IMAGE_ATTRIBUTE_RAW_HEIGHT_PIXELS = 2,
	CMISS_FIELD_IMAGE_ATTRIBUTE_RAW_DEPTH_PIXELS = 3,
	CMISS_FIELD_IMAGE_ATTRIBUTE_PHYSICAL_WIDTH_PIXELS = 4,
	CMISS_FIELD_IMAGE_ATTRIBUTE_PHYSICAL_HEIGHT_PIXELS = 5,
	CMISS_FIELD_IMAGE_ATTRIBUTE_PHYSICAL_DEPTH_PIXELS = 6
};

/***************************************************************************//**
 * Convert a short attribute name into an enum if the attribute name matches
 * any of the members in the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
enum Cmiss_field_image_attribute Cmiss_field_image_attribute_enum_from_string(
	const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param attribute  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
char *Cmiss_field_image_attribute_enum_to_string(
	enum Cmiss_field_image_attribute attribute);

/***************************************************************************//**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
enum Cmiss_stream_information_image_pixel_format
	Cmiss_stream_information_image_pixel_format_enum_from_string(
		const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param format  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
char *Cmiss_stream_information_image_pixel_format_enum_to_string(
	enum Cmiss_stream_information_image_pixel_format format);

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
 * @return  Status CMISS_OK if successfully destroyed the image handle,
 * any other value on failure.
 */
int Cmiss_field_image_destroy(Cmiss_field_image_id *image_address);

/***************************************************************************//**
 * Get an integer or Boolean attribute of the field image.
 *
 * @param image  Handle to the cmiss field image.
 * @param attribute  The identifier of the integer attribute to get.
 * @return  Value of the attribute. Boolean values are 1 if true, 0 if false.
 */
int Cmiss_field_image_get_attribute_integer(Cmiss_field_image_id image,
	enum Cmiss_field_image_attribute attribute);

/***************************************************************************//**
 * Get a real attribute of the field image.
 *
 * @param image  Handle to the cmiss field image.
 * @param attribute  The identifier of the real attribute to get.
 * @return  Value of the attribute.
 */
double Cmiss_field_image_get_attribute_real(Cmiss_field_image_id image,
	enum Cmiss_field_image_attribute attribute);

/***************************************************************************//**
 * Set an double attribute of the image.
 *
 * @param stream_information  Handle to the cmiss stream_information_image.
 * @param attribute  The identifier of the double attribute to set.
 * @param value  The new value for the attribute.
 * @return  Status CMISS_OK if attribute successfully set, any other value if
 * failed or attribute not valid or unable to be set for this image.
 */
int Cmiss_field_image_set_attribute_real(Cmiss_field_image_id image,
	enum Cmiss_field_image_attribute attribute, double value);

/*****************************************************************************//**
 * Reads image data into the field.
 * The stream_information may specify a filename, series of filenames or
 * a memory block reference to read from.
 * If the format specified in the stream_information
 * is a "raw" format (such as rgb or gray) which does not embed
 * information about the pixel storage then the data size is expected to be
 * supplied in the stream_information parameter.
 *
 * @param image_field The image field.
 * @param stream_information  Information about the supplied formatted image data.
 * At a minimum it should specify either a filename or a memory block
 * reference.
 * @return  Status CMISS_OK if the operation is successful, any other value on
 * failure.
 */
int Cmiss_field_image_read(Cmiss_field_image_id image_field,
	Cmiss_stream_information_id stream_information);

/***************************************************************************//**
 * Convenient function to read a file with the provided name into a field image
 * directly.
 *
 * @param image_field  The field image to be read into.
 * @param file_name  name of the file to read from.
 *
 * @return  Status CMISS_OK if data successfully read and merged into specified
 * region, any other value on failure.
 */
int Cmiss_field_image_read_file(Cmiss_field_image_id image_field, const char *file_name);

/*****************************************************************************//**
 * Writes a formatted representation of the image data.
 * The stream_information is used to control the formatted output.
 * If a memory block reference has been specified to the io_stream
 * then this will be allocated and set and the corresponding memory block
 * length set.
 * Otherwise the routine will try to write to the filename set on the
 * storage information.
 * The routine should fail if the values specified in the stream_information
 * cannot be respected.
 * If one or two of the size parameters are set on the stream_information
 * then other dimensions will be adjusted to maintain aspect ratio and then the image is
 * resized just for this output.
 *
 * @param image_field The image field.
 * @param stream_information  Information specifying the required format
 * for the returned formatted image data.
 * @return  Status CMISS_OK if the operation is successful, any other value
 * on failure.
 */
int Cmiss_field_image_write(Cmiss_field_image_id image_field,
	Cmiss_stream_information_id stream_information);

/***************************************************************************//**
 * Convenient function to write the image into a file with the provided name.
 *
 * @param image_field  The image_field which stores the image.
 * @param file_name  name of the file to write to..
 *
 * @return  Status CMISS_OK if data is successfully written out, any other value
 * on failure.
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
 * @return  Status CMISS_OK on success, any other value on failure.
 */
int Cmiss_field_image_set_combine_mode(Cmiss_field_image_id image_field,
   enum Cmiss_field_image_combine_mode combine_mode);

/*****************************************************************************//**
 * Returns how the image is stored in graphics memory.
 *
 * @param image_field  The image field.
 * @return  Returns enum describing how the image is stored in graphics memory.
 */
enum Cmiss_field_image_hardware_compression_mode Cmiss_field_image_get_hardware_compression_mode(
   Cmiss_field_image_id image_field);

/*****************************************************************************//**
 * Indicate to the graphics hardware how you would like the texture stored in
 * graphics memory.
 *
 * @param image_field  The image field.
 * @param compression_mode  Enumerator describing how the image is combined with the
 * 		material.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
int Cmiss_field_image_set_hardware_compression_mode(Cmiss_field_image_id image_field,
   enum Cmiss_field_image_hardware_compression_mode compression_mode);

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
 * @return  Status CMISS_OK on success, any other value on failure.
 */
int Cmiss_field_image_set_filter_mode(Cmiss_field_image_id image_field,
   enum Cmiss_field_image_filter_mode filter_mode);

/**
 * Gets the property named in the given property string from the given field image.
 * The returned string must be deallocated by the receiver.
 *
 * @param	image   	The image field.
 * @param	property	The property to retrieve.
 *
 * @return	null if it fails, else an allocated string containing the properties value.
 */
char *Cmiss_field_image_get_property(Cmiss_field_image_id image,
	const char* property);

enum Cmiss_stream_information_image_attribute
{
	CMISS_STREAM_INFORMATION_IMAGE_ATTRIBUTE_RAW_WIDTH_PIXELS = 1,
	/*!< Integer specifies the pixel width for binary data reading in using this
	 * stream_information.
	 */
	CMISS_STREAM_INFORMATION_IMAGE_ATTRIBUTE_RAW_HEIGHT_PIXELS = 2,
	/*!< Integer specifies the pixel height for binary data reading in using this
	 * stream_information.
	 */
	CMISS_STREAM_INFORMATION_IMAGE_ATTRIBUTE_BITS_PER_COMPONENT = 3,
	/*!< Integer specifies the number of bytes per component for binary data using
	 * this stream_information. Only 8 and 16 bits are supported at the moment.
	 */
	CMISS_STREAM_INFORMATION_IMAGE_ATTRIBUTE_COMPRESSION_QUALITY = 4
	/*!< Real number specifies the quality for binary data using this stream_information.
	 * This parameter controls compression for compressed lossy formats,
	 * where a quality of 1.0 specifies the least lossy output for a given format and a
	 * quality of 0.0 specifies the most compression.
	 */
};

/***************************************************************************//**
 * Convert a short attribute name into an enum if the attribute name matches
 * any of the members in the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
enum Cmiss_stream_information_image_attribute
	Cmiss_stream_information_image_attribute_enum_from_string(const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param attribute  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
char *Cmiss_stream_information_image_attribute_enum_to_string(
	enum Cmiss_stream_information_image_attribute attribute);

/*****************************************************************************//**
 * Creates a Cmiss_stream_information_image object.
 * @return The created object.
 */
Cmiss_stream_information_id Cmiss_field_image_create_stream_information(
	Cmiss_field_image_id image_field);

/*****************************************************************************//**
 * Destroys a Cmiss_stream_information_image object.
 *
 * @param stream_information_address  Pointer to a stream_information object, which
 * is destroyed and the pointer is set to NULL.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
int Cmiss_stream_information_image_destroy(
	Cmiss_stream_information_image_id *stream_information_address);

/***************************************************************************//**
 * If the stream_information is of field_image type, then this function returns
 * the field_image specific representation, otherwise it returns NULL.
 * Caller is responsible for destroying the returned derived reference.
 *
 * @param stream_information  The generic stream_information to be cast.
 * @return  field_image specific representation if the input stream_information is
 * of this type, otherwise NULL.
 */
Cmiss_stream_information_image_id Cmiss_stream_information_cast_image(
	Cmiss_stream_information_id stream_information);

/***************************************************************************//**
 * Cast stream_information_image back to its base stream_information and
 * return the stream_information.
 * IMPORTANT NOTE: Returned stream_information does not have incremented
 * reference count and must not be destroyed. Use Cmiss_stream_information_access()
 * to add a reference if maintaining returned handle beyond the lifetime of the
 * stream_information_image argument.
 *
 * @param stream_information  Handle to the stream_information_image_ to cast.
 * @return  Non-accessed handle to the base stream information or NULL if failed.
 */
CMISS_C_INLINE Cmiss_stream_information_id
	Cmiss_stream_information_image_base_cast(
		Cmiss_stream_information_image_id stream_information)
{
	return (Cmiss_stream_information_id)(stream_information);
}

/***************************************************************************//**
 * Set an integer or Boolean attribute of the stream_information_image.
 *
 * @param stream_information  Handle to the cmiss stream_information_image.
 * @param attribute  The identifier of the integer attribute to set.
 * @param value  The new value for the attribute. For Boolean values use 1 for
 * true.
 *
 * @return  Status CMISS_OK if attribute successfully set, any other value if
 *	failed or attribute not valid or unable to be set for this
 * 	stream_information_image.
 */
int Cmiss_stream_information_image_set_attribute_integer(
	Cmiss_stream_information_image_id stream_information,
	enum Cmiss_stream_information_image_attribute attribute, int value);

/***************************************************************************//**
 * Set an double attribute of the stream_information_image.
 *
 * @param stream_information  Handle to the cmiss stream_information_image.
 * @param attribute  The identifier of the double attribute to set.
 * @param value  The new value for the attribute.
 * @return   Status CMISS_OK if attribute successfully set, any other value if
 * failed or attribute not valid or unable to be set for this
 * stream_information_image.
 */
int Cmiss_stream_information_image_set_attribute_real(
	Cmiss_stream_information_image_id stream_information,
	enum Cmiss_stream_information_image_attribute attribute,
	double value);

/*****************************************************************************//**
 * Specifies the format for binary data to be read/written using this
 * stream_information.
 *
 * @param stream_information  The storage information object.
 * @param file_format  The image file format.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
int Cmiss_stream_information_image_set_file_format(
	Cmiss_stream_information_image_id stream_information,
	enum Cmiss_stream_information_image_file_format file_format);

/*****************************************************************************//**
 * Specifies the pixel format for binary data of the images  to be read/written
 * using this stream_information.
 *
 * @param stream_information  The storage information object.
 * @param pixel_format  The pixel_format of the formatted data.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
int Cmiss_stream_information_image_set_pixel_format(
	Cmiss_stream_information_image_id stream_information,
	enum Cmiss_stream_information_image_pixel_format pixel_format);

#endif /* !defined (CMISS_FIELD_IMAGE_H) */
