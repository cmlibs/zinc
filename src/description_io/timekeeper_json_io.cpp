/***************************************************************************//**
 * FILE : timekeeper_json_io.cpp
 *
 * The definition to timekeeper_json_io.
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "description_io/timekeeper_json_io.hpp"
#include "general/debug.h"
#include "cmlibs/zinc/timekeeper.hpp"
#include "cmlibs/zinc/status.h"

void TimekeeperJsonIO::ioEntries(Json::Value &timekeeperSettings)
{
	if (mode == IO_MODE_EXPORT)
	{
		timekeeperSettings["MaximumTime"] = timekeeper.getMaximumTime();
		timekeeperSettings["MinimumTime"] = timekeeper.getMinimumTime();
		timekeeperSettings["Time"] = timekeeper.getTime();
	}
	else
	{
		if (timekeeperSettings["MaximumTime"].isDouble())
		{
			timekeeper.setMaximumTime(timekeeperSettings["MaximumTime"].asDouble());
		}
		if (timekeeperSettings["MinimumTime"].isDouble())
		{
			timekeeper.setMinimumTime(timekeeperSettings["MinimumTime"].asDouble());
		}
		if (timekeeperSettings["Time"].isDouble())
		{
			timekeeper.setTime(timekeeperSettings["Time"].asDouble());
		}
	}
}

int TimekeepermoduleJsonImport::import(const std::string &jsonString)
{
	int return_code = CMZN_ERROR_ARGUMENT;
	std::string returned_string;
	Json::Value root;

	if (Json::Reader().parse(jsonString, root, true))
	{
		if (root.isObject())
		{
			Json::Value timekeeperJson = root["Timekeeper"];
			if (timekeeperJson.size() > 0)
				importTimekeeper(timekeeperJson[0]);
		}
		return_code = CMZN_OK;
	}

	return return_code;
}

void TimekeepermoduleJsonImport::importTimekeeper(Json::Value &timekeeperSettings)
{
	CMLibs::Zinc::Timekeeper timekeeper = timekeepermodule.getDefaultTimekeeper();
	TimekeeperJsonIO(timekeeper, IO_MODE_IMPORT).ioEntries(timekeeperSettings);
}

std::string TimekeepermoduleJsonExport::getExportString()
{
	Json::Value root;

	CMLibs::Zinc::Timekeeper timekeeper = timekeepermodule.getDefaultTimekeeper();
	if (timekeeper.isValid())
	{
		Json::Value timekeeperSettings;
		TimekeeperJsonIO(timekeeper, IO_MODE_EXPORT).ioEntries(timekeeperSettings);
		root["Timekeeper"].append(timekeeperSettings);
	}

	return Json::StyledWriter().write(root);
}
