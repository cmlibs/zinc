/***************************************************************************//**
 * FILE : cmiss_node.h
 *
 * The public interface to Cmiss_node.
 *
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
 * Portions created by the Initial Developer are Copyright (C) 2005-2010
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
#ifndef __CMISS_NODE_H__
#define __CMISS_NODE_H__

#include "types/cmiss_c_inline.h"
#include "types/cmiss_field_id.h"
#include "types/cmiss_field_module_id.h"
#include "types/cmiss_node_id.h"
#include "types/cmiss_time_sequence_id.h"

#include "cmiss_shared_object.h"

/*
Global functions
----------------
*/

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum Cmiss_nodal_value_type Cmiss_nodal_value_type_enum_from_string(
	const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param type  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *Cmiss_nodal_value_type_enum_to_string(enum Cmiss_nodal_value_type type);

/***************************************************************************//**
 * Get a handle to a nodeset from its name in the field module. A nodeset is the
 * container of nodes - i.e. Cmiss_node objects. Valid names may be any
 * node_group field, or the following special names:
 * "cmiss_nodes" = the primary set of nodes for a region, able to be indexed by
 * Cmiss_elements for storing or mapping to finite element field parameters.
 * "cmiss_data" = an additional set of nodes generally used to represent data
 * points, not for finite element field parameters.
 * Note that the default names for node group fields created from a group
 * is GROUP_NAME.NODESET_NAME, with nodeset names as above.
 *
 * @param field_module  The field module the nodeset belongs to.
 * @param name  The name of the nodeset: "cmiss_nodes" or "cmiss_data".
 * @return  Handle to the nodeset, or NULL if error.
 */
ZINC_API Cmiss_nodeset_id Cmiss_field_module_find_nodeset_by_name(
	Cmiss_field_module_id field_module, const char *nodeset_name);

/*******************************************************************************
 * Returns a new handle to the nodeset with reference count incremented.
 * Caller is responsible for destroying the new handle.
 *
 * @param nodeset  The nodeset to obtain a new reference to.
 * @return  New nodeset handle with incremented reference count.
 */
ZINC_API Cmiss_nodeset_id Cmiss_nodeset_access(Cmiss_nodeset_id nodeset);

/***************************************************************************//**
 * Destroys this handle to the nodeset and sets it to NULL.
 * Internally this just decrements the reference count.
 *
 * @param nodeset_address  Address of handle to the nodeset to destroy.
 *  @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_nodeset_destroy(Cmiss_nodeset_id *nodeset_address);

/***************************************************************************//**
 * Returns whether the node is from the nodeset.
 *
 * @param nodeset  The nodeset to query.
 * @param node  The node to query about.
 * @return  1 if node is in the nodeset, 0 if not or error.
 */
ZINC_API int Cmiss_nodeset_contains_node(Cmiss_nodeset_id nodeset, Cmiss_node_id node);

/***************************************************************************//**
 * Create a blank template from which new nodes can be created in this nodeset.
 * Used to describe how finite element fields are defined.
 * Also used for defining new fields on existing nodes.
 *
 * @param nodeset  Handle to the nodeset the template works with.
 * @return  Handle to node_template, or NULL if error.
 */
ZINC_API Cmiss_node_template_id Cmiss_nodeset_create_node_template(
	Cmiss_nodeset_id nodeset);

/***************************************************************************//**
 * Create a new node in this nodeset with fields defined as in the
 * node_template. Returns handle to new element.
 *
 * @param nodeset  Handle to the nodeset to create the new node in.
 * @param identifier  Non-negative integer identifier of new node, or -1
 * to automatically generate, starting from 1. Fails if supplied identifier
 * already used by an existing node.
 * @param node_template  Template for defining node fields.
 * @return  Handle to newly created node, or NULL if error.
 */
ZINC_API Cmiss_node_id Cmiss_nodeset_create_node(Cmiss_nodeset_id nodeset,
	int identifier, Cmiss_node_template_id node_template);

/***************************************************************************//**
 * Create a node iterator object for iterating through the nodes in the nodeset
 * which are ordered from lowest to highest identifier. The iterator initially
 * points at the position before the first node, so the first call to
 * Cmiss_node_iterator_next() returns the first node and advances the iterator.
 * Iterator becomes invalid if nodeset is modified or any of its nodes are
 * given new identifiers.
 *
 * @param nodeset  Handle to the nodeset to iterate over.
 * @return  Handle to node_iterator at position before first, or NULL if error.
 */
ZINC_API Cmiss_node_iterator_id Cmiss_nodeset_create_node_iterator(
	Cmiss_nodeset_id nodeset);

/***************************************************************************//**
 * Destroy all nodes in nodeset, also removing them from any related groups.
 * All handles to the destroyed nodes become invalid.
 *
 * @param nodeset  Handle to nodeset to destroy nodes from.
 * @return  Status CMISS_OK if all nodes destroyed, any other value if failed.
 */
ZINC_API int Cmiss_nodeset_destroy_all_nodes(Cmiss_nodeset_id nodeset);

/***************************************************************************//**
 * Destroy the node if it is in the nodeset. Removes node from any related
 * groups it is in. All handles to the destroyed node become invalid.
 *
 * @param nodeset  Handle to the nodeset whose node is to be destroyed.
 * @param node  The node to destroy.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_nodeset_destroy_node(Cmiss_nodeset_id nodeset, Cmiss_node_id node);

/***************************************************************************//**
 * Destroy all nodes in the nodeset for which the conditional field is true i.e.
 * non-zero valued. These nodes are removed from any related groups they are in.
 * All handles to removed nodes become invalid.
 * Nodes are only removed if not in use by elements in region.
 * Note that group and node_group fields are valid conditional fields.
 *
 * @param nodeset  Handle to the nodeset to destroy nodes from.
 * @param conditional_field  Field which if non-zero at any node indicates it
 * is to be destroyed.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_nodeset_destroy_nodes_conditional(Cmiss_nodeset_id nodeset,
	Cmiss_field_id conditional_field);

/***************************************************************************//**
 * Return a handle to the node in the nodeset with this identifier.
 *
 * @param nodeset  Handle to the nodeset to find the node in.
 * @param identifier  Non-negative integer identifier of node.
 * @return  Handle to the node, or NULL if not found.
 */
ZINC_API Cmiss_node_id Cmiss_nodeset_find_node_by_identifier(Cmiss_nodeset_id nodeset,
	int identifier);

/***************************************************************************//**
 * Get the master nodeset which owns the nodes for this nodeset. Can be the
 * same as the supplied nodeset if it is a master.
 *
 * @param nodeset  The nodeset to query.
 * @return  Handle to the master nodeset. Caller is responsible for destroying
 * the returned handle.
 */
ZINC_API Cmiss_nodeset_id Cmiss_nodeset_get_master(Cmiss_nodeset_id nodeset);

/***************************************************************************//**
 * Return the name of the nodeset.
 *
 * @see Cmiss_deallocate()
 * @param nodeset  The nodeset whose name is requested.
 * @return  On success: allocated string containing nodeset name. Up to caller
 * to free using Cmiss_deallocate().
 */
ZINC_API char *Cmiss_nodeset_get_name(Cmiss_nodeset_id nodeset);

/***************************************************************************//**
 * Return the number of nodes in the nodeset.
 *
 * @param nodeset  Handle to the nodeset to query.
 * @return  Number of nodes in nodeset.
 */
ZINC_API int Cmiss_nodeset_get_size(Cmiss_nodeset_id nodeset);

/***************************************************************************//**
 * Check if two nodeset handles refer to the same object.
 *
 * @param nodeset1  The first nodeset to match.
 * @param nodeset2  The second nodeset to match.
 * @return  1 if the two nodesets match, 0 if not.
 */
ZINC_API int Cmiss_nodeset_match(Cmiss_nodeset_id nodeset1, Cmiss_nodeset_id nodeset2);

/***************************************************************************//**
 * If the nodeset is a nodeset group i.e. subset of nodes from a master nodeset,
 * get the nodeset group specific interface for add/remove functions.
 * Caller is responsible for destroying the returned reference.
 *
 * @param field  The nodeset to be cast.
 * @return  Nodeset group specific representation if the input nodeset is of
 * this type, otherwise returns NULL.
 */
ZINC_API Cmiss_nodeset_group_id Cmiss_nodeset_cast_group(Cmiss_nodeset_id nodeset);

/***************************************************************************//**
 * Destroys this handle to the nodeset group and sets it to NULL.
 * Internally this just decrements the reference count.
 *
 * @param nodeset_group_address  Address of nodeset group handle to destroy.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_nodeset_group_destroy(Cmiss_nodeset_group_id *nodeset_group_address);

/***************************************************************************//**
 * Cast nodeset group back to its base nodeset class.
 * IMPORTANT NOTE: Returned nodeset does not have incremented reference count
 * and must not be destroyed. Use Cmiss_nodeset_access() to add a reference if
 * maintaining returned handle beyond the lifetime of the nodeset_group.
 * Use this function to call base-class API, e.g.:
 * int size = Cmiss_nodeset_get_size(Cmiss_nodeset_group_base_cast(nodeset_group);
 *
 * @param nodeset_group  Handle to the nodeset group to cast.
 * @return  Non-accessed handle to the nodeset or NULL if failed.
 */
CMISS_C_INLINE Cmiss_nodeset_id Cmiss_nodeset_group_base_cast(
	Cmiss_nodeset_group_id nodeset_group)
{
	return (Cmiss_nodeset_id)(nodeset_group);
}

/***************************************************************************//**
 * Add specified node to nodeset group.
 *
 * @param nodeset_group  Handle to nodeset group to modify.
 * @param node  Handle to node to add. Must be from the group's master nodeset.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_nodeset_group_add_node(Cmiss_nodeset_group_id nodeset_group,
	Cmiss_node_id node);

/***************************************************************************//**
 * Remove all nodes from nodeset group.
 *
 * @param nodeset_group  Handle to nodeset group to modify.
 * @return  Status CMISS_OK if all nodes removed, any other value if failed.
 */
ZINC_API int Cmiss_nodeset_group_remove_all_nodes(Cmiss_nodeset_group_id nodeset_group);

/***************************************************************************//**
 * Remove specified node from nodeset group.
 *
 * @param nodeset_group  Handle to nodeset group to modify.
 * @param node  Handle to node to remove.
 * @return  Status CMISS_OK if node is removed, any other value if failed.
 */
ZINC_API int Cmiss_nodeset_group_remove_node(Cmiss_nodeset_group_id nodeset_group,
	Cmiss_node_id node);

/***************************************************************************//**
 * Remove all nodes from the nodeset group for which the conditional field is
 * true i.e. non-zero valued in the node.
 * Note that group and node_group fields are valid conditional fields.
 *
 * @param nodeset_group  Handle to the nodeset group to remove nodes from.
 * @param conditional_field  Field which if non-zero in the node indicates it
 * is to be removed.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_nodeset_group_remove_nodes_conditional(
	Cmiss_nodeset_group_id nodeset_group, Cmiss_field_id conditional_field);

/*******************************************************************************
 * Returns a new handle to the node iterator with reference count incremented.
 * Caller is responsible for destroying the new handle.
 *
 * @param mesh  The node iterator to obtain a new reference to.
 * @return  New node iterator handle with incremented reference count.
 */
ZINC_API Cmiss_node_iterator_id Cmiss_node_iterator_access(
	Cmiss_node_iterator_id node_iterator);

/***************************************************************************//**
 * Destroys this handle to the node_iterator and sets it to NULL.
 *
 * @param node_iterator_address  Address of handle to node_iterator to destroy.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_node_iterator_destroy(Cmiss_node_iterator_id *node_iterator_address);

/***************************************************************************//**
 * Returns a handle to the next node in the container being iterated over then
 * advances the iterator position. The caller is required to destroy the
 * returned node handle.
 *
 * @param node_iterator  Node iterator to query and advance.
 * @return  Handle to the next node, or NULL if none remaining.
 */
ZINC_API Cmiss_node_id Cmiss_node_iterator_next(Cmiss_node_iterator_id node_iterator);

/*******************************************************************************
 * Returns a new handle to the node template with reference count incremented.
 * Caller is responsible for destroying the new handle.
 *
 * @param mesh  The node template to obtain a new reference to.
 * @return  New node template handle with incremented reference count.
 */
ZINC_API Cmiss_node_template_id Cmiss_node_template_access(
	Cmiss_node_template_id node_template);

/***************************************************************************//**
 * Destroys this handle to the node_template and sets it to NULL.
 * Internally this just decrements the reference count.
 *
 * @param node_template_address  Address of handle to node_template
 * to destroy.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_node_template_destroy(Cmiss_node_template_id *node_template_address);

/***************************************************************************//**
 * Defines the field on the node_template with just a single node value per
 * field component with no time variation.
 * Per-component derivatives and multiple versions can be added subsequently.
 *
 * @param node_template  Node template to modify.
 * @param field  The field to define. May be finite_element, stored_string or
 * stored_mesh_location type only.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_node_template_define_field(Cmiss_node_template_id node_template,
	Cmiss_field_id field);

/***************************************************************************//**
 * Defines the field on the node_template based on its definition in the
 * supplied node.
 *
 * @param node_template  Node template to modify.
 * @param field  The field to define. May be finite_element, stored_string or
 * stored_mesh_location type only.
 * @param node  The node to obtain the field definition from.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_node_template_define_field_from_node(
	Cmiss_node_template_id node_template, Cmiss_field_id field,
	Cmiss_node_id node);

/***************************************************************************//**
 * Adds storage for the supplied derivative type for the component/s of the
 * field in the node template.
 * Must have first called Cmiss_node_template_define_field for field.
 *
 * @param node_template  Node template to modify.
 * @param field  The field to define derivatives for. May be finite_element
 * type only.
 * @param component_number  The component from 1 to the number of field
 * components, or -1 to define the derivative for all components.
 * @param derivative_type  The type of nodal derivative to define.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_node_template_define_derivative(Cmiss_node_template_id node_template,
	Cmiss_field_id field, int component_number,
	enum Cmiss_nodal_value_type derivative_type);

/***************************************************************************//**
 * Defines variation of all nodal values/derivatives * versions with the
 * supplied time sequence for all components of the field in the node template.
 * Hence there will be as many of each parameter as times in the time sequence.
 *
 * @param node_template  Node template to modify.
 * @param field  The field to define versions for. May be finite_element type
 * only.
 * @param time_sequence  Time sequence object defining the number of times for
 * which field parameters are stored, and the times they are for (increasing).
 * @param node_field_creator  Optionally defines different versions and/or
 * derivative types. If it is NULL then a single nodal value for each component
 * will be defined.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_node_template_define_time_sequence(
	Cmiss_node_template_id node_template, Cmiss_field_id field,
	struct Cmiss_time_sequence *time_sequence);

/***************************************************************************//**
 * Adds storage for multiple versions of nodal values and derivatives for the
 * component/s of the field in the node template.
 * Note: currently limited to having the same number of versions for all values
 * and derivatives in a given component.
 * Must have first called Cmiss_node_template_define_field for field.
 *
 * @param node_template  Node template to modify.
 * @param field  The field to define versions for. May be finite_element type
 * only.
 * @param component_number  The component from 1 to the number of field
 * components, or -1 to define the number of versions for all components.
 * @param number_of_versions  The number of versions of each value & derivative
 * stored for the component/s, at least 1 (the default).
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_node_template_define_versions(Cmiss_node_template_id node_template,
	Cmiss_field_id field, int component_number, int number_of_versions);

/***************************************************************************//**
 * Returns the number of versions defined for a given component of the field in
 * the node template.
 *
 * @param node_template  Node template to query.
 * @param field  The field to get number of versions for. May be finite_element
 * type only.
 * @param component_number  The component from 1 to the number of field
 * components, or -1 to get maximum number of versions in any component.
 * @return  Number of versions for component of field, or maximum in any
 * component if component_number is -1). Returns 0 if field not defined or
 * invalid arguments are supplied.
 */
ZINC_API int Cmiss_node_template_get_number_of_versions(Cmiss_node_template_id node_template,
	Cmiss_field_id field, int component_number);

/***************************************************************************//**
 * Returns the time sequence defined for field in node_template, if any.
 *
 * @param node_template  Node template to query.
 * @param field  The field to get time sequence for. May be finite_element
 * type only.
 * @return  Handle to time sequence object if defined for field, or NULL if none
 * or error. Up to caller to destroy returned handle.
 */
ZINC_API Cmiss_time_sequence_id Cmiss_node_template_get_time_sequence(
	Cmiss_node_template_id node_template, Cmiss_field_id field);

/***************************************************************************//**
 * Returns whether a nodal derivative is defined for a component of the field
 * in the node template.
 *
 * @param node_template  Node template to query.
 * @param field  The field to check derivatives for. May be finite_element
 * type only.
 * @param component_number  The component from 1 to the number of field
 * components, or -1 to check if *any* component has the nodal derivative.
 * @param derivative_type  The type of nodal derivative to check.
 * @return  1 if derivative_type is defined for component_number of field (or
 * for any component if component_number is -1), 0 if not.
 */
ZINC_API int Cmiss_node_template_has_derivative(Cmiss_node_template_id node_template,
	Cmiss_field_id field, int component_number,
	enum Cmiss_nodal_value_type derivative_type);

/***************************************************************************//**
 * Sets field to be undefined when next merged into an existing node. Has no
 * effect on newly created nodes. It is illegal to define and undefine the same
 * field in a node template.
 *
 * @param node_template  Node template to modify.
 * @param field  The field to undefine. May be finite_element, stored_string or
 * stored_mesh_location type only.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_node_template_undefine_field(Cmiss_node_template_id node_template,
	Cmiss_field_id field);

/*******************************************************************************
 * Returns a new handle to the node with reference count incremented.
 * Caller is responsible for destroying the new handle.
 *
 * @param node  The node to obtain a new reference to.
 * @return  New node handle with incremented reference count.
 */
ZINC_API Cmiss_node_id Cmiss_node_access(Cmiss_node_id node);

/***************************************************************************//**
 * Destroys this handle to the node and sets it to NULL.
 * Internally this just decrements the reference count.
 *
 * @param node_address  Address of handle to the node to destroy.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_node_destroy(Cmiss_node_id *node_address);

/***************************************************************************//**
 * Returns the non-negative integer uniquely identifying the node in its
 * nodeset.
 *
 * @param node  The node to query.
 * @return  The non-negative integer identifier of the node, or a negative
 * value if node is invalid.
 */
ZINC_API int Cmiss_node_get_identifier(Cmiss_node_id node);

/***************************************************************************//**
 * Modifies the node to define fields as described in the node_template.
 *
 * @param node  The node to modify.
 * @param node_template  Template containing node field descriptions.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_node_merge(Cmiss_node_id node, Cmiss_node_template_id node_template);

#ifdef __cplusplus
}
#endif

#endif /* __CMISS_NODE_H__ */
