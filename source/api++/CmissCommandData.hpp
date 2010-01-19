/***************************************************************************//**
 * FILE : CmissCommandData.hpp
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
 * The Original Code is cmgui.
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
#ifndef __CMISS_COMMAND_DATA_HPP__
#define __CMISS_COMMAND_DATA_HPP__

extern "C" {
#include "api/cmiss_command_data.h"
}

namespace Cmiss
{

class Region;
class GraphicsModule;

class CommandData
{
private:
	Cmiss_command_data_id id;

public:
	// Creates a new Cmiss CommandData instance
	CommandData(int argc, const char *argv[], const char *version_string) :
		id(Cmiss_command_data_create(argc, argv, version_string))
	{ }

	// temporary constructor to use from SWIG
	CommandData(const char *appName) :
		id(Cmiss_command_data_create(1, &appName, "0.0"))
	{ }

	CommandData() : id(NULL)
	{ }

	// takes ownership of C-style command data reference
	CommandData(Cmiss_command_data_id command_data_id) :
		id(command_data_id)
	{ }

	CommandData(const CommandData& commandData) :
		id(Cmiss_command_data_access(commandData.id))
	{ }

	CommandData& operator=(const CommandData& commandData)
	{
		Cmiss_command_data_id temp_id = Cmiss_command_data_access(commandData.id);
		if (NULL != id)
		{
			Cmiss_command_data_destroy(&id);
		}
		id = temp_id;
		return *this;
	}
	
	~CommandData()
	{
		if (NULL != id)
		{
			Cmiss_command_data_destroy(&id);
		}
	}

	bool isValid() const
	{
		return (NULL != id);
	}

	int executeCommand(const char *command) const
	{
		return Cmiss_command_data_execute_command(id, command);
	}

	template<class RegionType> RegionType getRootRegion() const
	{
		return Region(Cmiss_command_data_get_root_region(id));
	}

	template<class GraphicsModuleType> GraphicsModuleType getDefaultGraphicsModule() const
	{
		return GraphicsModule(Cmiss_command_data_get_graphics_module(id));
	}

	int runMainLoop() const
	{
		return Cmiss_command_data_main_loop(id);
	}
};

} // namespace Cmiss

#endif /* __CMISS_COMMAND_DATA_HPP__ */
