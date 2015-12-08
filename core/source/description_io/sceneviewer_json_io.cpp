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

#include "description_io/sceneviewer_json_io.hpp"
#include "general/debug.h"
#include "zinc/scene.hpp"
#include "zinc/sceneviewer.hpp"
#include "zinc/region.hpp"
#include "region/cmiss_region.h"
#include "zinc/status.h"


void SceneviewerJsonIO::ioEntries(Json::Value &sceneviewerSettings)
{
	ioBoolEntries(sceneviewerSettings);
	ioEnumEntries(sceneviewerSettings);
	ioFilterEntries(sceneviewerSettings);
	ioSceneEntries(sceneviewerSettings);
	ioViewParameterEntries(sceneviewerSettings);
}

void SceneviewerJsonIO::ioBoolEntries(Json::Value &sceneviewerSettings)
{
	if (mode == IO_MODE_EXPORT)
	{
		sceneviewerSettings["PerturbLinesFlag"] = sceneviewer.getPerturbLinesFlag();
		sceneviewerSettings["LightingTwoSided"] = sceneviewer.isLightingTwoSided();
		sceneviewerSettings["LightingLocalViewer"] = sceneviewer.isLightingLocalViewer();
	}
	else
	{
		if (sceneviewerSettings["PerturbLinesFlag"].isBool())
			sceneviewer.setPerturbLinesFlag(sceneviewerSettings["PerturbLinesFlag"].asBool());
		if (sceneviewerSettings["LightingTwoSided"].isBool())
			sceneviewer.setLightingTwoSided(sceneviewerSettings["LightingTwoSided"].asBool());
		if (sceneviewerSettings["LightingLocalViewer"].isBool())
			sceneviewer.setLightingLocalViewer(sceneviewerSettings["LightingLocalViewer"].asBool());
	}
}

void SceneviewerJsonIO::ioEnumEntries(Json::Value &sceneviewerSettings)
{
	if (mode == IO_MODE_EXPORT)
	{
		char *return_string = cmzn_sceneviewer_projection_mode_enum_to_string(
			cmzn_sceneviewer_get_projection_mode(sceneviewer.getId()));
		sceneviewerSettings["ProjectionMode"] = return_string;
		DEALLOCATE(return_string);
		return_string = cmzn_sceneviewer_transparency_mode_enum_to_string(
			cmzn_sceneviewer_get_transparency_mode(sceneviewer.getId()));
		sceneviewerSettings["TransparencyMode"] = return_string;
		DEALLOCATE(return_string);
	}
	else
	{
		if (sceneviewerSettings["ProjectionMode"].isString())
			cmzn_sceneviewer_set_projection_mode(sceneviewer.getId(),
				cmzn_sceneviewer_projection_mode_enum_from_string(
					sceneviewerSettings["ProjectionMode"].asCString()));
		if (sceneviewerSettings["TransparencyMode"].isString())
			cmzn_sceneviewer_set_transparency_mode(sceneviewer.getId(),
				cmzn_sceneviewer_transparency_mode_enum_from_string(
					sceneviewerSettings["TransparencyMode"].asCString()));
	}
}

void SceneviewerJsonIO::ioFilterEntries(Json::Value &sceneviewerSettings)
{
	if (mode == IO_MODE_EXPORT)
	{
		OpenCMISS::Zinc::Scenefilter scenefilter = sceneviewer.getScenefilter();
		if (scenefilter.isValid())
		{
			char *name = scenefilter.getName();
			sceneviewerSettings["Scenefilter"] = name;
			DEALLOCATE(name);
		}
	}
	else
	{
		if (sceneviewerSettings["Scenefilter"].isString())
		{
			OpenCMISS::Zinc::Scene scene = sceneviewer.getScene();
			OpenCMISS::Zinc::Scenefiltermodule filtermodule = scene.getScenefiltermodule();
			OpenCMISS::Zinc::Scenefilter scenefilter = filtermodule.findScenefilterByName(
				sceneviewerSettings["Scenefilter"].asCString());
			if (scenefilter.isValid())
			{
				sceneviewer.setScenefilter(scenefilter);
			}
		}
	}
}

void SceneviewerJsonIO::ioSceneEntries(Json::Value &sceneviewerSettings)
{
	if (mode == IO_MODE_EXPORT)
	{
		OpenCMISS::Zinc::Scene scene = sceneviewer.getScene();
		OpenCMISS::Zinc::Region region = scene.getRegion();
		if (region.isValid())
		{
			char * region_path = cmzn_region_get_path(region.getId());
			sceneviewerSettings["Scene"] = region_path;
			DEALLOCATE(region_path);
		}
	}
	else
	{
		if (sceneviewerSettings["Scene"].isString())
		{
			OpenCMISS::Zinc::Scene scene = sceneviewer.getScene();
			OpenCMISS::Zinc::Region region = scene.getRegion();
			if (region.isValid())
			{
				cmzn_region *root_region = cmzn_region_get_root(region.getId());
				cmzn_region *target_region = cmzn_region_find_subregion_at_path(
					root_region, sceneviewerSettings["Scene"].asCString());
				cmzn_scene_id target_scene = cmzn_region_get_scene(target_region);
				cmzn_sceneviewer_set_scene(sceneviewer.getId(), target_scene);
				cmzn_scene_destroy(&target_scene);
				cmzn_region_destroy(&root_region);
				cmzn_region_destroy(&target_region);
			}

		}
	}
}

void SceneviewerJsonIO::ioViewParameterEntries(Json::Value &sceneviewerSettings)
{
	if (mode == IO_MODE_EXPORT)
	{
		sceneviewerSettings["AntialiasSampling"] = sceneviewer.getAntialiasSampling();
		double values3[3];
		sceneviewer.getEyePosition(&(values3[0]));
		sceneviewerSettings["EyePosition"].append(values3[0]);
		sceneviewerSettings["EyePosition"].append(values3[1]);
		sceneviewerSettings["EyePosition"].append(values3[2]);
		sceneviewer.getLookatPosition(&(values3[0]));
		sceneviewerSettings["LookatPosition"].append(values3[0]);
		sceneviewerSettings["LookatPosition"].append(values3[1]);
		sceneviewerSettings["LookatPosition"].append(values3[2]);
		sceneviewerSettings["TranslationRate"] = sceneviewer.getTranslationRate();
		sceneviewerSettings["TumbleRate"] = sceneviewer.getTumbleRate();
		sceneviewerSettings["ZoomRate"] = sceneviewer.getZoomRate();
		sceneviewer.getUpVector(&(values3[0]));
		sceneviewerSettings["UpVector"].append(values3[0]);
		sceneviewerSettings["UpVector"].append(values3[1]);
		sceneviewerSettings["UpVector"].append(values3[2]);
		sceneviewer.getBackgroundColourRGB(&(values3[0]));
		sceneviewerSettings["BackgroundColourRGB"].append(values3[0]);
		sceneviewerSettings["BackgroundColourRGB"].append(values3[1]);
		sceneviewerSettings["BackgroundColourRGB"].append(values3[2]);
		sceneviewerSettings["FarClippingPlane"] = sceneviewer.getFarClippingPlane();
		sceneviewerSettings["NearClippingPlane"] = sceneviewer.getNearClippingPlane();
		sceneviewerSettings["ViewAngle"] = sceneviewer.getViewAngle();
	}
	else
	{
		if (sceneviewerSettings["AntialiasSampling"].isInt())
			sceneviewer.setAntialiasSampling(sceneviewerSettings["AntialiasSampling"].asInt());
		if (sceneviewerSettings["EyePosition"].isArray() &&
			(sceneviewerSettings["EyePosition"].size() == 3))
		{
			double values3[3];
			values3[0] = sceneviewerSettings["EyePosition"][0].asDouble();
			values3[1] = sceneviewerSettings["EyePosition"][1].asDouble();
			values3[2] = sceneviewerSettings["EyePosition"][2].asDouble();
			sceneviewer.setEyePosition(&(values3[0]));
		}
		if (sceneviewerSettings["LookatPosition"].isArray() &&
			(sceneviewerSettings["LookatPosition"].size() == 3))
		{
			double values3[3];
			values3[0] = sceneviewerSettings["LookatPosition"][0].asDouble();
			values3[1] = sceneviewerSettings["LookatPosition"][1].asDouble();
			values3[2] = sceneviewerSettings["LookatPosition"][2].asDouble();
			sceneviewer.setLookatPosition(&(values3[0]));
		}
		if (sceneviewerSettings["TranslationRate"].isDouble())
			sceneviewer.setTranslationRate(sceneviewerSettings["TranslationRate"].asDouble());
		if (sceneviewerSettings["TumbleRate"].isDouble())
			sceneviewer.setTumbleRate(sceneviewerSettings["TumbleRate"].asDouble());
		if (sceneviewerSettings["ZoomRate"].isDouble())
			sceneviewer.setZoomRate(sceneviewerSettings["ZoomRate"].asDouble());
		if (sceneviewerSettings["UpVector"].isArray() &&
			(sceneviewerSettings["UpVector"].size() == 3))
		{
			double values3[3];
			values3[0] = sceneviewerSettings["UpVector"][0].asDouble();
			values3[1] = sceneviewerSettings["UpVector"][1].asDouble();
			values3[2] = sceneviewerSettings["UpVector"][2].asDouble();
			sceneviewer.setUpVector(&(values3[0]));
		}
		if (sceneviewerSettings["BackgroundColourRGB"].isArray() &&
			(sceneviewerSettings["BackgroundColourRGB"].size() == 3))
		{
			double values3[3];
			values3[0] = sceneviewerSettings["BackgroundColourRGB"][0].asDouble();
			values3[1] = sceneviewerSettings["BackgroundColourRGB"][1].asDouble();
			values3[2] = sceneviewerSettings["BackgroundColourRGB"][2].asDouble();
			sceneviewer.setBackgroundColourRGB(&(values3[0]));
		}
		if (sceneviewerSettings["FarClippingPlane"].isDouble())
			sceneviewer.setFarClippingPlane(sceneviewerSettings["FarClippingPlane"].asDouble());
		if (sceneviewerSettings["NearClippingPlane"].isDouble())
			sceneviewer.setNearClippingPlane(sceneviewerSettings["NearClippingPlane"].asDouble());
		if (sceneviewerSettings["ViewAngle"].isDouble())
			sceneviewer.setViewAngle(sceneviewerSettings["ViewAngle"].asDouble());
	}
}

int SceneviewerJsonImport::import(const std::string &jsonString)
{
	int return_code = CMZN_ERROR_ARGUMENT;
	std::string returned_string;
	Json::Value root;

	if (Json::Reader().parse(jsonString, root, true))
	{
		sceneviewer.beginChange();
		if (root.isObject())
		{
			ioEntries(root["Sceneviewer"]);
		}

		return_code = CMZN_OK;
		sceneviewer.endChange();
	}

	return return_code;
}

std::string SceneviewerJsonExport::getExportString()
{
	std::string returned_string;
	Json::Value sceneviewerSettings;
	ioEntries(sceneviewerSettings["Sceneviewer"]);
	returned_string = Json::StyledWriter().write(sceneviewerSettings);

	return returned_string;
}
