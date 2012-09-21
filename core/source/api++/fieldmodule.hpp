/***************************************************************************//**
 * FILE : fieldmodule.hpp
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
 * Portions created by the Initial Developer are Copyright (C) 2012
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
#ifndef __FIELD_MODULE_HPP__
#define __FIELD_MODULE_HPP__

extern "C" {
#include "api/cmiss_field_module.h"
}
#include "api++/field.hpp"
#include "api++/fieldcache.hpp"
#include "api++/element.hpp"
#include "api++/node.hpp"
#include "api++/optimisation.hpp"
#include "api++/timesequence.hpp"

namespace Zn
{

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
class FieldFiniteElement;
class FieldEmbedded;
class FieldFindMeshLocation;
class FieldNodeValue;
class FieldStoredMeshLocation;
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
class FieldElementGroup;
class FieldNodeGroup;
class FieldTimeLookup;
class FieldTimeValue;
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
class TimeKeeper;

class FieldModule
{
private:

	Cmiss_field_module_id id;

public:

	FieldModule() : id(NULL)
	{	}

	// takes ownership of C-style field module reference
	FieldModule(Cmiss_field_module_id field_module_id) :
		id(field_module_id)
	{ }

	FieldModule(const FieldModule& fieldModule) :
		id(Cmiss_field_module_access(fieldModule.id))
	{ }

	FieldModule(Field& field) :
		id(Cmiss_field_get_field_module(field.getId()))
	{ }

	FieldModule& operator=(const FieldModule& fieldModule)
	{
		Cmiss_field_module_id temp_id = Cmiss_field_module_access(fieldModule.id);
		if (NULL != id)
		{
			Cmiss_field_module_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~FieldModule()
	{
		if (NULL != id)
		{
			Cmiss_field_module_destroy(&id);
		}
	}

	Cmiss_field_module_id getId()
	{
		return id;
	}

	int beginChange()
	{
		return Cmiss_field_module_begin_change(id);
	}

	int endChange()
	{
		return Cmiss_field_module_end_change(id);
	}

	int defineAllFaces()
	{
		return Cmiss_field_module_define_all_faces(id);
	}

	Field findFieldByName(const char *fieldName)
	{
		return Field(Cmiss_field_module_find_field_by_name(id, fieldName));
	}

	int defineField(const char *fieldName, const char *commandString)
	{
		return Cmiss_field_module_define_field(id, fieldName, commandString);
	}

	Field createField(const char *fieldName, const char *commandString)
	{
		return Field(Cmiss_field_module_create_field(id, fieldName, commandString));
	}

	FieldCache createCache()
	{
		return FieldCache(Cmiss_field_module_create_cache(id));
	}

	ElementBasis createElementBasis(int dimension, enum ElementBasis::FunctionType functionType)
	{
		return ElementBasis(Cmiss_field_module_create_element_basis(
			id, dimension, static_cast<Cmiss_basis_function_type>(functionType)));
	}

	Nodeset findNodesetByName(const char *nodeset_name)
	{
		return Nodeset(Cmiss_field_module_find_nodeset_by_name(id,
			nodeset_name));
	}

	Mesh findMeshByDimension(int dimension)
	{
		return Mesh(Cmiss_field_module_find_mesh_by_dimension(id, dimension));
	}

	Mesh findMeshByName(const char *meshName)
	{
		return Mesh(Cmiss_field_module_find_mesh_by_name(id, meshName));
	}

	TimeSequence getMatchingTimeSequence(int numberOfTimes, double *times)
	{
		return TimeSequence(Cmiss_field_module_get_matching_time_sequence(
			id, numberOfTimes, times));
	}

	Optimisation createOptimisation()
	{
		return Optimisation(Cmiss_field_module_create_optimisation(id));
	}

	FieldAdd createAdd(Field& sourceField1, Field& sourceField2);

	FieldPower createPower(Field& sourceField1, Field& sourceField2);

	FieldMultiply createMultiply(Field& sourceField1, Field& sourceField2);

	FieldDivide createDivide(Field& sourceField1, Field& sourceField2);

	FieldSubtract createSubtract(Field& sourceField1, Field& sourceField2);

	FieldSumComponents createSumComponents(Field& sourceField, double *weights);

	FieldLog createLog(Field& sourceField);

	FieldSqrt createSqrt(Field& sourceField);

	FieldExp createExp(Field& sourceField);

	FieldAbs createAbs(Field& sourceField);

	FieldIdentity createIdentity(Field& sourceField);

	FieldComponent createComponent(Field& sourceField, int componentIndex);

	FieldConcatenate createConcatenate(int numberOfSourceFields, Field **sourceFields);

	FieldIf createIf(Field& sourceField1, Field& sourceField2, Field& sourceField3);

	FieldConstant createConstant(int numberOfValues, double *values);

	FieldStringConstant createStringConstant(const char *stringConstant);

	FieldCoordinateTransformation createCoordinateTransformation(Field& sourceField);

	FieldVectorCoordinateTransformation createVectorCoordinateTransformation(
		Field& vectorField, Field& coordinateField);

	FieldFiniteElement createFiniteElement(int numberOfComponents);

	FieldEmbedded createEmbedded(Field& sourceField, Field& embeddedLocationField);

	FieldFindMeshLocation createFindMeshLocation(
		Field sourceField, Field meshField, Mesh mesh);

	FieldNodeValue createNodeValue(Field sourceField,
		NodalValueType nodalValueType, int versionNumber);

	FieldStoredMeshLocation createStoredMeshLocation(Mesh mesh);

	FieldGroup createGroup();

	FieldImage createImage(Field& domain_field, Field& source_field);

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

	FieldNodeGroup createNodeGroup(Nodeset& nodeset);

	FieldElementGroup createElementGroup(Mesh& mesh);

	FieldTimeLookup createTimeLookup(Field& sourceField, Field& timeField);

	FieldTimeValue createTimeValue(TimeKeeper& timeKeeper);

	FieldSin createSin(Field& sourceField);

	FieldCos createCos(Field& sourceField);

	FieldTan createTan(Field& sourceField);

	FieldAsin createAsin(Field& sourceField);

	FieldAcos createAcos(Field& sourceField);

	FieldAtan createAtan(Field& sourceField);

	FieldAtan2 createAtan2(Field& sourceField1, Field& sourceField2);

	FieldCrossProduct createCrossProduct(int dimension, Field* sourceFields);

	FieldCrossProduct3D createCrossProduct3D(Field& sourceField1, Field& sourceField2);

	FieldDotProduct createDotProduct(Field& sourceField1, Field& sourceField2);

	FieldMagnitude createMagnitude(Field& sourceField);

	FieldNormalise createNormalise(Field& sourceField);

};

inline FieldModule Field::getFieldModule()
{
	return FieldModule(*this);
}

}  // namespace Zn

#endif /* __FIELD_MODULE_HPP__ */
