/***************************************************************************//**
 * FILE : fieldmodule.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDMODULE_HPP__
#define CMZN_FIELDMODULE_HPP__

#include "zinc/fieldmodule.h"
#include "zinc/field.hpp"
#include "zinc/fieldcache.hpp"
#include "zinc/element.hpp"
#include "zinc/node.hpp"
#include "zinc/optimisation.hpp"
#include "zinc/timesequence.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class FieldAlias;
class FieldAdd;
class FieldPower;
class FieldMultiply;
class FieldDivide;
class FieldSubtract;
class FieldSumComponents;
class FieldLog;
class FieldSqrt;
class FieldExp;
class FieldAbs;
class FieldIdentity;
class FieldComponent;
class FieldConcatenate;
class FieldIf;
class FieldConstant;
class FieldStringConstant;
class FieldCoordinateTransformation;
class FieldVectorCoordinateTransformation;
class FieldFibreAxes;
class FieldFiniteElement;
class FieldEmbedded;
class FieldFindMeshLocation;
class FieldNodeValue;
class FieldStoredMeshLocation;
class FieldStoredString;
class FieldGroup;
class FieldImage;
class FieldAnd;
class FieldEqualTo;
class FieldGreaterThan;
class FieldLessThan;
class FieldOr;
class FieldNot;
class FieldXor;
class FieldDeterminant;
class FieldEigenvalues;
class FieldEigenvectors;
class FieldMatrixInvert;
class FieldMatrixMultiply;
class FieldProjection;
class FieldTranspose;
class FieldNodesetSum;
class FieldNodesetMean;
class FieldNodesetSumSquares;
class FieldNodesetMeanSquares;
class FieldNodesetMinimum;
class FieldNodesetMaximum;
class FieldElementGroup;
class FieldNodeGroup;
class FieldTimeLookup;
class FieldTimeValue;
class FieldDerivative;
class FieldCurl;
class FieldDivergence;
class FieldGradient;
class FieldSin;
class FieldCos;
class FieldTan;
class FieldAsin;
class FieldAcos;
class FieldAtan;
class FieldAtan2;
class FieldCrossProduct;
class FieldCrossProduct3D;
class FieldDotProduct;
class FieldMagnitude;
class FieldNormalise;
class FieldBinaryDilateImageFilter;
class FieldBinaryErodeImageFilter;
class FieldBinaryThresholdImageFilter;
class FieldCannyEdgeDetectionImageFilter;
class FieldConnectedThresholdImageFilter;
class FieldCurvatureAnisotropicDiffusionImageFilter;
class FieldDiscreteGaussianImageFilter;
class FieldGradientMagnitudeRecursiveGaussianImageFilter;
class FieldRescaleIntensityImageFilter;
class FieldSigmoidImageFilter;
class TimeKeeper;
class Optimisation;

class FieldModule
{
private:

	cmzn_field_module_id id;

public:

	FieldModule() : id(0)
	{	}

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldModule(cmzn_field_module_id field_module_id) :
		id(field_module_id)
	{ }

	FieldModule(const FieldModule& fieldModule) :
		id(cmzn_field_module_access(fieldModule.id))
	{ }

	FieldModule(Field& field) :
		id(cmzn_field_get_field_module(field.getId()))
	{ }

	FieldModule& operator=(const FieldModule& fieldModule)
	{
		cmzn_field_module_id temp_id = cmzn_field_module_access(fieldModule.id);
		if (0 != id)
		{
			cmzn_field_module_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~FieldModule()
	{
		if (0 != id)
		{
			cmzn_field_module_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_field_module_id getId()
	{
		return id;
	}

	int beginChange()
	{
		return cmzn_field_module_begin_change(id);
	}

	int endChange()
	{
		return cmzn_field_module_end_change(id);
	}

	int defineAllFaces()
	{
		return cmzn_field_module_define_all_faces(id);
	}

	Field findFieldByName(const char *fieldName)
	{
		return Field(cmzn_field_module_find_field_by_name(id, fieldName));
	}

	FieldCache createCache()
	{
		return FieldCache(cmzn_field_module_create_cache(id));
	}

	ElementBasis createElementBasis(int dimension, enum ElementBasis::FunctionType functionType)
	{
		return ElementBasis(cmzn_field_module_create_element_basis(
			id, dimension, static_cast<cmzn_basis_function_type>(functionType)));
	}

	Nodeset findNodesetByDomainType(Field::DomainType domainType)
	{
		return Nodeset(cmzn_field_module_find_nodeset_by_domain_type(id,
			static_cast<cmzn_field_domain_type>(domainType)));
	}

	Nodeset findNodesetByName(const char *nodeset_name)
	{
		return Nodeset(cmzn_field_module_find_nodeset_by_name(id,
			nodeset_name));
	}

	Mesh findMeshByDimension(int dimension)
	{
		return Mesh(cmzn_field_module_find_mesh_by_dimension(id, dimension));
	}

	Mesh findMeshByName(const char *meshName)
	{
		return Mesh(cmzn_field_module_find_mesh_by_name(id, meshName));
	}

	TimeSequence getMatchingTimeSequence(int timesCount, const double *timesIn)
	{
		return TimeSequence(cmzn_field_module_get_matching_time_sequence(
			id, timesCount, timesIn));
	}

	Optimisation createOptimisation()
	{
		return Optimisation(cmzn_field_module_create_optimisation(id));
	}

	FieldAlias createAlias(Field& sourceField);

	FieldAdd createAdd(Field& sourceField1, Field& sourceField2);

	FieldPower createPower(Field& sourceField1, Field& sourceField2);

	FieldMultiply createMultiply(Field& sourceField1, Field& sourceField2);

	FieldDivide createDivide(Field& sourceField1, Field& sourceField2);

	FieldSubtract createSubtract(Field& sourceField1, Field& sourceField2);

	FieldSumComponents createSumComponents(Field& sourceField);

	FieldLog createLog(Field& sourceField);

	FieldSqrt createSqrt(Field& sourceField);

	FieldExp createExp(Field& sourceField);

	FieldAbs createAbs(Field& sourceField);

	FieldIdentity createIdentity(Field& sourceField);

	FieldComponent createComponent(Field& sourceField, int componentIndex);

	FieldConcatenate createConcatenate(int fieldsCount, Field *sourceFields);

	FieldIf createIf(Field& sourceField1, Field& sourceField2, Field& sourceField3);

	FieldConstant createConstant(int valuesCount, const double *valuesIn);

	FieldStringConstant createStringConstant(const char *stringConstant);

	FieldCoordinateTransformation createCoordinateTransformation(Field& sourceField);

	FieldVectorCoordinateTransformation createVectorCoordinateTransformation(
		Field& vectorField, Field& coordinateField);

	FieldFibreAxes createFibreAxes(Field& fibreField, Field& coordinateField);

	FieldFiniteElement createFiniteElement(int numberOfComponents);

	FieldEmbedded createEmbedded(Field& sourceField, Field& embeddedLocationField);

	FieldFindMeshLocation createFindMeshLocation(
		Field& sourceField, Field& meshField, Mesh& mesh);

	FieldNodeValue createNodeValue(Field& sourceField,
		Node::ValueType valueType, int versionNumber);

	FieldStoredMeshLocation createStoredMeshLocation(Mesh& mesh);

	FieldStoredString createStoredString();

	FieldGroup createGroup();

	FieldImage createImage();

	FieldImage createImageFromSource(Field& sourceField);

	FieldAnd createAnd(Field& sourceField1, Field& sourceField2);

	FieldEqualTo createEqualTo(Field& sourceField1, Field& sourceField2);

	FieldGreaterThan createGreaterThan(Field& sourceField1, Field& sourceField2);

	FieldLessThan createLessThan(Field& sourceField1, Field& sourceField2);

	FieldOr createOr(Field& sourceField1, Field& sourceField2);

	FieldNot createNot(Field& sourceField);

	FieldXor createXor(Field& sourceField1, Field& sourceField2);

	FieldDeterminant createDeterminant(Field& sourceField);

	FieldEigenvalues createEigenvalues(Field& sourceField);

	FieldEigenvectors createEigenvectors(FieldEigenvalues& eigenValuesField);

	FieldMatrixInvert createMatrixInvert(Field& sourceField);

	FieldMatrixMultiply createMatrixMultiply(int numberOfRows,
		Field sourceField1, Field& sourceField2);

	FieldProjection createProjection(Field& sourceField,	Field& projectionMatrixField);

	FieldTranspose createTranspose(int sourceNumberOfRows, Field& sourceField);

	FieldNodesetSum createNodesetSum(Field& sourceField, Nodeset& nodeset);

	FieldNodesetMean createNodesetMean(Field& sourceField, Nodeset& nodeset);

	FieldNodesetSumSquares createNodesetSumSquares(Field& sourceField, Nodeset& nodeset);

	FieldNodesetMeanSquares createNodesetMeanSquares(Field& sourceField, Nodeset& nodeset);

	FieldNodesetMinimum createNodesetMinimum(Field& sourceField, Nodeset& nodeset);

	FieldNodesetMaximum createNodesetMaximum(Field& sourceField, Nodeset& nodeset);

	FieldNodeGroup createNodeGroup(Nodeset& nodeset);

	FieldElementGroup createElementGroup(Mesh& mesh);

	FieldTimeLookup createTimeLookup(Field& sourceField, Field& timeField);

	FieldTimeValue createTimeValue(TimeKeeper& timeKeeper);

	FieldDerivative createDerivative(Field& sourceField, int xi_index);

	FieldCurl createCurl(Field& vectorField, Field& coordinateField);

	FieldDivergence createDivergence(Field& vectorField, Field& coordinateField);

	FieldGradient createGradient(Field& sourceField, Field& coordinateField);

	FieldSin createSin(Field& sourceField);

	FieldCos createCos(Field& sourceField);

	FieldTan createTan(Field& sourceField);

	FieldAsin createAsin(Field& sourceField);

	FieldAcos createAcos(Field& sourceField);

	FieldAtan createAtan(Field& sourceField);

	FieldAtan2 createAtan2(Field& sourceField1, Field& sourceField2);

	FieldCrossProduct createCrossProduct(int fieldsCount, Field *sourceFields);

	FieldCrossProduct createCrossProduct(Field& sourceField1, Field& sourceField2);

	FieldDotProduct createDotProduct(Field& sourceField1, Field& sourceField2);

	FieldMagnitude createMagnitude(Field& sourceField);

	FieldNormalise createNormalise(Field& sourceField);

	FieldBinaryDilateImageFilter createBinaryDilateImageFilter(Field& sourceField,
		int radius, double erode_value);

	FieldBinaryErodeImageFilter createBinaryErodeImageFilter(Field& sourceField,
		int radius, double dilate_value);

	FieldBinaryThresholdImageFilter createBinaryThresholdImageFilter(Field& sourceField,
			double lower_threshold, double upper_threshold);

	FieldCannyEdgeDetectionImageFilter createCannyEdgeDetectionImageFilter(Field& sourceField,
			double variance, double maximumError, double upperThreshold, double lowerThreshold);

	FieldConnectedThresholdImageFilter createConnectedThresholdImageFilter(Field& sourceField,
		double lowerThreshold, double upperThreshold, double replaceValue,
		int dimension, int seedPointsCount, const double *seedPoints);

	FieldCurvatureAnisotropicDiffusionImageFilter createCurvatureAnisotropicDiffusionImageFilter(
		Field& sourceField, double timeStep, double conductance, int numIterations);

	FieldDiscreteGaussianImageFilter createDiscreteGaussianImageFilter(Field& sourceField,
		double variance, int maxKernelWidth);

	FieldGradientMagnitudeRecursiveGaussianImageFilter
		createGradientMagnitudeRecursiveGaussianImageFilter(Field& sourceField,
			double sigma);

	FieldRescaleIntensityImageFilter createRescaleIntensityImageFilter(Field& sourceField,
		double outputMin, double outputMax);

	FieldSigmoidImageFilter createSigmoidImageFilter(Field& sourceField,
		double min, double max,	double alpha, double beta);
};

inline FieldModule Field::getFieldModule()
{
	return FieldModule(*this);
}

}  // namespace Zinc
}

#endif
