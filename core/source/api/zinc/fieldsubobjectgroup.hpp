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
public:

	FieldElementGroup() : Field(0)
	{	}

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldElementGroup(cmzn_field_element_group_id field_element_group_id) :
		Field(reinterpret_cast<cmzn_field_id>(field_element_group_id))
	{ }

	MeshGroup getMesh()
	{
		return MeshGroup(cmzn_field_element_group_get_mesh(
			reinterpret_cast<cmzn_field_element_group_id>(id)));
	}
};

class FieldNodeGroup : public Field
{
public:

	FieldNodeGroup() : Field(0)
	{	}

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldNodeGroup(cmzn_field_node_group_id field_node_group_id) :
		Field(reinterpret_cast<cmzn_field_id>(field_node_group_id))
	{ }

	NodesetGroup getNodeset()
	{
		return NodesetGroup(cmzn_field_node_group_get_nodeset(
			reinterpret_cast<cmzn_field_node_group_id>(id)));
	}
};

inline FieldElementGroup Fieldmodule::createFieldElementGroup(Mesh& mesh)
{
	return FieldElementGroup(reinterpret_cast<cmzn_field_element_group_id>(
		cmzn_fieldmodule_create_field_element_group(id, mesh.getId())));
}

inline FieldElementGroup Field::castElementGroup()
{
	return FieldElementGroup(cmzn_field_cast_element_group(id));
}

inline FieldNodeGroup Fieldmodule::createFieldNodeGroup(Nodeset& nodeset)
{
	return FieldNodeGroup(reinterpret_cast<cmzn_field_node_group_id>(
		cmzn_fieldmodule_create_field_node_group(id, nodeset.getId())));
}

inline FieldNodeGroup Field::castNodeGroup()
{
	return FieldNodeGroup(cmzn_field_cast_node_group(id));
}

}  // namespace Zinc
}

#endif
