/***************************************************************************//**
 * FILE : graphics_json_export.hpp
 *
 * The interface to graphics_json_export.
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (GRAPHICS_JSON_EXPORT_HPP)
#define GRAPHICS_JSON_EXPORT_HPP

#include "cmlibs/zinc/graphics.h"
#include "cmlibs/zinc/graphics.hpp"
#include "jsoncpp/json.h"
#include "description_io/graphics_json_io.hpp"
#include <string>

class GraphicsJsonExport : GraphicsJsonIO
{

public:
	/*
	 * With order set to positive integer, order will be use as key.
	 */
	GraphicsJsonExport(cmzn_graphics_id graphics_in, int order_in) :
		GraphicsJsonIO(graphics_in, GraphicsJsonIO::IO_MODE_EXPORT), order(order_in)
	{  }

	GraphicsJsonExport(const CMLibs::Zinc::Graphics &graphics_in, int order_in):
		GraphicsJsonIO(graphics_in, GraphicsJsonIO::IO_MODE_EXPORT), order(order_in)
	{	}

	std::string getExportString();

	Json::Value *exportJsonValue();

private:

	int order;
	Json::Value root;
	void addEntries();
};

#endif
