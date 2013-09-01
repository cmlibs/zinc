/***************************************************************************//**
 * FILE : fieldtypeimageprocessing.hpp
 */
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
 * The Original Code is libZinc.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2013
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
#ifndef CMZN_FIELDIMAGEPROCESSING_HPP__
#define CMZN_FIELDIMAGEPROCESSING_HPP__

#include "zinc/field.hpp"
#include "zinc/fieldimageprocessing.h"
#include "zinc/fieldmodule.hpp"

namespace zinc
{

class FieldBinaryDilateImageFilter : public Field
{

private:
	explicit FieldBinaryDilateImageFilter(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldBinaryDilateImageFilter
		FieldModule::createBinaryDilateImageFilter(Field& sourceField,
			int radius, double dilate_value);

public:

	FieldBinaryDilateImageFilter() : Field(0)
	{	}

};

class FieldBinaryErodeImageFilter : public Field
{

private:
	explicit FieldBinaryErodeImageFilter(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldBinaryErodeImageFilter
		FieldModule::createBinaryErodeImageFilter(Field& sourceField,
			int radius, double erode_value);

public:

	FieldBinaryErodeImageFilter() : Field(0)
	{	}

};

class FieldBinaryThresholdImageFilter : public Field
{

private:
	explicit FieldBinaryThresholdImageFilter(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldBinaryThresholdImageFilter
		FieldModule::createBinaryThresholdImageFilter(Field& sourceField,
			double lower_threshold, double upper_threshold);

public:

	FieldBinaryThresholdImageFilter() : Field(0)
	{	}

};

class FieldCannyEdgeDetectionImageFilter : public Field
{

private:
	explicit FieldCannyEdgeDetectionImageFilter(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldCannyEdgeDetectionImageFilter
		FieldModule::createCannyEdgeDetectionImageFilter(Field& sourceField,
			double variance, double maximumError, double upperThreshold, double lowerThreshold);

public:

	FieldCannyEdgeDetectionImageFilter() : Field(0)
	{	}

};

class FieldConnectedThresholdImageFilter : public Field
{

private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldConnectedThresholdImageFilter(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldConnectedThresholdImageFilter
		FieldModule::createConnectedThresholdImageFilter(Field& sourceField,
			  double lowerThreshold, double upperThreshold, double replaceValue,
			  int dimension, int seedPointsCount, const double *seedPoints);

public:

	FieldConnectedThresholdImageFilter() : Field(0)
	{	}

};

class FieldCurvatureAnisotropicDiffusionImageFilter : public Field
{

private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldCurvatureAnisotropicDiffusionImageFilter(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldCurvatureAnisotropicDiffusionImageFilter
		FieldModule::createCurvatureAnisotropicDiffusionImageFilter(Field& sourceField,
			double timeStep, double conductance, int numIterations);

public:

	FieldCurvatureAnisotropicDiffusionImageFilter() : Field(0)
	{	}

};

class FieldDiscreteGaussianImageFilter : public Field
{

private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldDiscreteGaussianImageFilter(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldDiscreteGaussianImageFilter
		FieldModule::createDiscreteGaussianImageFilter(Field& sourceField,
			double variance, int maxKernelWidth);

public:

	FieldDiscreteGaussianImageFilter() : Field(0)
	{	}

};

class FieldGradientMagnitudeRecursiveGaussianImageFilter : public Field
{

private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldGradientMagnitudeRecursiveGaussianImageFilter(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldGradientMagnitudeRecursiveGaussianImageFilter
		FieldModule::createGradientMagnitudeRecursiveGaussianImageFilter(Field& sourceField,
			double sigma);

public:

	FieldGradientMagnitudeRecursiveGaussianImageFilter() : Field(0)
	{	}

};

class FieldRescaleIntensityImageFilter : public Field
{

private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldRescaleIntensityImageFilter(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldRescaleIntensityImageFilter
		FieldModule::createRescaleIntensityImageFilter(Field& sourceField,
			double outputMin, double outputMax);

public:

	FieldRescaleIntensityImageFilter() : Field(0)
	{	}

};

class FieldSigmoidImageFilter : public Field
{

private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldSigmoidImageFilter(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldSigmoidImageFilter
		FieldModule::createSigmoidImageFilter(Field& sourceField,
			double min, double max,	double alpha, double beta);

public:

	FieldSigmoidImageFilter() : Field(0)
	{	}

};

inline FieldBinaryDilateImageFilter
	FieldModule::createBinaryDilateImageFilter(Field& sourceField,
		int radius, double erode_value)
{
	return FieldBinaryDilateImageFilter(
		Cmiss_field_module_create_binary_dilate_image_filter(id, sourceField.getId(),
			radius, erode_value));
}

inline FieldBinaryErodeImageFilter
	FieldModule::createBinaryErodeImageFilter(Field& sourceField,
		int radius, double dilate_value)
{
	return FieldBinaryErodeImageFilter(
		Cmiss_field_module_create_binary_erode_image_filter(id, sourceField.getId(),
			radius, dilate_value));
}

inline FieldBinaryThresholdImageFilter
	FieldModule::createBinaryThresholdImageFilter(Field& sourceField,
		double lower_threshold, double upper_threshold)
{
	return FieldBinaryThresholdImageFilter(
		Cmiss_field_module_create_binary_threshold_image_filter(
			id, sourceField.getId(), lower_threshold, upper_threshold));
}

inline FieldCannyEdgeDetectionImageFilter
	FieldModule::createCannyEdgeDetectionImageFilter(Field& sourceField,
		double variance, double maximumError, double upperThreshold, double lowerThreshold)
{
	return FieldCannyEdgeDetectionImageFilter(
		Cmiss_field_module_create_canny_edge_detection_image_filter(
			id, sourceField.getId(),
			variance, maximumError, upperThreshold, lowerThreshold));
}

inline FieldConnectedThresholdImageFilter
	FieldModule::createConnectedThresholdImageFilter(Field& sourceField,
		  double lowerThreshold, double upperThreshold, double replaceValue,
		  int dimension, int seedPointsCount, const double *seedPoints)
{
	return FieldConnectedThresholdImageFilter(
		Cmiss_field_module_create_connected_threshold_image_filter(id, sourceField.getId(),
		lowerThreshold, upperThreshold, replaceValue, seedPointsCount, dimension, seedPoints));
}

inline FieldCurvatureAnisotropicDiffusionImageFilter
	FieldModule::createCurvatureAnisotropicDiffusionImageFilter(Field& sourceField,
		double timeStep, double conductance, int numIterations)
{
	return FieldCurvatureAnisotropicDiffusionImageFilter(
		Cmiss_field_module_create_curvature_anisotropic_diffusion_image_filter(id, sourceField.getId(),
			timeStep, conductance, numIterations));
}

inline FieldDiscreteGaussianImageFilter
	FieldModule::createDiscreteGaussianImageFilter(Field& sourceField,
		double variance, int maxKernelWidth)
{
	return FieldDiscreteGaussianImageFilter(
		Cmiss_field_module_create_discrete_gaussian_image_filter(id, sourceField.getId(),
			variance, maxKernelWidth));
}

inline FieldGradientMagnitudeRecursiveGaussianImageFilter
	FieldModule::createGradientMagnitudeRecursiveGaussianImageFilter(Field& sourceField,
		double sigma)
{
	return FieldGradientMagnitudeRecursiveGaussianImageFilter(
		Cmiss_field_module_create_gradient_magnitude_recursive_gaussian_image_filter(id,
			sourceField.getId(), sigma));
}

inline FieldRescaleIntensityImageFilter
	FieldModule::createRescaleIntensityImageFilter(Field& sourceField,
		double outputMin, double outputMax)
{
	return FieldRescaleIntensityImageFilter(
		Cmiss_field_module_create_rescale_intensity_image_filter(id,
			sourceField.getId(), outputMin, outputMax));
}

inline FieldSigmoidImageFilter
	FieldModule::createSigmoidImageFilter(Field& sourceField,
		double min, double max,	double alpha, double beta)
{
	return FieldSigmoidImageFilter(
		Cmiss_field_module_create_sigmoid_image_filter(id,
			sourceField.getId(), min, max, alpha, beta));
}

}  // namespace zinc

#endif
