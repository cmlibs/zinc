/***************************************************************************//**
 * FILE : computed_field_subobject_group_internal.hpp
 * 
 * Internal APIs for subobject group types.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (COMPUTED_FIELD_SUBOBJECT_GROUP_INTERNAL_HPP)
#define COMPUTED_FIELD_SUBOBJECT_GROUP_INTERNAL_HPP

#include "zinc/fieldsubobjectgroup.h"

/***************************************************************************//**
 * List statistics about btree structure of node_group.
 */
void cmzn_field_node_group_list_btree_statistics(
	cmzn_field_node_group_id node_group);

/***************************************************************************//**
 * List statistics about btree structure of element_group.
 */
void cmzn_field_element_group_list_btree_statistics(
	cmzn_field_element_group_id element_group);

#endif /* COMPUTED_FIELD_SUBOBJECT_GROUP_INTERNAL_HPP */

