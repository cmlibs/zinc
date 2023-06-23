/**
 * @file fieldgroup.hpp
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDGROUP_HPP__
#define CMZN_FIELDGROUP_HPP__

#include "cmlibs/zinc/fieldgroup.h"
#include "cmlibs/zinc/element.hpp"
#include "cmlibs/zinc/field.hpp"
#include "cmlibs/zinc/fieldmodule.hpp"
#include "cmlibs/zinc/mesh.hpp"
#include "cmlibs/zinc/node.hpp"
#include "cmlibs/zinc/nodeset.hpp"
#include "cmlibs/zinc/region.hpp"

namespace CMLibs
{
namespace Zinc
{

class FieldGroup : public Field
{
	inline cmzn_field_group_id getDerivedId() const
	{
		return reinterpret_cast<cmzn_field_group_id>(id);
	}

public:

	FieldGroup() : Field(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldGroup(cmzn_field_group_id field_group_id) :
		Field(reinterpret_cast<cmzn_field_id>(field_group_id))
	{	}

	enum SubelementHandlingMode
	{
		SUBELEMENT_HANDLING_MODE_INVALID = CMZN_FIELD_GROUP_SUBELEMENT_HANDLING_MODE_INVALID,
		SUBELEMENT_HANDLING_MODE_NONE = CMZN_FIELD_GROUP_SUBELEMENT_HANDLING_MODE_NONE,
		SUBELEMENT_HANDLING_MODE_FULL = CMZN_FIELD_GROUP_SUBELEMENT_HANDLING_MODE_FULL
	};

	bool isEmpty() const
	{
		return cmzn_field_group_is_empty(getDerivedId());
	}

	bool isEmptyLocal() const
	{
		return cmzn_field_group_is_empty_local(getDerivedId());
	}

	int clear()
	{
		return cmzn_field_group_clear(getDerivedId());
	}

	int clearLocal()
	{
		return cmzn_field_group_clear_local(getDerivedId());
	}

	int removeEmptySubgroups()
	{
		return cmzn_field_group_remove_empty_subgroups(getDerivedId());
	}

	int addLocalRegion()
	{
		return cmzn_field_group_add_local_region(getDerivedId());
	}

	bool containsLocalRegion() const
	{
		return cmzn_field_group_contains_local_region(getDerivedId());
	}

	int removeLocalRegion()
	{
		return cmzn_field_group_remove_local_region(getDerivedId());
	}

	int addRegion(const Region& region)
	{
		return cmzn_field_group_add_region(getDerivedId(),
			region.getId());
	}

	int removeRegion(const Region& region)
	{
		return cmzn_field_group_remove_region(getDerivedId(), region.getId());
	}

	bool containsRegion(const Region& region) const
	{
		return cmzn_field_group_contains_region(getDerivedId(), region.getId());
	}

	SubelementHandlingMode getSubelementHandlingMode() const
	{
		return static_cast<SubelementHandlingMode>(
			cmzn_field_group_get_subelement_handling_mode(getDerivedId()));
	}

	int setSubelementHandlingMode(SubelementHandlingMode mode)
	{
		return cmzn_field_group_set_subelement_handling_mode(getDerivedId(),
			static_cast<cmzn_field_group_subelement_handling_mode>(mode));
	}

	FieldGroup createSubregionFieldGroup(const Region& region)
	{
		return FieldGroup(cmzn_field_group_create_subregion_field_group(
			getDerivedId(), region.getId()));
	}

	FieldGroup getSubregionFieldGroup(const Region& region) const
	{
		return FieldGroup(cmzn_field_group_get_subregion_field_group(
			getDerivedId(), region.getId()));
	}

	FieldGroup getOrCreateSubregionFieldGroup(const Region& region)
	{
		return FieldGroup(cmzn_field_group_get_or_create_subregion_field_group(
			getDerivedId(), region.getId()));
	}

	NodesetGroup createNodesetGroup(const Nodeset& nodeset)
	{
		return NodesetGroup(cmzn_field_group_create_nodeset_group(
			getDerivedId(), nodeset.getId()));
	}

	NodesetGroup getNodesetGroup(const Nodeset& nodeset) const
	{
		return NodesetGroup(cmzn_field_group_get_nodeset_group(
			getDerivedId(), nodeset.getId()));
	}

	NodesetGroup getOrCreateNodesetGroup(const Nodeset& nodeset) const
	{
		return NodesetGroup(cmzn_field_group_get_or_create_nodeset_group(
			getDerivedId(), nodeset.getId()));
	}

	MeshGroup createMeshGroup(const Mesh& mesh)
	{
		return MeshGroup(cmzn_field_group_create_mesh_group(
			getDerivedId(), mesh.getId()));
	}

	MeshGroup getMeshGroup(const Mesh& mesh) const
	{
		return MeshGroup(cmzn_field_group_get_mesh_group(
			getDerivedId(), mesh.getId()));
	}

	MeshGroup getOrCreateMeshGroup(const Mesh& mesh) const
	{
		return MeshGroup(cmzn_field_group_get_or_create_mesh_group(
			getDerivedId(), mesh.getId()));
	}

	FieldGroup getFirstNonEmptySubregionFieldGroup() const
	{
		return FieldGroup(cmzn_field_group_get_first_non_empty_subregion_field_group(
			getDerivedId()));
	}

};

inline FieldGroup Fieldmodule::createFieldGroup()
{
	return FieldGroup(reinterpret_cast<cmzn_field_group_id>(
		cmzn_fieldmodule_create_field_group(id)));
}

inline FieldGroup Field::castGroup()
{
	return FieldGroup(cmzn_field_cast_group(id));
}

inline FieldGroup MeshGroup::getFieldGroup() const
{
	return FieldGroup(cmzn_mesh_group_get_field_group(this->getDerivedId()));
}

inline FieldGroup NodesetGroup::getFieldGroup() const
{
	return FieldGroup(cmzn_nodeset_group_get_field_group(this->getDerivedId()));
}

}  // namespace Zinc
}

#endif
