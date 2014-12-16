/***************************************************************************//**
 * FILE : graphics_json_export.hpp
 *
 * The interface to graphics_json_export.
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "zinc/graphics.h"
#include "zinc/graphics.hpp"
#include "jsoncpp/json.h"
#include <string>

class GraphicsJsonExport
{

public:

	/*
	 * With order set to positive integer, order will be use as key.
	 */

	GraphicsJsonExport(cmzn_graphics_id graphics_in, int order_in) :
		graphics(cmzn_graphics_access(graphics_in)),
		order(order_in)
	{  }

	GraphicsJsonExport(const OpenCMISS::Zinc::Graphics &graphics_in, int order_in):
		graphics(graphics_in), order(order_in)
	{	}

	std::string getExportString();

	Json::Value *exportJsonValue();

private:

	OpenCMISS::Zinc::Graphics graphics;
	int order;
	Json::Value root;

	void addEntries();

	void addGeneralEntries(Json::Value &graphicsSettings);

	void addGeneralBoolEntries(Json::Value &graphicsSettings);

	void addGeneralDobuleEntries(Json::Value &graphicsSettings);

	void addGeneralEnumEntries(Json::Value &graphicsSettings);

	void addGeneralFieldEntries(Json::Value &graphicsSettings);

	void addGeneralObjectEntries(Json::Value &graphicsSettings);

	void addAttributesEntries(Json::Value &graphicsSettings);

	void addLineAttributesEntries(Json::Value &graphicsSettings);

	void addPointAttributesEntries(Json::Value &graphicsSettings);

	void addSamplingAttributesEntries(Json::Value &graphicsSettings);

	void addContoursEntries(Json::Value &graphicsSettings);

	void addLinesEntries(Json::Value &graphicsSettings);

	void addPointsEntries(Json::Value &graphicsSettings);

	void addStreamlinesEntries(Json::Value &graphicsSettings);

	void addSurfacesEntries(Json::Value &graphicsSettings);

	void addTypeEntries(Json::Value &graphicsSettings);

};

