/***************************************************************************//**
 * graphics_filter.cpp
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

#include <list>
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
#include "general/enumerator_conversion.hpp"
#include "general/indexed_list_stl_private.hpp"
#include "graphics/scene.hpp"
#include "graphics/graphics_filter.hpp"

class Cmiss_graphics_filter_operator : public Cmiss_graphics_filter
{
protected:

	struct Graphics_filter_operand
	{
		Cmiss_graphics_filter *filter;
		bool isActive;

		Graphics_filter_operand(Cmiss_graphics_filter *filter) :
			filter(Cmiss_graphics_filter_access(filter)),
			isActive(true)
		{
		}

		~Graphics_filter_operand()
		{
			Cmiss_graphics_filter_destroy(&filter);
		}
	};

	typedef std::list<Graphics_filter_operand*> OperandList;

	OperandList operands;

	OperandList::iterator find_operand(Cmiss_graphics_filter *operand)
	{
		for (OperandList::iterator pos = operands.begin(); pos != operands.end(); ++pos)
		{
			if ((*pos)->filter == operand)
				return pos;
		}
		return operands.end();
	}

public:
	Cmiss_graphics_filter_operator()
	{
		filter_type = CMISS_GRAPHICS_FILTER_TYPE_OPERATOR;
	}

	virtual ~Cmiss_graphics_filter_operator()
	{
		for (OperandList::iterator pos = operands.begin(); pos != operands.end(); ++pos)
		{
			delete *pos;
		}
	}

	virtual bool match(struct Cmiss_graphic *graphic) = 0;

	void list_operands() const
	{
		if (0 < operands.size())
		{
			display_message(INFORMATION_MESSAGE, " add_filters");
			for (OperandList::const_iterator pos = operands.begin(); pos != operands.end(); ++pos)
			{
				display_message(INFORMATION_MESSAGE, " %s", (*pos)->filter->name);
			}
		}
	}

	/**
	 * Check for circular dependencies / infinite loops.
	 * @return  true if this filter depends on other_filter, false if not.
	 */
	virtual bool depends_on_filter(const Cmiss_graphics_filter *other_filter) const
	{
		if (other_filter == this)
			return true;
		for (OperandList::const_iterator pos = operands.begin(); pos != operands.end(); ++pos)
		{
			if ((*pos)->filter->depends_on_filter(other_filter))
				return true;
		}
		return false;
	}

	int append_operand(Cmiss_graphics_filter *operand)
	{
		int return_code = 1;
		OperandList::iterator pos = find_operand(operand);
		if (pos != operands.end())
		{
			// move existing operand to end of list
			operands.push_back(*pos);
			operands.erase(pos);
		}
		else
		{
			// prevent circular dependency / infinite loop
			if (!this->depends_on_filter(operand))
			{
				operands.push_back(new Graphics_filter_operand(operand));
			}
			else
			{
				return_code = 0;
			}
		}
		return return_code;
	}

	int remove_operand(Cmiss_graphics_filter *operand)
	{
		int return_code = 1;
		OperandList::iterator pos = find_operand(operand);
		if (pos != operands.end())
		{
			delete *pos;
			operands.erase(pos);
		}
		else
		{
			return_code = 0;
		}
		return return_code;
	}

	Cmiss_graphics_filter_id getFirstOperand()
	{
		Cmiss_graphics_filter_id operand = NULL;
		OperandList::iterator pos = operands.begin();
		if (pos != operands.end())
		{
			operand = (*pos)->filter->access();
		}
		return operand;
	}

	Cmiss_graphics_filter_id getNextOperand(Cmiss_graphics_filter_id ref_operand)
	{
		Cmiss_graphics_filter_id operand = NULL;
		OperandList::iterator pos = find_operand(ref_operand);
		pos++;
		if (pos != operands.end())
		{
			operand = (*pos)->filter->access();
		}
		return operand;
	}

	int getOperandIsActive(Cmiss_graphics_filter_id operand)
	{
		int return_code = 0;
		OperandList::iterator pos = find_operand(operand);
		if (pos != operands.end())
		{
			return_code = (*pos)->isActive;
		}
		return return_code;
	}

	int setOperandIsActive(Cmiss_graphics_filter_id operand, int is_active)
	{
		int return_code = 0;
		OperandList::iterator pos = find_operand(operand);
		if (pos != operands.end())
		{
			(*pos)->isActive = (is_active != 0);
			return_code = 1;
		}
		return return_code;
	}

	int insertOperandBefore(Cmiss_graphics_filter_id operand, Cmiss_graphics_filter_id ref_operand)
	{
		int return_code = 0;
		OperandList::iterator refpos = find_operand(ref_operand);
		if (refpos != operands.end())
		{
			OperandList::iterator pos = find_operand(operand);
			if (pos != operands.end())
			{
				// move existing operand to before refpos
				operands.insert(refpos, (*pos));
				operands.erase(pos);
			}
			else
			{
				operands.insert(refpos, new Graphics_filter_operand(operand));
			}
			return_code = 1;
		}
		return return_code;
	}
};

class Cmiss_graphics_filter_operator_and : public Cmiss_graphics_filter_operator
{
public:
	Cmiss_graphics_filter_operator_and()
	{
		filter_type = CMISS_GRAPHICS_FILTER_TYPE_OPERATOR_AND;
	}

	virtual bool match(struct Cmiss_graphic *graphic)
	{
		int return_code = 1;
		for (OperandList::iterator pos = operands.begin(); pos != operands.end(); ++pos)
		{
			if ((*pos)->isActive && (!(*pos)->filter->match(graphic)))
			{
				return_code = 0;
				break;
			}
		}
		return (!isInverse() == return_code);
	}

	virtual void list_type_specific() const
	{
		display_message(INFORMATION_MESSAGE, "operator_and");
		list_operands();
	}
};

class Cmiss_graphics_filter_operator_or : public Cmiss_graphics_filter_operator
{
public:
	Cmiss_graphics_filter_operator_or()
	{
		filter_type = CMISS_GRAPHICS_FILTER_TYPE_OPERATOR_OR;
	}

	virtual bool match(struct Cmiss_graphic *graphic)
	{
		int return_code = 1;
		if (operands.size() > 0)
		{
			return_code = 0;
		}
		for (OperandList::iterator pos = operands.begin(); pos != operands.end(); ++pos)
		{
			if ((*pos)->isActive && (*pos)->filter->match(graphic))
			{
				return_code = 1;
				break;
			}
		}
		return (!isInverse() == return_code);
	}

	virtual void list_type_specific() const
	{
		display_message(INFORMATION_MESSAGE, "operator_or");
		list_operands();
	}
};


namespace {

class Cmiss_graphics_filter_graphic_name : public Cmiss_graphics_filter
{
	char *matchName;

public:
	Cmiss_graphics_filter_graphic_name(const char *inMatchName) :
		matchName(duplicate_string(inMatchName))
	{
		filter_type = CMISS_GRAPHICS_FILTER_TYPE_GRAPHIC_NAME;
	}

	virtual ~Cmiss_graphics_filter_graphic_name()
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

class Cmiss_graphics_filter_visibility_flags : public Cmiss_graphics_filter
{
public:
	Cmiss_graphics_filter_visibility_flags()
	{
		filter_type = CMISS_GRAPHICS_FILTER_TYPE_VISIBILITY_FLAGS;
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

class Cmiss_graphics_filter_region : public Cmiss_graphics_filter
{
	Cmiss_region *matchRegion;

public:
	Cmiss_graphics_filter_region(Cmiss_region *inRegion) :
		matchRegion(inRegion)
	{
		filter_type = CMISS_GRAPHICS_FILTER_TYPE_REGION;
	}

	virtual bool match(struct Cmiss_graphic *graphic)
	{
		return (!isInverse() == Cmiss_graphic_is_from_region_hierarchical(graphic, matchRegion));
	}

	virtual void list_type_specific() const
	{
		char *region_name = Cmiss_region_get_path(matchRegion);
		display_message(INFORMATION_MESSAGE, "match_region_path %s", region_name);
		DEALLOCATE(region_name);
	}
};

}

Cmiss_graphics_filter *Cmiss_graphics_filter_access(Cmiss_graphics_filter *filter)
{
	if (filter)
		filter->access();
	return filter;
}

int Cmiss_graphics_filter_destroy(Cmiss_graphics_filter **filter_address)
{
	return DEACCESS(Cmiss_graphics_filter)(filter_address);
}

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
		Cmiss_graphics_filter::name = const_cast<char *>(name);
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

FULL_DECLARE_MANAGER_TYPE_WITH_OWNER(Cmiss_graphics_filter, Cmiss_graphics_module, void *);

namespace {

/***************************************************************************//**
 * Frees the memory for the graphics_filter of <*graphics_filter_address>.
 * Sets *graphics_filter_address to NULL.
 */
int DESTROY(Cmiss_graphics_filter)(struct Cmiss_graphics_filter **graphics_filter_address)
{
	int return_code;

	ENTER(DESTROY(Cmiss_graphics_filter));
	if (graphics_filter_address && (*graphics_filter_address))
	{
		delete *graphics_filter_address;
		*graphics_filter_address = NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Cmiss_graphics_filter).  Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

DECLARE_LOCAL_MANAGER_FUNCTIONS(Cmiss_graphics_filter)

} /* anonymous namespace */

DECLARE_ACCESS_OBJECT_FUNCTION(Cmiss_graphics_filter)

/***************************************************************************//**
 * Custom version handling is_managed_flag.
 */
PROTOTYPE_DEACCESS_OBJECT_FUNCTION(Cmiss_graphics_filter)
{
	int return_code;
	Cmiss_graphics_filter *object;

	ENTER(DEACCESS(Cmiss_graphics_filter));
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
				(MANAGER_CHANGE_NONE(Cmiss_graphics_filter) != object->manager_change_status))))
		{
			return_code =
				REMOVE_OBJECT_FROM_MANAGER(Cmiss_graphics_filter)(object, object->manager);
		}
		else
		{
			return_code = 1;
		}
		*object_address = (struct Cmiss_graphics_filter *)NULL;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DEACCESS(Cmiss_graphics_filter) */

PROTOTYPE_REACCESS_OBJECT_FUNCTION(Cmiss_graphics_filter)
{
	int return_code;

	ENTER(REACCESS(Cmiss_graphics_filter));
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
			DEACCESS(Cmiss_graphics_filter)(object_address);
		}
		/* point to the new object */
		*object_address = new_object;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"REACCESS(Cmiss_graphics_filter).  Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* REACCESS(Cmiss_graphics_filter) */

DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Cmiss_graphics_filter)

DECLARE_INDEXED_LIST_STL_FUNCTIONS(Cmiss_graphics_filter)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_STL_FUNCTION(Cmiss_graphics_filter,name,const char *)

DECLARE_MANAGER_FUNCTIONS(Cmiss_graphics_filter,manager)
DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Cmiss_graphics_filter,manager)
DECLARE_MANAGER_IDENTIFIER_WITHOUT_MODIFY_FUNCTIONS(Cmiss_graphics_filter,name,const char *,manager)
DECLARE_MANAGER_OWNER_FUNCTIONS(Cmiss_graphics_filter, struct Cmiss_graphics_module)

int Cmiss_graphics_filter_manager_set_owner_private(struct MANAGER(Cmiss_graphics_filter) *manager,
	struct Cmiss_graphics_module *graphics_module)
{
	return MANAGER_SET_OWNER(Cmiss_graphics_filter)(manager, graphics_module);
}

char *Cmiss_graphics_filter_get_name(Cmiss_graphics_filter_id graphics_filter)
{
	char *name = NULL;

	if (graphics_filter)
	{
		name = graphics_filter->getName();
	}
	return name;
}

int Cmiss_graphics_filter_set_name(Cmiss_graphics_filter_id graphics_filter, const char *name)
{
	int return_code;

	ENTER(Cmiss_graphics_filter_set_name);
	if (graphics_filter && is_standard_object_name(name))
	{
		return_code = 1;
		Cmiss_set_Cmiss_graphics_filter *manager_graphics_filter_list = 0;
		bool restore_changed_object_to_lists = false;
		if (graphics_filter->manager)
		{
			if (FIND_BY_IDENTIFIER_IN_MANAGER(Cmiss_graphics_filter, name)(
				name, graphics_filter->manager))
			{
				display_message(ERROR_MESSAGE, "Cmiss_graphics_filter_set_name.  "
					"graphics_filter named '%s' already exists.", name);
				return_code = 0;
			}
			if (return_code)
			{
				manager_graphics_filter_list = reinterpret_cast<Cmiss_set_Cmiss_graphics_filter *>(
					graphics_filter->manager->object_list);
				// this temporarily removes the object from all related lists
				restore_changed_object_to_lists =
					manager_graphics_filter_list->begin_identifier_change(graphics_filter);
				if (!restore_changed_object_to_lists)
				{
					display_message(ERROR_MESSAGE, "Cmiss_graphics_filter_set_name.  "
						"Could not safely change identifier in manager");
					return_code = 0;
				}
			}
		}
		if (return_code)
		{
			if (name)
			{
				graphics_filter->setName(name);
			}
			else
			{
				return_code = 0;
			}
		}
		if (restore_changed_object_to_lists)
		{
			manager_graphics_filter_list->end_identifier_change();
		}
		if (graphics_filter->manager && return_code)
		{
			MANAGED_OBJECT_CHANGE(Cmiss_graphics_filter)(graphics_filter,
				MANAGER_CHANGE_IDENTIFIER(Cmiss_graphics_filter));
		}
	}
	else
	{
		if (graphics_filter)
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_graphics_filter_set_name.  Invalid graphics_filter name '%s'", name);
		}
		return_code=0;
	}

	return (return_code);
}

char *get_valid_temporary_name_for_graphics_filter(
	Cmiss_graphics_module_id graphics_module)
{
	char *name = NULL;
	if (graphics_module)
	{
		struct MANAGER(Cmiss_graphics_filter) *graphics_filter_manager =
			Cmiss_graphics_module_get_filter_manager(graphics_module);
		char temp_name[20];
		int i = NUMBER_IN_MANAGER(Cmiss_graphics_filter)(graphics_filter_manager);
		do
		{
			i++;
			sprintf(temp_name, "temp%d",i);
		}
		while (FIND_BY_IDENTIFIER_IN_MANAGER(Cmiss_graphics_filter,name)(temp_name,
			graphics_filter_manager));
		name = duplicate_string(temp_name);
	}
	return name;
}

int Cmiss_graphics_filter_evaluate_graphic(Cmiss_graphics_filter_id filter,
	Cmiss_graphic_id graphic)
{
	int return_code = 0;
	if (filter && graphic)
	{
		return_code = filter->match(graphic);
	}

	return return_code;
}

Cmiss_graphics_filter_id Cmiss_graphics_module_create_filter_visibility_flags(
	Cmiss_graphics_module_id graphics_module)
{
	Cmiss_graphics_filter_id graphics_filter = NULL;
	if (graphics_module)
	{
		char *name = get_valid_temporary_name_for_graphics_filter(graphics_module);
		struct MANAGER(Cmiss_graphics_filter) *graphics_filter_manager =
			Cmiss_graphics_module_get_filter_manager(graphics_module);
		graphics_filter = new Cmiss_graphics_filter_visibility_flags();
		Cmiss_graphics_filter_set_name(graphics_filter, name);
		if (!ADD_OBJECT_TO_MANAGER(Cmiss_graphics_filter)(graphics_filter, graphics_filter_manager))
		{
			DEACCESS(Cmiss_graphics_filter)(&graphics_filter);
		}
		DEALLOCATE(name);
	}
	return graphics_filter;
}

Cmiss_graphics_filter_id Cmiss_graphics_module_create_filter_graphic_name(
	Cmiss_graphics_module_id graphics_module, const char *match_name)
{
	Cmiss_graphics_filter_id graphics_filter = NULL;
	if (graphics_module && match_name)
	{
		char *name = get_valid_temporary_name_for_graphics_filter(graphics_module);
		struct MANAGER(Cmiss_graphics_filter) *graphics_filter_manager =
			Cmiss_graphics_module_get_filter_manager(graphics_module);
		graphics_filter = new Cmiss_graphics_filter_graphic_name(match_name);
		Cmiss_graphics_filter_set_name(graphics_filter, name);
		if (!ADD_OBJECT_TO_MANAGER(Cmiss_graphics_filter)(graphics_filter, graphics_filter_manager))
		{
			DEACCESS(Cmiss_graphics_filter)(&graphics_filter);
		}
		DEALLOCATE(name);
	}
	return graphics_filter;
}

Cmiss_graphics_filter_id Cmiss_graphics_module_create_filter_region(
	Cmiss_graphics_module_id graphics_module, Cmiss_region_id match_region)
{
	Cmiss_graphics_filter_id graphics_filter = NULL;
	if (graphics_module && match_region)
	{
		char *name = get_valid_temporary_name_for_graphics_filter(graphics_module);
		struct MANAGER(Cmiss_graphics_filter) *graphics_filter_manager =
			Cmiss_graphics_module_get_filter_manager(graphics_module);
		graphics_filter =  new Cmiss_graphics_filter_region(match_region);
		Cmiss_graphics_filter_set_name(graphics_filter, name);
		if (!ADD_OBJECT_TO_MANAGER(Cmiss_graphics_filter)(graphics_filter, graphics_filter_manager))
		{
			DEACCESS(Cmiss_graphics_filter)(&graphics_filter);
		}
		DEALLOCATE(name);
	}
	return graphics_filter;
}

Cmiss_graphics_filter_id Cmiss_graphics_module_create_filter_operator_and(
	Cmiss_graphics_module_id graphics_module)
{
	Cmiss_graphics_filter_id graphics_filter = NULL;
	if (graphics_module)
	{
		char *name = get_valid_temporary_name_for_graphics_filter(graphics_module);
		struct MANAGER(Cmiss_graphics_filter) *graphics_filter_manager =
			Cmiss_graphics_module_get_filter_manager(graphics_module);
		graphics_filter = new Cmiss_graphics_filter_operator_and();
		Cmiss_graphics_filter_set_name(graphics_filter, name);
		if (!ADD_OBJECT_TO_MANAGER(Cmiss_graphics_filter)(graphics_filter, graphics_filter_manager))
		{
			DEACCESS(Cmiss_graphics_filter)(&graphics_filter);
		}
		DEALLOCATE(name);
	}
	return graphics_filter;
}

Cmiss_graphics_filter_id Cmiss_graphics_module_create_filter_operator_or(
	Cmiss_graphics_module_id graphics_module)
{
	Cmiss_graphics_filter_id graphics_filter = NULL;
	if (graphics_module)
	{
		char *name = get_valid_temporary_name_for_graphics_filter(graphics_module);
		struct MANAGER(Cmiss_graphics_filter) *graphics_filter_manager =
			Cmiss_graphics_module_get_filter_manager(graphics_module);
		graphics_filter = new Cmiss_graphics_filter_operator_or();
		Cmiss_graphics_filter_set_name(graphics_filter, name);
		if (!ADD_OBJECT_TO_MANAGER(Cmiss_graphics_filter)(graphics_filter, graphics_filter_manager))
		{
			DEACCESS(Cmiss_graphics_filter)(&graphics_filter);
		}
		DEALLOCATE(name);
	}
	return graphics_filter;
}

enum Cmiss_graphics_filter_type Cmiss_graphics_filter_get_type(Cmiss_graphics_filter_id graphics_filter)
{
	enum Cmiss_graphics_filter_type filter_type = CMISS_GRAPHICS_FILTER_TYPE_INVALID;
	if (graphics_filter)
	{
		filter_type = graphics_filter->getType();
	}
	return filter_type;
}

struct Define_graphics_filter_data
{
	Cmiss_region *root_region;
	Cmiss_graphics_module *graphics_module;
	int number_of_filters;
	Cmiss_graphics_filter **source_filters;
};

int set_Cmiss_graphics_filter_source_data(struct Parse_state *state,
	void *filter_data_void,void *dummy_void)
{
	int return_code = 1;
	struct Define_graphics_filter_data *filter_data = (struct Define_graphics_filter_data *)filter_data_void;
	const char *current_token;
	Cmiss_graphics_filter *filter = NULL, **temp_source_filters = NULL;
	Cmiss_graphics_module *graphics_module = filter_data->graphics_module;

	USE_PARAMETER(dummy_void);
	if (state && filter_data && graphics_module)
	{
		current_token=state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				while (return_code && (current_token = state->current_token))
				{
					/* first try to find a number in the token */
					filter=Cmiss_graphics_module_find_filter_by_name(graphics_module, current_token);
					if (filter)
					{
						shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown filter: %s",current_token);
						display_parse_state_location(state);
						return_code=0;
					}
					if (return_code)
					{
						if (REALLOCATE(temp_source_filters, filter_data->source_filters,
							Cmiss_graphics_filter *, filter_data->number_of_filters+1))
						{
							filter_data->source_filters = temp_source_filters;
							temp_source_filters[filter_data->number_of_filters] =	filter;
							(filter_data->number_of_filters)++;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"set_Computed_field_composite_source_data.  "
								"Not enough memory");
							return_code=0;
						}
					}
				}
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"Missing component filters");
			display_parse_state_location(state);
			return_code=0;
		}
	}


	return return_code;
}

int gfx_define_graphics_filter_operator_or(struct Parse_state *state, void *graphics_filter_handle_void,
	void *filter_data_void)
{
	int return_code = 1, add_filter = 1;
	enum Cmiss_graphics_filter_type filter_type;
	struct Define_graphics_filter_data *filter_data = (struct Define_graphics_filter_data *)filter_data_void;
	if (state && filter_data)
	{
		Cmiss_graphics_filter_id *graphics_filter_handle = (Cmiss_graphics_filter_id *)graphics_filter_handle_void; // can be null
		Cmiss_graphics_filter_id graphics_filter = *graphics_filter_handle;
		if (graphics_filter)
		{
			filter_type = Cmiss_graphics_filter_get_type(graphics_filter);
		}
		else
		{
			graphics_filter = Cmiss_graphics_module_create_filter_operator_or(
				filter_data->graphics_module);
			filter_type = Cmiss_graphics_filter_get_type(graphics_filter);
		}
		struct Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table," define an operator_or filter, multiple filters defined earlier "
			"can be added or removed. This filter will perform a boolean \"or\" check on the filters provided. "
			"Graphics that match any of the filters will be shown.");
		Option_table_add_switch(option_table,"add_filters","remove_filters",&add_filter);
		Option_table_add_entry(option_table, "FILTERS", filter_data,
			NULL, set_Cmiss_graphics_filter_source_data);
		return_code = Option_table_multi_parse(option_table, state);
		if (return_code && filter_type == CMISS_GRAPHICS_FILTER_TYPE_OPERATOR_OR )
		{
			Cmiss_graphics_filter_operator_id operator_filter = Cmiss_graphics_filter_cast_operator(graphics_filter);
			if (operator_filter)
			{
				if (add_filter)
				{
					for (int i = 0; i < filter_data->number_of_filters; i++)
					{
						Cmiss_graphics_filter_operator_append_operand(operator_filter, filter_data->source_filters[i]);
						Cmiss_graphics_filter_destroy(&filter_data->source_filters[i]);
					}
				}
				else
				{
					for (int i = 0; i < filter_data->number_of_filters; i++)
					{
						Cmiss_graphics_filter_operator_remove_operand(operator_filter, filter_data->source_filters[i]);
						Cmiss_graphics_filter_destroy(&filter_data->source_filters[i]);
					}
				}
				Cmiss_graphics_filter_operator_destroy(&operator_filter);
			}
			else
			{
				return_code = 0;
			}
		}
		else
		{
			return_code = 0;
		}
		DESTROY(Option_table)(&option_table);
		if (filter_data->source_filters)
			DEALLOCATE(filter_data->source_filters);
		if (graphics_filter)
			*graphics_filter_handle = graphics_filter;
	}

	return return_code;
}

int gfx_define_graphics_filter_operator_and(struct Parse_state *state, void *graphics_filter_handle_void,
	void *filter_data_void)
{
	int return_code = 1, add_filter = 1;
	enum Cmiss_graphics_filter_type filter_type;
	struct Define_graphics_filter_data *filter_data = (struct Define_graphics_filter_data *)filter_data_void;
	if (state && filter_data)
	{
		Cmiss_graphics_filter_id *graphics_filter_handle = (Cmiss_graphics_filter_id *)graphics_filter_handle_void; // can be null
		Cmiss_graphics_filter_id graphics_filter = *graphics_filter_handle;
		if (!graphics_filter)
		{
			graphics_filter = Cmiss_graphics_module_create_filter_operator_and(
				filter_data->graphics_module);
		}
		filter_type = Cmiss_graphics_filter_get_type(graphics_filter);
		struct Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table," define an operator_and filter, multiple filters defined earlier "
			"can be added or removed. This filter will perform a boolean \"and\" check on the filters provided. "
			"Only graphics that match all of the filters will be shown.");
		Option_table_add_switch(option_table,"add_filters","remove_filters",&add_filter);
		Option_table_add_entry(option_table,  "FILTERS", filter_data,
			NULL, set_Cmiss_graphics_filter_source_data);
		return_code = Option_table_multi_parse(option_table, state);
		if (return_code && filter_type == CMISS_GRAPHICS_FILTER_TYPE_OPERATOR_AND)
		{
			Cmiss_graphics_filter_operator_id operator_filter = Cmiss_graphics_filter_cast_operator(graphics_filter);
			if (operator_filter)
			{
				if (add_filter)
				{
					for (int i = 0; i < filter_data->number_of_filters; i++)
					{
						Cmiss_graphics_filter_operator_append_operand(operator_filter, filter_data->source_filters[i]);
						Cmiss_graphics_filter_destroy(&filter_data->source_filters[i]);
					}
				}
				else
				{
					for (int i = 0; i < filter_data->number_of_filters; i++)
					{
						Cmiss_graphics_filter_operator_remove_operand(operator_filter, filter_data->source_filters[i]);
						Cmiss_graphics_filter_destroy(&filter_data->source_filters[i]);
					}
				}
				Cmiss_graphics_filter_operator_destroy(&operator_filter);
			}
			else
			{
				return_code = 0;
			}
		}
		else
		{
			return_code = 0;
		}
		DESTROY(Option_table)(&option_table);
		if (filter_data->source_filters)
			DEALLOCATE(filter_data->source_filters);
		if (graphics_filter)
			*graphics_filter_handle = graphics_filter;
	}

	return return_code;
}

int gfx_define_graphics_filter_contents(struct Parse_state *state, void *graphics_filter_handle_void,
	void *filter_data_void)
{
	int return_code = 1;
	enum Cmiss_graphics_filter_type filter_type;
	struct Define_graphics_filter_data *filter_data = (struct Define_graphics_filter_data *)filter_data_void;
	char *match_graphic_name, match_visibility_flags, *match_region_path;
	int inverse;

	if (state && filter_data)
	{
		Cmiss_graphics_filter_id *graphics_filter_handle = (Cmiss_graphics_filter_id *)graphics_filter_handle_void; // can be null
		Cmiss_graphics_filter_id graphics_filter = *graphics_filter_handle;
		if (graphics_filter)
		{
			filter_type = Cmiss_graphics_filter_get_type(graphics_filter);
		}
		else
		{
			filter_type = CMISS_GRAPHICS_FILTER_TYPE_INVALID;
		}
		match_graphic_name = NULL;
		match_visibility_flags = 0;
		match_region_path = NULL;
		inverse = 0;
		if (graphics_filter)
		{
			inverse = graphics_filter->isInverse();
		}

		struct Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table," Filter to set up what will be and "
				"what will not be included in a scene. The optional inverse_match "
				"flag will invert the filter's match criterion. The behaviour is to show matching graphic "
				"with the matching criteria. <match_graphic_name> filters graphic with the matching name. "
				"<match_visibility_flags> filters graphic with the setting on the visibility flag. "
				"<match_region_path> filters graphic in the specified region or its subregion. "
				"<operator_or> filters the scene using the logical operation 'or' on a collective of filters. "
				"<operator_and> filters the scene using the logical operation 'and' on a collective of filters. "
				"Filters created earlier can be added or removed from the <operator_or> and <operator_and> filter.");

		Option_table_add_entry(option_table, "operator_or", graphics_filter_handle_void, filter_data_void,
			gfx_define_graphics_filter_operator_or);
		Option_table_add_entry(option_table, "operator_and", graphics_filter_handle_void, filter_data_void,
			gfx_define_graphics_filter_operator_and);
		Option_table_add_string_entry(option_table, "match_graphic_name",
			&(match_graphic_name), " MATCH_NAME");
		Option_table_add_char_flag_entry(option_table, "match_visibility_flags",
			&(match_visibility_flags));
		Option_table_add_string_entry(option_table, "match_region_path",
			&(match_region_path), " REGION_PATH");
		Option_table_add_switch(option_table, "inverse_match", "normal_match",
			&(inverse));
		return_code = Option_table_multi_parse(option_table, state);
		if (return_code)
		{
			int number_of_match_criteria = match_visibility_flags +
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
			if (!graphics_filter)
			{
				if (match_visibility_flags)
				{
					graphics_filter = Cmiss_graphics_module_create_filter_visibility_flags(
						filter_data->graphics_module);
				}
				else if (match_graphic_name)
				{
					graphics_filter = Cmiss_graphics_module_create_filter_graphic_name(
						filter_data->graphics_module, match_graphic_name);
				}
				else if (match_region_path)
				{
					Cmiss_region *match_region = Cmiss_region_find_subregion_at_path(
						filter_data->root_region, match_region_path);
					if (match_region)
					{
						graphics_filter = Cmiss_graphics_module_create_filter_region(
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

			if (graphics_filter)
			{
				Cmiss_graphics_filter_set_attribute_integer(graphics_filter,
					CMISS_GRAPHICS_FILTER_ATTRIBUTE_IS_INVERSE, inverse);
				*graphics_filter_handle = graphics_filter;
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
			"gfx_define_graphics_filter_contents.  Invalid arguments");
		return_code = 0;
	}
	return return_code;
}

int gfx_define_graphics_filter(struct Parse_state *state, void *root_region_void,
	void *graphics_module_void)
{
	int return_code = 1;
	Cmiss_graphics_filter *graphics_filter = NULL;
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
				MANAGER(Cmiss_graphics_filter) *graphics_filter_manager =
					Cmiss_graphics_module_get_filter_manager(graphics_module);
				MANAGER_BEGIN_CACHE(Cmiss_graphics_filter)(graphics_filter_manager);
				graphics_filter = Cmiss_graphics_module_find_filter_by_name(graphics_module, current_token);
				shift_Parse_state(state,1);
				Cmiss_graphics_filter **graphics_filter_address = &graphics_filter;
				struct Define_graphics_filter_data filter_data;
				filter_data.graphics_module = graphics_module;
				filter_data.root_region = root_region;
				filter_data.number_of_filters = 0;
				filter_data.source_filters = NULL;
				if (!graphics_filter)
				{
					return_code = gfx_define_graphics_filter_contents(state, (void *)graphics_filter_address, (void*)&filter_data);
					graphics_filter = *graphics_filter_address;
					Cmiss_graphics_filter_set_name(graphics_filter, current_token);
					Cmiss_graphics_filter_set_attribute_integer(graphics_filter, CMISS_GRAPHICS_FILTER_ATTRIBUTE_IS_MANAGED, 1);
				}
				else
				{
					return_code = gfx_define_graphics_filter_contents(state, (void *)graphics_filter_address, (void*)&filter_data);
				}
				Cmiss_graphics_filter_destroy(&graphics_filter);
				MANAGER_END_CACHE(Cmiss_graphics_filter)(graphics_filter_manager);
			}
			else
			{
				Option_table *option_table = CREATE(Option_table)();
				Option_table_add_entry(option_table, "GRAPHICS_FILTER_NAME",
					/*graphics_filter*/(void *)&graphics_filter, root_region_void,
					gfx_define_graphics_filter_contents);
				return_code = Option_table_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing graphics_filter name");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_define_graphics_filter.  Invalid arguments");
		return_code = 0;
	}
	return return_code;
}

int gfx_list_graphics_filter(struct Parse_state *state, void *dummy_to_be_modified,
	void *graphics_module_void)
{
	USE_PARAMETER(dummy_to_be_modified);
	int return_code = 1;
	Cmiss_graphics_module *graphics_module = (Cmiss_graphics_module *)graphics_module_void;
	if (state && graphics_module)
	{
		char *filter_name = NULL;
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_default_string_entry(option_table,
			&filter_name, "GRAPHICS_FILTER_NAME[all]");
		return_code = Option_table_multi_parse(option_table,state);
		if (return_code)
		{
			if (filter_name)
			{
				Cmiss_graphics_filter *filter =
					Cmiss_graphics_module_find_filter_by_name(graphics_module, filter_name);
				if (filter)
				{
					filter->list("gfx define graphics_filter");
					Cmiss_graphics_filter_destroy(&filter);
				}
				else
				{
					display_message(ERROR_MESSAGE, "Unknown graphics_filter %s", filter_name);
					display_parse_state_location(state);
					return_code = 0;
				}
			}
			else
			{
				MANAGER(Cmiss_graphics_filter) *manager =
					Cmiss_graphics_module_get_filter_manager(graphics_module);
				Cmiss_set_Cmiss_graphics_filter *filter_list =
					reinterpret_cast<Cmiss_set_Cmiss_graphics_filter *>(manager->object_list);
				// Note: doesn't list in dependency order
				for (Cmiss_set_Cmiss_graphics_filter::iterator iter = filter_list->begin();
					iter != filter_list->end(); ++iter)
				{
					(*iter)->list("gfx define graphics_filter");
				}
			}
		}
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_graphics_filter.  Invalid arguments");
		return_code = 0;
	}
	return return_code;
}

int set_Cmiss_graphics_filter(struct Parse_state *state,
	void *graphics_filter_address_void, void *graphics_module_void)
{
	int return_code = 1;

	Cmiss_graphics_filter **graphics_filter_address = (Cmiss_graphics_filter **)graphics_filter_address_void;
	Cmiss_graphics_module *graphics_module = (Cmiss_graphics_module *)graphics_module_void;
	if (state && graphics_filter_address && graphics_module)
	{
		const char *current_token = state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				Cmiss_graphics_filter *graphics_filter = NULL;
				if (!fuzzy_string_compare(current_token, "NONE"))
				{
					graphics_filter = Cmiss_graphics_module_find_filter_by_name(graphics_module, current_token);
					if (!graphics_filter)
					{
						display_message(ERROR_MESSAGE, "Unknown graphics_filter : %s", current_token);
						display_parse_state_location(state);
						return_code = 0;
					}
				}
				if (return_code)
				{
					REACCESS(Cmiss_graphics_filter)(graphics_filter_address, graphics_filter);
					shift_Parse_state(state,1);
				}
				if (graphics_filter)
					Cmiss_graphics_filter_destroy(&graphics_filter);
			}
			else
			{
				display_message(INFORMATION_MESSAGE," GRAPHICS_FILTER_NAME|none[%s]",
					(*graphics_filter_address) ? (*graphics_filter_address)->name : "none");
			}
		}
		else
		{
			display_message(WARNING_MESSAGE, "Missing graphics_filter name");
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

int Cmiss_graphics_filter_get_attribute_integer(Cmiss_graphics_filter_id graphics_filter,
	enum Cmiss_graphics_filter_attribute attribute)
{
	int value = 0;
	if (graphics_filter)
	{
		switch (attribute)
		{
			case CMISS_GRAPHICS_FILTER_ATTRIBUTE_IS_MANAGED:
				value = (int)graphics_filter->is_managed_flag;
				break;
			case CMISS_GRAPHICS_FILTER_ATTRIBUTE_IS_INVERSE:
				value = (int)graphics_filter->isInverse();
				break;
			default:
				display_message(ERROR_MESSAGE,
					"Cmiss_graphics_filter_get_attribute_integer.  Invalid attribute");
				break;
		}
	}
	return value;
}

int Cmiss_graphics_filter_set_attribute_integer(Cmiss_graphics_filter_id graphics_filter,
	enum Cmiss_graphics_filter_attribute attribute, int value)
{
	int return_code = 0;
	if (graphics_filter)
	{
		return_code = 1;
		int old_value =
			Cmiss_graphics_filter_get_attribute_integer(graphics_filter, attribute);
		enum MANAGER_CHANGE(Cmiss_graphics_filter) change =
			MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Cmiss_graphics_filter);
		switch (attribute)
		{
			case CMISS_GRAPHICS_FILTER_ATTRIBUTE_IS_MANAGED:
				graphics_filter->is_managed_flag = (value != 0);
				change = MANAGER_CHANGE_NOT_RESULT(Cmiss_graphics_filter);
			break;
			case CMISS_GRAPHICS_FILTER_ATTRIBUTE_IS_INVERSE:
				graphics_filter->setInverse(value != 0);
				change = MANAGER_CHANGE_RESULT(Cmiss_graphics_filter);
			break;
			default:
				display_message(ERROR_MESSAGE,
					"Cmiss_graphics_filter_set_attribute_integer.  Invalid attribute");
				return_code = 0;
			break;
		}
		if (Cmiss_graphics_filter_get_attribute_integer(graphics_filter, attribute) != old_value)
		{
			MANAGED_OBJECT_CHANGE(Cmiss_graphics_filter)(graphics_filter, change);
		}
	}
	return return_code;
}

Cmiss_graphics_filter_operator_id Cmiss_graphics_filter_cast_operator(Cmiss_graphics_filter_id graphics_filter)
{
	if (dynamic_cast<Cmiss_graphics_filter_operator*>(graphics_filter))
	{
		Cmiss_graphics_filter_access(graphics_filter);
		return (reinterpret_cast<Cmiss_graphics_filter_operator_id>(graphics_filter));
	}
	else
	{
		return (NULL);
	}
}

int Cmiss_graphics_filter_operator_destroy(Cmiss_graphics_filter_operator_id *operator_filter_address)
{
	return Cmiss_graphics_filter_destroy(reinterpret_cast<Cmiss_graphics_filter_id *>(operator_filter_address));
}

int Cmiss_graphics_filter_operator_append_operand(
	Cmiss_graphics_filter_operator_id operator_filter,
	Cmiss_graphics_filter_id operand)
{
	int return_code = 0;
	if (operator_filter && operand)
	{
		return_code = operator_filter->append_operand(operand);
		if (return_code)
		{
			MANAGED_OBJECT_CHANGE(Cmiss_graphics_filter)(
				operator_filter, MANAGER_CHANGE_RESULT(Cmiss_graphics_filter));
		}
	}
	return return_code;
}

Cmiss_graphics_filter_id Cmiss_graphics_filter_operator_get_first_operand(
	Cmiss_graphics_filter_operator_id operator_filter)
{
	Cmiss_graphics_filter_id operand = NULL;
	if (operator_filter)
	{
		operand = operator_filter->getFirstOperand();
	}
	return operand;
}

Cmiss_graphics_filter_id Cmiss_graphics_filter_operator_get_next_operand(
	Cmiss_graphics_filter_operator_id operator_filter,
	Cmiss_graphics_filter_id ref_operand)
{
	Cmiss_graphics_filter_id operand = NULL;
	if (operator_filter && ref_operand)
	{
		operand = operator_filter->getNextOperand(ref_operand);
	}
	return operand;
}

int Cmiss_graphics_filter_operator_get_operand_is_active(
	Cmiss_graphics_filter_operator_id operator_filter,
	Cmiss_graphics_filter_id operand)
{
	int return_code = 0;
	if (operator_filter && operand)
	{
		return_code = operator_filter->getOperandIsActive(operand);
	}
	return return_code;
}

int Cmiss_graphics_filter_operator_set_operand_is_active(
	Cmiss_graphics_filter_operator_id operator_filter,
	Cmiss_graphics_filter_id operand, int is_active)
{
	int return_code = 0;
	if (operator_filter && operand)
	{
		return_code = operator_filter->setOperandIsActive(operand, is_active);
	}
	return return_code;
}

int Cmiss_graphics_filter_operator_insert_operand_before(
	Cmiss_graphics_filter_operator_id operator_filter,
	Cmiss_graphics_filter_id operand, Cmiss_graphics_filter_id ref_operand)
{
	int return_code = 0;
	if (operator_filter && operand && ref_operand)
	{
		return_code = operator_filter->insertOperandBefore(operand, ref_operand);
	}
	return return_code;
}

int Cmiss_graphics_filter_operator_remove_operand(
	Cmiss_graphics_filter_operator_id operator_filter,
	Cmiss_graphics_filter_id operand)
{
	int return_code = 0;
	if (operator_filter && operand)
	{
		return_code = operator_filter->remove_operand(operand);
		if (return_code)
		{
			MANAGED_OBJECT_CHANGE(Cmiss_graphics_filter)(
				operator_filter, MANAGER_CHANGE_RESULT(Cmiss_graphics_filter));
		}
	}
	return return_code;
}

class Cmiss_graphics_filter_attribute_conversion
{
public:
    static const char *to_string(enum Cmiss_graphics_filter_attribute attribute)
    {
        const char *enum_string = 0;
        switch (attribute)
        {
        case CMISS_GRAPHICS_FILTER_ATTRIBUTE_IS_MANAGED:
            enum_string = "IS_MANAGED";
            break;
        case 	CMISS_GRAPHICS_FILTER_ATTRIBUTE_IS_INVERSE:
            enum_string = "IS_INVERSE";
            break;
        default:
            break;
        }
        return enum_string;
    }
};

enum Cmiss_graphics_filter_attribute Cmiss_graphics_filter_attribute_enum_from_string(
	const char *string)
{
	return string_to_enum<enum Cmiss_graphics_filter_attribute,
		Cmiss_graphics_filter_attribute_conversion>(string);
}

char *Cmiss_graphics_filter_attribute_enum_to_string(
	enum Cmiss_graphics_filter_attribute attribute)
{
	const char *attribute_string =Cmiss_graphics_filter_attribute_conversion::to_string(attribute);
	return (attribute_string ? duplicate_string(attribute_string) : 0);
}
