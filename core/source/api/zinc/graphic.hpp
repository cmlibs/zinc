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
#ifndef __ZN_CMISS_GRAPHIC_HPP__
#define __ZN_CMISS_GRAPHIC_HPP__

#include "zinc/graphic.h"
#include "zinc/element.hpp"
#include "zinc/field.hpp"
#include "zinc/glyph.hpp"
#include "zinc/font.hpp"
#include "zinc/graphicsmaterial.hpp"
#include "zinc/spectrum.hpp"
#include "zinc/tessellation.hpp"

namespace zinc
{

class GraphicLineAttributes;
class GraphicPointAttributes;
class GraphicSamplingAttributes;

class Graphic
{

protected:
	Cmiss_graphic_id id;

public:

	Graphic() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Graphic(Cmiss_graphic_id graphic_id) : id(graphic_id)
	{  }

	Graphic(const Graphic& graphic) : id(Cmiss_graphic_access(graphic.id))
	{  }

	Graphic& operator=(const Graphic& graphic)
	{
		Cmiss_graphic_id temp_id = Cmiss_graphic_access(graphic.id);
		if (0 != id)
		{
			Cmiss_graphic_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Graphic()
	{
		if (0 != id)
		{
			Cmiss_graphic_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	enum RenderPolygonMode
	{
		RENDER_POLYGON_MODE_INVALID = CMISS_GRAPHIC_RENDER_POLYGON_MODE_INVALID,
		RENDER_POLYGON_SHADED = CMISS_GRAPHIC_RENDER_POLYGON_SHADED,
		RENDER_POLYGON_WIREFRAME = CMISS_GRAPHIC_RENDER_POLYGON_WIREFRAME
	};

	enum SelectMode
	{
		SELECT_MODE_INVALID = CMISS_GRAPHIC_SELECT_MODE_INVALID,
		SELECT_ON = CMISS_GRAPHIC_SELECT_ON,
		NO_SELECT = CMISS_GRAPHIC_NO_SELECT,
		DRAW_SELECTED = CMISS_GRAPHIC_DRAW_SELECTED,
		DRAW_UNSELECTED = CMISS_GRAPHIC_DRAW_UNSELECTED
	};

	enum CoordinateSystem
	{
		COORDINATE_SYSTEM_INVALID = CMISS_GRAPHICS_COORDINATE_SYSTEM_INVALID,
		COORDINATE_SYSTEM_LOCAL = CMISS_GRAPHICS_COORDINATE_SYSTEM_LOCAL,
		COORDINATE_SYSTEM_WORLD = CMISS_GRAPHICS_COORDINATE_SYSTEM_WORLD,
		COORDINATE_SYSTEM_NORMALISED_WINDOW_FILL = CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FILL,
		COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_CENTRE = CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_CENTRE,
		COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_LEFT = CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_LEFT,
		COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_RIGHT = CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_RIGHT,
		COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_BOTTOM = CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_BOTTOM,
		COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_TOP = CMISS_GRAPHICS_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_TOP,
		COORDINATE_SYSTEM_WINDOW_PIXEL_BOTTOM_LEFT = CMISS_GRAPHICS_COORDINATE_SYSTEM_WINDOW_PIXEL_BOTTOM_LEFT,
		COORDINATE_SYSTEM_WINDOW_PIXEL_TOP_LEFT = CMISS_GRAPHICS_COORDINATE_SYSTEM_WINDOW_PIXEL_TOP_LEFT
	};

	enum GraphicType
	{
		GRAPHIC_TYPE_INVALID = CMISS_GRAPHIC_TYPE_INVALID,
		GRAPHIC_POINTS = CMISS_GRAPHIC_POINTS,
		GRAPHIC_LINES = CMISS_GRAPHIC_LINES,
		GRAPHIC_SURFACES = CMISS_GRAPHIC_SURFACES,
		GRAPHIC_CONTOURS = CMISS_GRAPHIC_CONTOURS,
		GRAPHIC_STREAMLINES = CMISS_GRAPHIC_STREAMLINES
	};

	Cmiss_graphic_id getId()
	{
		return id;
	}

	Field getCoordinateField()
	{
		return Field(Cmiss_graphic_get_coordinate_field(id));
	}

	int setCoordinateField(Field& coordinateField)
	{
		return Cmiss_graphic_set_coordinate_field(id, coordinateField.getId());
	}

	Field getDataField()
	{
		return Field(Cmiss_graphic_get_data_field(id));
	}

	int setDataField(Field& dataField)
	{
		return Cmiss_graphic_set_data_field(id, dataField.getId());
	}

	double getRenderLineWidth()
	{
		return Cmiss_graphic_get_render_line_width(id);
	}

	int setRenderLineWidth(double width)
	{
		return Cmiss_graphic_set_render_line_width(id, width);
	}

	double getRenderPointSize()
	{
		return Cmiss_graphic_get_render_point_size(id);
	}

	int setRenderPointSize(double size)
	{
		return Cmiss_graphic_set_render_point_size(id, size);
	}

	enum RenderPolygonMode getRenderPolygonMode()
	{
		return static_cast<RenderPolygonMode>(Cmiss_graphic_get_render_polygon_mode(id));
	}

	int setRenderPolygonMode(RenderPolygonMode renderPolygonMode)
	{
		return Cmiss_graphic_set_render_polygon_mode(id,
			static_cast<Cmiss_graphic_render_polygon_mode>(renderPolygonMode));
	}

	enum SelectMode getSelectMode()
	{
		return static_cast<SelectMode>(Cmiss_graphic_get_select_mode(id));
	}

	int setSelectMode(SelectMode selectMode)
	{
		return Cmiss_graphic_set_select_mode(id, static_cast<Cmiss_graphic_select_mode>(selectMode));
	}

	Field getSubgroupField()
	{
		return Field(Cmiss_graphic_get_subgroup_field(id));
	}

	int setSubgroupField(Field& subgroupField)
	{
		return Cmiss_graphic_set_subgroup_field(id, subgroupField.getId());
	}

	Field getTextureCoordinateField()
	{
		return Field(Cmiss_graphic_get_texture_coordinate_field(id));
	}

	int setTextureCoordinateField(Field& textureCoordinateField)
	{
		return Cmiss_graphic_set_texture_coordinate_field(id, textureCoordinateField.getId());
	}

	GraphicsMaterial getMaterial()
	{
		return GraphicsMaterial(Cmiss_graphic_get_material(id));
	}

	int setMaterial(GraphicsMaterial& graphicsMaterial)
	{
		return Cmiss_graphic_set_material(id, graphicsMaterial.getId());
	}

	GraphicLineAttributes getLineAttributes();

	GraphicPointAttributes getPointAttributes();

	GraphicSamplingAttributes getSamplingAttributes();

	GraphicsMaterial getSelectedMaterial()
	{
		return GraphicsMaterial(Cmiss_graphic_get_selected_material(id));
	}

	int setSelectedMaterial(GraphicsMaterial& graphicsMaterial)
	{
		return Cmiss_graphic_set_selected_material(id, graphicsMaterial.getId());
	}

	Spectrum getSpectrum()
	{
		return Spectrum(Cmiss_graphic_get_spectrum(id));
	}

	int setSpectrum(Spectrum& spectrum)
	{
		return Cmiss_graphic_set_spectrum(id, spectrum.getId());
	}

	Tessellation getTessellation()
	{
		return Tessellation(Cmiss_graphic_get_tessellation(id));
	}

	int setTessellation(Tessellation& tessellation)
	{
		return Cmiss_graphic_set_tessellation(id, tessellation.getId());
	}

	Field getTessellationField()
	{
		return Field(Cmiss_graphic_get_tessellation_field(id));
	}

	int setTessellationField(Field& tessellationField)
	{
		return Cmiss_graphic_set_tessellation_field(id, tessellationField.getId());
	}

	bool getVisibilityFlag()
	{
		return Cmiss_graphic_get_visibility_flag(id);
	}

	int setVisibilityFlag(bool visibilityFlag)
	{
		return Cmiss_graphic_set_visibility_flag(id, visibilityFlag);
	}

	enum CoordinateSystem getCoordinateSystem()
	{
		return static_cast<CoordinateSystem>(Cmiss_graphic_get_coordinate_system(id));
	}

	int setCoordinateSystem(CoordinateSystem coordinateSystem)
	{
		return Cmiss_graphic_set_coordinate_system(id,
			static_cast<Cmiss_graphics_coordinate_system>(coordinateSystem));
	}

	Field::DomainType getDomainType()
	{
		return static_cast<Field::DomainType>(Cmiss_graphic_get_domain_type(id));
	}

	int setDomainType(Field::DomainType domainType)
	{
		return Cmiss_graphic_set_domain_type(id, static_cast<Cmiss_field_domain_type>(domainType));
	}

	char *getName()
	{
		return Cmiss_graphic_get_name(id);
	}

	int setName(const char *name)
	{
		return Cmiss_graphic_set_name(id, name);
	}

	int setFace(Element::FaceType face)
	{
		return Cmiss_graphic_set_face(id, static_cast<Cmiss_element_face_type>(face));
	}

	Element::FaceType getFace()
	{
		return static_cast<Element::FaceType>(Cmiss_graphic_get_face(id));
	}

	int setExterior(int exterior)
	{
		return Cmiss_graphic_set_exterior(id, exterior);
	}

	int getExterior()
	{
		return Cmiss_graphic_get_exterior(id);
	}
};

class GraphicContours : public Graphic
{
private:
	explicit GraphicContours(Cmiss_graphic_id graphic_id) : Graphic(graphic_id) {}

public:
	GraphicContours() : Graphic(0) {}

	explicit GraphicContours(Cmiss_graphic_contours_id graphic_contours_id)
		: Graphic(reinterpret_cast<Cmiss_graphic_id>(graphic_contours_id))
	{}

	GraphicContours(Graphic& graphic)
		: Graphic(reinterpret_cast<Cmiss_graphic_id>(Cmiss_graphic_cast_contours(graphic.getId())))
	{}

	Field getIsoscalarField()
	{
		return Field(Cmiss_graphic_contours_get_isoscalar_field(reinterpret_cast<Cmiss_graphic_contours_id>(id)));
	}

	int setIsoscalarField(Field& field)
	{
		return Cmiss_graphic_contours_set_isoscalar_field(reinterpret_cast<Cmiss_graphic_contours_id>(id), field.getId());
	}

	int getListIsovalues(int valuesCount, double *valuesOut)
	{
		return Cmiss_graphic_contours_get_list_isovalues(reinterpret_cast<Cmiss_graphic_contours_id>(id),
			valuesCount, valuesOut);
	}

	int setListIsovalues(int valuesCount, const double *valuesIn)
	{
		return Cmiss_graphic_contours_set_list_isovalues(reinterpret_cast<Cmiss_graphic_contours_id>(id),
			valuesCount, valuesIn);
	}

	double getRangeFirstIsovalue()
	{
		return Cmiss_graphic_contours_get_range_first_isovalue(
			reinterpret_cast<Cmiss_graphic_contours_id>(id));
	}

	double getRangeLastIsovalue()
	{
		return Cmiss_graphic_contours_get_range_last_isovalue(
			reinterpret_cast<Cmiss_graphic_contours_id>(id));
	}

	int getRangeNumberOfIsovalues()
	{
		return Cmiss_graphic_contours_get_range_number_of_isovalues(
			reinterpret_cast<Cmiss_graphic_contours_id>(id));
	}

	int setRangeIsovalues(int numberOfValues, double firstIsovalue, double lastIsovalue)
	{
		return Cmiss_graphic_contours_set_range_isovalues(reinterpret_cast<Cmiss_graphic_contours_id>(id),
			numberOfValues, firstIsovalue, lastIsovalue);
	}

};

class GraphicLines : public Graphic
{
private:
	explicit GraphicLines(Cmiss_graphic_id graphic_id) : Graphic(graphic_id) {}

public:
	GraphicLines() : Graphic(0) {}

	explicit GraphicLines(Cmiss_graphic_lines_id graphic_lines_id)
		: Graphic(reinterpret_cast<Cmiss_graphic_id>(graphic_lines_id))
	{}

	GraphicLines(Graphic& graphic)
		: Graphic(reinterpret_cast<Cmiss_graphic_id>(Cmiss_graphic_cast_lines(graphic.getId())))
	{}
};

class GraphicPoints : public Graphic
{
private:
	explicit GraphicPoints(Cmiss_graphic_id graphic_id) : Graphic(graphic_id) {}

public:
	GraphicPoints() : Graphic(0) {}

	explicit GraphicPoints(Cmiss_graphic_points_id graphic_points_id)
		: Graphic(reinterpret_cast<Cmiss_graphic_id>(graphic_points_id))
	{}

	GraphicPoints(Graphic& graphic)
		: Graphic(reinterpret_cast<Cmiss_graphic_id>(Cmiss_graphic_cast_points(graphic.getId())))
	{}
};

class GraphicStreamlines : public Graphic
{
private:
	explicit GraphicStreamlines(Cmiss_graphic_id graphic_id) : Graphic(graphic_id) {}

public:
	GraphicStreamlines() : Graphic(0) {}

	explicit GraphicStreamlines(Cmiss_graphic_streamlines_id graphic_streamlines_id)
		: Graphic(reinterpret_cast<Cmiss_graphic_id>(graphic_streamlines_id))
	{}

	GraphicStreamlines(Graphic& graphic)
		: Graphic(reinterpret_cast<Cmiss_graphic_id>(Cmiss_graphic_cast_streamlines(graphic.getId())))
	{}

	enum TrackDirection
	{
		TRACK_DIRECTION_INVALID = CMISS_GRAPHIC_STREAMLINES_TRACK_DIRECTION_INVALID,
		FORWARD_TRACK = CMISS_GRAPHIC_STREAMLINES_FORWARD_TRACK,
		REVERSE_TRACK = CMISS_GRAPHIC_STREAMLINES_REVERSE_TRACK
	};

	Field getStreamVectorField()
	{
		return Field(Cmiss_graphic_streamlines_get_stream_vector_field(reinterpret_cast<Cmiss_graphic_streamlines_id>(id)));
	}

	int setStreamVectorField(Field& field)
	{
		return Cmiss_graphic_streamlines_set_stream_vector_field(reinterpret_cast<Cmiss_graphic_streamlines_id>(id), field.getId());
	}

	TrackDirection getTrackDirection()
	{
		return static_cast<TrackDirection>(
			Cmiss_graphic_streamlines_get_track_direction(reinterpret_cast<Cmiss_graphic_streamlines_id>(id)));
	}

	int setTrackDirection(TrackDirection trackDirection)
	{
		return Cmiss_graphic_streamlines_set_track_direction(reinterpret_cast<Cmiss_graphic_streamlines_id>(id),
			static_cast<Cmiss_graphic_streamlines_track_direction>(trackDirection));
	}

	double getTrackLength()
	{
		return Cmiss_graphic_streamlines_get_track_length(reinterpret_cast<Cmiss_graphic_streamlines_id>(id));
	}

	int setTrackLength(double length)
	{
		return Cmiss_graphic_streamlines_set_track_length(reinterpret_cast<Cmiss_graphic_streamlines_id>(id), length);
	}

};

class GraphicSurfaces : public Graphic
{
private:
	explicit GraphicSurfaces(Cmiss_graphic_id graphic_id) : Graphic(graphic_id) {}

public:
	GraphicSurfaces() : Graphic(0) {}

	explicit GraphicSurfaces(Cmiss_graphic_surfaces_id graphic_surfaces_id)
		: Graphic(reinterpret_cast<Cmiss_graphic_id>(graphic_surfaces_id))
	{}

	GraphicSurfaces(Graphic& graphic)
		: Graphic(reinterpret_cast<Cmiss_graphic_id>(Cmiss_graphic_cast_surfaces(graphic.getId())))
	{}
};

class GraphicLineAttributes
{
protected:
	Cmiss_graphic_line_attributes_id id;

public:

	// takes ownership of C handle, responsibility for destroying it
	explicit GraphicLineAttributes(Cmiss_graphic_line_attributes_id line_attributes_id) :
		id(line_attributes_id)
	{}

	GraphicLineAttributes(const GraphicLineAttributes& lineAttributes) :
		id(Cmiss_graphic_line_attributes_access(lineAttributes.id))
	{}

	~GraphicLineAttributes()
	{
		Cmiss_graphic_line_attributes_destroy(&id);
	}

	bool isValid()
	{
		return (0 != id);
	}

	enum Shape
	{
		SHAPE_INVALID = CMISS_GRAPHIC_LINE_ATTRIBUTES_SHAPE_INVALID,
		SHAPE_LINE = CMISS_GRAPHIC_LINE_ATTRIBUTES_SHAPE_LINE,
		SHAPE_RIBBON = CMISS_GRAPHIC_LINE_ATTRIBUTES_SHAPE_RIBBON,
		SHAPE_CIRCLE_EXTRUSION = CMISS_GRAPHIC_LINE_ATTRIBUTES_SHAPE_CIRCLE_EXTRUSION,
		SHAPE_SQUARE_EXTRUSION = CMISS_GRAPHIC_LINE_ATTRIBUTES_SHAPE_SQUARE_EXTRUSION
	};

	int getBaseSize(int valuesCount, double *valuesOut)
	{
		return Cmiss_graphic_line_attributes_get_base_size(id, valuesCount, valuesOut);
	}

	int setBaseSize(int valuesCount, const double *valuesIn)
	{
		return Cmiss_graphic_line_attributes_set_base_size(id, valuesCount, valuesIn);
	}

	Field getOrientationScaleField()
	{
		return Field(Cmiss_graphic_line_attributes_get_orientation_scale_field(id));
	}

	int setOrientationScaleField(Field& orientationScaleField)
	{
		return Cmiss_graphic_line_attributes_set_orientation_scale_field(id, orientationScaleField.getId());
	}

	int getScaleFactors(int valuesCount, double *valuesOut)
	{
		return Cmiss_graphic_line_attributes_get_scale_factors(id, valuesCount, valuesOut);
	}

	int setScaleFactors(int valuesCount, const double *valuesIn)
	{
		return Cmiss_graphic_line_attributes_set_scale_factors(id, valuesCount, valuesIn);
	}

	Shape getShape()
	{
		return static_cast<Shape>(Cmiss_graphic_line_attributes_get_shape(id));
	}

	int setShape(Shape shape)
	{
		return Cmiss_graphic_line_attributes_set_shape(id, static_cast<Cmiss_graphic_line_attributes_shape>(shape));
	}

};

inline GraphicLineAttributes Graphic::getLineAttributes()
{
	return GraphicLineAttributes(Cmiss_graphic_get_line_attributes(id));
}

class GraphicPointAttributes
{
protected:
	Cmiss_graphic_point_attributes_id id;

public:

	// takes ownership of C handle, responsibility for destroying it
	explicit GraphicPointAttributes(Cmiss_graphic_point_attributes_id point_attributes_id) :
		id(point_attributes_id)
	  {}

	GraphicPointAttributes(const GraphicPointAttributes& pointAttributes) :
		id(Cmiss_graphic_point_attributes_access(pointAttributes.id))
		{}

	~GraphicPointAttributes()
	{
		Cmiss_graphic_point_attributes_destroy(&id);
	}

	bool isValid()
	{
		return (0 != id);
	}

	int getBaseSize(int valuesCount, double *valuesOut)
	{
		return Cmiss_graphic_point_attributes_get_base_size(id, valuesCount, valuesOut);
	}

	int setBaseSize(int valuesCount, const double *valuesIn)
	{
		return Cmiss_graphic_point_attributes_set_base_size(id, valuesCount, valuesIn);
	}

	Font getFont()
	{
		return Font(Cmiss_graphic_point_attributes_get_font(id));
	}

	int setFont(Font& font)
	{
		return Cmiss_graphic_point_attributes_set_font(id, font.getId());
	}

	Glyph getGlyph()
	{
		return Glyph(Cmiss_graphic_point_attributes_get_glyph(id));
	}

	int setGlyph(Glyph& glyph)
	{
		return Cmiss_graphic_point_attributes_set_glyph(id, glyph.getId());
	}

	int getGlyphOffset(int valuesCount, double *valuesOut)
	{
		return Cmiss_graphic_point_attributes_get_glyph_offset(id, valuesCount, valuesOut);
	}

	int setGlyphOffset(int valuesCount, const double *valuesIn)
	{
		return Cmiss_graphic_point_attributes_set_glyph_offset(id, valuesCount, valuesIn);
	}

	Glyph::RepeatMode getGlyphRepeatMode()
	{
		return static_cast<Glyph::RepeatMode>(Cmiss_graphic_point_attributes_get_glyph_repeat_mode(id));
	}

	int setGlyphRepeatMode(Glyph::RepeatMode glyphRepeatMode)
	{
		return Cmiss_graphic_point_attributes_set_glyph_repeat_mode(id,
			static_cast<enum Cmiss_glyph_repeat_mode>(glyphRepeatMode));
	}

	Glyph::Type getGlyphType()
	{
		return static_cast<Glyph::Type>(Cmiss_graphic_point_attributes_get_glyph_type(id));
	}

	int setGlyphType(Glyph::Type type)
	{
		return Cmiss_graphic_point_attributes_set_glyph_type(id,
			static_cast<Cmiss_glyph_type>(type));
	}

	Field getLabelField()
	{
		return Field(Cmiss_graphic_point_attributes_get_label_field(id));
	}

	int setLabelField(Field& labelField)
	{
		return Cmiss_graphic_point_attributes_set_label_field(id, labelField.getId());
	}

	int getLabelOffset(int valuesCount, double *valuesOut)
	{
		return Cmiss_graphic_point_attributes_get_label_offset(id, valuesCount, valuesOut);
	}

	int setLabelOffset(int valuesCount, const double *valuesIn)
	{
		return Cmiss_graphic_point_attributes_set_label_offset(id, valuesCount, valuesIn);
	}

	char *getLabelText(int labelNumber)
	{
		return Cmiss_graphic_point_attributes_get_label_text(id, labelNumber);
	}

	int setLabelText(int labelNumber, const char *labelText)
	{
		return Cmiss_graphic_point_attributes_set_label_text(id, labelNumber, labelText);
	}

	Field getOrientationScaleField()
	{
		return Field(Cmiss_graphic_point_attributes_get_orientation_scale_field(id));
	}

	int setOrientationScaleField(Field& orientationScaleField)
	{
		return Cmiss_graphic_point_attributes_set_orientation_scale_field(id, orientationScaleField.getId());
	}

	int getScaleFactors(int valuesCount, double *valuesOut)
	{
		return Cmiss_graphic_point_attributes_get_scale_factors(id, valuesCount, valuesOut);
	}

	int setScaleFactors(int valuesCount, const double *valuesIn)
	{
		return Cmiss_graphic_point_attributes_set_scale_factors(id, valuesCount, valuesIn);
	}

	Field getSignedScaleField()
	{
		return Field(Cmiss_graphic_point_attributes_get_signed_scale_field(id));
	}

	int setSignedScaleField(Field& signedScaleField)
	{
		return Cmiss_graphic_point_attributes_set_signed_scale_field(id, signedScaleField.getId());
	}

};

inline GraphicPointAttributes Graphic::getPointAttributes()
{
	return GraphicPointAttributes(Cmiss_graphic_get_point_attributes(id));
}

class GraphicSamplingAttributes
{
protected:
	Cmiss_graphic_sampling_attributes_id id;

public:

	// takes ownership of C handle, responsibility for destroying it
	explicit GraphicSamplingAttributes(Cmiss_graphic_sampling_attributes_id sampling_attributes_id) :
		id(sampling_attributes_id)
	  {}

	GraphicSamplingAttributes(const GraphicSamplingAttributes& samplingAttributes) :
		id(Cmiss_graphic_sampling_attributes_access(samplingAttributes.id))
		{}

	~GraphicSamplingAttributes()
	{
		Cmiss_graphic_sampling_attributes_destroy(&id);
	}

	bool isValid()
	{
		return (0 != id);
	}

	Field getDensityField()
	{
		return Field(Cmiss_graphic_sampling_attributes_get_density_field(id));
	}

	int setDensityField(Field& densityField)
	{
		return Cmiss_graphic_sampling_attributes_set_density_field(id, densityField.getId());
	}

	int getLocation(int valuesCount, double *valuesOut)
	{
		return Cmiss_graphic_sampling_attributes_get_location(id, valuesCount, valuesOut);
	}

	int setLocation(int valuesCount, const double *valuesIn)
	{
		return Cmiss_graphic_sampling_attributes_set_location(id, valuesCount, valuesIn);
	}

	Element::PointSampleMode getMode()
	{
		return static_cast<Element::PointSampleMode>(Cmiss_graphic_sampling_attributes_get_mode(id));
	}

	int setMode(Element::PointSampleMode sampleMode)
	{
		return Cmiss_graphic_sampling_attributes_set_mode(id,
			static_cast<Cmiss_element_point_sample_mode>(sampleMode));
	}

};

inline GraphicSamplingAttributes Graphic::getSamplingAttributes()
{
	return GraphicSamplingAttributes(Cmiss_graphic_get_sampling_attributes(id));
}

} // namespace zinc

#endif /* __ZN_CMISS_GRAPHIC_HPP__ */
