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
#ifndef CMZN_GRAPHICSMODULE_HPP__
#define CMZN_GRAPHICSMODULE_HPP__

#include "zinc/graphicsmodule.h"
#include "zinc/region.hpp"
#include "zinc/scene.hpp"
#include "zinc/glyph.hpp"
#include "zinc/graphic.hpp"
#include "zinc/graphicsmaterial.hpp"
#include "zinc/graphicsfilter.hpp"
#include "zinc/font.hpp"
#include "zinc/sceneviewer.hpp"
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
	
	Scene getScene(Region& region)
	{
		return Scene(Cmiss_graphics_module_get_scene(id, region.getId()));
	}

	GlyphModule getGlyphModule()
	{
		return GlyphModule(Cmiss_graphics_module_get_glyph_module(id));
	}

	GraphicsMaterialModule getMaterialModule()
	{
		return GraphicsMaterialModule(Cmiss_graphics_module_get_material_module(id));
	}

	GraphicsFilterModule getFilterModule()
	{
		return GraphicsFilterModule(Cmiss_graphics_module_get_filter_module(id));
	}

	SceneViewerModule getSceneViewerModule()
	{
		return SceneViewerModule(Cmiss_graphics_module_get_scene_viewer_module(id));
	}

	SpectrumModule getSpectrumModule()
	{
		return SpectrumModule(Cmiss_graphics_module_get_spectrum_module(id));
	}

	FontModule getFontModule()
	{
		return FontModule(Cmiss_graphics_module_get_font_module(id));
	}

	TessellationModule getTessellationModule()
	{
		return TessellationModule(Cmiss_graphics_module_get_tessellation_module(id));
	}


};

} // namespace zinc

#endif
