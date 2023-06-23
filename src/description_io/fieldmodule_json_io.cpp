/***************************************************************************//**
 * FILE : fieldmodule_json_io.cpp
 *
 * The definition to fieldmodule_json_io.
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "cmlibs/zinc/changemanager.hpp"
#include "cmlibs/zinc/core.h"
#include "cmlibs/zinc/field.hpp"
#include "cmlibs/zinc/fieldconstant.hpp"
#include "cmlibs/zinc/status.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_apply.hpp"
#include "computed_field/computed_field_private.hpp"
#include "description_io/field_json_io.hpp"
#include "description_io/fieldmodule_json_io.hpp"
#include "general/debug.h"
#include "general/message.h"
#include <cstring>

CMLibs::Zinc::Field FieldmoduleJsonImport::importField(const Json::Value &fieldSettings)
{
	CMLibs::Zinc::Field field = importTypeSpecificField(fieldmodule, fieldSettings, this);
	const char *fieldName = fieldSettings["Name"].asCString();
	CMLibs::Zinc::Field existingField = this->fieldmodule.findFieldByName(fieldName);
	if (existingField.isValid())
	{
		if (existingField.getId()->compareFullDefinition(*(field.getId())))
		{
			field = existingField;
		}
		else if (existingField.getId()->hasAutomaticName())
		{
			// rename the existing automatically named field, so new field can use its name
			existingField.getId()->setNameAutomatic();
		}
		else if (cmzn_field_is_dummy_real(existingField.getId()))
		{
			// can only replace definition of a dummy real field create for apply evaluator field
			// copy new definition to existing field. Fails if new definition is incompatible
			const int copyResult = existingField.getId()->copyDefinition(*(field.getId()));
			if (copyResult == CMZN_OK)
			{
				field = existingField;
			}
			else
			{
				display_message(ERROR_MESSAGE, "Fieldmodule.readDescription:  Failed to define field %s over temporary dummy real field as defined differently", fieldName);
				field = CMLibs::Zinc::Field();
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Fieldmodule.readDescription:  Field %s is incompatible with existing field", fieldName);
			field = CMLibs::Zinc::Field();
		}
	}
	if (field.isValid())
	{
		FieldJsonIO(field, fieldmodule, FieldJsonIO::IO_MODE_IMPORT).importEntries(fieldSettings);
	}
	return field;
}

void FieldmoduleJsonImport::setManaged(const Json::Value &fieldSettings)
{
	CMLibs::Zinc::Field field(0);
	field = fieldmodule.findFieldByName(fieldSettings["Name"].asCString());
	if (field.isValid())
	{
		if (fieldSettings["IsManaged"].isBool())
		{
			field.setManaged(fieldSettings["IsManaged"].asBool());
		}
	}
}

CMLibs::Zinc::Field FieldmoduleJsonImport::getFieldByName(const char *field_name)
{
	CMLibs::Zinc::Field field = fieldmodule.findFieldByName(field_name);
	if (!field.isValid())
	{
		unsigned int index = 0;
		while (index < fieldsList.size() || field.isValid())
		{
			Json::Value fieldJson = fieldsList[index];
			if (fieldJson["Name"].isString())
			{
				if ((0 == strcmp(fieldJson["Name"].asCString(), field_name)))
				{
					field = this->importField(fieldJson);
					break;
				}
			}
			++index;
		}
		if (!field.isValid())
		{
			char* regionName = this->getRegion().getName();
			display_message(WARNING_MESSAGE, "Fieldmodule.readDescription: Region %s could not find field %s",
				regionName ? regionName : "/", field_name);
			cmzn_deallocate(regionName);
		}
	}

	return field;
}

int FieldmoduleJsonImport::import(const std::string &jsonString)
{
	int return_code = CMZN_ERROR_ARGUMENT;
	std::string returned_string;
	Json::Value root;

	if (Json::Reader().parse(jsonString, root, true))
	{
		CMLibs::Zinc::ChangeManager<CMLibs::Zinc::Fieldmodule> changeFields(this->fieldmodule);
		if (root.isObject())
		{
			fieldsList = root["Fields"];
			for (unsigned int index = 0; index < fieldsList.size(); ++index )
			{
                importField(fieldsList[index]);
			}
		}
		return_code = CMZN_OK;
	}

	return return_code;
}

std::string FieldmoduleJsonExport::getExportString()
{
	Json::Value root;

	CMLibs::Zinc::Fielditerator fielditerator =
		fieldmodule.createFielditerator();
	CMLibs::Zinc::Field field = fielditerator.next();
	while (field.isValid())
	{
		if (cmzn_field_get_type(field.getId()) != CMZN_FIELD_TYPE_INVALID)
		{
			Json::Value fieldSettings;
			FieldJsonIO(field, fieldmodule, FieldJsonIO::IO_MODE_EXPORT).exportEntries(fieldSettings);
			root["Fields"].append(fieldSettings);
		}
		field = fielditerator.next();
	}

	return Json::StyledWriter().write(root);
}
