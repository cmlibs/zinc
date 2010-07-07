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
#include "general/mystring.h"
#include "graphics/scene.h"
#include "graphics/cmiss_graphic.h"
}
#include "graphics/scene.hpp"
#include "graphics/scene_filters.hpp"

namespace {

class Cmiss_scene_filter_graphic_name : public Cmiss_scene_filter
{
	char *matchName;

public:
	Cmiss_scene_filter_graphic_name(Cmiss_scene *inScene, const char *inMatchName) :
		Cmiss_scene_filter(inScene),
		matchName(duplicate_string(inMatchName))
	{
	}

	virtual bool match(struct Cmiss_graphic *graphic)
	{
		return ::Cmiss_graphic_has_name(graphic, (void *)matchName);
	}
};

class Cmiss_scene_filter_graphic_visibility : public Cmiss_scene_filter
{
	bool matchVisibility;

public:
	Cmiss_scene_filter_graphic_visibility(Cmiss_scene *inScene, bool inMatchVisibility) :
		Cmiss_scene_filter(inScene),
		matchVisibility(inMatchVisibility)
	{
	}

	virtual bool match(struct Cmiss_graphic *graphic)
	{
		bool visibility = Cmiss_graphic_get_visibility(graphic) != 0;
		return (matchVisibility == visibility);
	}
};

class Cmiss_scene_filter_rendition_visibility : public Cmiss_scene_filter
{
	bool matchVisibility;

public:
	Cmiss_scene_filter_rendition_visibility(Cmiss_scene *inScene, bool inMatchVisibility) :
		Cmiss_scene_filter(inScene),
		matchVisibility(inMatchVisibility)
	{
	}

	virtual bool match(struct Cmiss_graphic *graphic)
	{
		Cmiss_rendition *rendition = Cmiss_graphic_get_rendition_private(graphic);
		bool visibility = Cmiss_rendition_get_visibility(rendition) != 0;
		return (matchVisibility == visibility);
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

Cmiss_scene_filter *Cmiss_scene_create_filter_graphic_name(Cmiss_scene *scene, const char *match_name)
{
	Cmiss_scene_filter_graphic_name *filter = new Cmiss_scene_filter_graphic_name(scene, match_name);
	Scene_add_filter(scene, filter);
	return filter;
}

Cmiss_scene_filter *Cmiss_scene_create_filter_graphic_visibility(Cmiss_scene *scene, int match_visibility)
{
	Cmiss_scene_filter_graphic_visibility *filter = new Cmiss_scene_filter_graphic_visibility(scene, match_visibility);
	Scene_add_filter(scene, filter);
	return filter;
}

Cmiss_scene_filter *Cmiss_scene_create_filter_rendition_visibility(Cmiss_scene *scene, int match_visibility)
{
	Cmiss_scene_filter_rendition_visibility *filter = new Cmiss_scene_filter_rendition_visibility(scene, match_visibility);
	Scene_add_filter(scene, filter);
	return filter;
}
