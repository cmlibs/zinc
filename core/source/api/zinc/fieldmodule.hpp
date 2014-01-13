/**
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
#include "zinc/element.hpp"
#include "zinc/node.hpp"
#include "zinc/region.hpp"
#include "zinc/timesequence.hpp"
#include "zinc/types/scenecoordinatesystem.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class Fieldcache;
class Fieldmodulenotifier;
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
class FieldImagefilterBinaryDilate;
class FieldImagefilterBinaryErode;
class FieldImagefilterBinaryThreshold;
class FieldImagefilterCannyEdgeDetection;
class FieldImagefilterConnectedThreshold;
class FieldImagefilterCurvatureAnisotropicDiffusion;
class FieldImagefilterDiscreteGaussian;
class FieldImagefilterHistogram;
class FieldImagefilterMean;
class FieldImagefilterGradientMagnitudeRecursiveGaussian;
class FieldImagefilterRescaleIntensity;
class FieldImagefilterSigmoid;
class FieldImagefilterThreshold;
class FieldSceneviewerProjection;
class Timekeeper;
class Optimisation;
class Sceneviewer;

class Fieldmodule
{
private:

	cmzn_fieldmodule_id id;

public:

	Fieldmodule() : id(0)
	{	}

	// takes ownership of C handle, responsibility for destroying it
	explicit Fieldmodule(cmzn_fieldmodule_id field_module_id) :
		id(field_module_id)
	{ }

	Fieldmodule(const Fieldmodule& fieldModule) :
		id(cmzn_fieldmodule_access(fieldModule.id))
	{ }

	Fieldmodule& operator=(const Fieldmodule& fieldModule)
	{
		cmzn_fieldmodule_id temp_id = cmzn_fieldmodule_access(fieldModule.id);
		if (0 != id)
		{
			cmzn_fieldmodule_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Fieldmodule()
	{
		if (0 != id)
		{
			cmzn_fieldmodule_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_fieldmodule_id getId()
	{
		return id;
	}

	int beginChange()
	{
		return cmzn_fieldmodule_begin_change(id);
	}

	int endChange()
	{
		return cmzn_fieldmodule_end_change(id);
	}

	int defineAllFaces()
	{
		return cmzn_fieldmodule_define_all_faces(id);
	}

	Field findFieldByName(const char *fieldName)
	{
		return Field(cmzn_fieldmodule_find_field_by_name(id, fieldName));
	}

	inline Fieldcache createFieldcache();

	Fielditerator createFielditerator()
	{
		return Fielditerator(cmzn_fieldmodule_create_fielditerator(id));
	}

	Fieldmodulenotifier createFieldmodulenotifier();

	Elementbasis createElementbasis(int dimension, enum Elementbasis::FunctionType functionType)
	{
		return Elementbasis(cmzn_fieldmodule_create_elementbasis(
			id, dimension, static_cast<cmzn_elementbasis_function_type>(functionType)));
	}

	Nodeset findNodesetByFieldDomainType(Field::DomainType domainType)
	{
		return Nodeset(cmzn_fieldmodule_find_nodeset_by_field_domain_type(id,
			static_cast<cmzn_field_domain_type>(domainType)));
	}

	Nodeset findNodesetByName(const char *nodeset_name)
	{
		return Nodeset(cmzn_fieldmodule_find_nodeset_by_name(id,
			nodeset_name));
	}

	Mesh findMeshByDimension(int dimension)
	{
		return Mesh(cmzn_fieldmodule_find_mesh_by_dimension(id, dimension));
	}

	Mesh findMeshByName(const char *meshName)
	{
		return Mesh(cmzn_fieldmodule_find_mesh_by_name(id, meshName));
	}

	inline Timesequence getMatchingTimesequence(int timesCount, const double *timesIn)
	{
		return Timesequence(cmzn_fieldmodule_get_matching_timesequence(
			id, timesCount, timesIn));
	}

	inline Optimisation createOptimisation();

	inline FieldAlias createFieldAlias(Field& sourceField);

	inline FieldAdd createFieldAdd(Field& sourceField1, Field& sourceField2);

	inline FieldPower createFieldPower(Field& sourceField1, Field& sourceField2);

	inline FieldMultiply createFieldMultiply(Field& sourceField1, Field& sourceField2);

	inline FieldDivide createFieldDivide(Field& sourceField1, Field& sourceField2);

	inline FieldSubtract createFieldSubtract(Field& sourceField1, Field& sourceField2);

	inline FieldSumComponents createFieldSumComponents(Field& sourceField);

	inline FieldLog createFieldLog(Field& sourceField);

	inline FieldSqrt createFieldSqrt(Field& sourceField);

	inline FieldExp createFieldExp(Field& sourceField);

	inline FieldAbs createFieldAbs(Field& sourceField);

	inline FieldIdentity createFieldIdentity(Field& sourceField);

	inline FieldComponent createFieldComponent(Field& sourceField, int componentIndex);

	inline FieldConcatenate createFieldConcatenate(int fieldsCount, Field *sourceFields);

	inline FieldIf createFieldIf(Field& sourceField1, Field& sourceField2, Field& sourceField3);

	inline FieldConstant createFieldConstant(int valuesCount, const double *valuesIn);

	inline FieldStringConstant createFieldStringConstant(const char *stringConstant);

	inline FieldCoordinateTransformation createFieldCoordinateTransformation(Field& sourceField);

	inline FieldVectorCoordinateTransformation createFieldVectorCoordinateTransformation(
		Field& vectorField, Field& coordinateField);

	inline FieldFibreAxes createFieldFibreAxes(Field& fibreField, Field& coordinateField);

	inline FieldFiniteElement createFieldFiniteElement(int numberOfComponents);

	inline FieldEmbedded createFieldEmbedded(Field& sourceField, Field& embeddedLocationField);

	inline FieldFindMeshLocation createFieldFindMeshLocation(
		Field& sourceField, Field& meshField, Mesh& mesh);

	inline FieldNodeValue createFieldNodeValue(Field& sourceField,
		Node::ValueLabel nodeValueLabel, int versionNumber);

	inline FieldStoredMeshLocation createFieldStoredMeshLocation(Mesh& mesh);

	inline FieldStoredString createFieldStoredString();

	inline FieldGroup createFieldGroup();

	inline FieldImage createFieldImage();

	inline FieldImage createFieldImageFromSource(Field& sourceField);

	inline FieldAnd createFieldAnd(Field& sourceField1, Field& sourceField2);

	inline FieldEqualTo createFieldEqualTo(Field& sourceField1, Field& sourceField2);

	inline FieldGreaterThan createFieldGreaterThan(Field& sourceField1, Field& sourceField2);

	inline FieldLessThan createFieldLessThan(Field& sourceField1, Field& sourceField2);

	inline FieldOr createFieldOr(Field& sourceField1, Field& sourceField2);

	inline FieldNot createFieldNot(Field& sourceField);

	inline FieldXor createFieldXor(Field& sourceField1, Field& sourceField2);

	inline FieldDeterminant createFieldDeterminant(Field& sourceField);

	inline FieldEigenvalues createFieldEigenvalues(Field& sourceField);

	inline FieldEigenvectors createFieldEigenvectors(FieldEigenvalues& eigenValuesField);

	inline FieldMatrixInvert createFieldMatrixInvert(Field& sourceField);

	inline FieldMatrixMultiply createFieldMatrixMultiply(int numberOfRows,
		Field sourceField1, Field& sourceField2);

	inline FieldProjection createFieldProjection(Field& sourceField,	Field& projectionMatrixField);

	inline FieldTranspose createFieldTranspose(int sourceNumberOfRows, Field& sourceField);

	inline FieldNodesetSum createFieldNodesetSum(Field& sourceField, Nodeset& nodeset);

	inline FieldNodesetMean createFieldNodesetMean(Field& sourceField, Nodeset& nodeset);

	inline FieldNodesetSumSquares createFieldNodesetSumSquares(Field& sourceField, Nodeset& nodeset);

	inline FieldNodesetMeanSquares createFieldNodesetMeanSquares(Field& sourceField, Nodeset& nodeset);

	inline FieldNodesetMinimum createFieldNodesetMinimum(Field& sourceField, Nodeset& nodeset);

	inline FieldNodesetMaximum createFieldNodesetMaximum(Field& sourceField, Nodeset& nodeset);

	inline FieldNodeGroup createFieldNodeGroup(Nodeset& nodeset);

	inline FieldElementGroup createFieldElementGroup(Mesh& mesh);

	inline FieldTimeLookup createFieldTimeLookup(Field& sourceField, Field& timeField);

	inline FieldTimeValue createFieldTimeValue(Timekeeper& timeKeeper);

	inline FieldDerivative createFieldDerivative(Field& sourceField, int xi_index);

	inline FieldCurl createFieldCurl(Field& vectorField, Field& coordinateField);

	inline FieldDivergence createFieldDivergence(Field& vectorField, Field& coordinateField);

	inline FieldGradient createFieldGradient(Field& sourceField, Field& coordinateField);

	inline FieldSin createFieldSin(Field& sourceField);

	inline FieldCos createFieldCos(Field& sourceField);

	inline FieldTan createFieldTan(Field& sourceField);

	inline FieldAsin createFieldAsin(Field& sourceField);

	inline FieldAcos createFieldAcos(Field& sourceField);

	inline FieldAtan createFieldAtan(Field& sourceField);

	inline FieldAtan2 createFieldAtan2(Field& sourceField1, Field& sourceField2);

	inline FieldCrossProduct createFieldCrossProduct(int fieldsCount, Field *sourceFields);

	inline FieldCrossProduct createFieldCrossProduct(Field& sourceField1, Field& sourceField2);

	inline FieldDotProduct createFieldDotProduct(Field& sourceField1, Field& sourceField2);

	inline FieldMagnitude createFieldMagnitude(Field& sourceField);

	inline FieldNormalise createFieldNormalise(Field& sourceField);

	inline FieldImagefilterBinaryDilate createFieldImagefilterBinaryDilate(Field& sourceField,
		int radius, double dilate_value);

	inline FieldImagefilterBinaryErode createFieldImagefilterBinaryErode(Field& sourceField,
		int radius, double erode_value);

	inline FieldImagefilterBinaryThreshold createFieldImagefilterBinaryThreshold(Field& sourceField);

	inline FieldImagefilterCannyEdgeDetection createFieldImagefilterCannyEdgeDetection(Field& sourceField,
			double variance, double maximumError, double upperThreshold, double lowerThreshold);

	inline FieldImagefilterConnectedThreshold createFieldImagefilterConnectedThreshold(Field& sourceField,
		double lowerThreshold, double upperThreshold, double replaceValue,
		int dimension, int seedPointsCount, const double *seedPoints);

	inline FieldImagefilterCurvatureAnisotropicDiffusion createFieldImagefilterCurvatureAnisotropicDiffusion(
		Field& sourceField, double timeStep, double conductance, int numIterations);

	inline FieldImagefilterDiscreteGaussian createFieldImagefilterDiscreteGaussian(Field& sourceField);

	inline FieldImagefilterHistogram createFieldImagefilterHistogram(Field& sourceField);

	inline FieldImagefilterMean createFieldImagefilterMean(Field& sourceField, int valuesCount,
		const int *radiusSizesIn);

	inline FieldImagefilterGradientMagnitudeRecursiveGaussian
		createFieldImagefilterGradientMagnitudeRecursiveGaussian(Field& sourceField,
			double sigma);

	inline FieldImagefilterRescaleIntensity createFieldImagefilterRescaleIntensity(Field& sourceField,
		double outputMin, double outputMax);

	inline FieldImagefilterSigmoid createFieldImagefilterSigmoid(Field& sourceField,
		double min, double max,	double alpha, double beta);

	inline FieldImagefilterThreshold createFieldImagefilterThreshold(Field& sourceField);

	inline FieldSceneviewerProjection createFieldSceneviewerProjection(
		Sceneviewer& sceneviewer, Scenecoordinatesystem fromCoordinateSystem,
		Scenecoordinatesystem toCoordinateSystem);
};

class Fieldmoduleevent
{
protected:
	cmzn_fieldmoduleevent_id id;

public:

	Fieldmoduleevent() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Fieldmoduleevent(cmzn_fieldmoduleevent_id in_fieldmodule_event_id) :
		id(in_fieldmodule_event_id)
	{  }

	Fieldmoduleevent(const Fieldmoduleevent& fieldmoduleEvent) :
		id(cmzn_fieldmoduleevent_access(fieldmoduleEvent.id))
	{  }

	Fieldmoduleevent& operator=(const Fieldmoduleevent& fieldmoduleEvent)
	{
		cmzn_fieldmoduleevent_id temp_id = cmzn_fieldmoduleevent_access(fieldmoduleEvent.id);
		if (0 != id)
			cmzn_fieldmoduleevent_destroy(&id);
		id = temp_id;
		return *this;
	}

	~Fieldmoduleevent()
	{
		if (0 != id)
		{
			cmzn_fieldmoduleevent_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_fieldmoduleevent_id getId()
	{
		return id;
	}

	Field::ChangeFlags getFieldChangeFlags(Field& field) const
	{
		return cmzn_fieldmoduleevent_get_field_change_flags(id, field.getId());
	}

	Meshchanges getMeshchanges(Mesh& mesh) const
	{
		return Meshchanges(cmzn_fieldmoduleevent_get_meshchanges(id, mesh.getId()));
	}

	Nodesetchanges getNodesetchanges(Nodeset& nodeset) const
	{
		return Nodesetchanges(cmzn_fieldmoduleevent_get_nodesetchanges(id, nodeset.getId()));
	}

	Field::ChangeFlags getSummaryFieldChangeFlags() const
	{
		return cmzn_fieldmoduleevent_get_summary_field_change_flags(id);
	}

};

/**
 * Base class functor for field module notifier callbacks:
 * - Derive from this class adding any user data required.
 * - Implement virtual operator()(const Fieldmoduleevent&) to handle callback.
 * @see Fieldmodulenotifier::setCallback()
 */
class Fieldmodulecallback
{
friend class Fieldmodulenotifier;
private:
	Fieldmodulecallback(Fieldmodulecallback&); // not implemented
	Fieldmodulecallback& operator=(Fieldmodulecallback&); // not implemented

	static void C_callback(cmzn_fieldmoduleevent_id fieldmoduleevent_id, void *callbackVoid)
	{
		Fieldmoduleevent fieldmoduleevent(cmzn_fieldmoduleevent_access(fieldmoduleevent_id));
		Fieldmodulecallback *callback = reinterpret_cast<Fieldmodulecallback *>(callbackVoid);
		(*callback)(fieldmoduleevent);
	}

  virtual void operator()(const Fieldmoduleevent &fieldmoduleevent) = 0;

protected:
	Fieldmodulecallback()
	{ }

public:
	virtual ~Fieldmodulecallback()
	{ }
};

class Fieldmodulenotifier
{
protected:
	cmzn_fieldmodulenotifier_id id;

public:

	Fieldmodulenotifier() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Fieldmodulenotifier(cmzn_fieldmodulenotifier_id in_fieldmodulenotifier_id) :
		id(in_fieldmodulenotifier_id)
	{  }

	Fieldmodulenotifier(const Fieldmodulenotifier& fieldmoduleNotifier) :
		id(cmzn_fieldmodulenotifier_access(fieldmoduleNotifier.id))
	{  }

	Fieldmodulenotifier& operator=(const Fieldmodulenotifier& fieldmoduleNotifier)
	{
		cmzn_fieldmodulenotifier_id temp_id = cmzn_fieldmodulenotifier_access(fieldmoduleNotifier.id);
		if (0 != id)
		{
			cmzn_fieldmodulenotifier_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Fieldmodulenotifier()
	{
		if (0 != id)
		{
			cmzn_fieldmodulenotifier_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_fieldmodulenotifier_id getId()
	{
		return id;
	}

	int setCallback(Fieldmodulecallback& callback)
	{
		return cmzn_fieldmodulenotifier_set_callback(id, callback.C_callback, static_cast<void*>(&callback));
	}

	int clearCallback()
	{
		return cmzn_fieldmodulenotifier_clear_callback(id);
	}
};

inline Fieldmodule Region::getFieldmodule()
{
	return Fieldmodule(cmzn_region_get_fieldmodule(id));
}

inline Fieldmodule Field::getFieldmodule()
{
	return Fieldmodule(cmzn_field_get_fieldmodule(id));
}

inline Fieldmodulenotifier Fieldmodule::createFieldmodulenotifier()
{
	return Fieldmodulenotifier(cmzn_fieldmodule_create_fieldmodulenotifier(id));
}

}  // namespace Zinc
}

#endif
