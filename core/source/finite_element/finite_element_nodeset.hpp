/**
 * FILE : finite_element_nodeset.hpp
 *
 * Class defining a domain consisting of a set of nodes.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (FINITE_ELEMENT_NODESET_HPP)
#define FINITE_ELEMENT_NODESET_HPP

#include "opencmiss/zinc/status.h"
#include "datastore/labels.hpp"
#include "datastore/labelschangelog.hpp"
#include "datastore/maparray.hpp"
#include "finite_element/finite_element.h"
#include "general/block_array.hpp"
#include "general/list.h"

/**
* Template for creating a new node in the given FE_nodeset
* @see FE_nodeset::create_FE_node()
*/
class FE_node_template : public cmzn::RefCounted
{
	friend class FE_nodeset;

	FE_nodeset *nodeset; // Note: accessed: prevents nodeset from being destroyed
	FE_node *template_node;

	FE_node_template(FE_nodeset *nodeset_in, struct FE_node_field_info *node_field_info);

	/** Creates a template copying from an existing node */
	FE_node_template(FE_nodeset *nodeset_in, struct FE_node *node);

	~FE_node_template();

	FE_node_template(); // not implemented
	FE_node_template(const FE_node_template&); // not implemented

public:

	FE_nodeset *get_nodeset() const
	{
		return this->nodeset;
	}

	FE_node *get_template_node() const
	{
		return this->template_node;
	}
};

/**
 * A set of nodes/datapoints in the FE_region.
 */
class FE_nodeset
{
	FE_region *fe_region; // not accessed
	cmzn_field_domain_type domainType;

	DsLabels labels; // node identifiers

	// map labels -> FE_node (accessed)
	block_array<DsLabelIndex, FE_node*> fe_nodes;

	// number of element references to node, so can't destroy while in use
	typedef unsigned short ElementUsageCountType; // internal use only
	block_array<DsLabelIndex, ElementUsageCountType> elementUsageCount;

	struct LIST(FE_node_field_info) *node_field_info_list;
	struct FE_node_field_info *last_fe_node_field_info;

	// log of nodes added, removed or otherwise changed
	DsLabelsChangeLog *changeLog;

	// list of node iterators to invalidate when nodeset destroyed
	cmzn_nodeiterator *activeNodeIterators;

	int access_count;

	FE_nodeset(FE_region *fe_region);

	~FE_nodeset();

	void createChangeLog();

	int remove_FE_node_private(struct FE_node *node);

	int merge_FE_node_existing(struct FE_node *destination, struct FE_node *source);

	struct Merge_FE_node_external_data;
	int merge_FE_node_external(struct FE_node *node,
		Merge_FE_node_external_data &data);

public:

	static FE_nodeset *create(FE_region *fe_region)
	{
		if (fe_region)
			return new FE_nodeset(fe_region);
		return 0;
	}

	void detach_from_FE_region();

	FE_nodeset *access()
	{
		++(this->access_count);
		return this;
	}

	static void deaccess(FE_nodeset *&nodeset)
	{
		if (nodeset)
		{
			--(nodeset->access_count);
			if (nodeset->access_count <= 0)
				delete nodeset;
			nodeset = 0;
		}
	}

	// in following change is a logical OR of values from enum DsLabelChangeType
	void nodeChange(DsLabelIndex nodeIndex, int change);
	void nodeChange(DsLabelIndex nodeIndex, int change, FE_node *field_info_node);
	void nodeFieldChange(FE_node *node, FE_field *fe_field);

	void nodeIdentifierChange(FE_node *node)
	{
		nodeChange(get_FE_node_index(node), DS_LABEL_CHANGE_TYPE_IDENTIFIER);
	}

	void nodeAddedChange(FE_node *node)
	{
		nodeChange(get_FE_node_index(node), DS_LABEL_CHANGE_TYPE_ADD, node);
	}

	void nodeRemovedChange(FE_node *node)
	{
		nodeChange(get_FE_node_index(node), DS_LABEL_CHANGE_TYPE_REMOVE, node);
	}

	FE_region *get_FE_region()
	{
		return this->fe_region;
	}

	cmzn_field_domain_type getFieldDomainType() const
	{
		return this->domainType;
	}

	void setFieldDomainType(cmzn_field_domain_type domainTypeIn)
	{
		this->domainType = domainTypeIn;
		this->labels.setName(this->getName());
	}

	const char *getName() const;

	const DsLabels& getLabels() const
	{
		return this->labels;
	}

	void clear();

	/** @return Accessed changes */
	DsLabelsChangeLog *extractChangeLog();

	/** @retrun non-Accessed changes */
	DsLabelsChangeLog *getChangeLog()
	{
		return this->changeLog;
	}

	struct LIST(FE_node_field_info) *get_FE_node_field_info_list_private()
	{
		return this->node_field_info_list;
	}

	struct FE_node_field_info *get_FE_node_field_info(int number_of_values,
		struct LIST(FE_node_field) *fe_node_field_list);

	int get_FE_node_field_info_adding_new_field(
		struct FE_node_field_info **node_field_info_address,
		struct FE_node_field *new_node_field, int new_number_of_values);

	struct FE_node_field_info *clone_FE_node_field_info(
		struct FE_node_field_info *fe_node_field_info);

	int remove_FE_node_field_info(struct FE_node_field_info *fe_node_field_info);

	bool is_FE_field_in_use(struct FE_field *fe_field);

	int getElementUsageCount(DsLabelIndex nodeIndex);
	void incrementElementUsageCount(DsLabelIndex nodeIndex);
	void decrementElementUsageCount(DsLabelIndex nodeIndex);

	/** get size i.e. number of nodes in nodeset */
	int getSize() const
	{
		return this->labels.getSize();
	}

	inline DsLabelIdentifier getNodeIdentifier(DsLabelIndex nodeIndex) const
	{
		return this->labels.getIdentifier(nodeIndex);
	}

	/** @return  Non-accessed node object at index */
	inline FE_node *getNode(DsLabelIndex nodeIndex) const
	{
		FE_node *node = 0;
		if (nodeIndex >= 0)
			this->fe_nodes.getValue(nodeIndex, node);
		return node;
	}

	DsLabelIdentifier get_next_FE_node_identifier(DsLabelIdentifier start_identifier)
	{
		return this->labels.getFirstFreeIdentifier(start_identifier);
	}

	void list_btree_statistics();

	bool containsNode(FE_node *node) const
	{
		return (FE_node_get_FE_nodeset(node) == this) && (get_FE_node_index(node) >= 0);
	}

	DsLabelIndex findIndexByIdentifier(DsLabelIdentifier identifier) const
	{
		return this->labels.findLabelByIdentifier(identifier);
	}

	/** @return  Non-accessed node */
	FE_node *findNodeByIdentifier(DsLabelIdentifier identifier) const
	{
		return this->getNode(this->labels.findLabelByIdentifier(identifier));
	}

	void removeNodeiterator(cmzn_nodeiterator *iterator); // private, but needed by cmzn_nodeiterator

	cmzn_nodeiterator *createNodeiterator(DsLabelsGroup *labelsGroup = 0);

	int for_each_FE_node(LIST_ITERATOR_FUNCTION(FE_node) iterator_function, void *user_data_void);

	DsLabelsGroup *createLabelsGroup();

	int change_FE_node_identifier(struct FE_node *node, int new_identifier);

	FE_node_template *create_FE_node_template(FE_node *node);

	FE_node_template *create_FE_node_template();

	FE_node *get_or_create_FE_node_with_identifier(int identifier);

	FE_node *create_FE_node(int identifier, FE_node_template *node_template);

	int merge_FE_node_template(struct FE_node *destination, FE_node_template *fe_node_template);

	int undefineFieldAtNode(struct FE_node *node, struct FE_field *fe_field);

	int destroyNode(struct FE_node *node);

	int destroyAllNodes();

	int destroyNodesInGroup(DsLabelsGroup& labelsGroup);

	int get_last_FE_node_identifier();

	bool FE_field_has_multiple_times(struct FE_field *fe_field);

	bool canMerge(FE_nodeset &source);

	int merge(FE_nodeset &source);
};

struct cmzn_nodeiterator : public cmzn::RefCounted
{
	friend class FE_nodeset;

private:
	FE_nodeset *fe_nodeset;
	DsLabelIterator *iter;
	cmzn_nodeiterator *nextIterator; // for linked list of active iterators in FE_nodeset

	// takes ownership of iter_in access count
	cmzn_nodeiterator(FE_nodeset *fe_nodeset_in, DsLabelIterator *iter_in) :
		fe_nodeset(fe_nodeset_in),
		iter(iter_in),
		nextIterator(0)
	{
	}

	virtual ~cmzn_nodeiterator()
	{
		if (this->fe_nodeset)
			this->fe_nodeset->removeNodeiterator(this);
		cmzn::Deaccess(this->iter);
	}

	void invalidate()
	{
		// sufficient to clear fe_nodeset as only called from FE_nodeset destructor
		this->fe_nodeset = 0;
	}

	template<class REFCOUNTED>
	friend inline void cmzn::Deaccess(REFCOUNTED*& nodeIterator);

public:

	/** @return  Non-accessed node or 0 if iteration ended or iterator invalidated */
	FE_node *nextNode()
	{
		if (this->fe_nodeset)
		{
			const DsLabelIndex nodeIndex = this->iter->nextIndex();
			return this->fe_nodeset->getNode(nodeIndex);
		}
		return 0;
	}

	void setIndex(DsLabelIndex index)
	{
		this->iter->setIndex(index);
	}
};

#endif /* !defined (FINITE_node_NODESET_HPP) */
