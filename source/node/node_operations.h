/*******************************************************************************
FILE : node_operations.h

LAST MODIFIED : 3 March 2003

DESCRIPTION :
FE_node functions that utilise non finite element data structures and therefore
cannot reside in finite element modules.
==============================================================================*/
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
 * Portions created by the Initial Developer are Copyright (C) 2005
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
#if !defined (NODE_OPERATIONS_H)
#define NODE_OPERATIONS_H

#include "computed_field/computed_field.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "general/multi_range.h"
#include "selection/node_selection.h"

/*
Global functions
----------------
*/

struct LIST(FE_node) *
	FE_node_list_from_fe_region_selection_ranges_condition(
		struct FE_region *fe_region, struct FE_node_selection *node_selection,
		int selected_flag, struct Multi_range *node_ranges,
		struct Computed_field *conditional_field, FE_value time);
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Creates and returns an node list that is the intersection of:
- all the nodes in <fe_region>;
- all nodes in the <node_selection> if <selected_flag> is set;
- all nodes in the given <node_ranges>, if any.
- all nodes for which the <conditional_field> evaluates as "true"
  in its centre at the specified <time>
Up to the calling function to destroy the returned node list.
==============================================================================*/

int FE_region_change_node_identifiers(struct FE_region *fe_region,
	int node_offset, struct Computed_field *sort_by_field, FE_value time);
/*******************************************************************************
LAST MODIFIED : 17 January 2003

DESCRIPTION :
Changes the identifiers of all nodes in <fe_region>.
If <sort_by_field> is NULL, adds <node_offset> to the identifiers.
If <sort_by_field> is specified, it is evaluated for all nodes
in <fe_region> and they are sorted by it - changing fastest with the first
component and keeping the current order where the field has the same values.
Checks for and fails if attempting to give any of the nodes in <fe_region> an
identifier already used by a node in the same master FE_region.
Calls to this function should be enclosed in FE_region_begin_change/end_change.
Note function avoids iterating through FE_region node lists as this is not
allowed during identifier changes.
==============================================================================*/

#endif /* !defined (NODE_OPERATIONS_H) */
