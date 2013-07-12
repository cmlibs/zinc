/***************************************************************************//**
 * graphics_filter.hpp
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

#ifndef GRAPHICS_FILTER_HPP
#define GRAPHICS_FILTER_HPP

#include "zinc/graphicsmodule.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/mystring.h"
#include "general/object.h"
#include "general/debug.h"
#include "general/cmiss_set.hpp"

struct Cmiss_scene;
struct Cmiss_graphic;

enum Cmiss_graphics_filter_type
{
	CMISS_GRAPHICS_FILTER_TYPE_INVALID,
	CMISS_GRAPHICS_FILTER_TYPE_BASE,
	CMISS_GRAPHICS_FILTER_TYPE_IDENTIFIER ,
	CMISS_GRAPHICS_FILTER_TYPE_DOMAIN_TYPE,
	CMISS_GRAPHICS_FILTER_TYPE_GRAPHIC_NAME,
	CMISS_GRAPHICS_FILTER_TYPE_GRAPHIC_TYPE,
	CMISS_GRAPHICS_FILTER_TYPE_VISIBILITY_FLAGS ,
	CMISS_GRAPHICS_FILTER_TYPE_REGION,
	CMISS_GRAPHICS_FILTER_TYPE_OPERATOR,
	CMISS_GRAPHICS_FILTER_TYPE_OPERATOR_AND,
	CMISS_GRAPHICS_FILTER_TYPE_OPERATOR_OR
};

DECLARE_LIST_TYPES(Cmiss_graphics_filter);

DECLARE_MANAGER_TYPES(Cmiss_graphics_filter);

PROTOTYPE_OBJECT_FUNCTIONS(Cmiss_graphics_filter);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Cmiss_graphics_filter);

PROTOTYPE_LIST_FUNCTIONS(Cmiss_graphics_filter);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Cmiss_graphics_filter,name,const char *);

PROTOTYPE_MANAGER_FUNCTIONS(Cmiss_graphics_filter);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Cmiss_graphics_filter,name,const char *);

struct Cmiss_graphics_filter_change_detail
{
	virtual ~Cmiss_graphics_filter_change_detail()
	{
	}
};

struct Cmiss_graphics_filter
{
private:
	bool inverse;

public:
	enum Cmiss_graphics_filter_type filter_type;
	const char *name;
	int access_count;
	struct MANAGER(Cmiss_graphics_filter) *manager;
	int manager_change_status;
	bool is_managed_flag;


	Cmiss_graphics_filter() :
		inverse(false),
		filter_type(CMISS_GRAPHICS_FILTER_TYPE_BASE),
		name(NULL),
		access_count(1),
		manager(NULL),
		manager_change_status(MANAGER_CHANGE_NONE(Cmiss_graphics_filter)),
		is_managed_flag(false)
	{
	}

	virtual ~Cmiss_graphics_filter()
	{
		DEALLOCATE(name);
	}

	virtual bool match(struct Cmiss_graphic *graphic) = 0;

	bool setName(const char *name_in)
	{
		char *new_name = duplicate_string(name_in);
		if (!new_name)
			return false;
		if (name)
			DEALLOCATE(name);
		name = new_name;
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

	int changed()
	{
		return MANAGED_OBJECT_CHANGE(Cmiss_graphics_filter)(this,
			MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Cmiss_graphics_filter));
	}

	bool isInverse() const
	{
		return inverse;
	}

	bool setInverse(bool newInverse)
	{
		inverse = newInverse;
		changed();
		return true;
	}

	inline Cmiss_graphics_filter *access()
	{
		++access_count;
		return this;
	}

	enum Cmiss_graphics_filter_type getType()
	{
		return filter_type;
	};

	void list(const char *prefix) const
	{
		display_message(INFORMATION_MESSAGE, "%s %s %s", prefix, name, inverse ? "inverse_match " : "normal_match ");
		list_type_specific();
		display_message(INFORMATION_MESSAGE, ";\n");
	}

	virtual void list_type_specific() const = 0;

	virtual int check_dependency()
	{
		if (manager_change_status & MANAGER_CHANGE_RESULT(Cmiss_graphics_filter))
		{
			return 1;
		}
		return 0;
	}

	/** clones and clears type-specific change detail, if any.
	 * override for classes with type-specific change detail
	 * @return  change detail prior to clearing, or NULL if none.
	 */
	virtual Cmiss_graphics_filter_change_detail *extract_change_detail()
	{
		return NULL;
	}

	/**
	 * override for filter types with that are functions of other filters to
	 * prevent circular dependencies / infinite loops.
	 * @return  true if this filter depends on other_filter, false if not.
	 */
	virtual bool depends_on_filter(const Cmiss_graphics_filter *other_filter) const
	{
		return (other_filter == this);
	}

	static inline int deaccess(Cmiss_graphics_filter **graphics_filter_address)
	{
		return DEACCESS(Cmiss_graphics_filter)(graphics_filter_address);
	}
};

/* Only to be used from FIND_BY_IDENTIFIER_IN_INDEXED_LIST_STL function
 * Creates a pseudo object with name identifier suitable for finding
 * objects by identifier with Cmiss_set.
 */
class Cmiss_graphics_filter_identifier : private Cmiss_graphics_filter
{
public:
	Cmiss_graphics_filter_identifier(const char *name)
	{
		// const_cast OK as must never be modified & cleared in destructor
		Cmiss_graphics_filter::name = name;
		filter_type = CMISS_GRAPHICS_FILTER_TYPE_IDENTIFIER;
	}

	virtual ~Cmiss_graphics_filter_identifier()
	{
		Cmiss_graphics_filter::name = NULL;
	}

	virtual bool match(struct Cmiss_graphic *graphic)
	{
		USE_PARAMETER(graphic);
		return false;
	}

	virtual void list_type_specific() const
	{
	}

	Cmiss_graphics_filter *getPseudoObject()
	{
		return this;
	}
};

/** functor for ordering Cmiss_set<Cmiss_graphics_filter> by name */
struct Cmiss_graphics_filter_compare_name
{
	bool operator() (const Cmiss_graphics_filter * graphics_filter1, const Cmiss_graphics_filter * graphics_filter2) const
	{
		return strcmp(graphics_filter1->name, graphics_filter2->name) < 0;
	}
};

typedef Cmiss_set<Cmiss_graphics_filter *,Cmiss_graphics_filter_compare_name> Cmiss_set_Cmiss_graphics_filter;

int Cmiss_graphics_filter_manager_set_owner_private(struct MANAGER(Cmiss_graphics_filter) *manager,
	struct Cmiss_graphics_filter_module *graphics_filter_module);

struct MANAGER(Cmiss_graphics_filter) *Cmiss_graphics_filter_module_get_manager(
	struct Cmiss_graphics_filter_module *graphics_filter_module);

Cmiss_graphics_filter_module_id Cmiss_graphics_filter_module_create();

#endif /* GRAPHICS_FILTER_HPP_ */
