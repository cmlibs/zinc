/**
 * @file nodeset.h
 *
 * The public interface to finite element nodesets, 0-D equivalent of a mesh.
 * Nodes are also used for other point domains e.g. data points.
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_NODESET_H__
#define CMZN_NODESET_H__

#include "types/fieldid.h"
#include "types/fieldmoduleid.h"
#include "types/nodeid.h"
#include "types/nodesetid.h"
#include "types/nodetemplateid.h"

#include "opencmiss/zinc/zincsharedobject.h"

/*
Global functions
----------------
*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get a handle to a nodeset by its field domain type, either
 * CMZN_FIELD_DOMAIN_TYPE_NODES or CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS.
 *
 * @param fieldmodule  The field module the nodeset belongs to.
 * @param domain_type  CMZN_FIELD_DOMAIN_TYPE_NODES or CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS.
 * @return  Handle to the nodeset, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_nodeset_id cmzn_fieldmodule_find_nodeset_by_field_domain_type(
	cmzn_fieldmodule_id fieldmodule, enum cmzn_field_domain_type domain_type);

/**
 * Get a handle to a nodeset from its name in the field module. A nodeset is the
 * container of nodes - i.e. cmzn_node objects. Valid names may be any
 * node_group field, or the following special names:
 * "nodes" = the primary set of nodes for a region, able to be indexed by
 * elements for storing or mapping to finite element field parameters.
 * "datapoints" = an additional set of nodes generally used to represent data
 * points, not for finite element field parameters.
 * Note that the default names for node group fields created from a group
 * is GROUP_NAME.NODESET_NAME, with nodeset names as above.
 *
 * @param fieldmodule  The field module the nodeset belongs to.
 * @param name  The name of the nodeset.
 * @return  Handle to the nodeset, or NULL/invalid handle if not found or failed.
 */
ZINC_API cmzn_nodeset_id cmzn_fieldmodule_find_nodeset_by_name(
	cmzn_fieldmodule_id fieldmodule, const char *nodeset_name);

/**
 * Returns a new handle to the nodeset with reference count incremented.
 *
 * @param nodeset  The nodeset to obtain a new handle to.
 * @return  New handle to nodeset, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_nodeset_id cmzn_nodeset_access(cmzn_nodeset_id nodeset);

/**
 * Destroys this handle to the nodeset and sets it to NULL.
 * Internally this decrements the reference count.
 *
 * @param nodeset_address  Address of handle to the nodeset to destroy.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_nodeset_destroy(cmzn_nodeset_id *nodeset_address);

/**
 * Returns whether the node is from the nodeset.
 *
 * @param nodeset  The nodeset to query.
 * @param node  The node to query about.
 * @return  Boolean true if node is in the nodeset, otherwise false.
 */
ZINC_API bool cmzn_nodeset_contains_node(cmzn_nodeset_id nodeset, cmzn_node_id node);

/**
 * Create a blank template from which new nodes can be created in this nodeset.
 * Used to describe how finite element fields are defined.
 * Also used for defining new fields on existing nodes.
 *
 * @param nodeset  Handle to the nodeset the template works with.
 * @return  Handle to new nodetemplate, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_nodetemplate_id cmzn_nodeset_create_nodetemplate(
	cmzn_nodeset_id nodeset);

/**
 * Create a new node in this nodeset with fields defined as in the
 * node_template. Returns handle to new node.
 *
 * @param nodeset  Handle to the nodeset to create the new node in.
 * @param identifier  Non-negative integer identifier of new node, or -1
 * to automatically generate, starting from 1. Fails if supplied identifier
 * already used by an existing node.
 * @param node_template  Template for defining node fields.
 * @return  Handle to new node, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_node_id cmzn_nodeset_create_node(cmzn_nodeset_id nodeset,
	int identifier, cmzn_nodetemplate_id node_template);

/**
 * Create a node iterator object for iterating through the nodes in the nodeset
 * which are ordered from lowest to highest identifier. The iterator initially
 * points at the position before the first node, so the first call to
 * cmzn_nodeiterator_next() returns the first node and advances the iterator.
 * Iterator becomes invalid if nodeset is modified or any of its nodes are
 * given new identifiers.
 *
 * @param nodeset  Handle to the nodeset to iterate over.
 * @return  Handle to new node iterator, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_nodeiterator_id cmzn_nodeset_create_nodeiterator(
	cmzn_nodeset_id nodeset);

/**
 * Destroy all nodes in nodeset, also removing them from any related groups.
 * Any nodes in use by elements are prevented from being destroyed.
 * All handles to the destroyed nodes become invalid.
 *
 * @param nodeset  Handle to nodeset to destroy nodes from.
 * @return  Result OK if all nodes destroyed, ERROR_IN_USE if partial success
 * with some nodes not destroyed due to being in use by elements, or any other
 * error if more serious failure.
 */
ZINC_API int cmzn_nodeset_destroy_all_nodes(cmzn_nodeset_id nodeset);

/**
 * Destroy the node if it is in the nodeset and not in use by elements.
 * Removes node from any related groups it is in.
 * All handles to the destroyed node become invalid.
 *
 * @param nodeset  Handle to the nodeset whose node is to be destroyed.
 * @param node  The node to destroy.
 * @return  Result OK if node destroyed, ERROR_IN_USE if it could not be
 * destroyed due to being in use by elements, or any other error if more
 * serious failure.
 */
ZINC_API int cmzn_nodeset_destroy_node(cmzn_nodeset_id nodeset, cmzn_node_id node);

/**
 * Destroy all nodes in the nodeset for which the conditional field is true i.e.
 * non-zero valued. These nodes are removed from any related groups they are in.
 * Nodes are only removed if not in use by elements in region.
 * All handles to destroyed nodes become invalid.
 * Note that group and node_group fields are valid conditional fields.
 *
 * @param nodeset  Handle to the nodeset to destroy nodes from.
 * @param conditional_field  Field which if non-zero at any node indicates it
 * is to be destroyed.
 * @return  Status CMZN_OK on success, any other value on failure.
 * @return  Result OK if all nodes passing condition destroyed, ERROR_IN_USE if
 * partial success with some nodes not destroyed due to being in use by
 * elements, or any other error if more serious failure.
 */
ZINC_API int cmzn_nodeset_destroy_nodes_conditional(cmzn_nodeset_id nodeset,
	cmzn_field_id conditional_field);

/**
 * Return a handle to the node in the nodeset with this identifier.
 *
 * @param nodeset  Handle to the nodeset to find the node in.
 * @param identifier  Non-negative integer identifier of node.
 * @return  Handle to node, or NULL/invalid handle if not found or failed.
 */
ZINC_API cmzn_node_id cmzn_nodeset_find_node_by_identifier(cmzn_nodeset_id nodeset,
	int identifier);

/**
 * Returns handle to field module for region this nodeset belongs to.
 *
 * @param nodeset  The nodeset from which to obtain the field module.
 * @return  Handle to field module, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_fieldmodule_id cmzn_nodeset_get_fieldmodule(cmzn_nodeset_id nodeset);

/**
 * Get the master nodeset which owns the nodes for this nodeset. Can be the
 * same as the supplied nodeset if it is a master.
 *
 * @param nodeset  The nodeset to query.
 * @return  Handle to master nodeset, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_nodeset_id cmzn_nodeset_get_master_nodeset(cmzn_nodeset_id nodeset);

/**
 * Return the name of the nodeset.
 *
 * @see cmzn_deallocate
 * @param nodeset  The nodeset whose name is requested.
 * @return  On success: allocated string containing nodeset name. Up to caller
 * to free using cmzn_deallocate().
 */
ZINC_API char *cmzn_nodeset_get_name(cmzn_nodeset_id nodeset);

/**
 * Return the number of nodes in the nodeset.
 *
 * @param nodeset  Handle to the nodeset to query.
 * @return  Number of nodes in nodeset.
 */
ZINC_API int cmzn_nodeset_get_size(cmzn_nodeset_id nodeset);

/**
 * Check if two nodeset handles refer to the same object.
 *
 * @param nodeset1  The first nodeset to match.
 * @param nodeset2  The second nodeset to match.
 * @return  Boolean true if the two nodesets match, false if not.
 */
ZINC_API bool cmzn_nodeset_match(cmzn_nodeset_id nodeset1, cmzn_nodeset_id nodeset2);

/**
 * If the nodeset is a nodeset group i.e. subset of nodes from a master nodeset,
 * get the nodeset group specific interface for add/remove functions.
 *
 * @param field  The nodeset to be cast.
 * @return  Handle to derived nodeset group, or NULL/invalid handle if wrong type or failed.
 */
ZINC_API cmzn_nodeset_group_id cmzn_nodeset_cast_group(cmzn_nodeset_id nodeset);

/**
 * Destroys this handle to the nodeset group and sets it to NULL.
 * Internally this decrements the reference count.
 *
 * @param nodeset_group_address  Address of nodeset group handle to destroy.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_nodeset_group_destroy(cmzn_nodeset_group_id *nodeset_group_address);

/**
 * Cast nodeset group back to its base nodeset class.
 * IMPORTANT NOTE: Returned nodeset does not have incremented reference count
 * and must not be destroyed. Use cmzn_nodeset_access() to add a reference if
 * maintaining returned handle beyond the lifetime of the nodeset_group.
 * Use this function to call base-class API, e.g.:
 * int size = cmzn_nodeset_get_size(cmzn_nodeset_group_base_cast(nodeset_group);
 *
 * @param nodeset_group  Handle to the nodeset group to cast.
 * @return  Non-accessed handle to the nodeset or NULL if failed.
 */
ZINC_C_INLINE cmzn_nodeset_id cmzn_nodeset_group_base_cast(
	cmzn_nodeset_group_id nodeset_group)
{
	return (cmzn_nodeset_id)(nodeset_group);
}

/**
 * Add specified node to nodeset group.
 *
 * @param nodeset_group  Handle to nodeset group to modify.
 * @param node  Handle to node to add. Must be from the group's master nodeset.
 * @return  Status CMZN_OK on success, CMZN_ERROR_ALREADY_EXISTS if node was
 * already in group but otherwise successful, or any other value on more
 * serious failure.
 */
ZINC_API int cmzn_nodeset_group_add_node(cmzn_nodeset_group_id nodeset_group,
	cmzn_node_id node);

/**
 * Ensure this nodeset group contains all nodes from the master nodeset for
 * which the conditional field is true i.e. non-zero valued at the node.
 *
 * @param nodeset_group  Handle to the nodeset group to add nodes to.
 * @param conditional_field  Field which if non-zero in a node indicates it
 * is to be added.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_nodeset_group_add_nodes_conditional(
	cmzn_nodeset_group_id nodeset_group, cmzn_field_id conditional_field);

/**
 * Remove all nodes from nodeset group.
 *
 * @param nodeset_group  Handle to nodeset group to modify.
 * @return  Status CMZN_OK if all nodes removed, any other value if failed.
 */
ZINC_API int cmzn_nodeset_group_remove_all_nodes(cmzn_nodeset_group_id nodeset_group);

/**
 * Remove specified node from nodeset group.
 *
 * @param nodeset_group  Handle to nodeset group to modify.
 * @param node  Handle to node to remove.
 * @return  Status CMZN_OK on success, CMZN_ERROR_NOT_FOUND if node
 * was not in the group but otherwise successful, or any other value
 * on more serious failure.
 */
ZINC_API int cmzn_nodeset_group_remove_node(cmzn_nodeset_group_id nodeset_group,
	cmzn_node_id node);

/**
 * Remove all nodes from the nodeset group for which the conditional field is
 * true i.e. non-zero valued in the node.
 * Note that group and node_group fields are valid conditional fields.
 *
 * @param nodeset_group  Handle to the nodeset group to remove nodes from.
 * @param conditional_field  Field which if non-zero in the node indicates it
 * is to be removed.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_nodeset_group_remove_nodes_conditional(
	cmzn_nodeset_group_id nodeset_group, cmzn_field_id conditional_field);

/**
 * Returns a new handle to the nodeset changes with reference count incremented.
 *
 * @param nodesetchanges  The nodeset changes to obtain a new handle to.
 * @return  New handle to the nodeset changes, or NULL/invalid handle on failure.
 */
ZINC_API cmzn_nodesetchanges_id cmzn_nodesetchanges_access(
	cmzn_nodesetchanges_id nodesetchanges);

/**
 * Destroys this handle to the nodesetchanges and sets it to NULL.
 * Internally this decrements the reference count.
 *
 * @param nodesetchanges_address  Address of handle to the nodeset changes to destroy.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_nodesetchanges_destroy(cmzn_nodesetchanges_id *nodesetchanges_address);

/**
 * Returns if and how a node has changed in the nodeset changes.
 *
 * @param nodesetchanges  The nodeset changes to query.
 * @param node  The node to query about.
 * @return  Logical OR of change bit flags. See #cmzn_node_change_flag.
 */
ZINC_API cmzn_node_change_flags cmzn_nodesetchanges_get_node_change_flags(
	cmzn_nodesetchanges_id nodesetchanges, cmzn_node_id node);

/**
 * Returns the number of changes to nodes in the nodeset changes, or
 * -1 if all considered changed, i.e. too many changes occurred to record.
 *
 * @param nodesetchanges  The nodeset changes to query.
 * @return  The number of changes to nodes, or -1 if all changed.
 */
ZINC_API int cmzn_nodesetchanges_get_number_of_changes(
	cmzn_nodesetchanges_id nodesetchanges);

/**
 * Returns logical OR of change flags for all nodes in the nodeset changes.
 *
 * @param nodesetchanges  The nodeset changes to query.
 * @return  Logical OR of change bit flags. See #cmzn_node_change_flag.
 */
ZINC_API cmzn_node_change_flags cmzn_nodesetchanges_get_summary_node_change_flags(
	cmzn_nodesetchanges_id nodesetchanges);

#ifdef __cplusplus
}
#endif

#endif // CMZN_NODESET_H__
