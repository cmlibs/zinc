/***************************************************************************//**
 * FILE : scene.hpp
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
#ifndef __ZN_SCENE_HPP__
#define __ZN_SCENE_HPP__

#include "zinc/scene.h"
#include "zinc/graphicsfilter.hpp"
#include "zinc/region.hpp"

namespace zinc
{

class ScenePicker;

class Scene
{
protected:
	Cmiss_scene_id id;

public:

	Scene() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Scene(Cmiss_scene_id in_scene_id) :
		id(in_scene_id)
	{  }

	Scene(const Scene& scene) :
		id(Cmiss_scene_access(scene.id))
	{  }

	Scene& operator=(const Scene& scene)
	{
		Cmiss_scene_id temp_id = Cmiss_scene_access(scene.id);
		if (0 != id)
		{
			Cmiss_scene_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Scene()
	{
		if (0 != id)
		{
			Cmiss_scene_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Cmiss_scene_id getId()
	{
		return id;
	}

	enum Attribute
	{
		ATTRIBUTE_INVALID = CMISS_SCENE_ATTRIBUTE_INVALID,
		ATTRIBUTE_IS_MANAGED = CMISS_SCENE_ATTRIBUTE_IS_MANAGED
	};

	int getAttributeInteger(Attribute attribute)
	{
		return Cmiss_scene_get_attribute_integer(id,
			static_cast<Cmiss_scene_attribute>(attribute));
	}

	int setAttributeInteger(Attribute attribute, int value)
	{
		return Cmiss_scene_set_attribute_integer(id,
			static_cast<Cmiss_scene_attribute>(attribute), value);
	}

	int setRegion(Region& rootRegion)
	{
		return Cmiss_scene_set_region(id, rootRegion.getId());
	}

	char *getName()
	{
		return Cmiss_scene_get_name(id);
	}

	int setName(const char *name)
	{
		return Cmiss_scene_set_name(id, name);
	}

	GraphicsFilter getFilter()
	{
		return GraphicsFilter(Cmiss_scene_get_filter(id));
	}

	int setFilter(GraphicsFilter& filter)
	{
		return Cmiss_scene_set_filter(id, filter.getId());
	}

	ScenePicker createPicker();

};

}  // namespace zinc

#endif /* __ZN_SCENE_HPP__ */
