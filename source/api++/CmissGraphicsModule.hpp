/***************************************************************************//**
 * FILE : CmissGraphicsModule.hpp
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
#ifndef __CMISS_GRAPHICS_MODULE_HPP__
#define __CMISS_GRAPHICS_MODULE_HPP__

extern "C" {
#include "api/zn_graphics_module.h"
}
#include "api++/CmissMaterial.hpp"

namespace Cmiss
{

class GraphicsMaterial;

class GraphicsModule
{
protected:
	Cmiss_graphics_module_id id;

public:
	GraphicsModule() : id(NULL)
	{ }

	// takes ownership of C-style graphics_module reference
	GraphicsModule(Cmiss_graphics_module_id graphics_module_id) : 
		id(graphics_module_id)
	{ }

	GraphicsModule(const GraphicsModule& graphicsModule) :
		id(Cmiss_graphics_module_access(graphicsModule.id))
	{ }

	GraphicsModule& operator=(const GraphicsModule& graphicsModule)
	{
		Cmiss_graphics_module_id temp_id = graphicsModule.id;
		if (NULL != id)
		{
			Cmiss_graphics_module_destroy(&id);
		}
		id = temp_id;
		return *this;
	}
	
	~GraphicsModule()
	{
		if (NULL != id)
		{
			Cmiss_graphics_module_destroy(&id);
		}
	}

	GraphicsMaterial createMaterial() const
	{
		return GraphicsMaterial(Cmiss_graphics_module_create_material(id));
	}

	GraphicsMaterial findMaterialByName(const char *material_name) const
	{
		return GraphicsMaterial(Cmiss_graphics_module_find_material_by_name(id, material_name));
	}
	
};

} // namespace Cmiss

#endif /* __CMISS_GRAPHICS_MODULE_HPP__ */
