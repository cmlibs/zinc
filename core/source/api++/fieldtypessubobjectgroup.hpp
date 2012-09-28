/***************************************************************************//**
 * FILE : fieldtypessubobjectgroup.hpp
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
#ifndef __ZN_FIELD_TYPES_SUBOBJECT_GROUP_HPP__
#define __ZN_FIELD_TYPES_SUBOBJECT_GROUP_HPP__

#include "api/cmiss_field_subobject_group.h"
#include "api++/field.hpp"
#include "api++/fieldmodule.hpp"
#include "api++/node.hpp"
#include "api++/element.hpp"

namespace Zn
{

class FieldElementGroup : public Field
{
public:

	FieldElementGroup() : Field(0)
	{	}

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldElementGroup(Cmiss_field_id field_id) : Field(field_id)
	{	}

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldElementGroup(Cmiss_field_element_group_id field_element_group_id) :
		Field(reinterpret_cast<Cmiss_field_id>(field_element_group_id))
	{ }

	FieldElementGroup(Field& field) :
		Field(reinterpret_cast<Cmiss_field_id>(Cmiss_field_cast_element_group(field.getId())))
	{	}

	MeshGroup getMesh()
	{
		return MeshGroup(Cmiss_field_element_group_get_mesh(
			reinterpret_cast<Cmiss_field_element_group_id>(id)));
	}
};

class FieldNodeGroup : public Field
{
public:

	FieldNodeGroup() : Field(0)
	{	}

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldNodeGroup(Cmiss_field_id field_id) : Field(field_id)
	{	}

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldNodeGroup(Cmiss_field_node_group_id field_node_group_id) :
		Field(reinterpret_cast<Cmiss_field_id>(field_node_group_id))
	{ }

	FieldNodeGroup(Field& field) :
		Field(reinterpret_cast<Cmiss_field_id>(Cmiss_field_cast_node_group(field.getId())))
	{	}

	NodesetGroup getNodeset()
	{
		return NodesetGroup(Cmiss_field_node_group_get_nodeset(
			reinterpret_cast<Cmiss_field_node_group_id>(id)));
	}
};

inline FieldElementGroup FieldModule::createElementGroup(Mesh& mesh)
{
	return FieldElementGroup(Cmiss_field_module_create_element_group(id, mesh.getId()));
}

inline FieldNodeGroup FieldModule::createNodeGroup(Nodeset& nodeset)
{
	return FieldNodeGroup(Cmiss_field_module_create_node_group(id, nodeset.getId()));
}


}  // namespace Zn

#endif /* __ZN_FIELD_TYPES_SUBOBJECT_GROUP_HPP__ */
