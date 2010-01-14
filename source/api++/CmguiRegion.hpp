/***************************************************************************//**
 * FILE : CmguiRegion.hpp
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
#ifndef __CMGUI_REGION_HPP__
#define __CMGUI_REGION_HPP__

extern "C" {
#include "api/cmiss_region.h"
}

namespace Cmgui
{

class FieldModule;

class Region
{
protected:
	Cmiss_region_id id;

public:
	Region() : id(NULL)
	{ }

	// takes ownership of C-style region reference
	Region(Cmiss_region_id in_region_id) : id(in_region_id)
	{ }

	Region(const Region& region) : id(Cmiss_region_access(region.id))
	{ }

	Region& operator=(const Region& region)
	{
		Cmiss_region_id temp_id = Cmiss_region_access(region.id);
		if (NULL != id)
		{
			Cmiss_region_destroy(&id);
		}
		id = temp_id;
		return *this;
	}
	
	~Region()
	{
		if (NULL != id)
		{
			Cmiss_region_destroy(&id);
		}
	}
	
	bool isValid() const
	{
		return (NULL != id);
	}

	template<class FieldModuleType> FieldModuleType getFieldModule() const
	{
		return FieldModule(Cmiss_region_get_field_module(id));
	}

	int readFile(const char *fileName)
	{
		return Cmiss_region_read_file(id, fileName);
	}

};

} // namespace Cmgui

#endif /* __CMGUI_REGION_HPP__ */
