/**
 * FILE : material_json_io.hpp
 *
 * Interface to material serialisation in JSON format.
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (MATERIAL_JSON_IO_HPP)
#define MATERIAL_JSON_IO_HPP

#include "cmlibs/zinc/material.hpp"
#include <string>
#include "jsoncpp/json.h"

/*
 * Class to export attributes from material module.
 */
class MaterialmoduleJsonExport
{
private:
	CMLibs::Zinc::Materialmodule materialmodule;

public:

	MaterialmoduleJsonExport(cmzn_materialmodule_id materialmodule_in) :
		materialmodule(cmzn_materialmodule_access(materialmodule_in))
	{  }

	std::string getExportString();
};

/*
 * Class to import attributes into material module.
 */
class MaterialmoduleJsonImport
{
private:
	CMLibs::Zinc::Materialmodule materialmodule;

public:

	MaterialmoduleJsonImport(cmzn_materialmodule_id materialmodule_in) :
		materialmodule(cmzn_materialmodule_access(materialmodule_in))
	{  }

	int import(const std::string &jsonString);

	int importMaterial(Json::Value &materialSettings);
};

#endif
