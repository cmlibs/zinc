/**
 * @file fieldgroup.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDGROUP_HPP__
#define CMZN_FIELDGROUP_HPP__

#include "zinc/fieldgroup.h"
#include "zinc/field.hpp"
#include "zinc/fieldmodule.hpp"
#include "zinc/fieldsubobjectgroup.hpp"
#include "zinc/node.hpp"
#include "zinc/element.hpp"
#include "zinc/region.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class FieldGroup : public Field
{
public:

	FieldGroup() : Field(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldGroup(cmzn_field_group_id field_group_id) :
		Field(reinterpret_cast<cmzn_field_id>(field_group_id))
	{	}

	bool isEmpty()
	{
		return cmzn_field_group_is_empty(reinterpret_cast<cmzn_field_group_id>(id));
	}

	bool isEmptyLocal()
	{
		return cmzn_field_group_is_empty_local(reinterpret_cast<cmzn_field_group_id>(id));
	}

	int clear()
	{
		return cmzn_field_group_clear(reinterpret_cast<cmzn_field_group_id>(id));
	}

	int clearLocal()
	{
		return cmzn_field_group_clear_local(reinterpret_cast<cmzn_field_group_id>(id));
	}

	int removeEmptySubgroups()
	{
		return cmzn_field_group_remove_empty_subgroups(
			reinterpret_cast<cmzn_field_group_id>(id));
	}

	int addLocalRegion()
	{
		return cmzn_field_group_add_local_region(
			reinterpret_cast<cmzn_field_group_id>(id));
	}

	bool containsLocalRegion()
	{
		return cmzn_field_group_contains_local_region(
			reinterpret_cast<cmzn_field_group_id>(id));
	}

	int removeLocalRegion()
	{
		return cmzn_field_group_remove_local_region(
			reinterpret_cast<cmzn_field_group_id>(id));
	}

	int addRegion(const Region& region)
	{
		return cmzn_field_group_add_region(reinterpret_cast<cmzn_field_group_id>(id),
			region.getId());
	}

	int removeRegion(const Region& region)
	{
		return cmzn_field_group_remove_region(reinterpret_cast<cmzn_field_group_id>(id),
			region.getId());
	}

	bool containsRegion(const Region& region)
	{
		return cmzn_field_group_contains_region(reinterpret_cast<cmzn_field_group_id>(id),
			region.getId());
	}

	FieldGroup createSubregionFieldGroup(const Region& region)
	{
		return FieldGroup(cmzn_field_group_create_subregion_field_group(
			reinterpret_cast<cmzn_field_group_id>(id), region.getId()));
	}

	FieldGroup getSubregionFieldGroup(const Region& region)
	{
		return FieldGroup(cmzn_field_group_get_subregion_field_group(
			reinterpret_cast<cmzn_field_group_id>(id), region.getId()));
	}

	FieldNodeGroup createFieldNodeGroup(const Nodeset& nodeset)
	{
		return FieldNodeGroup(cmzn_field_group_create_field_node_group(
			reinterpret_cast<cmzn_field_group_id>(id), nodeset.getId()));
	}

	FieldNodeGroup getFieldNodeGroup(const Nodeset& nodeset)
	{
		return FieldNodeGroup(cmzn_field_group_get_field_node_group(
			reinterpret_cast<cmzn_field_group_id>(id), nodeset.getId()));
	}

	FieldElementGroup createFieldElementGroup(const Mesh& mesh)
	{
		return FieldElementGroup(cmzn_field_group_create_field_element_group(
			reinterpret_cast<cmzn_field_group_id>(id), mesh.getId()));
	}

	FieldElementGroup getFieldElementGroup(const Mesh& mesh)
	{
		return FieldElementGroup(cmzn_field_group_get_field_element_group(
			reinterpret_cast<cmzn_field_group_id>(id), mesh.getId()));
	}

	Field getSubobjectGroupFieldForDomainField(const Field& domainField)
	{
		return Field(cmzn_field_group_get_subobject_group_field_for_domain_field(
			reinterpret_cast<cmzn_field_group_id>(id), domainField.getId()));
	}

	FieldGroup getFirstNonEmptySubregionFieldGroup()
	{
		return FieldGroup(cmzn_field_group_get_first_non_empty_subregion_field_group(
			reinterpret_cast<cmzn_field_group_id>(id)));
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

}  // namespace Zinc
}

#endif
