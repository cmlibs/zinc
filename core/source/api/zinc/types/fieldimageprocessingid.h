/***************************************************************************//**
 * FILE : fieldimageprocessingid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_FIELDIMAGEPROCESSINGID_H__
#define CMZN_FIELDIMAGEPROCESSINGID_H__

/*****************************************************************************//**
 * The image field specific handle to a zinc binary threshold field.
 */
struct cmzn_field_binary_threshold_image_filter;
typedef struct cmzn_field_binary_threshold_image_filter * cmzn_field_binary_threshold_image_filter_id;

/*****************************************************************************//**
 * The image field specific handle to a zinc discrete gaussian field.
 */
struct cmzn_field_discrete_gaussian_image_filter;
typedef struct cmzn_field_discrete_gaussian_image_filter * cmzn_field_discrete_gaussian_image_filter_id;

/*****************************************************************************//**
 * The image field specific handle to a zinc threshold field.
 */
struct cmzn_field_threshold_image_filter;
typedef struct cmzn_field_threshold_image_filter * cmzn_field_threshold_image_filter_id;

#endif
