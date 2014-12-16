/***************************************************************************//**
 * FILE : graphics_json_export.cpp
 *
 * The definition to graphics_json_export.
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

//#include "zinc/spectrum.h"
//#include "zinc/spectrum.hpp"
#include "general/debug.h"
#include "graphics/graphics_json_export.hpp"
#include "graphics/graphics.h"

std::string GraphicsJsonExport::getExportString()
{
	std::string returned_string;
	if (graphics.isValid())
	{
		this->addEntries();

		returned_string = Json::StyledWriter().write(root);
	}

	return returned_string;
}

void GraphicsJsonExport::addGeneralFieldEntries(Json::Value &graphicsSettings)
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

void GraphicsJsonExport::addGeneralBoolEntries(Json::Value &graphicsSettings)
{
	bool value = graphics.getVisibilityFlag();
	graphicsSettings["VisibilityFlag"] = value;

	value = graphics.isExterior();
	graphicsSettings["isExterior"] = value;

}

void GraphicsJsonExport::addGeneralObjectEntries(Json::Value &graphicsSettings)
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
//	OpenCMISS::Spectrum spectrum = graphics.getSpectrum();
//	if (spectrum.isValid())
//	{
//		name = spectrum.getName();
//		graphicsSettings["Spectrum"] = name;
//		DEALLOCATE(name);
//	}
	OpenCMISS::Zinc::Tessellation tessellation = graphics.getTessellation();
	if (tessellation.isValid())
	{
		name = tessellation.getName();
		graphicsSettings["Tessellation"] = name;
		DEALLOCATE(name);
	}

}

void GraphicsJsonExport::addGeneralDobuleEntries(Json::Value &graphicsSettings)
{
	double value = graphics.getRenderLineWidth();
	graphicsSettings["RenderLineWidth"] = value;

	value = graphics.getRenderPointSize();
	graphicsSettings["RenderPointSize"] = value;
}

void GraphicsJsonExport::addGeneralEnumEntries(Json::Value &graphicsSettings)
{
	int enumInt = (int)graphics.getRenderPolygonMode();
	graphicsSettings["RenderPolygonMode"] = enumInt;

	enumInt = (int)graphics.getSelectMode();
	graphicsSettings["SelectMode"] = enumInt;

	enumInt = (int)graphics.getScenecoordinatesystem();
	graphicsSettings["Scenecoordinatesystem"] = enumInt;

	enumInt = (int)graphics.getFieldDomainType();
	graphicsSettings["FieldDomainType"] = enumInt;

	enumInt = (int)graphics.getElementFaceType();
	graphicsSettings["ElementFaceType"] = enumInt;

}

void GraphicsJsonExport::addGeneralEntries(Json::Value &graphicsSettings)
{
	addGeneralFieldEntries(graphicsSettings);
	addGeneralDobuleEntries(graphicsSettings);
	addGeneralEnumEntries(graphicsSettings);
	addGeneralObjectEntries(graphicsSettings);
}

void GraphicsJsonExport::addLineAttributesEntries(Json::Value &graphicsSettings)
{
	OpenCMISS::Zinc::Graphicslineattributes lineAttributes = graphics.getGraphicslineattributes();
	if (lineAttributes.isValid())
	{
		Json::Value attributesSettings;
		double values[3];
		lineAttributes.getBaseSize(3, &(values[0]));
		attributesSettings["BaseSize"].append(values[0]);
		attributesSettings["BaseSize"].append(values[1]);
		attributesSettings["BaseSize"].append(values[2]);
		char *name = 0;
		OpenCMISS::Zinc::Field field = lineAttributes.getOrientationScaleField();
		if (field.isValid())
		{
			name = field.getName();
			attributesSettings["OrientationScaleField"] = name;
			DEALLOCATE(name);
		}
		lineAttributes.getScaleFactors(3, &(values[0]));
		attributesSettings["ScaleFactors"].append(values[0]);
		attributesSettings["ScaleFactors"].append(values[1]);
		attributesSettings["ScaleFactors"].append(values[2]);
		int enumType = (int)lineAttributes.getShapeType();
		attributesSettings["ShapeType"] = enumType;
		graphicsSettings["LineAttributes"] = attributesSettings;
	}
}


void GraphicsJsonExport::addPointAttributesEntries(Json::Value &graphicsSettings)
{
	OpenCMISS::Zinc::Graphicspointattributes pointAttributes = graphics.getGraphicspointattributes();
	if (pointAttributes.isValid())
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
		int enumType = (int)pointAttributes.getGlyphRepeatMode();
		attributesSettings["GlyphRepeatMode"] = enumType;
		enumType = (int)pointAttributes.getGlyphShapeType();
		attributesSettings["GlyphShapeType"] = enumType;
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
		name = pointAttributes.getLabelText(1);
		if (name)
		{
			attributesSettings["LabelText"].append(name);
			DEALLOCATE(name);
		}
		else
		{
			attributesSettings["LabelText"].append("");
		}
		name = pointAttributes.getLabelText(2);
		if (name)
		{
			attributesSettings["LabelText"].append(name);
			DEALLOCATE(name);
		}
		else
		{
			attributesSettings["LabelText"].append("");
		}
		name = pointAttributes.getLabelText(3);
		if (name)
		{
			attributesSettings["LabelText"].append(name);
			DEALLOCATE(name);
		}
		else
		{
			attributesSettings["LabelText"].append("");
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
}

void GraphicsJsonExport::addSamplingAttributesEntries(Json::Value &graphicsSettings)
{
	OpenCMISS::Zinc::Graphicssamplingattributes samplingAttributes = graphics.getGraphicssamplingattributes();
	if (samplingAttributes.isValid())
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
		int enumType = (int)samplingAttributes.getElementPointSamplingMode();
		attributesSettings["ElementPointSamplingMode"] = enumType;
		graphicsSettings["SamplingAttributes"] = attributesSettings;
	}
}

void GraphicsJsonExport::addAttributesEntries(Json::Value &graphicsSettings)
{
	addLineAttributesEntries(graphicsSettings);
	addPointAttributesEntries(graphicsSettings);
	addSamplingAttributesEntries(graphicsSettings);
}

void GraphicsJsonExport::addContoursEntries(Json::Value &graphicsSettings)
{
	OpenCMISS::Zinc::GraphicsContours contours = graphics.castContours();
	if (contours.isValid())
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
		temp = contours.getRangeFirstIsovalue();
		attributesSettings["RangeFirstIsovalue"] = temp;
		temp = contours.getRangeLastIsovalue();
		attributesSettings["RangeLastIsovalue"] = temp;
		num = contours.getRangeNumberOfIsovalues();
		attributesSettings["RangeNumberOfIsovalues"] = num;
		graphicsSettings["Contours"] = attributesSettings;
	}
}

void GraphicsJsonExport::addLinesEntries(Json::Value &graphicsSettings)
{
	OpenCMISS::Zinc::GraphicsLines lines = graphics.castLines();
	if (lines.isValid())
	{
		graphicsSettings["Lines"] = Json::Value(Json::objectValue);
	}
}

void GraphicsJsonExport::addPointsEntries(Json::Value &graphicsSettings)
{
	OpenCMISS::Zinc::GraphicsPoints points = graphics.castPoints();
	if (points.isValid())
	{
		graphicsSettings["Points"] = Json::Value(Json::objectValue);
	}
}

void GraphicsJsonExport::addStreamlinesEntries(Json::Value &graphicsSettings)
{
	OpenCMISS::Zinc::GraphicsStreamlines streamlines = graphics.castStreamlines();
	if (streamlines.isValid())
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
		int enumType = (int)streamlines.getTrackDirection();
		attributesSettings["TrackDirection"] = enumType;
		double value = streamlines.getTrackLength();
		attributesSettings["TrackLength"] = value;
		graphicsSettings["Streamlines"] = attributesSettings;
	}
}

void GraphicsJsonExport::addSurfacesEntries(Json::Value &graphicsSettings)
{
	OpenCMISS::Zinc::GraphicsSurfaces surfaces = graphics.castSurfaces();
	if (surfaces.isValid())
	{
		graphicsSettings["Surfaces"] = Json::Value(Json::objectValue);
	}
}

void GraphicsJsonExport::addTypeEntries(Json::Value &graphicsSettings)
{
	addContoursEntries(graphicsSettings);
	addLinesEntries(graphicsSettings);
	addPointsEntries(graphicsSettings);
	addStreamlinesEntries(graphicsSettings);
	addSurfacesEntries(graphicsSettings);
}

void GraphicsJsonExport::addEntries()
{
	root.clear();
	cmzn_graphics_id c_graphics = graphics.getId();
	char *name = graphics.getName();
	Json::Value graphicsSettings;
	if (name)
	{
		graphicsSettings["id"] = name;
		DEALLOCATE(name);
	}
	enum cmzn_graphics_type type = cmzn_graphics_get_graphics_type(c_graphics);
	char *type_string = cmzn_graphics_type_enum_to_string(type);
	graphicsSettings["Type"] = type_string;
	addTypeEntries(graphicsSettings);
	addGeneralEntries(graphicsSettings);
	addAttributesEntries(graphicsSettings);
	if (order > 0)
	{
		char order_string[5];
		sprintf(order_string, "%d", order);
		root[order_string] = graphicsSettings;
	}
	else
	{
		root = graphicsSettings;
	}

	if (type_string)
		DEALLOCATE(type_string);
}

Json::Value *GraphicsJsonExport::exportJsonValue()
{
	if (graphics.isValid())
	{
		this->addEntries();

		return &root;
	}

	return 0;
}
