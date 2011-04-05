/***************************************************************************//**
 * graphic_filter.hpp
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

#ifndef GRAPHIC_FILTER_HPP
#define GRAPHIC_FILTER_HPP

extern "C" {
#include "api/cmiss_scene.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
}

struct Cmiss_rendition;
struct Cmiss_graphic;

enum Cmiss_graphic_filter_type
{
	CMISS_GRAPHIC_FILTER_TYPE_INVALID = 0,
	CMISS_GRAPHIC_FILTER_TYPE_BASE = 1,
	CMISS_GRAPHIC_FILTER_TYPE_IDENTIFIER = 2,
	CMISS_GRAPHIC_FILTER_TYPE_ALL = 3,
	CMISS_GRAPHIC_FILTER_TYPE_GRAPHIC_NAME = 4,
	CMISS_GRAPHIC_FILTER_TYPE_VISIBILITY_FLAGS = 5,
	CMISS_GRAPHIC_FILTER_TYPE_REGION = 6,
	CMISS_GRAPHIC_FILTER_TYPE_AND = 7,
	CMISS_GRAPHIC_FILTER_TYPE_OR = 8
};

DECLARE_LIST_TYPES(Cmiss_graphic_filter);

DECLARE_MANAGER_TYPES(Cmiss_graphic_filter);

PROTOTYPE_OBJECT_FUNCTIONS(Cmiss_graphic_filter);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Cmiss_graphic_filter);

PROTOTYPE_LIST_FUNCTIONS(Cmiss_graphic_filter);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Cmiss_graphic_filter,name,const char *);

PROTOTYPE_MANAGER_FUNCTIONS(Cmiss_graphic_filter);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Cmiss_graphic_filter,name,const char *);


class Cmiss_graphic_filter
{
private:
	bool active;
	bool inverse;
	bool show_matching;

public:
	enum Cmiss_graphic_filter_type filter_type;
	char *name;
	int access_count;
	struct MANAGER(Cmiss_graphic_filter) *manager;
	int manager_change_status;
	bool is_managed_flag;


	Cmiss_graphic_filter() :
		active(true),
		inverse(false),
		show_matching(true),
		filter_type(CMISS_GRAPHIC_FILTER_TYPE_BASE),
		name(NULL),
		access_count(1),
		manager(NULL),
		manager_change_status(MANAGER_CHANGE_NONE(Cmiss_graphic_filter)),
		is_managed_flag(false)
	{
	}

	virtual ~Cmiss_graphic_filter()
	{
		DEALLOCATE(name);
	}

	virtual bool match(struct Cmiss_graphic *graphic) = 0;
	
	bool isActive() const
	{
		return active;
	}

	bool setName(const char *name_in)
	{
		if (name)
			DEALLOCATE(name);
		name = duplicate_string(name_in);
		return true;
	}

	bool getShowMatching()
	{
		return show_matching;
	}

	bool setShowMatching(bool show_matching_in)
	{
		show_matching = show_matching_in;
		return true;
	}

	char *getName()
	{
		char *name_out = NULL;
		if (name)
		{
			name_out = duplicate_string(name);
		}

		return name_out;
	}

	/** eventually: may fail if filter is only intermediary of an expression */
	bool setActive(bool newActive)
	{
		active = newActive;
		return true;
	}
	
	bool isInverse() const
	{
		return inverse;
	}

	bool setInverse(bool newInverse)
	{
		inverse = newInverse;
		return true;
	}

	inline Cmiss_graphic_filter *access()
	{
		++access_count;
		return this;
	}
	
	enum Cmiss_graphic_filter_type getType()
	{
		return filter_type;
	};

	void list() const
	{
		display_message(INFORMATION_MESSAGE, "%s", active ? "active " : "inactive ");
		display_message(INFORMATION_MESSAGE, "%s", (show_matching == true) ? "show " : "hide ");
		display_message(INFORMATION_MESSAGE, "%s", inverse ? "inverse_match " : "normal_match ");
		list_type_specific();
		display_message(INFORMATION_MESSAGE, "\n");
	}

	virtual void list_type_specific() const = 0;

	virtual int add(Cmiss_graphic_filter *graphic_filter)
	{
		USE_PARAMETER(graphic_filter);
		return 0;
	}

	virtual int remove(Cmiss_graphic_filter *graphic_filter)
	{
		USE_PARAMETER(graphic_filter);
		return 0;
	}

	static inline int deaccess(Cmiss_graphic_filter **graphic_filter_address)
	{
		return DEACCESS(Cmiss_graphic_filter)(graphic_filter_address);
	}
};

int Cmiss_graphic_filter_manager_set_owner_private(struct MANAGER(Cmiss_graphic_filter) *manager,
	struct Cmiss_graphics_module *graphics_module);

int gfx_define_graphic_filter(struct Parse_state *state, void *root_region_void,
	void *graphics_module_void);

int set_Cmiss_graphic_filter(struct Parse_state *state,
	void *graphic_filter_address_void, void *graphics_module_void);
#endif /* GRAPHIC_FILTER_HPP_ */
