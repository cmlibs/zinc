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

#include "types/cmiss_field_id.h"
#include "types/cmiss_field_image_processing_id.h"
#include "types/cmiss_field_module_id.h"

/***************************************************************************//**
 * Creates a field performing ITK binary dilate image filter on scalar source
 * field image. Sets number of components to same number as <source_field>.
 * The <radius> and <dilate_value> specify the radius of pixels to use
 * for dilation and what pixel value to use for dilation
 */
Cmiss_field_id Cmiss_field_module_create_binary_dilate_image_filter(
	Cmiss_field_module_id field_module, Cmiss_field_id source_field,
	int radius, double dilate_value);

int Cmiss_field_get_type_binary_dilate_image_filter(Cmiss_field_id field,
	Cmiss_field_id *source_field,
	int *radius, double *dilate_value);
/*******************************************************************************
LAST MODIFIED : 16 July 2007

DESCRIPTION :
If the field is of type FIELD_BINARYDILATEFILTER, the source_field and binary_dilate_image_filter
used by it are returned - otherwise an error is reported.
WARNING: To be deprecated.
==============================================================================*/

/***************************************************************************//**
 * Creates a field performing ITK binary erode image filter on scalar source
 * field image. Sets number of components to same number as <source_field>.
 * The <radius> and <erode_value> specify the radius of pixels to use
 * for dilation and what pixel value to use for dilation
 */
Cmiss_field_id Cmiss_field_module_create_binary_erode_image_filter(
		Cmiss_field_module_id field_module, Cmiss_field_id source_field,
		int radius, double erode_value);

int Cmiss_field_get_type_binary_erode_image_filter(Cmiss_field_id field,
	Cmiss_field_id *source_field, int *radius, double *erode_value);
/*******************************************************************************
LAST MODIFIED : 16 July 2007

DESCRIPTION :
If the field is of type FIELD_BINARYERODEFILTER, the source_field and binary_erode_image_filter
used by it are returned - otherwise an error is reported.
WARNING: To be deprecated.
==============================================================================*/

/*****************************************************************************//**
 * Creates a field which applies an ITK binary threshold image filter on source.
 * The newly created field consists of binary values (either 0 or 1) which are
 * determined by applying the threshold range to the source field. 
 * Input values with an intensity range between lower_threshold and the 
 * upper_threshold are set to 1, the rest are set to 0.
 * 
 * @param field_module  Region field module which will own new field.
 * @param source_field  The field to be filtered
 * @param lower_threshold  Threshold value below which all pixels are set to 0 
 * @param upper_threshold  Theshold value above which all values are set to 0
 * @return  Newly created field
*/
Cmiss_field_id Cmiss_field_module_create_binary_threshold_image_filter(
	Cmiss_field_module_id field_module, Cmiss_field_id source_field,
	double lower_threshold, double upper_threshold);

/*****************************************************************************//**
 * If field can be cast to a Cmiss_field_binary_threshold_image_filter_id do so
 * and return the field.  Otherwise return NULL.
 * Caller is responsible for destroying the new image filter reference.
 * 
 * @param field  Id of the field to cast
 * @return  Id of the cast field, or NULL
*/
Cmiss_field_binary_threshold_image_filter_id Cmiss_field_cast_binary_threshold_image_filter(Cmiss_field_id field);

int Cmiss_field_get_type_binary_threshold_image_filter(Cmiss_field_id field,
	Cmiss_field_id *source_field,
	double *lower_threshold, double *upper_threshold);
/*******************************************************************************
LAST MODIFIED : 9 September 2006

DESCRIPTION :
If the field is of type FIELD_BINARYTHRESHOLDFILTER, the source_field and binary_threshold_image_filter
used by it are returned - otherwise an error is reported.
WARNING: To be deprecated.
==============================================================================*/



/***************************************************************************//**
 * Creates a field returning result of ITK canny edge detection filter on the
 * source field image. Sets number of components to same number as source field.
 */
Cmiss_field_id Cmiss_field_module_create_canny_edge_detection_image_filter(
	Cmiss_field_module_id field_module, Cmiss_field_id source_field,
	double variance, double maximumError, 
  double upperThreshold, double lowerThreshold);

int Cmiss_field_get_type_canny_edge_detection_image_filter(Cmiss_field_id field,
	Cmiss_field_id *source_field, double *variance, double *maximumError,
	double *upperThreshold, double *lowerThreshold);
/*******************************************************************************
LAST MODIFIED : 9 September 2006

DESCRIPTION :
If the field is of type FIELD_CANNYEDGEDETECTIONFILTER, the source_field and canny_edge_detection_image_filter
used by it are returned - otherwise an error is reported.
WARNING: To be deprecated.
==============================================================================*/



/***************************************************************************//**
 * Creates a field performing ITK connected threshold image filter on scalar
 * source field image. Sets number of components to same number as source field.
 */
Cmiss_field_id Cmiss_field_module_create_connected_threshold_image_filter(
	Cmiss_field_module_id field_module, Cmiss_field_id source_field,
  double lower_threshold, double upper_threshold, double replace_value,
	int num_seed_points, int dimension, const double *seed_points);

int Cmiss_field_get_type_connected_threshold_image_filter(Cmiss_field_id field,
	Cmiss_field_id *source_field,
  double *lower_threshold, double *upper_threshold, double *replace_value,
  int *num_seed_points, int *dimension, double **seed_points);
/*******************************************************************************
LAST MODIFIED : 16 July 2007

DESCRIPTION :
If the field is of type FIELD_CONNECTED_THRESHOLD_IMAGE_FILTER, the source_field and connected_threshold_image_filter
used by it are returned - otherwise an error is reported.
WARNING: To be deprecated.
==============================================================================*/



/***************************************************************************//**
 * Creates a field performing ITK curvature anisotropic diffusion image filter
 * on scalar source field image.
 * Sets number of components to same number as <source_field>.
 */
Cmiss_field_id Cmiss_field_module_create_curvature_anisotropic_diffusion_image_filter(
	Cmiss_field_module_id field_module, Cmiss_field_id source_field,
	double timeStep, double conductance, int numIterations);

int Cmiss_field_get_type_curvature_anisotropic_diffusion_image_filter(Cmiss_field_id field,
	Cmiss_field_id *source_field, double *timeStep, double *conductance, int *numIterations);
/*******************************************************************************
LAST MODIFIED : 18 Nov 2006

DESCRIPTION :
If the field is of type FIELD_CURVATUREANISOTROPICDIFFUSIONIMAGEFILTER, the source_field and curvature_anisotropic_diffusion_image_filter
used by it are returned - otherwise an error is reported.
WARNING: To be deprecated.
==============================================================================*/

/*****************************************************************************//**
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
 * @param variance  The variance of the gaussian distribution used in the filter
 * @param max_kernel_width  The limit on the maximum kernel width that may be used
 * @return  Newly created field
*/
Cmiss_field_id Cmiss_field_module_create_discrete_gaussian_image_filter(
	Cmiss_field_module_id field_module, Cmiss_field_id source_field,
	double variance, int maxKernelWidth);

/*****************************************************************************//**
 * If field can be cast to a Cmiss_field_discrete_gaussian_image_filter_id do so
 * and return the field.  Otherwise return NULL.
 * Caller is responsible for destroying the new image filter reference.
 * 
 * @param field Id of the field to cast
 * @return Id of the cast field, or NULL
*/
Cmiss_field_discrete_gaussian_image_filter_id Cmiss_field_cast_discrete_gaussian_image_filter(Cmiss_field_id field);

int Cmiss_field_get_type_discrete_gaussian_image_filter(Cmiss_field_id field,
	Cmiss_field_id *source_field, double *variance, int *maxKernelWidth);
/*******************************************************************************
LAST MODIFIED : 18 Nov 2006

DESCRIPTION :
If the field is of type FIELD_DISCRETEGAUSSIANIMAGEFILTER_H, the source_field and discrete_gaussian_image_filter
used by it are returned - otherwise an error is reported.
WARNING: To be deprecated.
==============================================================================*/

/***************************************************************************//**
 * Creates a field performing ITK fast marching image filter on scalar source field
 * image. Sets number of components to same number as <source_field>.
 */
Cmiss_field_id Cmiss_field_module_create_fast_marching_image_filter(
	Cmiss_field_module_id field_module, Cmiss_field_id source_field,
	double stopping_value, int num_seed_points, int dimension,
	const double *seed_points, const double *seed_values, const int *output_size);

int Cmiss_field_get_type_fast_marching_image_filter(Cmiss_field_id field,
	Cmiss_field_id *source_field, double *stopping_value,
  int *num_seed_points, int *dimension, double **seed_points,
  double **seed_values, int **output_size);
/*******************************************************************************
LAST MODIFIED : 18 Nov 2006

DESCRIPTION :
If the field is of type FIELD_FAST_MARCHING_IMAGE_FILTER, the source_field and fast_marching_image_filter
used by it are returned - otherwise an error is reported.
WARNING: To be deprecated.
==============================================================================*/



/***************************************************************************//**
 * Creates a field performing ITK gradient magnitude recursive gaussian image
 * filter on scalar source field image.
 * Sets number of components to same number as <source_field>.
 */
Cmiss_field_id Cmiss_field_module_create_gradient_magnitude_recursive_gaussian_image_filter(
	Cmiss_field_module_id field_module, Cmiss_field_id source_field,
	double sigma);

int Cmiss_field_get_type_gradient_magnitude_recursive_gaussian_image_filter(
	Cmiss_field_id field, Cmiss_field_id *source_field, double *sigma);
/*******************************************************************************
LAST MODIFIED : 18 Nov 2006

DESCRIPTION :
If the field is of type FIELD_GRADIENT_MAGNITUDE_RECURSIVE_GAUSSIAN_IMAGE_FILTER, 
the source_field and discrete_gaussian_image_filter
used by it are returned - otherwise an error is reported.
WARNING: To be deprecated.
==============================================================================*/

/***************************************************************************//**
 * Creates a field performing ITK histogram image filter on source field image.
 * If neither histogramMinimum or histogramMaximum are specified then the minimums and
 * maximums are calculated based on the minimum and maximum values in the input image.
 * @param numberOfBins  Number of bins per source field component.
 * @param marginalScale  A measure of precision with which the histogram is calculated
 * @param histogramMinimum  Optional array of minimum value of histogram for each source field component
 * @param histogramMaximum  Optional array of maximum value of histogram for each source field component
 */
Cmiss_field_id Cmiss_field_module_create_histogram_image_filter(
	Cmiss_field_module_id field_module, Cmiss_field_id source_field,
	const int *numberOfBins, double marginalScale,
	const double *histogramMinimum, const double *histogramMaximum);

int Cmiss_field_get_type_histogram_image_filter(Cmiss_field_id field,
	Cmiss_field_id *source_field, int **numberOfBins, double *marginalScale,
	double **histogramMinimum, double **histogramMaximum);
/*******************************************************************************
LAST MODIFIED : 20 March 2008

DESCRIPTION :
If the field is of type FIELD_HISTOGRAM_IMAGE_FILTER, the source_field and histogram_image_filter
used by it are returned - otherwise an error is reported.
WARNING: To be deprecated.
==============================================================================*/

/***************************************************************************//**
 * Create field performing ITK mean image filter on source_field image.
 * The <radius_sizes> is a vector of integers of dimension specified by the
 * <source_field> dimension.
 * Sets number of components to same number as <source_field>.
 */
Cmiss_field_id Cmiss_field_module_create_mean_image_filter(
	Cmiss_field_module_id field_module, Cmiss_field_id source_field,
	int *radius_sizes);

int Cmiss_field_get_type_mean_image_filter(Cmiss_field_id field,
	Cmiss_field_id *source_field, int **radius_sizes);
/*******************************************************************************
LAST MODIFIED : 30 August 2006

DESCRIPTION :
If the field is of type FIELD_MEANIMAGEFILTER, the source_field and mean_image_filter
used by it are returned - otherwise an error is reported.
WARNING: To be deprecated.
==============================================================================*/

/***************************************************************************//**
 * Creates a field performing ITK rescale intensity image filter on scalar
 * source field image. Sets number of components to same number as source field.
 */
Cmiss_field_id Cmiss_field_module_create_rescale_intensity_image_filter(
	Cmiss_field_module_id field_module, Cmiss_field_id source_field,
	double outputMin, double outputMax);

int Cmiss_field_get_type_rescale_intensity_image_filter(Cmiss_field_id field,
	Cmiss_field_id *source_field, double *outputMin, double *outputMax);
/*******************************************************************************
LAST MODIFIED : 18 Nov 2006

DESCRIPTION :
If the field is of type FIELD_RESCALEINTENSITYIMAGEFILTER, the source_field and rescale_intensity_image_filter
used by it are returned - otherwise an error is reported.
WARNING: To be deprecated.
==============================================================================*/

/***************************************************************************//**
 * Creates a field performing ITK sigmoid image filter on scalar source field
 * image. Sets number of components to same number as <source_field>.
 */
Cmiss_field_id Cmiss_field_module_create_sigmoid_image_filter(
	Cmiss_field_module_id field_module, Cmiss_field_id source_field,
	double min, double max,	double alpha, double beta);

int Cmiss_field_get_type_sigmoid_image_filter(Cmiss_field_id field,
	Cmiss_field_id *source_field, double *min, double *max, double *alpha, double *beta);
/*******************************************************************************
LAST MODIFIED : 18 October 2006

DESCRIPTION :
If the field is of type FIELD_SIGMOIDIMAGEFILTER, the source_field and sigmoid_image_filter
used by it are returned - otherwise an error is reported.
WARNING: To be deprecated.
==============================================================================*/

enum General_threshold_filter_mode
{
	GENERAL_THRESHOLD_FILTER_MODE_BELOW,
	GENERAL_THRESHOLD_FILTER_MODE_ABOVE,
	GENERAL_THRESHOLD_FILTER_MODE_OUTSIDE
}; /* enum General_threshold_filter_mode */

/*****************************************************************************//**
 * Creates a field applying the ITK threshold image filter to the source field.
 * The newly created field replaces certain values with a specified outside
 * value, based on which threshold mode and the threshold values.
 * For the below mode, all pixels BELOW the below value are set to 
 * the outside value
 * For the above mode, all pixels ABOVE the above value are set to a
 * outside value
 * For the oustide mode, all pixels OUTSIDE the range defined by the 
 * below and above values are set to the outside value
 * 
 * @param field_module  Region field module which will own new field.
 * @param source_field The field to be filtered
 * @param threshold_mode The threshold mode to apply, either BELOW, ABOVE or OUTSIDE
 * @param outside_value The value to replace all thresholded values with
 * @param below_value Below value used by BELOW and OUTSIDE modes
 * @param above_value Above value used by ABOVE and OUTSIDE modes
 * @return Newly created field
*/
Cmiss_field_id Cmiss_field_module_create_threshold_image_filter(
	Cmiss_field_module_id field_module, Cmiss_field_id source_field, 
	enum General_threshold_filter_mode threshold_mode, 
	double outside_value, double below_value, double above_value);

/*****************************************************************************//**
 * If field can be cast to a Cmiss_field_threshold_image_filter_id do so
 * and return the field.  Otherwise return NULL.
 * Caller is responsible for destroying the new image filter reference.
 * 
 * @param field Id of the field to cast
 * @return Id of the cast field, or NULL
*/
Cmiss_field_threshold_image_filter_id Cmiss_field_cast_threshold_image_filter(Cmiss_field_id field);

#endif /* !defined (CMISS_FIELD_IMAGE_PROCESSING_H) */
