/*******************************************************************************
FILE : computed_field_update.h

LAST MODIFIED : 7 January 2003

DESCRIPTION :
Functions for updating values of one computed field from those of another.
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
#if !defined (COMPUTED_FIELD_UPDATE_H)
#define COMPUTED_FIELD_UPDATE_H

#include "finite_element/finite_element.h"
#include "region/cmiss_region.h"
#include "selection/element_point_ranges_selection.h"
#include "selection/element_selection.h"
#include "selection/node_selection.h"
#include "user_interface/message.h"

/*
Global types
------------
*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int Computed_field_copy_values_at_node(struct FE_node *node,
	struct Computed_field *destination_field,
	struct Computed_field *source_field, FE_value time);
/*******************************************************************************
LAST MODIFIED : 4 February 2002

DESCRIPTION :
Evaluates <source_field> at node and sets <destination_field> to those values.
<node> must not be managed -- ie. it should be a local copy.
Both fields must have the same number of values.
Assumes both fields are defined at the node.
Up to user to call Computed_field_clear_cache for each field after calls to
this function are finished.
==============================================================================*/

int Computed_field_update_nodal_values_from_source(
	struct Computed_field *destination_field,	struct Computed_field *source_field,
	struct Cmiss_region *region, struct FE_node_selection *node_selection,
	FE_value time);
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Set <destination_field> in all the nodes in <region> to the values from
<source_field>. Restricts update to nodes in <node_selection>, if supplied.
==============================================================================*/

int Computed_field_update_element_values_from_source(
	struct Computed_field *destination_field,	struct Computed_field *source_field,
	struct Cmiss_region *region,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct FE_element_selection *element_selection, FE_value time);
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Set grid-based <destination_field> in all the elements in <region> to the
values from <source_field>.
Restricts update to grid points which are in <element_point_ranges_selection>
or whose elements are in <element_selection>, if either supplied.
Note the union of these two selections is used if both supplied.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !defined (COMPUTED_FIELD_UPDATE_H) */
