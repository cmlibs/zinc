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
#include "zinc/scene.h"
#include "zinc/status.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/manager_private.h"
#include "graphics/element_point_ranges.h"
#include "graphics/graphic.h"
#include "graphics/graphics_module.h"
#include "region/cmiss_region.h"
#include "general/enumerator_conversion.hpp"
#include "general/indexed_list_stl_private.hpp"
#include "graphics/graphics_filter.hpp"

struct Cmiss_graphics_filter_operator : public Cmiss_graphics_filter
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
	const char *matchName;

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
		return (!isInverse() == Cmiss_graphic_and_scene_visibility_flags_is_set(graphic));
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

class Cmiss_graphics_filter_domain_type : public Cmiss_graphics_filter
{
	enum Cmiss_field_domain_type domain_type;

public:
	Cmiss_graphics_filter_domain_type(enum Cmiss_field_domain_type inDomainType) :
		domain_type(inDomainType)
	{
		filter_type = CMISS_GRAPHICS_FILTER_TYPE_DOMAIN_TYPE;
	}

	virtual bool match(struct Cmiss_graphic *graphic)
	{
		return (!isInverse() == (domain_type == Cmiss_graphic_get_domain_type(graphic)));
	}

	virtual void list_type_specific() const
	{
		display_message(INFORMATION_MESSAGE, "%s", ENUMERATOR_STRING(Cmiss_field_domain_type)(domain_type));
	}
};

class Cmiss_graphics_filter_graphic_type : public Cmiss_graphics_filter
{
	enum Cmiss_graphic_type graphic_type;

public:
	Cmiss_graphics_filter_graphic_type(enum Cmiss_graphic_type inGraphicType) :
		graphic_type(inGraphicType)
	{
		filter_type = CMISS_GRAPHICS_FILTER_TYPE_GRAPHIC_TYPE;
	}

	virtual bool match(struct Cmiss_graphic *graphic)
	{
		return (!isInverse() == (graphic_type == Cmiss_graphic_get_graphic_type(graphic)));
	}

	virtual void list_type_specific() const
	{
		char *filter_type_name = Cmiss_graphic_type_enum_to_string(graphic_type);
		display_message(INFORMATION_MESSAGE, "%s", filter_type_name);
		DEALLOCATE(filter_type_name);
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

FULL_DECLARE_MANAGER_TYPE_WITH_OWNER(Cmiss_graphics_filter, Cmiss_graphics_filter_module, void *);

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
DECLARE_MANAGER_OWNER_FUNCTIONS(Cmiss_graphics_filter, struct Cmiss_graphics_filter_module)

int Cmiss_graphics_filter_manager_set_owner_private(struct MANAGER(Cmiss_graphics_filter) *manager,
	struct Cmiss_graphics_filter_module *graphics_filter_module)
{
	return MANAGER_SET_OWNER(Cmiss_graphics_filter)(manager, graphics_filter_module);
}


struct Cmiss_graphics_filter_module
{

private:

	struct MANAGER(Cmiss_graphics_filter) *graphicsFilterManager;
	Cmiss_graphics_filter *defaultGraphicsFilter;
	int access_count;

	Cmiss_graphics_filter_module() :
		graphicsFilterManager(CREATE(MANAGER(Cmiss_graphics_filter))()),
		defaultGraphicsFilter(0),
		access_count(1)
	{
	}

	~Cmiss_graphics_filter_module()
	{
		if (defaultGraphicsFilter)
		{
			DEACCESS(Cmiss_graphics_filter)(&(this->defaultGraphicsFilter));
		}
		DESTROY(MANAGER(Cmiss_graphics_filter))(&(this->graphicsFilterManager));
	}

public:

	static Cmiss_graphics_filter_module *create()
	{
		return new Cmiss_graphics_filter_module();
	}

	Cmiss_graphics_filter_module *access()

	{
		++access_count;
		return this;
	}

	static int deaccess(Cmiss_graphics_filter_module* &graphics_filter_module)
	{
		if (graphics_filter_module)
		{
			--(graphics_filter_module->access_count);
			if (graphics_filter_module->access_count <= 0)
			{
				delete graphics_filter_module;
			}
			graphics_filter_module = 0;
			return CMISS_OK;
		}
		return CMISS_ERROR_ARGUMENT;
	}

	struct MANAGER(Cmiss_graphics_filter) *getManager()
	{
		return this->graphicsFilterManager;
	}

	int beginChange()
	{
		return MANAGER_BEGIN_CACHE(Cmiss_graphics_filter)(this->graphicsFilterManager);
	}

	int endChange()
	{
		return MANAGER_END_CACHE(Cmiss_graphics_filter)(this->graphicsFilterManager);
	}

	char *getValidTemporaryNameForGraphicsFilter()
	{
		char *name = NULL;
		if (graphicsFilterManager)
		{
			char temp_name[20];
			int i = NUMBER_IN_MANAGER(Cmiss_graphics_filter)(graphicsFilterManager);
			do
			{
				i++;
				sprintf(temp_name, "temp%d",i);
			}
			while (FIND_BY_IDENTIFIER_IN_MANAGER(Cmiss_graphics_filter,name)(temp_name,
				graphicsFilterManager));
			name = duplicate_string(temp_name);
		}
		return name;
	}

	Cmiss_graphics_filter *createFilterVisibilityFlags()
	{
		Cmiss_graphics_filter_id graphics_filter = NULL;
		if (graphicsFilterManager)
		{
			char *name = getValidTemporaryNameForGraphicsFilter();
			graphics_filter = new Cmiss_graphics_filter_visibility_flags();
			Cmiss_graphics_filter_set_name(graphics_filter, name);
			if (!ADD_OBJECT_TO_MANAGER(Cmiss_graphics_filter)(graphics_filter, graphicsFilterManager))
			{
				DEACCESS(Cmiss_graphics_filter)(&graphics_filter);
			}
			DEALLOCATE(name);
		}
		return graphics_filter;
	}

	Cmiss_graphics_filter_id createFilterDomainType(
		enum Cmiss_field_domain_type domain_type)
	{
		Cmiss_graphics_filter_id graphics_filter = 0;
		if (graphicsFilterManager)
		{
			graphics_filter = new Cmiss_graphics_filter_domain_type(domain_type);
			char *name = getValidTemporaryNameForGraphicsFilter();
			Cmiss_graphics_filter_set_name(graphics_filter, name);
			DEALLOCATE(name);
			if (!ADD_OBJECT_TO_MANAGER(Cmiss_graphics_filter)(graphics_filter, graphicsFilterManager))
			{
				DEACCESS(Cmiss_graphics_filter)(&graphics_filter);
			}
		}
		return graphics_filter;
	}

	Cmiss_graphics_filter *createFilterGraphicName(const char *match_name)
	{
		Cmiss_graphics_filter_id graphics_filter = NULL;
		if (graphicsFilterManager && match_name)
		{
			char *name = getValidTemporaryNameForGraphicsFilter();
			graphics_filter = new Cmiss_graphics_filter_graphic_name(match_name);
			Cmiss_graphics_filter_set_name(graphics_filter, name);
			if (!ADD_OBJECT_TO_MANAGER(Cmiss_graphics_filter)(graphics_filter, graphicsFilterManager))
			{
				DEACCESS(Cmiss_graphics_filter)(&graphics_filter);
			}
			DEALLOCATE(name);
		}
		return graphics_filter;
	}

	Cmiss_graphics_filter_id createFilterGraphicType(
		enum Cmiss_graphic_type graphic_type)
	{
		Cmiss_graphics_filter_id graphics_filter = NULL;
		if (graphicsFilterManager)
		{
			char *name = getValidTemporaryNameForGraphicsFilter();
			graphics_filter = new Cmiss_graphics_filter_graphic_type(graphic_type);
			Cmiss_graphics_filter_set_name(graphics_filter, name);
			if (!ADD_OBJECT_TO_MANAGER(Cmiss_graphics_filter)(graphics_filter, graphicsFilterManager))
			{
				DEACCESS(Cmiss_graphics_filter)(&graphics_filter);
			}
			DEALLOCATE(name);
		}
		return graphics_filter;
	}

	Cmiss_graphics_filter_id createFilterRegion(
		Cmiss_region_id match_region)
	{
		Cmiss_graphics_filter_id graphics_filter = NULL;
		if (graphicsFilterManager && match_region)
		{
			char *name = getValidTemporaryNameForGraphicsFilter();
			graphics_filter =  new Cmiss_graphics_filter_region(match_region);
			Cmiss_graphics_filter_set_name(graphics_filter, name);
			if (!ADD_OBJECT_TO_MANAGER(Cmiss_graphics_filter)(graphics_filter, graphicsFilterManager))
			{
				DEACCESS(Cmiss_graphics_filter)(&graphics_filter);
			}
			DEALLOCATE(name);
		}
		return graphics_filter;
	}

	Cmiss_graphics_filter_id createFilterOperatorAnd()
	{
		Cmiss_graphics_filter_id graphics_filter = NULL;
		if (graphicsFilterManager)
		{
			char *name = getValidTemporaryNameForGraphicsFilter();
			graphics_filter = new Cmiss_graphics_filter_operator_and();
			Cmiss_graphics_filter_set_name(graphics_filter, name);
			if (!ADD_OBJECT_TO_MANAGER(Cmiss_graphics_filter)(graphics_filter, graphicsFilterManager))
			{
				DEACCESS(Cmiss_graphics_filter)(&graphics_filter);
			}
			DEALLOCATE(name);
		}
		return graphics_filter;
	}

	Cmiss_graphics_filter_id createFilterOperatorOr()
	{
		Cmiss_graphics_filter_id graphics_filter = NULL;
		if (graphicsFilterManager)
		{
			char *name = getValidTemporaryNameForGraphicsFilter();
			graphics_filter = new Cmiss_graphics_filter_operator_or();
			Cmiss_graphics_filter_set_name(graphics_filter, name);
			if (!ADD_OBJECT_TO_MANAGER(Cmiss_graphics_filter)(graphics_filter, graphicsFilterManager))
			{
				DEACCESS(Cmiss_graphics_filter)(&graphics_filter);
			}
			DEALLOCATE(name);
		}
		return graphics_filter;
	}

	Cmiss_graphics_filter *findFilterByName(const char *name)
	{
		Cmiss_graphics_filter *graphicsFilter = FIND_BY_IDENTIFIER_IN_MANAGER(Cmiss_graphics_filter,name)(name,
			this->graphicsFilterManager);
		if (graphicsFilter)
		{
			return ACCESS(Cmiss_graphics_filter)(graphicsFilter);
		}
		return 0;
	}

	Cmiss_graphics_filter *getDefaultFilter()
	{
		if (this->defaultGraphicsFilter)
		{
			ACCESS(Cmiss_graphics_filter)(this->defaultGraphicsFilter);
			return this->defaultGraphicsFilter;
		}
		else
		{
			const char *default_graphics_filter_name = "default";
			struct Cmiss_graphics_filter *graphicsFilter = findFilterByName(default_graphics_filter_name);
			if (NULL == graphicsFilter)
			{
				graphicsFilter = createFilterVisibilityFlags();
				Cmiss_graphics_filter_set_name(graphicsFilter, default_graphics_filter_name);
				Cmiss_graphics_filter_set_managed(graphicsFilter, true);
			}
			if (graphicsFilter)
			{
				setDefaultFilter(graphicsFilter);
				Cmiss_graphics_filter_set_managed(graphicsFilter, true);
			}
			return graphicsFilter;
		}

		return 0;
	}

	int setDefaultFilter(Cmiss_graphics_filter *graphicsFilter)
	{
		REACCESS(Cmiss_graphics_filter)(&this->defaultGraphicsFilter, graphicsFilter);
		return CMISS_OK;
	}

};

Cmiss_graphics_filter_id Cmiss_graphics_filter_module_create_filter_visibility_flags(
	Cmiss_graphics_filter_module_id graphics_filter_module)
{
	if (graphics_filter_module)
	{
		return graphics_filter_module->createFilterVisibilityFlags();
	}
	return 0;
}

Cmiss_graphics_filter_id Cmiss_graphics_filter_module_create_filter_domain_type(
	Cmiss_graphics_filter_module_id graphics_filter_module,
	enum Cmiss_field_domain_type domain_type)
{
	if (graphics_filter_module)
	{
		return graphics_filter_module->createFilterDomainType(domain_type);
	}
	return 0;

}

Cmiss_graphics_filter_id Cmiss_graphics_filter_module_create_filter_graphic_name(
	Cmiss_graphics_filter_module_id graphics_filter_module, const char *match_name)
{
	if (graphics_filter_module)
	{
		return graphics_filter_module->createFilterGraphicName(match_name);
	}
	return 0;
}

Cmiss_graphics_filter_id Cmiss_graphics_filter_module_create_filter_graphic_type(
	Cmiss_graphics_filter_module_id graphics_filter_module, enum Cmiss_graphic_type graphic_type)
{
	if (graphics_filter_module)
	{
		return graphics_filter_module->createFilterGraphicType(graphic_type);
	}
	return 0;
}

Cmiss_graphics_filter_id Cmiss_graphics_filter_module_create_filter_region(
	Cmiss_graphics_filter_module_id graphics_filter_module, Cmiss_region_id match_region)
{
	if (graphics_filter_module)
	{
		return graphics_filter_module->createFilterRegion(match_region);
	}
	return 0;
}

Cmiss_graphics_filter_id Cmiss_graphics_filter_module_create_filter_operator_and(
	Cmiss_graphics_filter_module_id graphics_filter_module)
{
	if (graphics_filter_module)
	{
		return graphics_filter_module->createFilterOperatorAnd();
	}
	return 0;
}

Cmiss_graphics_filter_id Cmiss_graphics_filter_module_create_filter_operator_or(
	Cmiss_graphics_filter_module_id graphics_filter_module)
{
	if (graphics_filter_module)
	{
		return graphics_filter_module->createFilterOperatorOr();
	}
	return 0;
}

Cmiss_graphics_filter_module_id Cmiss_graphics_filter_module_create()
{
	return Cmiss_graphics_filter_module::create();
}

Cmiss_graphics_filter_module_id Cmiss_graphics_filter_module_access(
	Cmiss_graphics_filter_module_id graphics_filter_module)
{
	if (graphics_filter_module)
		return graphics_filter_module->access();
	return 0;
}

int Cmiss_graphics_filter_module_destroy(Cmiss_graphics_filter_module_id *graphics_filter_module_address)
{
	if (graphics_filter_module_address)
		return Cmiss_graphics_filter_module::deaccess(*graphics_filter_module_address);
	return CMISS_ERROR_ARGUMENT;
}

struct MANAGER(Cmiss_graphics_filter) *Cmiss_graphics_filter_module_get_manager(
	Cmiss_graphics_filter_module_id graphics_filter_module)
{
	if (graphics_filter_module)
		return graphics_filter_module->getManager();
	return 0;
}

int Cmiss_graphics_filter_module_begin_change(Cmiss_graphics_filter_module_id graphics_filter_module)
{
	if (graphics_filter_module)
		return graphics_filter_module->beginChange();
   return CMISS_ERROR_ARGUMENT;
}

int Cmiss_graphics_filter_module_end_change(Cmiss_graphics_filter_module_id graphics_filter_module)
{
	if (graphics_filter_module)
		return graphics_filter_module->endChange();
   return CMISS_ERROR_ARGUMENT;
}

Cmiss_graphics_filter_id Cmiss_graphics_filter_module_find_filter_by_name(
	Cmiss_graphics_filter_module_id graphics_filter_module, const char *name)
{
	if (graphics_filter_module)
		return graphics_filter_module->findFilterByName(name);
   return 0;
}

Cmiss_graphics_filter_id Cmiss_graphics_filter_module_get_default_filter(
	Cmiss_graphics_filter_module_id graphics_filter_module)
{
	if (graphics_filter_module)
		return graphics_filter_module->getDefaultFilter();
	return 0;
}

int Cmiss_graphics_filter_module_set_default_filter(
	Cmiss_graphics_filter_module_id graphics_filter_module,
	Cmiss_graphics_filter_id graphics_filter)
{
	if (graphics_filter_module)
		return graphics_filter_module->setDefaultFilter(graphics_filter);
	return 0;
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

bool Cmiss_graphics_filter_is_managed(Cmiss_graphics_filter_id filter)
{
	if (filter)
	{
		return filter->is_managed_flag;
	}
	return 0;
}

int Cmiss_graphics_filter_set_managed(Cmiss_graphics_filter_id filter,
	bool value)
{
	if (filter)
	{
		bool old_value = filter->is_managed_flag;
		filter->is_managed_flag = (value != 0);
		if (value != old_value)
		{
			MANAGED_OBJECT_CHANGE(Cmiss_graphics_filter)(filter,
				MANAGER_CHANGE_NOT_RESULT(Cmiss_graphics_filter));
		}
		return CMISS_OK;
	}
	return CMISS_ERROR_ARGUMENT;
}

int Cmiss_graphics_filter_get_attribute_integer(Cmiss_graphics_filter_id graphics_filter,
	enum Cmiss_graphics_filter_attribute attribute)
{
	int value = 0;
	if (graphics_filter)
	{
		switch (attribute)
		{
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
