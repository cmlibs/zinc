/***************************************************************************//**
 * FILE : cmiss_node_private.hpp
 *
 * Private header file of cmzn_node, cmzn_nodeset.
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (CMZN_NODE_PRIVATE_HPP)
#define CMZN_NODE_PRIVATE_HPP

#include "zinc/node.h"

struct FE_region;

/***************************************************************************//**
 * Ensures all nodes of the supplied element are in this nodeset_group.
 * Candidate for external API.
 *
 * @param nodeset_group  The nodeset group to add nodes to. Must be a subgroup
 * for the master nodeset expected to own the element's nodes.
 * @param element  The element whose nodes are to be added. Face elements
 * inherit nodes from parent elements via field mappings.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
int cmzn_nodeset_group_add_element_nodes(
	cmzn_nodeset_group_id nodeset_group, cmzn_element_id element);

/***************************************************************************//**
 * Ensures all nodes of the supplied element are not in this nodeset_group.
 * Candidate for external API.
 *
 * @param nodeset_group  The nodeset group to remove nodes from. Must be a
 * subgroup for the master nodeset expected to own the element's nodes.
 * @param element  The element whose nodes are to be removed. Face elements
 * inherit nodes from parent elements via field mappings.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
int cmzn_nodeset_group_remove_element_nodes(
	cmzn_nodeset_group_id nodeset_group, cmzn_element_id element);

/** Internal use only.
 * Create a related node list to that in nodeset.
 * @return  New node list.
 */
struct LIST(FE_node) *cmzn_nodeset_create_node_list_internal(cmzn_nodeset_id nodeset);

/** Internal use only
 * @return non-accessed fe_region for this nodeset. Different for datapoints.
 */
FE_region *cmzn_nodeset_get_FE_region_internal(cmzn_nodeset_id nodeset);

/** Internal use only
 * @return non-accessed region for this nodeset.
 */
cmzn_region_id cmzn_nodeset_get_region_internal(cmzn_nodeset_id nodeset);

/** Internal use only
 * @return non-accessed master region for this nodeset.
 */
cmzn_region_id cmzn_nodeset_get_master_region_internal(cmzn_nodeset_id nodeset);

/** Internal use only
 * @return  1 if nodeset represents data points.
 */
int cmzn_nodeset_is_data_internal(cmzn_nodeset_id nodeset);

/***************************************************************************//**
 * If the name is of the form GROUP_NAME.NODESET_NAME. Create a nodeset group.
 * For internal use in command migration only.
 *
 * @param field_module  The field module the nodeset belongs to.
 * @param name  The name of the nodeset: GROUP_NAME.{nodes|datapoints}.
 * @return  Handle to the nodeset, or NULL if error, name already in use or no
 * such nodeset name.
 */
cmzn_nodeset_group_id cmzn_field_module_create_nodeset_group_from_name_internal(
	cmzn_field_module_id field_module, const char *nodeset_group_name);

#endif /* !defined (CMZN_NODE_PRIVATE_HPP) */
