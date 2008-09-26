/*******************************************************************************
FILE : cmiss_field_image_processing.h

LAST MODIFIED : 26 Septemeber 2008

DESCRIPTION :
Implements cmiss fields which deal with image processing
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
#if !defined (CMISS_FIELD_IMAGE_PROCESSING_H)
#define CMISS_FIELD_IMAGE_PROCESSING_H

#include "api/cmiss_field.h"

/*****************************************************************************//**
 * The image field specific handle to a Cmiss binary threshold field.
 */
struct Cmiss_field_binary_threshold_image_filter;

typedef struct Cmiss_field_binary_threshold_image_filter * Cmiss_field_binary_threshold_image_filter_id;

/*****************************************************************************//**
 * Creates a field of type COMPUTED_FIELD_BINARY_THRESHOLD_IMAGE_FILTER.
 * The newly created field consists of binary values (either 0 or 1) which are
 * determined by applying the threshold range to the source field. 
 * Input values with an intensity range between lower_threshold and the 
 * upper_threshold are set to 1, the rest are set to 0.
 * 
 * @param source_field The field to be filtered
 * @param lower_threshold Threshold value below which all pixels are set to 0 
 * @param upper_threshold Theshold value above which all values are set to 0
 * @return Newly created field
*/
Cmiss_field_id Cmiss_field_create_binary_threshold_image_filter(Cmiss_field_id source_field,
	double lower_threshold, double upper_threshold);

/*****************************************************************************//**
 * If field can be cast to a COMPUTED_FIELD_BINARY_THRESHOLD_IMAGE_FILTER do so
 * and return the field.  Otherwise return NULL.
 * 
 * @param field Id of the field to cast
 * @return Id of the cast field, or NULL
*/
Cmiss_field_binary_threshold_image_filter_id Cmiss_field_binary_threshold_image_filter_cast(Cmiss_field_id field);


/*****************************************************************************//**
 * The image field specific handle to a Cmiss discrete gaussian field.
 */
struct Cmiss_field_discrete_gaussian_image_filter;

typedef struct Cmiss_field_discrete_gaussian_image_filter * Cmiss_field_discrete_gaussian_image_filter_id;

/*****************************************************************************//**
 * Creates a field of type COMPUTED_FIELD_DISCRETE_GAUSSIAN_IMAGE_FILTER.
 * The newly created field applies a discrete gaussian image filter to the 
 * source field.  This means that each pixel value in the new field
 * is based on a weighted average of the pixel and the surrounding pixel values
 * from the source field. Pixels further away are given a lower weighting.
 * Increasing the variance increases the width of the gaussian distribution 
 * used and hence the number of pixels used to calculate the weighted average. 
 * This smooths the image more.  A limit is set on the max_kernel_width used 
 * to approximate the guassian to ensure the calculation completes.
 * 
 * @param source_field The field to be filtered
 * @param variance The variance of the gaussian distribution used in the filter
 * @param max_kernel_width The limit on the maximum kernel width that may be used
 * @return Newly created field
*/
Cmiss_field_id Cmiss_field_create_discrete_gaussian_image_filter(
	struct Computed_field *source_field, double variance, int maxKernelWidth);

/*****************************************************************************//**
 * If field can be cast to a COMPUTED_FIELD_DISCRETE_GAUSSIAN_IMAGE_FILTER do so
 * and return the field.  Otherwise return NULL.
 * 
 * @param field Id of the field to cast
 * @return Id of the cast field, or NULL
*/
Cmiss_field_discrete_gaussian_image_filter_id Cmiss_field_discrete_gaussian_image_filter_cast(Cmiss_field_id field);



enum General_threshold_filter_mode
{
	BELOW,
	ABOVE,
	OUTSIDE
}; /* enum General_threshold_filter_mode */

/*****************************************************************************//**
 * The image field specific handle to a Cmiss threshold field.
 */
struct Cmiss_field_threshold_image_filter;

typedef struct Cmiss_field_threshold_image_filter * Cmiss_field_threshold_image_filter_id;

/*****************************************************************************//**
 * Creates a field of type COMPUTED_FIELD_THRESHOLD_IMAGE_FILTER.
 * The newly created field replaces certain values with a specified outside
 * value, based on which threshold mode and the threshold values.
 * For the below mode, all pixels BELOW the below value are set to 
 * the outside value
 * For the above mode, all pixels ABOVE the above value are set to a
 * outside value
 * For the oustide mode, all pixels OUTSIDE the range defined by the 
 * below and above values are set to the outside value
 * 
 * @param source_field The field to be filtered
 * @param threshold_mode The threshold mode to apply, either BELOW, ABOVE or OUTSIDE
 * @param outside_value The value to replace all thresholded values with
 * @param below_value Below value used by BELOW and OUTSIDE modes
 * @param above_value Above value used by ABOVE and OUTSIDE modes
 * @return Newly created field
*/
Cmiss_field_id Cmiss_field_create_threshold_image_filter(
	struct Computed_field *source_field, 
	enum General_threshold_filter_mode threshold_mode, 
	double outside_value, double below_value, double above_value);

/*****************************************************************************//**
 * If field can be cast to a COMPUTED_FIELD_THRESHOLD_IMAGE_FILTER do so
 * and return the field.  Otherwise return NULL.
 * 
 * @param field Id of the field to cast
 * @return Id of the cast field, or NULL
*/
Cmiss_field_threshold_image_filter_id Cmiss_field_threshold_image_filter_cast(Cmiss_field_id field);


#endif /* !defined (CMISS_FIELD_IMAGE_PROCESSING_H) */
