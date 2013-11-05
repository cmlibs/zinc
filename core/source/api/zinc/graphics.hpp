/**
 * FILE : graphics.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_GRAPHICS_HPP__
#define CMZN_GRAPHICS_HPP__

#include "zinc/types/scenecoordinatesystem.hpp"
#include "zinc/graphics.h"
#include "zinc/element.hpp"
#include "zinc/field.hpp"
#include "zinc/glyph.hpp"
#include "zinc/font.hpp"
#include "zinc/material.hpp"
#include "zinc/spectrum.hpp"
#include "zinc/tessellation.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class Graphicslineattributes;
class Graphicspointattributes;
class Graphicsamplingattributes;

class Graphics
{

protected:
	cmzn_graphics_id id;

public:

	Graphics() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Graphics(cmzn_graphics_id graphics_id) : id(graphics_id)
	{  }

	Graphics(const Graphics& graphics) : id(cmzn_graphics_access(graphics.id))
	{  }

	Graphics& operator=(const Graphics& graphics)
	{
		cmzn_graphics_id temp_id = cmzn_graphics_access(graphics.id);
		if (0 != id)
		{
			cmzn_graphics_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Graphics()
	{
		if (0 != id)
		{
			cmzn_graphics_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	enum RenderPolygonMode
	{
		RENDER_POLYGON_MODE_INVALID = CMZN_GRAPHICS_RENDER_POLYGON_MODE_INVALID,
		RENDER_POLYGON_MODE_SHADED = CMZN_GRAPHICS_RENDER_POLYGON_MODE_SHADED,
		RENDER_POLYGON_MODE_WIREFRAME = CMZN_GRAPHICS_RENDER_POLYGON_MODE_WIREFRAME
	};

	enum SelectMode
	{
		SELECT_MODE_INVALID = CMZN_GRAPHICS_SELECT_MODE_INVALID,
		SELECT_MODE_ON = CMZN_GRAPHICS_SELECT_MODE_ON,
		SELECT_MODE_OFF = CMZN_GRAPHICS_SELECT_MODE_OFF,
		SELECT_MODE_DRAW_SELECTED = CMZN_GRAPHICS_SELECT_MODE_DRAW_SELECTED,
		SELECT_MODE_DRAW_UNSELECTED = CMZN_GRAPHICS_SELECT_MODE_DRAW_UNSELECTED
	};

	enum Type
	{
		TYPE_INVALID = CMZN_GRAPHICS_TYPE_INVALID,
		TYPE_POINTS = CMZN_GRAPHICS_TYPE_POINTS,
		TYPE_LINES = CMZN_GRAPHICS_TYPE_LINES,
		TYPE_SURFACES = CMZN_GRAPHICS_TYPE_SURFACES,
		TYPE_CONTOURS = CMZN_GRAPHICS_TYPE_CONTOURS,
		TYPE_STREAMLINES = CMZN_GRAPHICS_TYPE_STREAMLINES
	};

	cmzn_graphics_id getId()
	{
		return id;
	}

	Field getCoordinateField()
	{
		return Field(cmzn_graphics_get_coordinate_field(id));
	}

	int setCoordinateField(Field& coordinateField)
	{
		return cmzn_graphics_set_coordinate_field(id, coordinateField.getId());
	}

	Field getDataField()
	{
		return Field(cmzn_graphics_get_data_field(id));
	}

	int setDataField(Field& dataField)
	{
		return cmzn_graphics_set_data_field(id, dataField.getId());
	}

	double getRenderLineWidth()
	{
		return cmzn_graphics_get_render_line_width(id);
	}

	int setRenderLineWidth(double width)
	{
		return cmzn_graphics_set_render_line_width(id, width);
	}

	double getRenderPointSize()
	{
		return cmzn_graphics_get_render_point_size(id);
	}

	int setRenderPointSize(double size)
	{
		return cmzn_graphics_set_render_point_size(id, size);
	}

	enum RenderPolygonMode getRenderPolygonMode()
	{
		return static_cast<RenderPolygonMode>(cmzn_graphics_get_render_polygon_mode(id));
	}

	int setRenderPolygonMode(RenderPolygonMode renderPolygonMode)
	{
		return cmzn_graphics_set_render_polygon_mode(id,
			static_cast<cmzn_graphics_render_polygon_mode>(renderPolygonMode));
	}

	enum SelectMode getSelectMode()
	{
		return static_cast<SelectMode>(cmzn_graphics_get_select_mode(id));
	}

	int setSelectMode(SelectMode selectMode)
	{
		return cmzn_graphics_set_select_mode(id, static_cast<cmzn_graphics_select_mode>(selectMode));
	}

	Field getSubgroupField()
	{
		return Field(cmzn_graphics_get_subgroup_field(id));
	}

	int setSubgroupField(Field& subgroupField)
	{
		return cmzn_graphics_set_subgroup_field(id, subgroupField.getId());
	}

	Field getTextureCoordinateField()
	{
		return Field(cmzn_graphics_get_texture_coordinate_field(id));
	}

	int setTextureCoordinateField(Field& textureCoordinateField)
	{
		return cmzn_graphics_set_texture_coordinate_field(id, textureCoordinateField.getId());
	}

	Material getMaterial()
	{
		return Material(cmzn_graphics_get_material(id));
	}

	int setMaterial(Material& material)
	{
		return cmzn_graphics_set_material(id, material.getId());
	}

	Graphicslineattributes getGraphicslineattributes();

	Graphicspointattributes getGraphicspointattributes();

	Graphicsamplingattributes getGraphicsamplingattributes();

	Material getSelectedMaterial()
	{
		return Material(cmzn_graphics_get_selected_material(id));
	}

	int setSelectedMaterial(Material& material)
	{
		return cmzn_graphics_set_selected_material(id, material.getId());
	}

	Spectrum getSpectrum()
	{
		return Spectrum(cmzn_graphics_get_spectrum(id));
	}

	int setSpectrum(Spectrum& spectrum)
	{
		return cmzn_graphics_set_spectrum(id, spectrum.getId());
	}

	Tessellation getTessellation()
	{
		return Tessellation(cmzn_graphics_get_tessellation(id));
	}

	int setTessellation(Tessellation& tessellation)
	{
		return cmzn_graphics_set_tessellation(id, tessellation.getId());
	}

	Field getTessellationField()
	{
		return Field(cmzn_graphics_get_tessellation_field(id));
	}

	int setTessellationField(Field& tessellationField)
	{
		return cmzn_graphics_set_tessellation_field(id, tessellationField.getId());
	}

	bool getVisibilityFlag()
	{
		return cmzn_graphics_get_visibility_flag(id);
	}

	int setVisibilityFlag(bool visibilityFlag)
	{
		return cmzn_graphics_set_visibility_flag(id, visibilityFlag);
	}

	enum SceneCoordinateSystem getCoordinateSystem()
	{
		return static_cast<SceneCoordinateSystem>(cmzn_graphics_get_coordinate_system(id));
	}

	int setCoordinateSystem(SceneCoordinateSystem coordinateSystem)
	{
		return cmzn_graphics_set_coordinate_system(id,
			static_cast<cmzn_scene_coordinate_system>(coordinateSystem));
	}

	Field::DomainType getFieldDomainType()
	{
		return static_cast<Field::DomainType>(cmzn_graphics_get_field_domain_type(id));
	}

	int setFieldDomainType(Field::DomainType domainType)
	{
		return cmzn_graphics_set_field_domain_type(id, static_cast<cmzn_field_domain_type>(domainType));
	}

	char *getName()
	{
		return cmzn_graphics_get_name(id);
	}

	int setName(const char *name)
	{
		return cmzn_graphics_set_name(id, name);
	}

	int setElementFaceType(Element::FaceType faceType)
	{
		return cmzn_graphics_set_element_face_type(id, static_cast<cmzn_element_face_type>(faceType));
	}

	Element::FaceType getElementFaceType()
	{
		return static_cast<Element::FaceType>(cmzn_graphics_get_element_face_type(id));
	}

	bool isExterior()
	{
		return cmzn_graphics_is_exterior(id);
	}

	int setExterior(bool exterior)
	{
		return cmzn_graphics_set_exterior(id, exterior);
	}
};

class GraphicsContours : public Graphics
{
friend class Scene;
private:
	explicit GraphicsContours(cmzn_graphics_id graphics_id) : Graphics(graphics_id) {}

public:
	GraphicsContours() : Graphics(0) {}

	explicit GraphicsContours(cmzn_graphics_contours_id contours_id)
		: Graphics(reinterpret_cast<cmzn_graphics_id>(contours_id))
	{}

	GraphicsContours(Graphics& graphics)
		: Graphics(reinterpret_cast<cmzn_graphics_id>(cmzn_graphics_cast_contours(graphics.getId())))
	{}

	Field getIsoscalarField()
	{
		return Field(cmzn_graphics_contours_get_isoscalar_field(reinterpret_cast<cmzn_graphics_contours_id>(id)));
	}

	int setIsoscalarField(Field& field)
	{
		return cmzn_graphics_contours_set_isoscalar_field(reinterpret_cast<cmzn_graphics_contours_id>(id), field.getId());
	}

	int getListIsovalues(int valuesCount, double *valuesOut)
	{
		return cmzn_graphics_contours_get_list_isovalues(reinterpret_cast<cmzn_graphics_contours_id>(id),
			valuesCount, valuesOut);
	}

	int setListIsovalues(int valuesCount, const double *valuesIn)
	{
		return cmzn_graphics_contours_set_list_isovalues(reinterpret_cast<cmzn_graphics_contours_id>(id),
			valuesCount, valuesIn);
	}

	double getRangeFirstIsovalue()
	{
		return cmzn_graphics_contours_get_range_first_isovalue(
			reinterpret_cast<cmzn_graphics_contours_id>(id));
	}

	double getRangeLastIsovalue()
	{
		return cmzn_graphics_contours_get_range_last_isovalue(
			reinterpret_cast<cmzn_graphics_contours_id>(id));
	}

	int getRangeNumberOfIsovalues()
	{
		return cmzn_graphics_contours_get_range_number_of_isovalues(
			reinterpret_cast<cmzn_graphics_contours_id>(id));
	}

	int setRangeIsovalues(int numberOfValues, double firstIsovalue, double lastIsovalue)
	{
		return cmzn_graphics_contours_set_range_isovalues(reinterpret_cast<cmzn_graphics_contours_id>(id),
			numberOfValues, firstIsovalue, lastIsovalue);
	}

};

class GraphicsLines : public Graphics
{
friend class Scene;
private:
	explicit GraphicsLines(cmzn_graphics_id graphics_id) : Graphics(graphics_id) {}

public:
	GraphicsLines() : Graphics(0) {}

	explicit GraphicsLines(cmzn_graphics_lines_id lines_id)
		: Graphics(reinterpret_cast<cmzn_graphics_id>(lines_id))
	{}

	GraphicsLines(Graphics& graphics)
		: Graphics(reinterpret_cast<cmzn_graphics_id>(cmzn_graphics_cast_lines(graphics.getId())))
	{}
};

class GraphicsPoints : public Graphics
{
friend class Scene;
private:
	explicit GraphicsPoints(cmzn_graphics_id graphics_id) : Graphics(graphics_id) {}

public:
	GraphicsPoints() : Graphics(0) {}

	explicit GraphicsPoints(cmzn_graphics_points_id points_id)
		: Graphics(reinterpret_cast<cmzn_graphics_id>(points_id))
	{}

	GraphicsPoints(Graphics& graphics)
		: Graphics(reinterpret_cast<cmzn_graphics_id>(cmzn_graphics_cast_points(graphics.getId())))
	{}
};

class GraphicsStreamlines : public Graphics
{
friend class Scene;
private:
	explicit GraphicsStreamlines(cmzn_graphics_id graphics_id) : Graphics(graphics_id) {}

public:
	GraphicsStreamlines() : Graphics(0) {}

	explicit GraphicsStreamlines(cmzn_graphics_streamlines_id streamlines_id)
		: Graphics(reinterpret_cast<cmzn_graphics_id>(streamlines_id))
	{}

	GraphicsStreamlines(Graphics& graphics)
		: Graphics(reinterpret_cast<cmzn_graphics_id>(cmzn_graphics_cast_streamlines(graphics.getId())))
	{}

	enum TrackDirection
	{
		TRACK_DIRECTION_INVALID = CMZN_GRAPHICS_STREAMLINES_TRACK_DIRECTION_INVALID,
		TRACK_DIRECTION_FORWARD = CMZN_GRAPHICS_STREAMLINES_TRACK_DIRECTION_FORWARD,
		TRACK_DIRECTION_REVERSE = CMZN_GRAPHICS_STREAMLINES_TRACK_DIRECTION_REVERSE
	};

	Field getStreamVectorField()
	{
		return Field(cmzn_graphics_streamlines_get_stream_vector_field(reinterpret_cast<cmzn_graphics_streamlines_id>(id)));
	}

	int setStreamVectorField(Field& field)
	{
		return cmzn_graphics_streamlines_set_stream_vector_field(reinterpret_cast<cmzn_graphics_streamlines_id>(id), field.getId());
	}

	TrackDirection getTrackDirection()
	{
		return static_cast<TrackDirection>(
			cmzn_graphics_streamlines_get_track_direction(reinterpret_cast<cmzn_graphics_streamlines_id>(id)));
	}

	int setTrackDirection(TrackDirection trackDirection)
	{
		return cmzn_graphics_streamlines_set_track_direction(reinterpret_cast<cmzn_graphics_streamlines_id>(id),
			static_cast<cmzn_graphics_streamlines_track_direction>(trackDirection));
	}

	double getTrackLength()
	{
		return cmzn_graphics_streamlines_get_track_length(reinterpret_cast<cmzn_graphics_streamlines_id>(id));
	}

	int setTrackLength(double length)
	{
		return cmzn_graphics_streamlines_set_track_length(reinterpret_cast<cmzn_graphics_streamlines_id>(id), length);
	}

};

class GraphicsSurfaces : public Graphics
{
friend class Scene;
private:
	explicit GraphicsSurfaces(cmzn_graphics_id graphics_id) : Graphics(graphics_id) {}

public:
	GraphicsSurfaces() : Graphics(0) {}

	explicit GraphicsSurfaces(cmzn_graphics_surfaces_id surfaces_id)
		: Graphics(reinterpret_cast<cmzn_graphics_id>(surfaces_id))
	{}

	GraphicsSurfaces(Graphics& graphics)
		: Graphics(reinterpret_cast<cmzn_graphics_id>(cmzn_graphics_cast_surfaces(graphics.getId())))
	{}
};

class Graphicslineattributes
{
protected:
	cmzn_graphicslineattributes_id id;

public:

	// takes ownership of C handle, responsibility for destroying it
	explicit Graphicslineattributes(cmzn_graphicslineattributes_id line_attributes_id) :
		id(line_attributes_id)
	{}

	Graphicslineattributes(const Graphicslineattributes& lineAttributes) :
		id(cmzn_graphicslineattributes_access(lineAttributes.id))
	{}

	~Graphicslineattributes()
	{
		cmzn_graphicslineattributes_destroy(&id);
	}

	bool isValid()
	{
		return (0 != id);
	}

	enum ShapeType
	{
		SHAPE_TYPE_INVALID = CMZN_GRAPHICSLINEATTRIBUTES_SHAPE_TYPE_INVALID,
		SHAPE_TYPE_LINE = CMZN_GRAPHICSLINEATTRIBUTES_SHAPE_TYPE_LINE,
		SHAPE_TYPE_RIBBON = CMZN_GRAPHICSLINEATTRIBUTES_SHAPE_TYPE_RIBBON,
		SHAPE_TYPE_CIRCLE_EXTRUSION = CMZN_GRAPHICSLINEATTRIBUTES_SHAPE_TYPE_CIRCLE_EXTRUSION,
		SHAPE_TYPE_SQUARE_EXTRUSION = CMZN_GRAPHICSLINEATTRIBUTES_SHAPE_TYPE_SQUARE_EXTRUSION
	};

	int getBaseSize(int valuesCount, double *valuesOut)
	{
		return cmzn_graphicslineattributes_get_base_size(id, valuesCount, valuesOut);
	}

	int setBaseSize(int valuesCount, const double *valuesIn)
	{
		return cmzn_graphicslineattributes_set_base_size(id, valuesCount, valuesIn);
	}

	Field getOrientationScaleField()
	{
		return Field(cmzn_graphicslineattributes_get_orientation_scale_field(id));
	}

	int setOrientationScaleField(Field& orientationScaleField)
	{
		return cmzn_graphicslineattributes_set_orientation_scale_field(id, orientationScaleField.getId());
	}

	int getScaleFactors(int valuesCount, double *valuesOut)
	{
		return cmzn_graphicslineattributes_get_scale_factors(id, valuesCount, valuesOut);
	}

	int setScaleFactors(int valuesCount, const double *valuesIn)
	{
		return cmzn_graphicslineattributes_set_scale_factors(id, valuesCount, valuesIn);
	}

	ShapeType getShapeType()
	{
		return static_cast<ShapeType>(cmzn_graphicslineattributes_get_shape_type(id));
	}

	int setShapeType(ShapeType shapeType)
	{
		return cmzn_graphicslineattributes_set_shape_type(id, static_cast<cmzn_graphicslineattributes_shape_type>(shapeType));
	}

};

inline Graphicslineattributes Graphics::getGraphicslineattributes()
{
	return Graphicslineattributes(cmzn_graphics_get_graphicslineattributes(id));
}

class Graphicspointattributes
{
protected:
	cmzn_graphicspointattributes_id id;

public:

	// takes ownership of C handle, responsibility for destroying it
	explicit Graphicspointattributes(cmzn_graphicspointattributes_id point_attributes_id) :
		id(point_attributes_id)
	  {}

	Graphicspointattributes(const Graphicspointattributes& pointAttributes) :
		id(cmzn_graphicspointattributes_access(pointAttributes.id))
		{}

	~Graphicspointattributes()
	{
		cmzn_graphicspointattributes_destroy(&id);
	}

	bool isValid()
	{
		return (0 != id);
	}

	int getBaseSize(int valuesCount, double *valuesOut)
	{
		return cmzn_graphicspointattributes_get_base_size(id, valuesCount, valuesOut);
	}

	int setBaseSize(int valuesCount, const double *valuesIn)
	{
		return cmzn_graphicspointattributes_set_base_size(id, valuesCount, valuesIn);
	}

	Font getFont()
	{
		return Font(cmzn_graphicspointattributes_get_font(id));
	}

	int setFont(Font& font)
	{
		return cmzn_graphicspointattributes_set_font(id, font.getId());
	}

	Glyph getGlyph()
	{
		return Glyph(cmzn_graphicspointattributes_get_glyph(id));
	}

	int setGlyph(Glyph& glyph)
	{
		return cmzn_graphicspointattributes_set_glyph(id, glyph.getId());
	}

	int getGlyphOffset(int valuesCount, double *valuesOut)
	{
		return cmzn_graphicspointattributes_get_glyph_offset(id, valuesCount, valuesOut);
	}

	int setGlyphOffset(int valuesCount, const double *valuesIn)
	{
		return cmzn_graphicspointattributes_set_glyph_offset(id, valuesCount, valuesIn);
	}

	Glyph::RepeatMode getGlyphRepeatMode()
	{
		return static_cast<Glyph::RepeatMode>(cmzn_graphicspointattributes_get_glyph_repeat_mode(id));
	}

	int setGlyphRepeatMode(Glyph::RepeatMode glyphRepeatMode)
	{
		return cmzn_graphicspointattributes_set_glyph_repeat_mode(id,
			static_cast<enum cmzn_glyph_repeat_mode>(glyphRepeatMode));
	}

	Glyph::Type getGlyphType()
	{
		return static_cast<Glyph::Type>(cmzn_graphicspointattributes_get_glyph_type(id));
	}

	int setGlyphType(Glyph::Type type)
	{
		return cmzn_graphicspointattributes_set_glyph_type(id,
			static_cast<cmzn_glyph_type>(type));
	}

	Field getLabelField()
	{
		return Field(cmzn_graphicspointattributes_get_label_field(id));
	}

	int setLabelField(Field& labelField)
	{
		return cmzn_graphicspointattributes_set_label_field(id, labelField.getId());
	}

	int getLabelOffset(int valuesCount, double *valuesOut)
	{
		return cmzn_graphicspointattributes_get_label_offset(id, valuesCount, valuesOut);
	}

	int setLabelOffset(int valuesCount, const double *valuesIn)
	{
		return cmzn_graphicspointattributes_set_label_offset(id, valuesCount, valuesIn);
	}

	char *getLabelText(int labelNumber)
	{
		return cmzn_graphicspointattributes_get_label_text(id, labelNumber);
	}

	int setLabelText(int labelNumber, const char *labelText)
	{
		return cmzn_graphicspointattributes_set_label_text(id, labelNumber, labelText);
	}

	Field getOrientationScaleField()
	{
		return Field(cmzn_graphicspointattributes_get_orientation_scale_field(id));
	}

	int setOrientationScaleField(Field& orientationScaleField)
	{
		return cmzn_graphicspointattributes_set_orientation_scale_field(id, orientationScaleField.getId());
	}

	int getScaleFactors(int valuesCount, double *valuesOut)
	{
		return cmzn_graphicspointattributes_get_scale_factors(id, valuesCount, valuesOut);
	}

	int setScaleFactors(int valuesCount, const double *valuesIn)
	{
		return cmzn_graphicspointattributes_set_scale_factors(id, valuesCount, valuesIn);
	}

	Field getSignedScaleField()
	{
		return Field(cmzn_graphicspointattributes_get_signed_scale_field(id));
	}

	int setSignedScaleField(Field& signedScaleField)
	{
		return cmzn_graphicspointattributes_set_signed_scale_field(id, signedScaleField.getId());
	}

};

inline Graphicspointattributes Graphics::getGraphicspointattributes()
{
	return Graphicspointattributes(cmzn_graphics_get_graphicspointattributes(id));
}

class Graphicsamplingattributes
{
protected:
	cmzn_graphicssamplingattributes_id id;

public:

	// takes ownership of C handle, responsibility for destroying it
	explicit Graphicsamplingattributes(cmzn_graphicssamplingattributes_id sampling_attributes_id) :
		id(sampling_attributes_id)
	  {}

	Graphicsamplingattributes(const Graphicsamplingattributes& samplingAttributes) :
		id(cmzn_graphicssamplingattributes_access(samplingAttributes.id))
		{}

	~Graphicsamplingattributes()
	{
		cmzn_graphicssamplingattributes_destroy(&id);
	}

	bool isValid()
	{
		return (0 != id);
	}

	Field getDensityField()
	{
		return Field(cmzn_graphicssamplingattributes_get_density_field(id));
	}

	int setDensityField(Field& densityField)
	{
		return cmzn_graphicssamplingattributes_set_density_field(id, densityField.getId());
	}

	int getLocation(int valuesCount, double *valuesOut)
	{
		return cmzn_graphicssamplingattributes_get_location(id, valuesCount, valuesOut);
	}

	int setLocation(int valuesCount, const double *valuesIn)
	{
		return cmzn_graphicssamplingattributes_set_location(id, valuesCount, valuesIn);
	}

	Element::PointSamplingMode getElementPointSamplingMode()
	{
		return static_cast<Element::PointSamplingMode>(cmzn_graphicssamplingattributes_get_element_point_sampling_mode(id));
	}

	int setElementPointSamplingMode(Element::PointSamplingMode samplingMode)
	{
		return cmzn_graphicssamplingattributes_set_element_point_sampling_mode(id,
			static_cast<cmzn_element_point_sampling_mode>(samplingMode));
	}

};

inline Graphicsamplingattributes Graphics::getGraphicsamplingattributes()
{
	return Graphicsamplingattributes(cmzn_graphics_get_graphicssamplingattributes(id));
}

} // namespace Zinc
}

#endif
