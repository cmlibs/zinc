/***************************************************************************//**
 * FILE : cmiss_node_private.hpp
 *
 * Private header file of Cmiss_node, Cmiss_nodeset.
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
#if !defined (CMISS_NODE_PRIVATE_HPP)
#define CMISS_NODE_PRIVATE_HPP

#include "api/cmiss_node.h"

struct FE_region;

/***************************************************************************//**
 * Ensures all nodes of the supplied element are in this nodeset_group.
 * Candidate for external API.
 *
 * @param nodeset_group  The nodeset group to add nodes to. Must be a subgroup
 * for the master nodeset expected to own the element's nodes.
 * @param element  The element whose nodes are to be added. Face elements
 * inherit nodes from parent elements via field mappings.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
int Cmiss_nodeset_group_add_element_nodes(
	Cmiss_nodeset_group_id nodeset_group, Cmiss_element_id element);

/***************************************************************************//**
 * Ensures all nodes of the supplied element are not in this nodeset_group.
 * Candidate for external API.
 *
 * @param nodeset_group  The nodeset group to remove nodes from. Must be a
 * subgroup for the master nodeset expected to own the element's nodes.
 * @param element  The element whose nodes are to be removed. Face elements
 * inherit nodes from parent elements via field mappings.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
int Cmiss_nodeset_group_remove_element_nodes(
	Cmiss_nodeset_group_id nodeset_group, Cmiss_element_id element);

/** Internal use only.
 * Create a related node list to that in nodeset.
 * @return  New node list.
 */
struct LIST(FE_node) *Cmiss_nodeset_create_node_list_internal(Cmiss_nodeset_id nodeset);

/** Internal use only
 * @return non-accessed fe_region for this nodeset. Different for cmiss_data.
 */
FE_region *Cmiss_nodeset_get_FE_region_internal(Cmiss_nodeset_id nodeset);

/** Internal use only
 * @return non-accessed region for this nodeset.
 */
Cmiss_region_id Cmiss_nodeset_get_region_internal(Cmiss_nodeset_id nodeset);

/** Internal use only
 * @return non-accessed master region for this nodeset.
 */
Cmiss_region_id Cmiss_nodeset_get_master_region_internal(Cmiss_nodeset_id nodeset);

/** Internal use only
 * @return  1 if nodeset represents data points.
 */
int Cmiss_nodeset_is_data_internal(Cmiss_nodeset_id nodeset);

/***************************************************************************//**
 * If the name is of the form GROUP_NAME.NODESET_NAME. Create a nodeset group.
 * For internal use in command migration only.
 *
 * @param field_module  The field module the nodeset belongs to.
 * @param name  The name of the nodeset: GROUP_NAME.{cmiss_nodes|cmiss_data}.
 * @return  Handle to the nodeset, or NULL if error, name already in use or no
 * such nodeset name.
 */
Cmiss_nodeset_group_id Cmiss_field_module_create_nodeset_group_from_name_internal(
	Cmiss_field_module_id field_module, const char *nodeset_group_name);

#endif /* !defined (CMISS_NODE_PRIVATE_HPP) */
