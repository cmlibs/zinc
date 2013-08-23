/**
 * FILE : glyph.hpp
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
#ifndef ZINC_GLYPH_HPP
#define ZINC_GLYPH_HPP

#include "zinc/glyph.h"
#include "zinc/graphicsmaterial.hpp"
#include "zinc/spectrum.hpp"

namespace zinc
{

class Glyph
{
protected:
	Cmiss_glyph_id id;

public:

	Glyph() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Glyph(Cmiss_glyph_id in_glyph_id) :
		id(in_glyph_id)
	{  }

	Glyph(const Glyph& glyph) :
		id(Cmiss_glyph_access(glyph.id))
	{  }

	Glyph& operator=(const Glyph& glyph)
	{
		Cmiss_glyph_id temp_id = Cmiss_glyph_access(glyph.id);
		if (0 != id)
		{
			Cmiss_glyph_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Glyph()
	{
		if (0 != id)
		{
			Cmiss_glyph_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Cmiss_glyph_id getId()
	{
		return id;
	}

	enum RepeatMode
	{
		REPEAT_MODE_INVALID = CMISS_GLYPH_REPEAT_MODE_INVALID,
		REPEAT_NONE = CMISS_GLYPH_REPEAT_NONE,
		REPEAT_AXES_2D = CMISS_GLYPH_REPEAT_AXES_2D,
		REPEAT_AXES_3D = CMISS_GLYPH_REPEAT_AXES_3D,
		REPEAT_MIRROR = CMISS_GLYPH_REPEAT_MIRROR
	};


	enum Type
	{
		TYPE_INVALID = CMISS_GLYPH_TYPE_INVALID,
		NONE = CMISS_GLYPH_NONE,
		ARROW = CMISS_GLYPH_ARROW,
		ARROW_SOLID = CMISS_GLYPH_ARROW_SOLID,
		AXIS = CMISS_GLYPH_AXIS,
		AXIS_SOLID = CMISS_GLYPH_AXIS_SOLID,
		CONE = CMISS_GLYPH_CONE,
		CONE_SOLID = CMISS_GLYPH_CONE_SOLID,
		CROSS = CMISS_GLYPH_CROSS,
		CUBE_SOLID = CMISS_GLYPH_CUBE_SOLID,
		CUBE_WIREFRAME = CMISS_GLYPH_CUBE_WIREFRAME,
		CYLINDER = CMISS_GLYPH_CYLINDER,
		CYLINDER_SOLID = CMISS_GLYPH_CYLINDER_SOLID,
		DIAMOND = CMISS_GLYPH_DIAMOND,
		LINE = CMISS_GLYPH_LINE,
		POINT = CMISS_GLYPH_POINT,
		SHEET = CMISS_GLYPH_SHEET,
		SPHERE = CMISS_GLYPH_SPHERE,
		AXES = CMISS_GLYPH_AXES,
		AXES_123 = CMISS_GLYPH_AXES_123,
		AXES_XYZ = CMISS_GLYPH_AXES_XYZ,
		AXES_COLOUR = CMISS_GLYPH_AXES_COLOUR,
		AXES_SOLID = CMISS_GLYPH_AXES_SOLID,
		AXES_SOLID_123 = CMISS_GLYPH_AXES_SOLID_123,
		AXES_SOLID_XYZ = CMISS_GLYPH_AXES_SOLID_XYZ,
		AXES_SOLID_COLOUR = CMISS_GLYPH_AXES_SOLID_COLOUR
	};

	char *getName()
	{
		return Cmiss_glyph_get_name(id);
	}

	int setName(const char *name)
	{
		return Cmiss_glyph_set_name(id, name);
	}

	bool isManaged()
	{
		return (0 != Cmiss_glyph_is_managed(id));
	}

	int setManaged(bool value)
	{
		return Cmiss_glyph_set_managed(id, static_cast<int>(value));
	}
};

class GlyphAxes : public Glyph
{
private:
	explicit GlyphAxes(Cmiss_glyph_id glyph_id) : Glyph(glyph_id) {}

	inline Cmiss_glyph_axes_id getDerivedId()
	{
		return reinterpret_cast<Cmiss_glyph_axes_id>(id);
	}

public:
	GlyphAxes() : Glyph(0) {}

	explicit GlyphAxes(Cmiss_glyph_axes_id axes_id)
		: Glyph(reinterpret_cast<Cmiss_glyph_id>(axes_id))
	{}

	GlyphAxes(Glyph& glyph)
		: Glyph(reinterpret_cast<Cmiss_glyph_id>(Cmiss_glyph_cast_axes(glyph.getId())))
	{}

	double getAxisWidth() 
	{
		return Cmiss_glyph_axes_get_axis_width(getDerivedId());
	}

	int setAxisWidth(double axisWidth)
	{
		return Cmiss_glyph_axes_set_axis_width(getDerivedId(), axisWidth);
	}

	char *getAxisLabel(int axisNumber)
	{
		return Cmiss_glyph_axes_get_axis_label(getDerivedId(), axisNumber);
	}

	int setAxisLabel(int axisNumber, const char *label)
	{
		return Cmiss_glyph_axes_set_axis_label(getDerivedId(), axisNumber, label);
	}

	GraphicsMaterial getAxisMaterial(int axisNumber)
	{
		return GraphicsMaterial(Cmiss_glyph_axes_get_axis_material(getDerivedId(), axisNumber));
	}

	int setAxisMaterial(int axisNumber, GraphicsMaterial material)
	{
		return Cmiss_glyph_axes_set_axis_material(getDerivedId(), axisNumber, material.getId());
	}

};

class GlyphColourBar : public Glyph
{
private:
	explicit GlyphColourBar(Cmiss_glyph_id glyph_id) : Glyph(glyph_id) {}

	inline Cmiss_glyph_colour_bar_id getDerivedId()
	{
		return reinterpret_cast<Cmiss_glyph_colour_bar_id>(id);
	}

public:
	GlyphColourBar() : Glyph(0) {}

	explicit GlyphColourBar(Cmiss_glyph_colour_bar_id colour_bar_id)
		: Glyph(reinterpret_cast<Cmiss_glyph_id>(colour_bar_id))
	{}

	GlyphColourBar(Glyph& glyph)
		: Glyph(reinterpret_cast<Cmiss_glyph_id>(Cmiss_glyph_cast_colour_bar(glyph.getId())))
	{}

	int getAxis(int valuesCount, double *valuesOut)
	{
		return Cmiss_glyph_colour_bar_get_axis(getDerivedId(), valuesCount, valuesOut);
	}

	int setAxis(int valuesCount, const double *valuesIn)
	{
		return Cmiss_glyph_colour_bar_set_axis(getDerivedId(), valuesCount, valuesIn);
	}

	int getCentre(int valuesCount, double *valuesOut)
	{
		return Cmiss_glyph_colour_bar_get_centre(getDerivedId(), valuesCount, valuesOut);
	}

	int setCentre(int valuesCount, const double *valuesIn)
	{
		return Cmiss_glyph_colour_bar_set_centre(getDerivedId(), valuesCount, valuesIn);
	}

	double getExtendLength() 
	{
		return Cmiss_glyph_colour_bar_get_extend_length(getDerivedId());
	}

	int setExtendLength(double extendLength)
	{
		return Cmiss_glyph_colour_bar_set_extend_length(getDerivedId(), extendLength);
	}

	int getLabelDivisions() 
	{
		return Cmiss_glyph_colour_bar_get_label_divisions(getDerivedId());
	}

	int setLabelDivisions(int labelDivisions)
	{
		return Cmiss_glyph_colour_bar_set_label_divisions(getDerivedId(), labelDivisions);
	}

	GraphicsMaterial getLabelMaterial()
	{
		return GraphicsMaterial(Cmiss_glyph_colour_bar_get_label_material(getDerivedId()));
	}

	int setLabelMaterial(GraphicsMaterial& material)
	{
		return Cmiss_glyph_colour_bar_set_label_material(getDerivedId(), material.getId());
	}

	char *getNumberFormat()
	{
		return Cmiss_glyph_colour_bar_get_number_format(getDerivedId());
	}

	int setNumberFormat(const char *numberFormat)
	{
		return Cmiss_glyph_colour_bar_set_number_format(getDerivedId(), numberFormat);
	}

	int getSideAxis(int valuesCount, double *valuesOut)
	{
		return Cmiss_glyph_colour_bar_get_side_axis(getDerivedId(), valuesCount, valuesOut);
	}

	int setSideAxis(int valuesCount, const double *valuesIn)
	{
		return Cmiss_glyph_colour_bar_set_side_axis(getDerivedId(), valuesCount, valuesIn);
	}

	double getTickLength() 
	{
		return Cmiss_glyph_colour_bar_get_tick_length(getDerivedId());
	}

	int setTickLength(double tickLength)
	{
		return Cmiss_glyph_colour_bar_set_tick_length(getDerivedId(), tickLength);
	}

};

class GlyphModule
{
protected:
	Cmiss_glyph_module_id id;

public:

	GlyphModule() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit GlyphModule(Cmiss_glyph_module_id in_glyph_module_id) :
		id(in_glyph_module_id)
	{  }

	GlyphModule(const GlyphModule& glyphModule) :
		id(Cmiss_glyph_module_access(glyphModule.id))
	{  }

	GlyphModule& operator=(const GlyphModule& glyphModule)
	{
		Cmiss_glyph_module_id temp_id = Cmiss_glyph_module_access(glyphModule.id);
		if (0 != id)
		{
			Cmiss_glyph_module_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~GlyphModule()
	{
		if (0 != id)
		{
			Cmiss_glyph_module_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Cmiss_glyph_module_id getId()
	{
		return id;
	}

	int beginChange()
	{
		return Cmiss_glyph_module_begin_change(id);
	}

	int endChange()
	{
		return Cmiss_glyph_module_end_change(id);
	}

	GlyphAxes createAxes(Glyph& axisGlyph, double axisWidth)
	{
		return GlyphAxes(Cmiss_glyph_module_create_axes(id, axisGlyph.getId(), axisWidth));
	}

	GlyphColourBar createColourBar(Spectrum& spectrum)
	{
		return GlyphColourBar(Cmiss_glyph_module_create_colour_bar(id, spectrum.getId()));
	}

	int defineStandardGlyphs()
	{
		return Cmiss_glyph_module_define_standard_glyphs(id);
	}

	Glyph findGlyphByName(const char *name)
	{
		return Glyph(Cmiss_glyph_module_find_glyph_by_name(id, name));
	}

	Glyph getDefaultPointGlyph()
	{
		return Glyph(Cmiss_glyph_module_get_default_point_glyph(id));
	}

	int setDefaultPointGlyph(Glyph& glyph)
	{
		return Cmiss_glyph_module_set_default_point_glyph(id, glyph.getId());
	}

};

}  // namespace zinc

#endif /* ZINC_GLYPH_HPP */
