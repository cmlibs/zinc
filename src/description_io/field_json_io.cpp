/***************************************************************************//**
 * FILE : graphics_json_io.cpp
 *
 * The definition to graphics_json_io.
 *
 */
/* OpenCMISS-Zinc Library
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
#include "opencmiss/zinc/changemanager.hpp"
#include "opencmiss/zinc/element.hpp"
#include "opencmiss/zinc/fieldalias.hpp"
#include "opencmiss/zinc/fieldapply.hpp"
#include "opencmiss/zinc/fieldarithmeticoperators.hpp"
#include "opencmiss/zinc/fieldcomposite.hpp"
#include "opencmiss/zinc/fieldconditional.hpp"
#include "opencmiss/zinc/fieldconstant.hpp"
#include "opencmiss/zinc/fieldcoordinatetransformation.hpp"
#include "opencmiss/zinc/fieldderivatives.hpp"
#include "opencmiss/zinc/fieldfibres.hpp"
#include "opencmiss/zinc/fieldfiniteelement.hpp"
#include "opencmiss/zinc/fieldlogicaloperators.hpp"
#include "opencmiss/zinc/fieldmatrixoperators.hpp"
#include "opencmiss/zinc/fieldmeshoperators.hpp"
#include "opencmiss/zinc/fieldmodule.hpp"
#include "opencmiss/zinc/fieldtime.hpp"
#include "opencmiss/zinc/fieldtrigonometry.hpp"
#include "opencmiss/zinc/fieldvectoroperators.hpp"
#include "opencmiss/zinc/field.hpp"
#include "opencmiss/zinc/field.h"
#include "opencmiss/zinc/fieldcache.hpp"
#include "opencmiss/zinc/region.hpp"
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
OpenCMISS::Zinc::Field *getSourceFields(const Json::Value &typeSettings, unsigned int *count,
	FieldmoduleJsonImport *jsonImport)
{
	unsigned int numberOfSourceFields = 0;

	OpenCMISS::Zinc::Field *sourceFields = 0;

	if (typeSettings["SourceFields"].isArray() &&
		typeSettings["SourceFields"].size() > 0)
	{
		numberOfSourceFields = typeSettings["SourceFields"].size();
		sourceFields = new OpenCMISS::Zinc::Field[numberOfSourceFields];
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
OpenCMISS::Zinc::Field importApplyField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, const Json::Value &typeSettings,
	FieldmoduleJsonImport *jsonImport)
{
	OpenCMISS::Zinc::Field field;
	switch (type)
	{
	case CMZN_FIELD_TYPE_APPLY:
	{
		// FieldApply is uniquely able to use source fields from another region
		OpenCMISS::Zinc::Region region = jsonImport->getRegion();
		// Following are only valid if evaluate and argument fields are from a different region
		OpenCMISS::Zinc::Region evaluateRegion;
		OpenCMISS::Zinc::Fieldmodule evaluateFieldmodule;
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
		OpenCMISS::Zinc::ChangeManager<OpenCMISS::Zinc::Fieldmodule> changeFields(evaluateFieldmodule.isValid() ? evaluateFieldmodule : fieldmodule);
		OpenCMISS::Zinc::Field evaluateField;
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
				evaluateField = OpenCMISS::Zinc::Field(cmzn_fieldmodule_create_field_dummy_real(evaluateFieldmodule.getId(), numberOfComponents));
				evaluateField.setName(evaluateFieldName);
			}
		}
		else
		{
			evaluateField = jsonImport->getFieldByName(evaluateFieldName);
		}
		OpenCMISS::Zinc::FieldApply fieldApply = fieldmodule.createFieldApply(evaluateField);
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
			OpenCMISS::Zinc::Field sourceField = jsonImport->getFieldByName(sourceFieldName);
			OpenCMISS::Zinc::Field argumentField;
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
OpenCMISS::Zinc::Field importGenericOneSourcesField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, const Json::Value &typeSettings,
	FieldmoduleJsonImport *jsonImport)
{
	unsigned int sourcesCount = 0;
	OpenCMISS::Zinc::Field field(0);
	OpenCMISS::Zinc::Field *sourcefields = getSourceFields(typeSettings, &sourcesCount,
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
			{
				field = OpenCMISS::Zinc::Field(cmzn_fieldmodule_create_field_eigenvectors(
					fieldmodule.getId(),	sourcefields[0].getId()));
			}	break;
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
OpenCMISS::Zinc::Field importGenericTwoSourcesField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, const Json::Value &typeSettings,
	FieldmoduleJsonImport *jsonImport)
{
	unsigned int sourcesCount = 0;
	OpenCMISS::Zinc::Field field(0);
	OpenCMISS::Zinc::Field *sourcefields = getSourceFields(typeSettings, &sourcesCount,
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
OpenCMISS::Zinc::Field importGenericThreeSourcesField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, const Json::Value &typeSettings,
	FieldmoduleJsonImport *jsonImport)
{
	unsigned int sourcesCount = 0;
	OpenCMISS::Zinc::Field field(0);
	OpenCMISS::Zinc::Field *sourcefields = getSourceFields(typeSettings, &sourcesCount,
		jsonImport);
	if (sourcesCount == 3)
	{
		switch (type)
		{
			case CMZN_FIELD_TYPE_IF:
				field = fieldmodule.createFieldIf(sourcefields[0], sourcefields[1], sourcefields[3]);
				break;
			default:
				break;
		}

	}
	delete[] sourcefields;
	return field;
}

/* Deserialise component and concatenate fields */
OpenCMISS::Zinc::Field importCompositeField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, const Json::Value &typeSettings,
	FieldmoduleJsonImport *jsonImport)
{
	unsigned int sourcesCount = 0;
	OpenCMISS::Zinc::Field field(0);
	OpenCMISS::Zinc::Field *sourcefields = getSourceFields(typeSettings, &sourcesCount,
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
OpenCMISS::Zinc::Field importConstantField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, const Json::Value &typeSettings)
{
	OpenCMISS::Zinc::Field field(0);

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
OpenCMISS::Zinc::Field importDerivativeField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, const Json::Value &typeSettings,
	FieldmoduleJsonImport *jsonImport)
{
	unsigned int sourcesCount = 0;
	OpenCMISS::Zinc::Field field(0);
	OpenCMISS::Zinc::Field *sourcefields = getSourceFields(typeSettings, &sourcesCount,
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
OpenCMISS::Zinc::Field importFiniteElementField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, const Json::Value &fieldSettings, const Json::Value &typeSettings,
	FieldmoduleJsonImport *jsonImport)
{
	OpenCMISS::Zinc::Field field;
	switch (type)
	{
		case CMZN_FIELD_TYPE_FINITE_ELEMENT:
			if (typeSettings["NumberOfComponents"].isInt())
			{
				const int numberOfComponents = typeSettings["NumberOfComponents"].asInt();
				OpenCMISS::Zinc::FieldFiniteElement fieldFiniteElement = fieldmodule.createFieldFiniteElement(numberOfComponents);
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
				OpenCMISS::Zinc::Element::FaceType elementFaceType =
					OpenCMISS::Zinc::Element::FaceTypeEnumFromString(typeSettings["ElementFaceType"].asCString());
				field = fieldmodule.createFieldIsOnFace(elementFaceType);
			}
		}	break;
		case CMZN_FIELD_TYPE_EDGE_DISCONTINUITY:
		{
			unsigned int sourcesCount = 0;
			OpenCMISS::Zinc::Field *sourcefields = getSourceFields(typeSettings, &sourcesCount,
				jsonImport);
			if (sourcesCount > 0)
			{
				OpenCMISS::Zinc::FieldEdgeDiscontinuity derivedField = fieldmodule.createFieldEdgeDiscontinuity(
					sourcefields[0]);
				if (sourcesCount == 2)
				{
					derivedField.setConditionalField(sourcefields[1]);
				}
				if (typeSettings["Measure"].isString())
					derivedField.setMeasure(static_cast<OpenCMISS::Zinc::FieldEdgeDiscontinuity::Measure>(
						cmzn_field_edge_discontinuity_measure_enum_from_string(
							typeSettings["Measure"].asCString())));
				field = derivedField;
			}
			delete[] sourcefields;
		}	break;
		case CMZN_FIELD_TYPE_NODE_VALUE:
		{
			unsigned int sourcesCount = 0;
			OpenCMISS::Zinc::Field *sourcefields = getSourceFields(typeSettings, &sourcesCount, jsonImport);
			if ((sourcesCount == 1) && typeSettings["NodeValueLabel"].isString() && typeSettings["VersionNumber"].isInt())
			{
				OpenCMISS::Zinc::Node::ValueLabel nodeValueLabel = OpenCMISS::Zinc::Node::ValueLabelEnumFromString(
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
				OpenCMISS::Zinc::Mesh mesh = fieldmodule.findMeshByName(typeSettings["Mesh"].asCString());
				field = fieldmodule.createFieldStoredMeshLocation(mesh);
			}
		}	break;
		case CMZN_FIELD_TYPE_FIND_MESH_LOCATION:
		{
			unsigned int sourcesCount = 0;
			OpenCMISS::Zinc::Field *sourcefields = getSourceFields(typeSettings, &sourcesCount,
				jsonImport);
			if (sourcesCount == 2 && typeSettings["Mesh"].isString() && typeSettings["SearchMode"].isString())
			{
				OpenCMISS::Zinc::Mesh mesh = fieldmodule.findMeshByName(typeSettings["Mesh"].asCString());
				OpenCMISS::Zinc::FieldFindMeshLocation::SearchMode searchMode;
				// migrate legacy non-standard SearchMode names to enums
				const char *searchModeName = typeSettings["SearchMode"].asCString();
				if (0 == strcmp("FIND_EXACT", searchModeName))
					searchMode = OpenCMISS::Zinc::FieldFindMeshLocation::SEARCH_MODE_EXACT;
				else if (0 == strcmp("FIND_NEAREST", searchModeName))
					searchMode = OpenCMISS::Zinc::FieldFindMeshLocation::SEARCH_MODE_NEAREST;
				else
					searchMode = OpenCMISS::Zinc::FieldFindMeshLocation::SearchModeEnumFromString(searchModeName);
				OpenCMISS::Zinc::FieldFindMeshLocation fieldFindMeshLocation =
					fieldmodule.createFieldFindMeshLocation(sourcefields[0], sourcefields[1], mesh);
				if (typeSettings["SearchMesh"].isString())
				{
					OpenCMISS::Zinc::Mesh searchMesh = fieldmodule.findMeshByName(typeSettings["SearchMesh"].asCString());
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

OpenCMISS::Zinc::Field importMeshOperatorsField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, const Json::Value &fieldSettings, const Json::Value &typeSettings,
	FieldmoduleJsonImport *jsonImport)
{
	OpenCMISS::Zinc::Field field;
	switch (type)
	{
	case CMZN_FIELD_TYPE_MESH_INTEGRAL:
	case CMZN_FIELD_TYPE_MESH_INTEGRAL_SQUARES:
	{
		unsigned int sourcesCount = 0;
		OpenCMISS::Zinc::Field *sourcefields = getSourceFields(typeSettings, &sourcesCount, jsonImport);
		if ((sourcesCount == 2)
			&& typeSettings["Mesh"].isString()
			&& typeSettings["ElementQuadratureRule"].isString()
			&& typeSettings["NumbersOfPoints"].isArray())
		{
			OpenCMISS::Zinc::Mesh mesh = fieldmodule.findMeshByName(typeSettings["Mesh"].asCString());
			// FieldMeshIntegral is the base class for FieldMeshIntegralSquares
			OpenCMISS::Zinc::FieldMeshIntegral fieldMeshIntegral =
				(type == CMZN_FIELD_TYPE_MESH_INTEGRAL) ? fieldmodule.createFieldMeshIntegral(sourcefields[0], sourcefields[1], mesh)
				: fieldmodule.createFieldMeshIntegralSquares(sourcefields[0], sourcefields[1], mesh);
			OpenCMISS::Zinc::Element::QuadratureRule quadratureRule =
				OpenCMISS::Zinc::Element::QuadratureRuleEnumFromString(typeSettings["ElementQuadratureRule"].asCString());
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

/* Deserialise field with varying number of fields */
OpenCMISS::Zinc::Field importGenericMultiComponentField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, const Json::Value &typeSettings,
	FieldmoduleJsonImport *jsonImport)
{
	unsigned int sourcesCount = 0;
	OpenCMISS::Zinc::Field field(0);
	OpenCMISS::Zinc::Field *sourcefields = getSourceFields(typeSettings, &sourcesCount,
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

OpenCMISS::Zinc::Field importTimeValueField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, const Json::Value &typeSettings,
	FieldmoduleJsonImport *jsonImport)
{
	OpenCMISS::Zinc::Field field(0);
	switch (type)
	{
		case CMZN_FIELD_TYPE_TIME_VALUE:
		{
			OpenCMISS::Zinc::Region region = fieldmodule.getRegion();
			cmzn_context_id context = region.getId()->getContext();
			OpenCMISS::Zinc::Timekeepermodule tm(cmzn_context_get_timekeepermodule(context));
			OpenCMISS::Zinc::Timekeeper timekeeper = tm.getDefaultTimekeeper();
			field = fieldmodule.createFieldTimeValue(timekeeper);
		} break;
		default:
			break;
	}
	return field;
}

/* Deserialise type specifc field */
OpenCMISS::Zinc::Field importTypeSpecificField(
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, const Json::Value &fieldSettings,
	FieldmoduleJsonImport *jsonImport)
{
	OpenCMISS::Zinc::Field field(0);
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
		OpenCMISS::Zinc::Field sourceField = field.getSourceField(1 + i);
		char *sourceName = sourceField.getName();
		typeSettings["SourceFields"].append(sourceName);
		DEALLOCATE(sourceName);
	}
	if (field.getValueType() == OpenCMISS::Zinc::Field::VALUE_TYPE_REAL)
	{
		// check if coordinate system should be exported
		bool exportCoordinateSystemType = false;
		OpenCMISS::Zinc::Field::CoordinateSystemType coordinateSystemType = field.getCoordinateSystemType();
		const bool coordinateSystemUsesFocus =
			(coordinateSystemType == OpenCMISS::Zinc::Field::COORDINATE_SYSTEM_TYPE_PROLATE_SPHEROIDAL) ||
			(coordinateSystemType == OpenCMISS::Zinc::Field::COORDINATE_SYSTEM_TYPE_OBLATE_SPHEROIDAL);

		if ((type == CMZN_FIELD_TYPE_FINITE_ELEMENT ||
			type == CMZN_FIELD_TYPE_VECTOR_COORDINATE_TRANSFORMATION ||
			type == CMZN_FIELD_TYPE_COORDINATE_TRANSFORMATION) ||
			(numberOfSourceFields == 0))
		{
			exportCoordinateSystemType = true;
		}
		else if (numberOfSourceFields > 0)
		{
			OpenCMISS::Zinc::Field sourceField = field.getSourceField(1);
			OpenCMISS::Zinc::Field::CoordinateSystemType sourceCoordinateSystemType = sourceField.getCoordinateSystemType();
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
				fieldSettings["CoordinateSystemFocus"] = field.getCoordinateSystemFocus();
		}
	}

	char *className = cmzn_field_type_enum_to_class_name(type);
	switch (type)
	{
		case CMZN_FIELD_TYPE_INVALID:
			break;
		case CMZN_FIELD_TYPE_APPLY:
		{
			OpenCMISS::Zinc::FieldApply fieldApply = this->field.castApply();
			// store number of components for cases where source field does not exist on import
			typeSettings["NumberOfComponents"] = field.getNumberOfComponents();
			// write region path if in another region
			OpenCMISS::Zinc::Region region = this->fieldmodule.getRegion();
			OpenCMISS::Zinc::Field evaluateField = fieldApply.getSourceField(1);
			OpenCMISS::Zinc::Region evaluateRegion = evaluateField.getFieldmodule().getRegion();
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
			OpenCMISS::Zinc::FieldComponent fieldComponent = field.castComponent();
			for (int i = 0; i < numberOfComponent; i++)
			{
				typeSettings["SourceComponentIndexes"].append(fieldComponent.getSourceComponentIndex(i+1));
			}
		} break;
		case CMZN_FIELD_TYPE_CONSTANT:
		{
			OpenCMISS::Zinc::Fieldcache fieldcache = fieldmodule.createFieldcache();
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
			OpenCMISS::Zinc::Fieldcache fieldcache = fieldmodule.createFieldcache();
			char *string_value = field.evaluateString(fieldcache);
			typeSettings["StringValue"] = string_value;
			DEALLOCATE(string_value);
		} break;
		case CMZN_FIELD_TYPE_DERIVATIVE:
		{
			OpenCMISS::Zinc::FieldDerivative fieldDerivative = this->field.castDerivative();
			const int xiIndex = fieldDerivative.getXiIndex();
			typeSettings["XiIndexes"].append(xiIndex);
		} break;
		case CMZN_FIELD_TYPE_IS_ON_FACE:
		{
			OpenCMISS::Zinc::FieldIsOnFace fieldIsOnFace = this->field.castIsOnFace();
			char *elementFaceTypeString = OpenCMISS::Zinc::Element::FaceTypeEnumToString(fieldIsOnFace.getElementFaceType());
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
			OpenCMISS::Zinc::FieldNodeValue fieldNodeValue = this->field.castNodeValue();
			char *nodeValueLabelString = OpenCMISS::Zinc::Node::ValueLabelEnumToString(fieldNodeValue.getNodeValueLabel());
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
			int numberOfRows = cmzn_field_matrix_multiply_get_number_of_rows(field.getId());
			typeSettings["NumberOfRows"] = numberOfRows;
		} break;
		case CMZN_FIELD_TYPE_TRANSPOSE:
		{
			int sourceNumberOfRows = cmzn_field_transpose_get_source_number_of_rows(field.getId());
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
			OpenCMISS::Zinc::FieldFindMeshLocation fieldFindMeshLocation = this->field.castFindMeshLocation();
			OpenCMISS::Zinc::Mesh mesh = fieldFindMeshLocation.getMesh();
			char *meshName = mesh.getName();
			OpenCMISS::Zinc::Mesh searchMesh = fieldFindMeshLocation.getSearchMesh();
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
			OpenCMISS::Zinc::FieldMeshIntegral fieldMeshIntegral = this->field.castMeshIntegral();
			OpenCMISS::Zinc::Mesh mesh = fieldMeshIntegral.getMesh();
			char *meshName = mesh.getName();
			char *elementQuadratureRuleName = OpenCMISS::Zinc::Element::QuadratureRuleEnumToString(
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
		default:
		{
		} break;
	}

	if (CMZN_FIELD_TYPE_INVALID != type)
		fieldSettings[className] = typeSettings;

	DEALLOCATE(className);
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
