/***************************************************************************//**
 * FILE : fieldtypesgroup.hpp
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
 * Portions created by the Initial Developer are Copyright (C) 2011
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
#ifndef CMZN_FIELDGROUP_HPP__
#define CMZN_FIELDGROUP_HPP__

#include "zinc/fieldgroup.h"
#include "zinc/field.hpp"
#include "zinc/fieldmodule.hpp"
#include "zinc/fieldtypessubobjectgroup.hpp"
#include "zinc/node.hpp"
#include "zinc/element.hpp"
#include "zinc/region.hpp"

namespace zinc
{

class FieldGroup : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldGroup(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldGroup FieldModule::createGroup();

public:

	FieldGroup() : Field(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldGroup(cmzn_field_group_id field_group_id) :
		Field(reinterpret_cast<cmzn_field_id>(field_group_id))
	{	}

	FieldGroup(Field& field) :
		Field(reinterpret_cast<cmzn_field_id>(cmzn_field_cast_group(field.getId())))
	{	}

	int isEmpty()
	{
		return cmzn_field_group_is_empty(reinterpret_cast<cmzn_field_group_id>(id));
	}

	int isEmptyLocal()
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

	int containsLocalRegion()
	{
		return cmzn_field_group_contains_local_region(
			reinterpret_cast<cmzn_field_group_id>(id));
	}

	int addRegion(Region& region)
	{
		return cmzn_field_group_add_region(reinterpret_cast<cmzn_field_group_id>(id),
			region.getId());
	}

	int removeRegion(Region& region)
	{
		return cmzn_field_group_remove_region(reinterpret_cast<cmzn_field_group_id>(id),
			region.getId());
	}

	int containsRegion(Region& region)
	{
		return cmzn_field_group_contains_region(reinterpret_cast<cmzn_field_group_id>(id),
			region.getId());
	}

	FieldGroup createSubregionGroup(Region& region)
	{
		return FieldGroup(cmzn_field_group_create_subregion_group(
			reinterpret_cast<cmzn_field_group_id>(id), region.getId()));
	}

	FieldGroup getSubregionGroup(Region& region)
	{
		return FieldGroup(cmzn_field_group_get_subregion_group(
			reinterpret_cast<cmzn_field_group_id>(id), region.getId()));
	}

	FieldNodeGroup createNodeGroup(Nodeset& nodeset)
	{
		return FieldNodeGroup(cmzn_field_group_create_node_group(
			reinterpret_cast<cmzn_field_group_id>(id), nodeset.getId()));
	}

	FieldNodeGroup getNodeGroup(Nodeset& nodeset)
	{
		return FieldNodeGroup(cmzn_field_group_get_node_group(
			reinterpret_cast<cmzn_field_group_id>(id), nodeset.getId()));
	}

	FieldElementGroup createElementGroup(Mesh& mesh)
	{
		return FieldElementGroup(cmzn_field_group_create_element_group(
			reinterpret_cast<cmzn_field_group_id>(id), mesh.getId()));
	}

	FieldElementGroup getElementGroup(Mesh& mesh)
	{
		return FieldElementGroup(cmzn_field_group_get_element_group(
			reinterpret_cast<cmzn_field_group_id>(id), mesh.getId()));
	}

	Field getSubobjectGroupforDomain(Field& domainField)
	{
		return Field(cmzn_field_group_get_subobject_group_for_domain(
			reinterpret_cast<cmzn_field_group_id>(id), domainField.getId()));
	}

	FieldGroup getFirstNonEmptyGroup()
	{
		return FieldGroup(cmzn_field_group_get_first_non_empty_group(
			reinterpret_cast<cmzn_field_group_id>(id)));
	}

};

inline FieldGroup FieldModule::createGroup()
{
	return FieldGroup(cmzn_field_module_create_group(id));
}

}  // namespace zinc

#endif
