/***************************************************************************//**
 * FILE : context.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_CONTEXT_HPP__
#define CMZN_CONTEXT_HPP__

#include "zinc/context.h"
#include "zinc/font.hpp"
#include "zinc/glyph.hpp"
#include "zinc/material.hpp"
#include "zinc/region.hpp"
#include "zinc/sceneviewer.hpp"
#include "zinc/spectrum.hpp"
#include "zinc/tessellation.hpp"
#include "zinc/timekeeper.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class Context
{
private:

	cmzn_context_id id;

public:

	Context() : id(0)
	{ }
	// Creates a new zinc Context instance
	Context(const char *contextName) :
		id(cmzn_context_create(contextName))
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit Context(cmzn_context_id context_id) :
		id(context_id)
	{ }

	Context(const Context& context) :
		id(cmzn_context_access(context.id))
	{ }

	~Context()
	{
		if (0 != id)
		{
			cmzn_context_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Context& operator=(const Context& context)
	{
		cmzn_context_id temp_id = cmzn_context_access(context.id);
		if (0 != id)
		{
			cmzn_context_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	cmzn_context_id getId()
	{
		return id;
	}

	Region getDefaultRegion()
	{
		return Region(cmzn_context_get_default_region(id));
	}

	Region createRegion()
	{
		return Region(cmzn_context_create_region(id));
	}

	Timekeeper getDefaultTimekeeper()
	{
		return Timekeeper(cmzn_context_get_default_timekeeper(id));
	}

	Sceneviewermodule getSceneviewermodule()
	{
		return Sceneviewermodule(cmzn_context_get_sceneviewermodule(id));
	}

	Glyphmodule getGlyphmodule()
	{
		return Glyphmodule(cmzn_context_get_glyphmodule(id));
	}

	Materialmodule getMaterialmodule()
	{
		return Materialmodule(cmzn_context_get_materialmodule(id));
	}

	Scenefiltermodule getScenefiltermodule()
	{
		return Scenefiltermodule(cmzn_context_get_scenefiltermodule(id));
	}

	Spectrummodule getSpectrummodule()
	{
		return Spectrummodule(cmzn_context_get_spectrummodule(id));
	}

	Fontmodule getFontmodule()
	{
		return Fontmodule(cmzn_context_get_fontmodule(id));
	}

	Tessellationmodule getTessellationmodule()
	{
		return Tessellationmodule(cmzn_context_get_tessellationmodule(id));
	}

};

}  // namespace Zinc
}

#endif /* CMZN_CONTEXT_HPP__ */
