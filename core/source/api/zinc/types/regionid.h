/**
 * FILE : regionid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_REGIONID_H__
#define CMZN_REGIONID_H__

struct cmzn_region;
typedef struct cmzn_region * cmzn_region_id;

/**
 * A handle to zinc stream information region. Stream information region is a
 * derived type of cmzn_streaminformation_id.
 * User can create and get a handle to stream information region with functions
 * provided with cmzn_region.
 * User can use this derived type to set number of informations associate with
 * images inputs and outputs. See region.h for more information.
 *
 * #see cmzn_streaminformation_id
 * #see cmzn_field_image_create_streaminformation
 * #see cmzn_streaminformation_cast_image
 * #see cmzn_streaminformation_image_base_cast
 */
struct cmzn_streaminformation_region;
typedef struct cmzn_streaminformation_region * cmzn_streaminformation_region_id;

enum cmzn_streaminformation_region_attribute
{
	CMZN_STREAMINFORMATION_REGION_ATTRIBUTE_INVALID = 0,
	CMZN_STREAMINFORMATION_REGION_ATTRIBUTE_TIME = 1
};

#endif /* CMZN_REGION_ID_H */
