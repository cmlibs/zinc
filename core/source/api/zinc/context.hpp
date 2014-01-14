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

namespace OpenCMISS
{
namespace Zinc
{
class Fontmodule;
class Glyphmodule;
class Materialmodule;
class Region;
class Scenefiltermodule;
class Sceneviewermodule;
class Spectrummodule;
class Tessellationmodule;
class Timekeeper;

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

	cmzn_context_id getId() const
	{
		return id;
	}

	inline Fontmodule getFontmodule();

	inline Glyphmodule getGlyphmodule();

	inline Materialmodule getMaterialmodule();

	inline Region createRegion();

	inline Region getDefaultRegion();

	inline Scenefiltermodule getScenefiltermodule();

	inline Sceneviewermodule getSceneviewermodule();

	inline Spectrummodule getSpectrummodule();

	inline Tessellationmodule getTessellationmodule();

	inline Timekeeper getDefaultTimekeeper();

};

}  // namespace Zinc
}

#endif /* CMZN_CONTEXT_HPP__ */
