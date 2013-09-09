/***************************************************************************//**
 * FILE : fieldsubobjectgroup.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDSUBOBJECTGROUP_HPP__
#define CMZN_FIELDSUBOBJECTGROUP_HPP__

#include "zinc/fieldsubobjectgroup.h"
#include "zinc/field.hpp"
#include "zinc/fieldmodule.hpp"
#include "zinc/node.hpp"
#include "zinc/element.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class FieldElementGroup : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldElementGroup(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldElementGroup FieldModule::createElementGroup(Mesh& mesh);

public:

	FieldElementGroup() : Field(0)
	{	}

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldElementGroup(cmzn_field_element_group_id field_element_group_id) :
		Field(reinterpret_cast<cmzn_field_id>(field_element_group_id))
	{ }

	FieldElementGroup(Field& field) :
		Field(reinterpret_cast<cmzn_field_id>(cmzn_field_cast_element_group(field.getId())))
	{	}

	MeshGroup getMesh()
	{
		return MeshGroup(cmzn_field_element_group_get_mesh(
			reinterpret_cast<cmzn_field_element_group_id>(id)));
	}
};

class FieldNodeGroup : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldNodeGroup(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldNodeGroup FieldModule::createNodeGroup(Nodeset& nodeset);

public:

	FieldNodeGroup() : Field(0)
	{	}

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldNodeGroup(cmzn_field_node_group_id field_node_group_id) :
		Field(reinterpret_cast<cmzn_field_id>(field_node_group_id))
	{ }

	FieldNodeGroup(Field& field) :
		Field(reinterpret_cast<cmzn_field_id>(cmzn_field_cast_node_group(field.getId())))
	{	}

	NodesetGroup getNodeset()
	{
		return NodesetGroup(cmzn_field_node_group_get_nodeset(
			reinterpret_cast<cmzn_field_node_group_id>(id)));
	}
};

inline FieldElementGroup FieldModule::createElementGroup(Mesh& mesh)
{
	return FieldElementGroup(cmzn_field_module_create_element_group(id, mesh.getId()));
}

inline FieldNodeGroup FieldModule::createNodeGroup(Nodeset& nodeset)
{
	return FieldNodeGroup(cmzn_field_module_create_node_group(id, nodeset.getId()));
}

}  // namespace Zinc
}

#endif
