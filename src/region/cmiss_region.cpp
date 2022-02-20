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
#include "opencmiss/zinc/fieldgroup.h"
#include "opencmiss/zinc/fieldmodule.h"
#include "opencmiss/zinc/fieldsubobjectgroup.h"
#include "opencmiss/zinc/mesh.h"
#include "opencmiss/zinc/node.h"
#include "opencmiss/zinc/nodeset.h"
#include "opencmiss/zinc/scene.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_apply.hpp"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/field_cache.hpp"
#include "computed_field/field_derivative.hpp"
#include "computed_field/field_module.hpp"
#include "computed_field/computed_field_finite_element.h"
#include "context/context.hpp"
#include "general/callback_private.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "graphics/scene.hpp"
#include "region/cmiss_region.hpp"
#include "finite_element/finite_element_basis.hpp"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_region_private.h"
#include "general/message.h"
#include <algorithm>
#include <vector>

/*
Module types
------------
*/

FULL_DECLARE_CMZN_CALLBACK_TYPES(cmzn_region_change, \
	struct cmzn_region *, cmzn_region_changes *);

/*
Module functions
----------------
*/

DEFINE_CMZN_CALLBACK_MODULE_FUNCTIONS(cmzn_region_change, void)

DEFINE_CMZN_CALLBACK_FUNCTIONS(cmzn_region_change, \
	struct cmzn_region *, cmzn_region_changes *)

/*
Class methods
-------------
*/

cmzn_region::cmzn_region(cmzn_context* contextIn) :
	name(nullptr),
	context(contextIn),
	parent(nullptr),
	first_child(nullptr),
	next_sibling(nullptr),
	previous_sibling(nullptr),
	field_manager(CREATE(MANAGER(Computed_field))()),
	field_manager_callback_id(MANAGER_REGISTER(Computed_field)(
		cmzn_region::Computed_field_change, (void *)this, this->field_manager)),
	fe_region(nullptr),
	field_cache_size(0),
	scene(nullptr),
	fieldModifyCounter(0),
	change_level(0),
	hierarchical_change_level(0),
	change_callback_list(CREATE(LIST(CMZN_CALLBACK_ITEM(cmzn_region_change)))()),
	access_count(1)
{
	Computed_field_manager_set_region(this->field_manager, this);
	// cmzn_region must be fully constructed before creating FE_region
	// get any existing region from context to share element bases and shapes with
	cmzn_region *baseRegion = contextIn->getBaseRegion();
	this->fe_region = new FE_region(this, (baseRegion) ? baseRegion->fe_region : nullptr);
	// create scene last as technically builds on FE_region
	this->scene = cmzn_scene::create(this, contextIn->getGraphicsmodule());
}

cmzn_region::~cmzn_region()
{
	// first notify clients as they call some region/fieldmodule APIs
	for (cmzn_fieldmodulenotifier_list::iterator iter = this->notifier_list.begin();
		iter != this->notifier_list.end(); ++iter)
	{
		cmzn_fieldmodulenotifier *notifier = *iter;
		notifier->regionDestroyed();
		cmzn_fieldmodulenotifier::deaccess(notifier);
	}

	DESTROY(LIST(CMZN_CALLBACK_ITEM(cmzn_region_change)))(&(this->change_callback_list));

	// release field derivative pointers to this region
	const int size = static_cast<int>(this->fieldDerivatives.size());
	for (int i = 0; i < size; ++i)
		if (this->fieldDerivatives[i])
			this->fieldDerivatives[i]->setRegionAndCacheIndexPrivate();

	// GRC move to changes object?
	cmzn_region::deaccess(this->changes.child_added);
	cmzn_region::deaccess(this->changes.child_removed);

	// destroy child list
	cmzn_region *next_child = this->first_child;
	this->first_child = NULL;
	cmzn_region *child;
	while ((child = next_child))
	{
		next_child = child->next_sibling;
		child->parent = NULL;
		child->next_sibling = NULL;
		child->previous_sibling = NULL;
		cmzn_region::deaccess(child);
	}

	this->detachFields();

	if (this->context)
		this->context->removeRegion(this);
	if (this->name)
		DEALLOCATE(this->name);
}

cmzn_region *cmzn_region::create(cmzn_context* contextIn)
{
	if (!contextIn)
	{
		display_message(ERROR_MESSAGE, "cmzn_region::create.  Missing context");
		return nullptr;
	}
	cmzn_region *region = new cmzn_region(contextIn);
	if ((region->change_callback_list) &&
		(region->field_manager) &&
		(region->field_manager_callback_id) &&
		(region->scene) &&
		(region->fe_region))
	{
		return region;
	}
	display_message(ERROR_MESSAGE, "cmzn_region::create.  Failed");
	delete region;
	return nullptr;
}

/** partial cleanup of region, needed by region and context destructors */
void cmzn_region::detachFields()
{
	if (this->field_manager)
	{
		// cease receiving field manager callbacks otherwise get problems from fields being destroyed
		// e.g. selection field accessed by scene in region
		if (this->field_manager_callback_id)
		{
			MANAGER_DEREGISTER(Computed_field)(this->field_manager_callback_id, this->field_manager);
			this->field_manager_callback_id = 0;
		}
		// clear region pointer otherwise get updates when finite element fields destroyed
		this->fe_region->clearRegionPrivate();
		if (this->scene)
		{
			this->beginChange();
			this->scene->detachFromOwner();
			cmzn_scene::deaccess(this->scene);
			this->endChange();
		}
		DESTROY(MANAGER(Computed_field))(&(this->field_manager));
		this->field_manager = 0;
		DEACCESS(FE_region)(&this->fe_region);
	}
}

/**
 * Computed field manager callback.
 * Calls notifier callbacks, propagates hierarchical field changes to
 * parent region (currently only for cmzn_field_group).
 *
 * @param message  The changes to the fields in the region's manager.
 * @param region_void  Void pointer to changed region (not the parent).
 */
void cmzn_region::Computed_field_change(
	struct MANAGER_MESSAGE(Computed_field) *message, void *region_void)
{
	cmzn_region *region = (cmzn_region *)region_void;
	// avoid if region is being destroyed (access_count == 0)
	// otherwise cmzn_fieldmoduleevent will access and destroy region again
	if (message && region && (region->access_count > 0))
	{
		int change_summary = MANAGER_MESSAGE_GET_CHANGE_SUMMARY(Computed_field)(message);
		// clear active field caches for changed fields
		if ((change_summary & MANAGER_CHANGE_RESULT(Computed_field)) &&
			(0 < region->field_caches.size()))
		{
			LIST(Computed_field) *changedFieldList =
				MANAGER_MESSAGE_GET_CHANGE_LIST(Computed_field)(message, MANAGER_CHANGE_RESULT(Computed_field));
			cmzn_fielditerator *iter = Computed_field_list_create_iterator(changedFieldList);
			cmzn_field *field;
			while (0 != (field = cmzn_fielditerator_next_non_access(iter)))
			{
				region->clearFieldValueCaches(field);
			}
			cmzn_fielditerator_destroy(&iter);
			DESTROY(LIST(Computed_field))(&changedFieldList);
		}
		if (0 < region->notifier_list.size())
		{
			cmzn_fieldmoduleevent_id event = cmzn_fieldmoduleevent::create(region);
			event->setChangeFlags(change_summary);
			event->setManagerMessage(message);
			FE_region_changes *changes = FE_region_changes::create(region->fe_region);
			event->setFeRegionChanges(changes);
			FE_region_changes::deaccess(changes);
			for (cmzn_fieldmodulenotifier_list::iterator iter = region->notifier_list.begin();
				iter != region->notifier_list.end(); ++iter)
			{
				(*iter)->notify(event);
			}
			cmzn_fieldmoduleevent::deaccess(event);
		}
		if (change_summary & (MANAGER_CHANGE_RESULT(Computed_field) |
			MANAGER_CHANGE_ADD(Computed_field)))
		{
			cmzn_region *parentRegion = region->getParent();
			if (parentRegion)
			{
				Computed_field_manager_propagate_hierarchical_field_changes(
					parentRegion->field_manager, message);
			}
		}
	}
}

/**
 * Tells the clients of the <region> about changes of children or objects in this
 * region. No messages sent if change count positive or no changes have occurred.
 */
void cmzn_region::updateClients()
{
	cmzn_region_changes changes;
	if ((0 == this->change_level) && ((this->changes.children_changed) ||
		(this->changes.name_changed)))
	{
		if (0 != this->hierarchical_change_level)
		{
			display_message(WARNING_MESSAGE, "cmzn_region_update.  Hierarchical change level mismatch");
		}
		changes = this->changes;
		this->changes.clear();  // next changes in the region must be cleared before notification
		/* send the callbacks */
		CMZN_CALLBACK_LIST_CALL(cmzn_region_change)(
			this->change_callback_list, this, &changes);
		// deaccess child pointers from message
		cmzn_region::deaccess(changes.child_added);
		cmzn_region::deaccess(changes.child_removed);
	}
}

/**
 * Forwards begin change cache to region fields.
 */
void cmzn_region::beginChangeFields()
{
	// reset field value caches so always re-evaluated. See cmzn_field::evaluate()
	for (std::list<cmzn_fieldcache_id>::iterator iter = this->field_caches.begin();
		iter != this->field_caches.end(); ++iter)
	{
		cmzn_fieldcache_id cache = *iter;
		cache->resetValueCacheEvaluationCounters();
	}
	MANAGER_BEGIN_CACHE(Computed_field)(this->field_manager);
	FE_region_begin_change(this->fe_region);
}

/**
 * Forwards end change cache to region fields.
 */
void cmzn_region::endChangeFields()
{
	FE_region_end_change(this->fe_region);
	MANAGER_END_CACHE(Computed_field)(this->field_manager);
}

void cmzn_region::clearCachedChanges()
{
	this->changes.clear();
	this->fe_region->clearCachedChanges();
}

int cmzn_region::endChange()
{
	if (0 < this->change_level)
	{
		this->endChangeFields();
		--(this->change_level);
		if (0 == this->change_level)
			this->updateClients();
		return CMZN_OK;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Region::endChange.  Change level is already zero");
		return CMZN_ERROR_GENERAL;
	}
}

/**
 * Returns the sum of the hierarchical_change_level members of region and all
 * its ancestors. Equals the number of begin_change calls needed for new
 * children of region.
 */
int cmzn_region::getSumHierarchicalChangeLevel() const
{
	int sum_hierarchical_change_level = 0;
	const cmzn_region *region = this;
	while (region)
	{
		sum_hierarchical_change_level += region->hierarchical_change_level;
		region = region->parent;
	}
	return sum_hierarchical_change_level;
}

/**
 * Adds delta_change_level to change_level of region and all its descendents.
 * Begins or ends change cache as many times as magnitude of delta_change_level.
 * +ve = beginChange, -ve = endChange.
 */
void cmzn_region::treeChange(int delta_change_level)
{
	for (int i = 0; i < delta_change_level; i++)
		this->beginChange();
	cmzn_region *child = this->first_child;
	while (child)
	{
		child->treeChange(delta_change_level);
		child = child->next_sibling;
	}
	for (int i = 0; i > delta_change_level; i--)
		this->endChange();
}

/**
 * Adds a callback to region so that when it changes <function> is called with
 * <user_data>. <function> has 3 arguments, a struct cmzn_region *, a
 * cmzn_region_changes * and the void *user_data.
 */
int cmzn_region::addCallback(CMZN_CALLBACK_FUNCTION(cmzn_region_change) *function, void *user_data)
{
	if (CMZN_CALLBACK_LIST_ADD_CALLBACK(cmzn_region_change)(
		this->change_callback_list, function, user_data))
	{
		return 1;
	}
	display_message(ERROR_MESSAGE, "cmzn_region::addCallback.  Could not add callback");
	return 0;
}

/**
 * Removes the callback calling <function> with <user_data> from <region>.
 */
int cmzn_region::removeCallback(CMZN_CALLBACK_FUNCTION(cmzn_region_change) *function, void *user_data)
{
	if (CMZN_CALLBACK_LIST_REMOVE_CALLBACK(cmzn_region_change)(
		this->change_callback_list, function, user_data))
	{
		return 1;
	}
	display_message(ERROR_MESSAGE, "cmzn_region::removeCallback.  Could not remove callback");
	return 0;
}

cmzn_region *cmzn_region::findChildByName(const char *name) const
{
	for (cmzn_region *child = this->first_child; (child); child = child->next_sibling)
	{
		if (0 == strcmp(child->name, name))
			return child;
	}
	return nullptr;
}

cmzn_region *cmzn_region::createChild(const char *name)
{
	if (!((this->context) && (name)))
		return nullptr;
	cmzn_region *childRegion = nullptr;
	// context stores all extant regions, so must ask it to create
	cmzn_region *region = this->getContext()->createRegion();  // accessed
	if ((CMZN_OK == region->setName(name)) &&
		(CMZN_OK == this->appendChild(region)))
	{
		childRegion = region;
	}
	cmzn_region::deaccess(region);  // cleans up region if failed, 
	return childRegion;
}

cmzn_region *cmzn_region::findSubregionAtPath(const char *path) const
{
	const cmzn_region *subregion = nullptr;
	if (!path)
		return nullptr;
	subregion = this;
	char *pathCopy = duplicate_string(path);
	char *childName = pathCopy;
	if (childName[0] == CMZN_REGION_PATH_SEPARATOR_CHAR)
		++childName;
	while ((subregion) && (childName[0] != '\0'))
	{
		char *childNameEnd = strchr(childName, CMZN_REGION_PATH_SEPARATOR_CHAR);
		if (childNameEnd)
			*childNameEnd = '\0';
		if (0 == strcmp(childName, CMZN_REGION_PATH_PARENT_NAME))
			subregion = subregion->parent;
		else
			subregion = subregion->findChildByName(childName);
		if (childNameEnd)
			childName = childNameEnd + 1;
		else
			break;
	}
	DEALLOCATE(pathCopy);
	return const_cast<cmzn_region *>(subregion);
}

cmzn_region *cmzn_region::createSubregion(const char *path)
{
	if (!path)
		return nullptr;
	cmzn_region *subregion = this;
	char *pathCopy = duplicate_string(path);
	char *childName = pathCopy;
	if (childName[0] == CMZN_REGION_PATH_SEPARATOR_CHAR)
		++childName;
	bool created = false;
	while ((subregion) && (childName[0] != '\0'))
	{
		char *childNameEnd = strchr(childName, CMZN_REGION_PATH_SEPARATOR_CHAR);
		if (childNameEnd)
			*childNameEnd = '\0';
		if (0 == strcmp(childName, CMZN_REGION_PATH_PARENT_NAME))
		{
			subregion = subregion->parent;
			if (!subregion)
				break;  // failed as no parent
		}
		else
		{
			cmzn_region *childRegion = subregion->findChildByName(childName);
			if (childRegion)
			{
				subregion = childRegion;
			}
			else
			{
				subregion = subregion->createChild(childName);
				created = true;
			}
		}
		if (childNameEnd)
			childName = childNameEnd + 1;
		else
			break;
	}
	DEALLOCATE(pathCopy);
	if (!created)
		return nullptr;  // failed to create or subregion already exists
	return subregion;
}

/**
 * Create an iterator for the region's fields.
 * @return  Accessed iterator.
 */
cmzn_fielditerator *cmzn_region::createFielditerator() const
{
	return Computed_field_manager_create_iterator(this->field_manager);
}

/** Called only by Field constructor code.
 * Ensures the new field has a unique cache_index.
 */
int cmzn_region::addField(cmzn_field *field)
{
	if (!field)
		return 0;
	int cache_index = this->field_cache_size;
	int number_in_manager = NUMBER_IN_MANAGER(Computed_field)(this->field_manager);
	if (cache_index == number_in_manager)
	{
		++this->field_cache_size;
	}
	else
	{
		std::vector<int> index_used(this->field_cache_size, 0);
		const cmzn_set_cmzn_field& fields = Computed_field_manager_get_fields(this->field_manager);
		for (cmzn_set_cmzn_field::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
		{
			index_used[cmzn_field_get_cache_index_private((*iter))] = 1;
		}
		for (int i = 0; i < this->field_cache_size; i++)
		{
			if (!index_used[i])
			{
				cache_index = i;
				break;
			}
		}
	}
	if (Computed_field_add_to_manager_private(field, this->field_manager))
	{
		int i = 1;
		for (std::list<cmzn_fieldcache_id>::iterator iter = this->field_caches.begin();
			iter != this->field_caches.end(); ++iter)
		{
			cmzn_fieldcache_id field_cache = *iter;
			field_cache->setValueCache(cache_index, 0);
			++i;
		}
		cmzn_field_set_cache_index_private(field, cache_index);
		return 1;
	}
	return 0;
}

/** Called only by FieldDerivative.
 * Add the field derivative to the list in the region and assign its unique
 * cache index.
 * NOTE: Throws an exception on any failure.
 * @return  Accessed field derivative or nullptr if failed.
 */
void cmzn_region::addFieldDerivative(FieldDerivative *fieldDerivative)
{
	const int size = static_cast<int>(this->fieldDerivatives.size());
	for (int i = 0; i < size; ++i)
		if (!this->fieldDerivatives[i])
		{
			this->fieldDerivatives[i] = fieldDerivative;
			fieldDerivative->setRegionAndCacheIndexPrivate(this, i);
			return;
		}
	this->fieldDerivatives.push_back(fieldDerivative);
	fieldDerivative->setRegionAndCacheIndexPrivate(this, size);
}

/** Called only by ~FieldDerivative */
void cmzn_region::removeFieldDerivative(FieldDerivative *fieldDerivative)
{
	if (!fieldDerivative)
	{
		display_message(ERROR_MESSAGE, "cmzn_region::removeFieldDerivative.  Invalid field derivative");
		return;
	}
	const int derivativeCacheIndex = fieldDerivative->getCacheIndex();
	const int size = static_cast<int>(this->fieldDerivatives.size());
	if ((fieldDerivative->getRegion() != this) || (derivativeCacheIndex < 0) || (derivativeCacheIndex >= size))
	{
		display_message(ERROR_MESSAGE, "cmzn_region::removeFieldDerivative.  Invalid field derivative");
		return;
	}
	fieldDerivative->setRegionAndCacheIndexPrivate();
	// remove derivative caches from field caches so index can be recycled
	for (std::list<cmzn_fieldcache_id>::iterator iter = this->field_caches.begin();
		iter != this->field_caches.end(); ++iter)
	{
		(*iter)->removeDerivativeCaches(derivativeCacheIndex);
	}
	this->fieldDerivatives[derivativeCacheIndex] = nullptr;
}

void cmzn_region::clearFieldValueCaches(cmzn_field *field)
{
	const int cacheIndex = cmzn_field_get_cache_index_private(field);
	for (std::list<cmzn_fieldcache_id>::iterator iter = this->field_caches.begin();
		iter != this->field_caches.end(); ++iter)
	{
		cmzn_fieldcache_id cache = *iter;
		FieldValueCache *valueCache = cache->getValueCache(cacheIndex);
		if (valueCache)
		{
			valueCache->clear();
		}
	}
}

int cmzn_region::setName(const char *name)
{
	if (!name)
		return CMZN_ERROR_ARGUMENT;
	if ((0 == this->name) || (0 != strcmp(this->name, name)))
	{
		if ((0 == this->parent) ||
			(0 == this->parent->findChildByName(name)))
		{
			char *temp_name = duplicate_string(name);
			if (this->name)
				DEALLOCATE(this->name);
			this->name = temp_name;
			this->changes.name_changed = 1;
			this->updateClients();
		}
		else
			return CMZN_ERROR_ARGUMENT; // name is in use by sibling
	}
	return CMZN_OK;
}

char *cmzn_region::getPath() const
{
	char *path = nullptr;
	if (this->parent)
	{
		int error = 0;
		const cmzn_region *region = this;
		while (region->parent)
		{
			if (path)
			{
				append_string(&path, CMZN_REGION_PATH_SEPARATOR_STRING, &error, /*prefix*/true);
			}
			append_string(&path, region->name, &error, /*prefix*/true);
			region = region->parent;
		}
	}
	else
	{
		path = duplicate_string("");
	}
	return path;
}

char *cmzn_region::getRelativePath(const cmzn_region *baseRegion) const
{
	if (!baseRegion)
		return nullptr;
	char *pathUp = nullptr;
	int error = 0;
	const cmzn_region *commonBaseRegion = baseRegion;
	while (!commonBaseRegion->containsSubregion(this))
	{
		if (!commonBaseRegion->parent)
		{
			DEALLOCATE(pathUp);
			return nullptr;
		}
		if (pathUp)
		{
			append_string(&pathUp, CMZN_REGION_PATH_SEPARATOR_STRING, &error, /*prefix*/true);
		}
		append_string(&pathUp, CMZN_REGION_PATH_PARENT_NAME, &error, /*prefix*/true);
		commonBaseRegion = commonBaseRegion->parent;
	}
	char *path = nullptr;
	const cmzn_region *region = this;
	while (region != commonBaseRegion)
	{
		if (path)
		{
			append_string(&path, CMZN_REGION_PATH_SEPARATOR_STRING, &error, /*prefix*/true);
		}
		append_string(&path, region->name, &error, /*prefix*/true);
		region = region->parent;
	}
	if (path)
	{
		if (pathUp)
		{
			append_string(&path, CMZN_REGION_PATH_SEPARATOR_STRING, &error, /*prefix*/true);
			append_string(&path, pathUp, &error, /*prefix*/true);
			DEALLOCATE(pathUp);
		}
	}
	else if (pathUp)
	{
		path = pathUp;
	}
	else
	{
		path = duplicate_string("");
	}
	return path;
}

int cmzn_region::insertChildBefore(cmzn_region *new_child, cmzn_region *ref_child)
{
	if (!(new_child &&
		((nullptr == ref_child) || (ref_child->parent == this)) &&
		(!cmzn_region_contains_subregion(new_child, this)) &&
		((nullptr != new_child->name) && ((new_child->parent == this) ||
		(nullptr == this->findChildByName(new_child->name))))))
	{
		return CMZN_ERROR_ARGUMENT;
	}
	if (new_child->context != this->context)
	{
		display_message(ERROR_MESSAGE, "Zinc Region appendChild()/insertChildBefore():  Child region is from a different context");
		return CMZN_ERROR_ARGUMENT_CONTEXT;
	}
	int delta_change_level = this->getSumHierarchicalChangeLevel();
	this->beginChange();
	if (new_child->parent)
	{
		delta_change_level -= new_child->parent->getSumHierarchicalChangeLevel();
		cmzn_region_remove_child(new_child->parent, new_child);
	}
	new_child->parent = this;
	if (ref_child)
	{
		new_child->next_sibling = ref_child;
		new_child->previous_sibling = ref_child->previous_sibling;
		ref_child->previous_sibling = new_child;
		if (new_child->previous_sibling)
		{
			new_child->previous_sibling->next_sibling = new_child->access();
		}
		else
		{
			this->first_child = new_child->access();
		}
	}
	else
	{
		if (this->first_child)
		{
			cmzn_region *last_child = this->first_child;
			while (last_child->next_sibling)
			{
				last_child = last_child->next_sibling;
			}
			last_child->next_sibling = new_child->access();
			new_child->previous_sibling = last_child;
		}
		else
		{
			this->first_child = new_child->access();
		}
	}
	if (!this->changes.children_changed)
	{
		this->changes.children_changed = 1;
		this->changes.child_added = new_child->access();
	}
	else
	{
		/* multiple changes */
		cmzn_region::deaccess(this->changes.child_added);
		cmzn_region::deaccess(this->changes.child_removed);
	}
	if (delta_change_level != 0)
	{
		new_child->treeChange(delta_change_level);
	}
	this->endChange();
	return CMZN_OK;
}

int cmzn_region::removeChild(cmzn_region *old_child)
{
	if ((!old_child) || (old_child->parent != this))
		return CMZN_ERROR_ARGUMENT;
	this->beginChange();
	Computed_field_manager_subregion_removed(this->field_manager, old_child);
	int delta_change_level = this->getSumHierarchicalChangeLevel();
	if (old_child == this->first_child)
	{
		this->first_child = old_child->next_sibling;
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
	if (!this->changes.children_changed)
	{
		this->changes.children_changed = 1;
		this->changes.child_removed = old_child->access();
	}
	else
	{
		/* multiple changes */
		cmzn_region::deaccess(this->changes.child_added);
		cmzn_region::deaccess(this->changes.child_removed);
	}
	if (delta_change_level != 0)
	{
		old_child->treeChange(delta_change_level);
	}
	this->updateClients();
	cmzn_region::deaccess(old_child);
	this->endChange();
	return CMZN_OK;
}

bool cmzn_region::containsSubregion(const cmzn_region *subregion) const
{
	while (subregion)
	{
		if (subregion == this)
			return true;
		subregion = subregion->parent;
	}
	return false;
}

void cmzn_region::addFieldmodulenotifier(cmzn_fieldmodulenotifier *notifier)
{
	if (notifier)
		this->notifier_list.push_back(notifier->access());
}

void cmzn_region::removeFieldmodulenotifier(cmzn_fieldmodulenotifier *notifier)
{
	if (notifier)
	{
		cmzn_fieldmodulenotifier_list::iterator iter = std::find(
			this->notifier_list.begin(), this->notifier_list.end(), notifier);
		if (iter != this->notifier_list.end())
		{
			cmzn_fieldmodulenotifier::deaccess(notifier);
			this->notifier_list.erase(iter);
		}
	}
}

int cmzn_region::getTimeRange(FE_value& minimumTime, FE_value& maximumTime, bool hierarchical) const
{
	int result = this->fe_region->getTimeRange(minimumTime, maximumTime);
	if (hierarchical)
	{
		for (cmzn_region *child = this->first_child; (child); child = child->next_sibling)
		{
			FE_value childMinimumTime, childMaximumTime;
			if (CMZN_OK == child->getTimeRange(childMinimumTime, childMaximumTime, true))
			{
				if (CMZN_OK == result)
				{
					if (childMinimumTime < minimumTime)
					{
						minimumTime = childMinimumTime;
					}
					if (childMaximumTime > maximumTime)
					{
						maximumTime = childMaximumTime;
					}
				}
				else
				{
					minimumTime = childMinimumTime;
					maximumTime = childMaximumTime;
					result = CMZN_OK;
				}
			}
		}
	}
	return result;
}

/*
Global functions
----------------
*/

PROTOTYPE_ACCESS_OBJECT_FUNCTION(cmzn_region)
{
	if (object)
		return object->access();
	return nullptr;
}

PROTOTYPE_DEACCESS_OBJECT_FUNCTION(cmzn_region)
{
	if (object_address)
		return cmzn_region::deaccess(*object_address);
	return CMZN_ERROR_ARGUMENT;
}

PROTOTYPE_REACCESS_OBJECT_FUNCTION(cmzn_region)
{
	if (!object_address)
		return CMZN_ERROR_ARGUMENT;
	if (new_object)
		new_object->access();
	cmzn_region::deaccess(*object_address);
	*object_address = new_object;
	return CMZN_OK;
}

cmzn_region_id cmzn_region_create_region(cmzn_region_id base_region)
{
	if ((base_region) && (base_region->getContext()))
		return base_region->getContext()->createRegion();
	return nullptr;
}

struct cmzn_region *cmzn_region_create_child(struct cmzn_region *region,
	const char *name)
{
	if (!region)
		return nullptr;
	cmzn_region *childRegion = region->createChild(name);
	if (childRegion)
		childRegion->access();
	return childRegion;
}

struct cmzn_region *cmzn_region_create_subregion(
	struct cmzn_region *region, const char *path)
{
	if (!region)
		return nullptr;
	cmzn_region *subregion = region->createSubregion(path);
	if (subregion)
		subregion->access();
	return subregion;
}

int cmzn_region_get_time_range(cmzn_region_id region,
	double *minimumValueOut, double *maximumValueOut)
{
	if ((region) && (minimumValueOut) && (minimumValueOut))
	{
		return region->getTimeRange(*minimumValueOut, *maximumValueOut, /*hierarchical*/false);
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_region_get_hierarchical_time_range(
	cmzn_region_id region, double *minimumValueOut, double *maximumValueOut)
{
	if ((region) && (minimumValueOut) && (minimumValueOut))
	{
		return region->getTimeRange(*minimumValueOut, *maximumValueOut, /*hierarchical*/true);
	}
	return CMZN_ERROR_ARGUMENT;
}


int cmzn_region_clear_finite_elements(struct cmzn_region *region)
{
	return FE_region_clear(region->get_FE_region());
}

struct FE_region *cmzn_region_get_FE_region(struct cmzn_region *region)
{
	if (region)
		return region->get_FE_region();
	return 0;
}

struct MANAGER(Computed_field) *cmzn_region_get_Computed_field_manager(
	struct cmzn_region *region)
{
	if (region)
		return region->getFieldManager();
	return 0;
}

int cmzn_fieldmodule_begin_change(cmzn_fieldmodule_id fieldmodule)
{
	if (!fieldmodule)
		return CMZN_ERROR_ARGUMENT;
	cmzn_fieldmodule_get_region_internal(fieldmodule)->beginChangeFields();
	return CMZN_OK;
}

int cmzn_fieldmodule_end_change(cmzn_fieldmodule_id fieldmodule)
{
	if (!fieldmodule)
		return CMZN_ERROR_ARGUMENT;
	cmzn_fieldmodule_get_region_internal(fieldmodule)->endChangeFields();
	return CMZN_OK;
}

int cmzn_fieldmodule_define_all_faces(cmzn_fieldmodule_id fieldmodule)
{
	if (!fieldmodule)
		return CMZN_ERROR_ARGUMENT;
	return FE_region_define_faces(
		cmzn_fieldmodule_get_region_internal(fieldmodule)->get_FE_region());
}

int cmzn_region_begin_change(struct cmzn_region *region)
{
	if (region)
	{
		region->beginChange();
		return CMZN_OK;
	}
	display_message(ERROR_MESSAGE, "cmzn_region_begin_change.  Invalid argument(s)");
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_region_end_change(struct cmzn_region *region)
{
	if (region)
		return region->endChange();
	display_message(ERROR_MESSAGE, "cmzn_region_end_change.  Invalid argument(s)");
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_region_begin_hierarchical_change(struct cmzn_region *region)
{
	if (region)
	{
		region->beginHierarchicalChange();
		return 1;
	}
	return 0;
}

int cmzn_region_end_hierarchical_change(struct cmzn_region *region)
{
	if (region)
	{
		region->endHierarchicalChange();
		return 1;
	}
	return 0;
}

int cmzn_region_add_callback(struct cmzn_region *region,
	CMZN_CALLBACK_FUNCTION(cmzn_region_change) *function, void *user_data)
{
	if (region)
		return region->addCallback(function, user_data);
	display_message(ERROR_MESSAGE, "cmzn_region_add_callback.  Invalid argument(s)");
	return 0;
}

int cmzn_region_remove_callback(struct cmzn_region *region,
	CMZN_CALLBACK_FUNCTION(cmzn_region_change) *function, void *user_data)
{
	if (region)
		return region->removeCallback(function, user_data);
	display_message(ERROR_MESSAGE, "cmzn_region_remove_callback.  Invalid argument(s)");
	return 0;
}

cmzn_context_id cmzn_region_get_context(cmzn_region_id region)
{
	if (region)
	{
		cmzn_context* context = region->getContext();
		if (context)
			return context->access();
	}
	return nullptr;
}

char *cmzn_region_get_name(struct cmzn_region *region)
{
	if (region)
	{
		const char *regionName = region->getName();
		if (regionName)
			return duplicate_string(regionName);
	}
	return nullptr;
}

int cmzn_region_set_name(struct cmzn_region *region, const char *name)
{
	if (region)
		return region->setName(name);
	return CMZN_ERROR_ARGUMENT;
}

char *cmzn_region_get_root_region_path(void)
{
	return duplicate_string(CMZN_REGION_PATH_SEPARATOR_STRING);
}

char *cmzn_region_get_path(struct cmzn_region *region)
{
	if (region)
		return region->getPath();
	return nullptr;
}

char *cmzn_region_get_relative_path(struct cmzn_region *region,
	struct cmzn_region *base_region)
{
	if (region)
		return region->getRelativePath(base_region);
	return nullptr;
}

struct cmzn_region *cmzn_region_get_parent(struct cmzn_region *region)
{
	if (!region)
		return nullptr;
	cmzn_region *parent = region->getParent();
	if (parent)
		parent->access();
	return parent;
}

cmzn_region_id cmzn_region_get_root(cmzn_region_id region)
{
	if (!region)
		return nullptr;
	return region->getRoot()->access();
}

struct cmzn_region *cmzn_region_get_first_child(struct cmzn_region *region)
{
	if (region)
	{
		cmzn_region *childRegion = region->getFirstChild();
		if (childRegion)
			return childRegion->access();
	}
	return nullptr;
}

struct cmzn_region *cmzn_region_get_next_sibling(struct cmzn_region *region)
{
	if (region)
	{
		cmzn_region *siblingRegion = region->getNextSibling();
		if (siblingRegion)
			return siblingRegion->access();
	}
	return nullptr;
}

struct cmzn_region *cmzn_region_get_previous_sibling(struct cmzn_region *region)
{
	if (region)
	{
		cmzn_region *siblingRegion = region->getPreviousSibling();
		if (siblingRegion)
			return siblingRegion->access();
	}
	return nullptr;
}

void cmzn_region_reaccess_next_sibling(struct cmzn_region **region_address)
{
	if (region_address && (*region_address))
	{
		cmzn_region *siblingRegion = (*region_address)->getNextSibling();
		if (siblingRegion)
			siblingRegion->access();
		cmzn_region::deaccess(*region_address);
		*region_address = siblingRegion;
	}
}

int cmzn_region_append_child(struct cmzn_region *region,
	struct cmzn_region *new_child)
{
	if (region)
		return region->appendChild(new_child);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_region_insert_child_before(struct cmzn_region *region,
	struct cmzn_region *new_child, struct cmzn_region *ref_child)
{
	if (region)
		return region->insertChildBefore(new_child, ref_child);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_region_remove_child(struct cmzn_region *region,
	struct cmzn_region *old_child)
{
	if (region)
		return region->removeChild(old_child);
	return CMZN_ERROR_ARGUMENT;
}

bool cmzn_region_contains_subregion(struct cmzn_region *region,
	struct cmzn_region *subregion)
{
	if (region)
		return region->containsSubregion(subregion);
	return false;
}

struct cmzn_region *cmzn_region_find_child_by_name(
	struct cmzn_region *region, const char *name)
{
	if ((region) && (name))
	{
		cmzn_region *child = region->findChildByName(name);
		if (child)
			return child->access();
	}
	return nullptr;
}

struct cmzn_region *cmzn_region_find_subregion_at_path(
	struct cmzn_region *region, const char *path)
{
	if (!region)
		return nullptr;
	cmzn_region *subregion = region->findSubregionAtPath(path);
	if (subregion)
		subregion->access();
	return subregion;
}

cmzn_field_id cmzn_region_find_field_by_name(cmzn_region_id region,
	const char *field_name)
{
	if (!region)
		return nullptr;
	struct cmzn_field *field = region->findFieldByName(field_name);
	if (field)
		ACCESS(Computed_field)(field);
	return (field);
}

int cmzn_region_get_region_from_path_deprecated(struct cmzn_region *region,
	const char *path, struct cmzn_region **subregion_address)
{
	int return_code = 0;
	if ((region) && (path) && (subregion_address))
	{
		*subregion_address = region->findSubregionAtPath(path);
		if (*subregion_address)
			return_code = 1;
	}
	return (return_code);
}

bool cmzn_region_is_root(struct cmzn_region *region)
{
	return (region) && (!region->getParent());
}

int cmzn_region_get_partial_region_path(cmzn_region *baseRegion,
	const char *path, cmzn_region **regionAddress,
	char **regionPathAddress, char **remainderAddress)
{
	if (!(baseRegion && path && regionAddress && regionPathAddress &&
		remainderAddress))
	{
		display_message(ERROR_MESSAGE, "cmzn_region_get_partial_region_path.  Invalid argument(s)");
		return 0;
	}
	int return_code = 1;
	cmzn_region *region = baseRegion;
	char *pathCopy = duplicate_string(path);
	char *childName = pathCopy;
	if (childName[0] == CMZN_REGION_PATH_SEPARATOR_CHAR)
		++childName;
	const char *childNameStart = childName;
	cmzn_region *nextRegion = region;
	while (nextRegion && (childName[0] != '\0'))
	{
		char *childNameEnd = strchr(childName, CMZN_REGION_PATH_SEPARATOR_CHAR);
		if (childNameEnd)
			*childNameEnd = '\0';
		if (0 == strcmp(childName, CMZN_REGION_PATH_PARENT_NAME))
			nextRegion = nextRegion->getParent();
		else
			nextRegion = nextRegion->findChildByName(childName);
		if (nextRegion)
		{
			region = nextRegion;
			if (childNameEnd)
				childName = childNameEnd + 1;
			else
				childName = childName + strlen(childName);
		}
		if (childNameEnd)
			*childNameEnd = CMZN_REGION_PATH_SEPARATOR_CHAR;
	}

	int length = childName - childNameStart;
	if ((length > 0) &&
		(*(childName - 1) == CMZN_REGION_PATH_SEPARATOR_CHAR))
	{
		--length;
	}
	if (ALLOCATE(*regionPathAddress, char, length + 1))
	{
		strncpy(*regionPathAddress, childNameStart, length);
		(*regionPathAddress)[length] = '\0';
	}
	else
	{
		return_code = 0;
	}

	length = static_cast<int>(strlen(childName));
	if (length == 0)
	{
		*remainderAddress = nullptr;
	}
	else
	{
		if (ALLOCATE(*remainderAddress, char, length + 1))
		{
			strncpy(*remainderAddress, childName, length);
			(*remainderAddress)[length] = '\0';
		}
		else
		{
			return_code = 0;
		}
	}
	if (return_code)
	{
		*regionAddress = region;
	}
	else
	{
		if (*regionAddress)
			DEALLOCATE(*regionAddress);
		if (*remainderAddress)
			DEALLOCATE(remainderAddress);
		*regionAddress = nullptr;
	}
	DEALLOCATE(pathCopy);
	return return_code;
}

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
		cmzn_region *child = region->getFirstChild();
		while (child)
		{
			display_message(INFORMATION_MESSAGE, "%*s%s : \n", indent, " ", child->getName());
			cmzn_region_list(child, indent + indent_increment, indent_increment);
			child = child->getNextSibling();
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
		return region->access();
	return 0;
}

int cmzn_region_destroy(cmzn_region_id *region)
{
	if (region)
		return cmzn_region::deaccess(*region);
	return 0;
}

bool cmzn_region_can_merge(cmzn_region_id target_region, cmzn_region_id source_region)
{
	if (!source_region)
		return false;

	if (target_region)
	{
		// check fields
		bool fieldsOK = true;
		cmzn_fielditerator *iter = source_region->createFielditerator();
		cmzn_field *field = nullptr;
		while ((field = iter->next_non_access()))
		{
			cmzn_field *existingField = target_region->findFieldByName(field->getName());
			if (existingField)
			{
				if (existingField->hasAutomaticName())
				{
					// will be renamed to make way for new field if not compatible
				}
				else if (existingField->compareFullDefinition(*field))
				{
					// found the matching field
				}
				else if (existingField->compareBasicDefinition(*field) &&
					cmzn_field_is_dummy_real(existingField))
				{
					// dummy_real will have incoming field merged into it
				}
				else
				{
					fieldsOK = false;
					char *target_path = target_region->getPath();
					display_message(ERROR_MESSAGE, "Cannot merge incompatible field %s into region %s%s", field->getName(), CMZN_REGION_PATH_SEPARATOR_STRING, target_path);
					DEALLOCATE(target_path);
				}
			}
		}
		cmzn_fielditerator::deaccess(iter);
		if (!fieldsOK)
		{
			return false;
		}
	}

	// check FE_regions
	if (!FE_region_can_merge((target_region) ? target_region->get_FE_region() : nullptr, source_region->get_FE_region()))
	{
		char *source_path = source_region->getPath();
		char *target_path = target_region->getPath();
		display_message(ERROR_MESSAGE,
			"Cannot merge source region %s into %s", source_path, target_path);
		DEALLOCATE(source_path);
		DEALLOCATE(target_path);
		return false;
	}

	// check child regions can be merged
	cmzn_region_id source_child = cmzn_region_get_first_child(source_region);
	while (NULL != source_child)
	{
		cmzn_region_id target_child = 0;
		if (target_region)
			target_child = target_region->findChildByName(source_child->getName());
		if (!cmzn_region_can_merge(target_child, source_child))
		{
			cmzn_region_destroy(&source_child);
			return 0;
		}
		cmzn_region_reaccess_next_sibling(&source_child);
	}

	return true;
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
				char *source_path = source_region->getPath();
				char *target_path = target_region->getPath();
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
	cmzn_region_id source_region)
{
	int return_code = 1;
	source_region->beginChange();

	if (!FE_region_merge(target_region->get_FE_region(), source_region->get_FE_region()))
	{
		char *source_path = source_region->getPath();
		char *target_path = target_region->getPath();
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
		cmzn_region_id target_child = cmzn_region_find_child_by_name(target_region, source_child->getName());
		if (target_child)
		{
			return_code = cmzn_region_merge_private(target_child, source_child);
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

	source_region->clearCachedChanges();  // so no notifications sent with endChange below
	source_region->endChange();
	return (return_code);
}

int cmzn_region_merge(cmzn_region_id target_region, cmzn_region_id source_region)
{
	if (!target_region || !source_region)
		return 0;
	cmzn_region_begin_hierarchical_change(target_region);
	int return_code = cmzn_region_merge_private(target_region, source_region);
	cmzn_region_end_hierarchical_change(target_region);
	return return_code;
}

/**
 * Ensures there is an up-to-date computed field wrapper for <fe_field>.
 * Assume field manager change cache is active while this is called.
 * @param fe_field  Field to wrap.
 * @param fieldmodule_void  Field module to contain wrapped FE_field.
 */
static int FE_field_to_Computed_field_change(struct FE_field *fe_field,
	int change, void *fieldmodule_void)
{
	cmzn_fieldmodule *fieldmodule = reinterpret_cast<cmzn_fieldmodule*>(fieldmodule_void);
	if (change & (CHANGE_LOG_OBJECT_ADDED(FE_field) |
		CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_field) |  // don't expect identifier change any more
		CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_field)))
	{
		cmzn_region *region = cmzn_fieldmodule_get_region_internal(fieldmodule);
		const char *fieldName = fe_field->getName();
		cmzn_field *existingField = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)(
			fieldName, region->getFieldManager());
		bool updateWrapper = false;
		if (existingField)
		{
			if (Computed_field_wraps_fe_field(existingField, (void *)fe_field))
			{
				// only need to ensure coordinate system is correct and propagate change
				existingField->setCoordinateSystem(fe_field->getCoordinateSystem());
				if (change & CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_field))
				{
					existingField->setChanged();
				}
				else if (change & CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_field))
				{
					existingField->setChangedRelated();
				}
			}
			else if (existingField->hasAutomaticName())
			{
				// rename the existing automatically named field, so new field can use its name
				existingField->setNameAutomatic();
				existingField = nullptr;
				updateWrapper = true;
			}
			else if (cmzn_field_is_dummy_real(existingField) &&
				(existingField->getNumberOfComponents() == fe_field->getNumberOfComponents()) &&
				(fe_field->getValueType() == FE_VALUE_VALUE))
			{
				// can replace dummy_real with real-valued finite element, replaces apply field's temporary evaluate field
				updateWrapper = true;
			}
			else
			{
				display_message(ERROR_MESSAGE, "Cannot create finite element field %s as another field is already using that name", fieldName);
			}
		}
		else if (change & CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_field))
		{
			// don't expect FE_field identifier change except through field wrapper
			display_message(ERROR_MESSAGE, "FE_field_to_Computed_field_change  Unexpected renaming of finite element field %s", fieldName);
		}
		else
		{
			// add new wrapper
			updateWrapper = true;
		}
		if (updateWrapper)
		{
			cmzn_field *field = cmzn_fieldmodule_create_field_finite_element_wrapper(fieldmodule, fe_field, (existingField) ? nullptr : fieldName);
			if (field)
			{
				if (existingField)
				{
					if (CMZN_OK != existingField->copyDefinition(*field))
					{
						display_message(WARNING_MESSAGE, "Failed to copy definition of finite element field %s over existing field using that name.",
							fieldName, field->getName());
					}
				}
				else
				{
					cmzn_field_set_managed(field, true);
				}
				cmzn_field::deaccess(field);
			}
		}
	}
	return 1;
}

void cmzn_region::FeRegionChange()
{
	struct CHANGE_LOG(FE_field) *fe_field_changes = FE_region_get_FE_field_changes(this->fe_region);
	int field_change_summary;
	CHANGE_LOG_GET_CHANGE_SUMMARY(FE_field)(fe_field_changes, &field_change_summary);

	cmzn_fieldmodule_id fieldmodule = cmzn_region_get_fieldmodule(this);
	MANAGER_BEGIN_CACHE(Computed_field)(this->field_manager);

	// update field wrappers
	if (0 != (field_change_summary & (~(CHANGE_LOG_OBJECT_REMOVED(FE_field) | CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_field)))))
	{
		CHANGE_LOG_FOR_EACH_OBJECT(FE_field)(fe_field_changes,
			FE_field_to_Computed_field_change, (void *)fieldmodule);
	}
	if (FE_region_need_add_cmiss_number_field(this->fe_region))
	{
		const char *cmiss_number_field_name = "cmiss_number";
		cmzn_field_id field = cmzn_fieldmodule_find_field_by_name(fieldmodule, cmiss_number_field_name);
		if (!field)
		{
			field = cmzn_fieldmodule_create_field_cmiss_number(fieldmodule);
			cmzn_field_set_name(field, cmiss_number_field_name);
			cmzn_field_set_managed(field, true);
		}
		cmzn_field_destroy(&field);
	}
	if (FE_region_need_add_xi_field(this->fe_region))
	{
		cmzn_field_id xi_field = cmzn_fieldmodule_get_or_create_xi_field(fieldmodule);
		cmzn_field_destroy(&xi_field);
	}
	// force field update for changes to nodes/elements etc.:
	MANAGER_EXTERNAL_CHANGE(Computed_field)(this->field_manager);
	MANAGER_END_CACHE(Computed_field)(this->field_manager);
	cmzn_fieldmodule_destroy(&fieldmodule);
#if 0
	if (field_change_summary & CHANGE_LOG_OBJECT_REMOVED(FE_field))
	{
		/* Currently we do nothing as the computed field wrapper is destroyed
			before the FE_field is removed from the manager.  This is not necessary
			and this response could be to delete the wrapper. */
	}
#endif
}
