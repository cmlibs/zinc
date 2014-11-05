/**
 * @file streamscene.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_STREAMSCENE_HPP__
#define CMZN_STREAMSCENE_HPP__

#include "zinc/streamscene.h"
#include "zinc/scene.hpp"
#include "zinc/stream.hpp"
#include "zinc/scenefilter.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class StreaminformationScene : public Streaminformation
{
public:
	StreaminformationScene() : Streaminformation()
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit StreaminformationScene(cmzn_streaminformation_scene_id streaminformation_scene_id) :
		Streaminformation(reinterpret_cast<cmzn_streaminformation_id>(streaminformation_scene_id))
	{ }

	bool isValid() const
	{
		return (0 != id);
	}

	cmzn_streaminformation_scene_id getId() const
	{
		return reinterpret_cast<cmzn_streaminformation_scene_id>(id);
	}

	enum ExportDataType
	{
		EXPORT_DATA_TYPE_INVALID = CMZN_STREAMINFORMATION_SCENE_EXPORT_DATA_TYPE_INVALID,
		EXPORT_DATA_TYPE_COLOUR = CMZN_STREAMINFORMATION_SCENE_EXPORT_DATA_TYPE_COLOUR ,
		EXPORT_DATA_TYPE_PER_VERTEX_VALUE = CMZN_STREAMINFORMATION_SCENE_EXPORT_DATA_TYPE_PER_VERTEX_VALUE,
		EXPORT_DATA_TYPE_PER_FACE_VALUE = CMZN_STREAMINFORMATION_SCENE_EXPORT_DATA_TYPE_PER_FACE_VALUE
	};

	enum ExportFormat
	{
		EXPORT_FORMAT_INVALID = CMZN_STREAMINFORMATION_SCENE_EXPORT_FORMAT_INVALID,
		EXPORT_FORMAT_THREEJS = CMZN_STREAMINFORMATION_SCENE_EXPORT_FORMAT_THREEJS
	};

	Scenefilter getScenefilter()
	{
		return Scenefilter(cmzn_streaminformation_scene_get_scenefilter(getId()));
	}

	int setScenefilter(const Scenefilter& scenefilter)
	{
		return cmzn_streaminformation_scene_set_scenefilter(getId(), scenefilter.getId());
	}

	ExportDataType getExportDataType()
	{
		return static_cast<ExportDataType>(cmzn_streaminformation_scene_get_export_data_type(getId()));
	}

	int setExportDataType(ExportDataType exportDataType)
	{
		return cmzn_streaminformation_scene_set_export_data_type(getId(),
			static_cast<cmzn_streaminformation_scene_export_data_type>(exportDataType));
	}

	ExportFormat getExportFormat()
	{
		return static_cast<ExportFormat>(cmzn_streaminformation_scene_get_export_format(getId()));
	}

	int setExportFormat(ExportFormat exportFormat)
	{
		return cmzn_streaminformation_scene_set_export_format(getId(),
			static_cast<cmzn_streaminformation_scene_export_format>(exportFormat));
	}

	int getNumberOfTimeSteps()
	{
		return cmzn_streaminformation_scene_get_number_of_time_steps(getId());
	}

	int setNumberOfTimeSteps(int numberOfTimeSteps)
	{
		return cmzn_streaminformation_scene_set_number_of_time_steps(getId(), numberOfTimeSteps);
	}

	double getFinishTime()
	{
		return cmzn_streaminformation_scene_get_finish_time(getId());
	}

	int setFinishTime(double finishTime)
	{
		return cmzn_streaminformation_scene_set_finish_time(getId(), finishTime);
	}

	double getInitialTime()
	{
		return cmzn_streaminformation_scene_get_initial_time(getId());
	}

	int setInitialTime(double initialTime)
	{
		return cmzn_streaminformation_scene_set_initial_time(getId(), initialTime);
	}

	int getNumberOfResourcesRequired()
	{
		return cmzn_streaminformation_scene_get_number_of_resources_required(getId());
	}

};

inline StreaminformationScene Streaminformation::castScene()
{
	return StreaminformationScene(cmzn_streaminformation_cast_scene(id));
}

inline StreaminformationScene Scene::createStreaminformationScene()
{
	return StreaminformationScene(reinterpret_cast<cmzn_streaminformation_scene_id>(
		cmzn_scene_create_streaminformation_scene(id)));
}

inline int Scene::exportScene(const StreaminformationScene& StreaminformationScene)
{
	return cmzn_scene_export_scene(id, StreaminformationScene.getId());
}


}  // namespace Zinc
}

#endif
