/**
 * @file glyph.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_GLYPH_HPP__
#define CMZN_GLYPH_HPP__

#include "zinc/glyph.h"
#include "zinc/context.hpp"
#include "zinc/material.hpp"
#include "zinc/spectrum.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class GlyphAxes;
class GlyphColourBar;

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

	bool isValid() const
	{
		return (0 != id);
	}

	cmzn_glyph_id getId() const
	{
		return id;
	}

	enum RepeatMode
	{
		REPEAT_MODE_INVALID = CMZN_GLYPH_REPEAT_MODE_INVALID,
		REPEAT_MODE_NONE = CMZN_GLYPH_REPEAT_MODE_NONE,
		/*!< default RepeatMode */
		REPEAT_MODE_AXES_2D = CMZN_GLYPH_REPEAT_MODE_AXES_2D,
		REPEAT_MODE_AXES_3D = CMZN_GLYPH_REPEAT_MODE_AXES_3D,
		REPEAT_MODE_MIRROR = CMZN_GLYPH_REPEAT_MODE_MIRROR
	};


	enum ShapeType
	{
		SHAPE_TYPE_INVALID = CMZN_GLYPH_SHAPE_TYPE_INVALID,
		SHAPE_TYPE_NONE = CMZN_GLYPH_SHAPE_TYPE_NONE,
		SHAPE_TYPE_ARROW = CMZN_GLYPH_SHAPE_TYPE_ARROW,
		SHAPE_TYPE_ARROW_SOLID = CMZN_GLYPH_SHAPE_TYPE_ARROW_SOLID,
		SHAPE_TYPE_AXIS = CMZN_GLYPH_SHAPE_TYPE_AXIS,
		SHAPE_TYPE_AXIS_SOLID = CMZN_GLYPH_SHAPE_TYPE_AXIS_SOLID,
		SHAPE_TYPE_CONE = CMZN_GLYPH_SHAPE_TYPE_CONE,
		SHAPE_TYPE_CONE_SOLID = CMZN_GLYPH_SHAPE_TYPE_CONE_SOLID,
		SHAPE_TYPE_CROSS = CMZN_GLYPH_SHAPE_TYPE_CROSS,
		SHAPE_TYPE_CUBE_SOLID = CMZN_GLYPH_SHAPE_TYPE_CUBE_SOLID,
		SHAPE_TYPE_CUBE_WIREFRAME = CMZN_GLYPH_SHAPE_TYPE_CUBE_WIREFRAME,
		SHAPE_TYPE_CYLINDER = CMZN_GLYPH_SHAPE_TYPE_CYLINDER,
		SHAPE_TYPE_CYLINDER_SOLID = CMZN_GLYPH_SHAPE_TYPE_CYLINDER_SOLID,
		SHAPE_TYPE_DIAMOND = CMZN_GLYPH_SHAPE_TYPE_DIAMOND,
		SHAPE_TYPE_LINE = CMZN_GLYPH_SHAPE_TYPE_LINE,
		SHAPE_TYPE_POINT = CMZN_GLYPH_SHAPE_TYPE_POINT,
		SHAPE_TYPE_SHEET = CMZN_GLYPH_SHAPE_TYPE_SHEET,
		SHAPE_TYPE_SPHERE = CMZN_GLYPH_SHAPE_TYPE_SPHERE,
		SHAPE_TYPE_AXES = CMZN_GLYPH_SHAPE_TYPE_AXES,
		SHAPE_TYPE_AXES_123 = CMZN_GLYPH_SHAPE_TYPE_AXES_123,
		SHAPE_TYPE_AXES_XYZ = CMZN_GLYPH_SHAPE_TYPE_AXES_XYZ,
		SHAPE_TYPE_AXES_COLOUR = CMZN_GLYPH_SHAPE_TYPE_AXES_COLOUR,
		SHAPE_TYPE_AXES_SOLID = CMZN_GLYPH_SHAPE_TYPE_AXES_SOLID,
		SHAPE_TYPE_AXES_SOLID_123 = CMZN_GLYPH_SHAPE_TYPE_AXES_SOLID_123,
		SHAPE_TYPE_AXES_SOLID_XYZ = CMZN_GLYPH_SHAPE_TYPE_AXES_SOLID_XYZ,
		SHAPE_TYPE_AXES_SOLID_COLOUR = CMZN_GLYPH_SHAPE_TYPE_AXES_SOLID_COLOUR
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

	inline GlyphAxes castAxes();
	inline GlyphColourBar castColourBar();
};

inline bool operator==(const Glyph& a, const Glyph& b)
{
	return a.getId() == b.getId();
}

class GlyphAxes : public Glyph
{
private:
	inline cmzn_glyph_axes_id getDerivedId()
	{
		return reinterpret_cast<cmzn_glyph_axes_id>(id);
	}

public:
	GlyphAxes() : Glyph(0) {}

	explicit GlyphAxes(cmzn_glyph_axes_id axes_id)
		: Glyph(reinterpret_cast<cmzn_glyph_id>(axes_id))
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

	Material getAxisMaterial(int axisNumber)
	{
		return Material(cmzn_glyph_axes_get_axis_material(getDerivedId(), axisNumber));
	}

	int setAxisMaterial(int axisNumber, const Material& material)
	{
		return cmzn_glyph_axes_set_axis_material(getDerivedId(), axisNumber, material.getId());
	}

};

inline GlyphAxes Glyph::castAxes()
{
	return GlyphAxes(cmzn_glyph_cast_axes(id));
}

class GlyphColourBar : public Glyph
{
private:
	inline cmzn_glyph_colour_bar_id getDerivedId()
	{
		return reinterpret_cast<cmzn_glyph_colour_bar_id>(id);
	}

public:
	GlyphColourBar() : Glyph(0) {}

	explicit GlyphColourBar(cmzn_glyph_colour_bar_id colour_bar_id)
		: Glyph(reinterpret_cast<cmzn_glyph_id>(colour_bar_id))
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

	Material getLabelMaterial()
	{
		return Material(cmzn_glyph_colour_bar_get_label_material(getDerivedId()));
	}

	int setLabelMaterial(const Material& material)
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

inline GlyphColourBar Glyph::castColourBar()
{
	return GlyphColourBar(cmzn_glyph_cast_colour_bar(id));
}

class Glyphiterator
{
private:

	cmzn_glyphiterator_id id;

public:

	Glyphiterator() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit Glyphiterator(cmzn_glyphiterator_id iterator_id) :
		id(iterator_id)
	{ }

	Glyphiterator(const Glyphiterator& glyphiterator) :
		id(cmzn_glyphiterator_access(glyphiterator.id))
	{ }

	Glyphiterator& operator=(const Glyphiterator& glyphiterator)
	{
		cmzn_glyphiterator_id temp_id = cmzn_glyphiterator_access(glyphiterator.id);
		if (0 != id)
		{
			cmzn_glyphiterator_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Glyphiterator()
	{
		if (0 != id)
		{
			cmzn_glyphiterator_destroy(&id);
		}
	}

	bool isValid() const
	{
		return (0 != id);
	}

	Glyph next()
	{
		return Glyph(cmzn_glyphiterator_next(id));
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

	bool isValid() const
	{
		return (0 != id);
	}

	cmzn_glyphmodule_id getId() const
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

	GlyphAxes createGlyphAxes(const Glyph& axisGlyph, double axisWidth)
	{
		return GlyphAxes(reinterpret_cast<cmzn_glyph_axes_id>(
			cmzn_glyphmodule_create_glyph_axes(id, axisGlyph.getId(), axisWidth)));
	}

	GlyphColourBar createGlyphColourBar(const Spectrum& spectrum)
	{
		return GlyphColourBar(reinterpret_cast<cmzn_glyph_colour_bar_id>(
			cmzn_glyphmodule_create_glyph_colour_bar(id, spectrum.getId())));
	}

	Glyphiterator createGlyphiterator()
	{
		return Glyphiterator(cmzn_glyphmodule_create_glyphiterator(id));
	}

	int defineStandardGlyphs()
	{
		return cmzn_glyphmodule_define_standard_glyphs(id);
	}

	Glyph findGlyphByName(const char *name)
	{
		return Glyph(cmzn_glyphmodule_find_glyph_by_name(id, name));
	}

	Glyph findGlyphByGlyphShapeType(Glyph::ShapeType glyphShapeType)
	{
		return Glyph(cmzn_glyphmodule_find_glyph_by_glyph_shape_type(id, static_cast<cmzn_glyph_shape_type>(glyphShapeType)));
	}

	Glyph getDefaultPointGlyph()
	{
		return Glyph(cmzn_glyphmodule_get_default_point_glyph(id));
	}

	int setDefaultPointGlyph(const Glyph& glyph)
	{
		return cmzn_glyphmodule_set_default_point_glyph(id, glyph.getId());
	}

};

inline Glyphmodule Context::getGlyphmodule()
{
	return Glyphmodule(cmzn_context_get_glyphmodule(id));
}

}  // namespace Zinc
}

#endif
