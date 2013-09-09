/***************************************************************************//**
 * FILE : fieldsubobjectgroup.h
 *
 * Implements region sub object groups, e.g. node group, element group.
 * These group fields evaluate to 1 (true) at domain locations in the group, and
 * 0 elsewhere.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_FIELDSUBOBJECTGROUP_H__
#define CMZN_FIELDSUBOBJECTGROUP_H__

#include "types/fieldid.h"
#include "types/fieldmoduleid.h"
#include "types/elementid.h"
#include "types/nodeid.h"
#include "types/fieldsubobjectgroupid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * Creates a node group field which packages a cmzn_nodeset_group i.e. a subset
 * of nodes from a master nodeset. As a field it evaluates to 1 for nodes in
 * the nodeset group and 0 elsewhere, i.e. it is the predicate for the sub-
 * domain, and this Boolean value can be combined in logical operations with
 * other fields.
 *
 * @param field_module  Region field module which will own new field.
 * @param mesh  Handle to a nodeset the node group is to be compatible with. If
 * it is not a master nodeset, the master is obtained from it.
 * Nodeset must be from the same region as field_module.
 * @return  Newly created field, or NULL if failed.
 */
ZINC_API cmzn_field_id cmzn_field_module_create_node_group(
	cmzn_field_module_id field_module, cmzn_nodeset_id nodeset);

/***************************************************************************//**
 * If field can be cast to a cmzn_field_node_group_id do so
 * and return the field.  Otherwise return NULL.
 * Caller is responsible for destroying the new reference.
 *
 * @param field  handle to the field to cast
 * @return  handle of the cast field, or NULL
*/
ZINC_API cmzn_field_node_group_id cmzn_field_cast_node_group(cmzn_field_id field);

/***************************************************************************//**
 * Cast node group field back to its base field and return the field.  Otherwise
 * return NULL.
 *
 * @param group  handle to the node group field to cast
 * @return  handle of the field, or NULL
 */
ZINC_C_INLINE cmzn_field_id cmzn_field_node_group_base_cast(cmzn_field_node_group_id group)
{
	return (cmzn_field_id)(group);
}

/***************************************************************************//**
 * Destroy the reference to the node group.
 *
 * @param group_address  address to the handle to the node group field
 * @return  Status CMZN_OK if successfully destroy the node group,
 * any other value on failure.
 */
ZINC_API int cmzn_field_node_group_destroy(cmzn_field_node_group_id *node_group_address);

/***************************************************************************//**
 * Get a handle to the 'dual' nodeset group of this node group, which provides
 * most of the methods for working with it.
 *
 * @param node_group  Handle to node group field.
 * @return  Handle to nodeset group. Caller is responsible for destroying this.
 */
ZINC_API cmzn_nodeset_group_id cmzn_field_node_group_get_nodeset(
	cmzn_field_node_group_id node_group);

/***************************************************************************//**
 * Creates an element group field which packages a cmzn_mesh_group i.e. a
 * subset of elements from a master mesh. As a field it evaluates to 1 for
 * elements in the mesh group and 0 elsewhere, i.e. it is the predicate for the
 * sub-domain, and this Boolean value can be combined in logical operations with
 * other fields.
 *
 * @param field_module  Region field module which will own new field.
 * @param mesh  Handle to a finite element mesh the element group is to be
 * compatible with. If it is not a master mesh, the master is obtained from it.
 * Mesh must be from the same region as field_module.
 * @return  Newly created field, or NULL if failed.
 */
ZINC_API cmzn_field_id cmzn_field_module_create_element_group(
	cmzn_field_module_id field_module, cmzn_mesh_id mesh);

/***************************************************************************//**
 * If field can be cast to a cmzn_field_element_group_id do so
 * and return the field.  Otherwise return NULL.
 * Caller is responsible for destroying the new reference.
 *
 * @param field  handle to the field to cast
 * @return  handle of the cast field, or NULL
*/
ZINC_API cmzn_field_element_group_id cmzn_field_cast_element_group(cmzn_field_id field);

/***************************************************************************//**
 * Cast element group field back to its base field and return the field.  Otherwise
 * return NULL.
 *
 * @param group  handle to the element group field to cast
 * @return  handle of the field, or NULL
 */
ZINC_C_INLINE cmzn_field_id cmzn_field_element_group_base_cast(cmzn_field_element_group_id group)
{
	return (cmzn_field_id)(group);
}

/***************************************************************************//**
 * Destroy the reference to the element group.
 *
 * @param element_group_address  address to the handle to the element group field
 * @return  Status CMZN_OK if successfully destroy the element group,
 * any other value on failure.
 */
ZINC_API int cmzn_field_element_group_destroy(cmzn_field_element_group_id *element_group_address);

/***************************************************************************//**
 * Get a handle to the 'dual' mesh group of this element group, which provides
 * most of the methods for working with it.
 *
 * @param element_group  Handle to element group field.
 * @return  Handle to mesh group. Caller is responsible for destroying this.
 */
ZINC_API cmzn_mesh_group_id cmzn_field_element_group_get_mesh(
	cmzn_field_element_group_id element_group);

#ifdef __cplusplus
}
#endif

#endif
