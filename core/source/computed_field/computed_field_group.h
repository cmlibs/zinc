/***************************************************************************//**
 *FILE : computed_field_group.h
 *
 * Implements a "group" computed_field which group regions, 
 * node and data point component.
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

#if !defined (COMPUTED_FIELD_GROUP_H)
#define COMPUTED_FIELD_GROUP_H

#include "api/cmiss_field_module.h"
#include "api/cmiss_field_group.h"

/*****************************************************************************//**
 * Sets up command data for group field.
 * 
 * @param computed_field_package  Container for command data.
 * @param root_region  Root region for getting paths to fields in other regions.
 * @return 1 on success, 0 on failure.
 */
int Computed_field_register_type_group(
	struct Computed_field_package *computed_field_package);

typedef int (*Cmiss_field_group_iterator_function)(Cmiss_field_group_id,void *);

/*****************************************************************************//**
 * A convenience function which calls the supplied function for this group and
 * each descendant group throughout the region hierarchy.
 *
 * @param group  group field.
 * @param function  Pointer to the function to be called for each group field.
 * @param user_data  Void pointer to user data to pass to each function.
 * @return 1 on success, 0 on failure.
 */
int Cmiss_field_group_for_each_group_hierarchical(Cmiss_field_group_id group,
	Cmiss_field_group_iterator_function function, void *user_data);

int Cmiss_field_group_clear_region_tree_node(Cmiss_field_group_id group);

int Cmiss_field_group_clear_region_tree_data(Cmiss_field_group_id group);

int Cmiss_field_group_clear_region_tree_element(Cmiss_field_group_id group);

int Cmiss_field_is_type_group(Cmiss_field_id field, void *dummy_void);

#endif /* !defined (COMPUTED_FIELD_GROUP_H) */
