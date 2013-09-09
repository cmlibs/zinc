/***************************************************************************//**
 *FILE : computed_field_group.h
 *
 * Implements a "group" computed_field which group regions, 
 * node and data point component.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (COMPUTED_FIELD_GROUP_H)
#define COMPUTED_FIELD_GROUP_H

#include "zinc/fieldmodule.h"
#include "zinc/fieldgroup.h"

/*****************************************************************************//**
 * Sets up command data for group field.
 * 
 * @param computed_field_package  Container for command data.
 * @param root_region  Root region for getting paths to fields in other regions.
 * @return 1 on success, 0 on failure.
 */
int Computed_field_register_type_group(
	struct Computed_field_package *computed_field_package);

typedef int (*cmzn_field_group_iterator_function)(cmzn_field_group_id,void *);

/*****************************************************************************//**
 * A convenience function which calls the supplied function for this group and
 * each descendant group throughout the region hierarchy.
 *
 * @param group  group field.
 * @param function  Pointer to the function to be called for each group field.
 * @param user_data  Void pointer to user data to pass to each function.
 * @return 1 on success, 0 on failure.
 */
int cmzn_field_group_for_each_group_hierarchical(cmzn_field_group_id group,
	cmzn_field_group_iterator_function function, void *user_data);

int cmzn_field_group_clear_region_tree_node(cmzn_field_group_id group);

int cmzn_field_group_clear_region_tree_data(cmzn_field_group_id group);

int cmzn_field_group_clear_region_tree_element(cmzn_field_group_id group);

int cmzn_field_is_type_group(cmzn_field_id field, void *dummy_void);

#endif /* !defined (COMPUTED_FIELD_GROUP_H) */
