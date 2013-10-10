/**
 * FILE : glyph.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_GLYPH_HPP__
#define CMZN_GLYPH_HPP__

#include "zinc/glyph.h"
#include "zinc/graphicsmaterial.hpp"
#include "zinc/spectrum.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class Glyph
{
protected:
	cmzn_glyph_id id;

public:

	Glyph() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Glyph(cmzn_glyph_id in_glyph_id) :
		id(in_glyph_id)
	{  }

	Glyph(const Glyph& glyph) :
		id(cmzn_glyph_access(glyph.id))
	{  }

	Glyph& operator=(const Glyph& glyph)
	{
		cmzn_glyph_id temp_id = cmzn_glyph_access(glyph.id);
		if (0 != id)
		{
			cmzn_glyph_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Glyph()
	{
		if (0 != id)
		{
			cmzn_glyph_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_glyph_id getId()
	{
		return id;
	}

	enum RepeatMode
	{
		REPEAT_MODE_INVALID = CMZN_GLYPH_REPEAT_MODE_INVALID,
		REPEAT_NONE = CMZN_GLYPH_REPEAT_NONE,
		REPEAT_AXES_2D = CMZN_GLYPH_REPEAT_AXES_2D,
		REPEAT_AXES_3D = CMZN_GLYPH_REPEAT_AXES_3D,
		REPEAT_MIRROR = CMZN_GLYPH_REPEAT_MIRROR
	};


	enum Type
	{
		TYPE_INVALID = CMZN_GLYPH_TYPE_INVALID,
		NONE = CMZN_GLYPH_NONE,
		ARROW = CMZN_GLYPH_ARROW,
		ARROW_SOLID = CMZN_GLYPH_ARROW_SOLID,
		AXIS = CMZN_GLYPH_AXIS,
		AXIS_SOLID = CMZN_GLYPH_AXIS_SOLID,
		CONE = CMZN_GLYPH_CONE,
		CONE_SOLID = CMZN_GLYPH_CONE_SOLID,
		CROSS = CMZN_GLYPH_CROSS,
		CUBE_SOLID = CMZN_GLYPH_CUBE_SOLID,
		CUBE_WIREFRAME = CMZN_GLYPH_CUBE_WIREFRAME,
		CYLINDER = CMZN_GLYPH_CYLINDER,
		CYLINDER_SOLID = CMZN_GLYPH_CYLINDER_SOLID,
		DIAMOND = CMZN_GLYPH_DIAMOND,
		LINE = CMZN_GLYPH_LINE,
		POINT = CMZN_GLYPH_POINT,
		SHEET = CMZN_GLYPH_SHEET,
		SPHERE = CMZN_GLYPH_SPHERE,
		AXES = CMZN_GLYPH_AXES,
		AXES_123 = CMZN_GLYPH_AXES_123,
		AXES_XYZ = CMZN_GLYPH_AXES_XYZ,
		AXES_COLOUR = CMZN_GLYPH_AXES_COLOUR,
		AXES_SOLID = CMZN_GLYPH_AXES_SOLID,
		AXES_SOLID_123 = CMZN_GLYPH_AXES_SOLID_123,
		AXES_SOLID_XYZ = CMZN_GLYPH_AXES_SOLID_XYZ,
		AXES_SOLID_COLOUR = CMZN_GLYPH_AXES_SOLID_COLOUR
	};

	char *getName()
	{
		return cmzn_glyph_get_name(id);
	}

	int setName(const char *name)
	{
		return cmzn_glyph_set_name(id, name);
	}

	bool isManaged()
	{
		return (0 != cmzn_glyph_is_managed(id));
	}

	int setManaged(bool value)
	{
		return cmzn_glyph_set_managed(id, static_cast<int>(value));
	}
};

class GlyphAxes : public Glyph
{
private:
	explicit GlyphAxes(cmzn_glyph_id glyph_id) : Glyph(glyph_id) {}

	inline cmzn_glyph_axes_id getDerivedId()
	{
		return reinterpret_cast<cmzn_glyph_axes_id>(id);
	}

public:
	GlyphAxes() : Glyph(0) {}

	explicit GlyphAxes(cmzn_glyph_axes_id axes_id)
		: Glyph(reinterpret_cast<cmzn_glyph_id>(axes_id))
	{}

	GlyphAxes(Glyph& glyph)
		: Glyph(reinterpret_cast<cmzn_glyph_id>(cmzn_glyph_cast_axes(glyph.getId())))
	{}

	double getAxisWidth() 
	{
		return cmzn_glyph_axes_get_axis_width(getDerivedId());
	}

	int setAxisWidth(double axisWidth)
	{
		return cmzn_glyph_axes_set_axis_width(getDerivedId(), axisWidth);
	}

	char *getAxisLabel(int axisNumber)
	{
		return cmzn_glyph_axes_get_axis_label(getDerivedId(), axisNumber);
	}

	int setAxisLabel(int axisNumber, const char *label)
	{
		return cmzn_glyph_axes_set_axis_label(getDerivedId(), axisNumber, label);
	}

	GraphicsMaterial getAxisMaterial(int axisNumber)
	{
		return GraphicsMaterial(cmzn_glyph_axes_get_axis_material(getDerivedId(), axisNumber));
	}

	int setAxisMaterial(int axisNumber, GraphicsMaterial material)
	{
		return cmzn_glyph_axes_set_axis_material(getDerivedId(), axisNumber, material.getId());
	}

};

class GlyphColourBar : public Glyph
{
private:
	explicit GlyphColourBar(cmzn_glyph_id glyph_id) : Glyph(glyph_id) {}

	inline cmzn_glyph_colour_bar_id getDerivedId()
	{
		return reinterpret_cast<cmzn_glyph_colour_bar_id>(id);
	}

public:
	GlyphColourBar() : Glyph(0) {}

	explicit GlyphColourBar(cmzn_glyph_colour_bar_id colour_bar_id)
		: Glyph(reinterpret_cast<cmzn_glyph_id>(colour_bar_id))
	{}

	GlyphColourBar(Glyph& glyph)
		: Glyph(reinterpret_cast<cmzn_glyph_id>(cmzn_glyph_cast_colour_bar(glyph.getId())))
	{}

	int getAxis(int valuesCount, double *valuesOut)
	{
		return cmzn_glyph_colour_bar_get_axis(getDerivedId(), valuesCount, valuesOut);
	}

	int setAxis(int valuesCount, const double *valuesIn)
	{
		return cmzn_glyph_colour_bar_set_axis(getDerivedId(), valuesCount, valuesIn);
	}

	int getCentre(int valuesCount, double *valuesOut)
	{
		return cmzn_glyph_colour_bar_get_centre(getDerivedId(), valuesCount, valuesOut);
	}

	int setCentre(int valuesCount, const double *valuesIn)
	{
		return cmzn_glyph_colour_bar_set_centre(getDerivedId(), valuesCount, valuesIn);
	}

	double getExtendLength() 
	{
		return cmzn_glyph_colour_bar_get_extend_length(getDerivedId());
	}

	int setExtendLength(double extendLength)
	{
		return cmzn_glyph_colour_bar_set_extend_length(getDerivedId(), extendLength);
	}

	int getLabelDivisions() 
	{
		return cmzn_glyph_colour_bar_get_label_divisions(getDerivedId());
	}

	int setLabelDivisions(int labelDivisions)
	{
		return cmzn_glyph_colour_bar_set_label_divisions(getDerivedId(), labelDivisions);
	}

	GraphicsMaterial getLabelMaterial()
	{
		return GraphicsMaterial(cmzn_glyph_colour_bar_get_label_material(getDerivedId()));
	}

	int setLabelMaterial(GraphicsMaterial& material)
	{
		return cmzn_glyph_colour_bar_set_label_material(getDerivedId(), material.getId());
	}

	char *getNumberFormat()
	{
		return cmzn_glyph_colour_bar_get_number_format(getDerivedId());
	}

	int setNumberFormat(const char *numberFormat)
	{
		return cmzn_glyph_colour_bar_set_number_format(getDerivedId(), numberFormat);
	}

	int getSideAxis(int valuesCount, double *valuesOut)
	{
		return cmzn_glyph_colour_bar_get_side_axis(getDerivedId(), valuesCount, valuesOut);
	}

	int setSideAxis(int valuesCount, const double *valuesIn)
	{
		return cmzn_glyph_colour_bar_set_side_axis(getDerivedId(), valuesCount, valuesIn);
	}

	double getTickLength() 
	{
		return cmzn_glyph_colour_bar_get_tick_length(getDerivedId());
	}

	int setTickLength(double tickLength)
	{
		return cmzn_glyph_colour_bar_set_tick_length(getDerivedId(), tickLength);
	}

};

class Glyphmodule
{
protected:
	cmzn_glyphmodule_id id;

public:

	Glyphmodule() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Glyphmodule(cmzn_glyphmodule_id in_glyphmodule_id) :
		id(in_glyphmodule_id)
	{  }

	Glyphmodule(const Glyphmodule& glyphModule) :
		id(cmzn_glyphmodule_access(glyphModule.id))
	{  }

	Glyphmodule& operator=(const Glyphmodule& glyphModule)
	{
		cmzn_glyphmodule_id temp_id = cmzn_glyphmodule_access(glyphModule.id);
		if (0 != id)
		{
			cmzn_glyphmodule_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Glyphmodule()
	{
		if (0 != id)
		{
			cmzn_glyphmodule_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_glyphmodule_id getId()
	{
		return id;
	}

	int beginChange()
	{
		return cmzn_glyphmodule_begin_change(id);
	}

	int endChange()
	{
		return cmzn_glyphmodule_end_change(id);
	}

	GlyphAxes createAxes(Glyph& axisGlyph, double axisWidth)
	{
		return GlyphAxes(cmzn_glyphmodule_create_axes(id, axisGlyph.getId(), axisWidth));
	}

	GlyphColourBar createColourBar(Spectrum& spectrum)
	{
		return GlyphColourBar(cmzn_glyphmodule_create_colour_bar(id, spectrum.getId()));
	}

	int defineStandardGlyphs()
	{
		return cmzn_glyphmodule_define_standard_glyphs(id);
	}

	Glyph findGlyphByName(const char *name)
	{
		return Glyph(cmzn_glyphmodule_find_glyph_by_name(id, name));
	}

	Glyph findGlyphByType(Glyph::Type glyphType)
	{
		return Glyph(cmzn_glyphmodule_find_glyph_by_type(id, static_cast<cmzn_glyph_type>(glyphType)));
	}

	Glyph getDefaultPointGlyph()
	{
		return Glyph(cmzn_glyphmodule_get_default_point_glyph(id));
	}

	int setDefaultPointGlyph(Glyph& glyph)
	{
		return cmzn_glyphmodule_set_default_point_glyph(id, glyph.getId());
	}

};

}  // namespace Zinc
}

#endif
