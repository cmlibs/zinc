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
#include "user_interface/message.h"

/*
Global types
------------
*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * Assign values of source field to destination field for nodes in nodeset at
 * the given time. Ignores nodes where either source or destination field is
 * undefined. Optional conditional field allows conditional assignment.
 *
 * @param nodeset  The nodeset to assign in.
 * @param destination_field  The field to assign values to.
 * @param source_field  The field to evaluate values from.
 * @param conditional_field  If supplied, only assigns to nodes for which this
 * field evaluates to true. If NULL, assigns to all nodes in nodeset.
 * @param time  The time to assign values at
 * @return  1 on success, 0 on error.
 */
int Cmiss_nodeset_assign_field_from_source(
	Cmiss_nodeset_id nodeset, Cmiss_field_id destination_field,
	Cmiss_field_id source_field, Cmiss_field_id conditional_field,
	FE_value time);

/*******************************************************************************
 * Assign values of source field to grid-based destination field for elements in
 * mesh at the given time. Ignores elements where either source or destination
 * field is undefined. Optional conditional field allows conditional assignment.
 *
 * @param mesh  The mesh to assign in.
 * @param destination_field  The field to assign values to.
 * @param source_field  The field to evaluate values from.
 * @param conditional_field  If supplied, does not assign to grid points for
 * which this field evaluates to false.
 * @param element_point_ranges_selection  If supplied, does not assign to grid
 * points which are not in this selection.
 * @param time  The time to assign values at
 * @return  1 on success, 0 on error.
 */
int Cmiss_mesh_assign_grid_field_from_source(
	Cmiss_mesh_id mesh, Cmiss_field_id destination_field,
	Cmiss_field_id source_field, Cmiss_field_id conditional_field,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	FE_value time);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !defined (COMPUTED_FIELD_UPDATE_H) */
