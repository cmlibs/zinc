/***************************************************************************//**
 * graphic_filter.cpp
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

#include <vector>
#include <iterator>
extern "C" {
#include "api/cmiss_rendition.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/manager_private.h"
#include "graphics/element_point_ranges.h"
#include "graphics/graphic.h"
#include "graphics/graphics_module.h"
#include "region/cmiss_region.h"
}
#include "general/cmiss_set.hpp"
#include "general/indexed_list_stl_private.hpp"
#include "graphics/scene.hpp"
#include "graphics/graphic_filter.hpp"

namespace {

typedef std::vector<Cmiss_graphic_filter*> Graphic_filter_vector;

class Cmiss_graphic_filter_and : public Cmiss_graphic_filter
{
	Graphic_filter_vector *filters;
public:
	Cmiss_graphic_filter_and() :
		filters(new Graphic_filter_vector())
	{
		filter_type = CMISS_GRAPHIC_FILTER_TYPE_AND;
	}

	virtual ~Cmiss_graphic_filter_and()
	{
		int number_of_filters = filters->size();
		for (int i = 0; i < number_of_filters; i++)
		{
			Cmiss_graphic_filter *filter = filters->at(i);
			Cmiss_graphic_filter_destroy(&filter);
		}
		filters->clear();
		delete filters;
	}

	virtual bool match(struct Cmiss_graphic *graphic)
	{
		int number_of_filters = filters->size();
		int return_code = 1;
		for (int i = 0; i < number_of_filters; i++)
		{
			Cmiss_graphic_filter *filter = filters->at(i);
			if (!filter->match(graphic))
			{
				return_code = 0;
				break;
			}
		}
		return (!isInverse() == return_code);
	}

	virtual void list_type_specific() const
	{
		display_message(INFORMATION_MESSAGE, "match_and");
	}

	virtual int add(Cmiss_graphic_filter *graphic_filter)
	{
		int return_code = 1;
		if (graphic_filter != this)
		{
			Graphic_filter_vector::iterator pos;
			pos = find(filters->begin(), filters->end(), graphic_filter);
			if (pos == filters->end())
				filters->push_back(graphic_filter->access());
		}
		else
		{
			return_code = 0;
		}
		return return_code;
	}

	virtual int remove(Cmiss_graphic_filter *graphic_filter)
	{
		int return_code = 1;
		if (graphic_filter != this)
		{
			Graphic_filter_vector::iterator pos;
			pos = find(filters->begin(), filters->end(), graphic_filter);
			if (pos != filters->end())
			{
				Cmiss_graphic_filter *filter_to_remove = *pos;
				Cmiss_graphic_filter_destroy(&filter_to_remove);
				filters->erase(pos);
			}
		}
		else
		{
			return_code = 0;
		}
		return return_code;
	}

};

class Cmiss_graphic_filter_or : public Cmiss_graphic_filter
{
	Graphic_filter_vector *filters;
public:
	Cmiss_graphic_filter_or() :
		filters(new Graphic_filter_vector())
	{
		filter_type = CMISS_GRAPHIC_FILTER_TYPE_OR;
	}

	virtual ~Cmiss_graphic_filter_or()
	{
		int number_of_filters = filters->size();
		for (int i = 0; i < number_of_filters; i++)
		{
			Cmiss_graphic_filter *filter = filters->at(i);
			Cmiss_graphic_filter_destroy(&filter);
		}
		filters->clear();
		delete filters;
	}

	virtual bool match(struct Cmiss_graphic *graphic)
	{

		int number_of_filters = filters->size();
		int return_code = 0;
		for (int i = 0; i < number_of_filters; i++)
		{
			Cmiss_graphic_filter *filter = filters->at(i);
			if (filter->match(graphic))
			{
				return_code = 1;
				break;
			}
		}
		return (!isInverse() == return_code);
	}

	virtual void list_type_specific() const
	{
		display_message(INFORMATION_MESSAGE, "match_or");
	}

	virtual int add(Cmiss_graphic_filter *graphic_filter)
	{
		int return_code = 1;
		if (graphic_filter != this)
		{
			Graphic_filter_vector::iterator pos;
			pos = find(filters->begin(), filters->end(), graphic_filter);
			if (pos == filters->end())
				filters->push_back(graphic_filter->access());
		}
		else
		{
			return_code = 0;
		}
		return return_code;
	}

	virtual int remove(Cmiss_graphic_filter *graphic_filter)
	{
		int return_code = 1;
		if (graphic_filter != this)
		{
			Graphic_filter_vector::iterator pos;
			pos = find(filters->begin(), filters->end(), graphic_filter);
			if (pos != filters->end())
			{
				Cmiss_graphic_filter *filter_to_remove = *pos;
				Cmiss_graphic_filter_destroy(&filter_to_remove);
				filters->erase(pos);
			}
		}
		else
		{
			return_code = 0;
		}
		return return_code;
	}


};

class Cmiss_graphic_filter_all : public Cmiss_graphic_filter
{
public:
	Cmiss_graphic_filter_all()
	{
		filter_type = CMISS_GRAPHIC_FILTER_TYPE_ALL;
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

class Cmiss_graphic_filter_graphic_name : public Cmiss_graphic_filter
{
	char *matchName;

public:
	Cmiss_graphic_filter_graphic_name(const char *inMatchName) :
		matchName(duplicate_string(inMatchName))
	{
		filter_type = CMISS_GRAPHIC_FILTER_TYPE_GRAPHIC_NAME;
	}

	virtual ~Cmiss_graphic_filter_graphic_name()
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

class Cmiss_graphic_filter_visibility_flags : public Cmiss_graphic_filter
{
public:
	Cmiss_graphic_filter_visibility_flags()
	{
		filter_type = CMISS_GRAPHIC_FILTER_TYPE_VISIBILITY_FLAGS;
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

class Cmiss_graphic_filter_region : public Cmiss_graphic_filter
{
	Cmiss_region *matchRegion;

public:
	Cmiss_graphic_filter_region(Cmiss_region *inRegion) :
		matchRegion(inRegion)
	{
		filter_type = CMISS_GRAPHIC_FILTER_TYPE_REGION;
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

Cmiss_graphic_filter *Cmiss_graphic_filter_access(Cmiss_graphic_filter *filter)
{
	if (filter)
		filter->access();
	return filter;
}

int Cmiss_graphic_filter_destroy(Cmiss_graphic_filter **filter_address)
{
	return DEACCESS(Cmiss_graphic_filter)(filter_address);
}

int Cmiss_graphic_filter_is_active(Cmiss_graphic_filter *filter)
{
	if (filter)
		return filter->isActive();
	return 0;
}

int Cmiss_graphic_filter_set_active(Cmiss_graphic_filter *filter,
	int active_flag)
{
	if (filter)
		return filter->setActive(active_flag);
	return 0;
}

int Cmiss_graphic_filter_is_inverse_match(Cmiss_graphic_filter *filter)
{
	if (filter)
		return filter->isInverse();
	return 0;
}

int Cmiss_graphic_filter_set_inverse_match(Cmiss_graphic_filter_id filter,
	int inverse_match_flag)
{
	if (filter)
		return filter->setInverse(inverse_match_flag);
	return 0;
}

/* Only to be used from FIND_BY_IDENTIFIER_IN_INDEXED_LIST_STL function
 * Creates a pseudo object with name identifier suitable for finding
 * objects by identifier with Cmiss_set.
 */
class Cmiss_graphic_filter_identifier : private Cmiss_graphic_filter
{
public:
	Cmiss_graphic_filter_identifier(const char *name)
	{
		// const_cast OK as must never be modified & cleared in destructor
		Cmiss_graphic_filter::name = const_cast<char *>(name);
		filter_type = CMISS_GRAPHIC_FILTER_TYPE_IDENTIFIER;
	}

	virtual ~Cmiss_graphic_filter_identifier()
	{
		Cmiss_graphic_filter::name = NULL;
	}

	virtual bool match(struct Cmiss_graphic *graphic)
	{
		USE_PARAMETER(graphic);
		return false;
	}

	virtual void list_type_specific() const
	{
	}

	Cmiss_graphic_filter *getPseudoObject()
	{
		return this;
	}
};

/** functor for ordering Cmiss_set<Cmiss_graphic_filter> by name */
struct Cmiss_graphic_filter_compare_name
{
	bool operator() (const Cmiss_graphic_filter * graphic_filter1, const Cmiss_graphic_filter * graphic_filter2) const
	{
		return strcmp(graphic_filter1->name, graphic_filter2->name) < 0;
	}
};

typedef Cmiss_set<Cmiss_graphic_filter *,Cmiss_graphic_filter_compare_name> Cmiss_set_Cmiss_graphic_filter;

FULL_DECLARE_MANAGER_TYPE_WITH_OWNER(Cmiss_graphic_filter, Cmiss_graphics_module, void *);

namespace {

/***************************************************************************//**
 * Frees the memory for the graphic_filter of <*graphic_filter_address>.
 * Sets *graphic_filter_address to NULL.
 */
int DESTROY(Cmiss_graphic_filter)(struct Cmiss_graphic_filter **graphic_filter_address)
{
	int return_code;

	ENTER(DESTROY(Cmiss_graphic_filter));
	if (graphic_filter_address && (*graphic_filter_address))
	{
		delete *graphic_filter_address;
		*graphic_filter_address = NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Cmiss_graphic_filter).  Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

DECLARE_LOCAL_MANAGER_FUNCTIONS(Cmiss_graphic_filter)

} /* anonymous namespace */

DECLARE_ACCESS_OBJECT_FUNCTION(Cmiss_graphic_filter)

/***************************************************************************//**
 * Custom version handling is_managed_flag.
 */
PROTOTYPE_DEACCESS_OBJECT_FUNCTION(Cmiss_graphic_filter)
{
	int return_code;
	Cmiss_graphic_filter *object;

	ENTER(DEACCESS(Cmiss_graphic_filter));
	if (object_address && (object = *object_address))
	{
		(object->access_count)--;
		if (object->access_count <= 0)
		{
			delete object;
			return_code = 1;
		}
		else if ((!object->is_managed_flag) && (object->manager) &&
			((1 == object->access_count) || ((2 == object->access_count) &&
				(MANAGER_CHANGE_NONE(Cmiss_graphic_filter) != object->manager_change_status))))
		{
			return_code =
				REMOVE_OBJECT_FROM_MANAGER(Cmiss_graphic_filter)(object, object->manager);
		}
		else
		{
			return_code = 1;
		}
		*object_address = (struct Cmiss_graphic_filter *)NULL;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DEACCESS(Cmiss_graphic_filter) */

PROTOTYPE_REACCESS_OBJECT_FUNCTION(Cmiss_graphic_filter)
{
	int return_code;

	ENTER(REACCESS(Cmiss_graphic_filter));
	if (object_address)
	{
		return_code = 1;
		if (new_object)
		{
			/* access the new object */
			(new_object->access_count)++;
		}
		if (*object_address)
		{
			/* deaccess the current object */
			DEACCESS(Cmiss_graphic_filter)(object_address);
		}
		/* point to the new object */
		*object_address = new_object;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"REACCESS(Cmiss_graphic_filter).  Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* REACCESS(Cmiss_graphic_filter) */

DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Cmiss_graphic_filter)

DECLARE_INDEXED_LIST_STL_FUNCTIONS(Cmiss_graphic_filter)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_STL_FUNCTION(Cmiss_graphic_filter,name,const char *)

DECLARE_MANAGER_FUNCTIONS(Cmiss_graphic_filter,manager)
DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Cmiss_graphic_filter,manager)
DECLARE_MANAGER_IDENTIFIER_WITHOUT_MODIFY_FUNCTIONS(Cmiss_graphic_filter,name,const char *,manager)
DECLARE_MANAGER_OWNER_FUNCTIONS(Cmiss_graphic_filter, struct Cmiss_graphics_module)

int Cmiss_graphic_filter_manager_set_owner_private(struct MANAGER(Cmiss_graphic_filter) *manager,
	struct Cmiss_graphics_module *graphics_module)
{
	return MANAGER_SET_OWNER(Cmiss_graphic_filter)(manager, graphics_module);
}

char *Cmiss_graphic_filter_get_name(Cmiss_graphic_filter_id graphic_filter)
{
	char *name = NULL;

	if (graphic_filter)
	{
		name = graphic_filter->getName();
	}
	return name;
}

int Cmiss_graphic_filter_set_name(Cmiss_graphic_filter_id graphic_filter, const char *name)
{
	int return_code;

	ENTER(Cmiss_graphic_filter_set_name);
	if (graphic_filter && is_standard_object_name(name))
	{
		return_code = 1;
		Cmiss_set_Cmiss_graphic_filter *manager_graphic_filter_list = 0;
		bool restore_changed_object_to_lists = false;
		if (graphic_filter->manager)
		{
			if (FIND_BY_IDENTIFIER_IN_MANAGER(Cmiss_graphic_filter, name)(
				name, graphic_filter->manager))
			{
				display_message(ERROR_MESSAGE, "Cmiss_graphic_filter_set_name.  "
					"graphic_filter named '%s' already exists.", name);
				return_code = 0;
			}
			if (return_code)
			{
				manager_graphic_filter_list = reinterpret_cast<Cmiss_set_Cmiss_graphic_filter *>(
					graphic_filter->manager->object_list);
				// this temporarily removes the object from all related lists
				restore_changed_object_to_lists =
					manager_graphic_filter_list->begin_identifier_change(graphic_filter);
				if (!restore_changed_object_to_lists)
				{
					display_message(ERROR_MESSAGE, "Cmiss_graphic_filter_set_name.  "
						"Could not safely change identifier in manager");
					return_code = 0;
				}
			}
		}
		if (return_code)
		{
			if (name)
			{
				graphic_filter->setName(name);
			}
			else
			{
				return_code = 0;
			}
		}
		if (restore_changed_object_to_lists)
		{
			manager_graphic_filter_list->end_identifier_change();
		}
		if (graphic_filter->manager && return_code)
		{
			MANAGED_OBJECT_CHANGE(Cmiss_graphic_filter)(graphic_filter,
				MANAGER_CHANGE_IDENTIFIER(Cmiss_graphic_filter));
		}
	}
	else
	{
		if (graphic_filter)
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_graphic_filter_set_name.  Invalid graphic_filter name '%s'", name);
		}
		return_code=0;
	}

	return (return_code);
}

int Cmiss_graphic_filter_add(Cmiss_graphic_filter_id graphic_filter, Cmiss_graphic_filter_id filter_to_add)
{
	int return_code = 0;
	if (graphic_filter && filter_to_add)
	{
		return_code = graphic_filter->add(filter_to_add);
		if (return_code)
		{
			MANAGED_OBJECT_CHANGE(Cmiss_graphic_filter)(
				graphic_filter, MANAGER_CHANGE_RESULT(Cmiss_graphic_filter));
		}
	}
	return return_code;
}

int Cmiss_graphic_filter_remove(Cmiss_graphic_filter_id graphic_filter, Cmiss_graphic_filter_id filter_to_remove)
{
	int return_code = 0;
	if (graphic_filter && filter_to_remove)
	{
		return_code = graphic_filter->remove(filter_to_remove);
		if (return_code)
		{
			MANAGED_OBJECT_CHANGE(Cmiss_graphic_filter)(
				graphic_filter, MANAGER_CHANGE_RESULT(Cmiss_graphic_filter));
		}
	}
	return return_code;
}

char *get_valid_temporary_name_for_graphic_filter(
	Cmiss_graphics_module_id graphics_module)
{
	char *name = NULL;
	if (graphics_module)
	{
		struct MANAGER(Cmiss_graphic_filter) *graphic_filter_manager =
			Cmiss_graphics_module_get_graphic_filter_manager(graphics_module);
		char temp_name[20];
		int i = NUMBER_IN_MANAGER(Cmiss_graphic_filter)(graphic_filter_manager);
		do
		{
			i++;
			sprintf(temp_name, "temp%d",i);
		}
		while (FIND_BY_IDENTIFIER_IN_MANAGER(Cmiss_graphic_filter,name)(temp_name,
			graphic_filter_manager));
		name = duplicate_string(temp_name);
	}
	return name;
}


Cmiss_graphic_filter_id Cmiss_graphics_module_create_graphic_filter_visibility_flags(
	Cmiss_graphics_module_id graphics_module)
{
	Cmiss_graphic_filter_id graphic_filter = NULL;
	if (graphics_module)
	{
		char *name = get_valid_temporary_name_for_graphic_filter(graphics_module);
		struct MANAGER(Cmiss_graphic_filter) *graphic_filter_manager =
			Cmiss_graphics_module_get_graphic_filter_manager(graphics_module);
		graphic_filter = new Cmiss_graphic_filter_visibility_flags();
		Cmiss_graphic_filter_set_name(graphic_filter, name);
		if (!ADD_OBJECT_TO_MANAGER(Cmiss_graphic_filter)(graphic_filter, graphic_filter_manager))
		{
			DEACCESS(Cmiss_graphic_filter)(&graphic_filter);
		}
		DEALLOCATE(name);
	}
	return graphic_filter;
}

Cmiss_graphic_filter_id Cmiss_graphics_module_create_graphic_filter_all(
	Cmiss_graphics_module_id graphics_module)
{
	Cmiss_graphic_filter_id graphic_filter = NULL;
	if (graphics_module)
	{
		char *name = get_valid_temporary_name_for_graphic_filter(graphics_module);
		struct MANAGER(Cmiss_graphic_filter) *graphic_filter_manager =
			Cmiss_graphics_module_get_graphic_filter_manager(graphics_module);
		graphic_filter = new Cmiss_graphic_filter_all();
		Cmiss_graphic_filter_set_name(graphic_filter, name);
		if (!ADD_OBJECT_TO_MANAGER(Cmiss_graphic_filter)(graphic_filter, graphic_filter_manager))
		{
			DEACCESS(Cmiss_graphic_filter)(&graphic_filter);
		}
		DEALLOCATE(name);
	}
	return graphic_filter;
}

Cmiss_graphic_filter_id Cmiss_graphics_module_create_graphic_filter_graphic_name(
	Cmiss_graphics_module_id graphics_module, const char *match_name)
{
	Cmiss_graphic_filter_id graphic_filter = NULL;
	if (graphics_module && match_name)
	{
		char *name = get_valid_temporary_name_for_graphic_filter(graphics_module);
		struct MANAGER(Cmiss_graphic_filter) *graphic_filter_manager =
			Cmiss_graphics_module_get_graphic_filter_manager(graphics_module);
		graphic_filter = new Cmiss_graphic_filter_graphic_name(match_name);
		Cmiss_graphic_filter_set_name(graphic_filter, name);
		if (!ADD_OBJECT_TO_MANAGER(Cmiss_graphic_filter)(graphic_filter, graphic_filter_manager))
		{
			DEACCESS(Cmiss_graphic_filter)(&graphic_filter);
		}
		DEALLOCATE(name);
	}
	return graphic_filter;
}

Cmiss_graphic_filter_id Cmiss_graphics_module_create_graphic_filter_region(
	Cmiss_graphics_module_id graphics_module, Cmiss_region_id match_region)
{
	Cmiss_graphic_filter_id graphic_filter = NULL;
	if (graphics_module && match_region)
	{
		char *name = get_valid_temporary_name_for_graphic_filter(graphics_module);
		struct MANAGER(Cmiss_graphic_filter) *graphic_filter_manager =
			Cmiss_graphics_module_get_graphic_filter_manager(graphics_module);
		graphic_filter =  new Cmiss_graphic_filter_region(match_region);
		Cmiss_graphic_filter_set_name(graphic_filter, name);
		if (!ADD_OBJECT_TO_MANAGER(Cmiss_graphic_filter)(graphic_filter, graphic_filter_manager))
		{
			DEACCESS(Cmiss_graphic_filter)(&graphic_filter);
		}
		DEALLOCATE(name);
	}
	return graphic_filter;
}

Cmiss_graphic_filter_id Cmiss_graphics_module_create_graphic_filter_and(
	Cmiss_graphics_module_id graphics_module)
{
	Cmiss_graphic_filter_id graphic_filter = NULL;
	if (graphics_module)
	{
		char *name = get_valid_temporary_name_for_graphic_filter(graphics_module);
		struct MANAGER(Cmiss_graphic_filter) *graphic_filter_manager =
			Cmiss_graphics_module_get_graphic_filter_manager(graphics_module);
		graphic_filter = new Cmiss_graphic_filter_and();
		Cmiss_graphic_filter_set_name(graphic_filter, name);
		if (!ADD_OBJECT_TO_MANAGER(Cmiss_graphic_filter)(graphic_filter, graphic_filter_manager))
		{
			DEACCESS(Cmiss_graphic_filter)(&graphic_filter);
		}
		DEALLOCATE(name);
	}
	return graphic_filter;
}

Cmiss_graphic_filter_id Cmiss_graphics_module_create_graphic_filter_or(
	Cmiss_graphics_module_id graphics_module)
{
	Cmiss_graphic_filter_id graphic_filter = NULL;
	if (graphics_module)
	{
		char *name = get_valid_temporary_name_for_graphic_filter(graphics_module);
		struct MANAGER(Cmiss_graphic_filter) *graphic_filter_manager =
			Cmiss_graphics_module_get_graphic_filter_manager(graphics_module);
		graphic_filter = new Cmiss_graphic_filter_or();
		Cmiss_graphic_filter_set_name(graphic_filter, name);
		if (!ADD_OBJECT_TO_MANAGER(Cmiss_graphic_filter)(graphic_filter, graphic_filter_manager))
		{
			DEACCESS(Cmiss_graphic_filter)(&graphic_filter);
		}
		DEALLOCATE(name);
	}
	return graphic_filter;
}

enum Cmiss_graphic_filter_type Cmiss_graphic_filter_get_type(Cmiss_graphic_filter_id graphic_filter)
{
	enum Cmiss_graphic_filter_type filter_type = CMISS_GRAPHIC_FILTER_TYPE_INVALID;
	if (graphic_filter)
	{
		filter_type = graphic_filter->getType();
	}
	return filter_type;
}

struct Define_graphic_filter_data
{
	Cmiss_region *root_region;
	Cmiss_graphics_module *graphics_module;
};

int gfx_define_graphic_filter_match_or(struct Parse_state *state, void *graphic_filter_handle_void,
	void *filter_data_void)
{
	int return_code = 1;
	enum Cmiss_graphic_filter_type filter_type;
	struct Define_graphic_filter_data *filter_data = (struct Define_graphic_filter_data *)filter_data_void;
	Cmiss_graphic_filter *filter_to_add = NULL, *filter_to_remove = NULL;
	if (state && filter_data)
	{
		Cmiss_graphic_filter_id *graphic_filter_handle = (Cmiss_graphic_filter_id *)graphic_filter_handle_void; // can be null
		Cmiss_graphic_filter_id graphic_filter = *graphic_filter_handle;
		if (graphic_filter)
		{
			filter_type = Cmiss_graphic_filter_get_type(graphic_filter);
		}
		else
		{
			graphic_filter = Cmiss_graphics_module_create_graphic_filter_or(
				filter_data->graphics_module);
			filter_type = Cmiss_graphic_filter_get_type(graphic_filter);
		}
		struct Option_table *option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table, "add_filter", &filter_to_add,
			filter_data->graphics_module, set_Cmiss_graphic_filter);
		Option_table_add_entry(option_table, "remove_filter", &filter_to_remove,
			filter_data->graphics_module, set_Cmiss_graphic_filter);
		return_code = Option_table_multi_parse(option_table, state);
		if (return_code && filter_type == CMISS_GRAPHIC_FILTER_TYPE_OR )
		{
			if (filter_to_add)
			{
				Cmiss_graphic_filter_add(graphic_filter, filter_to_add);
			}
			if (filter_to_remove)
			{
				Cmiss_graphic_filter_remove(graphic_filter, filter_to_remove);
			}
		}
		else
		{
			return_code = 0;
		}
		DESTROY(Option_table)(&option_table);
		if (filter_to_add)
			DEACCESS(Cmiss_graphic_filter)(&filter_to_add);
		if (filter_to_remove)
			DEACCESS(Cmiss_graphic_filter)(&filter_to_remove);
		if (graphic_filter)
			*graphic_filter_handle = graphic_filter;
	}

	return return_code;
}

int gfx_define_graphic_filter_match_and(struct Parse_state *state, void *graphic_filter_handle_void,
	void *filter_data_void)
{
	int return_code = 1;
	enum Cmiss_graphic_filter_type filter_type;
	struct Define_graphic_filter_data *filter_data = (struct Define_graphic_filter_data *)filter_data_void;
	Cmiss_graphic_filter *filter_to_add = NULL, *filter_to_remove = NULL;
	if (state && filter_data)
	{
		Cmiss_graphic_filter_id *graphic_filter_handle = (Cmiss_graphic_filter_id *)graphic_filter_handle_void; // can be null
		Cmiss_graphic_filter_id graphic_filter = *graphic_filter_handle;
		if (graphic_filter)
		{
			filter_type = Cmiss_graphic_filter_get_type(graphic_filter);
		}
		else
		{
			graphic_filter = Cmiss_graphics_module_create_graphic_filter_and(
				filter_data->graphics_module);
			filter_type = Cmiss_graphic_filter_get_type(graphic_filter);
		}
		struct Option_table *option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table, "add_filter", &filter_to_add,
			filter_data->graphics_module, set_Cmiss_graphic_filter);
		Option_table_add_entry(option_table, "remove_filter", &filter_to_remove,
			filter_data->graphics_module, set_Cmiss_graphic_filter);
		return_code = Option_table_multi_parse(option_table, state);
		if (return_code && filter_type == CMISS_GRAPHIC_FILTER_TYPE_AND)
		{
			if (filter_to_add)
			{
				Cmiss_graphic_filter_add(graphic_filter, filter_to_add);
			}
			if (filter_to_remove)
			{
				Cmiss_graphic_filter_remove(graphic_filter, filter_to_remove);
			}
		}
		else
		{
			return_code = 0;
		}
		DESTROY(Option_table)(&option_table);
		if (filter_to_add)
			DEACCESS(Cmiss_graphic_filter)(&filter_to_add);
		if (filter_to_remove)
			DEACCESS(Cmiss_graphic_filter)(&filter_to_remove);
		if (graphic_filter)
			*graphic_filter_handle = graphic_filter;
	}

	return return_code;
}

int gfx_define_graphic_filter_contents(struct Parse_state *state, void *graphic_filter_handle_void,
	void *filter_data_void)
{
	int return_code = 1;
	enum Cmiss_graphic_filter_type filter_type;
	struct Define_graphic_filter_data *filter_data = (struct Define_graphic_filter_data *)filter_data_void;
	char match_all, *match_graphic_name, match_visibility_flags, *match_region_path;
	int inverse, show;

	if (state && filter_data)
	{
		Cmiss_graphic_filter_id *graphic_filter_handle = (Cmiss_graphic_filter_id *)graphic_filter_handle_void; // can be null
		Cmiss_graphic_filter_id graphic_filter = *graphic_filter_handle;
		if (graphic_filter)
		{
			filter_type = Cmiss_graphic_filter_get_type(graphic_filter);
		}
		else
		{
			filter_type = CMISS_GRAPHIC_FILTER_TYPE_INVALID;
		}
		match_all = 0;
		match_graphic_name = NULL;
		match_visibility_flags = 0;
		match_region_path = NULL;
		inverse = 0;
		show = 1;
		if (graphic_filter)
		{
			inverse = graphic_filter->isInverse();
			show = (int)graphic_filter->getShowMatching();
		}

		struct Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table," Filter to set up what will be and "
				"what will not be included in a scene. The optional inverse_match "
				"flag will invert the filter's match criterion. The default "
				"behaviour is to show matching graphic unless the optional hide flag "
				"is specified.");
		Option_table_add_char_flag_entry(option_table, "match_all",
			&(match_all));
		Option_table_add_entry(option_table, "match_or", graphic_filter_handle_void, filter_data_void,
			gfx_define_graphic_filter_match_or);
		Option_table_add_entry(option_table, "match_and", graphic_filter_handle_void, filter_data_void,
			gfx_define_graphic_filter_match_and);
		Option_table_add_string_entry(option_table, "match_graphic_name",
			&(match_graphic_name), " MATCH_NAME");
		Option_table_add_char_flag_entry(option_table, "match_visibility_flags",
			&(match_visibility_flags));
		Option_table_add_string_entry(option_table, "match_region_path",
			&(match_region_path), " REGION_PATH");
		Option_table_add_switch(option_table, "inverse_match", "normal_match",
			&(inverse));
		Option_table_add_switch(option_table,
			"show", "hide", &(show));
		return_code = Option_table_multi_parse(option_table, state);
		if (return_code)
		{
			int number_of_match_criteria = match_all + match_visibility_flags +
				(NULL != match_region_path) +	(NULL != match_graphic_name);
			if (1 < number_of_match_criteria)
			{
				display_message(ERROR_MESSAGE,
					"Only one match criterion can be specified per filter.");
				display_parse_state_location(state);
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			if (!graphic_filter)
			{
				if (match_all)
				{
					graphic_filter = Cmiss_graphics_module_create_graphic_filter_all(
						filter_data->graphics_module);
				}
				else if (match_visibility_flags)
				{
					graphic_filter = Cmiss_graphics_module_create_graphic_filter_visibility_flags(
						filter_data->graphics_module);
				}
				else if (match_graphic_name)
				{
					graphic_filter = Cmiss_graphics_module_create_graphic_filter_graphic_name(
						filter_data->graphics_module, match_graphic_name);
				}
				else if (match_region_path)
				{
					Cmiss_region *match_region = Cmiss_region_find_subregion_at_path(
						filter_data->root_region, match_region_path);
					if (match_region)
					{
						graphic_filter = Cmiss_graphics_module_create_graphic_filter_region(
							filter_data->graphics_module, match_region);
						Cmiss_region_destroy(&match_region);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Cannot create filter region.  %s is not in region tree",
							match_region_path);
					}
				}
			}

			if (graphic_filter)
			{
				Cmiss_graphic_filter_set_attribute_integer(graphic_filter,
					CMISS_GRAPHIC_FILTER_ATTRIBUTE_SHOW_MATCHING,	show);
				Cmiss_graphic_filter_set_inverse_match(graphic_filter, inverse);
				*graphic_filter_handle = graphic_filter;
			}
		}
		if (match_graphic_name)
			DEALLOCATE(match_graphic_name);
		if (match_region_path)
			DEALLOCATE(match_region_path);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_define_graphic_filter_contents.  Invalid arguments");
		return_code = 0;
	}
	return return_code;
}

int gfx_define_graphic_filter(struct Parse_state *state, void *root_region_void,
	void *graphics_module_void)
{
	int return_code = 1;
	Cmiss_graphic_filter *graphic_filter = NULL;
	Cmiss_graphics_module *graphics_module = (Cmiss_graphics_module *)graphics_module_void;
	Cmiss_region *root_region = (Cmiss_region *)root_region_void;
	if (state && graphics_module && root_region)
	{
		const char *current_token = state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				MANAGER(Cmiss_graphic_filter) *graphic_filter_manager =
					Cmiss_graphics_module_get_graphic_filter_manager(graphics_module);
				MANAGER_BEGIN_CACHE(Cmiss_graphic_filter)(graphic_filter_manager);
				graphic_filter = Cmiss_graphics_module_find_graphic_filter_by_name(graphics_module, current_token);
				shift_Parse_state(state,1);
				Cmiss_graphic_filter **graphic_filter_address = &graphic_filter;
				struct Define_graphic_filter_data filter_data;
				filter_data.graphics_module = graphics_module;
				filter_data.root_region = root_region;
				if (!graphic_filter)
				{
					return_code = gfx_define_graphic_filter_contents(state, (void *)graphic_filter_address, (void*)&filter_data);
					graphic_filter = *graphic_filter_address;
					Cmiss_graphic_filter_set_name(graphic_filter, current_token);
					Cmiss_graphic_filter_set_attribute_integer(graphic_filter, CMISS_GRAPHIC_FILTER_ATTRIBUTE_IS_MANAGED, 1);
				}
				else
				{
					return_code = gfx_define_graphic_filter_contents(state, (void *)graphic_filter_address, (void*)&filter_data);
				}
				Cmiss_graphic_filter_destroy(&graphic_filter);
				MANAGER_END_CACHE(Cmiss_graphic_filter)(graphic_filter_manager);
			}
			else
			{
				Option_table *option_table = CREATE(Option_table)();
				Option_table_add_entry(option_table, "GRAPHIC_FILTER_NAME",
					/*graphic_filter*/(void *)&graphic_filter, root_region_void,
					gfx_define_graphic_filter_contents);
				return_code = Option_table_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing graphic_filter name");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_define_graphic_filter.  Invalid arguments");
		return_code = 0;
	}
	return return_code;
}

int set_Cmiss_graphic_filter(struct Parse_state *state,
	void *graphic_filter_address_void, void *graphics_module_void)
{
	int return_code = 1;

	Cmiss_graphic_filter **graphic_filter_address = (Cmiss_graphic_filter **)graphic_filter_address_void;
	Cmiss_graphics_module *graphics_module = (Cmiss_graphics_module *)graphics_module_void;
	if (state && graphic_filter_address && graphics_module)
	{
		const char *current_token = state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				Cmiss_graphic_filter *graphic_filter = NULL;
				if (!fuzzy_string_compare(current_token, "NONE"))
				{
					graphic_filter = Cmiss_graphics_module_find_graphic_filter_by_name(graphics_module, current_token);
					if (!graphic_filter)
					{
						display_message(ERROR_MESSAGE, "Unknown graphic_filter : %s", current_token);
						display_parse_state_location(state);
						return_code = 0;
					}
				}
				if (return_code)
				{
					REACCESS(Cmiss_graphic_filter)(graphic_filter_address, graphic_filter);
					shift_Parse_state(state,1);
				}
				if (graphic_filter)
					Cmiss_graphic_filter_destroy(&graphic_filter);
			}
			else
			{
				display_message(INFORMATION_MESSAGE," GRAPHIC_FILTER_NAME|none[%s]",
					(*graphic_filter_address) ? (*graphic_filter_address)->name : "none");
			}
		}
		else
		{
			display_message(WARNING_MESSAGE, "Missing graphic_filter name");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		return_code = 0;
	}
	return (return_code);
}

int Cmiss_graphic_filter_get_attribute_integer(Cmiss_graphic_filter_id graphic_filter,
	enum Cmiss_graphic_filter_attribute_id attribute_id)
{
	int value = 0;
	if (graphic_filter)
	{
		switch (attribute_id)
		{
			case CMISS_GRAPHIC_FILTER_ATTRIBUTE_IS_MANAGED:
			value = (int)graphic_filter->is_managed_flag;
			break;
			case CMISS_GRAPHIC_FILTER_ATTRIBUTE_SHOW_MATCHING:
			value = (int)graphic_filter->getShowMatching();
			break;
		default:
			display_message(ERROR_MESSAGE,
				"Cmiss_graphic_filter_get_attribute_integer.  Invalid attribute");
			break;
		}
	}
	return value;
}

int Cmiss_graphic_filter_set_attribute_integer(Cmiss_graphic_filter_id graphic_filter,
	enum Cmiss_graphic_filter_attribute_id attribute_id, int value)
{
	int return_code = 0;
	if (graphic_filter)
	{
		return_code = 1;
		int old_value =
			Cmiss_graphic_filter_get_attribute_integer(graphic_filter, attribute_id);
		enum MANAGER_CHANGE(Cmiss_graphic_filter) change =
			MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Cmiss_graphic_filter);
		switch (attribute_id)
		{
		case CMISS_GRAPHIC_FILTER_ATTRIBUTE_IS_MANAGED:
			graphic_filter->is_managed_flag = (value != 0);
			change = MANAGER_CHANGE_NOT_RESULT(Cmiss_graphic_filter);
			break;
		case CMISS_GRAPHIC_FILTER_ATTRIBUTE_SHOW_MATCHING:
			graphic_filter->setShowMatching(value != 0);
			change = MANAGER_CHANGE_RESULT(Cmiss_graphic_filter);
			break;
		default:
			display_message(ERROR_MESSAGE,
				"Cmiss_graphic_filter_set_attribute_integer.  Invalid attribute");
			return_code = 0;
			break;
		}
		if (Cmiss_graphic_filter_get_attribute_integer(graphic_filter, attribute_id) != old_value)
		{
			MANAGED_OBJECT_CHANGE(Cmiss_graphic_filter)(graphic_filter, change);
		}
	}
	return return_code;
}

