/***************************************************************************//**
 * FILE : graphics_json_io.cpp
 *
 * The definition to graphics_json_io.
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "computed_field/computed_field.h"
#include "computed_field/computed_field_apply.hpp"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_derivatives.h"
#include "computed_field/computed_field_matrix_operators.hpp"
#include "description_io/field_json_io.hpp"
#include "general/debug.h"
#include "cmlibs/zinc/changemanager.hpp"
#include "cmlibs/zinc/element.hpp"
#include "cmlibs/zinc/fieldalias.hpp"
#include "cmlibs/zinc/fieldapply.hpp"
#include "cmlibs/zinc/fieldarithmeticoperators.hpp"
#include "cmlibs/zinc/fieldcomposite.hpp"
#include "cmlibs/zinc/fieldconditional.hpp"
#include "cmlibs/zinc/fieldconstant.hpp"
#include "cmlibs/zinc/fieldcoordinatetransformation.hpp"
#include "cmlibs/zinc/fieldderivatives.hpp"
#include "cmlibs/zinc/fieldfibres.hpp"
#include "cmlibs/zinc/fieldfiniteelement.hpp"
#include "cmlibs/zinc/fieldlogicaloperators.hpp"
#include "cmlibs/zinc/fieldmatrixoperators.hpp"
#include "cmlibs/zinc/fieldmeshoperators.hpp"
#include "cmlibs/zinc/fieldnodesetoperators.hpp"
#include "cmlibs/zinc/fieldmodule.hpp"
#include "cmlibs/zinc/fieldtime.hpp"
#include "cmlibs/zinc/fieldtrigonometry.hpp"
#include "cmlibs/zinc/fieldvectoroperators.hpp"
#include "cmlibs/zinc/field.hpp"
#include "cmlibs/zinc/field.h"
#include "cmlibs/zinc/fieldcache.hpp"
#include "cmlibs/zinc/region.hpp"
#include <cstdio>
#include <cstring>
#include <vector>

/*
 * header not yet supported:
 * fieldgroup
 * fieldimage
 * fieldimageprocessing
 *
 */
CMLibs::Zinc::Field *getSourceFields(const Json::Value &typeSettings, unsigned int *count,
	FieldmoduleJsonImport *jsonImport)
{
	unsigned int numberOfSourceFields = 0;

	CMLibs::Zinc::Field *sourceFields = 0;

	if (typeSettings["SourceFields"].isArray() &&
		typeSettings["SourceFields"].size() > 0)
	{
		numberOfSourceFields = typeSettings["SourceFields"].size();
		sourceFields = new CMLibs::Zinc::Field[numberOfSourceFields];
		for (unsigned int i = 0; i < numberOfSourceFields; i++)
		{
			const char *sourceFieldName = typeSettings["SourceFields"][i].asCString();
			sourceFields[i] = jsonImport->getFieldByName(sourceFieldName);
		}
	}
	*count = numberOfSourceFields;

	return sourceFields;
}

/* Deserialise apply/argument fields */
CMLibs::Zinc::Field importApplyField(enum cmzn_field_type type,
	CMLibs::Zinc::Fieldmodule &fieldmodule, const Json::Value &typeSettings,
	FieldmoduleJsonImport *jsonImport)
{
	CMLibs::Zinc::Field field;
	switch (type)
	{
	case CMZN_FIELD_TYPE_APPLY:
	{
		// FieldApply is uniquely able to use source fields from another region
		CMLibs::Zinc::Region region = jsonImport->getRegion();
		// Following are only valid if evaluate and argument fields are from a different region
		CMLibs::Zinc::Region evaluateRegion;
		CMLibs::Zinc::Fieldmodule evaluateFieldmodule;
		if (typeSettings["EvaluateRegionPath"].isString())
		{
			// relative path from this region
			const char *evaluateRegionPath = typeSettings["EvaluateRegionPath"].asCString();
			evaluateRegion = region.findSubregionAtPath(evaluateRegionPath);
			if (!evaluateRegion.isValid())
			{
				evaluateRegion = region.createSubregion(evaluateRegionPath);
			}
			if (evaluateRegion.isValid())
			{
				evaluateFieldmodule = evaluateRegion.getFieldmodule();
			}
			else
			{
				display_message(ERROR_MESSAGE, "Fieldmodule readDescription.  FieldApply failed to get region at relative path %s", evaluateRegionPath);
				return field;
			}
		}
		// cache change messages as may not be active in evaluateRegion
		CMLibs::Zinc::ChangeManager<CMLibs::Zinc::Fieldmodule> changeFields(evaluateFieldmodule.isValid() ? evaluateFieldmodule : fieldmodule);
		CMLibs::Zinc::Field evaluateField;
		const Json::ArrayIndex sourceFieldsCount = typeSettings["SourceFields"].isArray() ? typeSettings["SourceFields"].size() : 0;
		// must be an odd number of source fields: evaluate field followed by bound argument-source field pairs
		if ((sourceFieldsCount % 2) == 0)
		{
			return field;
		}
		const char *evaluateFieldName = typeSettings["SourceFields"][0].asCString();
		if (evaluateRegion.isValid())
		{
			// number of components is needed to verify source field or create a placeholder if it does not yet exist
			const int numberOfComponents = (typeSettings["NumberOfComponents"].isInt()) ?
				typeSettings["NumberOfComponents"].asInt() : 0;
			evaluateField = evaluateFieldmodule.findFieldByName(evaluateFieldName);
			if (evaluateField.isValid())
			{
				if (evaluateField.getNumberOfComponents() != numberOfComponents)
				{
					display_message(ERROR_MESSAGE, "Fieldmodule readDescription.  FieldApply evaluate field %s has wrong number of components, %d expected",
						evaluateFieldName, numberOfComponents);
					return field;
				}
			}
			else
			{
				// create a dummy real field with the number of components
				evaluateField = CMLibs::Zinc::Field(cmzn_fieldmodule_create_field_dummy_real(evaluateFieldmodule.getId(), numberOfComponents));
				evaluateField.setName(evaluateFieldName);
			}
		}
		else
		{
			evaluateField = jsonImport->getFieldByName(evaluateFieldName);
		}
		CMLibs::Zinc::FieldApply fieldApply = fieldmodule.createFieldApply(evaluateField);
		if (!fieldApply.isValid())
		{
			display_message(ERROR_MESSAGE, "Fieldmodule readDescription.  Failed to create FieldApply with evaluate field %s", evaluateFieldName);
			return field;
		}
		for (Json::ArrayIndex i = 1; i < sourceFieldsCount; i += 2)
		{
			const char *argumentFieldName = typeSettings["SourceFields"][i].asCString();
			const char *sourceFieldName = typeSettings["SourceFields"][i + 1].asCString();
			// source field needs to exist in this region
			CMLibs::Zinc::Field sourceField = jsonImport->getFieldByName(sourceFieldName);
			CMLibs::Zinc::Field argumentField;
			if (evaluateRegion.isValid())
			{
				argumentField = evaluateFieldmodule.findFieldByName(argumentFieldName);
				if (!argumentField.isValid())
				{
					// create an ArgumentReal field with the number of components in source field
					const int numberOfComponents = sourceField.getNumberOfComponents();
					argumentField = evaluateFieldmodule.createFieldArgumentReal(numberOfComponents);
					argumentField.setName(argumentFieldName);
				}
			}
			else
			{
				argumentField = jsonImport->getFieldByName(argumentFieldName);
			}
			if (CMZN_OK != fieldApply.setBindArgumentSourceField(argumentField, sourceField))
			{
				display_message(ERROR_MESSAGE, "Fieldmodule readDescription.  FieldApply failed to set bind argument source field %d", (i + 1)/2);
				return field;
			}
		}
		field = fieldApply;
	}	break;
	case CMZN_FIELD_TYPE_ARGUMENT_REAL:
		if (typeSettings["NumberOfComponents"].isInt())
		{
			const int numberOfComponents = typeSettings["NumberOfComponents"].asInt();
			field = fieldmodule.createFieldArgumentReal(numberOfComponents);
		}
		break;
	default:
		break;
	}
	return field;
}

/* Deserialise field with one source field */
CMLibs::Zinc::Field importGenericOneSourcesField(enum cmzn_field_type type,
	CMLibs::Zinc::Fieldmodule &fieldmodule, const Json::Value &typeSettings,
	FieldmoduleJsonImport *jsonImport)
{
	unsigned int sourcesCount = 0;
	CMLibs::Zinc::Field field(0);
	CMLibs::Zinc::Field *sourcefields = getSourceFields(typeSettings, &sourcesCount,
		jsonImport);
	if (sourcesCount == 1)
	{
		switch (type)
		{
			case CMZN_FIELD_TYPE_LOG:
				field = fieldmodule.createFieldLog(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_SQRT:
				field = fieldmodule.createFieldSqrt(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_EXP:
				field = fieldmodule.createFieldExp(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_ABS:
				field = fieldmodule.createFieldAbs(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_IDENTITY:
				field = fieldmodule.createFieldIdentity(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_COORDINATE_TRANSFORMATION:
			{
				field = fieldmodule.createFieldCoordinateTransformation(sourcefields[0]);
				break;
			}
			case CMZN_FIELD_TYPE_IS_DEFINED:
				field = fieldmodule.createFieldIsDefined(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_NOT:
				field = fieldmodule.createFieldNot(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_DETERMINANT:
				field = fieldmodule.createFieldDeterminant(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_EIGENVALUES:
				field = fieldmodule.createFieldEigenvalues(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_EIGENVECTORS:
				field = fieldmodule.createFieldEigenvectors(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_MATRIX_INVERT:
				field = fieldmodule.createFieldMatrixInvert(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_TRANSPOSE:
			{
				if (typeSettings["SourceNumberOfRows"].isInt())
				{
					field = fieldmodule.createFieldTranspose(typeSettings["SourceNumberOfRows"].asInt(),
						sourcefields[0]);
				}
			}	break;
			case CMZN_FIELD_TYPE_SIN:
				field = fieldmodule.createFieldSin(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_COS:
				field = fieldmodule.createFieldCos(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_TAN:
				field = fieldmodule.createFieldTan(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_ASIN:
				field = fieldmodule.createFieldAsin(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_ACOS:
				field = fieldmodule.createFieldAcos(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_ATAN:
				field = fieldmodule.createFieldAtan(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_MAGNITUDE:
				field = fieldmodule.createFieldMagnitude(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_NORMALISE:
				field = fieldmodule.createFieldNormalise(sourcefields[0]);
				break;
			case CMZN_FIELD_TYPE_SUM_COMPONENTS:
				field = fieldmodule.createFieldSumComponents(sourcefields[0]);
				break;
			default:
				break;
		}
	}
	delete[] sourcefields;
	return field;
}

/* Deserialise field with two source fields */
CMLibs::Zinc::Field importGenericTwoSourcesField(enum cmzn_field_type type,
	CMLibs::Zinc::Fieldmodule &fieldmodule, const Json::Value &typeSettings,
	FieldmoduleJsonImport *jsonImport)
{
	unsigned int sourcesCount = 0;
	CMLibs::Zinc::Field field(0);
	CMLibs::Zinc::Field *sourcefields = getSourceFields(typeSettings, &sourcesCount,
		jsonImport);
	if (sourcesCount == 2)
	{
		switch (type)
		{
			case CMZN_FIELD_TYPE_ADD:
				field = fieldmodule.createFieldAdd(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_POWER:
				field = fieldmodule.createFieldPower(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_MULTIPLY:
				field = fieldmodule.createFieldMultiply(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_DIVIDE:
				field = fieldmodule.createFieldDivide(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_SUBTRACT:
				field = fieldmodule.createFieldSubtract(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_VECTOR_COORDINATE_TRANSFORMATION:
				field = fieldmodule.createFieldVectorCoordinateTransformation(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_CURL:
				field = fieldmodule.createFieldCurl(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_DIVERGENCE:
				field = fieldmodule.createFieldDivergence(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_GRADIENT:
				field = fieldmodule.createFieldGradient(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_FIBRE_AXES:
				field = fieldmodule.createFieldFibreAxes(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_EMBEDDED:
				field = fieldmodule.createFieldEmbedded(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_AND:
				field = fieldmodule.createFieldAnd(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_EQUAL_TO:
				field = fieldmodule.createFieldEqualTo(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_GREATER_THAN:
				field = fieldmodule.createFieldGreaterThan(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_LESS_THAN:
				field = fieldmodule.createFieldLessThan(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_OR:
				field = fieldmodule.createFieldOr(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_XOR:
				field = fieldmodule.createFieldXor(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_PROJECTION:
				field = fieldmodule.createFieldProjection(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_MATRIX_MULTIPLY:
			{
				if (typeSettings["NumberOfRows"].isInt())
				{
					field = fieldmodule.createFieldMatrixMultiply(
						typeSettings["NumberOfRows"].asInt(), sourcefields[0], sourcefields[1]);
				}
			}	break;
			case CMZN_FIELD_TYPE_TIME_LOOKUP:
				field = fieldmodule.createFieldTimeLookup(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_ATAN2:
				field = fieldmodule.createFieldAtan2(sourcefields[0], sourcefields[1]);
				break;
			case CMZN_FIELD_TYPE_DOT_PRODUCT:
				field = fieldmodule.createFieldDotProduct(sourcefields[0], sourcefields[1]);
				break;
			default:
				break;
		}

	}
	delete[] sourcefields;
	return field;
}

/* Deserialise field with three source fields */
CMLibs::Zinc::Field importGenericThreeSourcesField(enum cmzn_field_type type,
	CMLibs::Zinc::Fieldmodule &fieldmodule, const Json::Value &typeSettings,
	FieldmoduleJsonImport *jsonImport)
{
	unsigned int sourcesCount = 0;
	CMLibs::Zinc::Field field(0);
	CMLibs::Zinc::Field *sourcefields = getSourceFields(typeSettings, &sourcesCount,
		jsonImport);
	if (sourcesCount == 3)
	{
		switch (type)
		{
			case CMZN_FIELD_TYPE_IF:
				field = fieldmodule.createFieldIf(sourcefields[0], sourcefields[1], sourcefields[2]);
				break;
			default:
				break;
		}

	}
	delete[] sourcefields;
	return field;
}

/* Deserialise component and concatenate fields */
CMLibs::Zinc::Field importCompositeField(enum cmzn_field_type type,
	CMLibs::Zinc::Fieldmodule &fieldmodule, const Json::Value &typeSettings,
	FieldmoduleJsonImport *jsonImport)
{
	unsigned int sourcesCount = 0;
	CMLibs::Zinc::Field field(0);
	CMLibs::Zinc::Field *sourcefields = getSourceFields(typeSettings, &sourcesCount,
		jsonImport);
	if (sourcesCount > 0)
	{
		switch (type)
		{
			case CMZN_FIELD_TYPE_COMPONENT:
				if (typeSettings.isObject())
				{
					if (typeSettings["SourceComponentIndexes"].isArray())
					{
						int componentCount = typeSettings["SourceComponentIndexes"].size();
						if (componentCount == 1)
							field = fieldmodule.createFieldComponent(
								sourcefields[0], typeSettings["SourceComponentIndexes"][0].asInt());
						if (componentCount > 1)
						{
							int *indexes;
							indexes = new int[componentCount];
							for (int i = 0; i < componentCount; i++)
								indexes[i] = typeSettings["SourceComponentIndexes"][i].asInt();
							field = fieldmodule.createFieldComponent(sourcefields[0],
								componentCount, indexes);
							delete[] indexes;
						}
					}
				}	break;
			case CMZN_FIELD_TYPE_CONCATENATE:
				field = fieldmodule.createFieldConcatenate(sourcesCount, sourcefields);
				break;
			default:
				break;
		}
	}
	delete[] sourcefields;
	return field;
}

/* Deserialise constant fields */
CMLibs::Zinc::Field importConstantField(enum cmzn_field_type type,
	CMLibs::Zinc::Fieldmodule &fieldmodule, const Json::Value &typeSettings)
{
	CMLibs::Zinc::Field field(0);

	switch (type)
	{
		case CMZN_FIELD_TYPE_CONSTANT:
		{
			if (typeSettings["Values"].isArray())
			{
				unsigned int count = typeSettings["Values"].size();
				double *values;
				values = new double[count];
				for (unsigned int i = 0; i < count; i++)
					values[i] = typeSettings["Values"][i].asDouble();
				field = fieldmodule.createFieldConstant(count, values);
				delete[] values;
			}
		} break;
		case CMZN_FIELD_TYPE_STRING_CONSTANT:
		{
			if (typeSettings["StringValue"].isString())
				field = fieldmodule.createFieldStringConstant(
					typeSettings["StringValue"].asCString());
		} break;
		default:
			break;
	}
	return field;
}

/* Deserialise derivative fields */
CMLibs::Zinc::Field importDerivativeField(enum cmzn_field_type type,
	CMLibs::Zinc::Fieldmodule &fieldmodule, const Json::Value &typeSettings,
	FieldmoduleJsonImport *jsonImport)
{
	unsigned int sourcesCount = 0;
	CMLibs::Zinc::Field field(0);
	CMLibs::Zinc::Field *sourcefields = getSourceFields(typeSettings, &sourcesCount,
		jsonImport);
	if (sourcesCount == 1)
	{
		switch (type)
		{
			case CMZN_FIELD_TYPE_DERIVATIVE:
			{
				if (typeSettings["XiIndexes"].isArray())
				{
					unsigned int size = typeSettings["XiIndexes"].size();
					if (size == 1)
					{
						field = fieldmodule.createFieldDerivative(sourcefields[0],
							typeSettings["XiIndexes"][0].asInt());
					}
				}
			} break;
			default:
				break;
		}
	}
	delete[] sourcefields;
	return field;
}

/* Deserialise finite element fields */
CMLibs::Zinc::Field importFiniteElementField(enum cmzn_field_type type,
	CMLibs::Zinc::Fieldmodule &fieldmodule, const Json::Value &fieldSettings, const Json::Value &typeSettings,
	FieldmoduleJsonImport *jsonImport)
{
	CMLibs::Zinc::Field field;
	switch (type)
	{
		case CMZN_FIELD_TYPE_FINITE_ELEMENT:
			if (typeSettings["NumberOfComponents"].isInt())
			{
				const int numberOfComponents = typeSettings["NumberOfComponents"].asInt();
				CMLibs::Zinc::FieldFiniteElement fieldFiniteElement = fieldmodule.createFieldFiniteElement(numberOfComponents);
				if (fieldSettings["IsTypeCoordinate"].isBool())
					fieldFiniteElement.setTypeCoordinate(fieldSettings["IsTypeCoordinate"].asBool());
				if (typeSettings["ComponentNames"].isArray())
				{
					unsigned int numberOfComponentNames = typeSettings["ComponentNames"].size();
					for (unsigned int i = 0; i < numberOfComponentNames; i++)
					{
						fieldFiniteElement.setComponentName(i + 1, typeSettings["ComponentNames"][i].asCString());
					}
				}
				field = fieldFiniteElement;
			} break;
		case CMZN_FIELD_TYPE_EMBEDDED:
			field = importGenericTwoSourcesField(type, fieldmodule, typeSettings, jsonImport);
			break;
		case CMZN_FIELD_TYPE_STORED_STRING:
			field = fieldmodule.createFieldStoredString();
			break;
		case CMZN_FIELD_TYPE_IS_EXTERIOR:
			field = fieldmodule.createFieldIsExterior();
			break;
		case CMZN_FIELD_TYPE_IS_ON_FACE:
		{
			if (typeSettings["ElementFaceType"].isString())
			{
				CMLibs::Zinc::Element::FaceType elementFaceType =
					CMLibs::Zinc::Element::FaceTypeEnumFromString(typeSettings["ElementFaceType"].asCString());
				field = fieldmodule.createFieldIsOnFace(elementFaceType);
			}
		}	break;
		case CMZN_FIELD_TYPE_EDGE_DISCONTINUITY:
		{
			unsigned int sourcesCount = 0;
			CMLibs::Zinc::Field *sourcefields = getSourceFields(typeSettings, &sourcesCount,
				jsonImport);
			if (sourcesCount > 0)
			{
				CMLibs::Zinc::FieldEdgeDiscontinuity derivedField = fieldmodule.createFieldEdgeDiscontinuity(
					sourcefields[0]);
				if (sourcesCount == 2)
				{
					derivedField.setConditionalField(sourcefields[1]);
				}
				if (typeSettings["Measure"].isString())
					derivedField.setMeasure(static_cast<CMLibs::Zinc::FieldEdgeDiscontinuity::Measure>(
						cmzn_field_edge_discontinuity_measure_enum_from_string(
							typeSettings["Measure"].asCString())));
				field = derivedField;
			}
			delete[] sourcefields;
		}	break;
		case CMZN_FIELD_TYPE_NODE_VALUE:
		{
			unsigned int sourcesCount = 0;
			CMLibs::Zinc::Field *sourcefields = getSourceFields(typeSettings, &sourcesCount, jsonImport);
			if ((sourcesCount == 1) && typeSettings["NodeValueLabel"].isString() && typeSettings["VersionNumber"].isInt())
			{
				CMLibs::Zinc::Node::ValueLabel nodeValueLabel = CMLibs::Zinc::Node::ValueLabelEnumFromString(
					typeSettings["NodeValueLabel"].asCString());
				const int versionNumber = typeSettings["VersionNumber"].asInt();
				field = fieldmodule.createFieldNodeValue(sourcefields[0], nodeValueLabel, versionNumber);
			}
			delete[] sourcefields;
		}	break;
		case CMZN_FIELD_TYPE_STORED_MESH_LOCATION:
		{
			if (typeSettings["Mesh"].isString())
			{
				CMLibs::Zinc::Mesh mesh = fieldmodule.findMeshByName(typeSettings["Mesh"].asCString());
				field = fieldmodule.createFieldStoredMeshLocation(mesh);
			}
		}	break;
		case CMZN_FIELD_TYPE_FIND_MESH_LOCATION:
		{
			unsigned int sourcesCount = 0;
			CMLibs::Zinc::Field *sourcefields = getSourceFields(typeSettings, &sourcesCount,
				jsonImport);
			if (sourcesCount == 2 && typeSettings["Mesh"].isString() && typeSettings["SearchMode"].isString())
			{
				CMLibs::Zinc::Mesh mesh = fieldmodule.findMeshByName(typeSettings["Mesh"].asCString());
				CMLibs::Zinc::FieldFindMeshLocation::SearchMode searchMode;
				// migrate legacy non-standard SearchMode names to enums
				const char *searchModeName = typeSettings["SearchMode"].asCString();
				if (0 == strcmp("FIND_EXACT", searchModeName))
					searchMode = CMLibs::Zinc::FieldFindMeshLocation::SEARCH_MODE_EXACT;
				else if (0 == strcmp("FIND_NEAREST", searchModeName))
					searchMode = CMLibs::Zinc::FieldFindMeshLocation::SEARCH_MODE_NEAREST;
				else
					searchMode = CMLibs::Zinc::FieldFindMeshLocation::SearchModeEnumFromString(searchModeName);
				CMLibs::Zinc::FieldFindMeshLocation fieldFindMeshLocation =
					fieldmodule.createFieldFindMeshLocation(sourcefields[0], sourcefields[1], mesh);
				if (typeSettings["SearchMesh"].isString())
				{
					CMLibs::Zinc::Mesh searchMesh = fieldmodule.findMeshByName(typeSettings["SearchMesh"].asCString());
					fieldFindMeshLocation.setSearchMesh(searchMesh);
				}
				fieldFindMeshLocation.setSearchMode(searchMode);
				field = fieldFindMeshLocation;
			}
			delete[] sourcefields;
		} break;
		default:
			break;

	}
	return field;
}

CMLibs::Zinc::Field importMeshOperatorsField(enum cmzn_field_type type,
	CMLibs::Zinc::Fieldmodule &fieldmodule, const Json::Value &fieldSettings, const Json::Value &typeSettings,
	FieldmoduleJsonImport *jsonImport)
{
    USE_PARAMETER(fieldSettings);
	CMLibs::Zinc::Field field;
	switch (type)
	{
	case CMZN_FIELD_TYPE_MESH_INTEGRAL:
	case CMZN_FIELD_TYPE_MESH_INTEGRAL_SQUARES:
	{
		unsigned int sourcesCount = 0;
		CMLibs::Zinc::Field *sourcefields = getSourceFields(typeSettings, &sourcesCount, jsonImport);
		if ((sourcesCount == 2)
			&& typeSettings["Mesh"].isString()
			&& typeSettings["ElementQuadratureRule"].isString()
			&& typeSettings["NumbersOfPoints"].isArray())
		{
			CMLibs::Zinc::Mesh mesh = fieldmodule.findMeshByName(typeSettings["Mesh"].asCString());
			// FieldMeshIntegral is the base class for FieldMeshIntegralSquares
			CMLibs::Zinc::FieldMeshIntegral fieldMeshIntegral =
				(type == CMZN_FIELD_TYPE_MESH_INTEGRAL) ? fieldmodule.createFieldMeshIntegral(sourcefields[0], sourcefields[1], mesh) :
				(type == CMZN_FIELD_TYPE_MESH_INTEGRAL_SQUARES) ? fieldmodule.createFieldMeshIntegralSquares(sourcefields[0], sourcefields[1], mesh) :
				CMLibs::Zinc::FieldMeshIntegral();
			CMLibs::Zinc::Element::QuadratureRule quadratureRule =
				CMLibs::Zinc::Element::QuadratureRuleEnumFromString(typeSettings["ElementQuadratureRule"].asCString());
			fieldMeshIntegral.setElementQuadratureRule(quadratureRule);
			const int numbersCount = typeSettings["NumbersOfPoints"].size();
			int *numbersOfPoints = new int[numbersCount];
			for (int i = 0; i < numbersCount; i++)
			{
				numbersOfPoints[i] = typeSettings["NumbersOfPoints"][i].asInt();
			}
			fieldMeshIntegral.setNumbersOfPoints(numbersCount, numbersOfPoints);
			delete[] numbersOfPoints;
			field = fieldMeshIntegral;
		}
		delete[] sourcefields;
	} break;
	default:
		break;
	}
	return field;
}

CMLibs::Zinc::Field importNodesetOperatorsField(enum cmzn_field_type type,
	CMLibs::Zinc::Fieldmodule &fieldmodule, const Json::Value &fieldSettings, const Json::Value &typeSettings,
	FieldmoduleJsonImport *jsonImport)
{
    USE_PARAMETER(fieldSettings);
	CMLibs::Zinc::Field field;
	switch (type)
	{
	case CMZN_FIELD_TYPE_NODESET_MAXIMUM:
	case CMZN_FIELD_TYPE_NODESET_MEAN:
	case CMZN_FIELD_TYPE_NODESET_MEAN_SQUARES:
	case CMZN_FIELD_TYPE_NODESET_MINIMUM:
	case CMZN_FIELD_TYPE_NODESET_SUM:
	case CMZN_FIELD_TYPE_NODESET_SUM_SQUARES:
	{
		unsigned int sourcesCount = 0;
		CMLibs::Zinc::Field *sourcefields = getSourceFields(typeSettings, &sourcesCount, jsonImport);
		if ((sourcesCount >= 1) && typeSettings["Nodeset"].isString())
		{
			CMLibs::Zinc::Nodeset nodeset = fieldmodule.findNodesetByName(typeSettings["Nodeset"].asCString());
			// FieldNodesetOperator is the base class containing all API for all these types
			CMLibs::Zinc::FieldNodesetOperator fieldNodesetOperator =
				(type == CMZN_FIELD_TYPE_NODESET_MAXIMUM) ? fieldmodule.createFieldNodesetMaximum(sourcefields[0], nodeset) :
				(type == CMZN_FIELD_TYPE_NODESET_MEAN) ? fieldmodule.createFieldNodesetMean(sourcefields[0], nodeset) :
				(type == CMZN_FIELD_TYPE_NODESET_MEAN_SQUARES) ? fieldmodule.createFieldNodesetMeanSquares(sourcefields[0], nodeset) :
				(type == CMZN_FIELD_TYPE_NODESET_MINIMUM) ? fieldmodule.createFieldNodesetMinimum(sourcefields[0], nodeset) :
				(type == CMZN_FIELD_TYPE_NODESET_SUM) ? fieldmodule.createFieldNodesetSum(sourcefields[0], nodeset) :
				(type == CMZN_FIELD_TYPE_NODESET_SUM_SQUARES) ? fieldmodule.createFieldNodesetSumSquares(sourcefields[0], nodeset) :
				CMLibs::Zinc::FieldNodesetOperator();
			if (typeSettings["ElementMapField"].isString())
			{
				// optional element map field is also in source fields; store separately to support future optional fields
				const char *elementMapFieldName = typeSettings["ElementMapField"].asCString();
				CMLibs::Zinc::Field elementMapField = jsonImport->getFieldByName(elementMapFieldName);
				fieldNodesetOperator.setElementMapField(elementMapField);
			}
			field = fieldNodesetOperator;
		}
		delete[] sourcefields;
	} break;
	default:
		break;
	}
	return field;
}

/* Deserialise field with varying number of fields */
CMLibs::Zinc::Field importGenericMultiComponentField(enum cmzn_field_type type,
	CMLibs::Zinc::Fieldmodule &fieldmodule, const Json::Value &typeSettings,
	FieldmoduleJsonImport *jsonImport)
{
	unsigned int sourcesCount = 0;
	CMLibs::Zinc::Field field(0);
	CMLibs::Zinc::Field *sourcefields = getSourceFields(typeSettings, &sourcesCount,
		jsonImport);
	if (sourcesCount > 0)
	{
		switch (type)
		{
			case CMZN_FIELD_TYPE_CROSS_PRODUCT:
				field = fieldmodule.createFieldCrossProduct(sourcesCount, sourcefields);
				break;
			default:
				break;
		}
	}
	delete[] sourcefields;
	return field;

}

/*
 * Get the json object describing the derived field settings, it will also return
 * the field type in argument.
 */
const Json::Value getDerivedFieldValue(const Json::Value &fieldSettings, enum cmzn_field_type *type)
{
	Json::Value typeSettings;
	Json::Value::Members members = fieldSettings.getMemberNames();
	Json::Value::Members::iterator it = members.begin();
	*type = CMZN_FIELD_TYPE_INVALID;
	while (it != members.end() && (*type == CMZN_FIELD_TYPE_INVALID))
	{
		const char *typeName = (*it).c_str();
		*type = cmzn_field_type_enum_from_class_name(typeName);
		if (*type != CMZN_FIELD_TYPE_INVALID)
		{
			typeSettings = fieldSettings[(*it)];
		}
		else if (0 == strcmp(typeName, "FieldAlias"))
		{
			// replaced with FieldApply
			*type = CMZN_FIELD_TYPE_APPLY;
			typeSettings = fieldSettings[(*it)];
		}
		++it;
	}
	return typeSettings;
}

CMLibs::Zinc::Field importTimeValueField(enum cmzn_field_type type,
	CMLibs::Zinc::Fieldmodule &fieldmodule, const Json::Value &typeSettings,
	FieldmoduleJsonImport *jsonImport)
{
    USE_PARAMETER(typeSettings);
    USE_PARAMETER(jsonImport);

	CMLibs::Zinc::Field field(0);
	switch (type)
	{
		case CMZN_FIELD_TYPE_TIME_VALUE:
		{
			CMLibs::Zinc::Region region = fieldmodule.getRegion();
			cmzn_context_id context = region.getId()->getContext();
			CMLibs::Zinc::Timekeepermodule tm(cmzn_context_get_timekeepermodule(context));
			CMLibs::Zinc::Timekeeper timekeeper = tm.getDefaultTimekeeper();
			field = fieldmodule.createFieldTimeValue(timekeeper);
		} break;
		default:
			break;
	}
	return field;
}

/* Deserialise type specifc field */
CMLibs::Zinc::Field importTypeSpecificField(
	CMLibs::Zinc::Fieldmodule &fieldmodule, const Json::Value &fieldSettings,
	FieldmoduleJsonImport *jsonImport)
{
	CMLibs::Zinc::Field field(0);
	enum cmzn_field_type type = CMZN_FIELD_TYPE_INVALID;
	const Json::Value typeSettings = getDerivedFieldValue(fieldSettings, &type);
	switch (type)
	{
		case CMZN_FIELD_TYPE_APPLY:
		case CMZN_FIELD_TYPE_ARGUMENT_REAL:
			field = importApplyField(type, fieldmodule, typeSettings, jsonImport);
			break;
		case CMZN_FIELD_TYPE_LOG:
		case CMZN_FIELD_TYPE_SQRT:
		case CMZN_FIELD_TYPE_EXP:
		case CMZN_FIELD_TYPE_ABS:
		case CMZN_FIELD_TYPE_IDENTITY:
		case CMZN_FIELD_TYPE_COORDINATE_TRANSFORMATION:
		case CMZN_FIELD_TYPE_IS_DEFINED:
		case CMZN_FIELD_TYPE_NOT:
		case CMZN_FIELD_TYPE_DETERMINANT:
		case CMZN_FIELD_TYPE_EIGENVALUES:
		case CMZN_FIELD_TYPE_EIGENVECTORS:
		case CMZN_FIELD_TYPE_MATRIX_INVERT:
		case CMZN_FIELD_TYPE_TRANSPOSE:
		case CMZN_FIELD_TYPE_SIN:
		case CMZN_FIELD_TYPE_COS:
		case CMZN_FIELD_TYPE_TAN:
		case CMZN_FIELD_TYPE_ASIN:
		case CMZN_FIELD_TYPE_ACOS:
		case CMZN_FIELD_TYPE_ATAN:
		case CMZN_FIELD_TYPE_MAGNITUDE:
		case CMZN_FIELD_TYPE_NORMALISE:
		case CMZN_FIELD_TYPE_SUM_COMPONENTS:
			field = importGenericOneSourcesField(type, fieldmodule, typeSettings, jsonImport);
			break;
		case CMZN_FIELD_TYPE_ADD:
		case CMZN_FIELD_TYPE_POWER:
		case CMZN_FIELD_TYPE_MULTIPLY:
		case CMZN_FIELD_TYPE_DIVIDE:
		case CMZN_FIELD_TYPE_SUBTRACT:
		case CMZN_FIELD_TYPE_VECTOR_COORDINATE_TRANSFORMATION:
		case CMZN_FIELD_TYPE_CURL:
		case CMZN_FIELD_TYPE_DIVERGENCE:
		case CMZN_FIELD_TYPE_GRADIENT:
		case CMZN_FIELD_TYPE_FIBRE_AXES:
		case CMZN_FIELD_TYPE_AND:
		case CMZN_FIELD_TYPE_EQUAL_TO:
		case CMZN_FIELD_TYPE_GREATER_THAN:
		case CMZN_FIELD_TYPE_LESS_THAN:
		case CMZN_FIELD_TYPE_OR:
		case CMZN_FIELD_TYPE_XOR:
		case CMZN_FIELD_TYPE_PROJECTION:
		case CMZN_FIELD_TYPE_MATRIX_MULTIPLY:
		case CMZN_FIELD_TYPE_TIME_LOOKUP:
		case CMZN_FIELD_TYPE_ATAN2:
		case CMZN_FIELD_TYPE_DOT_PRODUCT:
			field = importGenericTwoSourcesField(type, fieldmodule, typeSettings, jsonImport);
			break;
		case CMZN_FIELD_TYPE_COMPONENT:
		case CMZN_FIELD_TYPE_CONCATENATE:
			field = importCompositeField(type, fieldmodule, typeSettings, jsonImport);
			break;
		case CMZN_FIELD_TYPE_IF:
			field = importGenericThreeSourcesField(type, fieldmodule, typeSettings, jsonImport);
			break;
		case CMZN_FIELD_TYPE_CONSTANT:
		case CMZN_FIELD_TYPE_STRING_CONSTANT:
			field = importConstantField(type, fieldmodule, typeSettings);
			break;
		case CMZN_FIELD_TYPE_DERIVATIVE:
			field = importDerivativeField(type, fieldmodule, typeSettings, jsonImport);
			break;
		case CMZN_FIELD_TYPE_FINITE_ELEMENT:
		case CMZN_FIELD_TYPE_EMBEDDED:
		case CMZN_FIELD_TYPE_STORED_STRING:
		case CMZN_FIELD_TYPE_IS_EXTERIOR:
		case CMZN_FIELD_TYPE_IS_ON_FACE:
		case CMZN_FIELD_TYPE_EDGE_DISCONTINUITY:
		case CMZN_FIELD_TYPE_NODE_VALUE:
		case CMZN_FIELD_TYPE_STORED_MESH_LOCATION:
		case CMZN_FIELD_TYPE_FIND_MESH_LOCATION:
			field = importFiniteElementField(type, fieldmodule, fieldSettings, typeSettings, jsonImport);
			break;
		case CMZN_FIELD_TYPE_MESH_INTEGRAL:
		case CMZN_FIELD_TYPE_MESH_INTEGRAL_SQUARES:
			field = importMeshOperatorsField(type, fieldmodule, fieldSettings, typeSettings, jsonImport);
			break;
		case CMZN_FIELD_TYPE_NODESET_MAXIMUM:
		case CMZN_FIELD_TYPE_NODESET_MEAN:
		case CMZN_FIELD_TYPE_NODESET_MEAN_SQUARES:
		case CMZN_FIELD_TYPE_NODESET_MINIMUM:
		case CMZN_FIELD_TYPE_NODESET_SUM:
		case CMZN_FIELD_TYPE_NODESET_SUM_SQUARES:
			field = importNodesetOperatorsField(type, fieldmodule, fieldSettings, typeSettings, jsonImport);
			break;
		case CMZN_FIELD_TYPE_CROSS_PRODUCT:
			field = importGenericMultiComponentField(type, fieldmodule, typeSettings, jsonImport);
			break;
		case CMZN_FIELD_TYPE_TIME_VALUE:
			field = importTimeValueField(type, fieldmodule, typeSettings, jsonImport);
			break;
		default:
			break;
	}

	return field;
}


void FieldJsonIO::exportTypeSpecificParameters(Json::Value &fieldSettings)
{
	enum cmzn_field_type type = cmzn_field_get_type(field.getId());
	int numberOfComponent = field.getNumberOfComponents();

	Json::Value typeSettings;
	int numberOfSourceFields = field.getNumberOfSourceFields();
	for (int i = 0; i < numberOfSourceFields; i++)
	{
		CMLibs::Zinc::Field sourceField = field.getSourceField(1 + i);
		char *sourceName = sourceField.getName();
		typeSettings["SourceFields"].append(sourceName);
		DEALLOCATE(sourceName);
	}
	if (field.getValueType() == CMLibs::Zinc::Field::VALUE_TYPE_REAL)
	{
		// check if coordinate system should be exported
		bool exportCoordinateSystemType = false;
		CMLibs::Zinc::Field::CoordinateSystemType coordinateSystemType = field.getCoordinateSystemType();
		const bool coordinateSystemUsesFocus =
			(coordinateSystemType == CMLibs::Zinc::Field::COORDINATE_SYSTEM_TYPE_PROLATE_SPHEROIDAL) ||
			(coordinateSystemType == CMLibs::Zinc::Field::COORDINATE_SYSTEM_TYPE_OBLATE_SPHEROIDAL);

		if ((type == CMZN_FIELD_TYPE_FINITE_ELEMENT ||
			type == CMZN_FIELD_TYPE_VECTOR_COORDINATE_TRANSFORMATION ||
			type == CMZN_FIELD_TYPE_COORDINATE_TRANSFORMATION) ||
			(numberOfSourceFields == 0))
		{
			exportCoordinateSystemType = true;
		}
		else if (numberOfSourceFields > 0)
		{
			CMLibs::Zinc::Field sourceField = field.getSourceField(1);
			CMLibs::Zinc::Field::CoordinateSystemType sourceCoordinateSystemType = sourceField.getCoordinateSystemType();
			if ((sourceCoordinateSystemType != coordinateSystemType) ||
				(coordinateSystemUsesFocus &&
					(field.getCoordinateSystemFocus() != sourceField.getCoordinateSystemFocus())))
			{
				exportCoordinateSystemType = true;
			}
		}
		if (exportCoordinateSystemType)
		{
			char *system_string = field.CoordinateSystemTypeEnumToString(coordinateSystemType);
			fieldSettings["CoordinateSystemType"] = system_string;
			DEALLOCATE(system_string);
			if (coordinateSystemUsesFocus)
			{
				fieldSettings["CoordinateSystemFocus"] = field.getCoordinateSystemFocus();
			}
		}
	}

	char *className = field.getClassName();
	switch (type)
	{
        case CMZN_FIELD_TYPE_ABS:
        case CMZN_FIELD_TYPE_ADD:
        case CMZN_FIELD_TYPE_DIVIDE:
        case CMZN_FIELD_TYPE_EXP:
        case CMZN_FIELD_TYPE_LOG:
        case CMZN_FIELD_TYPE_MULTIPLY:
        case CMZN_FIELD_TYPE_POWER:
        case CMZN_FIELD_TYPE_SQRT:
        case CMZN_FIELD_TYPE_SUBTRACT:
        case CMZN_FIELD_TYPE_CONCATENATE:
        case CMZN_FIELD_TYPE_IDENTITY:
        case CMZN_FIELD_TYPE_IF:
        case CMZN_FIELD_TYPE_COORDINATE_TRANSFORMATION:
        case CMZN_FIELD_TYPE_VECTOR_COORDINATE_TRANSFORMATION:
        case CMZN_FIELD_TYPE_CURL:
        case CMZN_FIELD_TYPE_DIVERGENCE:
        case CMZN_FIELD_TYPE_GRADIENT:
        case CMZN_FIELD_TYPE_FIBRE_AXES:
        case CMZN_FIELD_TYPE_EMBEDDED:
        case CMZN_FIELD_TYPE_IS_EXTERIOR:
        case CMZN_FIELD_TYPE_STORED_STRING:
        case CMZN_FIELD_TYPE_EQUAL_TO:
        case CMZN_FIELD_TYPE_AND:
        case CMZN_FIELD_TYPE_GREATER_THAN:
        case CMZN_FIELD_TYPE_IS_DEFINED:
        case CMZN_FIELD_TYPE_LESS_THAN:
        case CMZN_FIELD_TYPE_NOT:
        case CMZN_FIELD_TYPE_OR:
        case CMZN_FIELD_TYPE_XOR:
        case CMZN_FIELD_TYPE_DETERMINANT:
        case CMZN_FIELD_TYPE_EIGENVALUES:
        case CMZN_FIELD_TYPE_EIGENVECTORS:
        case CMZN_FIELD_TYPE_MATRIX_INVERT:
        case CMZN_FIELD_TYPE_PROJECTION:
        case CMZN_FIELD_TYPE_TIME_LOOKUP:
        case CMZN_FIELD_TYPE_ACOS:
        case CMZN_FIELD_TYPE_ASIN:
        case CMZN_FIELD_TYPE_ATAN:
        case CMZN_FIELD_TYPE_ATAN2:
        case CMZN_FIELD_TYPE_COS:
        case CMZN_FIELD_TYPE_SIN:
        case CMZN_FIELD_TYPE_TAN:
        case CMZN_FIELD_TYPE_CROSS_PRODUCT:
        case CMZN_FIELD_TYPE_DOT_PRODUCT:
        case CMZN_FIELD_TYPE_MAGNITUDE:
        case CMZN_FIELD_TYPE_NORMALISE:
        case CMZN_FIELD_TYPE_SUM_COMPONENTS:
        case CMZN_FIELD_TYPE_INVALID:
			break;
		case CMZN_FIELD_TYPE_APPLY:
		{
			CMLibs::Zinc::FieldApply fieldApply = this->field.castApply();
			// store number of components for cases where source field does not exist on import
			typeSettings["NumberOfComponents"] = field.getNumberOfComponents();
			// write region path if in another region
			CMLibs::Zinc::Region region = this->fieldmodule.getRegion();
			CMLibs::Zinc::Field evaluateField = fieldApply.getSourceField(1);
			CMLibs::Zinc::Region evaluateRegion = evaluateField.getFieldmodule().getRegion();
			if (!(evaluateRegion == region))
			{
				char *evaluateRegionPath = evaluateRegion.getRelativePath(region);
				typeSettings["EvaluateRegionPath"] = evaluateRegionPath;
				DEALLOCATE(evaluateRegionPath);
			}
		}	break;
		case CMZN_FIELD_TYPE_ARGUMENT_REAL:
			typeSettings["NumberOfComponents"] = field.getNumberOfComponents();
			break;
		case CMZN_FIELD_TYPE_COMPONENT:
		{
			CMLibs::Zinc::FieldComponent fieldComponent = field.castComponent();
			for (int i = 0; i < numberOfComponent; i++)
			{
				typeSettings["SourceComponentIndexes"].append(fieldComponent.getSourceComponentIndex(i+1));
			}
		} break;
		case CMZN_FIELD_TYPE_CONSTANT:
		{
			CMLibs::Zinc::Fieldcache fieldcache = fieldmodule.createFieldcache();
			double *values = 0;
			values = new double[numberOfComponent];
			field.evaluateReal(fieldcache, numberOfComponent, values);
			for (int i = 0; i < numberOfComponent; i++)
			{
				typeSettings["Values"].append(values[i]);
			}
			delete[] values;
		} break;
		case CMZN_FIELD_TYPE_STRING_CONSTANT:
		{
			CMLibs::Zinc::Fieldcache fieldcache = fieldmodule.createFieldcache();
			char *string_value = field.evaluateString(fieldcache);
			typeSettings["StringValue"] = string_value;
			DEALLOCATE(string_value);
		} break;
		case CMZN_FIELD_TYPE_DERIVATIVE:
		{
			CMLibs::Zinc::FieldDerivative fieldDerivative = this->field.castDerivative();
			const int xiIndex = fieldDerivative.getXiIndex();
			typeSettings["XiIndexes"].append(xiIndex);
		} break;
		case CMZN_FIELD_TYPE_IS_ON_FACE:
		{
			CMLibs::Zinc::FieldIsOnFace fieldIsOnFace = this->field.castIsOnFace();
			char *elementFaceTypeString = CMLibs::Zinc::Element::FaceTypeEnumToString(fieldIsOnFace.getElementFaceType());
			if (elementFaceTypeString)
			{
				typeSettings["ElementFaceType"] = elementFaceTypeString;
				DEALLOCATE(elementFaceTypeString);
			}
		} break;
		case CMZN_FIELD_TYPE_EDGE_DISCONTINUITY:
		{
			char *enumString = cmzn_field_edge_discontinuity_measure_enum_to_string(
				static_cast<cmzn_field_edge_discontinuity_measure>(field.castEdgeDiscontinuity().getMeasure()));
			if (enumString)
			{
				typeSettings["Measure"] = enumString;
				DEALLOCATE(enumString);
			}
		} break;
		case CMZN_FIELD_TYPE_NODE_VALUE:
		{
			CMLibs::Zinc::FieldNodeValue fieldNodeValue = this->field.castNodeValue();
			char *nodeValueLabelString = CMLibs::Zinc::Node::ValueLabelEnumToString(fieldNodeValue.getNodeValueLabel());
			if (nodeValueLabelString)
			{
				typeSettings["NodeValueLabel"] = nodeValueLabelString;
				DEALLOCATE(nodeValueLabelString);
			}
			const int versionNumber = fieldNodeValue.getVersionNumber();
			typeSettings["VersionNumber"] = versionNumber;
		} break;
		case CMZN_FIELD_TYPE_MATRIX_MULTIPLY:
		{
			CMLibs::Zinc::FieldMatrixMultiply fieldMatrixMultiply = this->field.castMatrixMultiply();
			const int numberOfRows = fieldMatrixMultiply.getNumberOfRows();
			typeSettings["NumberOfRows"] = numberOfRows;
		} break;
		case CMZN_FIELD_TYPE_TRANSPOSE:
		{
			CMLibs::Zinc::FieldTranspose fieldTranspose = this->field.castTranspose();
			const int sourceNumberOfRows = fieldTranspose.getSourceNumberOfRows();
			typeSettings["SourceNumberOfRows"] = sourceNumberOfRows;
		} break;
		case CMZN_FIELD_TYPE_FINITE_ELEMENT:
		{
			const int numberOfComponents = field.getNumberOfComponents();
			fieldSettings["IsTypeCoordinate"] = field.isTypeCoordinate();
			typeSettings["NumberOfComponents"] = numberOfComponents;
			for (int i = 0; i < numberOfComponents; i++)
			{
				char *name = field.getComponentName(1 + i);
				typeSettings["ComponentNames"].append(name);
				DEALLOCATE(name);
			}
		} break;
		case CMZN_FIELD_TYPE_TIME_VALUE:
		{
			typeSettings["Timekeeper"] = "default";
		} break;
		case CMZN_FIELD_TYPE_STORED_MESH_LOCATION:
		{
			const FE_mesh *mesh = cmzn_field_get_host_FE_mesh(field.getId());
			typeSettings["Mesh"] = (mesh) ? mesh->getName() : "unknown";
		} break;
		case CMZN_FIELD_TYPE_FIND_MESH_LOCATION:
		{
			CMLibs::Zinc::FieldFindMeshLocation fieldFindMeshLocation = this->field.castFindMeshLocation();
			CMLibs::Zinc::Mesh mesh = fieldFindMeshLocation.getMesh();
			char *meshName = mesh.getName();
			CMLibs::Zinc::Mesh searchMesh = fieldFindMeshLocation.getSearchMesh();
			char *searchMeshName = searchMesh.getName();
			char *searchModeName = fieldFindMeshLocation.SearchModeEnumToString(fieldFindMeshLocation.getSearchMode());
			typeSettings["Mesh"] = meshName;
			typeSettings["SearchMesh"] = searchMeshName;
			typeSettings["SearchMode"] = searchModeName;
			DEALLOCATE(meshName);
			DEALLOCATE(searchMeshName);
			DEALLOCATE(searchModeName);
		} break;
		case CMZN_FIELD_TYPE_MESH_INTEGRAL:
		case CMZN_FIELD_TYPE_MESH_INTEGRAL_SQUARES:
		{
			CMLibs::Zinc::FieldMeshIntegral fieldMeshIntegral = this->field.castMeshIntegral();
			CMLibs::Zinc::Mesh mesh = fieldMeshIntegral.getMesh();
			char *meshName = mesh.getName();
			char *elementQuadratureRuleName = CMLibs::Zinc::Element::QuadratureRuleEnumToString(
				fieldMeshIntegral.getElementQuadratureRule());
			int numbersOfPoints[MAXIMUM_ELEMENT_XI_DIMENSIONS];
			const int numbersCount = fieldMeshIntegral.getNumbersOfPoints(MAXIMUM_ELEMENT_XI_DIMENSIONS, numbersOfPoints);
			typeSettings["Mesh"] = meshName;
			typeSettings["ElementQuadratureRule"] = elementQuadratureRuleName;
			for (int i = 0; i < numbersCount; i++)
			{
				typeSettings["NumbersOfPoints"].append(numbersOfPoints[i]);
			}
			DEALLOCATE(meshName);
			DEALLOCATE(elementQuadratureRuleName);
		} break;
		case CMZN_FIELD_TYPE_NODESET_MAXIMUM:
		case CMZN_FIELD_TYPE_NODESET_MEAN:
		case CMZN_FIELD_TYPE_NODESET_MEAN_SQUARES:
		case CMZN_FIELD_TYPE_NODESET_MINIMUM:
		case CMZN_FIELD_TYPE_NODESET_SUM:
		case CMZN_FIELD_TYPE_NODESET_SUM_SQUARES:
		{
			CMLibs::Zinc::FieldNodesetOperator fieldNodesetOperator = this->field.castNodesetOperator();
			CMLibs::Zinc::Nodeset nodeset = fieldNodesetOperator.getNodeset();
			CMLibs::Zinc::Field elementMapField = fieldNodesetOperator.getElementMapField();
			if (elementMapField.isValid())
			{
				// optional element map field is also in source fields; store separately to support future optional fields
				char *elementMapFieldName = elementMapField.getName();
				typeSettings["ElementMapField"] = elementMapFieldName;
				DEALLOCATE(elementMapFieldName);
			}
			char *nodesetName = nodeset.getName();
			typeSettings["Nodeset"] = nodesetName;
			DEALLOCATE(nodesetName);
		} break;
		default:
			break;
	}

	if (className)
	{
		fieldSettings[className] = typeSettings;
		DEALLOCATE(className);
	}
}

void FieldJsonIO::exportEntries(Json::Value &fieldSettings)
{
	char *name = this->field.getName();
	fieldSettings["Name"] = name;
	DEALLOCATE(name);
	fieldSettings["IsManaged"] = this->field.isManaged();
	exportTypeSpecificParameters(fieldSettings);
}

void FieldJsonIO::importEntries(const Json::Value &fieldSettings)
{
	if (fieldSettings["CoordinateSystemType"].isString())
		this->field.setCoordinateSystemType(this->field.CoordinateSystemTypeEnumFromString(
			fieldSettings["CoordinateSystemType"].asCString()));
	if (fieldSettings["CoordinateSystemFocus"].isDouble())
		this->field.setCoordinateSystemFocus(fieldSettings["CoordinateSystemFocus"].asDouble());
	this->field.setManaged(true);
	if (fieldSettings["Name"].isString())
	{
		this->field.setName(fieldSettings["Name"].asCString());
	}
}
