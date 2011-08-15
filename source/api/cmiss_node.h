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

#include "api/types/cmiss_field_id.h"
#include "api/types/cmiss_field_module_id.h"
#include "api/types/cmiss_node_id.h"
#include "api/types/cmiss_time_sequence_id.h"

/***************************************************************************//**
 * The type of a nodal parameter value.
 */
enum Cmiss_nodal_value_type
{
	CMISS_NODAL_VALUE_TYPE_INVALID = 0,
	CMISS_NODAL_VALUE = 1,         /* literal field value */
	CMISS_NODAL_D_DS1 = 2,         /* derivative w.r.t. arc length S1 */
	CMISS_NODAL_D_DS2 = 3,         /* derivative w.r.t. arc length S2 */
	CMISS_NODAL_D_DS3 = 4,         /* derivative w.r.t. arc length S3 */
	CMISS_NODAL_D2_DS1DS2 = 5,     /* cross derivative w.r.t. arc lengths S1,S2 */
	CMISS_NODAL_D2_DS1DS3 = 6,     /* cross derivative w.r.t. arc lengths S1,S3 */
	CMISS_NODAL_D2_DS2DS3 = 7,     /* cross derivative w.r.t. arc lengths S2,S3 */
	CMISS_NODAL_D3_DS1DS2DS3 = 8   /* triple cross derivative w.r.t. arc lengths S1,S2,S3 */
};

/*
Global functions
----------------
*/

/***************************************************************************//**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
enum Cmiss_nodal_value_type Cmiss_nodal_value_type_enum_from_string(
	const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param type  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
char *Cmiss_nodal_value_type_enum_to_string(enum Cmiss_nodal_value_type type);

/***************************************************************************//**
 * Get a handle to a nodeset from its name in the field module. A nodeset is the
 * container of nodes - i.e. Cmiss_node objects.
 * Valid names are currently limited to:
 * "cmiss_nodes" = the primary set of nodes for a region, able to be indexed by
 * Cmiss_elements for storing or mapping to finite element field parameters.
 * "cmiss_data" = an additional set of nodes generally used to represent data
 * points, not for finite element field parameters.
 *
 * @param field_module  The field module the nodeset belongs to.
 * @param name  The name of the nodeset: "cmiss_nodes" or "cmiss_data".
 * @return  Handle to the nodeset, or NULL if error.
 */
Cmiss_nodeset_id Cmiss_field_module_get_nodeset_by_name(
	Cmiss_field_module_id field_module, const char *nodeset_name);

/*******************************************************************************
 * Returns a new handle to the nodeset with reference count incremented.
 * Caller is responsible for destroying the new handle.
 *
 * @param nodeset  The nodeset to obtain a new reference to.
 * @return  New nodeset handle with incremented reference count.
 */
Cmiss_nodeset_id Cmiss_nodeset_access(Cmiss_nodeset_id nodeset);

/***************************************************************************//**
 * Destroys this handle to the nodeset and sets it to NULL.
 * Internally this just decrements the reference count.
 *
 * @param nodeset_address  Address of handle to the nodeset to destroy.
 */
int Cmiss_nodeset_destroy(Cmiss_nodeset_id *nodeset_address);

/***************************************************************************//**
 * Returns whether the node is from the nodeset.
 *
 * @param nodeset  The nodeset to query.
 * @param node  The node to query about.
 * @return  1 if node is in the nodeset, 0 if not or error.
 */
int Cmiss_nodeset_contains_node(Cmiss_nodeset_id nodeset, Cmiss_node_id node);

/***************************************************************************//**
 * Create a blank template from which new nodes can be created in this mesh.
 * Used to describe how finite element fields are defined.
 * Also used for defining new fields on existing nodes.
 *
 * @param nodeset  Handle to the nodeset the template works with.
 * @return  Handle to node_template, or NULL if error.
 */
Cmiss_node_template_id Cmiss_nodeset_create_node_template(
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
Cmiss_node_id Cmiss_nodeset_create_node(Cmiss_nodeset_id nodeset,
	int identifier, Cmiss_node_template_id node_template);

/***************************************************************************//**
 * Create a node iterator object for iterating through the nodes in the nodeset
 * which are ordered from lowest to highest identifier. The iterator initially
 * points at the position before the first node, so the first call to
 * Cmiss_node_iterator_next() returns the first node and advances the iterator.
 *
 * @param nodeset  Handle to the nodeset to iterate over.
 * @return  Handle to node_iterator at position before first, or NULL if error.
 */
Cmiss_node_iterator_id Cmiss_nodeset_create_node_iterator(
	Cmiss_nodeset_id nodeset);

/***************************************************************************//**
 * Return a handle to the node in the nodeset with this identifier.
 *
 * @param nodeset  Handle to the nodeset to find the node in.
 * @param identifier  Non-negative integer identifier of node.
 * @return  Handle to the node, or NULL if not found.
 */
Cmiss_node_id Cmiss_nodeset_find_node_by_identifier(Cmiss_nodeset_id nodeset,
	int identifier);

/***************************************************************************//**
 * Get the master nodeset which owns the nodes for this nodeset. Can be the
 * same as the supplied nodeset if it is a master.
 *
 * @param nodeset  The nodeset to query.
 * @return  Handle to the master nodeset. Caller is responsible for destroying
 * the returned handle.
 */
Cmiss_nodeset_id Cmiss_nodeset_get_master(Cmiss_nodeset_id nodeset);

/***************************************************************************//**
 * Return the name of the nodeset.
 *
 * @see Cmiss_deallocate()
 * @param nodeset  The nodeset whose name is requested.
 * @return  On success: allocated string containing nodeset name. Up to caller
 * to free using Cmiss_deallocate().
 */
char *Cmiss_nodeset_get_name(Cmiss_nodeset_id nodeset);

/***************************************************************************//**
 * Return the number of nodes in the nodeset.
 *
 * @param nodeset  Handle to the nodeset to query.
 * @return  Number of nodes in nodeset.
 */
int Cmiss_nodeset_get_size(Cmiss_nodeset_id nodeset);

/***************************************************************************//**
 * Check if two nodeset handles refer to the same object.
 *
 * @param nodeset1  The first nodeset to match.
 * @param nodeset2  The second nodeset to match.
 * @return  1 if the two nodesets match, 0 if not.
 */
int Cmiss_nodeset_match(Cmiss_nodeset_id nodeset1, Cmiss_nodeset_id nodeset2);

/***************************************************************************//**
 * Remove a node from the nodeset and any related node groups it is in.
 * Nodes can only be removed if not in use by elements in region.
 * This destroys the node: any handles to it become invalid.
 *
 * @param nodeset  Handle to the nodeset to remove the node from.
 * @param node  The node to be removed.
 * @return  1 if node is successfully removed, 0 if error.
 */
int Cmiss_nodeset_remove_node(Cmiss_nodeset_id nodeset, Cmiss_node_id node);

/***************************************************************************//**
 * Remove from the nodeset and any related node groups all nodes for which the
 * conditional field is true i.e. non-zero valued. Note that group and
 * node_group fields are valid conditional fields.
 * Nodes are only removed if not in use by elements in region.
 * Any handles to removed nodes become invalid.
 *
 * @param nodeset  Handle to the nodeset to remove nodes from.
 * @param conditional_field  Field which if non-zero at any node indicates it
 * is to be removed.
 * @return  The number of nodes removed from the nodeset.
 */
int Cmiss_nodeset_remove_nodes_conditional(Cmiss_nodeset_id nodeset,
    Cmiss_field_id conditional_field);

/***************************************************************************//**
 * Destroys this handle to the node_iterator and sets it to NULL.
 *
 * @param node_iterator_address  Address of handle to node_iterator to destroy.
 */
int Cmiss_node_iterator_destroy(Cmiss_node_iterator_id *node_iterator_address);

/***************************************************************************//**
 * Returns a handle to the next node in the container being iterated over then
 * advances the iterator position. The caller is required to destroy the
 * returned node handle.
 *
 * @param node_iterator  Node iterator to query and advance.
 * @return  Handle to the next node, or NULL if none remaining.
 */
Cmiss_node_id Cmiss_node_iterator_next(Cmiss_node_iterator_id node_iterator);

/***************************************************************************//**
 * Destroys this handle to the node_template and sets it to NULL.
 * Internally this just decrements the reference count.
 *
 * @param node_template_address  Address of handle to node_template
 * to destroy.
 */
int Cmiss_node_template_destroy(Cmiss_node_template_id *node_template_address);

/***************************************************************************//**
 * Defines the field on the node_template with just a single node value per
 * field component with no time variation.
 * Per-component derivatives and multiple versions can be added subsequently.
 * Finalised state is removed by this call.
 *
 * @param node_template  Node template to modify.
 * @param field  The field to define. May be finite_element, stored_string or
 * stored_mesh_location type only.
 * @return  1 on success, 0 on error.
 */
int Cmiss_node_template_define_field(Cmiss_node_template_id node_template,
	Cmiss_field_id field);

/***************************************************************************//**
 * Adds storage for the supplied derivative type for the component/s of the
 * field in the node template. Finalised state is removed by this call.
 * Must have first called Cmiss_node_template_define_field for field.
 *
 * @param node_template  Node template to modify.
 * @param field  The field to define derivatives for. May be finite_element
 * type only.
 * @param component_number  The component from 1 to the number of field
 * components, or -1 to define the derivative for all components.
 * @param derivative_type  The type of nodal derivative to define.
 * @return  1 on success, 0 on error.
 */
int Cmiss_node_template_define_derivative(Cmiss_node_template_id node_template,
	Cmiss_field_id field, int component_number,
	enum Cmiss_nodal_value_type derivative_type);

/***************************************************************************//**
 * Defines variation of all nodal values/derivatives * versions with the
 * supplied time sequence for all components of the field in the node template.
 * Hence there will be as many of each parameter as times in the time sequence.
 * Finalised state is removed by this call.
 *
 * @param node_template  Node template to modify.
 * @param field  The field to define versions for. May be finite_element type
 * only.
 * @param time_sequence  Time sequence object defining the number of times for
 * which field parameters are stored, and the times they are for (increasing).
 * @param node_field_creator  Optionally defines different versions and/or
 * derivative types. If it is NULL then a single nodal value for each component
 * will be defined.
 */
int Cmiss_node_template_define_time_sequence(
	Cmiss_node_template_id node_template, Cmiss_field_id field,
	struct Cmiss_time_sequence *time_sequence);

/***************************************************************************//**
 * Adds storage for multiple versions of nodal values and derivatives for the
 * component/s of the field in the node template. Finalised state is removed by
 * this call.
 * Must have first called Cmiss_node_template_define_field for field.
 *
 * @param node_template  Node template to modify.
 * @param field  The field to define versions for. May be finite_element type
 * only.
 * @param component_number  The component from 1 to the number of field
 * components, or -1 to define the number of versions for all components.
 * @param number_of_versions  The number of versions of each value & derivative
 * stored for the component/s, at least 1 (the default).
 * @return  1 on success, 0 on error.
 */
int Cmiss_node_template_define_versions(Cmiss_node_template_id node_template,
	Cmiss_field_id field, int component_number, int number_of_versions);

/***************************************************************************//**
 * Checks the definition of node fields and prepares the node template for
 * creating new nodes or merging into existing nodes. This function must be
 * successfully called before using the template for these tasks.
 * Finalise state is removed by functions which modify the node template.
 *
 * @param node_template  Node template to finalise.
 * @return  1 if finalised successfully, 0 if field definitions are invalid.
 */
int Cmiss_node_template_finalise(Cmiss_node_template_id node_template);

/*******************************************************************************
 * Returns a new handle to the node with reference count incremented.
 * Caller is responsible for destroying the new handle.
 *
 * @param node  The node to obtain a new reference to.
 * @return  New node handle with incremented reference count.
 */
Cmiss_node_id Cmiss_node_access(Cmiss_node_id node);

/***************************************************************************//**
 * Destroys this handle to the node and sets it to NULL.
 * Internally this just decrements the reference count.
 *
 * @param node_address  Address of handle to the node to destroy.
 */
int Cmiss_node_destroy(Cmiss_node_id *node_address);

/***************************************************************************//**
 * Returns the non-negative integer uniquely identifying the node in its
 * nodeset.
 *
 * @param node  The node to query.
 * @return  The integer identifier of the node, or -1 if node is invalid.
 */
int Cmiss_node_get_identifier(Cmiss_node_id node);

/***************************************************************************//**
 * Modifies the node to define fields as described in the node_template.
 *
 * @param node  The node to modify.
 * @param node_template  Template containing node field descriptions.
 * @return  1 on success, 0 on error.
 */
int Cmiss_node_merge(Cmiss_node_id node, Cmiss_node_template_id node_template);

#endif /* __CMISS_NODE_H__ */
