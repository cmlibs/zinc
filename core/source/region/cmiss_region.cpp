/***************************************************************************//**
 * FILE : cmiss_region.cpp
 *
 * Definition of cmzn_region, container of fields for representing model data,
 * and child regions for building hierarchical models.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "zinc/element.h"
#include "zinc/fieldgroup.h"
#include "zinc/fieldmodule.h"
#include "zinc/fieldsubobjectgroup.h"
#include "zinc/node.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/field_cache.hpp"
#include "computed_field/field_module.hpp"
#include "computed_field/computed_field_finite_element.h"
#include "general/callback_private.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "region/cmiss_region.h"
#include "region/cmiss_region_private.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_region_private.h"
#include "general/message.h"
#include <algorithm>
#include <list>
#include <vector>

/*
Module types
------------
*/

FULL_DECLARE_CMZN_CALLBACK_TYPES(cmzn_region_change, \
	struct cmzn_region *, struct cmzn_region_changes *);

typedef std::list<cmzn_fieldmodulenotifier *> cmzn_fieldmodulenotifier_list;

/***************************************************************************//**
 * A region object which contains fields and child regions.
 * Other data can be attached using the any_object_list.
 */
struct cmzn_region
{
	char *name;
	/* non-accessed pointer to parent region, or NULL if root */
	cmzn_region *parent;
	/* accessed first child and next sibling for building region tree */
	cmzn_region *first_child, *next_sibling;
	/* non-access pointer to previous sibling, if any */
	cmzn_region *previous_sibling;

	/* fields owned by this region (or master) */
	struct MANAGER(Computed_field) *field_manager;
	void *field_manager_callback_id;
	struct FE_region *fe_region;
	int field_cache_size; // 1 more than highest field cache index given out
	// all field caches currently in use for this region, for clearing
	// when fields changed, and adding value caches for new fields.
	std::list<cmzn_fieldcache_id> *field_caches;

	/* list of objects attached to region */
	struct LIST(Any_object) *any_object_list;

	/* increment/decrement change_level to nest changes. Message sent when zero */
	int change_level;
	/* number of hierarchical changes in progress on this region tree. A region's
	 * change_level will have one increment per ancestor hierarchical_change_level.
	 * Must be tracked to safely transfer when re-parenting regions */
	int hierarchical_change_level;
	cmzn_region_changes changes;
	/* list of change callbacks */
	struct LIST(CMZN_CALLBACK_ITEM(cmzn_region_change)) *change_callback_list;

	// list of notifiers which receive field module callbacks
	cmzn_fieldmodulenotifier_list *notifier_list;

	/* number of objects using this region */
	int access_count;
};

/*
Module functions
----------------
*/

DEFINE_CMZN_CALLBACK_MODULE_FUNCTIONS(cmzn_region_change, void)

DEFINE_CMZN_CALLBACK_FUNCTIONS(cmzn_region_change, \
	struct cmzn_region *, struct cmzn_region_changes *)

/**
 * Computed field manager callback.
 * Calls notifier callbacks, propagates hierarchical field changes to
 * parent region (currently only for cmzn_field_group).
 *
 * @param message  The changes to the fields in the region's manager.
 * @param region_void  Void pointer to changed region (not the parent).
 */
static void cmzn_region_Computed_field_change(
	struct MANAGER_MESSAGE(Computed_field) *message, void *region_void)
{
	cmzn_region *region = (cmzn_region *)region_void;
	if (message && region)
	{
		int change_summary = MANAGER_MESSAGE_GET_CHANGE_SUMMARY(Computed_field)(message);
		// clear active field caches for changed fields
		if ((change_summary & MANAGER_CHANGE_RESULT(Computed_field)) &&
			(0 < region->field_caches->size()))
		{
			LIST(Computed_field) *changedFieldList =
				MANAGER_MESSAGE_GET_CHANGE_LIST(Computed_field)(message, MANAGER_CHANGE_RESULT(Computed_field));
			cmzn_fielditerator *iter = Computed_field_list_create_iterator(changedFieldList);
			cmzn_field *field;
			while (0 != (field = cmzn_fielditerator_next_non_access(iter)))
			{
				cmzn_region_clear_field_value_caches(region, field);
			}
			cmzn_fielditerator_destroy(&iter);
			DESTROY(LIST(Computed_field))(&changedFieldList);
		}
		if (0 < region->notifier_list->size())
		{
			cmzn_fieldmoduleevent_id event = cmzn_fieldmoduleevent::create(region);
			event->setChangeFlags(change_summary);
			event->setManagerMessage(message);
			FE_region_changes *changes = FE_region_changes::create(region->fe_region);
			event->setFeRegionChanges(changes);
			FE_region_changes::deaccess(changes);
			for (cmzn_fieldmodulenotifier_list::iterator iter = region->notifier_list->begin();
				iter != region->notifier_list->end(); ++iter)
			{
				(*iter)->notify(event);
			}
			cmzn_fieldmoduleevent::deaccess(event);
		}
		if (change_summary & (MANAGER_CHANGE_RESULT(Computed_field) |
			MANAGER_CHANGE_ADD(Computed_field)))
		{
			cmzn_region *parent = cmzn_region_get_parent_internal(region);
			if (parent)
			{
				Computed_field_manager_propagate_hierarchical_field_changes(
					parent->field_manager, message);
			}
		}
	}
}

/**
 * Forwards begin change cache to region fields.
 */
static int cmzn_region_fields_begin_change(struct cmzn_region *region)
{
	int return_code;
	if (region)
	{
		// reset field value caches so always re-evaluated. See cmzn_field::evaluate()
		for (std::list<cmzn_fieldcache_id>::iterator iter = region->field_caches->begin();
			iter != region->field_caches->end(); ++iter)
		{
			cmzn_fieldcache_id cache = *iter;
			cache->resetValueCacheEvaluationCounters();
		}
		MANAGER_BEGIN_CACHE(Computed_field)(region->field_manager);
		FE_region_begin_change(region->fe_region);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "cmzn_region_fields_begin_change.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

/**
 * Forwards end change cache to region fields.
 */
static int cmzn_region_fields_end_change(struct cmzn_region *region)
{
	int return_code;
	if (region)
	{
		FE_region_end_change(region->fe_region);
		MANAGER_END_CACHE(Computed_field)(region->field_manager);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "cmzn_region_fields_end_change.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

namespace {

static int cmzn_region_update(struct cmzn_region *region)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Tells the clients of the <region> about changes of children or objects in this
region. No messages sent if change count positive or no changes have occurred.
==============================================================================*/
{
	int return_code;
	struct cmzn_region_changes changes;

	ENTER(cmzn_region_update);
	if (region)
	{
		if ((0 == region->change_level) && ((region->changes.children_changed) ||
			(region->changes.name_changed)))
		{
			if (0 != region->hierarchical_change_level)
			{
				display_message(WARNING_MESSAGE, "cmzn_region_update.  Hierarchical change level mismatch");
			}
			changes = region->changes;
			/* must clear flags in the region before changes go out */
			region->changes.name_changed = 0;
			region->changes.children_changed = 0;
			region->changes.child_added = (struct cmzn_region *)NULL;
			region->changes.child_removed = (struct cmzn_region *)NULL;
			/* send the callbacks */
			CMZN_CALLBACK_LIST_CALL(cmzn_region_change)(
				region->change_callback_list, region, &changes);
			// deaccess child pointers from message
			REACCESS(cmzn_region)(&changes.child_added, NULL);
			REACCESS(cmzn_region)(&changes.child_removed, NULL);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "cmzn_region_update.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_region_update */

/***************************************************************************//**
 * Internal-only implementation of cmzn_region_find_child_by_name which does
 * not ACCESS the returned reference.
 * @see cmzn_region_find_child_by_name
 */
struct cmzn_region *cmzn_region_find_child_by_name_internal(
	struct cmzn_region *region, const char *name)
{
	struct cmzn_region *child = NULL;
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

/**
 * Internal-only implementation of cmzn_region_find_subregion_at_path which
 * does not ACCESS the returned reference.
 * @see cmzn_region_find_subregion_at_path
 */
struct cmzn_region *cmzn_region_find_subregion_at_path_internal(
	struct cmzn_region *region, const char *path)
{
	struct cmzn_region *subregion = NULL;
	if (region && path)
	{
		subregion = region;
		char *path_copy = duplicate_string(path);
		char *child_name = path_copy;
		/* skip leading separator */
		if (child_name[0] == CMZN_REGION_PATH_SEPARATOR_CHAR)
			++child_name;
		char *child_name_end;
		while (subregion && (child_name_end =
			strchr(child_name, CMZN_REGION_PATH_SEPARATOR_CHAR)))
		{
			*child_name_end = '\0';
			subregion = cmzn_region_find_child_by_name_internal(subregion, child_name);
			child_name = child_name_end + 1;
		}
		/* already found the subregion if there was a single trailing separator */
		if (subregion && (child_name[0] != '\0'))
			subregion = cmzn_region_find_child_by_name_internal(subregion, child_name);
		DEALLOCATE(path_copy);
	}
	return (subregion);
}

/***************************************************************************//**
 * Private constructor. Creates an empty cmzn_region with field containers.
 * Region is created with an access_count of 1; DEACCESS to destroy.
 * @base_region  Optional region to share element shape and basis data with.
 * If NULL, creates independent lists of shape and basis information.
 */
struct cmzn_region *CREATE(cmzn_region)(struct cmzn_region *base_region)
{
	struct cmzn_region *region;
	if (ALLOCATE(region, struct cmzn_region, 1))
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
			CREATE(LIST(CMZN_CALLBACK_ITEM(cmzn_region_change)))();
		region->notifier_list = new cmzn_fieldmodulenotifier_list();
		region->field_manager = CREATE(MANAGER(Computed_field))();
		Computed_field_manager_set_region(region->field_manager, region);
		region->field_manager_callback_id = MANAGER_REGISTER(Computed_field)(
			cmzn_region_Computed_field_change, (void *)region, region->field_manager);
		region->fe_region = FE_region_create(
			base_region ? FE_region_get_basis_manager(base_region->fe_region) : 0,
			base_region ? FE_region_get_FE_element_shape_list(base_region->fe_region) : 0);
		FE_region_set_cmzn_region_private(region->fe_region, region);
		region->field_cache_size = 0;
		region->field_caches = new std::list<cmzn_fieldcache_id>();
		region->access_count = 1;
		if (!(region->any_object_list && region->change_callback_list &&
			region->field_manager && region->field_manager_callback_id &&
			region->fe_region))
		{
			display_message(ERROR_MESSAGE, "CREATE(cmzn_region).  Could not build region");
			DEACCESS(cmzn_region)(&region);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(cmzn_region).  Could not allocate memory");
	}
	return (region);
}

/** partial cleanup of region, needed by destructor and cmzn_region_detach_fields_hierarchical */
static void cmzn_region_detach_fields(struct cmzn_region *region)
{
	if (region && region->field_manager_callback_id)
	{
		MANAGER_DEREGISTER(Computed_field)(region->field_manager_callback_id, region->field_manager);
		region->field_manager_callback_id = 0;
		// clear region pointer otherwise get updates when finite element fields destroyed
		FE_region_set_cmzn_region_private(region->fe_region, (cmzn_region *)0);
		DESTROY(MANAGER(Computed_field))(&(region->field_manager));
		DEACCESS(FE_region)(&region->fe_region);
	}
}

/**
 * Destructor for cmzn_region. Sets <*cmiss_region_address> to NULL.
 */
int DESTROY(cmzn_region)(struct cmzn_region **region_address)
{
	int return_code;
	struct cmzn_region *region;
	if (region_address && (0 != (region = *region_address)))
	{
		if (0 == region->access_count)
		{
			// first notify clients as they call some region/fieldmodule APIs
			for (cmzn_fieldmodulenotifier_list::iterator iter = region->notifier_list->begin();
				iter != region->notifier_list->end(); ++iter)
			{
				cmzn_fieldmodulenotifier *notifier = *iter;
				notifier->regionDestroyed();
				cmzn_fieldmodulenotifier::deaccess(notifier);
			}
			delete region->notifier_list;
			region->notifier_list = 0;

			delete region->field_caches;
			DESTROY(LIST(Any_object))(&(region->any_object_list));

			// destroy child list
			cmzn_region *next_child = region->first_child;
			region->first_child = NULL;
			cmzn_region *child;
			while ((child = next_child))
			{
				next_child = child->next_sibling;
				child->parent = NULL;
				child->next_sibling = NULL;
				child->previous_sibling = NULL;
				DEACCESS(cmzn_region)(&child);
			}

			REACCESS(cmzn_region)(&region->changes.child_added, NULL);
			REACCESS(cmzn_region)(&region->changes.child_removed, NULL);

			DESTROY(LIST(CMZN_CALLBACK_ITEM(cmzn_region_change)))(
				&(region->change_callback_list));

			cmzn_region_detach_fields(region);

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
				"DESTROY(cmzn_region).  Non-zero access count");
			return_code = 0;
		}
		*region_address = (struct cmzn_region *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(cmzn_region).  Missing cmzn_region");
		return_code = 0;
	}
	return (return_code);
}

} // anonymous namespace

/*
Global functions
----------------
*/

DECLARE_OBJECT_FUNCTIONS(cmzn_region)

int cmzn_region_add_field_private(cmzn_region_id region, cmzn_field_id field)
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
			const cmzn_set_cmzn_field& fields = Computed_field_manager_get_fields(region->field_manager);
			for (cmzn_set_cmzn_field::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
			{
				index_used[cmzn_field_get_cache_index_private((*iter))] = 1;
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
			for (std::list<cmzn_fieldcache_id>::iterator iter = region->field_caches->begin();
				iter != region->field_caches->end(); ++iter)
			{
				cmzn_fieldcache_id field_cache = *iter;
				field_cache->setValueCache(cache_index, 0);
				++i;
			}
			cmzn_field_set_cache_index_private(field, cache_index);
			return 1;
		}
	}
	return 0;
}

void cmzn_region_clear_field_value_caches(cmzn_region_id region, cmzn_field_id field)
{
	int cacheIndex = cmzn_field_get_cache_index_private(field);
	for (std::list<cmzn_fieldcache_id>::iterator iter = region->field_caches->begin();
		iter != region->field_caches->end(); ++iter)
	{
		cmzn_fieldcache_id cache = *iter;
		FieldValueCache *valueCache = cache->getValueCache(cacheIndex);
		if (valueCache)
		{
			valueCache->clear();
		}
	}
}

void cmzn_region_detach_fields_hierarchical(struct cmzn_region *region)
{
	if (region)
	{
		cmzn_region *child = region->first_child;
		while (child)
		{
			cmzn_region_detach_fields_hierarchical(child);
			child = child->next_sibling;
		}
		cmzn_region_detach_fields(region);
	}
}

struct cmzn_region *cmzn_region_create_internal(void)
{
	return CREATE(cmzn_region)(/*base_region*/NULL);
}

struct cmzn_region *cmzn_region_create_region(struct cmzn_region *base_region)
{
	if (!base_region)
		return 0;
	return CREATE(cmzn_region)(base_region);
}

struct cmzn_region *cmzn_region_create_child(struct cmzn_region *parent_region,
	const char *name)
{
	struct cmzn_region *region = NULL;
	if (parent_region)
	{
		region = cmzn_region_create_region(parent_region);
		if ((!cmzn_region_set_name(region, name)) ||
			(!cmzn_region_append_child(parent_region, region)))
		{
			cmzn_region_destroy(&region);
		}
	}
	return region;
}

struct cmzn_region *cmzn_region_create_subregion(
	struct cmzn_region *top_region, const char *path)
{
	// Fails if a subregion exists at that path already
	cmzn_region *region = cmzn_region_find_subregion_at_path_internal(top_region, path);
	if (region)
		return 0;
	if (top_region && path)
	{
		region = ACCESS(cmzn_region)(top_region);
		char *path_copy = duplicate_string(path);
		char *child_name = path_copy;
		if (child_name[0] == CMZN_REGION_PATH_SEPARATOR_CHAR)
			child_name++;
		while (region && child_name && (child_name[0] != '\0'))
		{
			char *child_name_end = strchr(child_name, CMZN_REGION_PATH_SEPARATOR_CHAR);
			if (child_name_end)
				*child_name_end = '\0';
			cmzn_region *child_region = cmzn_region_find_child_by_name(region, child_name);
			if (!child_region)
				child_region = cmzn_region_create_child(region, child_name);
			REACCESS(cmzn_region)(&region, child_region);
			DEACCESS(cmzn_region)(&child_region);
			if (child_name_end)
				child_name = child_name_end + 1;
			else
				child_name = (char *)0;
		}
		DEALLOCATE(path_copy);
	}
	return (region);
}

int cmzn_region_clear_finite_elements(struct cmzn_region *region)
{
	return FE_region_clear(cmzn_region_get_FE_region(region));
}

struct FE_region *cmzn_region_get_FE_region(struct cmzn_region *region)
{
	if (region)
		return region->fe_region;
	return 0;
}

struct MANAGER(Computed_field) *cmzn_region_get_Computed_field_manager(
	struct cmzn_region *region)
{
	if (region)
		return region->field_manager;
	return 0;
}

int cmzn_region_get_field_cache_size(cmzn_region_id region)
{
	if (region)
		return region->field_cache_size;
	return 0;
}

void cmzn_region_add_field_cache(cmzn_region_id region, cmzn_fieldcache_id cache)
{
	if (region && cache)
		region->field_caches->push_back(cache);
}

void cmzn_region_remove_field_cache(cmzn_region_id region,
	cmzn_fieldcache_id cache)
{
	if (region && cache)
		region->field_caches->remove(cache);
}

int cmzn_fieldmodule_begin_change(cmzn_fieldmodule_id field_module)
{
	return cmzn_region_fields_begin_change(cmzn_fieldmodule_get_region_internal(field_module));
}

int cmzn_fieldmodule_end_change(cmzn_fieldmodule_id field_module)
{
	return cmzn_region_fields_end_change(cmzn_fieldmodule_get_region_internal(field_module));
}

int cmzn_fieldmodule_define_all_faces(cmzn_fieldmodule_id field_module)
{
	return FE_region_define_faces(cmzn_region_get_FE_region(
		cmzn_fieldmodule_get_region_internal(field_module)));
}

int cmzn_region_begin_change(struct cmzn_region *region)
{
	int return_code;

	ENTER(cmzn_region_begin_change);
	if (region)
	{
		region->change_level++;
		cmzn_region_fields_begin_change(region);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_region_begin_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int cmzn_region_end_change(struct cmzn_region *region)
{
	int return_code;

	ENTER(cmzn_region_end_change);
	if (region)
	{
		if (0 < region->change_level)
		{
			cmzn_region_fields_end_change(region);
			region->change_level--;
			if (0 == region->change_level)
			{
				cmzn_region_update(region);
			}
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmzn_region_end_change.  Change count is already zero");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_region_end_change.  Invalid argument(s)");
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
static int cmzn_region_get_sum_hierarchical_change_level(struct cmzn_region *region)
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
static void cmzn_region_tree_change(struct cmzn_region *region,
	int delta_change_level)
{
	if (region)
	{
		for (int i = 0; i < delta_change_level; i++)
		{
			cmzn_region_begin_change(region);
		}
		cmzn_region *child = region->first_child;
		while (child)
		{
			cmzn_region_tree_change(child, delta_change_level);
			child = child->next_sibling;
		}
		for (int i = 0; i > delta_change_level; i--)
		{
			cmzn_region_end_change(region);
		}
	}
}

int cmzn_region_begin_hierarchical_change(struct cmzn_region *region)
{
	if (region)
	{
		region->hierarchical_change_level++;
		cmzn_region_tree_change(region, +1);
		return 1;
	}
	return 0;
}

int cmzn_region_end_hierarchical_change(struct cmzn_region *region)
{
	if (region)
	{
		region->hierarchical_change_level--;
		cmzn_region_tree_change(region, -1);
		return 1;
	}
	return 0;
}

int cmzn_region_add_callback(struct cmzn_region *region,
	CMZN_CALLBACK_FUNCTION(cmzn_region_change) *function, void *user_data)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Adds a callback to <region> so that when it changes <function> is called with
<user_data>. <function> has 3 arguments, a struct cmzn_region *, a
struct cmzn_region_changes * and the void *user_data.
==============================================================================*/
{
	int return_code;

	ENTER(cmzn_region_add_callback);
	if (region && function)
	{
		if (CMZN_CALLBACK_LIST_ADD_CALLBACK(cmzn_region_change)(
			region->change_callback_list, function, user_data))
		{
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmzn_region_add_callback.  Could not add callback");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_region_add_callback.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_region_add_callback */

int cmzn_region_remove_callback(struct cmzn_region *region,
	CMZN_CALLBACK_FUNCTION(cmzn_region_change) *function, void *user_data)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Removes the callback calling <function> with <user_data> from <region>.
==============================================================================*/
{
	int return_code;

	ENTER(cmzn_region_remove_callback);
	if (region && function)
	{
		if (CMZN_CALLBACK_LIST_REMOVE_CALLBACK(cmzn_region_change)(
			region->change_callback_list, function, user_data))
		{
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmzn_region_remove_callback.  Could not remove callback");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_region_remove_callback.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_region_remove_callback */

char *cmzn_region_get_name(struct cmzn_region *region)
{
	char *name = NULL;
	if (region && region->name)
	{
		name = duplicate_string(region->name);
	}
	return (name);
}

int cmzn_region_set_name(struct cmzn_region *region, const char *name)
{
	if (region && name)
	{
		if ((0 == region->name) || (0 != strcmp(region->name, name)))
		{
			if ((0 == region->parent) ||
				(0 == cmzn_region_find_child_by_name_internal(region->parent, name)))
			{
				char *temp_name = duplicate_string(name);
				if (region->name)
					DEALLOCATE(region->name);
				region->name = temp_name;
				region->changes.name_changed = 1;
				cmzn_region_update(region);
			}
			else
				return CMZN_ERROR_ARGUMENT; // name is in use by sibling
		}
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

char *cmzn_region_get_root_region_path(void)
{
	return duplicate_string(CMZN_REGION_PATH_SEPARATOR_STRING);
}

char *cmzn_region_get_path(struct cmzn_region *region)
{
	char *path = NULL;

	ENTER(cmzn_region_get_path);
	if (region)
	{
		int error = 0;
		cmzn_region* parent = region->parent;
		if (parent)
		{
			if ((path = cmzn_region_get_path(parent)))
			{
				append_string(&path, region->name, &error);
			}
			else
			{
				DEALLOCATE(path);
				error = 1;
			}
		}
		append_string(&path, CMZN_REGION_PATH_SEPARATOR_STRING, &error);
	}
	LEAVE;

	return (path);
}

char *cmzn_region_get_relative_path(struct cmzn_region *region,
	struct cmzn_region *other_region)
{
	char *path = NULL;

	ENTER(cmzn_region_get_relative_path);
	if (region && other_region)
	{
		int error = 0;
		if (region != other_region)
		{
			cmzn_region* parent = region->parent;
			if (parent)
			{
				if ((path = cmzn_region_get_relative_path(parent, other_region)))
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
		append_string(&path, CMZN_REGION_PATH_SEPARATOR_STRING, &error);
	}
	LEAVE;

	return (path);
}

struct cmzn_region *cmzn_region_get_parent(struct cmzn_region *region)
{
	return (region && region->parent) ?
		ACCESS(cmzn_region)(region->parent) : NULL;
}

struct cmzn_region *cmzn_region_get_parent_internal(struct cmzn_region *region)
{
	if (region)
		return region->parent;
	return 0;
}

struct cmzn_region *cmzn_region_get_first_child(struct cmzn_region *region)
{
	return (region && region->first_child) ?
		ACCESS(cmzn_region)(region->first_child) : NULL;
}

struct cmzn_region *cmzn_region_get_next_sibling(struct cmzn_region *region)
{
	return (region && region->next_sibling) ?
		ACCESS(cmzn_region)(region->next_sibling) : NULL;
}

struct cmzn_region *cmzn_region_get_previous_sibling(struct cmzn_region *region)
{
	return (region && region->previous_sibling) ?
		ACCESS(cmzn_region)(region->previous_sibling) : NULL;
}

void cmzn_region_reaccess_next_sibling(struct cmzn_region **region_address)
{
	if (region_address && (*region_address))
	{
		REACCESS(cmzn_region)(region_address, (*region_address)->next_sibling);
	}
}

int cmzn_region_append_child(struct cmzn_region *region,
	struct cmzn_region *new_child)
{
	return cmzn_region_insert_child_before(region, new_child, NULL);
}

int cmzn_region_insert_child_before(struct cmzn_region *region,
	struct cmzn_region *new_child, struct cmzn_region *ref_child)
{
	int return_code = 1;
	if (!(region && new_child &&
		((NULL == ref_child) || (ref_child->parent == region)) &&
		(!cmzn_region_contains_subregion(new_child, region)) &&
		((NULL != new_child->name) && ((new_child->parent == region) ||
		(NULL == cmzn_region_find_child_by_name_internal(region,
			new_child->name))))))
	{
		return_code = 0;
	}
	if (return_code)
	{
		int delta_change_level = cmzn_region_get_sum_hierarchical_change_level(region);
		cmzn_region_begin_change(region);
		if (new_child->parent)
		{
			delta_change_level -= cmzn_region_get_sum_hierarchical_change_level(new_child->parent);
			cmzn_region_remove_child(new_child->parent, new_child);
		}
		new_child->parent = region;
		if (ref_child)
		{
			new_child->next_sibling = ref_child;
			new_child->previous_sibling = ref_child->previous_sibling;
			ref_child->previous_sibling = new_child;
			if (new_child->previous_sibling)
			{
				new_child->previous_sibling->next_sibling = ACCESS(cmzn_region)(new_child);
			}
			else
			{
				region->first_child = ACCESS(cmzn_region)(new_child);
			}
		}
		else
		{
			if (region->first_child)
			{
				cmzn_region *last_child = region->first_child;
				while (last_child->next_sibling)
				{
					last_child = last_child->next_sibling;
				}
				last_child->next_sibling = ACCESS(cmzn_region)(new_child);
				new_child->previous_sibling = last_child;
			}
			else
			{
				region->first_child = ACCESS(cmzn_region)(new_child);
			}
		}
		if (!region->changes.children_changed)
		{
			region->changes.children_changed = 1;
			region->changes.child_added = ACCESS(cmzn_region)(new_child);
		}
		else
		{
			/* multiple changes */
			REACCESS(cmzn_region)(&region->changes.child_added, NULL);
			REACCESS(cmzn_region)(&region->changes.child_removed, NULL);
		}
		if (delta_change_level != 0)
		{
			cmzn_region_tree_change(new_child, delta_change_level);
		}
		cmzn_region_end_change(region);
	}
	return (return_code);
}

int cmzn_region_remove_child(struct cmzn_region *region,
	struct cmzn_region *old_child)
{
	if (region && old_child)
	{
		if (old_child->parent == region)
		{
			cmzn_region_begin_change(region);
			Computed_field_manager_subregion_removed(region->field_manager, old_child);
			int delta_change_level = cmzn_region_get_sum_hierarchical_change_level(region);
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
				region->changes.child_removed = ACCESS(cmzn_region)(old_child);
			}
			else
			{
				/* multiple changes */
				REACCESS(cmzn_region)(&region->changes.child_added, NULL);
				REACCESS(cmzn_region)(&region->changes.child_removed, NULL);
			}
			if (delta_change_level != 0)
			{
				cmzn_region_tree_change(old_child, delta_change_level);
			}
			cmzn_region_update(region);
			DEACCESS(cmzn_region)(&old_child);
			cmzn_region_end_change(region);
			return CMZN_OK;
		}
	}
	return CMZN_ERROR_ARGUMENT;
}

bool cmzn_region_contains_subregion(struct cmzn_region *region,
	struct cmzn_region *subregion)
{
	if (region && subregion)
	{
		do
		{
			if (subregion == region)
				return true;
		} while (0 != (subregion = subregion->parent));
	}
	return false;
}

struct cmzn_region *cmzn_region_find_child_by_name(
	struct cmzn_region *region, const char *name)
{
	cmzn_region *child = cmzn_region_find_child_by_name_internal(region, name);
	if (child)
	{
		ACCESS(cmzn_region)(child);
	}
	return (child);
}

struct cmzn_region *cmzn_region_find_subregion_at_path(
	struct cmzn_region *region, const char *path)
{
	cmzn_region *subregion =
		cmzn_region_find_subregion_at_path_internal(region, path);
	if (subregion)
		ACCESS(cmzn_region)(subregion);
	return (subregion);
}

cmzn_field_id cmzn_region_find_field_by_name(cmzn_region_id region,
	const char *field_name)
{
	struct cmzn_field *field = NULL;
	if (region && field_name)
	{
		field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
			(char *)field_name, cmzn_region_get_Computed_field_manager(region));
		if (field)
		{
			ACCESS(Computed_field)(field);
		}
	}
	return (field);
}

int cmzn_region_get_region_from_path_deprecated(struct cmzn_region *region,
	const char *path, struct cmzn_region **subregion_address)
{
	int return_code = 0;
	if (subregion_address)
	{
		*subregion_address =
			cmzn_region_find_subregion_at_path_internal(region, path);
		if (NULL != *subregion_address)
		{
			return_code = 1;
		}
	}
	return (return_code);
}

struct cmzn_region *cmzn_region_get_root(struct cmzn_region *region)
{
	if (!region)
		return NULL;
	cmzn_region *root = region;
	while (root->parent)
	{
		root = root->parent;
	}
	return ACCESS(cmzn_region)(root);
}

bool cmzn_region_is_root(struct cmzn_region *region)
{
	return region && (0 == region->parent);
}

int cmzn_region_get_partial_region_path(struct cmzn_region *root_region,
	const char *path, struct cmzn_region **region_address,
	char **region_path_address, char **remainder_address)
{
	char *child_name, *path_copy, *child_name_end, *child_name_start;
	int length, return_code;
	struct cmzn_region *next_region, *region;

	ENTER(cmzn_region_get_partial_region_path);
	if (root_region && path && region_address && region_path_address &&
		remainder_address)
	{
		return_code = 1;
		region = root_region;
		path_copy = duplicate_string(path);
		child_name = path_copy;
		/* skip leading separator */
		if (child_name[0] == CMZN_REGION_PATH_SEPARATOR_CHAR)
		{
			child_name++;
		}
		child_name_start = child_name;
		next_region = region;

		while (next_region && (*child_name != '\0'))
		{
			child_name_end = strchr(child_name, CMZN_REGION_PATH_SEPARATOR_CHAR);
			if (child_name_end)
			{
				*child_name_end = '\0';
			}
			if ((next_region = cmzn_region_find_child_by_name_internal(region, child_name)))
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
				*child_name_end = CMZN_REGION_PATH_SEPARATOR_CHAR;
			}
		}

		length = child_name - child_name_start;
		if ((length > 0) &&
			(*(child_name - 1) == CMZN_REGION_PATH_SEPARATOR_CHAR))
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

		length = static_cast<int>(strlen(child_name));
		if (length == 0)
		{
			*remainder_address = NULL;
		}
		else
		{
			/* remove trailing '/' */
			if (child_name[length-1] == CMZN_REGION_PATH_SEPARATOR_CHAR)
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
			"cmzn_region_get_partial_region_path.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_region_get_partial_region_path */

int cmzn_region_list(struct cmzn_region *region,
	int indent, int indent_increment)
/*******************************************************************************
LAST MODIFIED : 5 March 2003

DESCRIPTION :
Lists the cmzn_region hierarchy starting from <region>. Contents are listed
indented from the left margin by <indent> spaces; this is incremented by
<indent_increment> for each child level.
==============================================================================*/
{
	int return_code;

	ENTER(cmzn_region_list);
	if (region && (0 <= indent) && (0 < indent_increment))
	{
		cmzn_region *child = region->first_child;
		while (child)
		{
			display_message(INFORMATION_MESSAGE, "%*s%s : \n", indent, " ", child->name);
			cmzn_region_list(child, indent + indent_increment, indent_increment);
			child = child->next_sibling;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "cmzn_region_list.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_region_list */

int cmzn_region_private_attach_any_object(struct cmzn_region *region,
	struct Any_object *any_object)
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Adds <any_object> to the list of objects attached to <region>.
This function is only externally visible to context objects.
==============================================================================*/
{
	int return_code;

	ENTER(cmzn_region_private_attach_any_object);
	if (region && any_object)
	{
		return_code =
			ADD_OBJECT_TO_LIST(Any_object)(any_object, region->any_object_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_region_private_attach_any_object.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_region_private_attach_any_object */

int cmzn_region_private_detach_any_object(struct cmzn_region *region,
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

	ENTER(cmzn_region_private_detach_any_object);
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
				"cmzn_region_private_detach_any_object.  Object is not in list");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_region_private_detach_any_object.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_region_private_detach_any_object */

struct LIST(Any_object) *
cmzn_region_private_get_any_object_list(struct cmzn_region *region)
/*******************************************************************************
LAST MODIFIED : 1 October 2002

DESCRIPTION :
Returns the list of objects, abstractly stored as struct Any_object from
<region>. It is important that this list not be modified directly.
This function is only externally visible to context objects.
==============================================================================*/
{
	struct LIST(Any_object) *any_object_list;

	ENTER(cmzn_region_private_get_any_object_list);
	if (region)
	{
		any_object_list = region->any_object_list;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_region_private_get_any_object_list.  Missing region");
		any_object_list = (struct LIST(Any_object) *)NULL;
	}
	LEAVE;

	return (any_object_list);
} /* cmzn_region_private_get_any_object_list */

/***************************************************************************//**
 * Returns field module container for this region's fields, which must be passed
 * to field factory create methods.
 *
 * @param region  The region from which to obtain the field module.
 * @return  Field module object.
 */
struct cmzn_fieldmodule *cmzn_region_get_fieldmodule(struct cmzn_region *region)
{
	return cmzn_fieldmodule_create(region);
}

cmzn_region_id cmzn_region_access(cmzn_region_id region)
{
	if (region)
		return ACCESS(cmzn_region)(region);
	return 0;
}

int cmzn_region_destroy(cmzn_region_id *region)
/*******************************************************************************
LAST MODIFIED : 3 January 2008

DESCRIPTION :
Destroys the <region> and sets the pointer to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(cmzn_region_destroy);
	return_code = 0;
	if (region && *region)
	{
		return_code = DEACCESS(cmzn_region)(region);
	}
	LEAVE;

	return (return_code);
} /* cmzn_region_destroy */

int cmzn_region_can_merge(cmzn_region_id target_region, cmzn_region_id source_region)
{
	if (!target_region || !source_region)
		return 0;

	// check FE_regions
	FE_region *target_fe_region = cmzn_region_get_FE_region(target_region);
	FE_region *source_fe_region = cmzn_region_get_FE_region(source_region);
	if (!FE_region_can_merge(target_fe_region, source_fe_region))
	{
		char *target_path = cmzn_region_get_path(target_region);
		char *source_path = cmzn_region_get_path(source_region);
		display_message(ERROR_MESSAGE,
			"Cannot merge source region %s into %s", source_path, target_path);
		DEALLOCATE(source_path);
		DEALLOCATE(target_path);
		return 0;
	}

	// check child regions can be merged
	cmzn_region_id source_child = cmzn_region_get_first_child(source_region);
	while (NULL != source_child)
	{
		cmzn_region_id target_child = cmzn_region_find_child_by_name(target_region, source_child->name);
		if (target_child)
		{
			if (!cmzn_region_can_merge(target_child, source_child))
			{
				cmzn_region_destroy(&target_child);
				cmzn_region_destroy(&source_child);
				return 0;
			}
		}
		cmzn_region_destroy(&target_child);
		cmzn_region_reaccess_next_sibling(&source_child);
	}

	return 1;
}

/** currently just merges group fields */
static int cmzn_region_merge_fields(cmzn_region_id target_region,
	cmzn_region_id source_region)
{
	int return_code = 1;
	cmzn_fieldmodule_id target_field_module = cmzn_region_get_fieldmodule(target_region);
	cmzn_fieldmodule_id source_field_module = cmzn_region_get_fieldmodule(source_region);
	cmzn_fielditerator_id field_iter = cmzn_fieldmodule_create_fielditerator(source_field_module);
	cmzn_field_id source_field = 0;
	while ((0 != (source_field = cmzn_fielditerator_next_non_access(field_iter))) && return_code)
	{
		cmzn_field_group_id source_group = cmzn_field_cast_group(source_field);
		if (source_group)
		{
			char *name = cmzn_field_get_name(source_field);
			cmzn_field_id target_field = cmzn_fieldmodule_find_field_by_name(target_field_module, name);
			if (!target_field)
			{
				target_field = cmzn_fieldmodule_create_field_group(target_field_module);
				cmzn_field_set_managed(target_field, true);
				cmzn_field_set_name(target_field, name);
			}
			cmzn_field_group_id target_group = cmzn_field_cast_group(target_field);
			if (target_field && (!target_group))
			{
				char *target_path = cmzn_region_get_path(target_region);
				char *source_path = cmzn_region_get_path(source_region);
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
					cmzn_field_domain_type nodeset_domain_type = i ? CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS : CMZN_FIELD_DOMAIN_TYPE_NODES;
					cmzn_nodeset_id source_nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(source_field_module, nodeset_domain_type);
					cmzn_field_node_group_id source_node_group = cmzn_field_group_get_field_node_group(source_group, source_nodeset);
					if (source_node_group)
					{
						cmzn_nodeset_group_id source_nodeset_group = cmzn_field_node_group_get_nodeset_group(source_node_group);
						cmzn_nodeset_id target_nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(target_field_module, nodeset_domain_type);
						cmzn_field_node_group_id target_node_group = cmzn_field_group_get_field_node_group(target_group, target_nodeset);
						if (!target_node_group)
						{
							target_node_group = cmzn_field_group_create_field_node_group(target_group, target_nodeset);
						}
						cmzn_nodeset_group_id target_nodeset_group = cmzn_field_node_group_get_nodeset_group(target_node_group);

						cmzn_nodeiterator_id node_iter = cmzn_nodeset_create_nodeiterator(cmzn_nodeset_group_base_cast(source_nodeset_group));
						cmzn_node_id source_node = 0;
						while ((source_node = cmzn_nodeiterator_next_non_access(node_iter)) && return_code)
						{
							const int identifier =  cmzn_node_get_identifier(source_node);
							cmzn_node_id target_node = cmzn_nodeset_find_node_by_identifier(cmzn_nodeset_group_base_cast(target_nodeset_group), identifier);
							if (!target_node)
							{
								target_node = cmzn_nodeset_find_node_by_identifier(target_nodeset, identifier);
								return_code = cmzn_nodeset_group_add_node(target_nodeset_group, target_node);
							}
							cmzn_node_destroy(&target_node);
						}
						cmzn_nodeiterator_destroy(&node_iter);

						cmzn_nodeset_group_destroy(&target_nodeset_group);
						cmzn_field_node_group_destroy(&target_node_group);
						cmzn_nodeset_destroy(&target_nodeset);
						cmzn_nodeset_group_destroy(&source_nodeset_group);
						cmzn_field_node_group_destroy(&source_node_group);
					}
					cmzn_nodeset_destroy(&source_nodeset);
				}
				// merge element groups
				for (int dimension = 3; 0 < dimension; --dimension)
				{
					cmzn_mesh_id source_mesh = cmzn_fieldmodule_find_mesh_by_dimension(source_field_module, dimension);
					cmzn_field_element_group_id source_element_group = cmzn_field_group_get_field_element_group(source_group, source_mesh);
					if (source_element_group)
					{
						cmzn_mesh_group_id source_mesh_group = cmzn_field_element_group_get_mesh_group(source_element_group);
						cmzn_mesh_id target_mesh = cmzn_fieldmodule_find_mesh_by_dimension(target_field_module, dimension);
						cmzn_field_element_group_id target_element_group = cmzn_field_group_get_field_element_group(target_group, target_mesh);
						if (!target_element_group)
						{
							target_element_group = cmzn_field_group_create_field_element_group(target_group, target_mesh);
						}
						cmzn_mesh_group_id target_mesh_group = cmzn_field_element_group_get_mesh_group(target_element_group);

						cmzn_elementiterator_id element_iter = cmzn_mesh_create_elementiterator(cmzn_mesh_group_base_cast(source_mesh_group));
						cmzn_element_id source_element = 0;
						while ((source_element = cmzn_elementiterator_next_non_access(element_iter)) && return_code)
						{
							const int identifier =  cmzn_element_get_identifier(source_element);
							cmzn_element_id target_element = cmzn_mesh_find_element_by_identifier(cmzn_mesh_group_base_cast(target_mesh_group), identifier);
							if (!target_element)
							{
								target_element = cmzn_mesh_find_element_by_identifier(target_mesh, identifier);
								return_code = cmzn_mesh_group_add_element(target_mesh_group, target_element);
							}
							cmzn_element_destroy(&target_element);
						}
						cmzn_elementiterator_destroy(&element_iter);

						cmzn_mesh_group_destroy(&target_mesh_group);
						cmzn_field_element_group_destroy(&target_element_group);
						cmzn_mesh_destroy(&target_mesh);
						cmzn_mesh_group_destroy(&source_mesh_group);
						cmzn_field_element_group_destroy(&source_element_group);
					}
					cmzn_mesh_destroy(&source_mesh);
				}
				cmzn_field_group_destroy(&target_group);
			}
			else
			{
				return_code = 0;
			}
			cmzn_field_destroy(&target_field);
			DEALLOCATE(name);
			cmzn_field_group_destroy(&source_group);
		}
	}
	cmzn_fielditerator_destroy(&field_iter);
	cmzn_fieldmodule_destroy(&source_field_module);
	cmzn_fieldmodule_destroy(&target_field_module);
	return return_code;
}

static int cmzn_region_merge_private(cmzn_region_id target_region,
	cmzn_region_id source_region, cmzn_region_id root_region)
{
	int return_code = 1;

	// merge FE_region
	FE_region *target_fe_region = cmzn_region_get_FE_region(target_region);
	FE_region *source_fe_region = cmzn_region_get_FE_region(source_region);
	if (!FE_region_merge(target_fe_region, source_fe_region, cmzn_region_get_FE_region(root_region)))
	{
		char *target_path = cmzn_region_get_path(target_region);
		char *source_path = cmzn_region_get_path(source_region);
		display_message(ERROR_MESSAGE,
			"Could not merge source_region region %s into %s", source_path, target_path);
		DEALLOCATE(source_path);
		DEALLOCATE(target_path);
		return_code = 0;
	}

	if (!cmzn_region_merge_fields(target_region, source_region))
	{
		return_code = 0;
	}

	// merge child regions
	cmzn_region_id source_child = cmzn_region_get_first_child(source_region);
	while (source_child && return_code)
	{
		cmzn_region_id target_child = cmzn_region_find_child_by_name(target_region, source_child->name);
		if (target_child)
		{
			return_code = cmzn_region_merge_private(target_child, source_child, root_region);
			cmzn_region_reaccess_next_sibling(&source_child);
		}
		else
		{
			cmzn_region_id sibling_region = cmzn_region_get_next_sibling(source_child);
			return_code = cmzn_region_append_child(target_region, source_child);
			cmzn_region_destroy(&source_child);
			source_child = sibling_region;
		}
		cmzn_region_destroy(&target_child);
	}
	cmzn_region_destroy(&source_child);
	return (return_code);
}

int cmzn_region_merge(cmzn_region_id target_region, cmzn_region_id source_region)
{
	if (!target_region || !source_region)
		return 0;
	cmzn_region_begin_hierarchical_change(target_region);
	int return_code = cmzn_region_merge_private(target_region, source_region, /*root_region*/target_region);
	cmzn_region_end_hierarchical_change(target_region);
	return return_code;
}

void cmzn_region_add_fieldmodulenotifier(cmzn_region *region,
	cmzn_fieldmodulenotifier *notifier)
{
	if (region && notifier)
		region->notifier_list->push_back(notifier->access());
}

void cmzn_region_remove_fieldmodulenotifier(cmzn_region *region,
	cmzn_fieldmodulenotifier *notifier)
{
	if (region && notifier)
	{
		cmzn_fieldmodulenotifier_list::iterator iter = std::find(
			region->notifier_list->begin(), region->notifier_list->end(), notifier);
		if (iter != region->notifier_list->end())
		{
			cmzn_fieldmodulenotifier::deaccess(notifier);
			region->notifier_list->erase(iter);
		}
	}
}

/**
 * Ensures there is an up-to-date computed field wrapper for <fe_field>.
 * @param fe_field  Field to wrap.
 * @param fieldmodule_void  Field module to contain wrapped FE_field.
 */
static int FE_field_to_Computed_field_change(struct FE_field *fe_field,
	int change, void *fieldmodule_void)
{
	cmzn_fieldmodule *fieldmodule = reinterpret_cast<cmzn_fieldmodule*>(fieldmodule_void);
	if (change & (CHANGE_LOG_OBJECT_ADDED(FE_field) |
		CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_field) |
		CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_field)))
	{
		cmzn_region *region = cmzn_fieldmodule_get_region_internal(fieldmodule);
		char *field_name = NULL;
		GET_NAME(FE_field)(fe_field, &field_name);
		bool update_wrapper = (0 != (change & (CHANGE_LOG_OBJECT_ADDED(FE_field) |
			CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_field))));
		cmzn_field *existing_wrapper = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
			field_name, cmzn_region_get_Computed_field_manager(region));
		if (existing_wrapper && !Computed_field_wraps_fe_field(existing_wrapper, (void *)fe_field))
		{
			existing_wrapper = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_wraps_fe_field, (void *)fe_field,
				cmzn_region_get_Computed_field_manager(region));
			update_wrapper = true;
		}
		if (update_wrapper)
		{
			if (existing_wrapper)
			{
				cmzn_fieldmodule_set_replace_field(fieldmodule, existing_wrapper);
			}
			else
			{
				cmzn_fieldmodule_set_field_name(fieldmodule, field_name);
				struct Coordinate_system *coordinate_system = get_FE_field_coordinate_system(fe_field);
				if (coordinate_system)
					cmzn_fieldmodule_set_coordinate_system(fieldmodule, *coordinate_system);
			}
			cmzn_field_id field = Computed_field_create_finite_element_internal(fieldmodule, fe_field);
			cmzn_field_set_managed(field, true);
			cmzn_field_destroy(&field);
			char *new_field_name = 0;
			GET_NAME(FE_field)(fe_field, &new_field_name);
			if (strcmp(new_field_name, field_name))
			{
				display_message(WARNING_MESSAGE, "Renamed finite element field %s to %s as another field is already using that name.",
					field_name, new_field_name);
			}
			DEALLOCATE(new_field_name);
		}
		DEALLOCATE(field_name);
	}
	return 1;
}

void cmzn_region_FE_region_change(cmzn_region *region)
{
	if (region)
	{
		FE_region *fe_region = region->fe_region;
		struct CHANGE_LOG(FE_field) *fe_field_changes = FE_region_get_FE_field_changes(fe_region);
		int field_change_summary;
		CHANGE_LOG_GET_CHANGE_SUMMARY(FE_field)(fe_field_changes, &field_change_summary);
		bool check_field_wrappers = (0 != (field_change_summary & (~CHANGE_LOG_OBJECT_REMOVED(FE_field))));
		bool add_cmiss_number_field = FE_region_need_add_cmiss_number_field(fe_region);
		bool add_xi_field = FE_region_need_add_xi_field(fe_region);
		if (check_field_wrappers || add_cmiss_number_field || add_xi_field)
		{
			cmzn_fieldmodule_id fieldmodule = cmzn_region_get_fieldmodule(region);
			MANAGER_BEGIN_CACHE(Computed_field)(region->field_manager);

			if (check_field_wrappers)
			{
				CHANGE_LOG_FOR_EACH_OBJECT(FE_field)(fe_field_changes,
					FE_field_to_Computed_field_change, (void *)fieldmodule);
			}
			if (add_cmiss_number_field)
			{
				const char *cmiss_number_field_name = "cmiss_number";
				cmzn_field_id field = cmzn_fieldmodule_find_field_by_name(fieldmodule, cmiss_number_field_name);
				if (!field)
				{
					field = Computed_field_create_cmiss_number(fieldmodule);
					cmzn_field_set_name(field, cmiss_number_field_name);
					cmzn_field_set_managed(field, true);
				}
				cmzn_field_destroy(&field);
			}
			if (add_xi_field)
			{
				cmzn_field_id xi_field = cmzn_fieldmodule_get_or_create_xi_field(fieldmodule);
				cmzn_field_destroy(&xi_field);
			}
			// force field update for changes to nodes/elements etc.:
			MANAGER_EXTERNAL_CHANGE(Computed_field)(region->field_manager);
			MANAGER_END_CACHE(Computed_field)(region->field_manager);
			cmzn_fieldmodule_destroy(&fieldmodule);
		}
#if 0
		if (field_change_summary & CHANGE_LOG_OBJECT_REMOVED(FE_field))
		{
			/* Currently we do nothing as the computed field wrapper is destroyed
				before the FE_field is removed from the manager.  This is not necessary
				and this response could be to delete the wrapper. */
		}
#endif
	}
}
