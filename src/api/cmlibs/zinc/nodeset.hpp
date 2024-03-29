/**
 * @file nodeset.hpp
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_NODESET_HPP__
#define CMZN_NODESET_HPP__

#include "cmlibs/zinc/nodeset.h"
#include "cmlibs/zinc/field.hpp"
#include "cmlibs/zinc/node.hpp"
#include "cmlibs/zinc/nodetemplate.hpp"

namespace CMLibs
{
namespace Zinc
{

class NodesetGroup;

class Nodeset
{
protected:

	cmzn_nodeset_id id;

public:

	Nodeset() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Nodeset(cmzn_nodeset_id nodeset_id) : id(nodeset_id)
	{  }

	Nodeset(const Nodeset& nodeset) :
		id(cmzn_nodeset_access(nodeset.id))
	{  }

	Nodeset& operator=(const Nodeset& nodeset)
	{
		cmzn_nodeset_id temp_id = cmzn_nodeset_access(nodeset.id);
		if (0 != id)
		{
			cmzn_nodeset_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Nodeset()
	{
		if (0 != id)
		{
			cmzn_nodeset_destroy(&id);
		}
	}

	bool isValid() const
	{
		return (0 != id);
	}

	cmzn_nodeset_id getId() const
	{
		return id;
	}

	inline NodesetGroup castGroup();

	bool containsNode(const Node& node) const
	{
		return cmzn_nodeset_contains_node(id, node.getId());
	}

	Nodetemplate createNodetemplate()
	{
		return Nodetemplate(cmzn_nodeset_create_nodetemplate(id));
	}

	Node createNode(int identifier, const Nodetemplate& nodeTemplate)
	{
		return Node(cmzn_nodeset_create_node(id, identifier, nodeTemplate.getId()));
	}

	Nodeiterator createNodeiterator()
	{
		return Nodeiterator(cmzn_nodeset_create_nodeiterator(id));
	}

	int destroyAllNodes()
	{
		return cmzn_nodeset_destroy_all_nodes(id);
	}

	int destroyNode(const Node& node)
	{
		return cmzn_nodeset_destroy_node(id, node.getId());
	}

	int destroyNodesConditional(const Field& conditionalField)
	{
		return cmzn_nodeset_destroy_nodes_conditional(id, conditionalField.getId());
	}

	Node findNodeByIdentifier(int identifier) const
	{
		return Node(cmzn_nodeset_find_node_by_identifier(id, identifier));
	}

	inline Fieldmodule getFieldmodule() const;

	Nodeset getMasterNodeset() const
	{
		return Nodeset(cmzn_nodeset_get_master_nodeset(id));
	}

	char *getName() const
	{
		return cmzn_nodeset_get_name(id);
	}

	int getSize() const
	{
		return cmzn_nodeset_get_size(id);
	}

};

inline bool operator==(const Nodeset& a, const Nodeset& b)
{
	return a.getId() == b.getId();
}

inline Nodeset Node::getNodeset() const
{
	return Nodeset(cmzn_node_get_nodeset(id));
}

class NodesetGroup : public Nodeset
{
	inline cmzn_nodeset_group_id getDerivedId() const
	{
		return reinterpret_cast<cmzn_nodeset_group_id>(this->id);
	}

public:

	// takes ownership of C handle, responsibility for destroying it
	explicit NodesetGroup(cmzn_nodeset_group_id nodeset_id) : Nodeset(reinterpret_cast<cmzn_nodeset_id>(nodeset_id))
	{ }

	NodesetGroup()
	{ }

	cmzn_nodeset_group_id getId() const
	{
		return (cmzn_nodeset_group_id)(this->id);
	}

	int addNode(const Node& node)
	{
		return cmzn_nodeset_group_add_node(this->getDerivedId(), node.getId());
	}

	int addNodesConditional(const Field& conditionalField)
	{
		return cmzn_nodeset_group_add_nodes_conditional(
			this->getDerivedId(), conditionalField.getId());
	}

	inline FieldGroup getFieldGroup() const;

	int removeAllNodes()
	{
		return cmzn_nodeset_group_remove_all_nodes(this->getDerivedId());
	}

	int removeNode(const Node& node)
	{
		return cmzn_nodeset_group_remove_node(this->getDerivedId(),
			node.getId());
	}

	int removeNodesConditional(const Field& conditionalField)
	{
		return cmzn_nodeset_group_remove_nodes_conditional(
			this->getDerivedId(), conditionalField.getId());
	}

};

inline NodesetGroup Nodeset::castGroup()
{
	return NodesetGroup(cmzn_nodeset_cast_group(id));
}

class Nodesetchanges
{
private:

	cmzn_nodesetchanges_id id;

public:

	Nodesetchanges() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit Nodesetchanges(cmzn_nodesetchanges_id nodesetchanges_id) :
		id(nodesetchanges_id)
	{ }

	Nodesetchanges(const Nodesetchanges& nodesetchanges) :
		id(cmzn_nodesetchanges_access(nodesetchanges.id))
	{ }

	Nodesetchanges& operator=(const Nodesetchanges& nodesetchanges)
	{
		cmzn_nodesetchanges_id temp_id = cmzn_nodesetchanges_access(nodesetchanges.id);
		if (0 != id)
			cmzn_nodesetchanges_destroy(&id);
		id = temp_id;
		return *this;
	}

	~Nodesetchanges()
	{
		if (0 != id)
			cmzn_nodesetchanges_destroy(&id);
	}

	bool isValid() const
	{
		return (0 != id);
	}

	Node::ChangeFlags getNodeChangeFlags(const Node& node) const
	{
		return cmzn_nodesetchanges_get_node_change_flags(id, node.getId());
	}

	int getNumberOfChanges() const
	{
		return cmzn_nodesetchanges_get_number_of_changes(id);
	}

	Node::ChangeFlags getSummaryNodeChangeFlags() const
	{
		return cmzn_nodesetchanges_get_summary_node_change_flags(id);
	}
};

}  // namespace Zinc
}

#endif // CMZN_NODESET_HPP__
