/**
 * FILE : material_json_io.hpp
 *
 * Interface to material serialisation in JSON format.
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (TESSELLATION_JSON_IO_HPP)
#define TESSELLATION_JSON_IO_HPP

#include "opencmiss/zinc/material.hpp"
#include <string>
#include "jsoncpp/json.h"

/*
 * Class to export attributes from material module.
 */
class MaterialmoduleJsonExport
{
private:
	OpenCMISS::Zinc::Materialmodule materialmodule;

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
	OpenCMISS::Zinc::Materialmodule materialmodule;

public:

	MaterialmoduleJsonImport(cmzn_materialmodule_id materialmodule_in) :
		materialmodule(cmzn_materialmodule_access(materialmodule_in))
	{  }

	int import(const std::string &jsonString);

	void importMaterial(Json::Value &materialSettings);
};

#endif
