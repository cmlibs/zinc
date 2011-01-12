/***************************************************************************//**
 * FILE : cmiss_field_sub_group.h
 * 
 * Implements region sub object groups, e.g. node group, element group.
 * These group fields evaluate to 1 (true) at domain locations in the group, and
 * 0 elsewhere.
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
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2010
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

#if !defined (CMISS_FIELD_SUB_GROUP_H)
#define CMISS_FIELD_SUB_GROUP_H

#include "api/cmiss_field.h"
#include "api/cmiss_field_module.h"

struct Cmiss_field_node_group;
typedef struct Cmiss_field_node_group *Cmiss_field_node_group_id;

/*****************************************************************************//**
 * Creates a field where nodes from the same nodeset can be put into group.
 *
 * @param field_module  Region field module which will own new field.
 * @param nodeset  Handle to a set of node
 * @return Newly created field
 */
Cmiss_field_id Cmiss_field_module_create_node_group(Cmiss_field_module_id field_module,
		Cmiss_nodeset_id nodeset);

/*****************************************************************************//**
 * If field can be cast to a Cmiss_field_node_group_id do so
 * and return the field.  Otherwise return NULL.
 * Caller is responsible for destroying the new reference.
 *
 * @param field  handle to the field to cast
 * @return  handle of the cast field, or NULL
*/
Cmiss_field_node_group_id Cmiss_field_cast_node_group(Cmiss_field_id field);

/*****************************************************************************//**
 * Add specified node to node group.
 *
 * @param node_group  handle to target node group field.
 * @param node  handle to target node to be added.
 * @return  1 if successfully add node into node group, otherwise 0.
 */
int Cmiss_field_node_group_add_node(Cmiss_field_node_group_id node_group,
	Cmiss_node_id node);

/*****************************************************************************//**
 * Remove specified node from node group.
 *
 * @param node_group  handle to target node group field.
 * @param node  handle to target node to be remove.
 * @return  1 if successfully remove node into node group, otherwise 0.
 */
int Cmiss_field_node_group_remove_node(Cmiss_field_node_group_id node_group,
	Cmiss_node_id node);

/*****************************************************************************//**
 * Remove all nodes in node group.
 *
 * @param node_group  handle to target node group field.
 * @return  1 if successfully remove all nodes from node group, otherwise 0.
 */
int Cmiss_field_node_group_clear(Cmiss_field_node_group_id node_group);

/*****************************************************************************//**
 * Check if the specified node is in node group.
 *
 * @param node_group  handle to target node group field.
 * @return  1 if node group contains node, otherwise 0.
 */
int Cmiss_field_node_group_contains_node(
	Cmiss_field_node_group_id node_group, Cmiss_node_id node);

/*****************************************************************************//**
 * Check if node group does not contain any node.
 *
 * @param node_group  handle to target node group field.
 * @return  1 if node group does not contain any node, otherwise 0.
 */
int Cmiss_field_node_group_is_empty(Cmiss_field_node_group_id node_group);

/*****************************************************************************//**
 * Cast node group field back to its base field and return the field.  Otherwise
 * return NULL.
 *
 * @param group  handle to the node group field to cast
 * @return  handle of the field, or NULL
 */
static inline Cmiss_field_id Cmiss_field_node_group_base_cast(Cmiss_field_node_group_id group)
{
	return (Cmiss_field_id)(group);
}

/*****************************************************************************//**
 * Destroy the reference to the node group.
 *
 * @param group_address  address to the handle to the node group field
 * @return  1 if successfully destroy the node group, otherwise 0.
 */
int Cmiss_field_node_group_destroy(Cmiss_field_node_group_id *node_group_address);

struct Cmiss_field_element_group;
typedef struct Cmiss_field_element_group *Cmiss_field_element_group_id;

/*****************************************************************************//**
 * Creates a field where elements from the same mesh can be put into group.
 *
 * @param field_module  Region field module which will own new field.
 * @param mesh  Handle to an element mesh
 * @return Newly created field
 */
Cmiss_field_id Cmiss_field_module_create_element_group(Cmiss_field_module_id field_module,
	Cmiss_fe_mesh_id mesh);

/*****************************************************************************//**
 * If field can be cast to a Cmiss_field_element_group_id do so
 * and return the field.  Otherwise return NULL.
 * Caller is responsible for destroying the new reference.
 *
 * @param field  handle to the field to cast
 * @return  handle of the cast field, or NULL
*/
Cmiss_field_element_group_id Cmiss_field_cast_element_group(Cmiss_field_id field);

/*****************************************************************************//**
 * Add specified element to element group.
 *
 * @param element_group  handle to target element group field.
 * @param element  handle to target element to be added.
 * @return  1 if successfully add element into element group, otherwise 0.
 */
int Cmiss_field_element_group_add_element(Cmiss_field_element_group_id element_group,
	Cmiss_element_id element);

/*****************************************************************************//**
 * Remove specified element from element group.
 *
 * @param element_group  handle to target element group field.
 * @param element  handle to target element to be remove.
 * @return  1 if successfully remove element into element group, otherwise 0.
 */
int Cmiss_field_element_group_remove_element(Cmiss_field_element_group_id element_group,
		Cmiss_element_id element);

/*****************************************************************************//**
 * Remove all elements in element group.
 *
 * @param element_group  handle to target element group field.
 * @return  1 if successfully remove all elements from element group, otherwise 0.
 */
int Cmiss_field_element_group_clear(Cmiss_field_element_group_id element_group);

/*****************************************************************************//**
 * Check if the specified element is in element group.
 *
 * @param element_group  handle to target element group field.
 * @return  1 if element group contains element, otherwise 0.
 */
int Cmiss_field_element_group_contains_element(
	Cmiss_field_element_group_id element_group, Cmiss_element_id element);

/*****************************************************************************//**
 * Check if element group does not contain any element.
 *
 * @param element_group  handle to target element group field.
 * @return  1 if element group does not contain any element, otherwise 0.
 */
int Cmiss_field_element_group_is_empty(Cmiss_field_element_group_id element_group);

/*****************************************************************************//**
 * Cast element group field back to its base field and return the field.  Otherwise
 * return NULL.
 *
 * @param group  handle to the element group field to cast
 * @return  handle of the field, or NULL
 */
static inline Cmiss_field_id Cmiss_field_element_group_base_cast(Cmiss_field_element_group_id group)
{
	return (Cmiss_field_id)(group);
}

/*****************************************************************************//**
 * Destroy the reference to the element group.
 *
 * @param element_group_address  address to the handle to the element group field
 * @return  1 if successfully destroy the element group, otherwise 0.
 */
int Cmiss_field_element_group_destroy(Cmiss_field_element_group_id *element_group_address);

#endif /* !defined (CMISS_FIELD_GROUP_H) */
