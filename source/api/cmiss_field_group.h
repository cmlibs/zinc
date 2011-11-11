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

#include "api/types/cmiss_c_inline.h"
#include "api/types/cmiss_element_id.h"
#include "api/types/cmiss_field_id.h"
#include "api/types/cmiss_field_group_id.h"
#include "api/types/cmiss_field_module_id.h"
#include "api/types/cmiss_node_id.h"
#include "api/types/cmiss_region_id.h"
#include "api/types/cmiss_field_subobject_group_id.h"

/***************************************************************************//**
 * Creates a group field which can contain an arbitrary set of subregions or
 * region subobjects, and works as a boolean-valued field returning 1 on domains
 * in the group, 0 otherwise.
 *
 * @param field_module  Region field module which will own new field.
 * @return  Handle to newly created field.
 */
Cmiss_field_id Cmiss_field_module_create_group(Cmiss_field_module_id field_module);

/***************************************************************************//**
 * If the field is of group type, then this function returns the group specific
 * representation, otherwise it returns NULL.
 * Caller is responsible for destroying the returned derived field reference.
 *
 * @param field  The generic field to be cast.
 * @return  Group specific representation if the input field is of this type,
 * otherwise NULL.
 */
Cmiss_field_group_id Cmiss_field_cast_group(Cmiss_field_id field);

/***************************************************************************//**
 * Cast group field back to its base field and return the field.
 * IMPORTANT NOTE: Returned field does not have incremented reference count and
 * must not be destroyed. Use Cmiss_field_access() to add a reference if
 * maintaining returned handle beyond the lifetime of the group argument.
 * Use this function to call base-class API, e.g.:
 * Cmiss_field_set_name(Cmiss_field_group_base_cast(group_field), "bob");
 *
 * @param group  Handle to the group field to cast.
 * @return  Non-accessed handle to the base field or NULL if failed.
 */
CMISS_C_INLINE Cmiss_field_id Cmiss_field_group_base_cast(Cmiss_field_group_id group)
{
	return (Cmiss_field_id)(group);
}

/***************************************************************************//**
 * Destroys this reference to the group field (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param group_address  Address of handle to the group field.
 * @return  Status CMISS_OK if successfully destroyed the group handle,
 * 		any other value on failure.
 */
int Cmiss_field_group_destroy(Cmiss_field_group_id *group_address);

/***************************************************************************//**
 * Query if this group and all its subregion and sub-object groups are empty.
 *
 * @param group  Handle to group field to query.
 * @return  1 if group and all its subgroups are empty, otherwise 0.
 */
int Cmiss_field_group_is_empty(Cmiss_field_group_id group);

/***************************************************************************//**
 * Query if this group contains no objects from the local region.
 *
 * @param group  Handle to group field to query.
 * @return  1 if group is empty locally, otherwise 0.
 */
int Cmiss_field_group_is_empty_local(Cmiss_field_group_id group);

/***************************************************************************//**
 * Remove all objects from this group, clear all its subgroups, and remove &
 * destroy them if possible.
 *
 * @param group  Handle to group field to modify.
 * @return  Status CMISS_OK if group and its child groups cleared successfully,
 * 		any other value on failure.
 */
int Cmiss_field_group_clear(Cmiss_field_group_id group);

/***************************************************************************//**
 * Remove all local objects from group, but leave subregion subgroups intact.
 *
 * @param group  Handle to group field to modify.
 * @return  Status CMISS_OK if group is successfully cleared locally,
 * 		any other value on failure.
 */
int Cmiss_field_group_clear_local(Cmiss_field_group_id group);

/***************************************************************************//**
 * Remove and destroy all empty subregion and subobject groups of this group.
 * Empty subgroups in use by other clients may remain after call.
 *
 * @param group  Handle to group field to modify.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
int Cmiss_field_group_remove_empty_subgroups(Cmiss_field_group_id group);

/***************************************************************************//**
 * Add the local/owning region of this group field to the group, i.e. all local
 * objects/domains. Local sub-object groups are cleared and destroyed.
 * This function is not hierarchical: subregions are not added.
 *
 * @param group  Handle to group field to modify.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
int Cmiss_field_group_add_local_region(Cmiss_field_group_id group);

/***************************************************************************//**
 * Query if group contains its local/owning region, i.e. all local objects/
 * domains.
 * This function is not hierarchical: subregions are not checked.
 *
 * @param group  Handle to group field to query.
 * @return  1 if this region is in the group, otherwise 0.
 */
int Cmiss_field_group_contains_local_region(Cmiss_field_group_id group);

/***************************************************************************//**
 * Add the specified region to the group i.e. all its objects/domains.
 * The specified region must be in the tree of this group's local/owning region
 * and not already in the group.
 * This function is not hierarchical: subregions are not added.
 *
 * @param group  Handle to group field to modify.
 * @param region  Handle to region to be added.
 * @return  Status CMISS_OK if successfully add region into group, any other
 * 		value on failure.
 */
int Cmiss_field_group_add_region(Cmiss_field_group_id group, Cmiss_region_id region);

/***************************************************************************//**
 * Remove specified region from group if currently in it.
 * The specified region must be in the tree of this group's local/owning region.
 * This function is not hierarchical: subregions are not removed.
 *
 * @param group  Handle to group field to modify.
 * @param region  Handle to region to be removed.
 * @return  Status CMISS_OK if region successfully removed from group, any other
 * 		value on failure.
 */
int Cmiss_field_group_remove_region(Cmiss_field_group_id group, Cmiss_region_id region);

/***************************************************************************//**
 * Query if specified region is in the group i.e. all its objects/domains.
 * The specified region must be in the tree of this group's local/owning region.
 * This function is not hierarchical: subregions are not checked.
 *
 * @param group  Handle to group field to query.
 * @param region  Handle to region to check.
 * @return  1 if group contains region, otherwise 0.
 */
int Cmiss_field_group_contains_region(Cmiss_field_group_id group, Cmiss_region_id region);

/***************************************************************************//**
 * Create a group field for the specified subregion, include it in the specified
 * group and return a handle to the newly created sub-group field.
 * The specified region must be in the tree of this group's local/owning region
 * and not already in the group.
 * Caller is responsible for destroying the returned group field handle.
 *
 * @param group  Handle to group field to modify.
 * @param subregion  Handle to region to create a subgroup for.
 * @return  Handle to new, empty sub-group field on success, NULL on failure.
 */
Cmiss_field_group_id Cmiss_field_group_create_subregion_group(
	Cmiss_field_group_id group, Cmiss_region_id subregion);

/***************************************************************************//**
 * Get the group field for subregion in the specified group if it exists.
 * The specified region must be in the tree of this group's local/owning region.
 * Caller is responsible for destroying the returned group field handle.
 *
 * @param group  Handle to group field to query.
 * @param subregion  Handle to region to get the subgroup for.
 * @return  Handle to sub-group field or NULL if none.
 */
Cmiss_field_group_id Cmiss_field_group_get_subregion_group(Cmiss_field_group_id group,
	Cmiss_region_id subregion);

/***************************************************************************//**
 * Create and return a handle to a node group field compatible with the supplied
 * nodeset, i.e. able to contain nodes from its master nodeset. The node group
 * field is registered as a sub-object group for this group.
 * Fails if a compatible node group field already exists.
 * Caller is responsible for destroying the returned field handle.
 *
 * @param group  Handle to group field to modify.
 * @param nodeset  Handle to a nodeset the node group is to be compatible with.
 * If not already a master nodeset, the master is obtained from it.
 * @return  Handle to new node group field, or NULL on failure.
 */
Cmiss_field_node_group_id Cmiss_field_group_create_node_group(
	Cmiss_field_group_id group, Cmiss_nodeset_id nodeset);

/***************************************************************************//**
 * Find and return handle to the sub-object node group compatible with the
 * specified nodeset, if one exists for the group.
 * Caller is responsible for destroying the returned field handle.
 *
 * @param group  Handle to group field to query.
 * @param nodeset  Handle to a nodeset the node group is to be compatible with.
 * If not already a master nodeset, the master is obtained from it.
 * @return  Handle to node group field, or NULL if none.
 */
Cmiss_field_node_group_id Cmiss_field_group_get_node_group(
	Cmiss_field_group_id group, Cmiss_nodeset_id nodeset);

/***************************************************************************//**
 * Create and return a handle to an element group field compatible with the
 * supplied mesh, i.e. able to contain elements from its master mesh. The
 * element group field is registered as a sub-object group for this group.
 * Fails if a compatible element group field already exists.
 * Caller is responsible for destroying the returned field handle.
 *
 * @param group  Handle to group field to modify.
 * @param mesh  Handle to a mesh the element group is to be compatible with.
 * If not already a master mesh, the master is obtained from it.
 * @return  Handle to new element group field, or NULL on failure.
 */
Cmiss_field_element_group_id Cmiss_field_group_create_element_group(
	Cmiss_field_group_id group, Cmiss_mesh_id mesh);

/***************************************************************************//**
 * Find and return handle to the sub-object element group compatible with the
 * specified mesh, if one exists for the group.
 * Caller is responsible for destroying the returned field handle.
 *
 * @param group  Handle to group field to query.
 * @param mesh  Handle to a mesh the element group is to be compatible with.
 * If not already a master mesh, the master is obtained from it.
 * @return  Handle to element group field, or NULL if none.
 */
Cmiss_field_element_group_id Cmiss_field_group_get_element_group(
	Cmiss_field_group_id group, Cmiss_mesh_id mesh);

/***************************************************************************//**
 * Get a subgroup of the given group for the specified domain.
 * 
 * @param group the group field
 * @param domain the domain field
 * @returns the subgroup field for the specified domain, NULL otherwise
 */
Cmiss_field_id Cmiss_field_group_get_subobject_group_for_domain(Cmiss_field_group_id group, Cmiss_field_id domain);

/***************************************************************************//**
 * Return the first non-empty subregion group in the group tree including itself.
 *
 * @param group  the group field
 * @returns  the first non-empty subregion group field, NULL otherwise.
 */
Cmiss_field_group_id Cmiss_field_group_get_first_non_empty_group(
	Cmiss_field_group_id group);
#endif /* !defined (CMISS_FIELD_GROUP_H) */
