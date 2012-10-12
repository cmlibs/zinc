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
#ifndef __ZN_NODE_HPP__
#define __ZN_NODE_HPP__

#include "api/cmiss_node.h"
#include "api++/field.hpp"

namespace Zn
{

class NodeTemplate;

class Node
{
private:

	Cmiss_node_id id;

public:

	Node() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit Node(Cmiss_node_id node_id) : id(node_id)
	{ }

	Node(const Node& node) :
		id(Cmiss_node_access(node.id))
	{ }

	enum ValueType
	{
		VALUE_TYPE_INVALID = CMISS_NODAL_VALUE_TYPE_INVALID,
		VALUE = CMISS_NODAL_VALUE,
		D_DS1 = CMISS_NODAL_D_DS1,
		D_DS2 = CMISS_NODAL_D_DS2,
		D_DS3 = CMISS_NODAL_D_DS3,
		D2_DS1DS2 = CMISS_NODAL_D2_DS1DS2,
		D2_DS1DS3 = CMISS_NODAL_D2_DS1DS3,
		D2_DS2DS3 = CMISS_NODAL_D2_DS2DS3,
		D3_DS1DS2DS3 = CMISS_NODAL_D3_DS1DS2DS3,
	};

	Node& operator=(const Node& node)
	{
		Cmiss_node_id temp_id = Cmiss_node_access(node.id);
		if (0 != id)
		{
			Cmiss_node_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Node()
	{
		if (0 != id)
		{
			Cmiss_node_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Cmiss_node_id getId()
	{
		return id;
	}

	int getIdentifier()
	{
		return Cmiss_node_get_identifier(id);
	}

	int merge(NodeTemplate nodeTemplate);

};


class NodeTemplate
{
private:

	Cmiss_node_template_id id;

public:

	NodeTemplate() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit NodeTemplate(Cmiss_node_template_id node_template_id) :
		id(node_template_id)
	{ }

	NodeTemplate(const NodeTemplate& nodeTemplate) :
		id(Cmiss_node_template_access(nodeTemplate.id))
	{ }

	NodeTemplate& operator=(const NodeTemplate& nodeTemplate)
	{
		Cmiss_node_template_id temp_id = Cmiss_node_template_access(nodeTemplate.id);
		if (0 != id)
		{
			Cmiss_node_template_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~NodeTemplate()
	{
		if (0 != id)
		{
			Cmiss_node_template_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	enum ValueType
	{
		VALUE_TYPE_INVALID = CMISS_NODAL_VALUE_TYPE_INVALID,
		NODAL_VALUE = CMISS_NODAL_VALUE,         /* literal field value */
		NODAL_D_DS1 = CMISS_NODAL_D_DS1,         /* derivative w.r.t. arc length S1 */
		NODAL_D_DS2 = CMISS_NODAL_D_DS2,         /* derivative w.r.t. arc length S2 */
		NODAL_D_DS3 = CMISS_NODAL_D_DS3,         /* derivative w.r.t. arc length S3 */
		NODAL_D2_DS1DS2 = CMISS_NODAL_D2_DS1DS2,     /* cross derivative w.r.t. arc lengths S1,S2 */
		NODAL_D2_DS1DS3 = CMISS_NODAL_D2_DS1DS3,     /* cross derivative w.r.t. arc lengths S1,S3 */
		NODAL_D2_DS2DS3 = CMISS_NODAL_D2_DS2DS3,     /* cross derivative w.r.t. arc lengths S2,S3 */
		NODAL_D3_DS1DS2DS3 = CMISS_NODAL_D3_DS1DS2DS3   /* triple cross derivative w.r.t. arc lengths S1,S2,S3 */
	};

	Cmiss_node_template_id getId()
	{
		return id;
	}

	int defineDerivative(Field& field, int componentNumber, ValueType derivativeType)
	{
		return Cmiss_node_template_define_derivative(id, field.getId(),
			componentNumber, static_cast<Cmiss_nodal_value_type>(derivativeType));
	}

	int defineField(Field& field)
	{
		return Cmiss_node_template_define_field(id, field.getId());
	}

	int defineVersions(Field& field, int componentNumber, int numberOfVersions)
	{
		return Cmiss_node_template_define_versions(id,
			field.getId(), componentNumber, numberOfVersions);
	}

	int undefineField(Field& field)
	{
		return Cmiss_node_template_undefine_field(id, field.getId());
	}
};

class NodeIterator
{
private:

	Cmiss_node_iterator_id id;

public:

	NodeIterator() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit NodeIterator(Cmiss_node_iterator_id node_iterator_id) :
		id(node_iterator_id)
	{ }

	NodeIterator(const NodeIterator& nodeIterator) :
		id(Cmiss_node_iterator_access(nodeIterator.id))
	{ }

	NodeIterator& operator=(const NodeIterator& nodeIterator)
	{
		Cmiss_node_iterator_id temp_id = Cmiss_node_iterator_access(nodeIterator.id);
		if (0 != id)
		{
			Cmiss_node_iterator_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~NodeIterator()
	{
		if (0 != id)
		{
			Cmiss_node_iterator_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Node next()
	{
		return Node(Cmiss_node_iterator_next(id));
	}
};

class Nodeset
{
protected:

	Cmiss_nodeset_id id;

public:

	Nodeset() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Nodeset(Cmiss_nodeset_id nodeset_id) : id(nodeset_id)
	{  }

	Nodeset(const Nodeset& nodeset) :
		id(Cmiss_nodeset_access(nodeset.id))
	{  }

	Nodeset& operator=(const Nodeset& nodeset)
	{
		Cmiss_nodeset_id temp_id = Cmiss_nodeset_access(nodeset.id);
		if (0 != id)
		{
			Cmiss_nodeset_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Nodeset()
	{
		if (0 != id)
		{
			Cmiss_nodeset_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	Cmiss_nodeset_id getId()
	{
		return id;
	}

	int containsNode(Node& node)
	{
		return Cmiss_nodeset_contains_node(id, node.getId());
	}

	NodeTemplate createNodeTemplate()
	{
		return NodeTemplate(Cmiss_nodeset_create_node_template(id));
	}

	Node createNode(int identifier, NodeTemplate& nodeTemplate)
	{
		return Node(Cmiss_nodeset_create_node(id, identifier, nodeTemplate.getId()));
	}

	NodeIterator createNodeIterator()
	{
		return NodeIterator(Cmiss_nodeset_create_node_iterator(id));
	}

	int destroyAllNodes()
	{
		return Cmiss_nodeset_destroy_all_nodes(id);
	}

	int destroyNode(Node& node)
	{
		return Cmiss_nodeset_destroy_node(id, node.getId());
	}

	int destroyNodesConditional(Field& conditionalField)
	{
		return Cmiss_nodeset_destroy_nodes_conditional(id, conditionalField.getId());
	}

	Node findNodeByIdentifier(int identifier)
	{
		return Node(Cmiss_nodeset_find_node_by_identifier(id, identifier));
	}

	Nodeset getMaster()
	{
		return Nodeset(Cmiss_nodeset_get_master(id));
	}

	char *getName()
	{
		return Cmiss_nodeset_get_name(id);
	}

	int getSize()
	{
		return Cmiss_nodeset_get_size(id);
	}

	int match(Nodeset& nodeset)
	{
		return Cmiss_nodeset_match(id, nodeset.id);
	}

};

class NodesetGroup  : public Nodeset
{

public:

	// takes ownership of C handle, responsibility for destroying it
	explicit NodesetGroup(Cmiss_nodeset_group_id nodeset_id) : Nodeset(reinterpret_cast<Cmiss_nodeset_id>(nodeset_id))
	{ }

	Cmiss_nodeset_group_id getId()
	{
		return (Cmiss_nodeset_group_id)(id);
	}

	int addNode(Node& node)
	{
		return Cmiss_nodeset_group_add_node(
			reinterpret_cast<Cmiss_nodeset_group_id>(id), node.getId());
	}

	int removeAllNodes()
	{
		return Cmiss_nodeset_group_remove_all_nodes(
			reinterpret_cast<Cmiss_nodeset_group_id>(id));
	}

	int removeNode(Node& node)
	{
		return Cmiss_nodeset_group_remove_node(reinterpret_cast<Cmiss_nodeset_group_id>(id),
			node.getId());
	}

	int removeNodesConditional(Field& conditionalField)
	{
		return Cmiss_nodeset_group_remove_nodes_conditional(
			reinterpret_cast<Cmiss_nodeset_group_id>(id), conditionalField.getId());
	}

};

inline int Node::merge(NodeTemplate nodeTemplate)
{
	return Cmiss_node_merge(id, nodeTemplate.getId());
}

}  // namespace Zn

#endif /* __ZN_NODE_HPP__ */
