/***************************************************************************//**
 * FILE : cmiss_region.cpp
 * 
 * Definition of Cmiss_region, container of fields for representing model data,
 * and child regions for building hierarchical models.
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
 * Portions created by the Initial Developer are Copyright (C) 2005
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
#include "api/cmiss_element.h"
#include "api/cmiss_field_group.h"
#include "api/cmiss_field_module.h"
#include "api/cmiss_field_subobject_group.h"
#include "api/cmiss_node.h"
#include "computed_field/computed_field.h"
}
#include "computed_field/computed_field_private.hpp"
#include "computed_field/field_cache.hpp"
extern "C" {
#include "computed_field/computed_field_finite_element.h"
#include "general/callback_private.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "region/cmiss_region.h"
#include "region/cmiss_region_private.h"
#include "finite_element/finite_element_region.h"
#include "general/message.h"
}
#include <list>
#include <vector>

/*
Module types
------------
*/

FULL_DECLARE_CMISS_CALLBACK_TYPES(Cmiss_region_change, \
	struct Cmiss_region *, struct Cmiss_region_changes *);

/***************************************************************************//**
 * A region object which contains fields and child regions.
 * Other data can be attached using the any_object_list.
 */
struct Cmiss_region
{
	char *name;
	/* non-accessed pointer to parent region, or NULL if root */
	Cmiss_region *parent;
	/* accessed first child and next sibling for building region tree */
	Cmiss_region *first_child, *next_sibling;
	/* non-access pointer to previous sibling, if any */
	Cmiss_region *previous_sibling;
	
	/* fields owned by this region (or master) */
	struct MANAGER(Computed_field) *field_manager;
	void *field_manager_callback_id;
	struct FE_region *fe_region;
	int field_cache_size; // 1 more than highest field cache index given out
	std::list<Cmiss_field_cache_id> *field_caches; // all caches currently in use
		// for this region, needed to add value caches for new fields

	/* list of objects attached to region */
	struct LIST(Any_object) *any_object_list;

	/* increment/decrement change_level to nest changes. Message sent when zero */
	int change_level;
	/* number of hierarchical changes in progress on this region tree. A region's
	 * change_level will have one increment per ancestor hierarchical_change_level.
	 * Must be tracked to safely transfer when re-parenting regions */
	int hierarchical_change_level;
	Cmiss_region_changes changes;
	/* list of change callbacks */
	struct LIST(CMISS_CALLBACK_ITEM(Cmiss_region_change)) *change_callback_list;

	/* number of objects using this region */
	int access_count;
};

/*
Module functions
----------------
*/

DEFINE_CMISS_CALLBACK_MODULE_FUNCTIONS(Cmiss_region_change, void)

DEFINE_CMISS_CALLBACK_FUNCTIONS(Cmiss_region_change, \
	struct Cmiss_region *, struct Cmiss_region_changes *)

/***************************************************************************//**
 * Computed field manager callback. Asks fields of
 * parent region to propagate field changes if hierarchical.
 * Initially supports Cmiss_field_group.
 *
 * @param message  The changes to the fields in the region's manager.
 * @param region_void  Void pointer to changed region (not the parent).
 */
static void Cmiss_region_Computed_field_change(
	struct MANAGER_MESSAGE(Computed_field) *message, void *region_void)
{
	Cmiss_region *region = (Cmiss_region *)region_void;
	if (message && region)
	{
		int change_summary =
			MANAGER_MESSAGE_GET_CHANGE_SUMMARY(Computed_field)(message);
		if (change_summary & (MANAGER_CHANGE_RESULT(Computed_field) |
			MANAGER_CHANGE_ADD(Computed_field)))
		{
			Cmiss_region *parent = Cmiss_region_get_parent_internal(region);
			if (parent)
			{
				Computed_field_manager_propagate_hierarchical_field_changes(
					parent->field_manager, message);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_Computed_field_change.  Invalid argument(s)");
	}
	LEAVE;
}

/***************************************************************************//**
 * Forwards begin change cache to region fields.
 */
static int Cmiss_region_fields_begin_change(struct Cmiss_region *region)
{
	int return_code;
	if (region)
	{
		MANAGER_BEGIN_CACHE(Computed_field)(region->field_manager);
		FE_region_begin_change(region->fe_region);
		FE_region_begin_change(FE_region_get_data_FE_region(region->fe_region));
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Cmiss_region_fields_begin_change.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

/***************************************************************************//**
 * Forwards end change cache to region fields.
 */
static int Cmiss_region_fields_end_change(struct Cmiss_region *region)
{
	int return_code;
	if (region)
	{
		FE_region_end_change(FE_region_get_data_FE_region(region->fe_region));
		FE_region_end_change(region->fe_region);
		MANAGER_END_CACHE(Computed_field)(region->field_manager);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Cmiss_region_fields_end_change.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

namespace {

static int Cmiss_region_update(struct Cmiss_region *region)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Tells the clients of the <region> about changes of children or objects in this
region. No messages sent if change count positive or no changes have occurred.
==============================================================================*/
{
	int return_code;
	struct Cmiss_region_changes changes;

	ENTER(Cmiss_region_update);
	if (region)
	{
		if ((0 == region->change_level) && ((region->changes.children_changed) ||
			(region->changes.name_changed)))
		{
			if (0 != region->hierarchical_change_level)
			{
				display_message(WARNING_MESSAGE, "Cmiss_region_update.  Hierarchical change level mismatch");
			}
			changes = region->changes;
			/* must clear flags in the region before changes go out */
			region->changes.name_changed = 0;
			region->changes.children_changed = 0;
			region->changes.child_added = (struct Cmiss_region *)NULL;
			region->changes.child_removed = (struct Cmiss_region *)NULL;
			/* send the callbacks */
			CMISS_CALLBACK_LIST_CALL(Cmiss_region_change)(
				region->change_callback_list, region, &changes);
			// deaccess child pointers from message
			REACCESS(Cmiss_region)(&changes.child_added, NULL);
			REACCESS(Cmiss_region)(&changes.child_removed, NULL);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Cmiss_region_update.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_update */

/***************************************************************************//**
 * Internal-only implementation of Cmiss_region_find_child_by_name which does
 * not ACCESS the returned reference.
 * @see Cmiss_region_find_child_by_name
 */
struct Cmiss_region *Cmiss_region_find_child_by_name_internal(
	struct Cmiss_region *region, const char *name)
{
	struct Cmiss_region *child = NULL;
	if (region && name)
	{
		child = region->first_child;
		while (child)
		{
			if (0 == strcmp(child->name, name))
			{
				break;
			}
			child = child->next_sibling;
		}
	}
	return (child);
}

/***************************************************************************//**
 * Internal-only implementation of Cmiss_region_find_subregion_at_path which
 * does not ACCESS the returned reference.
 * @see Cmiss_region_find_subregion_at_path
 */
struct Cmiss_region *Cmiss_region_find_subregion_at_path_internal(
	struct Cmiss_region *region, const char *path)
{
	struct Cmiss_region *subregion = NULL;
	if (region && path)
	{
		subregion = region;
		char *path_copy = duplicate_string(path);
		char *child_name = path_copy;
		/* skip leading separator */
		if (child_name[0] == CMISS_REGION_PATH_SEPARATOR_CHAR)
		{
			child_name++;
		}
		char *child_name_end;
		while (subregion && (child_name_end =
			strchr(child_name, CMISS_REGION_PATH_SEPARATOR_CHAR)))
		{
			*child_name_end = '\0';
			subregion = Cmiss_region_find_child_by_name_internal(subregion, child_name);
			child_name = child_name_end + 1;
		}
		/* already found the subregion if there was a single trailing separator */
		if (subregion && (child_name[0] != '\0'))
		{
			subregion = Cmiss_region_find_child_by_name_internal(subregion, child_name);
		}
		DEALLOCATE(path_copy);
	}

	return (subregion);
}

/***************************************************************************//**
 * Private constructor. Creates an empty Cmiss_region with field containers.
 * Region is created with an access_count of 1; DEACCESS to destroy.
 * @base_region  Optional region to share element shape and basis data with.
 * If NULL, creates independent lists of shape and basis information.
 */
struct Cmiss_region *CREATE(Cmiss_region)(struct Cmiss_region *base_region)
{
	struct Cmiss_region *region;
	if (ALLOCATE(region, struct Cmiss_region, 1))
	{
		region->name = NULL;
		region->parent = NULL;
		region->first_child = NULL;
		region->next_sibling = NULL;
		region->previous_sibling = NULL;
		region->any_object_list = CREATE(LIST(Any_object))();
		region->change_level = 0;
		region->hierarchical_change_level = 0;
		region->changes.name_changed = 0;
		region->changes.children_changed = 0;
		region->changes.child_added = NULL;
		region->changes.child_removed = NULL;
		region->change_callback_list =
			CREATE(LIST(CMISS_CALLBACK_ITEM(Cmiss_region_change)))();
		region->field_manager = CREATE(MANAGER(Computed_field))();
		Computed_field_manager_set_region(region->field_manager, region);
		region->field_manager_callback_id = MANAGER_REGISTER(Computed_field)(
			Cmiss_region_Computed_field_change, (void *)region, region->field_manager);
		region->fe_region = CREATE(FE_region)(/*master_fe_region*/(FE_region *)0,
			base_region ? FE_region_get_basis_manager(base_region->fe_region) : 0,
			base_region ? FE_region_get_FE_element_shape_list(base_region->fe_region) : 0);
		FE_region_set_Cmiss_region_private(region->fe_region, region);
		ACCESS(FE_region)(region->fe_region);
		FE_region_add_callback(region->fe_region, Cmiss_region_FE_region_change, (void *)region);
		region->field_cache_size = 0;
		region->field_caches = new std::list<Cmiss_field_cache_id>();
		region->access_count = 1;
		if (!(region->any_object_list && region->change_callback_list &&
			region->field_manager && region->field_manager_callback_id &&
			region->fe_region))
		{
			display_message(ERROR_MESSAGE, "CREATE(Cmiss_region).  Could not build region");
			DEACCESS(Cmiss_region)(&region);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Cmiss_region).  Could not allocate memory");
	}
	return (region);
}

/** partial cleanup of region, needed by destructor and Cmiss_region_detach_fields_hierarchical */
static void Cmiss_region_detach_fields(struct Cmiss_region *region)
{
	if (region && region->field_manager_callback_id)
	{
		MANAGER_DEREGISTER(Computed_field)(region->field_manager_callback_id, region->field_manager);
		region->field_manager_callback_id = 0;
		DESTROY(MANAGER(Computed_field))(&(region->field_manager));
		FE_region_remove_callback(region->fe_region, Cmiss_region_FE_region_change, (void *)region);
		FE_region_set_Cmiss_region_private(region->fe_region, (Cmiss_region *)0);
		DEACCESS(FE_region)(&region->fe_region);
	}
}

/***************************************************************************//**
 * Destructor for Cmiss_region. Sets <*cmiss_region_address> to NULL.
 */
int DESTROY(Cmiss_region)(struct Cmiss_region **region_address)
{
	int return_code;
	struct Cmiss_region *region;
	if (region_address && (0 != (region = *region_address)))
	{
		if (0 == region->access_count)
		{
			delete region->field_caches;
			DESTROY(LIST(Any_object))(&(region->any_object_list));

			// destroy child list
			Cmiss_region *next_child = region->first_child;
			region->first_child = NULL;
			Cmiss_region *child;
			while ((child = next_child))
			{
				next_child = child->next_sibling;
				child->parent = NULL;
				child->next_sibling = NULL;
				child->previous_sibling = NULL;
				DEACCESS(Cmiss_region)(&child);
			}

			REACCESS(Cmiss_region)(&region->changes.child_added, NULL);
			REACCESS(Cmiss_region)(&region->changes.child_removed, NULL);

			DESTROY(LIST(CMISS_CALLBACK_ITEM(Cmiss_region_change)))(
				&(region->change_callback_list));

			Cmiss_region_detach_fields(region);

			if (region->name)
			{
				DEALLOCATE(region->name);
			}
			DEALLOCATE(*region_address);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Cmiss_region).  Non-zero access count");
			return_code = 0;
		}
		*region_address = (struct Cmiss_region *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Cmiss_region).  Missing Cmiss_region");
		return_code = 0;
	}
	return (return_code);
}

} // anonymous namespace

/*
Global functions
----------------
*/

DECLARE_OBJECT_FUNCTIONS(Cmiss_region)

int Cmiss_region_add_field_private(Cmiss_region_id region, Cmiss_field_id field)
{
	if (region && field)
	{
		int cache_index = region->field_cache_size;
		int number_in_manager = NUMBER_IN_MANAGER(Computed_field)(region->field_manager);
		if (cache_index == number_in_manager)
		{
			++region->field_cache_size;
		}
		else
		{
			std::vector<int> index_used(region->field_cache_size, 0);
			const Cmiss_set_Cmiss_field& fields = Computed_field_manager_get_fields(region->field_manager);
			for (Cmiss_set_Cmiss_field::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
			{
				index_used[Cmiss_field_get_cache_index_private((*iter))] = 1;
			}
			for (int i = 0; i < region->field_cache_size; i++)
			{
				if (!index_used[i])
				{
					cache_index = i;
					break;
				}
			}
		}
		if (Computed_field_add_to_manager_private(field, region->field_manager))
		{
			int i = 1;
			for (std::list<Cmiss_field_cache_id>::iterator iter = region->field_caches->begin();
				iter != region->field_caches->end(); ++iter)
			{
				Cmiss_field_cache_id field_cache = *iter;
				field_cache->setValueCache(cache_index, 0);
				++i;
			}
			Cmiss_field_set_cache_index_private(field, cache_index);
			return 1;
		}
	}
	return 0;
}

void Cmiss_region_clear_field_value_caches(Cmiss_region_id region, Cmiss_field_id field)
{
	int cacheIndex = Cmiss_field_get_cache_index_private(field);
	for (std::list<Cmiss_field_cache_id>::iterator iter = region->field_caches->begin();
		iter != region->field_caches->end(); ++iter)
	{
		Cmiss_field_cache_id cache = *iter;
		FieldValueCache *valueCache = cache->getValueCache(cacheIndex);
		if (valueCache)
		{
			valueCache->clear();
		}
	}
}

void Cmiss_region_detach_fields_hierarchical(struct Cmiss_region *region)
{
	if (region) 
	{
		Cmiss_region *child = region->first_child;
		while (child)
		{
			Cmiss_region_detach_fields_hierarchical(child);
			child = child->next_sibling;
		}
		Cmiss_region_detach_fields(region);
	}
}

struct Cmiss_region *Cmiss_region_create_internal(void)
{
	return CREATE(Cmiss_region)(/*base_region*/NULL);
}

struct Cmiss_region *Cmiss_region_create_region(struct Cmiss_region *base_region)
{
	if (!base_region)
		return 0;
	return CREATE(Cmiss_region)(base_region);
}

struct Cmiss_region *Cmiss_region_create_child(struct Cmiss_region *parent_region, 
	const char *name)
{
	struct Cmiss_region *region = NULL;
	if (parent_region)
	{
		region = Cmiss_region_create_region(parent_region);
		if ((!Cmiss_region_set_name(region, name)) ||
			(!Cmiss_region_append_child(parent_region, region)))
		{
			Cmiss_region_destroy(&region);
		}
	}
	return region;
}

struct Cmiss_region *Cmiss_region_create_subregion(
	struct Cmiss_region *top_region, const char *path)
{
	// Fails if a subregion exists at that path already
	struct Cmiss_region *region = Cmiss_region_find_child_by_name(top_region, path);
	if (region)
	{
		Cmiss_region_destroy(&region);
		return NULL;
	}
	if (top_region && path)
	{
		region = ACCESS(Cmiss_region)(top_region);
		char *path_copy = duplicate_string(path);
		char *child_name = path_copy;
		if (child_name[0] == CMISS_REGION_PATH_SEPARATOR_CHAR)
		{
			child_name++;
		}
		while (region && child_name && (child_name[0] != '\0'))
		{
			char *child_name_end = strchr(child_name, CMISS_REGION_PATH_SEPARATOR_CHAR);
			if (child_name_end)
			{
				*child_name_end = '\0';
			}
			Cmiss_region *child_region = Cmiss_region_find_child_by_name(region, child_name);
			if (!child_region)
			{
				child_region = Cmiss_region_create_child(region, child_name);
			}
			REACCESS(Cmiss_region)(&region, child_region);
			DEACCESS(Cmiss_region)(&child_region);
			if (child_name_end)
			{
				child_name = child_name_end + 1;
			}
			else
			{
				child_name = (char *)NULL;
			}
		}
		DEALLOCATE(path_copy);
	}
	return (region);
}

int Cmiss_region_clear_finite_elements(struct Cmiss_region *region)
{
	return FE_region_clear(Cmiss_region_get_FE_region(region),
		/*destroy_in_master*/0);
}

struct FE_region *Cmiss_region_get_FE_region(struct Cmiss_region *region)
{
	if (region)
		return region->fe_region;
	return 0;
}

struct MANAGER(Computed_field) *Cmiss_region_get_Computed_field_manager(
	struct Cmiss_region *region)
{
	if (region)
		return region->field_manager;
	return 0;
}

int Cmiss_region_get_field_cache_size(Cmiss_region_id region)
{
	if (region)
		return region->field_cache_size;
	return 0;
}

void Cmiss_region_add_field_cache(Cmiss_region_id region, Cmiss_field_cache_id cache)
{
	if (region && cache)
		region->field_caches->push_back(cache);
}

void Cmiss_region_remove_field_cache(Cmiss_region_id region,
	Cmiss_field_cache_id cache)
{
	if (region && cache)
		region->field_caches->remove(cache);
}

int Cmiss_field_module_begin_change(Cmiss_field_module_id field_module)
{
	return Cmiss_region_fields_begin_change(Cmiss_field_module_get_region_internal(field_module));
}

int Cmiss_field_module_end_change(Cmiss_field_module_id field_module)
{
	return Cmiss_region_fields_end_change(Cmiss_field_module_get_region_internal(field_module));
}

int Cmiss_field_module_define_all_faces(Cmiss_field_module_id field_module)
{
	return FE_region_define_faces(Cmiss_region_get_FE_region(
		Cmiss_field_module_get_region_internal(field_module)));
}

int Cmiss_region_begin_change(struct Cmiss_region *region)
{
	int return_code;

	ENTER(Cmiss_region_begin_change);
	if (region)
	{
		region->change_level++;
		Cmiss_region_fields_begin_change(region);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_begin_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_region_end_change(struct Cmiss_region *region)
{
	int return_code;

	ENTER(Cmiss_region_end_change);
	if (region)
	{
		if (0 < region->change_level)
		{
			Cmiss_region_fields_end_change(region);
			region->change_level--;
			if (0 == region->change_level)
			{
				Cmiss_region_update(region);
			}
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_region_end_change.  Change count is already zero");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_end_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

/***************************************************************************//**
 * Returns the sum of the hierarchical_change_level members of region and all
 * its ancestors. Equals the number of begin_change calls needed for new
 * children of region.
 */
static int Cmiss_region_get_sum_hierarchical_change_level(struct Cmiss_region *region)
{
	int sum_hierarchical_change_level = 0;
	while (region)
	{
		sum_hierarchical_change_level += region->hierarchical_change_level;
		region = region->parent;
	}
	return sum_hierarchical_change_level;
}

/***************************************************************************//**
 * Adds delta_change_level to change_level of region and all its descendents.
 * Begins or ends change cache as many times as magnitude of delta_change_level.
 */
static void Cmiss_region_tree_change(struct Cmiss_region *region,
	int delta_change_level)
{
	if (region)
	{
		for (int i = 0; i < delta_change_level; i++)
		{
			Cmiss_region_begin_change(region);
		}
		Cmiss_region *child = region->first_child;
		while (child)
		{
			Cmiss_region_tree_change(child, delta_change_level);
			child = child->next_sibling;
		}
		for (int i = 0; i > delta_change_level; i--)
		{
			Cmiss_region_end_change(region);
		}
	}
}

int Cmiss_region_begin_hierarchical_change(struct Cmiss_region *region)
{
	if (region)
	{
		region->hierarchical_change_level++;
		Cmiss_region_tree_change(region, +1);
		return 1;
	}
	return 0;
}

int Cmiss_region_end_hierarchical_change(struct Cmiss_region *region)
{
	if (region)
	{
		region->hierarchical_change_level--;
		Cmiss_region_tree_change(region, -1);
		return 1;
	}
	return 0;
}

int Cmiss_region_add_callback(struct Cmiss_region *region,
	CMISS_CALLBACK_FUNCTION(Cmiss_region_change) *function, void *user_data)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Adds a callback to <region> so that when it changes <function> is called with
<user_data>. <function> has 3 arguments, a struct Cmiss_region *, a
struct Cmiss_region_changes * and the void *user_data.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_region_add_callback);
	if (region && function)
	{
		if (CMISS_CALLBACK_LIST_ADD_CALLBACK(Cmiss_region_change)(
			region->change_callback_list, function, user_data))
		{
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_region_add_callback.  Could not add callback");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_add_callback.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_add_callback */

int Cmiss_region_remove_callback(struct Cmiss_region *region,
	CMISS_CALLBACK_FUNCTION(Cmiss_region_change) *function, void *user_data)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Removes the callback calling <function> with <user_data> from <region>.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_region_remove_callback);
	if (region && function)
	{
		if (CMISS_CALLBACK_LIST_REMOVE_CALLBACK(Cmiss_region_change)(
			region->change_callback_list, function, user_data))
		{
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_region_remove_callback.  Could not remove callback");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_remove_callback.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_remove_callback */

char *Cmiss_region_get_name(struct Cmiss_region *region)
{
	char *name = NULL;
	if (region && region->name)
	{
		name = duplicate_string(region->name); 
	}
	return (name);
}

int Cmiss_region_set_name(struct Cmiss_region *region, const char *name)
{
	int return_code = 0;
	if (region && name)
	{
		if (is_standard_object_name(name))
		{
			return_code = 1;
			if ((NULL == region->name) || strcmp(region->name, name))
			{
				if ((NULL == region->parent) ||
					(NULL == Cmiss_region_find_child_by_name_internal(region->parent, name)))
				{
					char *temp_name = duplicate_string(name);
					if (region->name)
					{
						DEALLOCATE(region->name);
					}
					region->name = temp_name;
					region->changes.name_changed = 1;
					Cmiss_region_update(region);
				}
				else
				{
					return_code = 0; // name is in use by sibling
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_region_set_name.  Invalid region name '%s'", name);
		}
	}
	return (return_code);
}

char *Cmiss_region_get_root_region_path(void)
{
	return duplicate_string(CMISS_REGION_PATH_SEPARATOR_STRING);
}

char *Cmiss_region_get_path(struct Cmiss_region *region)
{
	char *path = NULL;

	ENTER(Cmiss_region_get_path);
	if (region)
	{
		int error = 0;
		Cmiss_region* parent = region->parent;
		if (parent)
		{
			if ((path = Cmiss_region_get_path(parent)))
			{
				append_string(&path, region->name, &error);
			}
			else
			{
				DEALLOCATE(path);
				error = 1;
			}
		}
		append_string(&path, CMISS_REGION_PATH_SEPARATOR_STRING, &error);
	}
	LEAVE;
	
	return (path);
}

char *Cmiss_region_get_relative_path(struct Cmiss_region *region,
	struct Cmiss_region *other_region)
{
	char *path = NULL;

	ENTER(Cmiss_region_get_relative_path);
	if (region && other_region)
	{
		int error = 0;
		if (region != other_region) 
		{
			Cmiss_region* parent = region->parent;
			if (parent)
			{
				if ((path = Cmiss_region_get_relative_path(parent, other_region)))
				{
					append_string(&path, region->name, &error);
				}
				else
				{
					error = 1;
				}
			}
			else
			{
				error = 1;
			}
		}
		append_string(&path, CMISS_REGION_PATH_SEPARATOR_STRING, &error);
	}
	LEAVE;
	
	return (path);
}

struct Cmiss_region *Cmiss_region_get_parent(struct Cmiss_region *region)
{
	return (region && region->parent) ?
		ACCESS(Cmiss_region)(region->parent) : NULL;
}

struct Cmiss_region *Cmiss_region_get_parent_internal(struct Cmiss_region *region)
{
	if (region)
		return region->parent;
	return 0;
}

struct Cmiss_region *Cmiss_region_get_first_child(struct Cmiss_region *region)
{
	return (region && region->first_child) ?
		ACCESS(Cmiss_region)(region->first_child) : NULL;
}

struct Cmiss_region *Cmiss_region_get_next_sibling(struct Cmiss_region *region)
{
	return (region && region->next_sibling) ?
		ACCESS(Cmiss_region)(region->next_sibling) : NULL;
}

struct Cmiss_region *Cmiss_region_get_previous_sibling(struct Cmiss_region *region)
{
	return (region && region->previous_sibling) ?
		ACCESS(Cmiss_region)(region->previous_sibling) : NULL;
}

void Cmiss_region_reaccess_next_sibling(struct Cmiss_region **region_address)
{
	if (region_address && (*region_address))
	{
		REACCESS(Cmiss_region)(region_address, (*region_address)->next_sibling);
	}
}

int Cmiss_region_append_child(struct Cmiss_region *region,
	struct Cmiss_region *new_child)
{
	return Cmiss_region_insert_child_before(region, new_child, NULL);
}

int Cmiss_region_insert_child_before(struct Cmiss_region *region,
	struct Cmiss_region *new_child, struct Cmiss_region *ref_child)
{
	int return_code = 1;
	if (!(region && new_child &&
		((NULL == ref_child) || (ref_child->parent == region)) &&
		(!Cmiss_region_contains_subregion(new_child, region)) &&
		((NULL != new_child->name) && ((new_child->parent == region) ||
		(NULL == Cmiss_region_find_child_by_name_internal(region,
			new_child->name))))))
	{
		return_code = 0;
	}
	if (return_code)
	{
		int delta_change_level = Cmiss_region_get_sum_hierarchical_change_level(region);
		Cmiss_region_begin_change(region);
		if (new_child->parent)
		{
			delta_change_level -= Cmiss_region_get_sum_hierarchical_change_level(new_child->parent);
			Cmiss_region_remove_child(new_child->parent, new_child);
		}
		new_child->parent = region;
		if (ref_child)
		{
			new_child->next_sibling = ref_child;
			new_child->previous_sibling = ref_child->previous_sibling;
			ref_child->previous_sibling = new_child;
			if (new_child->previous_sibling)
			{
				new_child->previous_sibling->next_sibling = ACCESS(Cmiss_region)(new_child);
			}
			else
			{
				region->first_child = ACCESS(Cmiss_region)(new_child);
			}
		}
		else
		{
			if (region->first_child)
			{
				Cmiss_region *last_child = region->first_child;
				while (last_child->next_sibling)
				{
					last_child = last_child->next_sibling;
				}
				last_child->next_sibling = ACCESS(Cmiss_region)(new_child);
				new_child->previous_sibling = last_child;
			}
			else
			{
				region->first_child = ACCESS(Cmiss_region)(new_child);
			}
		}
		if (!region->changes.children_changed)
		{
			region->changes.children_changed = 1;
			region->changes.child_added = ACCESS(Cmiss_region)(new_child);
		}
		else
		{
			/* multiple changes */
			REACCESS(Cmiss_region)(&region->changes.child_added, NULL);
			REACCESS(Cmiss_region)(&region->changes.child_removed, NULL);
		}
		if (delta_change_level != 0)
		{
			Cmiss_region_tree_change(new_child, delta_change_level);
		}
		Cmiss_region_end_change(region);
	}
	return (return_code);
}

int Cmiss_region_remove_child(struct Cmiss_region *region,
	struct Cmiss_region *old_child)
{
	int return_code = 0;
	if (region && old_child)
	{
		if (old_child->parent == region)
		{
			int delta_change_level = Cmiss_region_get_sum_hierarchical_change_level(region);
			if (old_child == region->first_child)
			{
				region->first_child = old_child->next_sibling;
			}
			else
			{
				old_child->previous_sibling->next_sibling = old_child->next_sibling;
			}
			if (old_child->next_sibling)
			{
				old_child->next_sibling->previous_sibling = old_child->previous_sibling;
				old_child->next_sibling = NULL;
			}
			old_child->previous_sibling = NULL;
			old_child->parent = NULL;
			if (!region->changes.children_changed)
			{
				region->changes.children_changed = 1;
				region->changes.child_removed = ACCESS(Cmiss_region)(old_child);
			}
			else
			{
				/* multiple changes */
				REACCESS(Cmiss_region)(&region->changes.child_added, NULL);
				REACCESS(Cmiss_region)(&region->changes.child_removed, NULL);
			}
			if (delta_change_level != 0)
			{
				Cmiss_region_tree_change(old_child, delta_change_level);
			}
			Cmiss_region_update(region);
			DEACCESS(Cmiss_region)(&old_child);
			return_code = 1;
		}
	}
	return (return_code);
}

int Cmiss_region_contains_subregion(struct Cmiss_region *region,
	struct Cmiss_region *subregion)
{
	int return_code = 0;
	if (region && subregion)
	{
		do
		{
			if (subregion == region)
			{
				return_code = 1;
				break;
			}
		} while (NULL != (subregion = subregion->parent));
	}
	return (return_code);
}

struct Cmiss_region *Cmiss_region_find_child_by_name(
	struct Cmiss_region *region, const char *name)
{
	Cmiss_region *child = Cmiss_region_find_child_by_name_internal(region, name);
	if (child)
	{
		ACCESS(Cmiss_region)(child);
	}
	return (child);
}

struct Cmiss_region *Cmiss_region_find_subregion_at_path(
	struct Cmiss_region *region, const char *path)
{
	Cmiss_region *subregion =
		Cmiss_region_find_subregion_at_path_internal(region, path);
	if (subregion)
	{
		ACCESS(Cmiss_region)(subregion);
	}
	return (subregion);
}

Cmiss_field_id Cmiss_region_find_field_by_name(Cmiss_region_id region,
	const char *field_name)
{
	struct Cmiss_field *field = NULL;
	if (region && field_name)
	{
		field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
			(char *)field_name, Cmiss_region_get_Computed_field_manager(region));
		if (field)
		{
			ACCESS(Computed_field)(field);
		}
	}
	return (field);
}

int Cmiss_region_get_region_from_path_deprecated(struct Cmiss_region *region,
	const char *path, struct Cmiss_region **subregion_address)
{
	int return_code = 0;
	if (subregion_address)
	{
		*subregion_address =
			Cmiss_region_find_subregion_at_path_internal(region, path);
		if (NULL != *subregion_address)
		{
			return_code = 1;
		}
	}
	return (return_code);
}

/***************************************************************************//**
 * Returns a reference to the root region of this region.
 *
 * @param region  The region.
 * @return  Accessed reference to root region, or NULL if none.
 */
struct Cmiss_region *Cmiss_region_get_root(struct Cmiss_region *region)
{
	if (!region)
		return NULL;
	Cmiss_region *root = region;
	while (root->parent)
	{
		root = root->parent;
	}
	return ACCESS(Cmiss_region)(root);
}

int Cmiss_region_get_partial_region_path(struct Cmiss_region *root_region,
	const char *path, struct Cmiss_region **region_address,
	char **region_path_address,	char **remainder_address)
{
	char *child_name, *path_copy, *child_name_end, *child_name_start;
	int length, return_code;
	struct Cmiss_region *next_region, *region;

	ENTER(Cmiss_region_get_partial_region_path);
	if (root_region && path && region_address && region_path_address &&
		remainder_address)
	{
		return_code = 1;
		region = root_region;
		path_copy = duplicate_string(path);
		child_name = path_copy;
		/* skip leading separator */
		if (child_name[0] == CMISS_REGION_PATH_SEPARATOR_CHAR)
		{
			child_name++;
		}
		child_name_start = child_name;
		next_region = region;

		while (next_region && (*child_name != '\0'))
		{
			child_name_end = strchr(child_name, CMISS_REGION_PATH_SEPARATOR_CHAR);
			if (child_name_end)
			{
				*child_name_end = '\0';
			}
			if ((next_region = Cmiss_region_find_child_by_name_internal(region, child_name)))
			{
				region = next_region;
				if (child_name_end)
				{
					child_name = child_name_end + 1;
				}
				else
				{
					child_name = child_name + strlen(child_name);
				}
			}
			if (child_name_end)
			{
				*child_name_end = CMISS_REGION_PATH_SEPARATOR_CHAR;
			}
		}

		length = child_name - child_name_start;
		if ((length > 0) &&
			(*(child_name - 1) == CMISS_REGION_PATH_SEPARATOR_CHAR))
		{
			length--;
		}
		if (ALLOCATE(*region_path_address, char, length + 1))
		{
			strncpy(*region_path_address, child_name_start, length);
			(*region_path_address)[length] = '\0';
		}
		else
		{
			return_code = 0;
		}

		length = strlen(child_name);
		if (length == 0)
		{
			*remainder_address = NULL;
		}
		else
		{
			/* remove trailing '/' */
			if (child_name[length-1] == CMISS_REGION_PATH_SEPARATOR_CHAR)
			{
				length--;
			}
			if (ALLOCATE(*remainder_address, char, length + 1))
			{
				strncpy(*remainder_address, child_name, length);
				(*remainder_address)[length] = '\0';
			}
			else
			{
				return_code = 0;
			}
		}

		*region_address = region;
		DEALLOCATE(path_copy);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_get_partial_region_path.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_get_partial_region_path */

int Cmiss_region_list(struct Cmiss_region *region,
	int indent, int indent_increment)
/*******************************************************************************
LAST MODIFIED : 5 March 2003

DESCRIPTION :
Lists the Cmiss_region hierarchy starting from <region>. Contents are listed
indented from the left margin by <indent> spaces; this is incremented by
<indent_increment> for each child level.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_region_list);
	if (region && (0 <= indent) && (0 < indent_increment))
	{
		Cmiss_region *child = region->first_child;
		while (child)
		{
			display_message(INFORMATION_MESSAGE, "%*s%s : \n", indent, " ", child->name);
			Cmiss_region_list(child, indent + indent_increment, indent_increment);
			child = child->next_sibling;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "Cmiss_region_list.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_list */

int Cmiss_region_private_attach_any_object(struct Cmiss_region *region,
	struct Any_object *any_object)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Adds <any_object> to the list of objects attached to <region>.
This function is only externally visible to context objects.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_region_private_attach_any_object);
	if (region && any_object)
	{
		return_code =
			ADD_OBJECT_TO_LIST(Any_object)(any_object, region->any_object_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_private_attach_any_object.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_private_attach_any_object */

int Cmiss_region_private_detach_any_object(struct Cmiss_region *region,
	struct Any_object *any_object)
/*******************************************************************************
LAST MODIFIED : 29 October 2002

DESCRIPTION :
Removes <any_object> from the list of objects attached to <region>.
Note that only in the case that <any_object> is the exact Any_object stored in
<region> may it be cleaned up. In any other case the <any_object> passed in
must be cleaned up by the calling function.
This function is only externally visible to context objects.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_region_private_detach_any_object);
	if (region && any_object)
	{
		if (IS_OBJECT_IN_LIST(Any_object)(any_object, region->any_object_list))
		{
			return_code = REMOVE_OBJECT_FROM_LIST(Any_object)(any_object,
				region->any_object_list);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_region_private_detach_any_object.  Object is not in list");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_private_detach_any_object.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_private_detach_any_object */

struct LIST(Any_object) *
Cmiss_region_private_get_any_object_list(struct Cmiss_region *region)
/*******************************************************************************
LAST MODIFIED : 1 October 2002

DESCRIPTION :
Returns the list of objects, abstractly stored as struct Any_object from
<region>. It is important that this list not be modified directly.
This function is only externally visible to context objects.
==============================================================================*/
{
	struct LIST(Any_object) *any_object_list;

	ENTER(Cmiss_region_private_get_any_object_list);
	if (region)
	{
		any_object_list = region->any_object_list;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_private_get_any_object_list.  Missing region");
		any_object_list = (struct LIST(Any_object) *)NULL;
	}
	LEAVE;

	return (any_object_list);
} /* Cmiss_region_private_get_any_object_list */

/***************************************************************************//**
 * Returns field module container for this region's fields, which must be passed
 * to field factory create methods.
 *
 * @param region  The region from which to obtain the field module.
 * @return  Field module object.
 */
struct Cmiss_field_module *Cmiss_region_get_field_module(struct Cmiss_region *region)
{
	return Cmiss_field_module_create(region);
}

Cmiss_region_id Cmiss_region_access(Cmiss_region_id region)
{
	return (ACCESS(Cmiss_region)(region));
}

int Cmiss_region_destroy(Cmiss_region_id *region)
/*******************************************************************************
LAST MODIFIED : 3 January 2008

DESCRIPTION :
Destroys the <region> and sets the pointer to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_region_destroy);
	return_code = 0;
	if (region && *region)
	{
		return_code = DEACCESS(Cmiss_region)(region);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_destroy */

int Cmiss_region_can_merge(Cmiss_region_id target_region, Cmiss_region_id source_region)
{
	if (!target_region || !source_region)
		return 0;

	// check FE_regions
	FE_region *target_fe_region = Cmiss_region_get_FE_region(target_region);
	FE_region *source_fe_region = Cmiss_region_get_FE_region(source_region);
	if (!FE_region_can_merge(target_fe_region, source_fe_region))
	{
		char *target_path = Cmiss_region_get_path(target_region);
		char *source_path = Cmiss_region_get_path(source_region);
		display_message(ERROR_MESSAGE,
			"Cannot merge incompatible fields from source region %s into %s", source_path, target_path);
		DEALLOCATE(source_path);
		DEALLOCATE(target_path);
		return 0;
	}

	// check child regions can be merged
	Cmiss_region_id source_child = Cmiss_region_get_first_child(source_region);
	while (NULL != source_child)
	{
		Cmiss_region_id target_child = Cmiss_region_find_child_by_name(target_region, source_child->name);
		if (target_child)
		{
			if (!Cmiss_region_can_merge(target_child, source_child))
			{
				Cmiss_region_destroy(&target_child);
				Cmiss_region_destroy(&source_child);
				return 0;
			}
		}
		Cmiss_region_destroy(&target_child);
		Cmiss_region_reaccess_next_sibling(&source_child);
	}

	return 1;
}

/** currently just merges group fields */
static int Cmiss_region_merge_fields(Cmiss_region_id target_region,
	Cmiss_region_id source_region)
{
	int return_code = 1;
	Cmiss_field_module_id target_field_module = Cmiss_region_get_field_module(target_region);
	Cmiss_field_module_id source_field_module = Cmiss_region_get_field_module(source_region);
	Cmiss_field_iterator_id field_iter = Cmiss_field_module_create_field_iterator(source_field_module);
	Cmiss_field_id source_field = 0;
	while ((0 != (source_field = Cmiss_field_iterator_next_non_access(field_iter))) && return_code)
	{
		Cmiss_field_group_id source_group = Cmiss_field_cast_group(source_field);
		if (source_group)
		{
			char *name = Cmiss_field_get_name(source_field);
			Cmiss_field_id target_field = Cmiss_field_module_find_field_by_name(target_field_module, name);
			if (!target_field)
			{
				target_field = Cmiss_field_module_create_group(target_field_module);
				Cmiss_field_set_attribute_integer(target_field, CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 1);
				Cmiss_field_set_name(target_field, name);
			}
			Cmiss_field_group_id target_group = Cmiss_field_cast_group(target_field);
			if (target_field && (!target_group))
			{
				char *target_path = Cmiss_region_get_path(target_region);
				char *source_path = Cmiss_region_get_path(source_region);
				display_message(ERROR_MESSAGE,
					"Cannot merge group %s from source region %s into field using same name in %s", name, source_path, target_path);
				DEALLOCATE(source_path);
				DEALLOCATE(target_path);
			}
			else if (target_group)
			{
				// merge node groups
				for (int i = 0; i < 2; i++)
				{
					const char *nodeset_name = i ? "cmiss_data" : "cmiss_nodes";
					Cmiss_nodeset_id source_nodeset = Cmiss_field_module_find_nodeset_by_name(source_field_module, nodeset_name);
					Cmiss_field_node_group_id source_node_group = Cmiss_field_group_get_node_group(source_group, source_nodeset);
					if (source_node_group)
					{
						Cmiss_nodeset_group_id source_nodeset_group = Cmiss_field_node_group_get_nodeset(source_node_group);
						Cmiss_nodeset_id target_nodeset = Cmiss_field_module_find_nodeset_by_name(target_field_module, nodeset_name);
						Cmiss_field_node_group_id target_node_group = Cmiss_field_group_get_node_group(target_group, target_nodeset);
						if (!target_node_group)
						{
							target_node_group = Cmiss_field_group_create_node_group(target_group, target_nodeset);
						}
						Cmiss_nodeset_group_id target_nodeset_group = Cmiss_field_node_group_get_nodeset(target_node_group);

						Cmiss_node_iterator_id node_iter = Cmiss_nodeset_create_node_iterator(Cmiss_nodeset_group_base_cast(source_nodeset_group));
						Cmiss_node_id source_node = 0;
						while ((source_node = Cmiss_node_iterator_next_non_access(node_iter)) && return_code)
						{
							const int identifier =  Cmiss_node_get_identifier(source_node);
							Cmiss_node_id target_node = Cmiss_nodeset_find_node_by_identifier(Cmiss_nodeset_group_base_cast(target_nodeset_group), identifier);
							if (!target_node)
							{
								target_node = Cmiss_nodeset_find_node_by_identifier(target_nodeset, identifier);
								return_code = Cmiss_nodeset_group_add_node(target_nodeset_group, target_node);
							}
							Cmiss_node_destroy(&target_node);
						}
						Cmiss_node_iterator_destroy(&node_iter);

						Cmiss_nodeset_group_destroy(&target_nodeset_group);
						Cmiss_field_node_group_destroy(&target_node_group);
						Cmiss_nodeset_destroy(&target_nodeset);
						Cmiss_nodeset_group_destroy(&source_nodeset_group);
						Cmiss_field_node_group_destroy(&source_node_group);
					}
					Cmiss_nodeset_destroy(&source_nodeset);
				}
				// merge element groups
				for (int dimension = 3; 0 < dimension; --dimension)
				{
					Cmiss_mesh_id source_mesh = Cmiss_field_module_find_mesh_by_dimension(source_field_module, dimension);
					Cmiss_field_element_group_id source_element_group = Cmiss_field_group_get_element_group(source_group, source_mesh);
					if (source_element_group)
					{
						Cmiss_mesh_group_id source_mesh_group = Cmiss_field_element_group_get_mesh(source_element_group);
						Cmiss_mesh_id target_mesh = Cmiss_field_module_find_mesh_by_dimension(target_field_module, dimension);
						Cmiss_field_element_group_id target_element_group = Cmiss_field_group_get_element_group(target_group, target_mesh);
						if (!target_element_group)
						{
							target_element_group = Cmiss_field_group_create_element_group(target_group, target_mesh);
						}
						Cmiss_mesh_group_id target_mesh_group = Cmiss_field_element_group_get_mesh(target_element_group);

						Cmiss_element_iterator_id element_iter = Cmiss_mesh_create_element_iterator(Cmiss_mesh_group_base_cast(source_mesh_group));
						Cmiss_element_id source_element = 0;
						while ((source_element = Cmiss_element_iterator_next_non_access(element_iter)) && return_code)
						{
							const int identifier =  Cmiss_element_get_identifier(source_element);
							Cmiss_element_id target_element = Cmiss_mesh_find_element_by_identifier(Cmiss_mesh_group_base_cast(target_mesh_group), identifier);
							if (!target_element)
							{
								target_element = Cmiss_mesh_find_element_by_identifier(target_mesh, identifier);
								return_code = Cmiss_mesh_group_add_element(target_mesh_group, target_element);
							}
							Cmiss_element_destroy(&target_element);
						}
						Cmiss_element_iterator_destroy(&element_iter);

						Cmiss_mesh_group_destroy(&target_mesh_group);
						Cmiss_field_element_group_destroy(&target_element_group);
						Cmiss_mesh_destroy(&target_mesh);
						Cmiss_mesh_group_destroy(&source_mesh_group);
						Cmiss_field_element_group_destroy(&source_element_group);
					}
					Cmiss_mesh_destroy(&source_mesh);
				}
				Cmiss_field_group_destroy(&target_group);
			}
			else
			{
				return_code = 0;
			}
			Cmiss_field_destroy(&target_field);
			DEALLOCATE(name);
			Cmiss_field_group_destroy(&source_group);
		}
	}
	Cmiss_field_iterator_destroy(&field_iter);
	Cmiss_field_module_destroy(&source_field_module);
	Cmiss_field_module_destroy(&target_field_module);
	return return_code;
}

static int Cmiss_region_merge_private(Cmiss_region_id target_region,
	Cmiss_region_id source_region, Cmiss_region_id root_region)
{
	int return_code = 1;

	// merge FE_region
	FE_region *target_fe_region = Cmiss_region_get_FE_region(target_region);
	FE_region *source_fe_region = Cmiss_region_get_FE_region(source_region);
	if (!FE_region_merge(target_fe_region, source_fe_region, Cmiss_region_get_FE_region(root_region)))
	{
		char *target_path = Cmiss_region_get_path(target_region);
		char *source_path = Cmiss_region_get_path(source_region);
		display_message(ERROR_MESSAGE,
			"Could not merge source_region region %s into %s", source_path, target_path);
		DEALLOCATE(source_path);
		DEALLOCATE(target_path);
		return_code = 0;
	}

	if (!Cmiss_region_merge_fields(target_region, source_region))
	{
		return_code = 0;
	}

	// merge child regions
	Cmiss_region_id source_child = Cmiss_region_get_first_child(source_region);
	while (source_child && return_code)
	{
		Cmiss_region_id target_child = Cmiss_region_find_child_by_name(target_region, source_child->name);
		if (target_child)
		{
			return_code = Cmiss_region_merge_private(target_child, source_child, root_region);
		}
		else
		{
			return_code = Cmiss_region_append_child(target_region, source_child);
		}
		Cmiss_region_destroy(&target_child);
		Cmiss_region_reaccess_next_sibling(&source_child);
	}
	Cmiss_region_destroy(&source_child);
	return (return_code);
}

int Cmiss_region_merge(Cmiss_region_id target_region, Cmiss_region_id source_region)
{
	if (!target_region || !source_region)
		return 0;
	Cmiss_region_begin_hierarchical_change(target_region);
	int return_code = Cmiss_region_merge_private(target_region, source_region, /*root_region*/target_region);
	Cmiss_region_end_hierarchical_change(target_region);
	return return_code;
}
