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
#include "general/cmiss_set.hpp"

FULL_DECLARE_MANAGER_TYPE_WITH_OWNER(cmzn_graphics_filter, cmzn_graphics_filter_module, struct cmzn_graphics_filter_change_detail *);

static inline void MANAGER_UPDATE_DEPENDENCIES(cmzn_graphics_filter)(
	struct MANAGER(cmzn_graphics_filter) *manager)
{
	cmzn_set_cmzn_graphics_filter *all_filters = reinterpret_cast<cmzn_set_cmzn_graphics_filter *>(manager->object_list);
	for (cmzn_set_cmzn_graphics_filter::iterator iter = all_filters->begin(); iter != all_filters->end(); iter++)
	{
		cmzn_graphics_filter_id filter = *iter;
		filter->check_dependency();
	}
}

static inline struct cmzn_graphics_filter_change_detail *MANAGER_EXTRACT_CHANGE_DETAIL(cmzn_graphics_filter)(
	cmzn_graphics_filter *filter)
{
	return filter->extract_change_detail();
}

static inline void MANAGER_CLEANUP_CHANGE_DETAIL(cmzn_graphics_filter)(
	cmzn_graphics_filter_change_detail **change_detail_address)
{
	delete *change_detail_address;
}

struct cmzn_graphics_filter_operator : public cmzn_graphics_filter
{
protected:

	struct Graphics_filter_operand
	{
		cmzn_graphics_filter *filter;
		bool isActive;

		Graphics_filter_operand(cmzn_graphics_filter *filter) :
			filter(cmzn_graphics_filter_access(filter)),
			isActive(true)
		{
		}

		~Graphics_filter_operand()
		{
			cmzn_graphics_filter_destroy(&filter);
		}
	};

	typedef std::list<Graphics_filter_operand*> OperandList;

	OperandList operands;

	OperandList::iterator find_operand(cmzn_graphics_filter *operand)
	{
		for (OperandList::iterator pos = operands.begin(); pos != operands.end(); ++pos)
		{
			if ((*pos)->filter == operand)
				return pos;
		}
		return operands.end();
	}

public:
	cmzn_graphics_filter_operator()
	{
		filter_type = CMZN_GRAPHICS_FILTER_TYPE_OPERATOR;
	}

	virtual ~cmzn_graphics_filter_operator()
	{
		for (OperandList::iterator pos = operands.begin(); pos != operands.end(); ++pos)
		{
			delete *pos;
		}
	}

	virtual int check_dependency()
	{
		if (manager_change_status & MANAGER_CHANGE_RESULT(cmzn_graphics_filter))
		{
			return 1;
		}
		for (OperandList::const_iterator pos = operands.begin(); pos != operands.end(); ++pos)
		{
			if ((*pos)->filter->check_dependency())
			{
				changed(MANAGER_CHANGE_RESULT(cmzn_graphics_filter));
				return 1;
			}
		}
		return 0;
	}

	virtual bool match(struct cmzn_graphic *graphic) = 0;

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
	virtual bool depends_on_filter(const cmzn_graphics_filter *other_filter) const
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

	int append_operand(cmzn_graphics_filter *operand)
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
				changed(MANAGER_CHANGE_RESULT(cmzn_graphics_filter));
			}
			else
			{
				return_code = 0;
			}
		}
		return return_code;
	}

	int remove_operand(cmzn_graphics_filter *operand)
	{
		int return_code = 1;
		OperandList::iterator pos = find_operand(operand);
		if (pos != operands.end())
		{
			delete *pos;
			operands.erase(pos);
			changed(MANAGER_CHANGE_RESULT(cmzn_graphics_filter));
		}
		else
		{
			return_code = 0;
		}
		return return_code;
	}

	cmzn_graphics_filter_id getFirstOperand()
	{
		cmzn_graphics_filter_id operand = NULL;
		OperandList::iterator pos = operands.begin();
		if (pos != operands.end())
		{
			operand = (*pos)->filter->access();
		}
		return operand;
	}

	cmzn_graphics_filter_id getNextOperand(cmzn_graphics_filter_id ref_operand)
	{
		cmzn_graphics_filter_id operand = NULL;
		OperandList::iterator pos = find_operand(ref_operand);
		pos++;
		if (pos != operands.end())
		{
			operand = (*pos)->filter->access();
		}
		return operand;
	}

	int getOperandIsActive(cmzn_graphics_filter_id operand)
	{
		int return_code = 0;
		OperandList::iterator pos = find_operand(operand);
		if (pos != operands.end())
		{
			return_code = (*pos)->isActive;
		}
		return return_code;
	}

	int setOperandIsActive(cmzn_graphics_filter_id operand, int is_active)
	{
		int return_code = 0;
		OperandList::iterator pos = find_operand(operand);
		if (pos != operands.end())
		{
			if (((*pos)->isActive != (is_active != 0)))
			{
				(*pos)->isActive = (is_active != 0);
				changed(MANAGER_CHANGE_RESULT(cmzn_graphics_filter));
			}
			return_code = 1;
		}
		return return_code;
	}

	int insertOperandBefore(cmzn_graphics_filter_id operand, cmzn_graphics_filter_id ref_operand)
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
				changed(MANAGER_CHANGE_RESULT(cmzn_graphics_filter));
			}
			return_code = 1;
		}
		return return_code;
	}
};

class cmzn_graphics_filter_operator_and : public cmzn_graphics_filter_operator
{
public:
	cmzn_graphics_filter_operator_and()
	{
		filter_type = CMZN_GRAPHICS_FILTER_TYPE_OPERATOR_AND;
	}

	virtual bool match(struct cmzn_graphic *graphic)
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

class cmzn_graphics_filter_operator_or : public cmzn_graphics_filter_operator
{
public:
	cmzn_graphics_filter_operator_or()
	{
		filter_type = CMZN_GRAPHICS_FILTER_TYPE_OPERATOR_OR;
	}

	virtual bool match(struct cmzn_graphic *graphic)
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

class cmzn_graphics_filter_graphic_name : public cmzn_graphics_filter
{
	const char *matchName;

public:
	cmzn_graphics_filter_graphic_name(const char *inMatchName) :
		matchName(duplicate_string(inMatchName))
	{
		filter_type = CMZN_GRAPHICS_FILTER_TYPE_GRAPHIC_NAME;
	}

	virtual ~cmzn_graphics_filter_graphic_name()
	{
		DEALLOCATE(matchName);
	}

	virtual bool match(struct cmzn_graphic *graphic)
	{
		return (::cmzn_graphic_has_name(graphic, (void *)matchName) == !isInverse());
	}

	virtual void list_type_specific() const
	{
		display_message(INFORMATION_MESSAGE, "match_graphic_name %s", matchName);
	}

};

class cmzn_graphics_filter_visibility_flags : public cmzn_graphics_filter
{
public:
	cmzn_graphics_filter_visibility_flags()
	{
		filter_type = CMZN_GRAPHICS_FILTER_TYPE_VISIBILITY_FLAGS;
	}

	virtual bool match(struct cmzn_graphic *graphic)
	{
		return (!isInverse() == cmzn_graphic_and_scene_visibility_flags_is_set(graphic));
	}

	virtual void list_type_specific() const
	{
		display_message(INFORMATION_MESSAGE, "match_visibility_flags");
	}
};

class cmzn_graphics_filter_region : public cmzn_graphics_filter
{
	cmzn_region *matchRegion;

public:
	cmzn_graphics_filter_region(cmzn_region *inRegion) :
		matchRegion(inRegion)
	{
		filter_type = CMZN_GRAPHICS_FILTER_TYPE_REGION;
	}

	virtual bool match(struct cmzn_graphic *graphic)
	{
		return (!isInverse() == cmzn_graphic_is_from_region_hierarchical(graphic, matchRegion));
	}

	virtual void list_type_specific() const
	{
		char *region_name = cmzn_region_get_path(matchRegion);
		display_message(INFORMATION_MESSAGE, "match_region_path %s", region_name);
		DEALLOCATE(region_name);
	}
};

class cmzn_graphics_filter_domain_type : public cmzn_graphics_filter
{
	enum cmzn_field_domain_type domain_type;

public:
	cmzn_graphics_filter_domain_type(enum cmzn_field_domain_type inDomainType) :
		domain_type(inDomainType)
	{
		filter_type = CMZN_GRAPHICS_FILTER_TYPE_DOMAIN_TYPE;
	}

	virtual bool match(struct cmzn_graphic *graphic)
	{
		return (!isInverse() == (domain_type == cmzn_graphic_get_domain_type(graphic)));
	}

	virtual void list_type_specific() const
	{
		display_message(INFORMATION_MESSAGE, "%s", ENUMERATOR_STRING(cmzn_field_domain_type)(domain_type));
	}
};

class cmzn_graphics_filter_graphic_type : public cmzn_graphics_filter
{
	enum cmzn_graphic_type graphic_type;

public:
	cmzn_graphics_filter_graphic_type(enum cmzn_graphic_type inGraphicType) :
		graphic_type(inGraphicType)
	{
		filter_type = CMZN_GRAPHICS_FILTER_TYPE_GRAPHIC_TYPE;
	}

	virtual bool match(struct cmzn_graphic *graphic)
	{
		return (!isInverse() == (graphic_type == cmzn_graphic_get_graphic_type(graphic)));
	}

	virtual void list_type_specific() const
	{
		char *filter_type_name = cmzn_graphic_type_enum_to_string(graphic_type);
		display_message(INFORMATION_MESSAGE, "%s", filter_type_name);
		DEALLOCATE(filter_type_name);
	}
};
}

cmzn_graphics_filter *cmzn_graphics_filter_access(cmzn_graphics_filter *filter)
{
	if (filter)
		filter->access();
	return filter;
}

int cmzn_graphics_filter_destroy(cmzn_graphics_filter **filter_address)
{
	return DEACCESS(cmzn_graphics_filter)(filter_address);
}

namespace {

/***************************************************************************//**
 * Frees the memory for the graphics_filter of <*graphics_filter_address>.
 * Sets *graphics_filter_address to NULL.
 */
int DESTROY(cmzn_graphics_filter)(struct cmzn_graphics_filter **graphics_filter_address)
{
	int return_code;

	ENTER(DESTROY(cmzn_graphics_filter));
	if (graphics_filter_address && (*graphics_filter_address))
	{
		delete *graphics_filter_address;
		*graphics_filter_address = NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(cmzn_graphics_filter).  Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

DECLARE_MANAGER_UPDATE_FUNCTION(cmzn_graphics_filter)

DECLARE_MANAGED_OBJECT_NOT_IN_USE_CONDITIONAL_FUNCTION(cmzn_graphics_filter)

} /* anonymous namespace */

DECLARE_ACCESS_OBJECT_FUNCTION(cmzn_graphics_filter)

/***************************************************************************//**
 * Custom version handling is_managed_flag.
 */
PROTOTYPE_DEACCESS_OBJECT_FUNCTION(cmzn_graphics_filter)
{
	int return_code;
	cmzn_graphics_filter *object;

	ENTER(DEACCESS(cmzn_graphics_filter));
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
				(MANAGER_CHANGE_NONE(cmzn_graphics_filter) != object->manager_change_status))))
		{
			return_code =
				REMOVE_OBJECT_FROM_MANAGER(cmzn_graphics_filter)(object, object->manager);
		}
		else
		{
			return_code = 1;
		}
		*object_address = (struct cmzn_graphics_filter *)NULL;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DEACCESS(cmzn_graphics_filter) */

PROTOTYPE_REACCESS_OBJECT_FUNCTION(cmzn_graphics_filter)
{
	int return_code;

	ENTER(REACCESS(cmzn_graphics_filter));
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
			DEACCESS(cmzn_graphics_filter)(object_address);
		}
		/* point to the new object */
		*object_address = new_object;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"REACCESS(cmzn_graphics_filter).  Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* REACCESS(cmzn_graphics_filter) */

DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(cmzn_graphics_filter)

DECLARE_INDEXED_LIST_STL_FUNCTIONS(cmzn_graphics_filter)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_STL_FUNCTION(cmzn_graphics_filter,name,const char *)

DECLARE_MANAGER_FUNCTIONS(cmzn_graphics_filter,manager)
DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(cmzn_graphics_filter,manager)
DECLARE_MANAGER_IDENTIFIER_WITHOUT_MODIFY_FUNCTIONS(cmzn_graphics_filter,name,const char *,manager)
DECLARE_MANAGER_OWNER_FUNCTIONS(cmzn_graphics_filter, struct cmzn_graphics_filter_module)

int cmzn_graphics_filter_manager_set_owner_private(struct MANAGER(cmzn_graphics_filter) *manager,
	struct cmzn_graphics_filter_module *graphics_filter_module)
{
	return MANAGER_SET_OWNER(cmzn_graphics_filter)(manager, graphics_filter_module);
}


struct cmzn_graphics_filter_module
{

private:

	struct MANAGER(cmzn_graphics_filter) *graphicsFilterManager;
	cmzn_graphics_filter *defaultGraphicsFilter;
	int access_count;

	cmzn_graphics_filter_module() :
		graphicsFilterManager(CREATE(MANAGER(cmzn_graphics_filter))()),
		defaultGraphicsFilter(0),
		access_count(1)
	{
	}

	~cmzn_graphics_filter_module()
	{
		if (defaultGraphicsFilter)
		{
			DEACCESS(cmzn_graphics_filter)(&(this->defaultGraphicsFilter));
		}
		DESTROY(MANAGER(cmzn_graphics_filter))(&(this->graphicsFilterManager));
	}

public:

	static cmzn_graphics_filter_module *create()
	{
		return new cmzn_graphics_filter_module();
	}

	cmzn_graphics_filter_module *access()

	{
		++access_count;
		return this;
	}

	static int deaccess(cmzn_graphics_filter_module* &graphics_filter_module)
	{
		if (graphics_filter_module)
		{
			--(graphics_filter_module->access_count);
			if (graphics_filter_module->access_count <= 0)
			{
				delete graphics_filter_module;
			}
			graphics_filter_module = 0;
			return CMZN_OK;
		}
		return CMZN_ERROR_ARGUMENT;
	}

	struct MANAGER(cmzn_graphics_filter) *getManager()
	{
		return this->graphicsFilterManager;
	}

	int beginChange()
	{
		return MANAGER_BEGIN_CACHE(cmzn_graphics_filter)(this->graphicsFilterManager);
	}

	int endChange()
	{
		return MANAGER_END_CACHE(cmzn_graphics_filter)(this->graphicsFilterManager);
	}

	char *getValidTemporaryNameForGraphicsFilter()
	{
		char *name = NULL;
		if (graphicsFilterManager)
		{
			char temp_name[20];
			int i = NUMBER_IN_MANAGER(cmzn_graphics_filter)(graphicsFilterManager);
			do
			{
				i++;
				sprintf(temp_name, "temp%d",i);
			}
			while (FIND_BY_IDENTIFIER_IN_MANAGER(cmzn_graphics_filter,name)(temp_name,
				graphicsFilterManager));
			name = duplicate_string(temp_name);
		}
		return name;
	}

	cmzn_graphics_filter *createFilterVisibilityFlags()
	{
		cmzn_graphics_filter_id graphics_filter = NULL;
		if (graphicsFilterManager)
		{
			char *name = getValidTemporaryNameForGraphicsFilter();
			graphics_filter = new cmzn_graphics_filter_visibility_flags();
			cmzn_graphics_filter_set_name(graphics_filter, name);
			if (!ADD_OBJECT_TO_MANAGER(cmzn_graphics_filter)(graphics_filter, graphicsFilterManager))
			{
				DEACCESS(cmzn_graphics_filter)(&graphics_filter);
			}
			DEALLOCATE(name);
		}
		return graphics_filter;
	}

	cmzn_graphics_filter_id createFilterDomainType(
		enum cmzn_field_domain_type domain_type)
	{
		cmzn_graphics_filter_id graphics_filter = 0;
		if (graphicsFilterManager)
		{
			graphics_filter = new cmzn_graphics_filter_domain_type(domain_type);
			char *name = getValidTemporaryNameForGraphicsFilter();
			cmzn_graphics_filter_set_name(graphics_filter, name);
			DEALLOCATE(name);
			if (!ADD_OBJECT_TO_MANAGER(cmzn_graphics_filter)(graphics_filter, graphicsFilterManager))
			{
				DEACCESS(cmzn_graphics_filter)(&graphics_filter);
			}
		}
		return graphics_filter;
	}

	cmzn_graphics_filter *createFilterGraphicName(const char *match_name)
	{
		cmzn_graphics_filter_id graphics_filter = NULL;
		if (graphicsFilterManager && match_name)
		{
			char *name = getValidTemporaryNameForGraphicsFilter();
			graphics_filter = new cmzn_graphics_filter_graphic_name(match_name);
			cmzn_graphics_filter_set_name(graphics_filter, name);
			if (!ADD_OBJECT_TO_MANAGER(cmzn_graphics_filter)(graphics_filter, graphicsFilterManager))
			{
				DEACCESS(cmzn_graphics_filter)(&graphics_filter);
			}
			DEALLOCATE(name);
		}
		return graphics_filter;
	}

	cmzn_graphics_filter_id createFilterGraphicType(
		enum cmzn_graphic_type graphic_type)
	{
		cmzn_graphics_filter_id graphics_filter = NULL;
		if (graphicsFilterManager)
		{
			char *name = getValidTemporaryNameForGraphicsFilter();
			graphics_filter = new cmzn_graphics_filter_graphic_type(graphic_type);
			cmzn_graphics_filter_set_name(graphics_filter, name);
			if (!ADD_OBJECT_TO_MANAGER(cmzn_graphics_filter)(graphics_filter, graphicsFilterManager))
			{
				DEACCESS(cmzn_graphics_filter)(&graphics_filter);
			}
			DEALLOCATE(name);
		}
		return graphics_filter;
	}

	cmzn_graphics_filter_id createFilterRegion(
		cmzn_region_id match_region)
	{
		cmzn_graphics_filter_id graphics_filter = NULL;
		if (graphicsFilterManager && match_region)
		{
			char *name = getValidTemporaryNameForGraphicsFilter();
			graphics_filter =  new cmzn_graphics_filter_region(match_region);
			cmzn_graphics_filter_set_name(graphics_filter, name);
			if (!ADD_OBJECT_TO_MANAGER(cmzn_graphics_filter)(graphics_filter, graphicsFilterManager))
			{
				DEACCESS(cmzn_graphics_filter)(&graphics_filter);
			}
			DEALLOCATE(name);
		}
		return graphics_filter;
	}

	cmzn_graphics_filter_id createFilterOperatorAnd()
	{
		cmzn_graphics_filter_id graphics_filter = NULL;
		if (graphicsFilterManager)
		{
			char *name = getValidTemporaryNameForGraphicsFilter();
			graphics_filter = new cmzn_graphics_filter_operator_and();
			cmzn_graphics_filter_set_name(graphics_filter, name);
			if (!ADD_OBJECT_TO_MANAGER(cmzn_graphics_filter)(graphics_filter, graphicsFilterManager))
			{
				DEACCESS(cmzn_graphics_filter)(&graphics_filter);
			}
			DEALLOCATE(name);
		}
		return graphics_filter;
	}

	cmzn_graphics_filter_id createFilterOperatorOr()
	{
		cmzn_graphics_filter_id graphics_filter = NULL;
		if (graphicsFilterManager)
		{
			char *name = getValidTemporaryNameForGraphicsFilter();
			graphics_filter = new cmzn_graphics_filter_operator_or();
			cmzn_graphics_filter_set_name(graphics_filter, name);
			if (!ADD_OBJECT_TO_MANAGER(cmzn_graphics_filter)(graphics_filter, graphicsFilterManager))
			{
				DEACCESS(cmzn_graphics_filter)(&graphics_filter);
			}
			DEALLOCATE(name);
		}
		return graphics_filter;
	}

	cmzn_graphics_filter *findFilterByName(const char *name)
	{
		cmzn_graphics_filter *graphicsFilter = FIND_BY_IDENTIFIER_IN_MANAGER(cmzn_graphics_filter,name)(name,
			this->graphicsFilterManager);
		if (graphicsFilter)
		{
			return ACCESS(cmzn_graphics_filter)(graphicsFilter);
		}
		return 0;
	}

	cmzn_graphics_filter *getDefaultFilter()
	{
		if (this->defaultGraphicsFilter)
		{
			ACCESS(cmzn_graphics_filter)(this->defaultGraphicsFilter);
			return this->defaultGraphicsFilter;
		}
		else
		{
			const char *default_graphics_filter_name = "default";
			struct cmzn_graphics_filter *graphicsFilter = findFilterByName(default_graphics_filter_name);
			if (NULL == graphicsFilter)
			{
				graphicsFilter = createFilterVisibilityFlags();
				cmzn_graphics_filter_set_name(graphicsFilter, default_graphics_filter_name);
				cmzn_graphics_filter_set_managed(graphicsFilter, true);
			}
			if (graphicsFilter)
			{
				setDefaultFilter(graphicsFilter);
				cmzn_graphics_filter_set_managed(graphicsFilter, true);
			}
			return graphicsFilter;
		}

		return 0;
	}

	int setDefaultFilter(cmzn_graphics_filter *graphicsFilter)
	{
		REACCESS(cmzn_graphics_filter)(&this->defaultGraphicsFilter, graphicsFilter);
		return CMZN_OK;
	}

};

cmzn_graphics_filter_id cmzn_graphics_filter_module_create_filter_visibility_flags(
	cmzn_graphics_filter_module_id graphics_filter_module)
{
	if (graphics_filter_module)
	{
		return graphics_filter_module->createFilterVisibilityFlags();
	}
	return 0;
}

cmzn_graphics_filter_id cmzn_graphics_filter_module_create_filter_domain_type(
	cmzn_graphics_filter_module_id graphics_filter_module,
	enum cmzn_field_domain_type domain_type)
{
	if (graphics_filter_module)
	{
		return graphics_filter_module->createFilterDomainType(domain_type);
	}
	return 0;

}

cmzn_graphics_filter_id cmzn_graphics_filter_module_create_filter_graphic_name(
	cmzn_graphics_filter_module_id graphics_filter_module, const char *match_name)
{
	if (graphics_filter_module)
	{
		return graphics_filter_module->createFilterGraphicName(match_name);
	}
	return 0;
}

cmzn_graphics_filter_id cmzn_graphics_filter_module_create_filter_graphic_type(
	cmzn_graphics_filter_module_id graphics_filter_module, enum cmzn_graphic_type graphic_type)
{
	if (graphics_filter_module)
	{
		return graphics_filter_module->createFilterGraphicType(graphic_type);
	}
	return 0;
}

cmzn_graphics_filter_id cmzn_graphics_filter_module_create_filter_region(
	cmzn_graphics_filter_module_id graphics_filter_module, cmzn_region_id match_region)
{
	if (graphics_filter_module)
	{
		return graphics_filter_module->createFilterRegion(match_region);
	}
	return 0;
}

cmzn_graphics_filter_id cmzn_graphics_filter_module_create_filter_operator_and(
	cmzn_graphics_filter_module_id graphics_filter_module)
{
	if (graphics_filter_module)
	{
		return graphics_filter_module->createFilterOperatorAnd();
	}
	return 0;
}

cmzn_graphics_filter_id cmzn_graphics_filter_module_create_filter_operator_or(
	cmzn_graphics_filter_module_id graphics_filter_module)
{
	if (graphics_filter_module)
	{
		return graphics_filter_module->createFilterOperatorOr();
	}
	return 0;
}

cmzn_graphics_filter_module_id cmzn_graphics_filter_module_create()
{
	return cmzn_graphics_filter_module::create();
}

cmzn_graphics_filter_module_id cmzn_graphics_filter_module_access(
	cmzn_graphics_filter_module_id graphics_filter_module)
{
	if (graphics_filter_module)
		return graphics_filter_module->access();
	return 0;
}

int cmzn_graphics_filter_module_destroy(cmzn_graphics_filter_module_id *graphics_filter_module_address)
{
	if (graphics_filter_module_address)
		return cmzn_graphics_filter_module::deaccess(*graphics_filter_module_address);
	return CMZN_ERROR_ARGUMENT;
}

struct MANAGER(cmzn_graphics_filter) *cmzn_graphics_filter_module_get_manager(
	cmzn_graphics_filter_module_id graphics_filter_module)
{
	if (graphics_filter_module)
		return graphics_filter_module->getManager();
	return 0;
}

int cmzn_graphics_filter_module_begin_change(cmzn_graphics_filter_module_id graphics_filter_module)
{
	if (graphics_filter_module)
		return graphics_filter_module->beginChange();
   return CMZN_ERROR_ARGUMENT;
}

int cmzn_graphics_filter_module_end_change(cmzn_graphics_filter_module_id graphics_filter_module)
{
	if (graphics_filter_module)
		return graphics_filter_module->endChange();
   return CMZN_ERROR_ARGUMENT;
}

cmzn_graphics_filter_id cmzn_graphics_filter_module_find_filter_by_name(
	cmzn_graphics_filter_module_id graphics_filter_module, const char *name)
{
	if (graphics_filter_module)
		return graphics_filter_module->findFilterByName(name);
   return 0;
}

cmzn_graphics_filter_id cmzn_graphics_filter_module_get_default_filter(
	cmzn_graphics_filter_module_id graphics_filter_module)
{
	if (graphics_filter_module)
		return graphics_filter_module->getDefaultFilter();
	return 0;
}

int cmzn_graphics_filter_module_set_default_filter(
	cmzn_graphics_filter_module_id graphics_filter_module,
	cmzn_graphics_filter_id graphics_filter)
{
	if (graphics_filter_module)
		return graphics_filter_module->setDefaultFilter(graphics_filter);
	return 0;
}

char *cmzn_graphics_filter_get_name(cmzn_graphics_filter_id graphics_filter)
{
	char *name = NULL;

	if (graphics_filter)
	{
		name = graphics_filter->getName();
	}
	return name;
}

int cmzn_graphics_filter_set_name(cmzn_graphics_filter_id graphics_filter, const char *name)
{
	int return_code;

	ENTER(cmzn_graphics_filter_set_name);
	if (graphics_filter && is_standard_object_name(name))
	{
		return_code = 1;
		cmzn_set_cmzn_graphics_filter *manager_graphics_filter_list = 0;
		bool restore_changed_object_to_lists = false;
		if (graphics_filter->manager)
		{
			if (FIND_BY_IDENTIFIER_IN_MANAGER(cmzn_graphics_filter, name)(
				name, graphics_filter->manager))
			{
				display_message(ERROR_MESSAGE, "cmzn_graphics_filter_set_name.  "
					"graphics_filter named '%s' already exists.", name);
				return_code = 0;
			}
			if (return_code)
			{
				manager_graphics_filter_list = reinterpret_cast<cmzn_set_cmzn_graphics_filter *>(
					graphics_filter->manager->object_list);
				// this temporarily removes the object from all related lists
				restore_changed_object_to_lists =
					manager_graphics_filter_list->begin_identifier_change(graphics_filter);
				if (!restore_changed_object_to_lists)
				{
					display_message(ERROR_MESSAGE, "cmzn_graphics_filter_set_name.  "
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
			MANAGED_OBJECT_CHANGE(cmzn_graphics_filter)(graphics_filter,
				MANAGER_CHANGE_IDENTIFIER(cmzn_graphics_filter));
		}
	}
	else
	{
		if (graphics_filter)
		{
			display_message(ERROR_MESSAGE,
				"cmzn_graphics_filter_set_name.  Invalid graphics_filter name '%s'", name);
		}
		return_code=0;
	}

	return (return_code);
}

int cmzn_graphics_filter_evaluate_graphic(cmzn_graphics_filter_id filter,
	cmzn_graphic_id graphic)
{
	int return_code = 0;
	if (filter && graphic)
	{
		return_code = filter->match(graphic);
	}

	return return_code;
}

bool cmzn_graphics_filter_is_managed(cmzn_graphics_filter_id filter)
{
	if (filter)
	{
		return filter->is_managed_flag;
	}
	return 0;
}

int cmzn_graphics_filter_set_managed(cmzn_graphics_filter_id filter,
	bool value)
{
	if (filter)
	{
		bool old_value = filter->is_managed_flag;
		filter->is_managed_flag = (value != 0);
		if (value != old_value)
		{
			MANAGED_OBJECT_CHANGE(cmzn_graphics_filter)(filter,
				MANAGER_CHANGE_NOT_RESULT(cmzn_graphics_filter));
		}
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

bool cmzn_graphics_filter_is_inverse(cmzn_graphics_filter_id filter)
{
	if (filter)
		return filter->isInverse();
	return false;
}

int cmzn_graphics_filter_set_inverse(cmzn_graphics_filter_id filter,
	bool value)
{
	if (filter)
	{
		filter->setInverse(value);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

cmzn_graphics_filter_operator_id cmzn_graphics_filter_cast_operator(cmzn_graphics_filter_id graphics_filter)
{
	if (dynamic_cast<cmzn_graphics_filter_operator*>(graphics_filter))
	{
		cmzn_graphics_filter_access(graphics_filter);
		return (reinterpret_cast<cmzn_graphics_filter_operator_id>(graphics_filter));
	}
	else
	{
		return (NULL);
	}
}

int cmzn_graphics_filter_operator_destroy(cmzn_graphics_filter_operator_id *operator_filter_address)
{
	return cmzn_graphics_filter_destroy(reinterpret_cast<cmzn_graphics_filter_id *>(operator_filter_address));
}

int cmzn_graphics_filter_operator_append_operand(
	cmzn_graphics_filter_operator_id operator_filter,
	cmzn_graphics_filter_id operand)
{
	if (operator_filter && operand)
	{
		return operator_filter->append_operand(operand);
	}
	return 0;
}

cmzn_graphics_filter_id cmzn_graphics_filter_operator_get_first_operand(
	cmzn_graphics_filter_operator_id operator_filter)
{
	cmzn_graphics_filter_id operand = NULL;
	if (operator_filter)
	{
		operand = operator_filter->getFirstOperand();
	}
	return operand;
}

cmzn_graphics_filter_id cmzn_graphics_filter_operator_get_next_operand(
	cmzn_graphics_filter_operator_id operator_filter,
	cmzn_graphics_filter_id ref_operand)
{
	cmzn_graphics_filter_id operand = NULL;
	if (operator_filter && ref_operand)
	{
		operand = operator_filter->getNextOperand(ref_operand);
	}
	return operand;
}

int cmzn_graphics_filter_operator_get_operand_is_active(
	cmzn_graphics_filter_operator_id operator_filter,
	cmzn_graphics_filter_id operand)
{
	int return_code = 0;
	if (operator_filter && operand)
	{
		return_code = operator_filter->getOperandIsActive(operand);
	}
	return return_code;
}

int cmzn_graphics_filter_operator_set_operand_is_active(
	cmzn_graphics_filter_operator_id operator_filter,
	cmzn_graphics_filter_id operand, int is_active)
{
	int return_code = 0;
	if (operator_filter && operand)
	{
		return_code = operator_filter->setOperandIsActive(operand, is_active);
	}
	return return_code;
}

int cmzn_graphics_filter_operator_insert_operand_before(
	cmzn_graphics_filter_operator_id operator_filter,
	cmzn_graphics_filter_id operand, cmzn_graphics_filter_id ref_operand)
{
	int return_code = 0;
	if (operator_filter && operand && ref_operand)
	{
		return_code = operator_filter->insertOperandBefore(operand, ref_operand);
	}
	return return_code;
}

int cmzn_graphics_filter_operator_remove_operand(
	cmzn_graphics_filter_operator_id operator_filter,
	cmzn_graphics_filter_id operand)
{
	if (operator_filter && operand)
	{
		return operator_filter->remove_operand(operand);
	}
	return 0;
}
