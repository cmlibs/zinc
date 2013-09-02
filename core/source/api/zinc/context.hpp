/***************************************************************************//**
 * FILE : context.hpp
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
 * Portions created by the Initial Developer are Copyright (C) 2010
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

#ifndef CMZN_CONTEXT_HPP__
#define CMZN_CONTEXT_HPP__

#include "zinc/context.h"
#include "zinc/region.hpp"
#include "zinc/graphicsmodule.hpp"
#include "zinc/timekeeper.hpp"
#include "zinc/sceneviewer.hpp"

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

	GraphicsModule getGraphicsModule()
	{
		return GraphicsModule(cmzn_context_get_graphics_module(id));
	}

	TimeKeeper getDefaultTimeKeeper()
	{
		return TimeKeeper(cmzn_context_get_default_time_keeper(id));
	}

};

}  // namespace Zinc
}

#endif /* CMZN_CONTEXT_HPP__ */
