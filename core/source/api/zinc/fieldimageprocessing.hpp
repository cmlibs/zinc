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
		Fieldmodule::createFieldBinaryDilateImageFilter(Field& sourceField,
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
		Fieldmodule::createFieldBinaryErodeImageFilter(Field& sourceField,
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
		Fieldmodule::createFieldBinaryThresholdImageFilter(Field& sourceField,
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
		Fieldmodule::createFieldCannyEdgeDetectionImageFilter(Field& sourceField,
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
		Fieldmodule::createFieldConnectedThresholdImageFilter(Field& sourceField,
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
		Fieldmodule::createFieldCurvatureAnisotropicDiffusionImageFilter(Field& sourceField,
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
		Fieldmodule::createFieldDiscreteGaussianImageFilter(Field& sourceField,
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
		Fieldmodule::createFieldGradientMagnitudeRecursiveGaussianImageFilter(Field& sourceField,
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
		Fieldmodule::createFieldRescaleIntensityImageFilter(Field& sourceField,
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
		Fieldmodule::createFieldSigmoidImageFilter(Field& sourceField,
			double min, double max,	double alpha, double beta);

public:

	FieldSigmoidImageFilter() : Field(0)
	{	}

};

inline FieldBinaryDilateImageFilter
	Fieldmodule::createFieldBinaryDilateImageFilter(Field& sourceField,
		int radius, double erode_value)
{
	return FieldBinaryDilateImageFilter(
		cmzn_fieldmodule_create_field_binary_dilate_image_filter(id, sourceField.getId(),
			radius, erode_value));
}

inline FieldBinaryErodeImageFilter
	Fieldmodule::createFieldBinaryErodeImageFilter(Field& sourceField,
		int radius, double dilate_value)
{
	return FieldBinaryErodeImageFilter(
		cmzn_fieldmodule_create_field_binary_erode_image_filter(id, sourceField.getId(),
			radius, dilate_value));
}

inline FieldBinaryThresholdImageFilter
	Fieldmodule::createFieldBinaryThresholdImageFilter(Field& sourceField,
		double lower_threshold, double upper_threshold)
{
	return FieldBinaryThresholdImageFilter(
		cmzn_fieldmodule_create_field_binary_threshold_image_filter(
			id, sourceField.getId(), lower_threshold, upper_threshold));
}

inline FieldCannyEdgeDetectionImageFilter
	Fieldmodule::createFieldCannyEdgeDetectionImageFilter(Field& sourceField,
		double variance, double maximumError, double upperThreshold, double lowerThreshold)
{
	return FieldCannyEdgeDetectionImageFilter(
		cmzn_fieldmodule_create_field_canny_edge_detection_image_filter(
			id, sourceField.getId(),
			variance, maximumError, upperThreshold, lowerThreshold));
}

inline FieldConnectedThresholdImageFilter
	Fieldmodule::createFieldConnectedThresholdImageFilter(Field& sourceField,
		  double lowerThreshold, double upperThreshold, double replaceValue,
		  int dimension, int seedPointsCount, const double *seedPoints)
{
	return FieldConnectedThresholdImageFilter(
		cmzn_fieldmodule_create_field_connected_threshold_image_filter(id, sourceField.getId(),
		lowerThreshold, upperThreshold, replaceValue, seedPointsCount, dimension, seedPoints));
}

inline FieldCurvatureAnisotropicDiffusionImageFilter
	Fieldmodule::createFieldCurvatureAnisotropicDiffusionImageFilter(Field& sourceField,
		double timeStep, double conductance, int numIterations)
{
	return FieldCurvatureAnisotropicDiffusionImageFilter(
		cmzn_fieldmodule_create_field_curvature_anisotropic_diffusion_image_filter(id, sourceField.getId(),
			timeStep, conductance, numIterations));
}

inline FieldDiscreteGaussianImageFilter
	Fieldmodule::createFieldDiscreteGaussianImageFilter(Field& sourceField,
		double variance, int maxKernelWidth)
{
	return FieldDiscreteGaussianImageFilter(
		cmzn_fieldmodule_create_field_discrete_gaussian_image_filter(id, sourceField.getId(),
			variance, maxKernelWidth));
}

inline FieldGradientMagnitudeRecursiveGaussianImageFilter
	Fieldmodule::createFieldGradientMagnitudeRecursiveGaussianImageFilter(Field& sourceField,
		double sigma)
{
	return FieldGradientMagnitudeRecursiveGaussianImageFilter(
		cmzn_fieldmodule_create_field_gradient_magnitude_recursive_gaussian_image_filter(id,
			sourceField.getId(), sigma));
}

inline FieldRescaleIntensityImageFilter
	Fieldmodule::createFieldRescaleIntensityImageFilter(Field& sourceField,
		double outputMin, double outputMax)
{
	return FieldRescaleIntensityImageFilter(
		cmzn_fieldmodule_create_field_rescale_intensity_image_filter(id,
			sourceField.getId(), outputMin, outputMax));
}

inline FieldSigmoidImageFilter
	Fieldmodule::createFieldSigmoidImageFilter(Field& sourceField,
		double min, double max,	double alpha, double beta)
{
	return FieldSigmoidImageFilter(
		cmzn_fieldmodule_create_field_sigmoid_image_filter(id,
			sourceField.getId(), min, max, alpha, beta));
}

}  // namespace Zinc
}

#endif
