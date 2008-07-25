/*******************************************************************************
FILE : finite_element_region_private.h

LAST MODIFIED : 21 April 2005

DESCRIPTION :
Private interfaces to FE_region types and functions to be included only by
privileged finite_element modules.
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
#if !defined (FINITE_ELEMENT_REGION_PRIVATE_H)
#define FINITE_ELEMENT_REGION_PRIVATE_H

#include "finite_element/finite_element_region.h"

/*
Private functions
-----------------
*/

struct FE_field_info *FE_region_get_FE_field_info(
	struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Returns a struct FE_field_info for <fe_region>.
This is an object private to FE_region that is common between all fields
owned by FE_region. FE_fields access this object, but this object maintains
a non-ACCESSed pointer to <fe_region> so fields can determine which FE_region
they belong to.
==============================================================================*/

struct FE_node_field_info *FE_region_get_FE_node_field_info(
	struct FE_region *fe_region,
	int number_of_values,	struct LIST(FE_node_field) *fe_node_field_list);
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
Returns a struct FE_node_field_info for the supplied <fe_node_field_list> with
<number_of_values>. The <fe_region> maintains an internal list of these
structures so they can be shared between nodes.
If <node_field_list> is omitted, an empty list is assumed.
==============================================================================*/

int FE_region_remove_FE_node_field_info(struct FE_region *fe_region,
	struct FE_node_field_info *fe_node_field_info);
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
Provided EXCLUSIVELY for the use of DEACCESS and REACCESS functions.
Called when the access_count of <fe_node_field_info> drops to 1 so that
<fe_region> can destroy FE_node_field_info not in use.
==============================================================================*/

struct FE_element_field_info *FE_region_get_FE_element_field_info(
	struct FE_region *fe_region,
	struct LIST(FE_element_field) *fe_element_field_list);
/*******************************************************************************
LAST MODIFIED : 20 February 2003

DESCRIPTION :
Returns a struct FE_element_field_info for the supplied <fe_element_field_list>.
The <fe_region> maintains an internal list of these structures so they can be
shared between elements.
If <element_field_list> is omitted, an empty list is assumed.
==============================================================================*/

int FE_region_get_FE_node_field_info_adding_new_field(
	struct FE_region *fe_region, struct FE_node_field_info **node_field_info_address, 
	struct FE_node_field *new_node_field, int new_number_of_values);
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Updates the pointer to <node_field_info_address> to point to a node_field info
which appends to the fields in <node_field_info_address> one <new_node_field>.
The node_field_info returned in <node_field_info_address> will be for the
<new_number_of_values>.
The <fe_region> maintains an internal list of these structures so they can be 
shared between nodes.  This function allows a fast path when adding a single 
field.  If the node_field passed in is only referenced by one external object
then it is assumed that this function can modify it rather than copying.  If 
there are more references then the object is copied and then modified.
This function handles the access and deaccess of the <node_field_info_address>
as if it is just updating then there is nothing to do.
==============================================================================*/

int FE_region_remove_FE_element_field_info(struct FE_region *fe_region,
	struct FE_element_field_info *fe_element_field_info);
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
Provided EXCLUSIVELY for the use of DEACCESS and REACCESS functions.
Called when the access_count of <fe_element_field_info> drops to 1 so that
<fe_region> can destroy FE_element_field_info not in use.
==============================================================================*/

int FE_region_notify_FE_node_field_change(struct FE_region *fe_region,
	struct FE_node *node, struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 21 April 2005

DESCRIPTION :
Tells the <fe_region> to notify any interested clients that the <node> has
been modified only for <fe_field>.  This is intended to be called by 
<finite_element.c> only as any external code will call through the modify
functions in <finite_element.c>.
==============================================================================*/

int FE_region_notify_FE_element_field_change(struct FE_region *fe_region,
	struct FE_element *element, struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 21 April 2005

DESCRIPTION :
Tells the <fe_region> to notify any interested clients that the <element> has
been modified only for <fe_field>.  This is intended to be called by 
<finite_element.c> only as any external code will call through the modify
functions in <finite_element.c>.
==============================================================================*/

/***************************************************************************//**
 * Private function for use by computed_field_finite_element field wrapping
 * code, to be called in response to region changes - tells it whether it to
 * create a cmiss_number field. Informs only once and requires the existence
 * of a node, element or data point (in the matching data_hack_fe_region).
 * 
 * @param fe_region the finite_element region to check for.
 * @return true if cmiss_number field needs to be made, 0 if not
 */
int FE_region_need_add_cmiss_number_field(struct FE_region *fe_region);

/***************************************************************************//**
 * Private function for use by computed_field_finite_element field wrapping
 * code, to be called in response to region changes - tells it whether to
 * create an xi coordinates field. Informs only once - if any elements exist.
 * 
 * @param fe_region the finite_element region to check for.
 * @return true if cmiss_number field needs to be made, 0 if not
 */
int FE_region_need_add_xi_field(struct FE_region *fe_region);

#endif /* !defined (FINITE_ELEMENT_REGION_PRIVATE_H) */
