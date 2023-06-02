/**
 * FILE : nodeset_group.cpp
 *
 * Nodeset group implementation.
 */
 /* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "computed_field/computed_field_group.hpp"
#include "general/mystring.h"
#include "mesh/nodeset_group.hpp"
#include "finite_element/finite_element_region.h"


cmzn_nodeset_group::cmzn_nodeset_group(FE_nodeset* feNodesetIn, cmzn_field_group* groupIn) :
	cmzn_nodeset(feNodesetIn),
	group(groupIn),
	labelsGroup(feNodesetIn->createLabelsGroup())
{
	feNodesetIn->addMapper(this);
}

cmzn_nodeset_group::~cmzn_nodeset_group()
{
	this->feNodeset->removeMapper(this);
	cmzn::Deaccess(this->labelsGroup);
}

void cmzn_nodeset_group::ownerAccess()
{
	cmzn_field_group_base_cast(this->group)->access();
}

void cmzn_nodeset_group::ownerDeaccess()
{
	// not clearing owner
	cmzn_field* field = cmzn_field_group_base_cast(this->group);
	cmzn_field::deaccess(field);
}

void cmzn_nodeset_group::changeAdd()
{
	this->invalidateIterators();
	this->getGroupCore()->changeAddLocal();
}

void cmzn_nodeset_group::changeRemove()
{
	this->invalidateIterators();
	this->getGroupCore()->changeRemoveLocal();
}
void cmzn_nodeset_group::changeRemoveNoNotify()
{
	this->invalidateIterators();
	this->getGroupCore()->changeRemoveLocalNoNotify();
}

const cmzn_nodeset_group* cmzn_nodeset_group::getConditionalNodesetGroup(
	const cmzn_field* conditionalField, bool& isEmptyNodesetGroup) const
{
	Computed_field_group* groupCore = cmzn_field_get_group_core(conditionalField);
	if (!groupCore)
	{
		isEmptyNodesetGroup = false;
		return nullptr;
	}
	cmzn_nodeset_group* conditionalNodesetGroup = groupCore->getLocalNodesetGroup(this->feNodeset);
	isEmptyNodesetGroup = (!conditionalNodesetGroup) || (conditionalNodesetGroup->labelsGroup->getSize() == 0);
	return conditionalNodesetGroup;
}

Computed_field_group* cmzn_nodeset_group::getGroupCore() const
{
	return cmzn_field_group_core_cast(this->group);
}

bool cmzn_nodeset_group::isElementCompatible(const cmzn_element* element)
{
	if ((element) && (element->getMesh()))
	{
		return (this->feNodeset->get_FE_region() == element->getMesh()->get_FE_region());
	}
	return false;
}

cmzn_node* cmzn_nodeset_group::createNode(int identifier, cmzn_nodetemplate* nodetemplate)
{
	cmzn_region* region = this->getRegion();
	region->beginChangeFields();
	cmzn_node* node = cmzn_nodeset::createNode(identifier, nodetemplate);
	if (node)
	{
		this->addNode(node);
	}
	region->endChangeFields();
	return node;
}

cmzn_node* cmzn_nodeset_group::findNodeByIdentifier(int identifier) const
{
	const DsLabelIndex index = this->feNodeset->findIndexByIdentifier(identifier);
	if (this->labelsGroup->hasIndex(index))
	{
		return this->feNodeset->getNode(index);
	}
	return nullptr;
}

char* cmzn_nodeset_group::getName() const
{
	char* name = nullptr;
	int error = 0;
	if (this->group)
	{
		append_string(&name, cmzn_field_group_base_cast(this->group)->getName(), &error);
	}
	append_string(&name, ".", &error);
	append_string(&name, this->feNodeset->getName(), &error);
	return name;
}

bool cmzn_nodeset_group::hasMembershipChanges() const
{
	return (this->group) ? (MANAGER_CHANGE_NONE(cmzn_field) !=
		cmzn_field_group_base_cast(this->group)->manager_change_status) : false;
}

int cmzn_nodeset_group::addNode(const cmzn_node* node)
{
	if (!this->feNodeset->containsNode(node))
	{
		return CMZN_ERROR_ARGUMENT;
	}
	int return_code = this->labelsGroup->setIndex(node->getIndex(), true);
	if (CMZN_OK == return_code)
	{
		this->changeAdd();
	}
	return return_code;
}

int cmzn_nodeset_group::addNodesConditional(cmzn_field* conditionalField)
{
	cmzn_region* region = this->getRegion();
	if ((!region) || (!conditionalField) || (conditionalField->getRegion() != region))
	{
		return CMZN_ERROR_ARGUMENT;
	}
	bool isEmptyNodesetGroup = false;
	const cmzn_nodeset_group* otherNodesetGroup = this->getConditionalNodesetGroup(conditionalField, isEmptyNodesetGroup);
	if (isEmptyNodesetGroup)
	{
		return CMZN_OK;
	}
	if (otherNodesetGroup)
	{
		return this->addNodesInLabelsGroup(*(otherNodesetGroup->labelsGroup));
	}
	cmzn_nodeiterator* iter = this->feNodeset->createNodeiterator();
	cmzn_fieldcache* fieldcache = cmzn_fieldcache::create(region);
	int return_code = CMZN_OK;
	const int oldSize = this->labelsGroup->getSize();
	if ((iter) && (fieldcache))
	{
		cmzn_node* node = nullptr;
		while ((node = iter->nextNode()))
		{
			fieldcache->setNode(node);
			if (!cmzn_field_evaluate_boolean(conditionalField, fieldcache))
			{
				continue;
			}
			const int result = this->labelsGroup->setIndex(node->getIndex(), true);
			if ((result != CMZN_OK) && (result != CMZN_ERROR_ALREADY_EXISTS))
			{
				return_code = result;
				break;
			}
		}
	}
	else
	{
		return_code = CMZN_ERROR_MEMORY;
	}
	cmzn_fieldcache::deaccess(fieldcache);
	cmzn::Deaccess(iter);
	if (this->labelsGroup->getSize() > oldSize)
	{
		this->changeAdd();
	}
	return return_code;
}

int cmzn_nodeset_group::addNodesInLabelsGroup(const DsLabelsGroup& addLabelsGroup)
{
	const int oldSize = this->labelsGroup->getSize();
	this->labelsGroup->addGroup(addLabelsGroup);
	if (this->labelsGroup->getSize() > oldSize)
	{
		this->changeAdd();
	}
	return CMZN_OK;
}

int cmzn_nodeset_group::removeAllNodes()
{
	if (0 < this->getSize())
	{
		this->labelsGroup->clear();
		this->changeRemove();
	}
	return CMZN_OK;
};

int cmzn_nodeset_group::removeNode(const cmzn_node* node)
{
	if (!this->feNodeset->containsNode(node))
	{
		return CMZN_ERROR_ARGUMENT;
	}
	int return_code = this->labelsGroup->setIndex(node->getIndex(), false);
	if (CMZN_OK == return_code)
	{
		this->changeRemove();
	}
	return return_code;
}

int cmzn_nodeset_group::removeNodesConditional(cmzn_field* conditionalField)
{
	cmzn_region* region = this->getRegion();
	if ((!region) || (!conditionalField) || (conditionalField->getRegion() != region))
	{
		return CMZN_ERROR_ARGUMENT;
	}
	const int oldSize = this->labelsGroup->getSize();
	if (oldSize == 0)
	{
		return CMZN_OK;
	}
	bool isEmptyNodesetGroup = false;
	const cmzn_nodeset_group* otherNodesetGroup = this->getConditionalNodesetGroup(conditionalField, isEmptyNodesetGroup);
	if (isEmptyNodesetGroup)
	{
		return CMZN_OK;
	}
	if (otherNodesetGroup)
	{
		return this->removeNodesInLabelsGroup(*(otherNodesetGroup->labelsGroup));
	}
	int return_code = CMZN_OK;
	cmzn_nodeiterator* iter = this->feNodeset->createNodeiterator();
	cmzn_fieldcache* fieldcache = cmzn_fieldcache::create(region);
	cmzn_node* node = nullptr;
	if ((iter) && (fieldcache))
	{
		while ((node = iter->nextNode()))
		{
			fieldcache->setNode(node);
			if (!cmzn_field_evaluate_boolean(conditionalField, fieldcache))
			{
				continue;
			}
			const int result = this->labelsGroup->setIndex(node->getIndex(), false);
			if ((result != CMZN_OK) && (result != CMZN_ERROR_ALREADY_EXISTS))
			{
				return_code = result;
				break;
			}
		}
	}
	else
	{
		return_code = CMZN_ERROR_MEMORY;
	}
	cmzn::Deaccess(iter);
	cmzn_fieldcache::deaccess(fieldcache);
	if (this->labelsGroup->getSize() < oldSize)
	{
		this->changeRemove();
	}
	return return_code;
}

int cmzn_nodeset_group::removeNodesInLabelsGroup(const DsLabelsGroup& removeLabelsGroup)
{
	const int oldSize = this->labelsGroup->getSize();
	this->labelsGroup->removeGroup(removeLabelsGroup);
	if (this->labelsGroup->getSize() < oldSize)
	{
		this->changeRemove();
	}
	return CMZN_OK;
}

int cmzn_nodeset_group::addElementNodes(cmzn_element* element)
{
	if (!isElementCompatible(element))
	{
		return CMZN_ERROR_ARGUMENT;
	}
	if (this->feNodeset != element->getMesh()->getNodeset())
	{
		return CMZN_ERROR_ARGUMENT;
	}
	const int oldSize = this->labelsGroup->getSize();
	const int result = cmzn_element_add_nodes_to_labels_group(element, *this->labelsGroup);
	if (this->labelsGroup->getSize() > oldSize)
	{
		this->changeAdd();
	}
	return result;
};

int cmzn_nodeset_group::removeElementNodes(cmzn_element* element)
{
	if (!isElementCompatible(element))
	{
		return CMZN_ERROR_ARGUMENT;
	}
	if (this->feNodeset != element->getMesh()->getNodeset())
	{
		return CMZN_ERROR_ARGUMENT;
	}
	const int oldSize = this->labelsGroup->getSize();
	const int result = cmzn_element_remove_nodes_from_labels_group(element, *this->labelsGroup);
	if (this->labelsGroup->getSize() < oldSize)
	{
		this->changeRemove();
	}
	return result;
};

void cmzn_nodeset_group::destroyedAllObjects()
{
	if (this->labelsGroup->getSize() > 0)
	{
		this->labelsGroup->clear();
		this->changeRemoveNoNotify();
	}
}

void cmzn_nodeset_group::destroyedObject(DsLabelIndex destroyedIndex)
{
	if (this->labelsGroup->hasIndex(destroyedIndex))
	{
		this->labelsGroup->setIndex(destroyedIndex, false);
		this->changeRemoveNoNotify();
	}
}

void cmzn_nodeset_group::destroyedObjectGroup(const DsLabelsGroup& destroyedLabelsGroup)
{
	const int oldSize = this->labelsGroup->getSize();
	this->labelsGroup->removeGroup(destroyedLabelsGroup);
	if (this->labelsGroup->getSize() < oldSize)
	{
		this->changeRemoveNoNotify();
	}
}

/*
Global functions
----------------
*/

cmzn_nodeset_group_id cmzn_nodeset_cast_group(cmzn_nodeset_id nodeset)
{
	cmzn_nodeset_group *nodesetGroup = dynamic_cast<cmzn_nodeset_group*>(nodeset);
	if (nodesetGroup)
	{
		nodesetGroup->access();
		return nodesetGroup;
	}
	return nullptr;
}

int cmzn_nodeset_group_destroy(cmzn_nodeset_group_id* nodeset_group_address)
{
	if (nodeset_group_address)
	{
		cmzn_nodeset_group::deaccess(*nodeset_group_address);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_nodeset_group_add_node(cmzn_nodeset_group_id nodeset_group, cmzn_node_id node)
{
	if (nodeset_group)
	{
		return nodeset_group->addNode(node);
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_nodeset_group_add_nodes_conditional(
	cmzn_nodeset_group_id nodeset_group, cmzn_field_id conditional_field)
{
	if (nodeset_group)
	{
		return nodeset_group->addNodesConditional(conditional_field);
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_nodeset_group_remove_all_nodes(cmzn_nodeset_group_id nodeset_group)
{
	if (nodeset_group)
	{
		return nodeset_group->removeAllNodes();
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_nodeset_group_remove_node(cmzn_nodeset_group_id nodeset_group, cmzn_node_id node)
{
	if (nodeset_group)
	{
		return nodeset_group->removeNode(node);
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_nodeset_group_remove_nodes_conditional(cmzn_nodeset_group_id nodeset_group,
	cmzn_field_id conditional_field)
{
	if (nodeset_group)
	{
		return nodeset_group->removeNodesConditional(conditional_field);
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_nodeset_group_add_element_nodes(
	cmzn_nodeset_group_id nodeset_group, cmzn_element_id element)
{
	if (nodeset_group)
	{
		return nodeset_group->addElementNodes(element);
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_nodeset_group_remove_element_nodes(
	cmzn_nodeset_group_id nodeset_group, cmzn_element_id element)
{
	if (nodeset_group)
	{
		return nodeset_group->removeElementNodes(element);
	}
	return CMZN_ERROR_ARGUMENT;
}
