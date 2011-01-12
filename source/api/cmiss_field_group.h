/***************************************************************************//**
 * FILE : cmiss_field_group.h
 * 
 * Implements a cmiss field which maintains a group or selection of objects
 * from the region including the region itself, other fields representing domain
 * object groups (e.g. node, element), and related groups from child regions.
 * The field evaluates to 1 (true) at domain locations in the group, and 0
 * elsewhere.  
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

#if !defined (CMISS_FIELD_GROUP_H)
#define CMISS_FIELD_GROUP_H

#include "api/cmiss_field.h"
#include "api/cmiss_field_module.h"

struct Cmiss_field_group;
typedef struct Cmiss_field_group *Cmiss_field_group_id;

struct Cmiss_field_node_group;
typedef struct Cmiss_field_node_group *Cmiss_field_node_group_id;

struct Cmiss_field_element_group;

typedef struct Cmiss_field_element_group *Cmiss_field_element_group_id;

/*new API*/
/*****************************************************************************//**
 * Remove all empty child groups in this group field.
 *
 * @param group  handle to target group field.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_field_group_remove_empty_subgroups(Cmiss_field_group_id group);

/*****************************************************************************//**
 * Check if this group and all its child groups are empty.
 *
 * @param group  handle to target group field.
 * @return  1 if group and all its child groups are empty, otherwise 0.
 */
int Cmiss_field_group_is_empty(Cmiss_field_group_id group);

/*****************************************************************************//**
 * Check if this group is empty.
 *
 * @param group  handle to target group field.
 * @return  1 if group is empty locally, otherwise 0.
 */
int Cmiss_field_group_is_empty_local(Cmiss_field_group_id group);

/*****************************************************************************//**
 * Clear any selection made in this group and its child groups.
 *
 * @param group  handle to target group field.
 * @return  1 if group and its child groups are cleared successfully, otherwise 0.
 */
int Cmiss_field_group_clear(Cmiss_field_group_id group);

/*****************************************************************************//**
 * Clear any selection made in this group only.
 *
 * @param group  handle to target group field.
 * @return  1 if local group is cleared successfully, otherwise 0.
 */
int Cmiss_field_group_clear_local(Cmiss_field_group_id group);

/*****************************************************************************//**
 * Include all domains of the region this group belongs to into the group.
 *
 * @param group  handle to target group field.
 * @return  1 if all domains are added to this group successfully, otherwise 0.
 */
int Cmiss_field_group_add_this_region(Cmiss_field_group_id group);

/*****************************************************************************//**
 * Check if all domains of the region this group belongs too are included.
 *
 * @param group  handle to target group field.
 * @return  1 if all domains are included in this group successfully, otherwise 0.
 */
int Cmiss_field_group_contains_this_region(Cmiss_field_group_id group);

/*****************************************************************************//**
 * Add specified region to group including all of its domains.
 * The specified region must be a child region of
 * the region which group belongs to and it is not already included in the group.
 *
 * @param group  handle to target group field.
 * @param region  handle to target region to be added.
 * @return  1 if successfully add region into group, otherwise 0.
 */
int Cmiss_field_group_add_region(Cmiss_field_group_id group, Cmiss_region_id region);

/*****************************************************************************//**
 * Remove specified region to group if it is in group. The specified region must
 * be a child region of the region which group belongs to.
 *
 * @param group  handle to target group field.
 * @param region  handle to target region to be removed.
 * @return  1 if successfully remove region from group, otherwise 0.
 */
int Cmiss_field_group_remove_region(Cmiss_field_group_id group, Cmiss_region_id region);

/*****************************************************************************//**
 * Check if specified region is included in group.
 *
 * @param group  handle to target group field.
 * @param region  handle to target region.
 * @return  1 if group contains region, otherwise 0.
 */
int Cmiss_field_group_contains_region(Cmiss_field_group_id group, Cmiss_region_id region);

/*****************************************************************************//**
 * Creates a field which can put regions and theirs domains into group.
 *
 * @param field_module  Region field module which will own new field.
 * @return Newly created field
 */
Cmiss_field_id Cmiss_field_module_create_group(Cmiss_field_module_id field_module);

/*****************************************************************************//**
 * If field can be cast to a Cmiss_field_group_id do so
 * and return the field.  Otherwise return NULL.
 * Caller is responsible for destroying the new reference.
 *
 * @param field  handle to the field to cast
 * @return  handle of the cast field, or NULL
 */
Cmiss_field_group_id Cmiss_field_cast_group(Cmiss_field_id field);

/*****************************************************************************//**
 * Cast group field back to its base field and return the field.  Otherwise
 * return NULL.
 *
 * @param group  handle to the group field to cast
 * @return  handle of the field, or NULL
 */
static inline Cmiss_field_id Cmiss_field_group_base_cast(Cmiss_field_group_id group)
{
	return (Cmiss_field_id)(group);
}

/*****************************************************************************//**
 * Destroy the reference to the group.
 *
 * @param group_address  address to the handle to the group field
 * @return  1 if successfully destroy the group, otherwise 0.
 */
int Cmiss_field_group_destroy(Cmiss_field_group_id *group_address);

/*****************************************************************************//**
 * Create a group field of subregion, include it in the specified group and return
 * a handle to the newly created field.
 * Region specified must be a child region of the region which field belongs
 * to and it is not already included in the group. User must destroy the reference
 * of the returned field.
 *
 * @param group  handle to target group field.
 * @param subregion  handle to target region to be added.
 * @return  field if successfully add region into group and create a group field,
 * 		otherwise 0.
 */
Cmiss_field_id Cmiss_field_group_create_subgroup(
	Cmiss_field_group_id group, Cmiss_region_id subregion);

/*****************************************************************************//**
 * Get the group field of subregion from the specified group if it exists.
 * Region specified must be a child region of the region which field belongs
 * to and it is not already included in the group. User must destroy the reference
 * of the returned field.
 *
 * @param group  handle to target group field.
 * @param subregion  handle to target region
 * @return  field if successfully get region from group, otherwise 0.
 */
Cmiss_field_id Cmiss_field_group_get_subgroup(
	Cmiss_field_group_id group, Cmiss_region_id subregion);

/*****************************************************************************//**
 * Create a node group for a nodeset if node group of the same nodeset is not
 * readily available in group and return a handle to the node group field.
 *
 * @param group  handle to target group field.
 * @param nodeset  handle to target nodeset
 * @return  field if successfully create node group, otherwise 0.
 */
Cmiss_field_node_group_id Cmiss_field_group_create_node_group(Cmiss_field_group_id group,
	Cmiss_nodeset_id nodeset);

/*****************************************************************************//**
 * Get a node group for the specified nodeset in group if it exists and return
 * a handle to the node group field.
 *
 * @param group  handle to target group field.
 * @param nodeset  handle to target nodeset
 * @return  field if successfully get node group, otherwise 0.
 * */
Cmiss_field_node_group_id Cmiss_field_group_get_node_group(Cmiss_field_group_id group,
 Cmiss_nodeset_id nodeset);

/*****************************************************************************//**
 * Create an element group for a mesh if element group of the same mesh is not
 * readily available in group and return a handle to the element group field.
 *
 * @param group  handle to target group field.
 * @param mesh  handle to target mesh
 * @return  field if successfully element node group, otherwise 0.
 */
Cmiss_field_element_group_id Cmiss_field_group_create_element_group(Cmiss_field_group_id group,
	Cmiss_fe_mesh_id mesh);

/*****************************************************************************//**
 * Get an element group for the specified mesh in group if it exists and return
 * a handle to the element group field.
 *
 * @param group  handle to target group field.
 * @param mesh  handle to target mesh
 * @return  field if successfully get element group, otherwise 0.
 * */
Cmiss_field_element_group_id Cmiss_field_group_get_element_group(Cmiss_field_group_id group,
	Cmiss_fe_mesh_id mesh);

/**
 * Get a subgroup of the given group for the specified domain.
 * 
 * @param group the group field
 * @param domain the domain field
 * @returns the subgroup field for the specified domain, NULL otherwise
 */
Cmiss_field_id Cmiss_field_group_get_subgroup_for_domain(Cmiss_field_group_id group, Cmiss_field_id domain);

#endif /* !defined (CMISS_FIELD_GROUP_H) */
