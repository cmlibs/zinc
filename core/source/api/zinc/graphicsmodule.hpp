/***************************************************************************//**
 * FILE : graphicsmodule.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_GRAPHICSMODULE_HPP__
#define CMZN_GRAPHICSMODULE_HPP__

#include "zinc/graphicsmodule.h"
#include "zinc/font.hpp"
#include "zinc/glyph.hpp"
#include "zinc/graphic.hpp"
#include "zinc/graphicsmaterial.hpp"
#include "zinc/region.hpp"
#include "zinc/scene.hpp"
#include "zinc/scenefilter.hpp"
#include "zinc/sceneviewer.hpp"
#include "zinc/spectrum.hpp"
#include "zinc/tessellation.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class GraphicsModule
{

protected:
	cmzn_graphics_module_id id;

public:

	GraphicsModule() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit GraphicsModule(cmzn_graphics_module_id graphics_module_id) :
		id(graphics_module_id)
	{ }

	GraphicsModule(const GraphicsModule& graphicsModule) :
		id(cmzn_graphics_module_access(graphicsModule.id))
	{ }

	GraphicsModule& operator=(const GraphicsModule& graphicsModule)
	{
		cmzn_graphics_module_id temp_id = cmzn_graphics_module_access(graphicsModule.id);
		if (0 != id)
		{
			cmzn_graphics_module_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~GraphicsModule()
	{
		if (0 != id)
		{
			cmzn_graphics_module_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_graphics_module_id getId()
	{
		return id;
	}
	
	Scene getScene(Region& region)
	{
		return Scene(cmzn_graphics_module_get_scene(id, region.getId()));
	}

	Glyphmodule getGlyphmodule()
	{
		return Glyphmodule(cmzn_graphics_module_get_glyphmodule(id));
	}

	GraphicsMaterialModule getMaterialModule()
	{
		return GraphicsMaterialModule(cmzn_graphics_module_get_material_module(id));
	}

	Scenefiltermodule getScenefiltermodule()
	{
		return Scenefiltermodule(cmzn_graphics_module_get_scenefiltermodule(id));
	}

	Sceneviewermodule getSceneviewermodule()
	{
		return Sceneviewermodule(cmzn_graphics_module_get_sceneviewermodule(id));
	}

	Spectrummodule getSpectrummodule()
	{
		return Spectrummodule(cmzn_graphics_module_get_spectrummodule(id));
	}

	Fontmodule getFontmodule()
	{
		return Fontmodule(cmzn_graphics_module_get_fontmodule(id));
	}

	Tessellationmodule getTessellationmodule()
	{
		return Tessellationmodule(cmzn_graphics_module_get_tessellationmodule(id));
	}


};

} // namespace Zinc
}

#endif
