/***************************************************************************//**
 * FILE : fieldimageprocessing.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDIMAGEPROCESSING_HPP__
#define CMZN_FIELDIMAGEPROCESSING_HPP__

#include "zinc/field.hpp"
#include "zinc/fieldimageprocessing.h"
#include "zinc/fieldmodule.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class FieldBinaryDilateImageFilter : public Field
{

private:
	explicit FieldBinaryDilateImageFilter(cmzn_field_id field_id) : Field(field_id)
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
	explicit FieldBinaryErodeImageFilter(cmzn_field_id field_id) : Field(field_id)
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
	explicit FieldBinaryThresholdImageFilter(cmzn_field_id field_id) : Field(field_id)
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
	explicit FieldCannyEdgeDetectionImageFilter(cmzn_field_id field_id) : Field(field_id)
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
	explicit FieldConnectedThresholdImageFilter(cmzn_field_id field_id) : Field(field_id)
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
	explicit FieldCurvatureAnisotropicDiffusionImageFilter(cmzn_field_id field_id) : Field(field_id)
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
	explicit FieldDiscreteGaussianImageFilter(cmzn_field_id field_id) : Field(field_id)
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
	explicit FieldGradientMagnitudeRecursiveGaussianImageFilter(cmzn_field_id field_id) : Field(field_id)
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
	explicit FieldRescaleIntensityImageFilter(cmzn_field_id field_id) : Field(field_id)
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
	explicit FieldSigmoidImageFilter(cmzn_field_id field_id) : Field(field_id)
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
		cmzn_field_module_create_binary_dilate_image_filter(id, sourceField.getId(),
			radius, erode_value));
}

inline FieldBinaryErodeImageFilter
	FieldModule::createBinaryErodeImageFilter(Field& sourceField,
		int radius, double dilate_value)
{
	return FieldBinaryErodeImageFilter(
		cmzn_field_module_create_binary_erode_image_filter(id, sourceField.getId(),
			radius, dilate_value));
}

inline FieldBinaryThresholdImageFilter
	FieldModule::createBinaryThresholdImageFilter(Field& sourceField,
		double lower_threshold, double upper_threshold)
{
	return FieldBinaryThresholdImageFilter(
		cmzn_field_module_create_binary_threshold_image_filter(
			id, sourceField.getId(), lower_threshold, upper_threshold));
}

inline FieldCannyEdgeDetectionImageFilter
	FieldModule::createCannyEdgeDetectionImageFilter(Field& sourceField,
		double variance, double maximumError, double upperThreshold, double lowerThreshold)
{
	return FieldCannyEdgeDetectionImageFilter(
		cmzn_field_module_create_canny_edge_detection_image_filter(
			id, sourceField.getId(),
			variance, maximumError, upperThreshold, lowerThreshold));
}

inline FieldConnectedThresholdImageFilter
	FieldModule::createConnectedThresholdImageFilter(Field& sourceField,
		  double lowerThreshold, double upperThreshold, double replaceValue,
		  int dimension, int seedPointsCount, const double *seedPoints)
{
	return FieldConnectedThresholdImageFilter(
		cmzn_field_module_create_connected_threshold_image_filter(id, sourceField.getId(),
		lowerThreshold, upperThreshold, replaceValue, seedPointsCount, dimension, seedPoints));
}

inline FieldCurvatureAnisotropicDiffusionImageFilter
	FieldModule::createCurvatureAnisotropicDiffusionImageFilter(Field& sourceField,
		double timeStep, double conductance, int numIterations)
{
	return FieldCurvatureAnisotropicDiffusionImageFilter(
		cmzn_field_module_create_curvature_anisotropic_diffusion_image_filter(id, sourceField.getId(),
			timeStep, conductance, numIterations));
}

inline FieldDiscreteGaussianImageFilter
	FieldModule::createDiscreteGaussianImageFilter(Field& sourceField,
		double variance, int maxKernelWidth)
{
	return FieldDiscreteGaussianImageFilter(
		cmzn_field_module_create_discrete_gaussian_image_filter(id, sourceField.getId(),
			variance, maxKernelWidth));
}

inline FieldGradientMagnitudeRecursiveGaussianImageFilter
	FieldModule::createGradientMagnitudeRecursiveGaussianImageFilter(Field& sourceField,
		double sigma)
{
	return FieldGradientMagnitudeRecursiveGaussianImageFilter(
		cmzn_field_module_create_gradient_magnitude_recursive_gaussian_image_filter(id,
			sourceField.getId(), sigma));
}

inline FieldRescaleIntensityImageFilter
	FieldModule::createRescaleIntensityImageFilter(Field& sourceField,
		double outputMin, double outputMax)
{
	return FieldRescaleIntensityImageFilter(
		cmzn_field_module_create_rescale_intensity_image_filter(id,
			sourceField.getId(), outputMin, outputMax));
}

inline FieldSigmoidImageFilter
	FieldModule::createSigmoidImageFilter(Field& sourceField,
		double min, double max,	double alpha, double beta)
{
	return FieldSigmoidImageFilter(
		cmzn_field_module_create_sigmoid_image_filter(id,
			sourceField.getId(), min, max, alpha, beta));
}

}  // namespace Zinc
}

#endif
