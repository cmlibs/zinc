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
#include "computed_field/computed_field_private.hpp"
#include "description_io/field_json_io.hpp"
#include "general/debug.h"
#include "opencmiss/zinc/fieldalias.hpp"
#include "opencmiss/zinc/fieldarithmeticoperators.hpp"
#include "opencmiss/zinc/fieldcomposite.hpp"
#include "opencmiss/zinc/field.hpp"
#include "opencmiss/zinc/field.h"

#include <string.h>

OpenCMISS::Zinc::Field *getSourceFields(Json::Value &fieldSettings, int *count,
	FieldmoduleJsonImport *jsonImport)
{
	unsigned int numberOfSourceFields = 0;
	if (fieldSettings["NumberOfSourceFields"].isInt())
	{
		numberOfSourceFields = fieldSettings["NumberOfSourceFields"].asInt();
	}

	OpenCMISS::Zinc::Field *sourceFields = 0;

	if (numberOfSourceFields > 0)
	{
		sourceFields = new OpenCMISS::Zinc::Field[numberOfSourceFields];
		if (fieldSettings["SourceField"].isArray() &&
			fieldSettings["SourceField"].size() == numberOfSourceFields)
		{
			for (unsigned int i = 0; i < fieldSettings["SourceField"].size(); i++)
			{
				const char *sourceFieldName = fieldSettings["SourceField"][i].asCString();
				sourceFields[i] = jsonImport->getFieldByName(sourceFieldName);
			}
		}
	}
	*count = numberOfSourceFields;

	return sourceFields;
}

OpenCMISS::Zinc::Field importGenericOneComponentsField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, Json::Value &fieldSettings,
	FieldmoduleJsonImport *jsonImport)
{
	int sourcesCount = 0;
	OpenCMISS::Zinc::Field field(0);
	OpenCMISS::Zinc::Field *sourcefields = getSourceFields(fieldSettings, &sourcesCount,
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
			default:
				break;
		}
	}
	delete[] sourcefields;
	return field;
}

OpenCMISS::Zinc::Field importGenericTwoComponentsField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, Json::Value &fieldSettings,
	FieldmoduleJsonImport *jsonImport)
{
	int sourcesCount = 0;
	OpenCMISS::Zinc::Field field(0);
	OpenCMISS::Zinc::Field *sourcefields = getSourceFields(fieldSettings, &sourcesCount,
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
			default:
				break;
		}

	}
	delete[] sourcefields;
	return field;
}

OpenCMISS::Zinc::Field importCompositeField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, Json::Value &fieldSettings,
	FieldmoduleJsonImport *jsonImport)
{
	int sourcesCount = 0;
	OpenCMISS::Zinc::Field field(0);
	OpenCMISS::Zinc::Field *sourcefields = getSourceFields(fieldSettings, &sourcesCount,
		jsonImport);
	if (sourcesCount > 0)
	{
		switch (type)
		{
			case CMZN_FIELD_TYPE_COMPONENT:
				if (fieldSettings["FieldComponent"].isObject())
				{
					Json::Value typeSettings = fieldSettings["FieldComponent"];
					if (typeSettings["SourceComponentIndex"].isArray())
					{
						int componentCount = typeSettings["SourceComponentIndex"].size();
						if (componentCount == 1)
							field = fieldmodule.createFieldComponent(
								sourcefields[0], typeSettings["SourceComponentIndex"][0].asInt());
						if (componentCount > 1)
						{
							int *indexes;
							indexes = new int[componentCount];
							for (int i = 0; i < componentCount; i++)
								indexes[i] = typeSettings["SourceComponentIndex"][i].asInt();
							field = fieldmodule.createFieldComponent(sourcefields[0],
								componentCount, indexes);
							delete[] indexes;
						}
					}
				}
				break;
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

OpenCMISS::Zinc::Field importTypeSpecificField(enum cmzn_field_type type,
	OpenCMISS::Zinc::Fieldmodule &fieldmodule, Json::Value &fieldSettings,
	FieldmoduleJsonImport *jsonImport)
{
	OpenCMISS::Zinc::Field field(0);

	switch (type)
	{
		case CMZN_FIELD_TYPE_ALIAS:
		case CMZN_FIELD_TYPE_LOG:
		case CMZN_FIELD_TYPE_SQRT:
		case CMZN_FIELD_TYPE_EXP:
		case CMZN_FIELD_TYPE_ABS:
		case CMZN_FIELD_TYPE_IDENTITY:
			field = importGenericOneComponentsField(type, fieldmodule, fieldSettings, jsonImport);
			break;
		case CMZN_FIELD_TYPE_ADD:
		case CMZN_FIELD_TYPE_POWER:
		case CMZN_FIELD_TYPE_MULTIPLY:
		case CMZN_FIELD_TYPE_DIVIDE:
		case CMZN_FIELD_TYPE_SUBTRACT:
			field = importGenericTwoComponentsField(type, fieldmodule, fieldSettings, jsonImport);
			break;
		case CMZN_FIELD_TYPE_COMPONENT:
		case CMZN_FIELD_TYPE_CONCATENATE:
			field = importCompositeField(type, fieldmodule, fieldSettings, jsonImport);
			break;
		default:
			break;
	}

	return field;
}


void exportTypeSpecificField(OpenCMISS::Zinc::Field &field, Json::Value &fieldSettings)
{
	enum cmzn_field_type type = cmzn_field_get_type(field.getId());
	char *type_string = cmzn_field_type_enum_to_string(cmzn_field_get_type(field.getId()));
	fieldSettings["Type"] = type_string;
	DEALLOCATE(type_string);
	int numberOfComponent = field.getNumberOfComponents();
	switch (type)
	{
		case CMZN_FIELD_TYPE_ALIAS:
		case CMZN_FIELD_TYPE_LOG:
		case CMZN_FIELD_TYPE_SQRT:
		case CMZN_FIELD_TYPE_EXP:
		case CMZN_FIELD_TYPE_ABS:
		case CMZN_FIELD_TYPE_IDENTITY:
		case CMZN_FIELD_TYPE_ADD:
		case CMZN_FIELD_TYPE_POWER:
		case CMZN_FIELD_TYPE_MULTIPLY:
		case CMZN_FIELD_TYPE_DIVIDE:
		case CMZN_FIELD_TYPE_SUBTRACT:
		case CMZN_FIELD_TYPE_CONCATENATE:
			break;
		case CMZN_FIELD_TYPE_COMPONENT:
		{
			OpenCMISS::Zinc::FieldComponent fieldComponent = field.castComponent();
			Json::Value typeSettings;
			for (int i = 0; i < numberOfComponent; i++)
			{
				typeSettings["SourceComponentIndex"].append(fieldComponent.getSourceComponentIndex(i+1));
			}
			fieldSettings["FieldComponent"] = typeSettings;
		}
			break;
		default:
			break;
	}
}

void FieldJsonIO::ioEntries(Json::Value &fieldSettings)
{
	if (mode == IO_MODE_EXPORT)
	{
		char *name = field.getName();
		fieldSettings["Name"] = name;
		DEALLOCATE(name);
		int numberOfComponents = field.getNumberOfComponents();
		fieldSettings["NumberOfComponents"] = numberOfComponents;
		fieldSettings["TypeCoordinate"] = field.isTypeCoordinate();
		fieldSettings["CoordinateSystemFocus"] = field.getCoordinateSystemFocus();
		fieldSettings["IsManaged"] = field.isManaged();
		for (int i = 0; i < numberOfComponents; i++)
		{
			name = field.getComponentName(1 + i);
			fieldSettings["ComponentName"].append(name);
			DEALLOCATE(name);
		}
		char *system_string = cmzn_field_coordinate_system_type_enum_to_string(
			cmzn_field_get_coordinate_system_type(field.getId()));
		fieldSettings["CoordinateSystemType"] = system_string;
		DEALLOCATE(system_string);
		int numberOfSourceFields = field.getNumberOfSourceFields();
		fieldSettings["NumberOfSourceFields"] = numberOfSourceFields;
		for (int i = 0; i < numberOfSourceFields; i++)
		{
			OpenCMISS::Zinc::Field sourceField = field.getSourceField(1 + i);
			char *sourceName = sourceField.getName();
			fieldSettings["SourceField"].append(sourceName);
			DEALLOCATE(sourceName);
		}
		exportTypeSpecificField(field, fieldSettings);
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

		field.setManaged(true);
	}
}
