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

#include "api/cmiss_graphic.h"
#include "api++/field.hpp"
#include "api++/graphicsmaterial.hpp"
#include "api++/tessellation.hpp"

namespace Zn
{

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

	enum RenderType
	{
		RENDER_TYPE_INVALID = CMISS_GRAPHICS_RENDER_TYPE_INVALID,
		RENDER_TYPE_SHADED = CMISS_GRAPHICS_RENDER_TYPE_SHADED,
		RENDER_TYPE_WIREFRAME = CMISS_GRAPHICS_RENDER_TYPE_WIREFRAME,
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
		COORDINATE_SYSTEM_WINDOW_PIXEL_TOP_LEFT = CMISS_GRAPHICS_COORDINATE_SYSTEM_WINDOW_PIXEL_TOP_LEFT,
	};

	enum GlyphType
	{
		GLYPH_TYPE_INVALID = CMISS_GRAPHIC_GLYPH_TYPE_INVALID,
		GLYPH_TYPE_POINT = CMISS_GRAPHIC_GLYPH_POINT,
		GLYPH_TYPE_AXES = CMISS_GRAPHIC_GLYPH_AXES
	};

	enum GraphicType
	{
		GRAPHIC_TYPE_INVALID = CMISS_GRAPHIC_TYPE_INVALID ,
		GRAPHIC_NODE_POINTS = CMISS_GRAPHIC_NODE_POINTS,
		GRAPHIC_DATA_POINTS = CMISS_GRAPHIC_DATA_POINTS,
		GRAPHIC_LINES = CMISS_GRAPHIC_LINES,
		GRAPHIC_CYLINDERS = CMISS_GRAPHIC_CYLINDERS,
		GRAPHIC_SURFACES = CMISS_GRAPHIC_SURFACES,
		GRAPHIC_ISO_SURFACES = CMISS_GRAPHIC_ISO_SURFACES,
		GRAPHIC_ELEMENT_POINTS = CMISS_GRAPHIC_ELEMENT_POINTS,
		GRAPHIC_STREAMLINES = CMISS_GRAPHIC_STREAMLINES,
		GRAPHIC_POINT = CMISS_GRAPHIC_POINT,
	};

	Cmiss_graphic_id getId()
	{
		return id;
	}

	int setCoordinateField(Field& coordinateField)
	{
		return Cmiss_graphic_set_coordinate_field(id, coordinateField.getId());
	}

	int setMaterial(GraphicsMaterial& graphicsMaterial)
	{
		return Cmiss_graphic_set_material(id, graphicsMaterial.getId());
	}

	int setSelectedMaterial(GraphicsMaterial& graphicsMaterial)
	{
		return Cmiss_graphic_set_selected_material(id, graphicsMaterial.getId());
	}

	Tessellation getTessellation()
	{
		return Tessellation(Cmiss_graphic_get_tessellation(id));
	}

	int setTessellation(Tessellation& tessellation)
	{
		return Cmiss_graphic_set_tessellation(id, tessellation.getId());
	}

	enum RenderType getRenderType()
	{
		return static_cast<RenderType>(Cmiss_graphic_get_render_type(id));
	}

	int setRenderType(RenderType renderType)
	{
		return Cmiss_graphic_set_render_type(id,
			static_cast<Cmiss_graphics_render_type>(renderType));
	}

	bool getVisibilityFlag()
	{
		return Cmiss_graphic_get_visibility_flag(id);
	}

	int setVisibilityFlag(bool visibilityFlag)
	{
		return Cmiss_graphic_set_visibility_flag(id, (int)visibilityFlag);
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

	char *getName()
	{
		return Cmiss_graphic_get_name(id);
	}

	int setName(const char *name)
	{
		return Cmiss_graphic_set_name(id, name);
	}

	int setGlyphType(GlyphType type)
	{
		return Cmiss_graphic_set_glyph_type(id,
			static_cast<Cmiss_graphic_glyph_type>(type));
	 }

};

} // namespace Cmiss

#endif /* __ZN_CMISS_GRAPHIC_HPP__ */
