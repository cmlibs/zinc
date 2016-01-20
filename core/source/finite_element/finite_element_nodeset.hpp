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

#include "finite_element/finite_element.h"
#include "general/change_log.h"
#include "general/list.h"

/**
 * A set of nodes/datapoints in the FE_region.
 */
class FE_nodeset
{
	FE_region *fe_region; // not accessed
	cmzn_field_domain_type domainType;
	struct LIST(FE_node) *nodeList;
	struct LIST(FE_node_field_info) *node_field_info_list;
	struct FE_node_field_info *last_fe_node_field_info;
	// nodes added, removed or otherwise changed
	struct CHANGE_LOG(FE_node) *fe_node_changes;
	int next_fe_node_identifier_cache;
	int access_count;

	FE_nodeset(FE_region *fe_region);

	~FE_nodeset();

public:

	static FE_nodeset *create(FE_region *fe_region)
	{
		if (fe_region)
			return new FE_nodeset(fe_region);
		return 0;
	}

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

	void createChangeLog();

	struct CHANGE_LOG(FE_node) *extractChangeLog();

	struct CHANGE_LOG(FE_node) *getChangeLog()
	{
		return this->fe_node_changes;
	}

	bool containsNode(FE_node *node);

	struct LIST(FE_node) *createRelatedNodeList();

	void detach_from_FE_region();

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

	FE_region *get_FE_region()
	{
		return this->fe_region;
	}

	struct LIST(FE_node) *getNodeList()
	{
		return this->nodeList;
	}

	// @return  Non-accessed node
	FE_node *findNodeByIdentifier(int identifier)
	{
		return FIND_BY_IDENTIFIER_IN_LIST(FE_node,cm_node_identifier)(identifier, this->nodeList);
	}

	cmzn_field_domain_type getFieldDomainType() const
	{
		return this->domainType;
	}

	void setFieldDomainType(cmzn_field_domain_type domainTypeIn)
	{
		this->domainType = domainTypeIn;
	}

	void nodeChange(FE_node *node, CHANGE_LOG_CHANGE(FE_node) change, FE_node *field_info_node);
	void nodeFieldChange(FE_node *node, FE_field *fe_field);
	void nodeIdentifierChange(FE_node *node);
	void nodeRemovedChange(FE_node *node);

	void clear();
	int change_FE_node_identifier(struct FE_node *node, int new_identifier);
	FE_node *get_or_create_FE_node_with_identifier(int identifier);
	int get_next_FE_node_identifier(int start_identifier);
	int undefine_FE_field_in_FE_node_list(struct FE_field *fe_field,
		struct LIST(FE_node) *fe_node_list, int *number_in_elements_address);
	struct FE_node *create_FE_node_copy(int identifier, struct FE_node *source);
	struct FE_node *merge_FE_node(struct FE_node *node);
	int merge_FE_node_existing(struct FE_node *destination, struct FE_node *source);
	int for_each_FE_node(LIST_ITERATOR_FUNCTION(FE_node) iterator_function, void *user_data);
	cmzn_nodeiterator_id createNodeiterator();
	int remove_FE_node(struct FE_node *node);
	int remove_FE_node_list(struct LIST(FE_node) *remove_node_list);
	int get_last_FE_node_identifier();
	int get_number_of_FE_nodes();
	bool FE_field_has_multiple_times(struct FE_field *fe_field);
	void list_btree_statistics();
	bool is_FE_field_in_use(struct FE_field *fe_field);

	struct Merge_FE_node_external_data;
	int merge_FE_node_external(struct FE_node *node,
		Merge_FE_node_external_data &data);

	bool canMerge(FE_nodeset &source);

	int merge(FE_nodeset &source);
};

#endif /* !defined (FINITE_ELEMENT_NODESET_HPP) */
