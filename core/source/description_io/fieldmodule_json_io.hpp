/***************************************************************************//**
 * FILE : fieldmodule_json_io.hpp
 *
 * The interface to fieldmodule_json_io.
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (FIELDMODULE_JSON_IO_HPP)
#define FIELDMODULE_JSON_IO_HPP

#include "opencmiss/zinc/field.hpp"
#include "opencmiss/zinc/fieldmodule.hpp"
#include <string>
#include "jsoncpp/json.h"

/*
 * Class to import attributes into field module.
 */
class FieldmoduleJsonImport
{

private:
	OpenCMISS::Zinc::Fieldmodule fieldmodule;
	Json::Value fieldsList;

public:

	FieldmoduleJsonImport(cmzn_fieldmodule_id fieldmodule_in) :
		fieldmodule(cmzn_fieldmodule_access(fieldmodule_in))
	{  }

	int import(const std::string &jsonString);

	OpenCMISS::Zinc::Field getFieldByName(const char *field_name);

	OpenCMISS::Zinc::Field importField(Json::Value &fieldSettings);

	void setManaged(Json::Value &fieldSettings);
};

/*
 * Class to export attributes from spectrum module.
 */
class FieldmoduleJsonExport
{
private:
	OpenCMISS::Zinc::Fieldmodule fieldmodule;

public:

	FieldmoduleJsonExport(cmzn_fieldmodule_id fieldmodule_in) :
		fieldmodule(cmzn_fieldmodule_access(fieldmodule_in))
	{  }

	std::string getExportString();
};
#endif
