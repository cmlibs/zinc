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

class FieldImagefilterBinaryDilate : public Field
{

private:
	explicit FieldImagefilterBinaryDilate(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldImagefilterBinaryDilate
		Fieldmodule::createFieldImagefilterBinaryDilate(Field& sourceField,
			int radius, double dilate_value);

public:

	FieldImagefilterBinaryDilate() : Field(0)
	{	}

};

class FieldImagefilterBinaryErode : public Field
{

private:
	explicit FieldImagefilterBinaryErode(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldImagefilterBinaryErode
		Fieldmodule::createFieldImagefilterBinaryErode(Field& sourceField,
			int radius, double erode_value);

public:

	FieldImagefilterBinaryErode() : Field(0)
	{	}

};

class FieldImagefilterBinaryThreshold : public Field
{

private:
	explicit FieldImagefilterBinaryThreshold(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldImagefilterBinaryThreshold
		Fieldmodule::createFieldImagefilterBinaryThreshold(Field& sourceField);

public:

	FieldImagefilterBinaryThreshold() : Field(0)
	{	}

	FieldImagefilterBinaryThreshold(Field& field) :
		Field(reinterpret_cast<cmzn_field_id>(cmzn_field_cast_imagefilter_binary_threshold(field.getId())))
	{	}

	double getLowerThreshold()
	{
		return cmzn_field_imagefilter_binary_threshold_get_lower_threshold(
			reinterpret_cast<cmzn_field_imagefilter_binary_threshold_id>(id));
	}

	int setLowerThreshold(double lowerThreshold)
	{
		return cmzn_field_imagefilter_binary_threshold_set_lower_threshold(
			reinterpret_cast<cmzn_field_imagefilter_binary_threshold_id>(id),
			lowerThreshold);
	}

	double getUpperThreshold()
	{
		return cmzn_field_imagefilter_binary_threshold_get_upper_threshold(
			reinterpret_cast<cmzn_field_imagefilter_binary_threshold_id>(id));
	}

	int setUpperThreshold(double upperThreshold)
	{
		return cmzn_field_imagefilter_binary_threshold_set_upper_threshold(
			reinterpret_cast<cmzn_field_imagefilter_binary_threshold_id>(id),
			upperThreshold);
	}

};

class FieldImagefilterCannyEdgeDetection : public Field
{

private:
	explicit FieldImagefilterCannyEdgeDetection(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldImagefilterCannyEdgeDetection
		Fieldmodule::createFieldImagefilterCannyEdgeDetection(Field& sourceField,
			double variance, double maximumError, double upperThreshold, double lowerThreshold);

public:

	FieldImagefilterCannyEdgeDetection() : Field(0)
	{	}

};

class FieldImagefilterConnectedThreshold : public Field
{

private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldImagefilterConnectedThreshold(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldImagefilterConnectedThreshold
		Fieldmodule::createFieldImagefilterConnectedThreshold(Field& sourceField,
			  double lowerThreshold, double upperThreshold, double replaceValue,
			  int dimension, int seedPointsCount, const double *seedPoints);

public:

	FieldImagefilterConnectedThreshold() : Field(0)
	{	}

};

class FieldImagefilterCurvatureAnisotropicDiffusion : public Field
{

private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldImagefilterCurvatureAnisotropicDiffusion(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldImagefilterCurvatureAnisotropicDiffusion
		Fieldmodule::createFieldImagefilterCurvatureAnisotropicDiffusion(Field& sourceField,
			double timeStep, double conductance, int numIterations);

public:

	FieldImagefilterCurvatureAnisotropicDiffusion() : Field(0)
	{	}

};

class FieldImagefilterDiscreteGaussian : public Field
{

private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldImagefilterDiscreteGaussian(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldImagefilterDiscreteGaussian
		Fieldmodule::createFieldImagefilterDiscreteGaussian(Field& sourceField);

public:

	FieldImagefilterDiscreteGaussian() : Field(0)
	{	}

	FieldImagefilterDiscreteGaussian(Field& field) :
		Field(reinterpret_cast<cmzn_field_id>(cmzn_field_cast_imagefilter_discrete_gaussian(field.getId())))
	{	}

	double getVariance()
	{
		return cmzn_field_imagefilter_discrete_gaussian_get_variance(
			reinterpret_cast<cmzn_field_imagefilter_discrete_gaussian_id>(id));
	}

	int setVariance(double variance)
	{
		return cmzn_field_imagefilter_discrete_gaussian_set_variance(
			reinterpret_cast<cmzn_field_imagefilter_discrete_gaussian_id>(id),
			variance);
	}

	int getMaxKernelWidth()
	{
		return cmzn_field_imagefilter_discrete_gaussian_get_max_kernel_width(
			reinterpret_cast<cmzn_field_imagefilter_discrete_gaussian_id>(id));
	}

	int setMaxKernelWidth(int maxKernelWidth)
	{
		return cmzn_field_imagefilter_discrete_gaussian_set_max_kernel_width(
			reinterpret_cast<cmzn_field_imagefilter_discrete_gaussian_id>(id),
			maxKernelWidth);
	}

};

class FieldImagefilterGradientMagnitudeRecursiveGaussian : public Field
{

private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldImagefilterGradientMagnitudeRecursiveGaussian(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldImagefilterGradientMagnitudeRecursiveGaussian
		Fieldmodule::createFieldImagefilterGradientMagnitudeRecursiveGaussian(Field& sourceField,
			double sigma);

public:

	FieldImagefilterGradientMagnitudeRecursiveGaussian() : Field(0)
	{	}

};

class FieldImagefilterRescaleIntensity : public Field
{

private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldImagefilterRescaleIntensity(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldImagefilterRescaleIntensity
		Fieldmodule::createFieldImagefilterRescaleIntensity(Field& sourceField,
			double outputMin, double outputMax);

public:

	FieldImagefilterRescaleIntensity() : Field(0)
	{	}

};

class FieldImagefilterSigmoid : public Field
{

private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldImagefilterSigmoid(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldImagefilterSigmoid
		Fieldmodule::createFieldImagefilterSigmoid(Field& sourceField,
			double min, double max,	double alpha, double beta);

public:

	FieldImagefilterSigmoid() : Field(0)
	{	}

};


class FieldImagefilterThreshold : public Field
{

private:
	explicit FieldImagefilterThreshold(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldImagefilterThreshold
		Fieldmodule::createFieldImagefilterThreshold(Field& sourceField);

public:

	FieldImagefilterThreshold() : Field(0)
	{	}

	FieldImagefilterThreshold(Field& field) :
		Field(reinterpret_cast<cmzn_field_id>(cmzn_field_cast_imagefilter_threshold(field.getId())))
	{	}

	enum Condition
	{
		CONDITION_INVALID = CMZN_FIELD_IMAGEFILTER_THRESHOLD_CONDITION_INVALID,
		CONDITION_ABOVE = CMZN_FIELD_IMAGEFILTER_THRESHOLD_CONDITION_ABOVE,
		CONDITION_BELOW = CMZN_FIELD_IMAGEFILTER_THRESHOLD_CONDITION_BELOW,
		CONDITION_OUTSIDE = CMZN_FIELD_IMAGEFILTER_THRESHOLD_CONDITION_OUTSIDE
	};

	enum Condition getCondition()
	{
		return static_cast<Condition>(cmzn_field_imagefilter_threshold_get_condition(
			reinterpret_cast<cmzn_field_imagefilter_threshold_id>(id)));
	}

	int setCondition(Condition condition)
	{
		return cmzn_field_imagefilter_threshold_set_condition(
			reinterpret_cast<cmzn_field_imagefilter_threshold_id>(id),
			static_cast<cmzn_field_imagefilter_threshold_condition>(condition));
	}

	double getOutsideValue()
	{
		return cmzn_field_imagefilter_threshold_get_outside_value(
			reinterpret_cast<cmzn_field_imagefilter_threshold_id>(id));
	}

	int setOutsideValue(double outsideValue)
	{
		return cmzn_field_imagefilter_threshold_set_outside_value(
			reinterpret_cast<cmzn_field_imagefilter_threshold_id>(id), outsideValue);
	}

	double getLowerThreshold()
	{
		return cmzn_field_imagefilter_threshold_get_lower_threshold(
			reinterpret_cast<cmzn_field_imagefilter_threshold_id>(id));
	}

	int setLowerThreshold(double lowerValue)
	{
		return cmzn_field_imagefilter_threshold_set_lower_threshold(
			reinterpret_cast<cmzn_field_imagefilter_threshold_id>(id), lowerValue);
	}

	double getUpperThreshold()
	{
		return cmzn_field_imagefilter_threshold_get_upper_threshold(
			reinterpret_cast<cmzn_field_imagefilter_threshold_id>(id));
	}

	int setUpperThreshold(double upperValue)
	{
		return cmzn_field_imagefilter_threshold_set_upper_threshold(
			reinterpret_cast<cmzn_field_imagefilter_threshold_id>(id), upperValue);
	}

};

inline FieldImagefilterBinaryDilate
	Fieldmodule::createFieldImagefilterBinaryDilate(Field& sourceField,
		int radius, double dilate_value)
{
	return FieldImagefilterBinaryDilate(
		cmzn_fieldmodule_create_field_imagefilter_binary_dilate(id, sourceField.getId(),
			radius, dilate_value));
}

inline FieldImagefilterBinaryErode
	Fieldmodule::createFieldImagefilterBinaryErode(Field& sourceField,
		int radius, double erode_value)
{
	return FieldImagefilterBinaryErode(
		cmzn_fieldmodule_create_field_imagefilter_binary_erode(id, sourceField.getId(),
			radius, erode_value));
}

inline FieldImagefilterBinaryThreshold
	Fieldmodule::createFieldImagefilterBinaryThreshold(Field& sourceField)
{
	return FieldImagefilterBinaryThreshold(
		cmzn_fieldmodule_create_field_imagefilter_binary_threshold(
			id, sourceField.getId()));
}

inline FieldImagefilterCannyEdgeDetection
	Fieldmodule::createFieldImagefilterCannyEdgeDetection(Field& sourceField,
		double variance, double maximumError, double upperThreshold, double lowerThreshold)
{
	return FieldImagefilterCannyEdgeDetection(
		cmzn_fieldmodule_create_field_imagefilter_canny_edge_detection(
			id, sourceField.getId(),
			variance, maximumError, upperThreshold, lowerThreshold));
}

inline FieldImagefilterConnectedThreshold
	Fieldmodule::createFieldImagefilterConnectedThreshold(Field& sourceField,
		  double lowerThreshold, double upperThreshold, double replaceValue,
		  int dimension, int seedPointsCount, const double *seedPoints)
{
	return FieldImagefilterConnectedThreshold(
		cmzn_fieldmodule_create_field_imagefilter_connected_threshold(id, sourceField.getId(),
		lowerThreshold, upperThreshold, replaceValue, seedPointsCount, dimension, seedPoints));
}

inline FieldImagefilterCurvatureAnisotropicDiffusion
	Fieldmodule::createFieldImagefilterCurvatureAnisotropicDiffusion(Field& sourceField,
		double timeStep, double conductance, int numIterations)
{
	return FieldImagefilterCurvatureAnisotropicDiffusion(
		cmzn_fieldmodule_create_field_imagefilter_curvature_anisotropic_diffusion(id, sourceField.getId(),
			timeStep, conductance, numIterations));
}

inline FieldImagefilterDiscreteGaussian
	Fieldmodule::createFieldImagefilterDiscreteGaussian(Field& sourceField)
{
	return FieldImagefilterDiscreteGaussian(
		cmzn_fieldmodule_create_field_imagefilter_discrete_gaussian(id, sourceField.getId()));
}

inline FieldImagefilterGradientMagnitudeRecursiveGaussian
	Fieldmodule::createFieldImagefilterGradientMagnitudeRecursiveGaussian(Field& sourceField,
		double sigma)
{
	return FieldImagefilterGradientMagnitudeRecursiveGaussian(
		cmzn_fieldmodule_create_field_imagefilter_gradient_magnitude_recursive_gaussian(id,
			sourceField.getId(), sigma));
}

inline FieldImagefilterRescaleIntensity
	Fieldmodule::createFieldImagefilterRescaleIntensity(Field& sourceField,
		double outputMin, double outputMax)
{
	return FieldImagefilterRescaleIntensity(
		cmzn_fieldmodule_create_field_imagefilter_rescale_intensity(id,
			sourceField.getId(), outputMin, outputMax));
}

inline FieldImagefilterSigmoid
	Fieldmodule::createFieldImagefilterSigmoid(Field& sourceField,
		double min, double max,	double alpha, double beta)
{
	return FieldImagefilterSigmoid(
		cmzn_fieldmodule_create_field_imagefilter_sigmoid(id,
			sourceField.getId(), min, max, alpha, beta));
}

inline FieldImagefilterThreshold
	Fieldmodule::createFieldImagefilterThreshold(Field& sourceField)
{
	return FieldImagefilterThreshold(
		cmzn_fieldmodule_create_field_imagefilter_threshold(
			id, sourceField.getId()));
}

}  // namespace Zinc
}

#endif
