/***************************************************************************//**
 * scene_filters.cpp
 *
 * Implementation of scene graphic filters.
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

extern "C" {
#include "api/cmiss_rendition.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "graphics/element_point_ranges.h"
#include "graphics/graphic.h"
#include "graphics/scene.h"
#include "region/cmiss_region.h"
}
#include "graphics/scene.hpp"
#include "graphics/scene_filters.hpp"

namespace {

class Cmiss_scene_filter_all : public Cmiss_scene_filter
{
public:
	Cmiss_scene_filter_all(Cmiss_scene *inScene) :
		Cmiss_scene_filter(inScene)
	{
	}

	virtual bool match(struct Cmiss_graphic *graphic)
	{
		USE_PARAMETER(graphic);
		return (!isInverse());
	}

	virtual void list_type_specific() const
	{
		display_message(INFORMATION_MESSAGE, "match_all");
	}

};

class Cmiss_scene_filter_graphic_name : public Cmiss_scene_filter
{
	char *matchName;

public:
	Cmiss_scene_filter_graphic_name(Cmiss_scene *inScene, const char *inMatchName) :
		Cmiss_scene_filter(inScene),
		matchName(duplicate_string(inMatchName))
	{
	}

	virtual ~Cmiss_scene_filter_graphic_name()
	{
		DEALLOCATE(matchName);
	}

	virtual bool match(struct Cmiss_graphic *graphic)
	{
		return (::Cmiss_graphic_has_name(graphic, (void *)matchName) == !isInverse());
	}

	virtual void list_type_specific() const
	{
		display_message(INFORMATION_MESSAGE, "match_graphic_name %s", matchName);
	}

};

class Cmiss_scene_filter_visibility_flags : public Cmiss_scene_filter
{
public:
	Cmiss_scene_filter_visibility_flags(Cmiss_scene *inScene) :
		Cmiss_scene_filter(inScene)
	{
	}

	virtual bool match(struct Cmiss_graphic *graphic)
	{
		return (!isInverse() == Cmiss_graphic_and_rendition_visibility_flags_set(graphic));
	}

	virtual void list_type_specific() const
	{
		display_message(INFORMATION_MESSAGE, "match_visibility_flags");
	}
};

class Cmiss_scene_filter_region : public Cmiss_scene_filter
{
	Cmiss_region *matchRegion;

public:
	Cmiss_scene_filter_region(Cmiss_scene *inScene, Cmiss_region *inRegion) :
		Cmiss_scene_filter(inScene), matchRegion(inRegion)
	{
	}

	virtual bool match(struct Cmiss_graphic *graphic)
	{
		return (!isInverse() == Cmiss_graphic_is_from_region(graphic, matchRegion));
	}

	virtual void list_type_specific() const
	{
		char *region_name = Cmiss_region_get_path(matchRegion);
		display_message(INFORMATION_MESSAGE, "match_region_path %s", region_name);
		DEALLOCATE(region_name);
	}
};

}

Cmiss_scene_filter *Cmiss_scene_filter_access(Cmiss_scene_filter *filter)
{
	if (filter)
		filter->access();
	return filter;
}

int Cmiss_scene_filter_destroy(Cmiss_scene_filter **filter_address)
{
	if (filter_address && (*filter_address))
	{
		if (0 == (*filter_address)->deaccess())
		{
			delete (*filter_address);
		}
		*filter_address = 0;
	}
	return 0;
}

enum Cmiss_scene_filter_action Cmiss_scene_filter_get_action(
	Cmiss_scene_filter *filter)
{
	if (filter)
		return filter->getAction();
	return CMISS_SCENE_FILTER_HIDE;
}

int Cmiss_scene_filter_set_action(Cmiss_scene_filter *filter,
	enum Cmiss_scene_filter_action action)
{
	if (filter)
		return filter->setAction(action);
	return 0;
}

int Cmiss_scene_filter_is_active(Cmiss_scene_filter *filter)
{
	if (filter)
		return filter->isActive();
	return 0;
}

int Cmiss_scene_filter_set_active(Cmiss_scene_filter *filter,
	int active_flag)
{
	if (filter)
		return filter->setActive(active_flag);
	return 0;
}

int Cmiss_scene_filter_is_inverse_match(Cmiss_scene_filter *filter)
{
	if (filter)
		return filter->isInverse();
	return 0;
}

int Cmiss_scene_filter_set_inverse_match(Cmiss_scene_filter_id filter,
	int inverse_match_flag)
{
	if (filter)
		return filter->setInverse(inverse_match_flag);
	return 0;
}

Cmiss_scene_filter *Cmiss_scene_create_filter_all(Cmiss_scene *scene)
{
	Cmiss_scene_filter_all *filter = new Cmiss_scene_filter_all(scene);
	Scene_add_filter_private(scene, filter);
	return filter;
}

Cmiss_scene_filter *Cmiss_scene_create_filter_graphic_name(Cmiss_scene *scene, const char *match_name)
{
	Cmiss_scene_filter_graphic_name *filter = new Cmiss_scene_filter_graphic_name(scene, match_name);
	Scene_add_filter_private(scene, filter);
	return filter;
}

Cmiss_scene_filter *Cmiss_scene_create_filter_visibility_flags(Cmiss_scene *scene)
{
	Cmiss_scene_filter_visibility_flags *filter = new Cmiss_scene_filter_visibility_flags(scene);
	Scene_add_filter_private(scene, filter);
	return filter;
}

Cmiss_scene_filter *Cmiss_scene_create_filter_region(Cmiss_scene *scene, Cmiss_region *match_region)
{
	Cmiss_scene_filter_region *filter = new Cmiss_scene_filter_region(scene, match_region);
	Scene_add_filter_private(scene, filter);
	return filter;
}

