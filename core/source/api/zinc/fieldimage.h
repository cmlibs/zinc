/**
 * FILE : fieldimage.h
 *
 * Public interface to image field which stores images / structured grid data.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDIMAGE_H__
#define CMZN_FIELDIMAGE_H__

#include "types/fieldid.h"
#include "types/fieldimageid.h"
#include "types/fieldmoduleid.h"
#include "types/streamid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum cmzn_streaminformation_image_file_format
	cmzn_streaminformation_image_file_format_enum_from_string(const char *string);

/**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call cmzn_deallocate to destroy the successfully returned string.
 *
 * @param format  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *cmzn_streaminformation_image_file_format_enum_to_string(
	enum cmzn_streaminformation_image_file_format format);

/**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum cmzn_field_image_combine_mode
	cmzn_field_image_combine_mode_enum_from_string(const char *string);

/**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call cmzn_deallocate to destroy the successfully returned string.
 *
 * @param mode  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *cmzn_field_image_combine_mode_enum_to_string(
	enum cmzn_field_image_combine_mode mode);

/**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum cmzn_field_image_hardware_compression_mode
	cmzn_field_image_hardware_compression_mode_enum_from_string(const char *string);

/**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call cmzn_deallocate to destroy the successfully returned string.
 *
 * @param mode  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *cmzn_field_image_hardware_compression_mode_enum_to_string(
	enum cmzn_field_image_hardware_compression_mode mode);

/**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum cmzn_field_image_filter_mode cmzn_field_image_filter_mode_enum_from_string(
		const char *string);

/**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call cmzn_deallocate to destroy the successfully returned string.
 *
 * @param mode  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *cmzn_field_image_filter_mode_enum_to_string(
	enum cmzn_field_image_filter_mode mode);

/**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum cmzn_field_image_wrap_mode
	cmzn_field_image_wrap_mode_enum_from_string(const char *string);

/**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call cmzn_deallocate to destroy the successfully returned string.
 *
 * @param mode  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *cmzn_field_image_wrap_mode_enum_to_string(
	enum cmzn_field_image_wrap_mode mode);

/**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum cmzn_streaminformation_image_pixel_format
	cmzn_streaminformation_image_pixel_format_enum_from_string(
		const char *string);

/**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call cmzn_deallocate to destroy the successfully returned string.
 *
 * @param format  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *cmzn_streaminformation_image_pixel_format_enum_to_string(
	enum cmzn_streaminformation_image_pixel_format format);

/**
 * Creates a new image field. The new field has no image data; this must be set
 * by calling cmzn_field_image_* functions, e.g. cmzn_field_image_read().
 * The new field is given a default domain field which one can get (or set) with
 * image field functions. To evaluate the image field you will need to set
 * values of the domain field to texture coordinate locations.
 *
 * @param field_module  Region field module which will own the image field.
 * @return  Newly created image field. Up to caller to destroy handle.
*/
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_image(
	cmzn_fieldmodule_id field_module);

/**
 * Creates a new image field whose image data is sampled from the source
 * field. The source field is typically an image or image-processing field, and
 * its dimension, native resolution and domain field are used as defaults for
 * the new field, or may be changed via image field functions.
 * Texture format will depend on the number of components of the source field:
 * 1 component field creates a LUMINANCE image
 * 2 component field creates a LUMINANCE_ALPHA image
 * 3 component field creates an RGB image
 * 4 component field creates an RGBA image
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  Source field providing image pixel values. Must be
 * image-based with up to 4 components.
 * @return  Newly created image field. Up to caller to destroy handle.
*/
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_image_from_source(
	cmzn_fieldmodule_id field_module, cmzn_field_id source_field);

/**
 * If the image_field is of type image field then this function returns
 * the image_field specific representation, otherwise returns NULL.
 * Caller is responsible for destroying the new image filter reference.
 *
 * @param image_field  The image field to be cast.
 * @return  Image field specific representation if the input is the correct
 * field type, otherwise returns NULL.
 */
ZINC_API cmzn_field_image_id cmzn_field_cast_image(cmzn_field_id image_field);

/**
 * Cast image field back to its base field and return the field.
 * IMPORTANT NOTE: Returned field does not have incremented reference count and
 * must not be destroyed. Use cmzn_field_access() to add a reference if
 * maintaining returned handle beyond the lifetime of the image argument.
 * Use this function to call base-class API, e.g.:
 * cmzn_field_set_name(cmzn_field_iamge_base_cast(image_field), "bob");
 *
 * @param image  Handle to the image field to cast.
 * @return  Non-accessed handle to the base field or NULL if failed.
 */
ZINC_C_INLINE cmzn_field_id cmzn_field_image_base_cast(cmzn_field_image_id image)
{
	return (cmzn_field_id)(image);
}

/**
 * Destroys this reference to the image field (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param image_address  Address of handle to the image field.
 * @return  Status CMZN_OK if successfully destroyed the image handle,
 * any other value on failure.
 */
ZINC_API int cmzn_field_image_destroy(cmzn_field_image_id *image_address);

/**
 * Get the width of the image.
 *
 * @param image   handle to the image field.
 * @return  width of the image
 */
ZINC_API int cmzn_field_image_get_width_in_pixels(cmzn_field_image_id image);

/**
 * Get the height of the image.
 *
 * @param image   handle to the image field.
 * @return  height of the image
 */
ZINC_API int cmzn_field_image_get_height_in_pixels(cmzn_field_image_id image);

/**
 * Get the depth of the image.
 *
 * @param image   handle to the image field.
 * @return  depth of the image
 */
ZINC_API int cmzn_field_image_get_depth_in_pixels(cmzn_field_image_id image);

/**
 * Get the pixel sizes of the image.
 *
 * @param image  The image to query.
 * @param valuesCount  The size of the sizes array to fill. Values
 * for dimensions beyond the size have the value of 1.
 * @param valuesOut  Array to receive pixel sizes.
 * @return  The actual dimension that have been read. This can be
 * more than the number requested, so a second call may be needed with a
 * larger array. Returns 0 on error.
 */
ZINC_API int cmzn_field_image_get_size_in_pixels(cmzn_field_image_id image,
	int valuesCount, int *valuesOut);

/**
 * Get the physical width of the image.
 *
 * @param image   handle to the image field.
 * @return physical width of the image
 */
ZINC_API double cmzn_field_image_get_texture_coordinate_width(cmzn_field_image_id image);

/**
 * Get the physical height of the image.
 *
 * @param image   handle to the image field.
 * @return physical height of the image
 */
ZINC_API double cmzn_field_image_get_texture_coordinate_height(cmzn_field_image_id image);

/**
 * Get the physical depth of the image.
 *
 * @param image   handle to the image field.
 * @return physical depth of the image
 */
ZINC_API double cmzn_field_image_get_texture_coordinate_depth(cmzn_field_image_id image);

/**
 * Gets the texture coordinate size to be used for this image field.
 *
 * @see cmzn_field_iamge_set_texture coordinate_sizes
 * @param image  The image to query.
 * @param valuesCount  The size of the sizes array to fill. Values
 * for dimensions beyond the size have the value of 1.
 * @param valuesOut  Array to receive texture coordinate sizes.
 * @return  The actual dimension of texture coordinates that have been
 * explicitly set. This can be
 * more than the number requested, so a second call may be needed with a
 * larger array. Returns 0 on error.
 */
ZINC_API int cmzn_field_image_get_texture_coordinate_sizes(cmzn_field_image_id image,
	int valuesCount, double *valuesOut);

/**
 * Set the texture coordinate width of the image.
 *
 * @param image  handle to the image field.
 * @param width  physical width to be set for the image
 *
 * @return CMISS_OK if set correctly, any other value otherwise.
 */
ZINC_API int cmzn_field_image_set_texture_coordinate_width(cmzn_field_image_id image, double width);

/**
 * Set the texture coordinate height of the image.
 *
 * @param image  handle to the image field.
 * @param height  physical height to be set for the image
 *
 * @return CMISS_OK if set correctly, any other value otherwise.
 */
ZINC_API int cmzn_field_image_set_texture_coordinate_height(cmzn_field_image_id image, double height);

/**
 * Set the texture coordinate depth of the image.
 *
 * @param image   handle to the image field.
 * @param depth  physical depth to be set for the image
 *
 * @return CMISS_OK if set correctly, any other value otherwise.
 */
ZINC_API int cmzn_field_image_set_texture_coordinate_depth(cmzn_field_image_id image, double depth);

/**
 * Sets the texture coordinate size to be used for this image field.
 *
 * @see cmzn_field_iamge_set_texture coordinate_sizes
 * @param image  The image to query.
 * @param valuesCount  The size of the sizes array to fill. Values
 * for dimensions beyond the size have the value of 1.
 * @param sizesOut  Array to receive texture coordinate sizes.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_field_image_set_texture_coordinate_sizes(cmzn_field_image_id image,
	int valuesCount, const double *valuesIn);

/**
 * Reads image data into the field.
 * The streaminformation may specify a filename, series of filenames or
 * a memory block reference to read from.
 * If the format specified in the streaminformation
 * is a "raw" format (such as rgb or gray) which does not embed
 * information about the pixel storage then the data size is expected to be
 * supplied in the streaminformation parameter.
 *
 * @param image_field The image field.
 * @param streaminformationImage Derived information about the supplied
 * formatted image data. At a minimum it should specify either a filename
 * or a memory block reference.
 * @return  Status CMZN_OK if the operation is successful, any other value on
 * failure.
 */
ZINC_API int cmzn_field_image_read(cmzn_field_image_id image_field,
	cmzn_streaminformation_image_id streaminformationImage);

/**
 * Convenient function to read a file with the provided name into a field image
 * directly.
 *
 * @param image_field  The field image to be read into.
 * @param file_name  name of the file to read from.
 *
 * @return  Status CMZN_OK if data successfully read and merged into specified
 * region, any other value on failure.
 */
ZINC_API int cmzn_field_image_read_file(cmzn_field_image_id image_field, const char *file_name);

/**
 * Writes a formatted representation of the image data.
 * The streaminformation is used to control the formatted output.
 * If a memory block reference has been specified to the io_stream
 * then this will be allocated and set and the corresponding memory block
 * length set.
 * Otherwise the routine will try to write to the filename set on the
 * storage information.
 * The routine should fail if the values specified in the streaminformation
 * cannot be respected.
 * If one or two of the size parameters are set on the streaminformation
 * then other dimensions will be adjusted to maintain aspect ratio and then the image is
 * resized just for this output.
 *
 * @param image_field The image field.
 * @param streaminformation  Information specifying the required format
 * for the returned formatted image data.
 * @return  Status CMZN_OK if the operation is successful, any other value
 * on failure.
 */
ZINC_API int cmzn_field_image_write(cmzn_field_image_id image_field,
	cmzn_streaminformation_image_id streaminformation_image);

/**
 * Convenient function to write the image into a file with the provided name.
 *
 * @param image_field  The image_field which stores the image.
 * @param file_name  name of the file to write to..
 *
 * @return  Status CMZN_OK if data is successfully written out, any other value
 * on failure.
 */
ZINC_API int cmzn_field_image_write_file(cmzn_field_image_id image_field,
	const char *file_name);

/**
 * Returns how the image is combined with the material: blend, decal or modulate.
 *
 * @param  image_field  The image field.
 * @return  Returns enum describing how the image is combined with the material.
 */
ZINC_API enum cmzn_field_image_combine_mode cmzn_field_image_get_combine_mode(
   cmzn_field_image_id image_field);

/**
 * Sets how the image is combined with the material: blend, decal or modulate.
 *
 * @param image_field  The image field.
 * @param combine_mode  Enumerator describing how the image is combined with the
 * 		material.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_image_set_combine_mode(cmzn_field_image_id image_field,
   enum cmzn_field_image_combine_mode combine_mode);

/**
 * Gets the domain field on which texture coordinates must be specified to
 * evaluate the image.
 * @see cmzn_fieldcache_set_field_real
 *
 * @param image_field  The image field to query.
 * @return  The domain field, or 0 if error. Up to caller to destroy handle.
 */
ZINC_API cmzn_field_id cmzn_field_image_get_domain_field(
	cmzn_field_image_id image_field);

/**
 * Sets the domain field on which texture coordinates must be specified to
 * evaluate the image.
 * @see cmzn_fieldcache_set_field_real
 *
 * @param image_field  The image field to modify.
 * @param domain_field  The new domain field. Must be real-valued with at least
 * as many components as the image dimension.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_field_image_set_domain_field(
	cmzn_field_image_id image_field, cmzn_field_id domain_field);

/**
 * Returns how the image is stored in graphics memory.
 *
 * @param image_field  The image field.
 * @return  Returns enum describing how the image is stored in graphics memory.
 */
ZINC_API enum cmzn_field_image_hardware_compression_mode cmzn_field_image_get_hardware_compression_mode(
   cmzn_field_image_id image_field);

/**
 * Indicate to the graphics hardware how you would like the texture stored in
 * graphics memory.
 *
 * @param image_field  The image field.
 * @param compression_mode  Enumerator describing how the image is combined with the
 * 		material.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_image_set_hardware_compression_mode(cmzn_field_image_id image_field,
   enum cmzn_field_image_hardware_compression_mode compression_mode);

/**
 * Returns how the image is rasterised onto the screen.
 *
 * @param image_field  The image field.
 * @return  Returns enum describing how the image is rasterised onto the screen.
 */
ZINC_API enum cmzn_field_image_filter_mode cmzn_field_image_get_filter_mode(
   cmzn_field_image_id image_field);

/**
 * Indicate to the graphics hardware how you would like the image rasterised
 * onto the screen.
 *
 * @param image_field  The image field.
 * @param filter_mode  Enumerator describing how the graphics hardware rasterises
 *   the texture onto the screen.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_image_set_filter_mode(cmzn_field_image_id image_field,
   enum cmzn_field_image_filter_mode filter_mode);

/**
 * Returns the mode describing how the image is wrapped outside the range [0,1].
 *
 * @param image_field  The image field.
 * @return  Returns enum describing how the image is wrapped.
 */
ZINC_API enum cmzn_field_image_wrap_mode cmzn_field_image_get_wrap_mode(
   cmzn_field_image_id image_field);

/**
 * Set the mode describing how the image is wrapped outside the range [0,1].
 *
 * @param image_field  The image field.
 * @param filter_mode  Enumerator describing how image is wrapped outside
 * 	the range [0,1].
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_image_set_wrap_mode(cmzn_field_image_id image_field,
   enum cmzn_field_image_wrap_mode filter_mode);

/**
 * Gets the property named in the given property string from the given field image.
 * The returned string must be deallocated by the receiver.
 *
 * @param	image   	The image field.
 * @param	property	The property to retrieve.
 *
 * @return	null if it fails, else an allocated string containing the properties value.
 */
ZINC_API char *cmzn_field_image_get_property(cmzn_field_image_id image,
	const char* property);

/**
 * Convert a short attribute name into an enum if the attribute name matches
 * any of the members in the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum cmzn_streaminformation_image_attribute
	cmzn_streaminformation_image_attribute_enum_from_string(const char *string);

/**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call cmzn_deallocate to destroy the successfully returned string.
 *
 * @param attribute  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *cmzn_streaminformation_image_attribute_enum_to_string(
	enum cmzn_streaminformation_image_attribute attribute);

/**
 * Creates a cmzn_streaminformation_image object.
 * @return The created object.
 */
ZINC_API cmzn_streaminformation_id cmzn_field_image_create_streaminformation(
	cmzn_field_image_id image_field);

/**
 * Destroys a cmzn_streaminformation_image object.
 *
 * @param streaminformation_address  Pointer to a streaminformation object, which
 * is destroyed and the pointer is set to NULL.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_streaminformation_image_destroy(
	cmzn_streaminformation_image_id *streaminformation_address);

/**
 * If the streaminformation is of field_image type, then this function returns
 * the field_image specific representation, otherwise it returns NULL.
 * Caller is responsible for destroying the returned derived reference.
 *
 * @param streaminformation  The generic streaminformation to be cast.
 * @return  field_image specific representation if the input streaminformation is
 * of this type, otherwise NULL.
 */
ZINC_API cmzn_streaminformation_image_id cmzn_streaminformation_cast_image(
	cmzn_streaminformation_id streaminformation);

/**
 * Cast streaminformation_image back to its base streaminformation and
 * return the streaminformation.
 * IMPORTANT NOTE: Returned streaminformation does not have incremented
 * reference count and must not be destroyed. Use cmzn_streaminformation_access()
 * to add a reference if maintaining returned handle beyond the lifetime of the
 * streaminformation_image argument.
 *
 * @param streaminformation_image  Handle to the streaminformation_image to cast.
 * @return  Non-accessed handle to the base stream information or NULL if failed.
 */
ZINC_C_INLINE cmzn_streaminformation_id
	cmzn_streaminformation_image_base_cast(
		cmzn_streaminformation_image_id streaminformation_image)
{
	return (cmzn_streaminformation_id)(streaminformation_image);
}

/**
 * Set an integer or Boolean attribute of the streaminformation_image.
 *
 * @param streaminformation  Handle to the zinc streaminformation_image.
 * @param attribute  The identifier of the integer attribute to set.
 * @param value  The new value for the attribute. For Boolean values use 1 for
 * true.
 *
 * @return  Status CMZN_OK if attribute successfully set, any other value if
 *	failed or attribute not valid or unable to be set for this
 * 	streaminformation_image.
 */
ZINC_API int cmzn_streaminformation_image_set_attribute_integer(
	cmzn_streaminformation_image_id streaminformation,
	enum cmzn_streaminformation_image_attribute attribute, int value);

/**
 * Set an double attribute of the streaminformation_image.
 *
 * @param streaminformation  Handle to the zinc streaminformation_image.
 * @param attribute  The identifier of the double attribute to set.
 * @param value  The new value for the attribute.
 * @return   Status CMZN_OK if attribute successfully set, any other value if
 * failed or attribute not valid or unable to be set for this
 * streaminformation_image.
 */
ZINC_API int cmzn_streaminformation_image_set_attribute_real(
	cmzn_streaminformation_image_id streaminformation,
	enum cmzn_streaminformation_image_attribute attribute,
	double value);

/**
 * Specifies the format for binary data to be read/written using this
 * streaminformation.
 *
 * @param streaminformation  The storage information object.
 * @param file_format  The image file format.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_streaminformation_image_set_file_format(
	cmzn_streaminformation_image_id streaminformation,
	enum cmzn_streaminformation_image_file_format file_format);

/**
 * Specifies the pixel format for binary data of the images  to be read/written
 * using this streaminformation.
 *
 * @param streaminformation  The storage information object.
 * @param pixel_format  The pixel_format of the formatted data.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_streaminformation_image_set_pixel_format(
	cmzn_streaminformation_image_id streaminformation,
	enum cmzn_streaminformation_image_pixel_format pixel_format);

#ifdef __cplusplus
}
#endif

#endif
