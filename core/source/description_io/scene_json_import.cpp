/***************************************************************************//**
 * FILE : scene_json_import.cpp
 *
 * The definition to scene_json_import.
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "general/debug.h"
#include "opencmiss/zinc/graphics.hpp"
#include "description_io/graphics_json_import.hpp"
#include "description_io/scene_json_import.hpp"
#include "graphics/scene.h"
#include "opencmiss/zinc/status.h"

int SceneJsonImport::import(const std::string &jsonString)
{
	int return_code = CMZN_ERROR_ARGUMENT;
	std::string returned_string;
	Json::Value root;

	if (Json::Reader().parse(jsonString, root, true))
	{
		scene.beginChange();
		if (overwrite == 1)
		{
			scene.removeAllGraphics();
		}
		if (root.isObject())
		{
			if (root["VisibilityFlag"].isBool())
				scene.setVisibilityFlag(root["VisibilityFlag"].asBool());
			Json::Value graphicsSettings = root["Graphics"];
			for (unsigned int index = 0; index < graphicsSettings.size(); ++index )
			{
				importGraphics(graphicsSettings[index]);
			}
		}

		return_code = CMZN_OK;
		scene.endChange();
	}

	return return_code;
}

void SceneJsonImport::importGraphics(Json::Value &graphicsJson)
{
	std::string typeString = graphicsJson["Type"].asString();
	enum OpenCMISS::Zinc::Graphics::Type graphicsType =  static_cast<OpenCMISS::Zinc::Graphics::Type>(
		cmzn_graphics_type_enum_from_string(typeString.c_str()));
	OpenCMISS::Zinc::Graphics graphics = scene.createGraphics(graphicsType);
	GraphicsJsonImport(graphics, graphicsJson).import();
}
