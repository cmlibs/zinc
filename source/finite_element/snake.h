/*******************************************************************************
FILE : snake.h

LAST MODIFIED : 29 March 2006

DESCRIPTION :
Functions for making a snake of 1-D cubic Hermite elements from a chain of
data points.
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
#if !defined (SNAKE_H)
#define SNAKE_H

int create_FE_element_snake_from_data_points(
	struct FE_region *fe_region, struct Computed_field *coordinate_field,
	int number_of_fitting_fields, struct Computed_field **fitting_fields,
	struct LIST(FE_node) *data_list,
	int number_of_elements, FE_value density_factor, FE_value stiffness);
/*******************************************************************************
LAST MODIFIED : 29 March 2006

DESCRIPTION :
Creates a snake out of <number_of_elements> 1-D cubic Hermite elements in
<element_manager> and <element_group>, nodes in <node_manager> and the node
group of the same name in <node_group_manager>. The snake follows the data in
<data_list>. <data_list> is unmodified by this function.
The <fitting_fields> which must be defined on the data are fitted and
defined on the created elements.  The <coordinate_field> is used to determine
distances between points.  The <density_factor> can vary from 0 to 1.
When <density_factor> is 0 then the elements are spread out to have the same
length in this coordinate field, when the <density_factor> is 1 then the
elements will each correspond to an equal proportion of the data points.
A positive value of <stiffness> penalises solutions with large second
derivatives; helps make smooth snakes from few data points.
==============================================================================*/

#endif /* !defined (SNAKE_H) */
