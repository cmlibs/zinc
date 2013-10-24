/*******************************************************************************
FILE : fieldimageprocessing.h

LAST MODIFIED : 26 Septemeber 2008

DESCRIPTION :
Implements zinc fields which deal with image processing
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDIMAGEPROCESSING_H__
#define CMZN_FIELDIMAGEPROCESSING_H__

#include "types/fieldid.h"
#include "types/fieldimageprocessingid.h"
#include "types/fieldmoduleid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * Creates a field performing ITK binary dilate image filter on scalar source
 * field image. Sets number of components to same number as <source_field>.
 * The <radius> and <dilate_value> specify the radius of pixels to use
 * for dilation and what pixel value to use for dilation
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_imagefilter_binary_dilate(
	cmzn_fieldmodule_id field_module, cmzn_field_id source_field,
	int radius, double dilate_value);

/***************************************************************************//**
 * Creates a field performing ITK binary erode image filter on scalar source
 * field image. Sets number of components to same number as <source_field>.
 * The <radius> and <erode_value> specify the radius of pixels to use
 * for dilation and what pixel value to use for dilation
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_imagefilter_binary_erode(
		cmzn_fieldmodule_id field_module, cmzn_field_id source_field,
		int radius, double erode_value);

/**
 * Creates a field which applies an ITK binary threshold image filter on source.
 * The newly created field consists of binary values (either 0 or 1) which are
 * determined by applying the threshold range to the source field.
 * Input values with an intensity range between lower_threshold and the
 * upper_threshold are set to 1, the rest are set to 0.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  The field to be filtered
 * @return  Newly created field
*/
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_imagefilter_binary_threshold(
	cmzn_fieldmodule_id field_module, cmzn_field_id source_field);

/**
 * If field can be cast to a cmzn_field_imagefilter_binary_threshold_id do so
 * and return the field.  Otherwise return NULL.
 * Caller is responsible for destroying the new image filter reference.
 *
 * @param field  Id of the field to cast
 * @return  Id of the cast field, or NULL
*/
ZINC_API cmzn_field_imagefilter_binary_threshold_id
	cmzn_field_cast_imagefilter_binary_threshold(cmzn_field_id field);

/**
 *  Get the lower threshold value for this image filter.
 *
 * @param imagefilter_binary_threshold  handle of the binary threshold image filter.
 * @return  the lower threshold set for this filter.
 */
ZINC_API double cmzn_field_imagefilter_binary_threshold_get_lower_threshold(
	cmzn_field_imagefilter_binary_threshold_id imagefilter_binary_threshold);

/**
 *  Set the lower threshold value for this image filter.
 *
 * @param imagefilter_binary_threshold  handle of the binary threshold image filter.
 * @param lower_threshold  Threshold value below which all pixels are set to 0
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_imagefilter_binary_threshold_set_lower_threshold(
	cmzn_field_imagefilter_binary_threshold_id imagefilter_binary_threshold,
	double lower_threshold);

/**
 *  Get the upper threshold value for this image filter.
 *
 * @param imagefilter_binary_threshold  handle of the binary threshold image filter.
 * @return  the upper threshold set for this filter.
 */
ZINC_API double cmzn_field_imagefilter_binary_threshold_get_upper_threshold(
	cmzn_field_imagefilter_binary_threshold_id imagefilter_binary_threshold);

/**
 *  Set the upper threshold value for this image filter.
 *
 * @param imagefilter_binary_threshold  handle of the binary threshold image filter.
 * @param upper_threshold  Threshold value above which all pixels are set to 0
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_imagefilter_binary_threshold_set_upper_threshold(
	cmzn_field_imagefilter_binary_threshold_id imagefilter_binary_threshold,
	double upper_threshold);

/**
 * Cast find_mesh_location field back to its base field and return the field.
 * IMPORTANT NOTE: Returned field does not have incremented reference count and
 * must not be destroyed. Use cmzn_field_access() to add a reference if
 * maintaining returned handle beyond the lifetime of the derived field.
 * Use this function to call base-class API, e.g.:
 * cmzn_field_set_name(cmzn_field_derived_base_cast(derived_field), "bob");
 *
 * @param imagefilter_binary_threshold  Handle to the imagefilter_binary_threshold field to cast.
 * @return  Non-accessed handle to the base field or NULL if failed.
 */
ZINC_C_INLINE cmzn_field_id cmzn_field_imagefilter_binary_threshold_base_cast(
	cmzn_field_imagefilter_binary_threshold_id imagefilter_binary_threshold)
{
	return (cmzn_field_id)(imagefilter_binary_threshold);
}

/**
 * Destroys this reference to the imagefilter_binary_threshold field and sets it
 * to NULL. Internally this just decrements the reference count.
 *
 * @param imagefilter_binary_threshold_address  Address of handle to the field to
 * 		destroy.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_imagefilter_binary_threshold_destroy(
	cmzn_field_imagefilter_binary_threshold_id *imagefilter_binary_threshold_address);

/***************************************************************************//**
 * Creates a field returning result of ITK canny edge detection filter on the
 * source field image. Sets number of components to same number as source field.
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_imagefilter_canny_edge_detection(
	cmzn_fieldmodule_id field_module, cmzn_field_id source_field,
	double variance, double maximumError,
  double upperThreshold, double lowerThreshold);

/**
 * Creates a field performing ITK connected threshold image filter on scalar
 * source field image. Sets number of components to same number as source field.
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_imagefilter_connected_threshold(
	cmzn_fieldmodule_id field_module, cmzn_field_id source_field,
	double lower_threshold, double upper_threshold, double replace_value,
	int num_seed_points, int dimension, const double *seed_points);

/**
 * Creates a field performing ITK curvature anisotropic diffusion image filter
 * on scalar source field image.
 * Sets number of components to same number as <source_field>.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  The field to be filtered
 * @param timeStep  The time step
 * @param conductance  The conductance
 * @param numIterations  The number of iterations to be performed
 * @return  Newly created field
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_imagefilter_curvature_anisotropic_diffusion(
	cmzn_fieldmodule_id field_module, cmzn_field_id source_field,
	double timeStep, double conductance, int numIterations);

/**
 * Creates a field applying the ITK discrete gaussian image filter to the source
 * field. This means that each pixel value in the new field
 * is based on a weighted average of the pixel and the surrounding pixel values
 * from the source field. Pixels further away are given a lower weighting.
 * Increasing the variance increases the width of the gaussian distribution
 * used and hence the number of pixels used to calculate the weighted average.
 * This smooths the image more.  A limit is set on the max_kernel_width used
 * to approximate the guassian to ensure the calculation completes.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  The field to be filtered
 * @return  Newly created field
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_imagefilter_discrete_gaussian(
	cmzn_fieldmodule_id field_module, cmzn_field_id source_field);

/**
 * If field can be cast to a cmzn_field_imagefilter_discrete_gaussian_id do so
 * and return the field.  Otherwise return NULL.
 * Caller is responsible for destroying the new image filter reference.
 *
 * @param field Id of the field to cast
 * @return Id of the cast field, or NULL
*/
ZINC_API cmzn_field_imagefilter_discrete_gaussian_id
	cmzn_field_cast_imagefilter_discrete_gaussian(cmzn_field_id field);

/**
 *  Get the variance value for this image filter.
 *
 * @param imagefilter_threshold  handle of the threshold image filter.
 * @return  the current variance value set on this filter.
 */
ZINC_API double cmzn_field_imagefilter_discrete_gaussian_get_variance(
	cmzn_field_imagefilter_discrete_gaussian_id imagefilter_discrete_gaussian);

/**
 *  Set the variance for this image filter.
 *
 * Increasing the variance increases the width of the gaussian distribution
 * used and hence the number of pixels used to calculate the weighted average.
 * This smooths the image more.
 *
 * @param imagefilter_discrete_gaussian  handle of the discrete gaussian image filter.
 * @param variance  The variance of the gaussian distribution used in the filter
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_imagefilter_discrete_gaussian_set_variance(
	cmzn_field_imagefilter_discrete_gaussian_id imagefilter_discrete_gaussian,
	double variance);

/**
 *  Get the max kernel width for this image filter.
 *
 * @param imagefilter_threshold  handle of the threshold image filter.
 * @return  the current variance value set on this filter.
 */
ZINC_API int cmzn_field_imagefilter_discrete_gaussian_get_max_kernel_width(
	cmzn_field_imagefilter_discrete_gaussian_id imagefilter_discrete_gaussian);

/**
 *  Set the max kernel width for this image filter.
 *
 * The max kernel width is a limit is set on the filter
 * to approximate the guassian to ensure the calculation completes.
 *
 * @param imagefilter_discrete_gaussian  handle of the discrete gaussian image filter.
 * @param max_kernel_width  The limit on the maximum kernel width that may be used
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_imagefilter_discrete_gaussian_set_max_kernel_width(
	cmzn_field_imagefilter_discrete_gaussian_id imagefilter_discrete_gaussian,
	int max_kernel_width);

/**
 * Cast imagefilter_discrete_gaussian field back to its base field and return the field.
 * IMPORTANT NOTE: Returned field does not have incremented reference count and
 * must not be destroyed. Use cmzn_field_access() to add a reference if
 * maintaining returned handle beyond the lifetime of the derived field.
 * Use this function to call base-class API, e.g.:
 * cmzn_field_set_name(cmzn_field_derived_base_cast(derived_field), "bob");
 *
 * @param imagefilter_discrete_gaussian  Handle to the imagefilter_discrete_gaussian field to cast.
 * @return  Non-accessed handle to the base field or NULL if failed.
 */
ZINC_C_INLINE cmzn_field_id cmzn_field_imagefilter_discrete_gaussian_base_cast(
	cmzn_field_imagefilter_discrete_gaussian_id imagefilter_discrete_gaussian)
{
	return (cmzn_field_id)(imagefilter_discrete_gaussian);
}

/**
 * Destroys this reference to the imagefilter_discrete_gaussian field and sets it
 * to NULL. Internally this just decrements the reference count.
 *
 * @param imagefilter_discrete_gaussian_address  Address of handle to the field to
 * 		destroy.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_imagefilter_discrete_gaussian_destroy(
		cmzn_field_imagefilter_discrete_gaussian_id *imagefilter_discrete_gaussian_address);

/***************************************************************************//**
 * Creates a field performing ITK fast marching image filter on scalar source field
 * image. Sets number of components to same number as <source_field>.
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_imagefilter_fast_marching(
	cmzn_fieldmodule_id field_module, cmzn_field_id source_field,
	double stopping_value, int num_seed_points, int dimension,
	const double *seed_points, const double *seed_values, const int *output_size);

/***************************************************************************//**
 * Creates a field performing ITK gradient magnitude recursive gaussian image
 * filter on scalar source field image.
 * Sets number of components to same number as <source_field>.
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_imagefilter_gradient_magnitude_recursive_gaussian(
	cmzn_fieldmodule_id field_module, cmzn_field_id source_field,
	double sigma);

/***************************************************************************//**
 * Creates a field performing ITK histogram image filter on source field image.
 * If neither histogramMinimum or histogramMaximum are specified then the minimums and
 * maximums are calculated based on the minimum and maximum values in the input image.
 * @param numberOfBins  Number of bins per source field component.
 * @param marginalScale  A measure of precision with which the histogram is calculated
 * @param histogramMinimum  Optional array of minimum value of histogram for each source field component
 * @param histogramMaximum  Optional array of maximum value of histogram for each source field component
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_imagefilter_histogram(
	cmzn_fieldmodule_id field_module, cmzn_field_id source_field,
	const int *numberOfBins, double marginalScale,
	const double *histogramMinimum, const double *histogramMaximum);

/***************************************************************************//**
 * Create field performing ITK mean image filter on source_field image.
 * The <radius_sizes> is a vector of integers of dimension specified by the
 * <source_field> dimension.
 * Sets number of components to same number as <source_field>.
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_imagefilter_mean(
	cmzn_fieldmodule_id field_module, cmzn_field_id source_field,
	int *radius_sizes);

/***************************************************************************//**
 * Creates a field performing ITK rescale intensity image filter on scalar
 * source field image. Sets number of components to same number as source field.
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_imagefilter_rescale_intensity(
	cmzn_fieldmodule_id field_module, cmzn_field_id source_field,
	double outputMin, double outputMax);

/***************************************************************************//**
 * Creates a field performing ITK sigmoid image filter on scalar source field
 * image. Sets number of components to same number as <source_field>.
 */
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_imagefilter_sigmoid(
	cmzn_fieldmodule_id field_module, cmzn_field_id source_field,
	double min, double max,	double alpha, double beta);

/**
 * Creates a field applying the ITK threshold image filter to the source field.
 * The newly created field replaces certain values with a specified outside
 * value, based on which threshold mode and the threshold values.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field The field to be filtered
 *
 * @return Newly created field
*/
ZINC_API cmzn_field_id cmzn_fieldmodule_create_field_imagefilter_threshold(
	cmzn_fieldmodule_id field_module, cmzn_field_id source_field);

/**
 * If field can be cast to a cmzn_field_imagefilter_threshold_id do so
 * and return the field.  Otherwise return NULL.
 * Caller is responsible for destroying the new image filter reference.
 *
 * @param field Id of the field to cast
 * @return Id of the cast field, or NULL
*/
ZINC_API cmzn_field_imagefilter_threshold_id cmzn_field_cast_imagefilter_threshold(cmzn_field_id field);

/**
 *  Get the threshold mode for this image filter.
 *
 * @param imagefilter_threshold  handle of the threshold image filter.
 * @return  the current threshold mode seleted for this filter.
 */
ZINC_API enum cmzn_field_imagefilter_threshold_mode cmzn_field_imagefilter_threshold_get_mode(
	cmzn_field_imagefilter_threshold_id imagefilter_threshold);

/**
 *  Set the threshold mode for this image filter.
 *
 * For the below mode, all pixels BELOW the below value are set to
 * the outside value
 * For the above mode, all pixels ABOVE the above value are set to a
 * outside value
 * For the oustide mode, all pixels OUTSIDE the range defined by the
 * below and above values are set to the outside value
 *
 * @param imagefilter_threshold  handle of the threshold image filter.
 * @param mode The threshold mode to apply, either BELOW, ABOVE or OUTSIDE
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_imagefilter_threshold_set_mode(
	cmzn_field_imagefilter_threshold_id imagefilter_threshold,
	enum cmzn_field_imagefilter_threshold_mode mode);

/**
 *  Get the outside value for this image filter.
 *
 * @param imagefilter_threshold  handle of the threshold image filter.
 * @return  the current outside value mode set on this filter.
 */
ZINC_API double cmzn_field_imagefilter_threshold_get_outside_value(
	cmzn_field_imagefilter_threshold_id imagefilter_threshold);

/**
 *  Set the outside value for this image filter.
 *
 * @param imagefilter_threshold  handle of the threshold image filter.
 * @param outside_value The value to replace all thresholded values with
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_imagefilter_threshold_set_outside_value(
	cmzn_field_imagefilter_threshold_id imagefilter_threshold, double outside_value);

/**
 *  Get the outside value for this image filter.
 *
 * @param imagefilter_threshold  handle of the threshold image filter.
 * @return  the current below value set on this filter.
 */
ZINC_API double cmzn_field_imagefilter_threshold_get_below(
	cmzn_field_imagefilter_threshold_id imagefilter_threshold);

/**
 *  Set the below value for this image filter.
 *
 *  all pixels BELOW the below value are set to a outside value
 *
 * @param imagefilter_threshold  handle of the threshold image filter.
 * @param below The below value to be set.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_imagefilter_threshold_set_below(cmzn_field_imagefilter_threshold_id
	imagefilter_threshold, double below_value);

/**
 *  Get the above value for this image filter.
 *
 * @param imagefilter_threshold  handle of the threshold image filter.
 * @return  the current above value set on this filter.
 */
ZINC_API double cmzn_field_imagefilter_threshold_get_above(
	cmzn_field_imagefilter_threshold_id imagefilter_threshold);

/**
 *  Set the above value for this image filter.
 *
 * all pixels ABOVE the above value are set to a outside value
 *
 * @param imagefilter_threshold  handle of the threshold image filter.
 * @param above The above value to be set.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_imagefilter_threshold_set_above(cmzn_field_imagefilter_threshold_id
	imagefilter_threshold, double above_value);

/**
 * Cast imagefilter_threshold field back to its base field and return the field.
 * IMPORTANT NOTE: Returned field does not have incremented reference count and
 * must not be destroyed. Use cmzn_field_access() to add a reference if
 * maintaining returned handle beyond the lifetime of the derived field.
 * Use this function to call base-class API, e.g.:
 * cmzn_field_set_name(cmzn_field_derived_base_cast(derived_field), "bob");
 *
 * @param imagefilter_discrete_gaussian  Handle to the imagefilter_discrete_gaussian field to cast.
 * @return  Non-accessed handle to the base field or NULL if failed.
 */
ZINC_C_INLINE cmzn_field_id cmzn_field_imagefilter_threshold_base_cast(
		cmzn_field_imagefilter_threshold_id imagefilter_threshold)
{
	return (cmzn_field_id)(imagefilter_threshold);
}

/**
 * Destroys this reference to the imagefilter_threshold field and sets it
 * to NULL. Internally this just decrements the reference count.
 *
 * @param imagefilter_threshold_address  Address of handle to the field to
 * 		destroy.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_field_imagefilter_threshold_destroy(
		cmzn_field_imagefilter_threshold_id *imagefilter_threshold_address);

#ifdef __cplusplus
}
#endif

#endif
