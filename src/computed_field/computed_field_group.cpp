/**
 * FILE : computed_field_group.cpp
 * 
 * Implements a field for representing hierarchical groups, essentially a
 * predicate function returning true for parts of domains considered in the group,
 * and coordinating related subobject groups.
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "cmlibs/zinc/fieldgroup.h"
#include "cmlibs/zinc/mesh.h"
#include "cmlibs/zinc/nodeset.h"
#include "cmlibs/zinc/scene.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_group.hpp"
#include "computed_field/computed_field_group_base.hpp"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/field_module.hpp"
#include "finite_element/finite_element_nodeset.hpp"
#include "finite_element/finite_element_region.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "region/cmiss_region.hpp"
#include "general/message.h"
#include "mesh/cmiss_node_private.hpp"
#include "mesh/cmiss_element_private.hpp"
#include "mesh/nodeset.hpp"
#include "region/cmiss_region.hpp"
#include <cstdlib>
#include <list>
#include <string>

char computed_field_group_type_string[] = "group";

Computed_field_group::Computed_field_group(cmzn_region *region)
	: Computed_field_group_base()
	, containsAllLocal(false)
	, subelementHandlingMode(CMZN_FIELD_GROUP_SUBELEMENT_HANDLING_MODE_NONE)
	, nodesetGroups{}
	, meshGroups{}
	, child_region_group_map()
{
}

Computed_field_group::~Computed_field_group()
{
	for (int i = 0; i < 2; ++i)
	{
		clearRemoveNodesetGroup(i, /*clear*/false, /*remove*/true);
	}
	for (int i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
	{
		clearRemoveMeshGroup(i, /*clear*/false, /*remove*/true);
	}
	for (Region_field_map_iterator iter = child_region_group_map.begin();
		iter != child_region_group_map.end(); ++iter)
	{
		cmzn_field_group_id subregion_group_field = iter->second;
		cmzn_field_group_destroy(&subregion_group_field);
	}
}

void Computed_field_group::clearRemoveNodesetGroup(int index, bool clear, bool remove)
{
	if (this->nodesetGroups[index])
	{
		if (clear)
		{
			this->nodesetGroups[index]->removeAllNodes();
		}
		if (remove && (this->nodesetGroups[index]->getAccessCount() == 1))
		{
			cmzn_nodeset_group::deaccess(this->nodesetGroups[index]);
		}
	}
}

void Computed_field_group::clearRemoveMeshGroup(int index, bool clear, bool remove)
{
	if (this->meshGroups[index])
	{
		if (clear)
		{
			this->meshGroups[index]->removeAllElements();
		}
		if (remove && (this->meshGroups[index]->getAccessCount() == 1))
		{
			cmzn_mesh_group::deaccess(this->meshGroups[index]);
		}
	}
}

/**
 * Compare the type specific data.
 */
int Computed_field_group::compare(Computed_field_core *other_core)
{
	if ((this->field) && dynamic_cast<Computed_field_group*>(other_core))
	{
		return 1;
	}
	return 0;
}

/**
 * Evaluates to 1 if domain location is in group, otherwise 0.
 */
int Computed_field_group::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	valueCache.values[0] = 0.0;
	if (containsAllLocal)
	{
		valueCache.values[0] = 1.0;
	}
	else
	{
		const Field_location_node* node_location;
		const Field_location_element_xi *element_xi_location;
		if ((node_location = cache.get_location_node()))
		{
			cmzn_node* node = node_location->get_node();
			const FE_nodeset* feNodeset = node->getNodeset();
			if (feNodeset)
			{
				const int index = FE_nodeset_to_index(feNodeset);
				if (this->nodesetGroups[index]->containsNode(node))
				{
					valueCache.values[0] = 1.0;
				}
			}
		}
		else if ((element_xi_location = cache.get_location_element_xi()))
		{
			cmzn_element* element = element_xi_location->get_element();
			const FE_mesh* feMesh = element->getMesh();
			if (feMesh)
			{
				const int index = feMesh->getDimension() - 1;
				if (this->meshGroups[index]->containsElement(element))
				{
					valueCache.values[0] = 1.0;
				}
			}
		}
	}
	return 1;
}

bool Computed_field_group::isEmptyLocal() const
{
	if (this->containsAllLocal)
	{
		return false;
	}
	for (int i = 0; i < 2; ++i)
	{
		if ((this->nodesetGroups[i]) && (this->nodesetGroups[i]->getSize() > 0))
		{
			return false;
		}
	}
	for (int i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
	{
		if ((this->meshGroups[i]) && (this->meshGroups[i]->getSize() > 0))
		{
			return false;
		}
	}
	return true;
}

bool Computed_field_group::isEmptyNonLocal() const
{
	for (Region_field_map_const_iterator iter = child_region_group_map.begin();
		iter != child_region_group_map.end(); iter++)
	{
		cmzn_field_group* subregionGroup = iter->second;
		if (!isSubGroupEmpty(cmzn_field_group_base_cast(subregionGroup)->core))
		{
			return false;
		}
	}
	return true;
}

int Computed_field_group::clearLocal()
{
	this->beginChange();
	// for efficiently, temporarily disable subelement handling
	cmzn_field_group_subelement_handling_mode oldSubelementHandlingMode = this->subelementHandlingMode;
	this->subelementHandlingMode = CMZN_FIELD_GROUP_SUBELEMENT_HANDLING_MODE_NONE;
	this->containsAllLocal = false;
	for (int i = 0; i < 2; i++)
	{
		this->clearRemoveNodesetGroup(i);
	}
	for (int i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
	{
		this->clearRemoveMeshGroup(i);
	}
	this->change_detail.changeRemoveLocal();
	this->subelementHandlingMode = oldSubelementHandlingMode;
	this->field->setChanged();
	this->endChange();
	return CMZN_OK;
};

int Computed_field_group::clear()
{
	int return_code = 0;
	this->beginChange();
	for (Region_field_map_iterator iter = child_region_group_map.begin();
		iter != child_region_group_map.end(); iter++)
	{
		Computed_field_group *group_core = cmzn_field_group_core_cast(iter->second);
		group_core->clear();
	}
	return_code = clearLocal();
	this->field->setChanged();
	this->endChange();
	return return_code;
};

cmzn_field_change_detail *Computed_field_group::extract_change_detail()
{
	if (this->change_detail.getChangeSummary() == CMZN_FIELD_GROUP_CHANGE_NONE)
	{
		return nullptr;
	}
	cmzn_field_hierarchical_group_change_detail *prior_change_detail =
		new cmzn_field_hierarchical_group_change_detail(change_detail);
#ifdef DEBUG_CODE
	{
		cmzn_region *region = Computed_field_get_region(field);
		char *path = region->getPath();
		display_message(INFORMATION_MESSAGE, "Group %s%s change local %d non-local %d\n", path, field->name,
			prior_change_detail->getLocalChange(), prior_change_detail->getNonLocalChange());
		DEALLOCATE(path);
	}
#endif // DEBUG_CODE
	change_detail.clear();
	return prior_change_detail;
}

void Computed_field_group::propagate_hierarchical_field_changes(
	MANAGER_MESSAGE(Computed_field) *message)
{
	if (message)
	{
		for (Region_field_map_iterator iter = child_region_group_map.begin();
			iter != child_region_group_map.end(); iter++)
		{
			cmzn_field_group* subregionGroup = iter->second;
			// future optimisation: check subfield is from changed region
			const cmzn_field_change_detail *source_change_detail = NULL;
			int change = Computed_field_manager_message_get_object_change_and_detail(
				message, cmzn_field_group_base_cast(subregionGroup), &source_change_detail);
			if (change != MANAGER_CHANGE_NONE(Computed_field))
			{
				if (source_change_detail)
				{
					const cmzn_field_group_base_change_detail *subregion_group_change_detail =
						dynamic_cast<const cmzn_field_group_base_change_detail *>(source_change_detail);
					if (subregion_group_change_detail)
					{
						const int subregion_group_change = subregion_group_change_detail->getChangeSummary();
						if (subregion_group_change != CMZN_FIELD_GROUP_CHANGE_NONE)
						{
							this->change_detail.changeMergeNonlocal(subregion_group_change);
							this->field->dependencyChanged();
						}
					}
					else
					{
						display_message(WARNING_MESSAGE, "Sub-region group changes could not be propagated.");
					}
				}
				// we have found only possible subgroup for sub-region:
				break;
			}
		}
	}
}

/**
 * Writes type-specific details of the field to the console. 
 */
int Computed_field_group::list()
{
	int return_code;
	if (field)
	{
		display_message(INFORMATION_MESSAGE, "    Region : ");
		cmzn_region* region = this->field->getRegion();
		if (region)
		{
			char *path = region->getPath();
			display_message(INFORMATION_MESSAGE, "%s", path);
			DEALLOCATE(path);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_group.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_group */

int Computed_field_group::setSubelementHandlingMode(
	cmzn_field_group_subelement_handling_mode modeIn)
{
	if (modeIn == CMZN_FIELD_GROUP_SUBELEMENT_HANDLING_MODE_INVALID)
		return CMZN_ERROR_ARGUMENT;
	if (modeIn != this->subelementHandlingMode)
	{
		this->subelementHandlingMode = modeIn;
		this->setChanged();
	}
	// propagate down subregion group tree
	for (Region_field_map_iterator iter = this->child_region_group_map.begin();
		iter != this->child_region_group_map.end(); iter++)
	{
		cmzn_field_group* subregionGroup = iter->second;
		cmzn_field_group_core_cast(subregionGroup)->setSubelementHandlingMode(modeIn);
	}
	return CMZN_OK;
}

cmzn_field_group* Computed_field_group::getOrCreateSubregionFieldGroup(cmzn_region *subregion, bool canGet, bool canCreate)
{
	cmzn_region* region = this->field->getRegion();
	if (!((subregion) && (canGet || canCreate) && (region)))
	{
		return nullptr;
	}
	if (subregion == region)
	{
		if (canGet)
		{
			return this->getFieldGroup();
		}
		return nullptr;
	}
	if (subregion->getParent() == region)
	{
		Region_field_map_iterator iter = this->child_region_group_map.find(subregion);
		if (iter != this->child_region_group_map.end())
		{
			if (canGet)
			{
				return iter->second;
			}
			return nullptr;
		}
	}
	else if (!region->containsSubregion(subregion))
	{
		display_message(ERROR_MESSAGE, "FieldGroup.get/createSubregionFieldGroup.  Subregion is not in region tree");
		return nullptr;
	}
	// find existing group of same name in subregion
	cmzn_field_group* subregionGroup = nullptr;
	cmzn_field *existingField = subregion->findFieldByName(this->getField()->name);
	if (existingField)
	{
		Computed_field_group* groupCore = dynamic_cast<Computed_field_group*>(existingField->core);
		if (!groupCore)
		{
			display_message(ERROR_MESSAGE, "FieldGroup.get/createSubregionFieldGroup.  Found existing non-group field of name %s", this->getField()->name);
			return nullptr;
		}
		subregionGroup = groupCore->getFieldGroup();
	}
	if ((canGet && (subregionGroup)) || (canCreate && (!existingField)))
	{
		region->beginHierarchicalChange();
		if (subregionGroup)
		{
			cmzn_field_group_base_cast(subregionGroup)->access();
		}
		else
		{
			cmzn_fieldmodule* fieldmodule = cmzn_fieldmodule_create(subregion);
			subregionGroup = reinterpret_cast<cmzn_field_group*>(cmzn_fieldmodule_create_field_group(fieldmodule));
			cmzn_field_set_name(cmzn_field_group_base_cast(subregionGroup), this->getField()->name);
			cmzn_field_group_set_subelement_handling_mode(subregionGroup, this->subelementHandlingMode);
			cmzn_fieldmodule_destroy(&fieldmodule);
		}
		cmzn_region* parentRegion = subregion->getParent();
		Computed_field_group* parentGroupCore = this;
		if (parentRegion != region)
		{
			// get/create and link intermediate groups
			cmzn_field_group* parentGroup = this->getOrCreateSubregionFieldGroup(parentRegion);
			parentGroupCore = cmzn_field_group_core_cast(parentGroup);
		}
		if (parentGroupCore)
		{
			// child_region_group_map takes over access
			parentGroupCore->child_region_group_map.insert(std::make_pair(subregion, subregionGroup));
			parentGroupCore->change_detail.changeMergeNonlocal(CMZN_FIELD_GROUP_CHANGE_ADD);
			parentGroupCore->field->setChanged();
		}
		else
		{
			// failed to get parent to add to
			cmzn_field_group_destroy(&subregionGroup);
		}
		region->endHierarchicalChange();
	}
	return subregionGroup;
}

cmzn_field_group_id Computed_field_group::getFirstNonEmptyGroup()
{
	if (!isEmptyLocal())
	{
		return cmzn_field_cast_group(this->getField());
	}
	if (!child_region_group_map.empty())
	{
		cmzn_field_group_id subregion_group = 0;
		Region_field_map_iterator iter;
		for (Region_field_map_iterator iter = child_region_group_map.begin();
			iter != child_region_group_map.end(); iter++)
		{
			cmzn_field_group_id temp = iter->second;
			Computed_field_group *group_core = cmzn_field_group_core_cast(temp);
			subregion_group = group_core->getFirstNonEmptyGroup();
			if (subregion_group)
				return subregion_group;
		}
	}
	return 0;
}

cmzn_nodeset_group* Computed_field_group::getOrCreateNodesetGroup(FE_nodeset* feNodeset, bool canGet, bool canCreate)
{
	cmzn_region* region = this->field->getRegion();
	if ((!feNodeset) || !(canGet || canCreate) || (!region))
	{
		display_message(ERROR_MESSAGE, "FieldGroup.getOrCreateNodesetGroup.  Invalid argument(s)");
		return nullptr;
	}
	cmzn_region* subregion = feNodeset->getRegion();
	Computed_field_group* groupCore = nullptr;
	if (subregion != region)
	{
		region->beginHierarchicalChange();
		cmzn_field_group* subregionGroup = this->getOrCreateSubregionFieldGroup(subregion, canGet || canCreate, canCreate);
		if (!subregionGroup)
		{
			region->endHierarchicalChange();
			return nullptr;
		}
		groupCore = cmzn_field_group_core_cast(subregionGroup);
	}
	else
	{
		this->beginChange();
		groupCore = this;
	}
	const int index = (feNodeset->getFieldDomainType() == CMZN_FIELD_DOMAIN_TYPE_NODES) ? 0 : 1;
	cmzn_nodeset_group* nodesetGroup = groupCore->nodesetGroups[index];
	if (nodesetGroup)
	{
		if (!canGet)
		{
			nodesetGroup = nullptr;
		}
	}
	else
	{
		nodesetGroup = groupCore->nodesetGroups[index] = new cmzn_nodeset_group(feNodeset, groupCore->getFieldGroup());
	}
	if (subregion != region)
	{
		region->endHierarchicalChange();
	}
	else
	{
		this->endChange();
	}
	return nodesetGroup;
}

cmzn_mesh_group* Computed_field_group::getOrCreateMeshGroup(FE_mesh* feMesh, bool canGet, bool canCreate)
{
	cmzn_region* region = this->field->getRegion();
	if ((!feMesh) || !(canGet || canCreate) || (!region))
	{
		display_message(ERROR_MESSAGE, "FieldGroup.getOrCreateMeshGroup.  Invalid argument(s)");
		return nullptr;
	}
	cmzn_region* subregion = feMesh->getRegion();
	Computed_field_group* groupCore = nullptr;
	if (subregion != region)
	{
		region->beginHierarchicalChange();
		cmzn_field_group* subregionGroup = this->getOrCreateSubregionFieldGroup(subregion, canGet || canCreate, canCreate);
		if (!subregionGroup)
		{
			region->endHierarchicalChange();
			return nullptr;
		}
		groupCore = cmzn_field_group_core_cast(subregionGroup);
	}
	else
	{
		this->beginChange();
		groupCore = this;
	}
	const int index = feMesh->getDimension() - 1;
	cmzn_mesh_group* meshGroup = groupCore->meshGroups[index];
	if (meshGroup)
	{
		if (!canGet)
		{
			meshGroup = nullptr;
		}
	}
	else
	{
		meshGroup = groupCore->meshGroups[index] = new cmzn_mesh_group(feMesh, groupCore->getFieldGroup());
	}
	if (subregion != region)
	{
		region->endHierarchicalChange();
	}
	else
	{
		this->endChange();
	}
	return meshGroup;
}

int Computed_field_group::addRegion(cmzn_region* subregion)
{
	cmzn_region *region = this->field->getRegion();
	if ((region) && region->containsSubregion(subregion))
	{
		region->beginHierarchicalChange();
		cmzn_field_group* subregionGroup = this->getOrCreateSubregionFieldGroup(subregion);
		if (subregionGroup)
		{
			Computed_field_group* groupCore = cmzn_field_group_core_cast(subregionGroup);
			groupCore->addLocalRegion();
		}
		region->endHierarchicalChange();
		return CMZN_OK;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_group::addRegion.  Not a subregion in this region tree");
	}
	return CMZN_ERROR_ARGUMENT;
}

bool Computed_field_group::containsRegion(cmzn_region* subregion)
{
	bool result = false;
	cmzn_field_group* subregionGroup = this->getOrCreateSubregionFieldGroup(subregion, true, false);
	if (subregionGroup)
	{
		Computed_field_group* groupCore = cmzn_field_group_core_cast(subregionGroup);
		result = groupCore->containsLocalRegion();
	}
	return result;
}

int Computed_field_group::removeRegion(cmzn_region* subregion)
{
	if (subregion)
	{
		cmzn_field_group* subregionGroup = this->getOrCreateSubregionFieldGroup(subregion, true, false);
		if (subregionGroup)
		{
			Computed_field_group* groupCore = cmzn_field_group_core_cast(subregionGroup);
			groupCore->clearLocal();
			return CMZN_OK;
		}
		return CMZN_ERROR_NOT_FOUND;
	}
	return CMZN_ERROR_ARGUMENT;
}

const char* Computed_field_group::get_type_string()
{
	return (computed_field_group_type_string);
}

void Computed_field_group::remove_child_group(struct cmzn_region *child_region)
{
	Region_field_map_iterator iter = this->child_region_group_map.find(child_region);
	if (iter != this->child_region_group_map.end())
	{
		cmzn_field_group_id subregion_group_field = iter->second;
		Computed_field_group *group_core = cmzn_field_group_core_cast(subregion_group_field);
		const bool nonEmptySubregionRemoved = !group_core->isEmpty();
		child_region_group_map.erase(iter);
		cmzn_field_group_destroy(&subregion_group_field);
		if (nonEmptySubregionRemoved)
		{
			this->change_detail.changeMergeNonlocal(CMZN_FIELD_GROUP_CHANGE_REMOVE);
			this->field->setChanged();
		}
	}
}

int Computed_field_group::remove_empty_subgroups()
{
	for (int i = 0; i < 2; i++)
	{
		if (this->nodesetGroups[i])
		{
			if (this->nodesetGroups[i]->getSize() == 0)
			{
				this->clearRemoveNodesetGroup(i, /*clear*/false, /*remove*/true);
			}
		}
	}
	for (int i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
	{
		if (this->meshGroups[i])
		{
			if (this->meshGroups[i]->getSize() == 0)
			{
				this->clearRemoveMeshGroup(i, /*clear*/false, /*remove*/true);
			}
		}
	}
	/* remove empty subregion group */
	for (Region_field_map_iterator iter = child_region_group_map.begin();
		iter != child_region_group_map.end();)
	{
		cmzn_field_group_id subregion_group_field = iter->second;
		Computed_field_group *group_core = cmzn_field_group_core_cast(subregion_group_field);
		group_core->remove_empty_subgroups();
		if (group_core->isEmpty())
		{
			// must transfer non-local changes now
			const int subregion_group_change = group_core->change_detail.getChangeSummary();
			this->change_detail.changeMergeNonlocal(subregion_group_change);
			child_region_group_map.erase(iter);
			cmzn_field_group_destroy(&subregion_group_field);
			// reset iterator as probably invalid
			iter = child_region_group_map.begin();
		}
		else
		{
			++iter;
		}
	}
	if (this->change_detail.getChangeSummary() != CMZN_FIELD_GROUP_CHANGE_NONE)
		this->field->dependencyChanged();
	return 1;
}

int Computed_field_group::addLocalRegion()
{
	if (!this->containsAllLocal)
	{
		this->containsAllLocal = true;
		change_detail.changeAddLocal();
		this->field->setChanged();
	}
	return CMZN_OK;
}

int Computed_field_group::removeLocalRegion()
{
	if (this->containsAllLocal)
	{
		this->containsAllLocal = false;
		change_detail.changeRemoveLocal();
		this->field->setChanged();
	}
	return CMZN_OK;
}

bool Computed_field_group::containsLocalRegion()
{
	return containsAllLocal;
}

int Computed_field_group::for_each_group_hiearchical(
	cmzn_field_group_iterator_function function, void *user_data)
{
	int return_code = 0;
	if (field)
	{
		// ensure group is accessed while user function is called so not destroyed
		cmzn_field_id access_field = field->access();
		return_code = function(reinterpret_cast<cmzn_field_group_id>(field), user_data);
		cmzn_field_destroy(&access_field);
		if (return_code)
		{
			for (Region_field_map_iterator child_group_iter = child_region_group_map.begin();
				child_group_iter !=child_region_group_map.end(); ++child_group_iter)
			{
				Computed_field_group *child_group_core = cmzn_field_group_core_cast(child_group_iter->second);
				if ((!child_group_core) ||
					(!child_group_core->for_each_group_hiearchical(function, user_data)))
				{
					return_code = 0;
					break;
				}
			}
		}
	}
	return return_code;
}

cmzn_field_group *cmzn_field_cast_group(cmzn_field_id field)
{

	if ((field) && (dynamic_cast<Computed_field_group*>(field->core)))
	{
		return reinterpret_cast<cmzn_field_group_id>(field->access());
	}
	return nullptr;
}

cmzn_nodeset_group_id cmzn_field_group_create_nodeset_group(
	cmzn_field_group_id group, cmzn_nodeset_id nodeset)
{
	if ((group) && (nodeset))
	{
		Computed_field_group* groupCore = cmzn_field_group_core_cast(group);
		cmzn_nodeset_group* nodesetGroup = groupCore->getOrCreateNodesetGroup(nodeset->getFeNodeset(), false, true);
		if (nodesetGroup)
		{
			nodesetGroup->access();
			return nodesetGroup;
		}
	}
	return nullptr;
}

cmzn_nodeset_group_id cmzn_field_group_get_nodeset_group(
	cmzn_field_group_id group, cmzn_nodeset_id nodeset)
{
	if ((group) && (nodeset))
	{
		Computed_field_group * groupCore = cmzn_field_group_core_cast(group);
		cmzn_nodeset_group* nodesetGroup = groupCore->getOrCreateNodesetGroup(nodeset->getFeNodeset(), true, false);
		if (nodesetGroup)
		{
			nodesetGroup->access();
			return nodesetGroup;
		}
	}
	return nullptr;
}

cmzn_nodeset_group_id cmzn_field_group_get_or_create_nodeset_group(
	cmzn_field_group_id group, cmzn_nodeset_id nodeset)
{
	if ((group) && (nodeset))
	{
		Computed_field_group* groupCore = cmzn_field_group_core_cast(group);
		cmzn_nodeset_group* nodesetGroup = groupCore->getOrCreateNodesetGroup(nodeset->getFeNodeset(), true, true);
		if (nodesetGroup)
		{
			nodesetGroup->access();
			return nodesetGroup;
		}
	}
	return nullptr;
}



cmzn_mesh_group_id cmzn_field_group_create_mesh_group(
	cmzn_field_group_id group, cmzn_mesh_id mesh)
{
	if ((group) && (mesh))
	{
		Computed_field_group* groupCore = cmzn_field_group_core_cast(group);
		cmzn_mesh_group* meshGroup = groupCore->getOrCreateMeshGroup(mesh->getFeMesh(), false, true);
		if (meshGroup)
		{
			meshGroup->access();
			return meshGroup;
		}
	}
	return nullptr;
}

cmzn_mesh_group_id cmzn_field_group_get_mesh_group(
	cmzn_field_group_id group, cmzn_mesh_id mesh)
{
	if ((group) && (mesh))
	{
		Computed_field_group* groupCore = cmzn_field_group_core_cast(group);
		cmzn_mesh_group* meshGroup = groupCore->getOrCreateMeshGroup(mesh->getFeMesh(), true, false);
		if (meshGroup)
		{
			meshGroup->access();
			return meshGroup;
		}
	}
	return nullptr;
}

cmzn_mesh_group_id cmzn_field_group_get_or_create_mesh_group(
	cmzn_field_group_id group, cmzn_mesh_id mesh)
{
	if ((group) && (mesh))
	{
		Computed_field_group* groupCore = cmzn_field_group_core_cast(group);
		cmzn_mesh_group* meshGroup = groupCore->getOrCreateMeshGroup(mesh->getFeMesh(), true, true);
		if (meshGroup)
		{
			meshGroup->access();
			return meshGroup;
		}
	}
	return nullptr;
}

cmzn_field_group_id cmzn_field_group_create_subregion_field_group(
	cmzn_field_group_id group, cmzn_region_id subregion)
{
	if (group)
	{
		Computed_field_group* groupCore = cmzn_field_group_core_cast(group);
		cmzn_field_group* subregionGroup = groupCore->getOrCreateSubregionFieldGroup(subregion, false, true);
		if (subregionGroup)
		{
			cmzn_field_group_base_cast(subregionGroup)->access();
			return subregionGroup;
		}
	}
	return nullptr;
}

cmzn_field_group_id cmzn_field_group_get_subregion_field_group(
	cmzn_field_group_id group, cmzn_region_id subregion)
{
	if (group)
	{
		Computed_field_group* groupCore = cmzn_field_group_core_cast(group);
		cmzn_field_group* subregionGroup = groupCore->getOrCreateSubregionFieldGroup(subregion, true, false);
		if (subregionGroup)
		{
			cmzn_field_group_base_cast(subregionGroup)->access();
			return subregionGroup;
		}
	}
	return nullptr;
}

cmzn_field_group_id cmzn_field_group_get_or_create_subregion_field_group(
	cmzn_field_group_id group, cmzn_region_id subregion)
{
	if (group)
	{
		Computed_field_group *groupCore = cmzn_field_group_core_cast(group);
		cmzn_field_group *subregionGroup = groupCore->getOrCreateSubregionFieldGroup(subregion, true, true);
		if (subregionGroup)
		{
			cmzn_field_group_base_cast(subregionGroup)->access();
			return subregionGroup;
		}
	}
	return nullptr;
}

int cmzn_field_group_remove_empty_subgroups(cmzn_field_group_id group)
{
	if (group)
	{
		Computed_field_group *group_core =
			cmzn_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->remove_empty_subgroups();
		}
	}
	return 0;
}

int cmzn_field_group_destroy(cmzn_field_group_id *group_address)
{
	return cmzn_field_destroy(reinterpret_cast<cmzn_field_id *>(group_address));
}

cmzn_field *cmzn_fieldmodule_create_field_group(cmzn_fieldmodule_id fieldmodule)
{
	cmzn_field *field = nullptr;
	if (fieldmodule)
	{
		cmzn_region_id region = cmzn_fieldmodule_get_region(fieldmodule);
		field = Computed_field_create_generic(fieldmodule,
			/*check_source_field_regions*/true, 1,
			/*number_of_source_fields*/0, NULL,
			/*number_of_source_values*/0, NULL,
			new Computed_field_group(region));
		cmzn_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_fieldmodule_create_field_group.  Invalid argument(s)");
	}
	return (field);
}

int cmzn_field_group_clear(cmzn_field_group_id group)
{
	if (group)
	{
		Computed_field_group *group_core =
			cmzn_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->clear();
		}
	}
	return 0;
}

int cmzn_field_group_clear_local(cmzn_field_group_id group)
{
	if (group)
	{
		Computed_field_group *group_core =
			cmzn_field_group_core_cast(group);
		if (group_core)
			return group_core->clearLocal();
	}
	return CMZN_ERROR_ARGUMENT;
}

bool cmzn_field_group_is_empty(cmzn_field_group_id group)
{
	if (group)
	{
		Computed_field_group *group_core = cmzn_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->isEmpty();
		}
	}
	return false;
}

bool cmzn_field_group_was_modified(cmzn_field_group_id group)
{
	if (group)
	{
		Computed_field_group *group_core = cmzn_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->wasModified();
		}
	}
	return false;
}

bool cmzn_field_group_is_empty_local(cmzn_field_group_id group)
{
	if (group)
	{
		Computed_field_group *group_core = cmzn_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->isEmptyLocal();
		}
	}
	return false;
}

int cmzn_field_group_add_local_region(cmzn_field_group_id group)
{
	if (group)
	{
		Computed_field_group *group_core = cmzn_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->addLocalRegion();
		}
	}
	return CMZN_ERROR_ARGUMENT;
}

bool cmzn_field_group_contains_local_region(cmzn_field_group_id group)
{
	if (group)
	{
		Computed_field_group *group_core = cmzn_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->containsLocalRegion();
		}
	}
	return false;
}

int cmzn_field_group_remove_local_region(cmzn_field_group_id group)
{
	if (group)
	{
		Computed_field_group *group_core = cmzn_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->removeLocalRegion();
		}
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_field_group_add_region(cmzn_field_group_id group, cmzn_region_id region)
{
	if (group && region)
	{
		Computed_field_group *group_core = cmzn_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->addRegion(region);
		}
	}
	return CMZN_ERROR_ARGUMENT;
}

bool cmzn_field_group_contains_region(cmzn_field_group_id group, cmzn_region_id region)
{
	if (group && region)
	{
		Computed_field_group *group_core = cmzn_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->containsRegion(region);
		}
	}
	return false;
}

int cmzn_field_group_remove_region(cmzn_field_group_id group, cmzn_region_id region)
{
	if (group && region)
	{
		Computed_field_group *group_core = cmzn_field_group_core_cast(group);
		if (group_core)
			return group_core->removeRegion(region);
	}
	return CMZN_ERROR_ARGUMENT;
}

enum cmzn_field_group_subelement_handling_mode
	cmzn_field_group_get_subelement_handling_mode(cmzn_field_group_id group)
{
	if (group)
		return cmzn_field_group_core_cast(group)->getSubelementHandlingMode();
	return CMZN_FIELD_GROUP_SUBELEMENT_HANDLING_MODE_INVALID;
}

int cmzn_field_group_set_subelement_handling_mode(cmzn_field_group_id group,
	enum cmzn_field_group_subelement_handling_mode mode)
{
	if (group)
		return cmzn_field_group_core_cast(group)->setSubelementHandlingMode(mode);
	return CMZN_ERROR_ARGUMENT;
}

cmzn_field_group_id cmzn_field_group_get_first_non_empty_subregion_field_group(
	cmzn_field_group_id group)
{
	if (group)
	{
		Computed_field_group *group_core =
			cmzn_field_group_core_cast(group);
		if (group_core)
		{
			return group_core->getFirstNonEmptyGroup();
		}
	}
	return 0;
}
