/**
 * @file context.hpp
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_CONTEXT_HPP__
#define CMZN_CONTEXT_HPP__

#include "cmlibs/zinc/context.h"

/**
 * \brief The CMLibs namespace
 *
 * This is the CMLibs namespace
 */
namespace CMLibs
{
/**
 * \brief The CMLibs::Zinc namespace
 *
 * This is the CMLibs::Zinc namespace, all libZinc cpp APIs are in this namespace.
 */
namespace Zinc
{
class Fontmodule;
class Glyphmodule;
class Lightmodule;
class Logger;
class Materialmodule;
class Region;
class Scenefiltermodule;
class Sceneviewermodule;
class Shadermodule;
class Spectrummodule;
class Tessellationmodule;
class Timekeepermodule;

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

	bool isValid() const
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

	char* getName() const
	{
		return cmzn_context_get_name(id);
	}

	int getVersion(int *versionOut3) const
	{
		return cmzn_context_get_version(id, versionOut3);
	}

	const char* getRevision() const
	{
		return cmzn_context_get_revision(id);
	}

	char *getVersionString() const
	{
		return cmzn_context_get_version_string(id);
	}

	inline Region createRegion();

	inline Region getDefaultRegion() const;

	inline int setDefaultRegion(const Region& region);

	inline Fontmodule getFontmodule() const;

	inline Glyphmodule getGlyphmodule() const;

	inline Lightmodule getLightmodule() const;

	inline Logger getLogger() const;

	inline Materialmodule getMaterialmodule() const;

	inline Scenefiltermodule getScenefiltermodule() const;

	inline Sceneviewermodule getSceneviewermodule() const;

	inline Shadermodule getShadermodule() const;

	inline Spectrummodule getSpectrummodule() const;

	inline Tessellationmodule getTessellationmodule() const;

	inline Timekeepermodule getTimekeepermodule() const;

};

inline bool operator==(const Context& a, const Context& b)
{
	return a.getId() == b.getId();
}

}  // namespace Zinc
}

#endif /* CMZN_CONTEXT_HPP__ */
