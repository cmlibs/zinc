/***************************************************************************//**
 * FILE : graphics_json_io.cpp
 *
 * The definition to graphics_json_io.
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "graphics/glyph.hpp"
#include "graphics/graphics.hpp"
#include "opencmiss/zinc/spectrum.hpp"
#include "opencmiss/zinc/scene.hpp"
#include "opencmiss/zinc/fieldmodule.hpp"
#include "general/debug.h"
#include "description_io/graphics_json_export.hpp"
#include <string.h>

OpenCMISS::Zinc::Field getFieldByName(OpenCMISS::Zinc::Graphics &graphics, const char *name)
{
	OpenCMISS::Zinc::Fieldmodule fm = graphics.getScene().getRegion().getFieldmodule();
	return fm.findFieldByName(name);
}

void GraphicsJsonIO::ioGeneralFieldEntries(Json::Value &graphicsSettings)
{
	if (mode == IO_MODE_EXPORT)
	{
		OpenCMISS::Zinc::Field field = graphics.getCoordinateField();
		char *fieldName = 0;
		if (field.isValid())
		{
			fieldName = field.getName();
			graphicsSettings["CoordinateField"] = fieldName;
			DEALLOCATE(fieldName);
		}
		field = graphics.getDataField();
		if (field.isValid())
		{
			fieldName = field.getName();
			graphicsSettings["DataField"] = fieldName;
			DEALLOCATE(fieldName);
		}
		field = graphics.getSubgroupField();
		if (field.isValid())
		{
			fieldName = field.getName();
			graphicsSettings["SubgroupField"] = fieldName;
			DEALLOCATE(fieldName);
		}
		field = graphics.getTextureCoordinateField();
		if (field.isValid())
		{
			fieldName = field.getName();
			graphicsSettings["TextureCoordinateField"] = fieldName;
			DEALLOCATE(fieldName);
		}
		field = graphics.getTessellationField();
		if (field.isValid())
		{
			fieldName = field.getName();
			graphicsSettings["TessellationField"] = fieldName;
			DEALLOCATE(fieldName);
		}
	}
	else
	{
		if (graphicsSettings["CoordinateField"].isString())
			graphics.setCoordinateField(getFieldByName(graphics,
				graphicsSettings["CoordinateField"].asCString()));

		if (graphicsSettings["DataField"].isString())
			graphics.setDataField(getFieldByName(graphics,
				graphicsSettings["DataField"].asCString()));

		if (graphicsSettings["SubgroupField"].isString())
			graphics.setSubgroupField(getFieldByName(graphics,
				graphicsSettings["SubgroupField"].asCString()));

		if (graphicsSettings["TextureCoordinateField"].isString())
			graphics.setTextureCoordinateField(getFieldByName(graphics,
				graphicsSettings["TextureCoordinateField"].asCString()));

		if (graphicsSettings["TessellationField"].isString())
			graphics.setTessellationField(getFieldByName(graphics,
				graphicsSettings["TessellationField"].asCString()));

	}
}

void GraphicsJsonIO::ioGeneralBoolEntries(Json::Value &graphicsSettings)
{
	if (mode == IO_MODE_EXPORT)
	{
		bool value = graphics.getVisibilityFlag();
		graphicsSettings["VisibilityFlag"] = value;

		// Now exported as enum BoundaryMode
		//value = graphics.isExterior();
		//graphicsSettings["Exterior"] = value;
	}
	else
	{
		if (graphicsSettings["VisibilityFlag"].isBool())
			graphics.setVisibilityFlag(graphicsSettings["VisibilityFlag"].asBool());
		if (graphicsSettings["Exterior"].isBool())
			graphics.setExterior(graphicsSettings["Exterior"].asBool());
	}
}

void GraphicsJsonIO::ioGeneralObjectEntries(Json::Value &graphicsSettings)
{
	if (mode == IO_MODE_EXPORT)
	{
		OpenCMISS::Zinc::Material material = graphics.getMaterial();
		char *name = 0;
		if (material.isValid())
		{
			name = material.getName();
			graphicsSettings["Material"] = name;
			DEALLOCATE(name);
		}
		material = graphics.getSelectedMaterial();
		if (material.isValid())
		{
			name = material.getName();
			graphicsSettings["SelectedMaterial"] = name;
			DEALLOCATE(name);
		}
		OpenCMISS::Zinc::Spectrum spectrum = graphics.getSpectrum();
		if (spectrum.isValid())
		{
			name = spectrum.getName();
			graphicsSettings["Spectrum"] = name;
			DEALLOCATE(name);
		}
		OpenCMISS::Zinc::Tessellation tessellation = graphics.getTessellation();
		if (tessellation.isValid())
		{
			name = tessellation.getName();
			graphicsSettings["Tessellation"] = name;
			DEALLOCATE(name);
		}
	}
	else
	{
		if (graphicsSettings["Material"].isString())
		{
			OpenCMISS::Zinc::Material material = graphics.getScene().getMaterialmodule().findMaterialByName(
				graphicsSettings["Material"].asCString());
			graphics.setMaterial(material);
		}
		if (graphicsSettings["SelectedMaterial"].isString())
		{
			OpenCMISS::Zinc::Material material = graphics.getScene().getMaterialmodule().findMaterialByName(
				graphicsSettings["SelectedMaterial"].asCString());
			graphics.setSelectedMaterial(material);
		}
		if (graphicsSettings["Spectrum"].isString())
		{
			OpenCMISS::Zinc::Spectrum spectrum = graphics.getScene().getSpectrummodule().findSpectrumByName(
				graphicsSettings["Spectrum"].asCString());
			graphics.setSpectrum(spectrum);
		}
		if (graphicsSettings["Tessellation"].isString())
		{
			OpenCMISS::Zinc::Tessellation tessellation = graphics.getScene().getTessellationmodule().findTessellationByName(
				graphicsSettings["Tessellation"].asCString());
			graphics.setTessellation(tessellation);
		}
	}

}

void GraphicsJsonIO::ioGeneralDoubleEntries(Json::Value &graphicsSettings)
{
	if (mode == IO_MODE_EXPORT)
	{
		double value = graphics.getRenderLineWidth();
		graphicsSettings["RenderLineWidth"] = value;

		value = graphics.getRenderPointSize();
		graphicsSettings["RenderPointSize"] = value;
	}
	else
	{
		if (graphicsSettings["RenderLineWidth"].isDouble())
			graphics.setRenderLineWidth(graphicsSettings["RenderLineWidth"].asDouble());
		if (graphicsSettings["RenderPointSize"].isDouble())
			graphics.setRenderLineWidth(graphicsSettings["RenderPointSize"].asDouble());
	}
}

void GraphicsJsonIO::ioGeneralEnumEntries(Json::Value &graphicsSettings)
{
	if (mode == IO_MODE_EXPORT)
	{
		char *enumString = nullptr;
		
		enumString = graphics.RenderPolygonModeEnumToString(graphics.getRenderPolygonMode());
		if (enumString)
		{
			graphicsSettings["RenderPolygonMode"] = enumString;
			DEALLOCATE(enumString);
		}

		enumString = graphics.SelectModeEnumToString(graphics.getSelectMode());
		if (enumString)
		{
			graphicsSettings["SelectMode"] = enumString;
			DEALLOCATE(enumString);
		}

		enumString = OpenCMISS::Zinc::ScenecoordinatesystemEnumToString(graphics.getScenecoordinatesystem());
		if (enumString)
		{
			graphicsSettings["Scenecoordinatesystem"] = enumString;
			DEALLOCATE(enumString);
		}

		enumString = OpenCMISS::Zinc::Field::DomainTypeEnumToString(graphics.getFieldDomainType());
		if (enumString)
		{
			graphicsSettings["FieldDomainType"] = enumString;
			DEALLOCATE(enumString);
		}

		enumString = graphics.BoundaryModeEnumToString(graphics.getBoundaryMode());
		if (enumString)
		{
			graphicsSettings["BoundaryMode"] = enumString;
			DEALLOCATE(enumString);
		}

		enumString = OpenCMISS::Zinc::Element::FaceTypeEnumToString(graphics.getElementFaceType());
		if (enumString)
		{
			graphicsSettings["ElementFaceType"] = enumString;
			DEALLOCATE(enumString);
		}
	}
	else
	{
		if (graphicsSettings["RenderPolygonMode"].isString())
		{
			const char *renderPolygonModeName = graphicsSettings["RenderPolygonMode"].asCString();
			// migrate legacy non-standard RenderPolygonMode names to enums
			OpenCMISS::Zinc::Graphics::RenderPolygonMode renderPolygonMode;
			if (0 == strcmp("RENDER_POLYGON_SHADED", renderPolygonModeName))
				renderPolygonMode = OpenCMISS::Zinc::Graphics::RENDER_POLYGON_MODE_SHADED;
			else if (0 == strcmp("RENDER_POLYGON_WIREFRAME", renderPolygonModeName))
				renderPolygonMode = OpenCMISS::Zinc::Graphics::RENDER_POLYGON_MODE_WIREFRAME;
			else
				renderPolygonMode = OpenCMISS::Zinc::Graphics::RenderPolygonModeEnumFromString(renderPolygonModeName);
			graphics.setRenderPolygonMode(renderPolygonMode);
		}

		if (graphicsSettings["SelectMode"].isString())
			graphics.setSelectMode(graphics.SelectModeEnumFromString(
				graphicsSettings["SelectMode"].asCString()));

		if (graphicsSettings["Scenecoordinatesystem"].isString())
			graphics.setScenecoordinatesystem(OpenCMISS::Zinc::ScenecoordinatesystemEnumFromString(
				graphicsSettings["Scenecoordinatesystem"].asCString()));

		if (graphicsSettings["FieldDomainType"].isString())
			graphics.setFieldDomainType(OpenCMISS::Zinc::Field::DomainTypeEnumFromString(
					graphicsSettings["FieldDomainType"].asCString()));

		if (graphicsSettings["BoundaryMode"].isString())
			graphics.setBoundaryMode(graphics.BoundaryModeEnumFromString(
				graphicsSettings["BoundaryMode"].asCString()));

		if (graphicsSettings["ElementFaceType"].isString())
			graphics.setElementFaceType(OpenCMISS::Zinc::Element::FaceTypeEnumFromString(
				graphicsSettings["ElementFaceType"].asCString()));
	}
}

void GraphicsJsonIO::ioGeneralEntries(Json::Value &graphicsSettings)
{
	ioGeneralBoolEntries(graphicsSettings);
	ioGeneralFieldEntries(graphicsSettings);
	ioGeneralDoubleEntries(graphicsSettings);
	ioGeneralEnumEntries(graphicsSettings);
	ioGeneralObjectEntries(graphicsSettings);
}

void GraphicsJsonIO::ioLineAttributesEntries(Json::Value &graphicsSettings)
{
	OpenCMISS::Zinc::Graphicslineattributes lineAttributes = graphics.getGraphicslineattributes();
	if (lineAttributes.isValid())
	{
		if (mode == IO_MODE_EXPORT)
		{
			Json::Value attributesSettings;
			double values[2];
			lineAttributes.getBaseSize(2, &(values[0]));
			attributesSettings["BaseSize"].append(values[0]);
			attributesSettings["BaseSize"].append(values[1]);
			char *name = 0;
			OpenCMISS::Zinc::Field field = lineAttributes.getOrientationScaleField();
			if (field.isValid())
			{
				name = field.getName();
				attributesSettings["OrientationScaleField"] = name;
				DEALLOCATE(name);
			}
			lineAttributes.getScaleFactors(2, &(values[0]));
			attributesSettings["ScaleFactors"].append(values[0]);
			attributesSettings["ScaleFactors"].append(values[1]);
			char *enumString = lineAttributes.ShapeTypeEnumToString(lineAttributes.getShapeType());
			if (enumString)
			{
				attributesSettings["ShapeType"] = enumString;
				DEALLOCATE(enumString);
			}
			graphicsSettings["LineAttributes"] = attributesSettings;
		}
		else if (graphicsSettings["LineAttributes"].isObject())
		{
			Json::Value attributesSettings = graphicsSettings["LineAttributes"];
			if (attributesSettings["BaseSize"].isArray() &&
				(attributesSettings["BaseSize"].size() == 2))
			{
				double values[2];
				values[0] = attributesSettings["BaseSize"][0].asDouble();
				values[1] = attributesSettings["BaseSize"][1].asDouble();
				lineAttributes.setBaseSize(2, &(values[0]));
			}
			if (attributesSettings["OrientationScaleField"].isString())
				lineAttributes.setOrientationScaleField(getFieldByName(graphics,
					attributesSettings["OrientationScaleField"].asCString()));
			if (attributesSettings["ScaleFactors"].isArray() &&
				(attributesSettings["ScaleFactors"].size() == 2))
			{
				double values[2];
				values[0] = attributesSettings["ScaleFactors"][0].asDouble();
				values[1] = attributesSettings["ScaleFactors"][1].asDouble();
				lineAttributes.setScaleFactors(2, &(values[0]));
			}
			if (attributesSettings["ShapeType"].isString())
				lineAttributes.setShapeType(lineAttributes.ShapeTypeEnumFromString(
					attributesSettings["ShapeType"].asCString()));
		}
	}
}


void GraphicsJsonIO::ioPointAttributesEntries(Json::Value &graphicsSettings)
{
	OpenCMISS::Zinc::Graphicspointattributes pointAttributes = graphics.getGraphicspointattributes();
	if (pointAttributes.isValid())
	{
		if (mode == IO_MODE_EXPORT)
		{
			Json::Value attributesSettings;
			double values[3];
			pointAttributes.getBaseSize(3, &(values[0]));
			attributesSettings["BaseSize"].append(values[0]);
			attributesSettings["BaseSize"].append(values[1]);
			attributesSettings["BaseSize"].append(values[2]);
			char *name = 0;
			OpenCMISS::Zinc::Font font = pointAttributes.getFont();
			if (font.isValid())
			{
				name = font.getName();
				attributesSettings["Font"] = name;
				DEALLOCATE(name);
			}
			OpenCMISS::Zinc::Glyph glyph = pointAttributes.getGlyph();
			if (glyph.isValid())
			{
				name = glyph.getName();
				attributesSettings["Glyph"] = name;
				DEALLOCATE(name);
			}
			pointAttributes.getGlyphOffset(3, &(values[0]));
			attributesSettings["GlyphOffset"].append(values[0]);
			attributesSettings["GlyphOffset"].append(values[1]);
			attributesSettings["GlyphOffset"].append(values[2]);
			char *enumString = OpenCMISS::Zinc::Glyph::RepeatModeEnumToString(pointAttributes.getGlyphRepeatMode());
			if (enumString)
			{
				attributesSettings["GlyphRepeatMode"] = enumString;
				DEALLOCATE(enumString);
			}
			OpenCMISS::Zinc::Field field = pointAttributes.getLabelField();
			if (field.isValid())
			{
				name = field.getName();
				attributesSettings["LabelField"] = name;
				DEALLOCATE(name);
			}
			pointAttributes.getLabelOffset(3, &(values[0]));
			attributesSettings["LabelOffset"].append(values[0]);
			attributesSettings["LabelOffset"].append(values[1]);
			attributesSettings["LabelOffset"].append(values[2]);
			for (int i = 0; i < 3; i++)
			{
				name = pointAttributes.getLabelText(i+1);
				if (name)
				{
					attributesSettings["LabelText"].append(name);
					DEALLOCATE(name);
				}
				else
				{
					attributesSettings["LabelText"].append("");
				}
			}
			field = pointAttributes.getOrientationScaleField();
			if (field.isValid())
			{
				name = field.getName();
				attributesSettings["OrientationScaleField"] = name;
				DEALLOCATE(name);
			}
			pointAttributes.getScaleFactors(3, &(values[0]));
			attributesSettings["ScaleFactors"].append(values[0]);
			attributesSettings["ScaleFactors"].append(values[1]);
			attributesSettings["ScaleFactors"].append(values[2]);
			field = pointAttributes.getSignedScaleField();
			if (field.isValid())
			{
				name = field.getName();
				attributesSettings["SignedScaleField"] = name;
				DEALLOCATE(name);
			}
			graphicsSettings["PointAttributes"] = attributesSettings;
		}
		else if (graphicsSettings["PointAttributes"].isObject())
		{
			Json::Value attributesSettings = graphicsSettings["PointAttributes"];
			if (attributesSettings["BaseSize"].isArray() &&
				(attributesSettings["BaseSize"].size() == 3))
			{
				double values[3];
				values[0] = attributesSettings["BaseSize"][0].asDouble();
				values[1] = attributesSettings["BaseSize"][1].asDouble();
				values[2] = attributesSettings["BaseSize"][2].asDouble();
				pointAttributes.setBaseSize(3, &(values[0]));
			}
			if (attributesSettings["Font"].isString())
			{
				OpenCMISS::Zinc::Font font = graphics.getScene().getFontmodule().findFontByName(
					attributesSettings["Font"].asCString());
				pointAttributes.setFont(font);
			}
			if (attributesSettings["Glyph"].isString())
			{
				OpenCMISS::Zinc::Glyph glyph = graphics.getScene().getGlyphmodule().findGlyphByName(
					attributesSettings["Glyph"].asCString());
				pointAttributes.setGlyph(glyph);
			}
			if (attributesSettings["GlyphOffset"].isArray() &&
				(attributesSettings["GlyphOffset"].size() == 3))
			{
				double values[3];
				values[0] = attributesSettings["GlyphOffset"][0].asDouble();
				values[1] = attributesSettings["GlyphOffset"][1].asDouble();
				values[2] = attributesSettings["GlyphOffset"][2].asDouble();
				pointAttributes.setGlyphOffset(3, &(values[0]));
			}
			if (attributesSettings["GlyphRepeatMode"].isString())
				pointAttributes.setGlyphRepeatMode(OpenCMISS::Zinc::Glyph::RepeatModeEnumFromString(
					attributesSettings["GlyphRepeatMode"].asCString()));
			if (attributesSettings["LabelField"].isString())
				pointAttributes.setLabelField(getFieldByName(graphics,
					attributesSettings["LabelField"].asCString()));
			if (attributesSettings["LabelOffset"].isArray() &&
				(attributesSettings["LabelOffset"].size() == 3))
			{
				double values[3];
				values[0] = attributesSettings["LabelOffset"][0].asDouble();
				values[1] = attributesSettings["LabelOffset"][1].asDouble();
				values[2] = attributesSettings["LabelOffset"][2].asDouble();
				pointAttributes.setLabelOffset(3, &(values[0]));
			}
			if (attributesSettings["LabelText"].isArray() &&
				(attributesSettings["LabelText"].size() == 3))
			{
				for (int i = 0; i < 3; i++)
				{
					if (attributesSettings["LabelText"][i].isString())
					{
						const char *label = attributesSettings["LabelText"][i].asCString();
						if (0 != strcmp(label, ""))
						{
							pointAttributes.setLabelText(i+1, label);
						}
					}
				}
			}
			if (attributesSettings["OrientationScaleField"].isString())
				pointAttributes.setOrientationScaleField(getFieldByName(graphics,
					attributesSettings["OrientationScaleField"].asCString()));
			if (attributesSettings["ScaleFactors"].isArray() &&
				(attributesSettings["ScaleFactors"].size() == 3))
			{
				double values[3];
				values[0] = attributesSettings["ScaleFactors"][0].asDouble();
				values[1] = attributesSettings["ScaleFactors"][1].asDouble();
				values[2] = attributesSettings["ScaleFactors"][2].asDouble();
				pointAttributes.setScaleFactors(3, &(values[0]));
			}
			if (attributesSettings["SignedScaleField"].isString())
				pointAttributes.setSignedScaleField(getFieldByName(graphics,
					attributesSettings["SignedScaleField"].asCString()));
		}
	}
}

void GraphicsJsonIO::ioSamplingAttributesEntries(Json::Value &graphicsSettings)
{
	OpenCMISS::Zinc::Graphicssamplingattributes samplingAttributes = graphics.getGraphicssamplingattributes();
	if (samplingAttributes.isValid())
	{
		if (mode == IO_MODE_EXPORT)
		{
			Json::Value attributesSettings;
			OpenCMISS::Zinc::Field field = samplingAttributes.getDensityField();
			char *name = 0;
			if (field.isValid())
			{
				name = field.getName();
				attributesSettings["DensityField"] = name;
				DEALLOCATE(name);
			}
			double values[3];
			samplingAttributes.getLocation(3, &(values[0]));
			attributesSettings["Location"].append(values[0]);
			attributesSettings["Location"].append(values[1]);
			attributesSettings["Location"].append(values[2]);
			char *enumString = OpenCMISS::Zinc::Element::PointSamplingModeEnumToString(
				samplingAttributes.getElementPointSamplingMode());
			if (enumString)
			{
				attributesSettings["ElementPointSamplingMode"] = enumString;
				DEALLOCATE(enumString);
			}
			graphicsSettings["SamplingAttributes"] = attributesSettings;
		}
		else if (graphicsSettings["SamplingAttributes"].isObject())
		{
			Json::Value attributesSettings = graphicsSettings["SamplingAttributes"];
			if (attributesSettings["DensityField"].isString())
				samplingAttributes.setDensityField(getFieldByName(graphics,
					attributesSettings["DensityField"].asCString()));
			if (attributesSettings["Location"].isArray() &&
				(attributesSettings["Location"].size() == 3))
			{
				double values[3];
				values[0] = attributesSettings["Location"][0].asDouble();
				values[1] = attributesSettings["Location"][1].asDouble();
				values[2] = attributesSettings["Location"][2].asDouble();
				samplingAttributes.setLocation(3, &(values[0]));
			}
			if (attributesSettings["ElementPointSamplingMode"].isString())
				samplingAttributes.setElementPointSamplingMode(
					OpenCMISS::Zinc::Element::PointSamplingModeEnumFromString(
						attributesSettings["ElementPointSamplingMode"].asCString()));
		}
	}
}

void GraphicsJsonIO::ioAttributesEntries(Json::Value &graphicsSettings)
{
	ioLineAttributesEntries(graphicsSettings);
	ioPointAttributesEntries(graphicsSettings);
	ioSamplingAttributesEntries(graphicsSettings);
}

void GraphicsJsonIO::ioContoursEntries(Json::Value &graphicsSettings)
{
	OpenCMISS::Zinc::GraphicsContours contours = graphics.castContours();
	if (contours.isValid())
	{
		if (mode == IO_MODE_EXPORT)
		{
			Json::Value attributesSettings;
			OpenCMISS::Zinc::Field field = contours.getIsoscalarField();
			char *name = 0;
			if (field.isValid())
			{
				name = field.getName();
				attributesSettings["IsoscalarField"] = name;
				DEALLOCATE(name);
			}
			int num = 0;
			double temp = 0.0;
			num = contours.getListIsovalues(1, &temp);
			if (num > 0)
			{
				double *listValues = new double[num];
				contours.getListIsovalues(num, listValues);
				for (int i = 0; i < num; i++)
					attributesSettings["ListIsovalues"].append(listValues[i]);
				delete[] listValues;
			}
			else
			{
				temp = contours.getRangeFirstIsovalue();
				attributesSettings["RangeFirstIsovalue"] = temp;
				temp = contours.getRangeLastIsovalue();
				attributesSettings["RangeLastIsovalue"] = temp;
				num = contours.getRangeNumberOfIsovalues();
				attributesSettings["RangeNumberOfIsovalues"] = num;
			}
			graphicsSettings["Contours"] = attributesSettings;
		}
		else if (graphicsSettings["Contours"].isObject())
		{
			Json::Value attributesSettings = graphicsSettings["Contours"];
			if (attributesSettings["IsoscalarField"].isString())
				contours.setIsoscalarField(getFieldByName(graphics,
					attributesSettings["IsoscalarField"].asCString()));
			if (attributesSettings["ListIsovalues"].isArray())
			{
				unsigned int num = attributesSettings["ListIsovalues"].size();
				if (num > 0)
				{
					double *values = new double[num];
					for (unsigned int i = 0; i < num; i++)
					{
						values[i] = attributesSettings["ListIsovalues"][i].asDouble();
					}
					contours.setListIsovalues(num, values);
					delete[] values;
				}
			}
			else if (attributesSettings["RangeFirstIsovalue"].isDouble() &&
				attributesSettings["RangeLastIsovalue"].isDouble() &&
				attributesSettings["RangeNumberOfIsovalues"].isInt())
			{
				contours.setRangeIsovalues(
					attributesSettings["RangeNumberOfIsovalues"].asInt(),
					attributesSettings["RangeFirstIsovalue"].asDouble(),
					attributesSettings["RangeLastIsovalue"].asDouble());
			}
		}
	}
}

void GraphicsJsonIO::ioLinesEntries(Json::Value &graphicsSettings)
{
	if (mode == IO_MODE_EXPORT)
	{
		OpenCMISS::Zinc::GraphicsLines lines = graphics.castLines();
		if (lines.isValid())
		{
			graphicsSettings["Lines"] = Json::Value(Json::objectValue);
		}
	}
}

void GraphicsJsonIO::ioPointsEntries(Json::Value &graphicsSettings)
{
	if (mode == IO_MODE_EXPORT)
	{
		OpenCMISS::Zinc::GraphicsPoints points = graphics.castPoints();
		if (points.isValid())
		{
			graphicsSettings["Points"] = Json::Value(Json::objectValue);
		}
	}
}

void GraphicsJsonIO::ioStreamlinesEntries(Json::Value &graphicsSettings)
{
	OpenCMISS::Zinc::GraphicsStreamlines streamlines = graphics.castStreamlines();
	if (streamlines.isValid())
	{
		if (mode == IO_MODE_EXPORT)
		{
			Json::Value attributesSettings;
			OpenCMISS::Zinc::Field field = streamlines.getStreamVectorField();
			char *name = 0;
			if (field.isValid())
			{
				name = field.getName();
				attributesSettings["StreamVectorField"] = name;
				DEALLOCATE(name);
			}
			char *enumString = streamlines.TrackDirectionEnumToString(streamlines.getTrackDirection());
			if (enumString)
			{
				attributesSettings["TrackDirection"] = enumString;
				DEALLOCATE(enumString);
			}
			enumString = streamlines.ColourDataTypeEnumToString(streamlines.getColourDataType());
			if (enumString)
			{
				attributesSettings["ColourDataType"] = enumString;
				DEALLOCATE(enumString);
			}
			double value = streamlines.getTrackLength();
			attributesSettings["TrackLength"] = value;
			graphicsSettings["Streamlines"] = attributesSettings;
		}
		else if (graphicsSettings["Streamlines"].isObject())
		{
			Json::Value attributesSettings = graphicsSettings["Streamlines"];
			if (attributesSettings["StreamVectorField"].isString())
				streamlines.setStreamVectorField(getFieldByName(graphics,
					attributesSettings["StreamVectorField"].asCString()));
			if (attributesSettings["TrackDirection"].isString())
				streamlines.setTrackDirection(streamlines.TrackDirectionEnumFromString(
					attributesSettings["TrackDirection"].asCString()));
			if (attributesSettings["ColourDataType"].isString())
				streamlines.setColourDataType(streamlines.ColourDataTypeEnumFromString(
					attributesSettings["ColourDataType"].asCString()));
			if (attributesSettings["TrackLength"].isDouble())
				streamlines.setTrackLength(attributesSettings["TrackLength"].asDouble());
		}
	}
}

void GraphicsJsonIO::ioSurfacesEntries(Json::Value &graphicsSettings)
{
	if (mode == IO_MODE_EXPORT)
	{
		OpenCMISS::Zinc::GraphicsSurfaces surfaces = graphics.castSurfaces();
		if (surfaces.isValid())
		{
			graphicsSettings["Surfaces"] = Json::Value(Json::objectValue);
		}
	}
}

void GraphicsJsonIO::ioTypeEntries(Json::Value &graphicsSettings)
{
	ioContoursEntries(graphicsSettings);
	ioLinesEntries(graphicsSettings);
	ioPointsEntries(graphicsSettings);
	ioStreamlinesEntries(graphicsSettings);
	ioSurfacesEntries(graphicsSettings);
}
