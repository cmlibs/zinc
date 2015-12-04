/***************************************************************************//**
 * FILE : scene_json_export.cpp
 *
 * The definition to graphics_json_export.
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "general/debug.h"
#include "zinc/graphics.hpp"
#include "description_io/graphics_json_export.hpp"
#include "description_io/scene_json_export.hpp"
#include "graphics/scene.h"

std::string SceneJsonExport::getExportString()
{
	std::string returned_string;
	int currentNumber = 1;

	Json::Value scene_settings;
	bool visibility = scene.getVisibilityFlag();
	scene_settings["VisibilityFlag"] = visibility;
	Json::Value graphics_settings;
	OpenCMISS::Zinc::Graphics graphics = scene.getFirstGraphics();
	while (graphics.isValid())
	{
		GraphicsJsonExport graphicsJsonExport = GraphicsJsonExport(graphics, -1);
		Json::Value *tempValue = graphicsJsonExport.exportJsonValue();
		char order_string[5];
		sprintf(order_string, "%d", currentNumber);
		graphics_settings.append(*tempValue);
		graphics = scene.getNextGraphics(graphics);
		currentNumber++;
	}
	scene_settings["Graphics"] = graphics_settings;
	root["Scene"]= scene_settings;
	returned_string = Json::StyledWriter().write(root);

	return returned_string;
}
