/*******************************************************************************
FILE : node_operations.h

LAST MODIFIED : 3 March 2003

DESCRIPTION :
FE_node functions that utilise non finite element data structures and therefore
cannot reside in finite element modules.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (NODE_OPERATIONS_H)
#define NODE_OPERATIONS_H

#include "computed_field/computed_field.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "general/multi_range.h"

/*
Global functions
----------------
*/

/**
 * Changes the identifiers of all nodes in nodeset.
 * If <sort_by_field> is NULL, adds <node_offset> to the identifiers.
 * If <sort_by_field> is specified, it is evaluated for all nodes
 * in nodeset and they are sorted by it - changing fastest with the first
 * component and keeping the current order where the field has the same values.
 * Checks for and fails if attempting to give any of the nodes an
 * identifier already used by a node in the same master nodeset.
 * Note function avoids iterating through nodeset's own list as this is not
 * allowed during identifier changes.
 */
int cmzn_nodeset_change_node_identifiers(cmzn_nodeset_id nodeset,
	int node_offset, cmzn_field_id sort_by_field, FE_value time);

/**
 * Create a node list from a subset of the nodeset at a specific time.
 *
 * @param nodeset  The nodeset to create a subset from.
 * @param node_ranges  Optional ranges of node identifiers to satisfy.
 * @param conditional_field  Optional conditional field node must return true for.
 * @param time  Time to evaluate the conditional field at.
 * @return  Returns node list if successfully create a node list with the given
 *    arguments, otherwise NULL.
 */
struct LIST(FE_node) *cmzn_nodeset_create_node_list_ranges_conditional(
	cmzn_nodeset *nodeset, struct Multi_range *node_ranges,
	cmzn_field *conditional_field, FE_value time);

#endif /* !defined (NODE_OPERATIONS_H) */
