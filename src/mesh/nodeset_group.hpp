/**
 * FILE : nodeset_group.hpp
 *
 * Interface to nodeset group implementation.
 */
 /* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "cmlibs/zinc/fieldgroup.h"
#include "datastore/labelsgroup.hpp"
#include "mesh/nodeset.hpp"


class Computed_field_group;

struct cmzn_nodeset_group : public cmzn_nodeset, public FE_domain_mapper
{
	friend struct cmzn_field_group;

	cmzn_field_group* group;  // not accessed, cleared if destroyed before this
	DsLabelsGroup* labelsGroup;

	cmzn_nodeset_group(FE_nodeset* feNodesetIn, cmzn_field_group* groupIn);

	~cmzn_nodeset_group();

	/* Record that group has changed by object add, with client notification */
	void changeAdd();

	/* Record that group has changed by object remove, with client notification */
	void changeRemove();

	/* Record that group has changed by object remove, but do not notify clients.
	 * Only used when objects removed because destroyed in parent domain. */
	void changeRemoveNoNotify();

	/** Only called by owning group when it is destroyed to detach this */
	void detachFromGroup();

	inline void invalidateIterators()
	{
		this->labelsGroup->invalidateLabelIterators();
	}

	/** If the conditionalField is a group, get the nodeset group for this master nodeset.
	 * @param  isEmptyNodesetGroup  Set to true if field is a group containing no nodes
	 * for this master nodeset, otherwise false.
	 * @return  Non-accessed cmzn_node_group, or nullptr if none found or not a group */
	const cmzn_nodeset_group* getConditionalNodesetGroup(
		const cmzn_field* conditionalField, bool& isEmptyNodesetGroup) const;

	/** get group core object or nullptr if detached */
	Computed_field_group* getGroupCore() const;

	/** @return  true if element is from same region as this nodeset */
	bool isElementCompatible(const cmzn_element* element);

public:

	static void deaccess(cmzn_nodeset_group*& nodeset_group)
	{
		cmzn_nodeset::deaccess(reinterpret_cast<cmzn_nodeset*&>(nodeset_group));
	}

	/** Create node and ensure it's in this group.
	 * @return  Accessed new node or nullptr if failed. */
	virtual cmzn_node* createNode(int identifier, cmzn_nodetemplate* nodetemplate);

	virtual bool containsNode(cmzn_node* node) const
	{
		return cmzn_nodeset::containsNode(node) && this->labelsGroup->hasIndex(node->getIndex());
	}

	bool containsIndex(DsLabelIndex elementIndex) const
	{
		return this->labelsGroup->hasIndex(elementIndex);
	}

	virtual cmzn_nodeiterator* createNodeiterator() const
	{
		return this->feNodeset->createNodeiterator(this->labelsGroup);
	}

	virtual int destroyAllNodes()
	{
		return this->feNodeset->destroyNodesInGroup(*(this->labelsGroup));
	}

	/** @return  Non-accessed node, or nullptr if not found */
	virtual cmzn_node* findNodeByIdentifier(int identifier) const;

	const DsLabelsGroup* getLabelsGroup() const
	{
		return this->labelsGroup;
	}

	/** @return  Allocated name as GROUP_NAME.MESH_NAME, or nullptr if failed */
	virtual char* getName() const;

	virtual int getSize() const
	{
		return this->labelsGroup->getSize();
	}

	/** @return  Non-accessed group owning this mesh group or nullptr if detached */
	cmzn_field_group* getFieldGroup() const
	{
		return this->group;
	}
	/** @return  True if nodeset group has recorded changes in membership */
	virtual bool cmzn_nodeset_group::hasMembershipChanges() const;

	/** @return  Result OK if added, ERROR_ALREADY_EXISTS if already in group,
	 * ERROR_ARGUMENT if invalid node */
	int addNode(const cmzn_node* node);

	int addNodesConditional(cmzn_field* conditionalField);

	/** Add nodes with identifier ranges from first to last, inclusive */
	int addNodesInIdentifierRange(DsLabelIdentifier first, DsLabelIdentifier last)
	{
		return this->labelsGroup->addIndexesInIdentifierRange(first, last);
	}

	int addNodesInLabelsGroup(const DsLabelsGroup& addLabelsGroup);

	int removeAllNodes();

	int removeNode(const cmzn_node* node);

	int removeNodesConditional(cmzn_field* conditional_field);

	int removeNodesInLabelsGroup(const DsLabelsGroup& removeLabelsGroup);

	int addElementNodes(cmzn_element* element);

	int removeElementNodes(cmzn_element* element);

	void destroyedAllObjects();

	void destroyedObject(DsLabelIndex destroyedIndex);

	void destroyedObjectGroup(const DsLabelsGroup& destroyedLabelsGroup);

};

/**
 * Ensures all nodes of the supplied element are in this nodeset_group.
 * Candidate for external API.
 *
 * @param nodeset_group  The nodeset group to add nodes to. Must be a subgroup
 * for the master nodeset expected to own the element's nodes.
 * @param element  The element whose nodes are to be added. Face elements
 * inherit nodes from parent elements via field mappings.
 * @return  Result OK on success, any other value on failure.
 */
int cmzn_nodeset_group_add_element_nodes(
	cmzn_nodeset_group_id nodeset_group, cmzn_element_id element);

/**
 * Ensures all nodes of the supplied element are not in this nodeset_group.
 * Candidate for external API.
 *
 * @param nodeset_group  The nodeset group to remove nodes from. Must be a
 * subgroup for the master nodeset expected to own the element's nodes.
 * @param element  The element whose nodes are to be removed. Face elements
 * inherit nodes from parent elements via field mappings.
 * @return  Result OK on success, any other value on failure.
 */
int cmzn_nodeset_group_remove_element_nodes(
	cmzn_nodeset_group_id nodeset_group, cmzn_element_id element);
