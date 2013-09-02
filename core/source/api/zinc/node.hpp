/***************************************************************************//**
 * FILE : node.hpp
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
 * The Original Code is libZinc.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2012
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
#ifndef CMZN_NODE_HPP__
#define CMZN_NODE_HPP__

#include "zinc/node.h"
#include "zinc/field.hpp"

namespace zinc
{

class NodeTemplate;

class Node
{
private:

	cmzn_node_id id;

public:

	Node() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit Node(cmzn_node_id node_id) : id(node_id)
	{ }

	Node(const Node& node) :
		id(cmzn_node_access(node.id))
	{ }

	enum ValueType
	{
		VALUE_TYPE_INVALID = CMZN_NODAL_VALUE_TYPE_INVALID,
		VALUE = CMZN_NODAL_VALUE,
		D_DS1 = CMZN_NODAL_D_DS1,
		D_DS2 = CMZN_NODAL_D_DS2,
		D_DS3 = CMZN_NODAL_D_DS3,
		D2_DS1DS2 = CMZN_NODAL_D2_DS1DS2,
		D2_DS1DS3 = CMZN_NODAL_D2_DS1DS3,
		D2_DS2DS3 = CMZN_NODAL_D2_DS2DS3,
		D3_DS1DS2DS3 = CMZN_NODAL_D3_DS1DS2DS3,
	};

	Node& operator=(const Node& node)
	{
		cmzn_node_id temp_id = cmzn_node_access(node.id);
		if (0 != id)
		{
			cmzn_node_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Node()
	{
		if (0 != id)
		{
			cmzn_node_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_node_id getId()
	{
		return id;
	}

	int getIdentifier()
	{
		return cmzn_node_get_identifier(id);
	}

	int setIdentifier(int identifier)
	{
		return cmzn_node_set_identifier(id, identifier);
	}

	int merge(NodeTemplate nodeTemplate);

};


class NodeTemplate
{
private:

	cmzn_node_template_id id;

public:

	NodeTemplate() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit NodeTemplate(cmzn_node_template_id node_template_id) :
		id(node_template_id)
	{ }

	NodeTemplate(const NodeTemplate& nodeTemplate) :
		id(cmzn_node_template_access(nodeTemplate.id))
	{ }

	NodeTemplate& operator=(const NodeTemplate& nodeTemplate)
	{
		cmzn_node_template_id temp_id = cmzn_node_template_access(nodeTemplate.id);
		if (0 != id)
		{
			cmzn_node_template_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~NodeTemplate()
	{
		if (0 != id)
		{
			cmzn_node_template_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	enum ValueType
	{
		VALUE_TYPE_INVALID = CMZN_NODAL_VALUE_TYPE_INVALID,
		NODAL_VALUE = CMZN_NODAL_VALUE,         /* literal field value */
		NODAL_D_DS1 = CMZN_NODAL_D_DS1,         /* derivative w.r.t. arc length S1 */
		NODAL_D_DS2 = CMZN_NODAL_D_DS2,         /* derivative w.r.t. arc length S2 */
		NODAL_D_DS3 = CMZN_NODAL_D_DS3,         /* derivative w.r.t. arc length S3 */
		NODAL_D2_DS1DS2 = CMZN_NODAL_D2_DS1DS2,     /* cross derivative w.r.t. arc lengths S1,S2 */
		NODAL_D2_DS1DS3 = CMZN_NODAL_D2_DS1DS3,     /* cross derivative w.r.t. arc lengths S1,S3 */
		NODAL_D2_DS2DS3 = CMZN_NODAL_D2_DS2DS3,     /* cross derivative w.r.t. arc lengths S2,S3 */
		NODAL_D3_DS1DS2DS3 = CMZN_NODAL_D3_DS1DS2DS3   /* triple cross derivative w.r.t. arc lengths S1,S2,S3 */
	};

	cmzn_node_template_id getId()
	{
		return id;
	}

	int defineDerivative(Field& field, int componentNumber, ValueType derivativeType)
	{
		return cmzn_node_template_define_derivative(id, field.getId(),
			componentNumber, static_cast<cmzn_nodal_value_type>(derivativeType));
	}

	int defineField(Field& field)
	{
		return cmzn_node_template_define_field(id, field.getId());
	}

	int defineVersions(Field& field, int componentNumber, int numberOfVersions)
	{
		return cmzn_node_template_define_versions(id,
			field.getId(), componentNumber, numberOfVersions);
	}

	int undefineField(Field& field)
	{
		return cmzn_node_template_undefine_field(id, field.getId());
	}
};

class NodeIterator
{
private:

	cmzn_node_iterator_id id;

public:

	NodeIterator() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit NodeIterator(cmzn_node_iterator_id node_iterator_id) :
		id(node_iterator_id)
	{ }

	NodeIterator(const NodeIterator& nodeIterator) :
		id(cmzn_node_iterator_access(nodeIterator.id))
	{ }

	NodeIterator& operator=(const NodeIterator& nodeIterator)
	{
		cmzn_node_iterator_id temp_id = cmzn_node_iterator_access(nodeIterator.id);
		if (0 != id)
		{
			cmzn_node_iterator_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~NodeIterator()
	{
		if (0 != id)
		{
			cmzn_node_iterator_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Node next()
	{
		return Node(cmzn_node_iterator_next(id));
	}
};

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

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_nodeset_id getId()
	{
		return id;
	}

	int containsNode(Node& node)
	{
		return cmzn_nodeset_contains_node(id, node.getId());
	}

	NodeTemplate createNodeTemplate()
	{
		return NodeTemplate(cmzn_nodeset_create_node_template(id));
	}

	Node createNode(int identifier, NodeTemplate& nodeTemplate)
	{
		return Node(cmzn_nodeset_create_node(id, identifier, nodeTemplate.getId()));
	}

	NodeIterator createNodeIterator()
	{
		return NodeIterator(cmzn_nodeset_create_node_iterator(id));
	}

	int destroyAllNodes()
	{
		return cmzn_nodeset_destroy_all_nodes(id);
	}

	int destroyNode(Node& node)
	{
		return cmzn_nodeset_destroy_node(id, node.getId());
	}

	int destroyNodesConditional(Field& conditionalField)
	{
		return cmzn_nodeset_destroy_nodes_conditional(id, conditionalField.getId());
	}

	Node findNodeByIdentifier(int identifier)
	{
		return Node(cmzn_nodeset_find_node_by_identifier(id, identifier));
	}

	Nodeset getMaster()
	{
		return Nodeset(cmzn_nodeset_get_master(id));
	}

	char *getName()
	{
		return cmzn_nodeset_get_name(id);
	}

	int getSize()
	{
		return cmzn_nodeset_get_size(id);
	}

	int match(Nodeset& nodeset)
	{
		return cmzn_nodeset_match(id, nodeset.id);
	}

};

class NodesetGroup  : public Nodeset
{

public:

	// takes ownership of C handle, responsibility for destroying it
	explicit NodesetGroup(cmzn_nodeset_group_id nodeset_id) : Nodeset(reinterpret_cast<cmzn_nodeset_id>(nodeset_id))
	{ }

	cmzn_nodeset_group_id getId()
	{
		return (cmzn_nodeset_group_id)(id);
	}

	int addNode(Node& node)
	{
		return cmzn_nodeset_group_add_node(
			reinterpret_cast<cmzn_nodeset_group_id>(id), node.getId());
	}

	int removeAllNodes()
	{
		return cmzn_nodeset_group_remove_all_nodes(
			reinterpret_cast<cmzn_nodeset_group_id>(id));
	}

	int removeNode(Node& node)
	{
		return cmzn_nodeset_group_remove_node(reinterpret_cast<cmzn_nodeset_group_id>(id),
			node.getId());
	}

	int removeNodesConditional(Field& conditionalField)
	{
		return cmzn_nodeset_group_remove_nodes_conditional(
			reinterpret_cast<cmzn_nodeset_group_id>(id), conditionalField.getId());
	}

};

inline int Node::merge(NodeTemplate nodeTemplate)
{
	return cmzn_node_merge(id, nodeTemplate.getId());
}

}  // namespace zinc

#endif
