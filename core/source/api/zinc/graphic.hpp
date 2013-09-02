/***************************************************************************//**
 * FILE : graphic.hpp
 */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is libZinc.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2012
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#ifndef CMZN_GRAPHIC_HPP__
#define CMZN_GRAPHIC_HPP__

#include "zinc/types/scenecoordinatesystem.hpp"
#include "zinc/graphic.h"
#include "zinc/element.hpp"
#include "zinc/field.hpp"
#include "zinc/glyph.hpp"
#include "zinc/font.hpp"
#include "zinc/graphicsmaterial.hpp"
#include "zinc/spectrum.hpp"
#include "zinc/tessellation.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class GraphicLineAttributes;
class GraphicPointAttributes;
class GraphicSamplingAttributes;

class Graphic
{

protected:
	cmzn_graphic_id id;

public:

	Graphic() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Graphic(cmzn_graphic_id graphic_id) : id(graphic_id)
	{  }

	Graphic(const Graphic& graphic) : id(cmzn_graphic_access(graphic.id))
	{  }

	Graphic& operator=(const Graphic& graphic)
	{
		cmzn_graphic_id temp_id = cmzn_graphic_access(graphic.id);
		if (0 != id)
		{
			cmzn_graphic_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Graphic()
	{
		if (0 != id)
		{
			cmzn_graphic_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	enum RenderPolygonMode
	{
		RENDER_POLYGON_MODE_INVALID = CMZN_GRAPHIC_RENDER_POLYGON_MODE_INVALID,
		RENDER_POLYGON_SHADED = CMZN_GRAPHIC_RENDER_POLYGON_SHADED,
		RENDER_POLYGON_WIREFRAME = CMZN_GRAPHIC_RENDER_POLYGON_WIREFRAME
	};

	enum SelectMode
	{
		SELECT_MODE_INVALID = CMZN_GRAPHIC_SELECT_MODE_INVALID,
		SELECT_ON = CMZN_GRAPHIC_SELECT_ON,
		NO_SELECT = CMZN_GRAPHIC_NO_SELECT,
		DRAW_SELECTED = CMZN_GRAPHIC_DRAW_SELECTED,
		DRAW_UNSELECTED = CMZN_GRAPHIC_DRAW_UNSELECTED
	};

	enum Type
	{
		TYPE_INVALID = CMZN_GRAPHIC_TYPE_INVALID,
		POINTS = CMZN_GRAPHIC_POINTS,
		LINES = CMZN_GRAPHIC_LINES,
		SURFACES = CMZN_GRAPHIC_SURFACES,
		CONTOURS = CMZN_GRAPHIC_CONTOURS,
		STREAMLINES = CMZN_GRAPHIC_STREAMLINES
	};

	cmzn_graphic_id getId()
	{
		return id;
	}

	Field getCoordinateField()
	{
		return Field(cmzn_graphic_get_coordinate_field(id));
	}

	int setCoordinateField(Field& coordinateField)
	{
		return cmzn_graphic_set_coordinate_field(id, coordinateField.getId());
	}

	Field getDataField()
	{
		return Field(cmzn_graphic_get_data_field(id));
	}

	int setDataField(Field& dataField)
	{
		return cmzn_graphic_set_data_field(id, dataField.getId());
	}

	double getRenderLineWidth()
	{
		return cmzn_graphic_get_render_line_width(id);
	}

	int setRenderLineWidth(double width)
	{
		return cmzn_graphic_set_render_line_width(id, width);
	}

	double getRenderPointSize()
	{
		return cmzn_graphic_get_render_point_size(id);
	}

	int setRenderPointSize(double size)
	{
		return cmzn_graphic_set_render_point_size(id, size);
	}

	enum RenderPolygonMode getRenderPolygonMode()
	{
		return static_cast<RenderPolygonMode>(cmzn_graphic_get_render_polygon_mode(id));
	}

	int setRenderPolygonMode(RenderPolygonMode renderPolygonMode)
	{
		return cmzn_graphic_set_render_polygon_mode(id,
			static_cast<cmzn_graphic_render_polygon_mode>(renderPolygonMode));
	}

	enum SelectMode getSelectMode()
	{
		return static_cast<SelectMode>(cmzn_graphic_get_select_mode(id));
	}

	int setSelectMode(SelectMode selectMode)
	{
		return cmzn_graphic_set_select_mode(id, static_cast<cmzn_graphic_select_mode>(selectMode));
	}

	Field getSubgroupField()
	{
		return Field(cmzn_graphic_get_subgroup_field(id));
	}

	int setSubgroupField(Field& subgroupField)
	{
		return cmzn_graphic_set_subgroup_field(id, subgroupField.getId());
	}

	Field getTextureCoordinateField()
	{
		return Field(cmzn_graphic_get_texture_coordinate_field(id));
	}

	int setTextureCoordinateField(Field& textureCoordinateField)
	{
		return cmzn_graphic_set_texture_coordinate_field(id, textureCoordinateField.getId());
	}

	GraphicsMaterial getMaterial()
	{
		return GraphicsMaterial(cmzn_graphic_get_material(id));
	}

	int setMaterial(GraphicsMaterial& graphicsMaterial)
	{
		return cmzn_graphic_set_material(id, graphicsMaterial.getId());
	}

	GraphicLineAttributes getLineAttributes();

	GraphicPointAttributes getPointAttributes();

	GraphicSamplingAttributes getSamplingAttributes();

	GraphicsMaterial getSelectedMaterial()
	{
		return GraphicsMaterial(cmzn_graphic_get_selected_material(id));
	}

	int setSelectedMaterial(GraphicsMaterial& graphicsMaterial)
	{
		return cmzn_graphic_set_selected_material(id, graphicsMaterial.getId());
	}

	Spectrum getSpectrum()
	{
		return Spectrum(cmzn_graphic_get_spectrum(id));
	}

	int setSpectrum(Spectrum& spectrum)
	{
		return cmzn_graphic_set_spectrum(id, spectrum.getId());
	}

	Tessellation getTessellation()
	{
		return Tessellation(cmzn_graphic_get_tessellation(id));
	}

	int setTessellation(Tessellation& tessellation)
	{
		return cmzn_graphic_set_tessellation(id, tessellation.getId());
	}

	Field getTessellationField()
	{
		return Field(cmzn_graphic_get_tessellation_field(id));
	}

	int setTessellationField(Field& tessellationField)
	{
		return cmzn_graphic_set_tessellation_field(id, tessellationField.getId());
	}

	bool getVisibilityFlag()
	{
		return cmzn_graphic_get_visibility_flag(id);
	}

	int setVisibilityFlag(bool visibilityFlag)
	{
		return cmzn_graphic_set_visibility_flag(id, visibilityFlag);
	}

	enum SceneCoordinateSystem getCoordinateSystem()
	{
		return static_cast<SceneCoordinateSystem>(cmzn_graphic_get_coordinate_system(id));
	}

	int setCoordinateSystem(SceneCoordinateSystem coordinateSystem)
	{
		return cmzn_graphic_set_coordinate_system(id,
			static_cast<cmzn_scene_coordinate_system>(coordinateSystem));
	}

	Field::DomainType getDomainType()
	{
		return static_cast<Field::DomainType>(cmzn_graphic_get_domain_type(id));
	}

	int setDomainType(Field::DomainType domainType)
	{
		return cmzn_graphic_set_domain_type(id, static_cast<cmzn_field_domain_type>(domainType));
	}

	char *getName()
	{
		return cmzn_graphic_get_name(id);
	}

	int setName(const char *name)
	{
		return cmzn_graphic_set_name(id, name);
	}

	int setFace(Element::FaceType face)
	{
		return cmzn_graphic_set_face(id, static_cast<cmzn_element_face_type>(face));
	}

	Element::FaceType getFace()
	{
		return static_cast<Element::FaceType>(cmzn_graphic_get_face(id));
	}

	bool isExterior()
	{
		return cmzn_graphic_is_exterior(id);
	}

	int setExterior(bool exterior)
	{
		return cmzn_graphic_set_exterior(id, exterior);
	}
};

class GraphicContours : public Graphic
{
private:
	explicit GraphicContours(cmzn_graphic_id graphic_id) : Graphic(graphic_id) {}

public:
	GraphicContours() : Graphic(0) {}

	explicit GraphicContours(cmzn_graphic_contours_id graphic_contours_id)
		: Graphic(reinterpret_cast<cmzn_graphic_id>(graphic_contours_id))
	{}

	GraphicContours(Graphic& graphic)
		: Graphic(reinterpret_cast<cmzn_graphic_id>(cmzn_graphic_cast_contours(graphic.getId())))
	{}

	Field getIsoscalarField()
	{
		return Field(cmzn_graphic_contours_get_isoscalar_field(reinterpret_cast<cmzn_graphic_contours_id>(id)));
	}

	int setIsoscalarField(Field& field)
	{
		return cmzn_graphic_contours_set_isoscalar_field(reinterpret_cast<cmzn_graphic_contours_id>(id), field.getId());
	}

	int getListIsovalues(int valuesCount, double *valuesOut)
	{
		return cmzn_graphic_contours_get_list_isovalues(reinterpret_cast<cmzn_graphic_contours_id>(id),
			valuesCount, valuesOut);
	}

	int setListIsovalues(int valuesCount, const double *valuesIn)
	{
		return cmzn_graphic_contours_set_list_isovalues(reinterpret_cast<cmzn_graphic_contours_id>(id),
			valuesCount, valuesIn);
	}

	double getRangeFirstIsovalue()
	{
		return cmzn_graphic_contours_get_range_first_isovalue(
			reinterpret_cast<cmzn_graphic_contours_id>(id));
	}

	double getRangeLastIsovalue()
	{
		return cmzn_graphic_contours_get_range_last_isovalue(
			reinterpret_cast<cmzn_graphic_contours_id>(id));
	}

	int getRangeNumberOfIsovalues()
	{
		return cmzn_graphic_contours_get_range_number_of_isovalues(
			reinterpret_cast<cmzn_graphic_contours_id>(id));
	}

	int setRangeIsovalues(int numberOfValues, double firstIsovalue, double lastIsovalue)
	{
		return cmzn_graphic_contours_set_range_isovalues(reinterpret_cast<cmzn_graphic_contours_id>(id),
			numberOfValues, firstIsovalue, lastIsovalue);
	}

};

class GraphicLines : public Graphic
{
private:
	explicit GraphicLines(cmzn_graphic_id graphic_id) : Graphic(graphic_id) {}

public:
	GraphicLines() : Graphic(0) {}

	explicit GraphicLines(cmzn_graphic_lines_id graphic_lines_id)
		: Graphic(reinterpret_cast<cmzn_graphic_id>(graphic_lines_id))
	{}

	GraphicLines(Graphic& graphic)
		: Graphic(reinterpret_cast<cmzn_graphic_id>(cmzn_graphic_cast_lines(graphic.getId())))
	{}
};

class GraphicPoints : public Graphic
{
private:
	explicit GraphicPoints(cmzn_graphic_id graphic_id) : Graphic(graphic_id) {}

public:
	GraphicPoints() : Graphic(0) {}

	explicit GraphicPoints(cmzn_graphic_points_id graphic_points_id)
		: Graphic(reinterpret_cast<cmzn_graphic_id>(graphic_points_id))
	{}

	GraphicPoints(Graphic& graphic)
		: Graphic(reinterpret_cast<cmzn_graphic_id>(cmzn_graphic_cast_points(graphic.getId())))
	{}
};

class GraphicStreamlines : public Graphic
{
private:
	explicit GraphicStreamlines(cmzn_graphic_id graphic_id) : Graphic(graphic_id) {}

public:
	GraphicStreamlines() : Graphic(0) {}

	explicit GraphicStreamlines(cmzn_graphic_streamlines_id graphic_streamlines_id)
		: Graphic(reinterpret_cast<cmzn_graphic_id>(graphic_streamlines_id))
	{}

	GraphicStreamlines(Graphic& graphic)
		: Graphic(reinterpret_cast<cmzn_graphic_id>(cmzn_graphic_cast_streamlines(graphic.getId())))
	{}

	enum TrackDirection
	{
		TRACK_DIRECTION_INVALID = CMZN_GRAPHIC_STREAMLINES_TRACK_DIRECTION_INVALID,
		FORWARD_TRACK = CMZN_GRAPHIC_STREAMLINES_FORWARD_TRACK,
		REVERSE_TRACK = CMZN_GRAPHIC_STREAMLINES_REVERSE_TRACK
	};

	Field getStreamVectorField()
	{
		return Field(cmzn_graphic_streamlines_get_stream_vector_field(reinterpret_cast<cmzn_graphic_streamlines_id>(id)));
	}

	int setStreamVectorField(Field& field)
	{
		return cmzn_graphic_streamlines_set_stream_vector_field(reinterpret_cast<cmzn_graphic_streamlines_id>(id), field.getId());
	}

	TrackDirection getTrackDirection()
	{
		return static_cast<TrackDirection>(
			cmzn_graphic_streamlines_get_track_direction(reinterpret_cast<cmzn_graphic_streamlines_id>(id)));
	}

	int setTrackDirection(TrackDirection trackDirection)
	{
		return cmzn_graphic_streamlines_set_track_direction(reinterpret_cast<cmzn_graphic_streamlines_id>(id),
			static_cast<cmzn_graphic_streamlines_track_direction>(trackDirection));
	}

	double getTrackLength()
	{
		return cmzn_graphic_streamlines_get_track_length(reinterpret_cast<cmzn_graphic_streamlines_id>(id));
	}

	int setTrackLength(double length)
	{
		return cmzn_graphic_streamlines_set_track_length(reinterpret_cast<cmzn_graphic_streamlines_id>(id), length);
	}

};

class GraphicSurfaces : public Graphic
{
private:
	explicit GraphicSurfaces(cmzn_graphic_id graphic_id) : Graphic(graphic_id) {}

public:
	GraphicSurfaces() : Graphic(0) {}

	explicit GraphicSurfaces(cmzn_graphic_surfaces_id graphic_surfaces_id)
		: Graphic(reinterpret_cast<cmzn_graphic_id>(graphic_surfaces_id))
	{}

	GraphicSurfaces(Graphic& graphic)
		: Graphic(reinterpret_cast<cmzn_graphic_id>(cmzn_graphic_cast_surfaces(graphic.getId())))
	{}
};

class GraphicLineAttributes
{
protected:
	cmzn_graphic_line_attributes_id id;

public:

	// takes ownership of C handle, responsibility for destroying it
	explicit GraphicLineAttributes(cmzn_graphic_line_attributes_id line_attributes_id) :
		id(line_attributes_id)
	{}

	GraphicLineAttributes(const GraphicLineAttributes& lineAttributes) :
		id(cmzn_graphic_line_attributes_access(lineAttributes.id))
	{}

	~GraphicLineAttributes()
	{
		cmzn_graphic_line_attributes_destroy(&id);
	}

	bool isValid()
	{
		return (0 != id);
	}

	enum Shape
	{
		SHAPE_INVALID = CMZN_GRAPHIC_LINE_ATTRIBUTES_SHAPE_INVALID,
		SHAPE_LINE = CMZN_GRAPHIC_LINE_ATTRIBUTES_SHAPE_LINE,
		SHAPE_RIBBON = CMZN_GRAPHIC_LINE_ATTRIBUTES_SHAPE_RIBBON,
		SHAPE_CIRCLE_EXTRUSION = CMZN_GRAPHIC_LINE_ATTRIBUTES_SHAPE_CIRCLE_EXTRUSION,
		SHAPE_SQUARE_EXTRUSION = CMZN_GRAPHIC_LINE_ATTRIBUTES_SHAPE_SQUARE_EXTRUSION
	};

	int getBaseSize(int valuesCount, double *valuesOut)
	{
		return cmzn_graphic_line_attributes_get_base_size(id, valuesCount, valuesOut);
	}

	int setBaseSize(int valuesCount, const double *valuesIn)
	{
		return cmzn_graphic_line_attributes_set_base_size(id, valuesCount, valuesIn);
	}

	Field getOrientationScaleField()
	{
		return Field(cmzn_graphic_line_attributes_get_orientation_scale_field(id));
	}

	int setOrientationScaleField(Field& orientationScaleField)
	{
		return cmzn_graphic_line_attributes_set_orientation_scale_field(id, orientationScaleField.getId());
	}

	int getScaleFactors(int valuesCount, double *valuesOut)
	{
		return cmzn_graphic_line_attributes_get_scale_factors(id, valuesCount, valuesOut);
	}

	int setScaleFactors(int valuesCount, const double *valuesIn)
	{
		return cmzn_graphic_line_attributes_set_scale_factors(id, valuesCount, valuesIn);
	}

	Shape getShape()
	{
		return static_cast<Shape>(cmzn_graphic_line_attributes_get_shape(id));
	}

	int setShape(Shape shape)
	{
		return cmzn_graphic_line_attributes_set_shape(id, static_cast<cmzn_graphic_line_attributes_shape>(shape));
	}

};

inline GraphicLineAttributes Graphic::getLineAttributes()
{
	return GraphicLineAttributes(cmzn_graphic_get_line_attributes(id));
}

class GraphicPointAttributes
{
protected:
	cmzn_graphic_point_attributes_id id;

public:

	// takes ownership of C handle, responsibility for destroying it
	explicit GraphicPointAttributes(cmzn_graphic_point_attributes_id point_attributes_id) :
		id(point_attributes_id)
	  {}

	GraphicPointAttributes(const GraphicPointAttributes& pointAttributes) :
		id(cmzn_graphic_point_attributes_access(pointAttributes.id))
		{}

	~GraphicPointAttributes()
	{
		cmzn_graphic_point_attributes_destroy(&id);
	}

	bool isValid()
	{
		return (0 != id);
	}

	int getBaseSize(int valuesCount, double *valuesOut)
	{
		return cmzn_graphic_point_attributes_get_base_size(id, valuesCount, valuesOut);
	}

	int setBaseSize(int valuesCount, const double *valuesIn)
	{
		return cmzn_graphic_point_attributes_set_base_size(id, valuesCount, valuesIn);
	}

	Font getFont()
	{
		return Font(cmzn_graphic_point_attributes_get_font(id));
	}

	int setFont(Font& font)
	{
		return cmzn_graphic_point_attributes_set_font(id, font.getId());
	}

	Glyph getGlyph()
	{
		return Glyph(cmzn_graphic_point_attributes_get_glyph(id));
	}

	int setGlyph(Glyph& glyph)
	{
		return cmzn_graphic_point_attributes_set_glyph(id, glyph.getId());
	}

	int getGlyphOffset(int valuesCount, double *valuesOut)
	{
		return cmzn_graphic_point_attributes_get_glyph_offset(id, valuesCount, valuesOut);
	}

	int setGlyphOffset(int valuesCount, const double *valuesIn)
	{
		return cmzn_graphic_point_attributes_set_glyph_offset(id, valuesCount, valuesIn);
	}

	Glyph::RepeatMode getGlyphRepeatMode()
	{
		return static_cast<Glyph::RepeatMode>(cmzn_graphic_point_attributes_get_glyph_repeat_mode(id));
	}

	int setGlyphRepeatMode(Glyph::RepeatMode glyphRepeatMode)
	{
		return cmzn_graphic_point_attributes_set_glyph_repeat_mode(id,
			static_cast<enum cmzn_glyph_repeat_mode>(glyphRepeatMode));
	}

	Glyph::Type getGlyphType()
	{
		return static_cast<Glyph::Type>(cmzn_graphic_point_attributes_get_glyph_type(id));
	}

	int setGlyphType(Glyph::Type type)
	{
		return cmzn_graphic_point_attributes_set_glyph_type(id,
			static_cast<cmzn_glyph_type>(type));
	}

	Field getLabelField()
	{
		return Field(cmzn_graphic_point_attributes_get_label_field(id));
	}

	int setLabelField(Field& labelField)
	{
		return cmzn_graphic_point_attributes_set_label_field(id, labelField.getId());
	}

	int getLabelOffset(int valuesCount, double *valuesOut)
	{
		return cmzn_graphic_point_attributes_get_label_offset(id, valuesCount, valuesOut);
	}

	int setLabelOffset(int valuesCount, const double *valuesIn)
	{
		return cmzn_graphic_point_attributes_set_label_offset(id, valuesCount, valuesIn);
	}

	char *getLabelText(int labelNumber)
	{
		return cmzn_graphic_point_attributes_get_label_text(id, labelNumber);
	}

	int setLabelText(int labelNumber, const char *labelText)
	{
		return cmzn_graphic_point_attributes_set_label_text(id, labelNumber, labelText);
	}

	Field getOrientationScaleField()
	{
		return Field(cmzn_graphic_point_attributes_get_orientation_scale_field(id));
	}

	int setOrientationScaleField(Field& orientationScaleField)
	{
		return cmzn_graphic_point_attributes_set_orientation_scale_field(id, orientationScaleField.getId());
	}

	int getScaleFactors(int valuesCount, double *valuesOut)
	{
		return cmzn_graphic_point_attributes_get_scale_factors(id, valuesCount, valuesOut);
	}

	int setScaleFactors(int valuesCount, const double *valuesIn)
	{
		return cmzn_graphic_point_attributes_set_scale_factors(id, valuesCount, valuesIn);
	}

	Field getSignedScaleField()
	{
		return Field(cmzn_graphic_point_attributes_get_signed_scale_field(id));
	}

	int setSignedScaleField(Field& signedScaleField)
	{
		return cmzn_graphic_point_attributes_set_signed_scale_field(id, signedScaleField.getId());
	}

};

inline GraphicPointAttributes Graphic::getPointAttributes()
{
	return GraphicPointAttributes(cmzn_graphic_get_point_attributes(id));
}

class GraphicSamplingAttributes
{
protected:
	cmzn_graphic_sampling_attributes_id id;

public:

	// takes ownership of C handle, responsibility for destroying it
	explicit GraphicSamplingAttributes(cmzn_graphic_sampling_attributes_id sampling_attributes_id) :
		id(sampling_attributes_id)
	  {}

	GraphicSamplingAttributes(const GraphicSamplingAttributes& samplingAttributes) :
		id(cmzn_graphic_sampling_attributes_access(samplingAttributes.id))
		{}

	~GraphicSamplingAttributes()
	{
		cmzn_graphic_sampling_attributes_destroy(&id);
	}

	bool isValid()
	{
		return (0 != id);
	}

	Field getDensityField()
	{
		return Field(cmzn_graphic_sampling_attributes_get_density_field(id));
	}

	int setDensityField(Field& densityField)
	{
		return cmzn_graphic_sampling_attributes_set_density_field(id, densityField.getId());
	}

	int getLocation(int valuesCount, double *valuesOut)
	{
		return cmzn_graphic_sampling_attributes_get_location(id, valuesCount, valuesOut);
	}

	int setLocation(int valuesCount, const double *valuesIn)
	{
		return cmzn_graphic_sampling_attributes_set_location(id, valuesCount, valuesIn);
	}

	Element::PointSampleMode getMode()
	{
		return static_cast<Element::PointSampleMode>(cmzn_graphic_sampling_attributes_get_mode(id));
	}

	int setMode(Element::PointSampleMode sampleMode)
	{
		return cmzn_graphic_sampling_attributes_set_mode(id,
			static_cast<cmzn_element_point_sample_mode>(sampleMode));
	}

};

inline GraphicSamplingAttributes Graphic::getSamplingAttributes()
{
	return GraphicSamplingAttributes(cmzn_graphic_get_sampling_attributes(id));
}

} // namespace Zinc
}

#endif
