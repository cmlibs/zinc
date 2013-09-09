/***************************************************************************//**
 * graphics_filter.hpp
 *
 * Declaration of scene graphic filter classes and functions.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef GRAPHICS_FILTER_HPP
#define GRAPHICS_FILTER_HPP

#include "zinc/graphicsmodule.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/mystring.h"
#include "general/object.h"
#include "general/debug.h"
#include "general/cmiss_set.hpp"

struct cmzn_scene;
struct cmzn_graphic;

enum cmzn_graphics_filter_type
{
	CMZN_GRAPHICS_FILTER_TYPE_INVALID,
	CMZN_GRAPHICS_FILTER_TYPE_BASE,
	CMZN_GRAPHICS_FILTER_TYPE_IDENTIFIER ,
	CMZN_GRAPHICS_FILTER_TYPE_DOMAIN_TYPE,
	CMZN_GRAPHICS_FILTER_TYPE_GRAPHIC_NAME,
	CMZN_GRAPHICS_FILTER_TYPE_GRAPHIC_TYPE,
	CMZN_GRAPHICS_FILTER_TYPE_VISIBILITY_FLAGS ,
	CMZN_GRAPHICS_FILTER_TYPE_REGION,
	CMZN_GRAPHICS_FILTER_TYPE_OPERATOR,
	CMZN_GRAPHICS_FILTER_TYPE_OPERATOR_AND,
	CMZN_GRAPHICS_FILTER_TYPE_OPERATOR_OR
};

DECLARE_LIST_TYPES(cmzn_graphics_filter);

DECLARE_MANAGER_TYPES(cmzn_graphics_filter);

PROTOTYPE_OBJECT_FUNCTIONS(cmzn_graphics_filter);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(cmzn_graphics_filter);

PROTOTYPE_LIST_FUNCTIONS(cmzn_graphics_filter);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(cmzn_graphics_filter,name,const char *);

PROTOTYPE_MANAGER_FUNCTIONS(cmzn_graphics_filter);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(cmzn_graphics_filter,name,const char *);

struct cmzn_graphics_filter_change_detail
{
	virtual ~cmzn_graphics_filter_change_detail()
	{
	}
};

struct cmzn_graphics_filter
{
private:
	bool inverse;

public:
	enum cmzn_graphics_filter_type filter_type;
	const char *name;
	int access_count;
	struct MANAGER(cmzn_graphics_filter) *manager;
	int manager_change_status;
	bool is_managed_flag;


	cmzn_graphics_filter() :
		inverse(false),
		filter_type(CMZN_GRAPHICS_FILTER_TYPE_BASE),
		name(NULL),
		access_count(1),
		manager(NULL),
		manager_change_status(MANAGER_CHANGE_NONE(cmzn_graphics_filter)),
		is_managed_flag(false)
	{
	}

	virtual ~cmzn_graphics_filter()
	{
		DEALLOCATE(name);
	}

	virtual bool match(struct cmzn_graphic *graphic) = 0;

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

	int changed(enum MANAGER_CHANGE(cmzn_graphics_filter) change)
	{
		return MANAGED_OBJECT_CHANGE(cmzn_graphics_filter)(this,
			change);
	}

	bool isInverse() const
	{
		return inverse;
	}

	bool setInverse(bool newInverse)
	{
		if (newInverse != inverse)
		{
			inverse = newInverse;
			changed(MANAGER_CHANGE_RESULT(cmzn_graphics_filter));
		}
		return true;
	}

	inline cmzn_graphics_filter *access()
	{
		++access_count;
		return this;
	}

	enum cmzn_graphics_filter_type getType()
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
		if (manager_change_status & MANAGER_CHANGE_RESULT(cmzn_graphics_filter))
		{
			return 1;
		}
		return 0;
	}

	/** clones and clears type-specific change detail, if any.
	 * override for classes with type-specific change detail
	 * @return  change detail prior to clearing, or NULL if none.
	 */
	virtual cmzn_graphics_filter_change_detail *extract_change_detail()
	{
		return NULL;
	}

	/**
	 * override for filter types with that are functions of other filters to
	 * prevent circular dependencies / infinite loops.
	 * @return  true if this filter depends on other_filter, false if not.
	 */
	virtual bool depends_on_filter(const cmzn_graphics_filter *other_filter) const
	{
		return (other_filter == this);
	}

	static inline int deaccess(cmzn_graphics_filter **graphics_filter_address)
	{
		return DEACCESS(cmzn_graphics_filter)(graphics_filter_address);
	}
};

/* Only to be used from FIND_BY_IDENTIFIER_IN_INDEXED_LIST_STL function
 * Creates a pseudo object with name identifier suitable for finding
 * objects by identifier with cmzn_set.
 */
class cmzn_graphics_filter_identifier : private cmzn_graphics_filter
{
public:
	cmzn_graphics_filter_identifier(const char *name)
	{
		// const_cast OK as must never be modified & cleared in destructor
		cmzn_graphics_filter::name = name;
		filter_type = CMZN_GRAPHICS_FILTER_TYPE_IDENTIFIER;
	}

	virtual ~cmzn_graphics_filter_identifier()
	{
		cmzn_graphics_filter::name = NULL;
	}

	virtual bool match(struct cmzn_graphic *graphic)
	{
		USE_PARAMETER(graphic);
		return false;
	}

	virtual void list_type_specific() const
	{
	}

	cmzn_graphics_filter *getPseudoObject()
	{
		return this;
	}
};

/** functor for ordering cmzn_set<cmzn_graphics_filter> by name */
struct cmzn_graphics_filter_compare_name
{
	bool operator() (const cmzn_graphics_filter * graphics_filter1, const cmzn_graphics_filter * graphics_filter2) const
	{
		return strcmp(graphics_filter1->name, graphics_filter2->name) < 0;
	}
};

typedef cmzn_set<cmzn_graphics_filter *,cmzn_graphics_filter_compare_name> cmzn_set_cmzn_graphics_filter;

int cmzn_graphics_filter_manager_set_owner_private(struct MANAGER(cmzn_graphics_filter) *manager,
	struct cmzn_graphics_filter_module *graphics_filter_module);

struct MANAGER(cmzn_graphics_filter) *cmzn_graphics_filter_module_get_manager(
	struct cmzn_graphics_filter_module *graphics_filter_module);

cmzn_graphics_filter_module_id cmzn_graphics_filter_module_create();

#endif /* GRAPHICS_FILTER_HPP_ */
