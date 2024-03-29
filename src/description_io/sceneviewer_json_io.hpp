/***************************************************************************//**
 * FILE : sceneviewer_json_io.hpp
 *
 * The interface to sceneviewer_json_io.
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (SCENEVIEWER_JSON_IO_HPP)
#define SCENEVIEWER_JSON_IO_HPP

#include "cmlibs/zinc/sceneviewer.hpp"
#include <string>
#include "jsoncpp/json.h"

/*
 * Class to import/export attributes into/from scene viewer.
 */
class SceneviewerJsonIO
{

public:
	enum IOMode
	{
		IO_MODE_INVALID = 0,
		IO_MODE_IMPORT = 1,
		IO_MODE_EXPORT= 2
	};

	SceneviewerJsonIO(cmzn_sceneviewer_id sceneviewer_in, IOMode mode_in) :
		sceneviewer(cmzn_sceneviewer_access(sceneviewer_in)), mode(mode_in)
	{  }

	SceneviewerJsonIO(const CMLibs::Zinc::Sceneviewer sceneviewer_in, IOMode mode_in) :
		sceneviewer(sceneviewer_in), mode(mode_in)
	{	}

protected:

	CMLibs::Zinc::Sceneviewer sceneviewer;
	IOMode mode;

	void ioEntries(Json::Value &sceneviewerSettings);

	void ioBoolEntries(Json::Value &sceneviewerSettings);

	void ioEnumEntries(Json::Value &sceneviewerSettings);

	void ioFilterEntries(Json::Value &sceneviewerSettings);

	void ioSceneEntries(Json::Value &sceneviewerSettings);

	void ioViewParameterEntries(Json::Value &sceneviewerSettings);

};

/*
 * Class to import attributes into scene viewer.
 */
class SceneviewerJsonImport : SceneviewerJsonIO
{

public:

	SceneviewerJsonImport(cmzn_sceneviewer_id sceneviewer_in) :
		SceneviewerJsonIO(sceneviewer_in, SceneviewerJsonIO::IO_MODE_IMPORT)
	{  }

	int import(const std::string &jsonString);

};

/*
 * Class to export attributes from scene viewer.
 */
class SceneviewerJsonExport : SceneviewerJsonIO
{

public:

	SceneviewerJsonExport(cmzn_sceneviewer_id sceneviewer_in) :
		SceneviewerJsonIO(sceneviewer_in, SceneviewerJsonIO::IO_MODE_EXPORT)
	{  }

	std::string getExportString();
};

#endif
