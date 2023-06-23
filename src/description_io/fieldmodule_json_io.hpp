/***************************************************************************//**
 * FILE : fieldmodule_json_io.hpp
 *
 * The interface to fieldmodule_json_io.
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (FIELDMODULE_JSON_IO_HPP)
#define FIELDMODULE_JSON_IO_HPP

#include "cmlibs/zinc/field.hpp"
#include "cmlibs/zinc/fieldmodule.hpp"
#include "cmlibs/zinc/region.hpp"
#include "jsoncpp/json.h"
#include <string>

/*
 * Class to import attributes into field module.
 */
class FieldmoduleJsonImport
{

private:
	CMLibs::Zinc::Fieldmodule fieldmodule;
	Json::Value fieldsList;

public:

	FieldmoduleJsonImport(cmzn_fieldmodule_id fieldmodule_in) :
		fieldmodule(cmzn_fieldmodule_access(fieldmodule_in))
	{  }

	int import(const std::string &jsonString);

	/* Get field by name from fieldmodule if available, otherwise create it
	 * based on the json definition
	 */
	CMLibs::Zinc::Field getFieldByName(const char *field_name);

	/** Get the region currently being read or written */
	CMLibs::Zinc::Region getRegion() const
	{
		return this->fieldmodule.getRegion();
	}

	/* deserialise field definition into the fieldmodule */
	CMLibs::Zinc::Field importField(const Json::Value &fieldSettings);

	void setManaged(const Json::Value &fieldSettings);

};

/*
 * Class to export attributes from fieldmodule module.
 */
class FieldmoduleJsonExport
{
private:
	CMLibs::Zinc::Fieldmodule fieldmodule;

public:

	FieldmoduleJsonExport(cmzn_fieldmodule_id fieldmodule_in) :
		fieldmodule(cmzn_fieldmodule_access(fieldmodule_in))
	{  }

	/* function to serialise fields definition into json string */
	std::string getExportString();
};
#endif
