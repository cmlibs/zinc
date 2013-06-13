/***************************************************************************//**
 * FILE : graphicsmodule.hpp
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
#ifndef __ZN_GRAPHICS_MODULE_HPP__
#define __ZN_GRAPHICS_MODULE_HPP__

#include "zinc/graphicsmodule.h"
#include "zinc/region.hpp"
#include "zinc/rendition.hpp"
#include "zinc/glyph.hpp"
#include "zinc/graphic.hpp"
#include "zinc/graphicsmaterial.hpp"
#include "zinc/graphicsfilter.hpp"
#include "zinc/font.hpp"
#include "zinc/scene.hpp"
#include "zinc/spectrum.hpp"
#include "zinc/tessellation.hpp"

namespace zinc
{

class GraphicsModule
{

protected:
	Cmiss_graphics_module_id id;

public:

	GraphicsModule() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit GraphicsModule(Cmiss_graphics_module_id graphics_module_id) :
		id(graphics_module_id)
	{ }

	GraphicsModule(const GraphicsModule& graphicsModule) :
		id(Cmiss_graphics_module_access(graphicsModule.id))
	{ }

	GraphicsModule& operator=(const GraphicsModule& graphicsModule)
	{
		Cmiss_graphics_module_id temp_id = Cmiss_graphics_module_access(graphicsModule.id);
		if (0 != id)
		{
			Cmiss_graphics_module_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~GraphicsModule()
	{
		if (0 != id)
		{
			Cmiss_graphics_module_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Cmiss_graphics_module_id getId()
	{
		return id;
	}
	
	int enableRenditions(Region& region)
	{
		return Cmiss_graphics_module_enable_renditions(id, region.getId());
	}

	Rendition getRendition(Region& region)
	{
		return Rendition(Cmiss_graphics_module_get_rendition(id, region.getId()));
	}

	GlyphModule getGlyphModule()
	{
		return GlyphModule(Cmiss_graphics_module_get_glyph_module(id));
	}

	GraphicsMaterialModule getGraphicsMaterialModule()
	{
		return GraphicsMaterialModule(Cmiss_graphics_module_get_material_module(id));
	}

	GraphicsMaterial createMaterial()
	{
		return GraphicsMaterial(Cmiss_graphics_module_create_material(id));
	}

	int defineStandardMaterials()
	{
		return Cmiss_graphics_module_define_standard_materials(id);
	}

	GraphicsMaterial findMaterialByName(const char *material_name)
	{
		return GraphicsMaterial(Cmiss_graphics_module_find_material_by_name(id, material_name));
	}

	GraphicsFilter createFilterVisibilityFlags()
	{
		return GraphicsFilter(Cmiss_graphics_module_create_filter_visibility_flags(id));
	}

	GraphicsFilter createFilterGraphicName(const char *matchName)
	{
		return GraphicsFilter(Cmiss_graphics_module_create_filter_graphic_name(id, matchName));
	}

	GraphicsFilter createFilterGraphicType(Graphic::GraphicType graphicType)
	{
		return GraphicsFilter(Cmiss_graphics_module_create_filter_graphic_type(id,
			static_cast<Cmiss_graphic_type>(graphicType)));
	}

	GraphicsFilter createFilterRegion(Region& matchRegion)
	{
		return GraphicsFilter(Cmiss_graphics_module_create_filter_region(
			id, matchRegion.getId()));
	}

	GraphicsFilterOperator createFilterOperatorAnd()
	{
		return GraphicsFilterOperator(reinterpret_cast<Cmiss_graphics_filter_operator_id>(
			Cmiss_graphics_module_create_filter_operator_and(id)));
	}

	GraphicsFilterOperator createFilterOperatorOr()
	{
		return GraphicsFilterOperator(reinterpret_cast<Cmiss_graphics_filter_operator_id>(
			Cmiss_graphics_module_create_filter_operator_or(id)));
	}

	GraphicsFilter findFilterByName(const char *name)
	{
		return GraphicsFilter(Cmiss_graphics_module_find_filter_by_name(id, name));
	}

	Scene createScene()
	{
		return Scene(Cmiss_graphics_module_create_scene(id));
	}

	Scene findSceneByName(const char *name)
	{
		return Scene(Cmiss_graphics_module_find_scene_by_name(id, name));
	}

	SpectrumModule getSpectrumModule()
	{
		return SpectrumModule(Cmiss_graphics_module_get_spectrum_module(id));
	}

	Spectrum createSpectrum()
	{
		return Spectrum(Cmiss_graphics_module_create_spectrum(id));
	}

	Spectrum findSpectrumByName(const char *name)
	{
		return Spectrum(Cmiss_graphics_module_find_spectrum_by_name(id, name));
	}

	FontModule getFontModule()
	{
		return FontModule(Cmiss_graphics_module_get_font_module(id));
	}

	Font createFont()
	{
		return Font(Cmiss_graphics_module_create_font(id));
	}

	Font findFontByName(const char *font_name)
	{
		return Font(Cmiss_graphics_module_find_font_by_name(id, font_name));
	}

	TessellationModule getTessellationModule()
	{
		return TessellationModule(Cmiss_graphics_module_get_tessellation_module(id));
	}

	Tessellation createTessellation()
	{
		return Tessellation(Cmiss_graphics_module_create_tessellation(id));
	}

	Tessellation findTessellationByName(const char *name)
	{
		return Tessellation(Cmiss_graphics_module_find_tessellation_by_name(id, name));
	}

};

} // namespace zinc

#endif /* __ZN_GRAPHICS_MODULE_HPP__ */
