/***************************************************************************//**
 * FILE : graphics_json_import.cpp
 *
 * The definition to graphics_json_import.
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "general/debug.h"
#include "cmlibs/zinc/graphics.hpp"
#include "description_io/graphics_json_import.hpp"
#include "cmlibs/zinc/status.h"

int GraphicsJsonImport::import()
{
	if (graphicsJson["Id"].isString())
		graphics.setName(graphicsJson["Id"].asCString());
	ioTypeEntries(graphicsJson);
	ioGeneralEntries(graphicsJson);
	ioAttributesEntries(graphicsJson);

	return CMZN_OK;
}
