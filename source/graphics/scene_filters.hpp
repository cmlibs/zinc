/***************************************************************************//**
 * scene_filters.hpp
 *
 * Declaration of scene graphic filter classes and functions.
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

#ifndef SCENE_FILTERS_HPP
#define SCENE_FILTERS_HPP

extern "C" {
#include "api/cmiss_scene.h"
}

struct Cmiss_rendition;
struct Cmiss_graphic;

class Cmiss_scene_filter
{
private:
	Cmiss_scene_filter_action action;
	bool active;
	Cmiss_scene *scene;
	int access_count;
	
public:
	Cmiss_scene_filter(Cmiss_scene *inScene) :
		action(CMISS_SCENE_FILTER_SHOW),
		active(true),
		scene(inScene),
		access_count(1)
	{
	}

	virtual ~Cmiss_scene_filter()
	{
	}

	virtual bool match(struct Cmiss_graphic *graphic) = 0;
	
	Cmiss_scene_filter_action getAction() const
	{
		return action;
	}
	
	bool setAction(Cmiss_scene_filter_action newAction)
	{
		action = newAction;
		return true;
	}

	bool isActive() const
	{
		return active;
	}

	/** eventually: may fail if filter is only intermediary of an expression */
	bool setActive(bool newActive)
	{
		active = newActive;
		return true;
	}
	
	int access()
	{
		return (++access_count);
	}
	
	int deaccess()
	{
		return (--access_count);
	}
	
	void list() const
	{
		display_message(INFORMATION_MESSAGE, "%s", active ? "active " : "inactive ");
		display_message(INFORMATION_MESSAGE, "%s", (action == CMISS_SCENE_FILTER_SHOW) ? "show " : "hide ");
		list_type_specific();
		display_message(INFORMATION_MESSAGE, "\n");
	}
	
	virtual void list_type_specific() const = 0;
};

#endif /* SCENE_FILTERS_HPP_ */
