/***************************************************************************//**
 * FILE : sceneviewer_json_io.cpp
 *
 * The definition to sceneviewer_json_io.
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "description_io/sceneviewer_json_io.hpp"
#include "general/debug.h"
#include "opencmiss/zinc/scene.hpp"
#include "opencmiss/zinc/sceneviewer.hpp"
#include "opencmiss/zinc/region.hpp"
#include "region/cmiss_region.hpp"
#include "opencmiss/zinc/status.h"

namespace
{
	template <typename VALUETYPE> void writeArray(Json::Value &value, const char *key,
		Json::ArrayIndex valuesCount, const VALUETYPE *values)
	{
		Json::Value& v = value[key];
		v.resize(valuesCount);
		for (Json::ArrayIndex i = 0; i < valuesCount; ++i)
			v[i] = values[i];
	}

	/** Read array of doubles in value[key].
	  * @return  True if array of correct size and type is read */
	bool readArray(Json::Value &value, const char *key,
		Json::ArrayIndex valuesCount, double *values)
	{
		if (!value.isMember(key))
			return false;
		const Json::Value& v = value[key];
		if (!v.isArray() || ((v.size() != valuesCount)))
		{
			display_message(WARNING_MESSAGE, "Sceneviewer readDescription.  Value of %s is not an array of size %d", key, valuesCount);
			return false;
		}
		for (Json::ArrayIndex i = 0; i < valuesCount; ++i)
			values[i] = v[i].asDouble();
		return true;
	}

}

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
		char *enumString;
		enumString = this->sceneviewer.ProjectionModeEnumToString(this->sceneviewer.getProjectionMode());
		sceneviewerSettings["ProjectionMode"] = enumString;
		DEALLOCATE(enumString);
		enumString = this->sceneviewer.TransparencyModeEnumToString(this->sceneviewer.getTransparencyMode());
		sceneviewerSettings["TransparencyMode"] = enumString;
		DEALLOCATE(enumString);
		sceneviewerSettings["TransparencyLayers"] = this->sceneviewer.getTransparencyLayers();
	}
	else
	{
		if (sceneviewerSettings["ProjectionMode"].isString())
			this->sceneviewer.setProjectionMode(
				this->sceneviewer.ProjectionModeEnumFromString(
					sceneviewerSettings["ProjectionMode"].asCString()));
		if (sceneviewerSettings["TransparencyMode"].isString())
			this->sceneviewer.setTransparencyMode(
				this->sceneviewer.TransparencyModeEnumFromString(
					sceneviewerSettings["TransparencyMode"].asCString()));
		if (sceneviewerSettings["TransparencyLayers"].isInt())
			this->sceneviewer.setTransparencyLayers(sceneviewerSettings["TransparencyLayers"].asInt());
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
			char *regionPath = region.getPath();
			sceneviewerSettings["Scene"] = regionPath;
			DEALLOCATE(regionPath);
		}
	}
	else
	{
		if (sceneviewerSettings["Scene"].isString())
		{
			OpenCMISS::Zinc::Scene scene = sceneviewer.getScene();
			OpenCMISS::Zinc::Region rootRegion = scene.getRegion().getRoot();
			if (rootRegion.isValid())
			{
				OpenCMISS::Zinc::Region region = rootRegion.findSubregionAtPath(sceneviewerSettings["Scene"].asCString());
				this->sceneviewer.setScene(region.getScene());
			}
		}
	}
}

void SceneviewerJsonIO::ioViewParameterEntries(Json::Value &sceneviewerSettings)
{
	double values3[3], values4[4];
	if (mode == IO_MODE_EXPORT)
	{
		sceneviewerSettings["AntialiasSampling"] = sceneviewer.getAntialiasSampling();
		sceneviewer.getEyePosition(&(values3[0]));
		writeArray(sceneviewerSettings, "EyePosition", 3, values3);
		sceneviewer.getLookatPosition(&(values3[0]));
		writeArray(sceneviewerSettings, "LookatPosition", 3, values3);
		sceneviewerSettings["TranslationRate"] = sceneviewer.getTranslationRate();
		sceneviewerSettings["TumbleRate"] = sceneviewer.getTumbleRate();
		sceneviewerSettings["ZoomRate"] = sceneviewer.getZoomRate();
		sceneviewer.getUpVector(&(values3[0]));
		writeArray(sceneviewerSettings, "UpVector", 3, values3);
		sceneviewer.getBackgroundColourRGBA(values4);
		writeArray(sceneviewerSettings, "BackgroundColourRGBA", 4, values4);
		sceneviewerSettings["FarClippingPlane"] = sceneviewer.getFarClippingPlane();
		sceneviewerSettings["NearClippingPlane"] = sceneviewer.getNearClippingPlane();
		sceneviewerSettings["ViewAngle"] = sceneviewer.getViewAngle();
	}
	else
	{
		if (sceneviewerSettings["AntialiasSampling"].isInt())
			sceneviewer.setAntialiasSampling(sceneviewerSettings["AntialiasSampling"].asInt());
		if (readArray(sceneviewerSettings, "EyePosition", 3, values3))
			sceneviewer.setEyePosition(&(values3[0]));
		if (readArray(sceneviewerSettings, "LookatPosition", 3, values3))
			sceneviewer.setLookatPosition(&(values3[0]));
		if (sceneviewerSettings["TranslationRate"].isDouble())
			sceneviewer.setTranslationRate(sceneviewerSettings["TranslationRate"].asDouble());
		if (sceneviewerSettings["TumbleRate"].isDouble())
			sceneviewer.setTumbleRate(sceneviewerSettings["TumbleRate"].asDouble());
		if (sceneviewerSettings["ZoomRate"].isDouble())
			sceneviewer.setZoomRate(sceneviewerSettings["ZoomRate"].asDouble());
		if (readArray(sceneviewerSettings, "UpVector", 3, values3))
			sceneviewer.setUpVector(&(values3[0]));
		if (readArray(sceneviewerSettings, "BackgroundColourRGBA", 4, values4))
			sceneviewer.setBackgroundColourRGBA(values4);
		else if (readArray(sceneviewerSettings, "BackgroundColourRGB", 3, values3))
			sceneviewer.setBackgroundColourRGB(values3);
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
			ioEntries(root);
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
	ioEntries(sceneviewerSettings);
	returned_string = Json::StyledWriter().write(sceneviewerSettings);

	return returned_string;
}
