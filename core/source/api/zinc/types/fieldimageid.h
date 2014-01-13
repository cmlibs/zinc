/**
 * FILE : fieldimageid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_FIELDIMAGEID_H__
#define CMZN_FIELDIMAGEID_H__

/**
 * The image field specific handle to a image cmzn_field.
 */
struct cmzn_field_image;
typedef struct cmzn_field_image *cmzn_field_image_id;

/**
 * A handle to zinc stream information image. Stream information image is a
 * derived type of cmzn_streaminformation_id.
 * User can create and get a handle to stream information image with functions
 * provided with cmzn_field_image.
 * User can use this derived type to set number of informations associate with
 * images inputs and outputs. See fieldimage.h for more information.
 *
 * #see cmzn_field_image_create_streaminformation_image
 * #see cmzn_streaminformation_cast_image
 */
struct cmzn_streaminformation_image;
typedef struct cmzn_streaminformation_image *cmzn_streaminformation_image_id;

/**
 * Describes the blending of the texture with the texture constant colour and
 * the underlying fragment colour
 */
enum cmzn_field_image_combine_mode
{
	CMZN_FIELD_IMAGE_COMBINE_MODE_INVALID = 0,
	CMZN_FIELD_IMAGE_COMBINE_MODE_BLEND = 1,
		/*!< default combine mode */
	CMZN_FIELD_IMAGE_COMBINE_MODE_DECAL = 2,
	CMZN_FIELD_IMAGE_COMBINE_MODE_MODULATE = 3,
	CMZN_FIELD_IMAGE_COMBINE_MODE_ADD = 4,
	CMZN_FIELD_IMAGE_COMBINE_MODE_ADD_SIGNED = 5,
		/*!< Add the value and subtract 0.5 so the texture value
			 effectively ranges from -0.5 to 0.5 */
	CMZN_FIELD_IMAGE_COMBINE_MODE_MODULATE_SCALE_4 = 6,
		/*!< Multiply and then scale by 4, so that we can
			 scale down or up */
	CMZN_FIELD_IMAGE_COMBINE_MODE_BLEND_SCALE_4 = 7,
		/*!< Same as blend with a 4 * scaling */
	CMZN_FIELD_IMAGE_COMBINE_MODE_SUBTRACT = 8,
	CMZN_FIELD_IMAGE_COMBINE_MODE_ADD_SCALE_4 = 9,
	CMZN_FIELD_IMAGE_COMBINE_MODE_SUBTRACT_SCALE_4 = 10,
	CMZN_FIELD_IMAGE_COMBINE_MODE_INVERT_ADD_SCALE_4 = 11,
	CMZN_FIELD_IMAGE_COMBINE_MODE_INVERT_SUBTRACT_SCALE_4 = 12
};

/**
 * Specfiy how the graphics hardware rasterises the texture onto the screen.
 */
enum cmzn_field_image_filter_mode
{
	CMZN_FIELD_IMAGE_FILTER_MODE_INVALID = 0,
	CMZN_FIELD_IMAGE_FILTER_MODE_NEAREST = 1,
		/*!< default combine mode */
	CMZN_FIELD_IMAGE_FILTER_MODE_LINEAR = 2,
	CMZN_FIELD_IMAGE_FILTER_MODE_NEAREST_MIPMAP_NEAREST = 3,
	CMZN_FIELD_IMAGE_FILTER_MODE_LINEAR_MIPMAP_NEAREST = 4,
	CMZN_FIELD_IMAGE_FILTER_MODE_LINEAR_MIPMAP_LINEAR = 5
};

/**
 * Whether the texture is compressed.  Could add specific compression formats that
 * are explictly requested from the hardware.
 */
enum cmzn_field_image_hardware_compression_mode
{
	CMZN_FIELD_IMAGE_HARDWARE_COMPRESSION_MODE_INVALID = 0,
	CMZN_FIELD_IMAGE_HARDWARE_COMPRESSION_MODE_UNCOMPRESSED = 1,
		/* default hardware compression mode */
	CMZN_FIELD_IMAGE_HARDWARE_COMPRESSION_MODE_AUTOMATIC = 2
		/*!< Allow the hardware to choose the compression */
};

/**
 * Describes how the image is to be wrapped when texture coordinate is assigned
 * outside the range [0,1], you can choose to have them clamp or repeat.
 */
enum cmzn_field_image_wrap_mode
{
	CMZN_FIELD_IMAGE_WRAP_MODE_INVALID = 0,
	CMZN_FIELD_IMAGE_WRAP_MODE_CLAMP = 1,
		/*!< Clamp to a blend of the pixel edge and border colour */
	CMZN_FIELD_IMAGE_WRAP_MODE_REPEAT = 2,
		/*!< Default wrap mode. Repeat texture cylically in multiples of the
			 texture coordinate range */
	CMZN_FIELD_IMAGE_WRAP_MODE_EDGE_CLAMP = 3,
		/*!< Always ignore the border, texels at or near the edge of the texure are
			 used for texturing */
	CMZN_FIELD_IMAGE_WRAP_MODE_BORDER_CLAMP = 4,
		/*!< Clamp to the border colour when outside the texture coordinate range. */
	CMZN_FIELD_IMAGE_WRAP_MODE_MIRROR_REPEAT = 5
		/*!< Repeat but mirror every second multiple of the texture coordinates
			 range. Texture may appear up-right in coordinate range[0,1] but
			 upside-down in coordinate range[1,2] */
};

/**
 * Describes the format for storage.
 * Whether a particular format is actually available depends on whether
 * it is compatible with a particular format type when used with
 * #cmzn_field_image_get_formatted_image_data and whether support for that combination
 * has been included when the program was built.
 * This is a small subset of formats available, more can be selected by specifying
 * the appropriate format_string for a cmzn_streaminformation_image.
 */
enum cmzn_streaminformation_image_file_format
{
	CMZN_STREAMINFORMATION_IMAGE_FILE_FORMAT_INVALID = 0,
	CMZN_STREAMINFORMATION_IMAGE_FILE_FORMAT_BMP = 1,
	CMZN_STREAMINFORMATION_IMAGE_FILE_FORMAT_DICOM = 2,
	CMZN_STREAMINFORMATION_IMAGE_FILE_FORMAT_JPG = 3,
	CMZN_STREAMINFORMATION_IMAGE_FILE_FORMAT_GIF = 4,
	CMZN_STREAMINFORMATION_IMAGE_FILE_FORMAT_PNG = 5,
	CMZN_STREAMINFORMATION_IMAGE_FILE_FORMAT_SGI = 6,
	CMZN_STREAMINFORMATION_IMAGE_FILE_FORMAT_TIFF = 7,
	CMZN_STREAMINFORMATION_IMAGE_FILE_FORMAT_ANALYZE = 8
};

/**
 * Optional information used to describe the binary data supplied to the images.
 */
enum cmzn_streaminformation_image_pixel_format
{
	CMZN_STREAMINFORMATION_IMAGE_PIXEL_FORMAT_INVALID = 0,
	CMZN_STREAMINFORMATION_IMAGE_PIXEL_FORMAT_LUMINANCE = 1,
	CMZN_STREAMINFORMATION_IMAGE_PIXEL_FORMAT_LUMINANCE_ALPHA = 2,
	CMZN_STREAMINFORMATION_IMAGE_PIXEL_FORMAT_RGB = 3,
	CMZN_STREAMINFORMATION_IMAGE_PIXEL_FORMAT_RGBA = 4,
	CMZN_STREAMINFORMATION_IMAGE_PIXEL_FORMAT_ABGR = 5,
	CMZN_STREAMINFORMATION_IMAGE_PIXEL_FORMAT_BGR = 6
};


enum cmzn_streaminformation_image_attribute
{
	CMZN_STREAMINFORMATION_IMAGE_ATTRIBUTE_RAW_WIDTH_PIXELS = 1,
	/*!< Integer specifies the pixel width for binary data reading in using this
	 * stream information.
	 */
	CMZN_STREAMINFORMATION_IMAGE_ATTRIBUTE_RAW_HEIGHT_PIXELS = 2,
	/*!< Integer specifies the pixel height for binary data reading in using this
	 * stream information.
	 */
	CMZN_STREAMINFORMATION_IMAGE_ATTRIBUTE_BITS_PER_COMPONENT = 3,
	/*!< Integer specifies the number of bytes per component for binary data using
	 * this stream information. Only 8 and 16 bits are supported at the moment.
	 */
	CMZN_STREAMINFORMATION_IMAGE_ATTRIBUTE_COMPRESSION_QUALITY = 4
	/*!< Real number specifies the quality for binary data using this stream information.
	 * This parameter controls compression for compressed lossy formats,
	 * where a quality of 1.0 specifies the least lossy output for a given format and a
	 * quality of 0.0 specifies the most compression.
	 */
};

#endif
