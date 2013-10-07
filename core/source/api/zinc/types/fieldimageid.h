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
 * #see cmzn_streaminformation_id
 * #see cmzn_field_image_create_streaminformation
 * #see cmzn_streaminformation_cast_image
 * #see cmzn_streaminformation_image_base_cast
 */
struct cmzn_streaminformation_image;
typedef struct cmzn_streaminformation_image *cmzn_streaminformation_image_id;

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

#endif
