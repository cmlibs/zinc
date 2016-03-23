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
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_derivatives.h"
#include "computed_field/computed_field_matrix_operators.hpp"
#include "description_io/field_json_io.hpp"
#include "general/debug.h"
#include "opencmiss/zinc/fieldalias.hpp"
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
#include "opencmiss/zinc/field.hpp"
#include "opencmiss/zinc/field.h"
#include "opencmiss/zinc/fieldcache.hpp"
#include <stdio.h>
#include <string.h>

/*
 * header not yet supported:
 * fieldgroup
 * fieldimage
 * fieldimageprocessing
 *
 *
 */

OpenCMISS::Zinc::Field *getSourceFields(Json::Value &typeSettings, unsigned int *count,
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

OpenCMISS::Zinc::Field importGenericOneComponentsField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, Json::Value &typeSettings,
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
			case CMZN_FIELD_TYPE_ALIAS:
				field = fieldmodule.createFieldAlias(sourcefields[0]);
				break;
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
			case CMZN_FIELD_TYPE_COORDINATE_TRANFORMATION:
				field = fieldmodule.createFieldCoordinateTransformation(sourcefields[0]);
				break;
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
			default:
				break;
		}
	}
	delete[] sourcefields;
	return field;
}

OpenCMISS::Zinc::Field importGenericTwoComponentsField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, Json::Value &typeSettings,
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
			case CMZN_FIELD_TYPE_VECTOR_COORDINATE_TRANFORMATION:
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
			default:
				break;
		}

	}
	delete[] sourcefields;
	return field;
}

OpenCMISS::Zinc::Field importGenericThreeComponentsField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, Json::Value &typeSettings,
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

OpenCMISS::Zinc::Field importCompositeField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, Json::Value &typeSettings,
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

OpenCMISS::Zinc::Field importConstantField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, Json::Value &typeSettings)
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

OpenCMISS::Zinc::Field importDerivativeField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, Json::Value &typeSettings,
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
				if (typeSettings["XiIndex"].isInt())
				{
					field = fieldmodule.createFieldDerivative(sourcefields[0],
						typeSettings["XiIndex"].asInt());
				}
			} break;
			default:
				break;
		}
	}
	delete[] sourcefields;
	return field;
}

OpenCMISS::Zinc::Field importFiniteElementField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, Json::Value &typeSettings,
	FieldmoduleJsonImport *jsonImport)
{
	OpenCMISS::Zinc::Field field(0);
	switch (type)
	{
		case CMZN_FIELD_TYPE_EMBEDDED:
			field = importGenericTwoComponentsField(type, fieldmodule, typeSettings, jsonImport);
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
				enum cmzn_element_face_type type = cmzn_element_face_type_enum_from_string(
					typeSettings["ElementFaceType"].asCString());
				field = OpenCMISS::Zinc::Field(cmzn_fieldmodule_create_field_is_on_face(
					fieldmodule.getId(), type));
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
					cmzn_field_edge_discontinuity_set_measure(derivedField.getDerivedId(),
						cmzn_field_edge_discontinuity_measure_enum_from_string(
							typeSettings["Measure"].asCString()));
				field = derivedField;
			}
			delete[] sourcefields;
		}	break;
		case CMZN_FIELD_TYPE_NODE_VALUE:
		{
			unsigned int sourcesCount = 0;
			OpenCMISS::Zinc::Field *sourcefields = getSourceFields(typeSettings, &sourcesCount,
				jsonImport);
			if (sourcesCount == 1 && typeSettings["NodeValueLabel"].isString())
			{
				enum cmzn_node_value_label valueLabel = cmzn_node_value_label_enum_from_string(
					typeSettings["NodeValueLabel"].asCString());
				int version = typeSettings["VersionNumber"].isInt();
				field = OpenCMISS::Zinc::Field(
					cmzn_fieldmodule_create_field_node_value(fieldmodule.getId(),
						sourcefields[0].getId(), valueLabel, version));
			}
			delete[] sourcefields;
		}	break;
		default:
			break;

	}
	return field;
}

Json::Value getDerviedFieldValue(Json::Value &fieldSettings, enum cmzn_field_type *type)
{
	Json::Value typeSettings;
	Json::Value::Members members = fieldSettings.getMemberNames();
	Json::Value::Members::iterator it = members.begin();
	*type = CMZN_FIELD_TYPE_INVALID;
	while (it != members.end() && (*type == CMZN_FIELD_TYPE_INVALID))
	{

		*type = cmzn_field_type_enum_from_class_name((*it).c_str());
		if (*type != CMZN_FIELD_TYPE_INVALID)
		{
			typeSettings = fieldSettings[(*it)];
		}
		++it;
	}
	return typeSettings;
}


OpenCMISS::Zinc::Field importTypeSpecificField(
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, Json::Value &fieldSettings,
	FieldmoduleJsonImport *jsonImport)
{
	OpenCMISS::Zinc::Field field(0);
	enum cmzn_field_type type = CMZN_FIELD_TYPE_INVALID;
	Json::Value typeSettings = getDerviedFieldValue(fieldSettings, &type);
	switch (type)
	{
		case CMZN_FIELD_TYPE_ALIAS:
		case CMZN_FIELD_TYPE_LOG:
		case CMZN_FIELD_TYPE_SQRT:
		case CMZN_FIELD_TYPE_EXP:
		case CMZN_FIELD_TYPE_ABS:
		case CMZN_FIELD_TYPE_IDENTITY:
		case CMZN_FIELD_TYPE_COORDINATE_TRANFORMATION:
		case CMZN_FIELD_TYPE_IS_DEFINED:
		case CMZN_FIELD_TYPE_NOT:
		case CMZN_FIELD_TYPE_DETERMINANT:
		case CMZN_FIELD_TYPE_EIGENVALUES:
		case CMZN_FIELD_TYPE_EIGENVECTORS:
		case CMZN_FIELD_TYPE_MATRIX_INVERT:
			field = importGenericOneComponentsField(type, fieldmodule, typeSettings, jsonImport);
			break;
		case CMZN_FIELD_TYPE_ADD:
		case CMZN_FIELD_TYPE_POWER:
		case CMZN_FIELD_TYPE_MULTIPLY:
		case CMZN_FIELD_TYPE_DIVIDE:
		case CMZN_FIELD_TYPE_SUBTRACT:
		case CMZN_FIELD_TYPE_VECTOR_COORDINATE_TRANFORMATION:
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
			field = importGenericTwoComponentsField(type, fieldmodule, typeSettings, jsonImport);
			break;
		case CMZN_FIELD_TYPE_COMPONENT:
		case CMZN_FIELD_TYPE_CONCATENATE:
			field = importCompositeField(type, fieldmodule, typeSettings, jsonImport);
			break;
		case CMZN_FIELD_TYPE_IF:
			field = importGenericThreeComponentsField(type, fieldmodule, typeSettings, jsonImport);
			break;
		case CMZN_FIELD_TYPE_CONSTANT:
		case CMZN_FIELD_TYPE_STRING_CONSTANT:
			field = importConstantField(type, fieldmodule, typeSettings);
			break;
		case CMZN_FIELD_TYPE_DERIVATIVE:
			field = importDerivativeField(type, fieldmodule, typeSettings, jsonImport);
			break;
		case CMZN_FIELD_TYPE_EMBEDDED:
		case CMZN_FIELD_TYPE_STORED_STRING:
		case CMZN_FIELD_TYPE_IS_EXTERIOR:
		case CMZN_FIELD_TYPE_IS_ON_FACE:
		case CMZN_FIELD_TYPE_EDGE_DISCONTINUITY:
		case CMZN_FIELD_TYPE_NODE_VALUE:
			field = importFiniteElementField(type, fieldmodule, typeSettings, jsonImport);
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
	char *className = cmzn_field_type_enum_to_class_name(type);
	switch (type)
	{
		case CMZN_FIELD_TYPE_INVALID:
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
			int xi_index = cmzn_field_derivative_get_xi_index(field.getId());
			typeSettings["XiIndex"] = xi_index;
		} break;
		case CMZN_FIELD_TYPE_IS_ON_FACE:
		{
			Json::Value typeSettings;
			char *enumString = cmzn_element_face_type_enum_to_string(
				cmzn_field_is_on_face_get_face_type(field.getId()));
			if (enumString)
			{
				typeSettings["ElementFaceType"] = enumString;
				DEALLOCATE(enumString);
			}
		} break;
		case CMZN_FIELD_TYPE_EDGE_DISCONTINUITY:
		{
			char *enumString = cmzn_field_edge_discontinuity_measure_enum_to_string(
				cmzn_field_edge_discontinuity_get_measure(field.castEdgeDiscontinuity().getDerivedId()));
			if (enumString)
			{
				typeSettings["Measure"] = enumString;
				DEALLOCATE(enumString);
			}
		} break;
		case CMZN_FIELD_TYPE_NODE_VALUE:
		{
			char *enumString = cmzn_node_value_label_enum_to_string(
				cmzn_field_node_value_get_value_label(field.getId()));
			if (enumString)
			{
				typeSettings["NodeValueLabel"] = enumString;
				DEALLOCATE(enumString);
			}
			int versioNumber = cmzn_field_node_value_get_version(field.getId());
			typeSettings["VersionNumber"] = versioNumber;
		} break;
		case CMZN_FIELD_TYPE_MATRIX_MULTIPLY:
		{
			int numberOfRows = cmzn_field_matrix_multiply_get_number_of_rows(field.getId());
			typeSettings["NumberOfRows"] = numberOfRows;
		} break;
		case CMZN_FIELD_TYPE_TRANSPOSE:
		{
			int sourceNumberOfRows = cmzn_field_transpose_get_source_number_of_rowss(field.getId());
			typeSettings["SourceNumberOfRows"] = sourceNumberOfRows;
		} break;
		default:
		{
		} break;
	}
	if (CMZN_FIELD_TYPE_INVALID != type)
		fieldSettings[className] = typeSettings;

	DEALLOCATE(className);
}

void FieldJsonIO::ioFiniteElementOnlyEntries(Json::Value &fieldSettings)
{
	if (mode == IO_MODE_EXPORT)
	{
		int numberOfComponents = field.getNumberOfComponents();
		fieldSettings["TypeCoordinate"] = field.isTypeCoordinate();
		fieldSettings["NumberOfComponents"] = numberOfComponents;
		for (int i = 0; i < numberOfComponents; i++)
		{
			char *name = field.getComponentName(1 + i);
			fieldSettings["ComponentName"].append(name);
			DEALLOCATE(name);
		}
	}
	else
	{
		if (fieldSettings["TypeCoordinate"].isBool())
			field.setTypeCoordinate(fieldSettings["TypeCoordinate"].asBool());
		if (fieldSettings["ComponentName"].isArray())
		{
			unsigned int numberOfComponents = fieldSettings["ComponentName"].size();
			for (unsigned int i = 0; i < numberOfComponents; i++)
			{
				field.setComponentName(i+1, fieldSettings["ComponentName"][i].asCString());
			}
		}
	}
}

void FieldJsonIO::ioEntries(Json::Value &fieldSettings)
{
	if (mode == IO_MODE_EXPORT)
	{
		char *name = field.getName();
		fieldSettings["Name"] = name;
		DEALLOCATE(name);

		fieldSettings["IsManaged"] = field.isManaged();
		char *system_string = cmzn_field_coordinate_system_type_enum_to_string(
			cmzn_field_get_coordinate_system_type(field.getId()));
		fieldSettings["CoordinateSystemType"] = system_string;
		fieldSettings["CoordinateSystemFocus"] = field.getCoordinateSystemFocus();
		DEALLOCATE(system_string);
		exportTypeSpecificParameters(fieldSettings);
	}
	else
	{
		if (fieldSettings["Name"].isString())
		{
			field.setName(fieldSettings["Name"].asCString());
		}
		if (fieldSettings["CoordinateSystemType"].isString())
			cmzn_field_set_coordinate_system_type(field.getId(),
				cmzn_field_coordinate_system_type_enum_from_string(
					fieldSettings["CoordinateSystemType"].asCString()));
		if (fieldSettings["CoordinateSystemFocus"].isDouble())
			field.setCoordinateSystemFocus(fieldSettings["CoordinateSystemFocus"].asDouble());

		field.setManaged(true);
	}
}
