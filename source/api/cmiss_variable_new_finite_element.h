/*******************************************************************************
FILE : api/cmiss_variable_new_finite_element.h

LAST MODIFIED : 4 November 2004

DESCRIPTION :
The public interface to the Cmiss_variable_new finite_element object.
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
#ifndef __API_CMISS_VARIABLE_NEW_FINITE_ELEMENT_H__
#define __API_CMISS_VARIABLE_NEW_FINITE_ELEMENT_H__

#include "api/cmiss_finite_element.h"
#include "api/cmiss_region.h"
#include "api/cmiss_variable_new.h"

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_variable_new_id Cmiss_variable_new_element_xi_create(
	struct Cmiss_element *element,unsigned int number_of_xi,Scalar *xi);
/*******************************************************************************
LAST MODIFIED : 12 November 2003

DESCRIPTION :
Creates a Cmiss_variable_new element/xi which represents the supplied <element>
and <xi>.  <element> should be non-NULL.  If <xi> is non-NULL, then it should
have <number_of_xi> values and <number_of_xi> should equal dimension(<element>).
==============================================================================*/

Cmiss_variable_new_input_id Cmiss_variable_new_input_element_xi_element_xi(
	Cmiss_variable_new_id variable_element_xi);
/*******************************************************************************
LAST MODIFIED : 11 November 2003

DESCRIPTION :
Returns the element/xi input for the <variable_element_xi>.
==============================================================================*/

Cmiss_variable_new_input_id Cmiss_variable_new_input_element_xi_xi(
	Cmiss_variable_new_id variable_element_xi,unsigned int number_of_indices,
	unsigned int *indices);
/*******************************************************************************
LAST MODIFIED : 11 November 2003

DESCRIPTION :
Returns the xi input made up of the specified <indices> for the
<variable_finite_element>.  If <number_of_indices> is zero or <indices> is NULL
then the input refers to all values.
==============================================================================*/

Cmiss_variable_new_id Cmiss_variable_new_finite_element_create(
	Cmiss_region_id region,char *field_name,char *component_name);
/*******************************************************************************
LAST MODIFIED : 4 November 2004

DESCRIPTION :
Creates a Cmiss_variable_new finite_element which represents the <field_name> in
<region>.  If <component_name> is not NULL then that is used to select a
particular component.
==============================================================================*/

Cmiss_variable_new_input_id Cmiss_variable_new_input_finite_element_element_xi(
	Cmiss_variable_new_id variable_finite_element);
/*******************************************************************************
LAST MODIFIED : 11 November 2003

DESCRIPTION :
Returns the element/xi input for the <variable_finite_element>.
==============================================================================*/

Cmiss_variable_new_input_id
	Cmiss_variable_new_input_finite_element_nodal_values(
	Cmiss_variable_new_id variable_finite_element,struct Cmiss_node *node,
	enum FE_nodal_value_type value_type,int version);
/*******************************************************************************
LAST MODIFIED : 11 November 2003

DESCRIPTION :
Returns the nodal values input for the <variable_finite_element>.
==============================================================================*/

Cmiss_variable_new_input_id Cmiss_variable_new_input_finite_element_xi(
	Cmiss_variable_new_id variable_finite_element,unsigned int number_of_indices,
	unsigned int *indices);
/*******************************************************************************
LAST MODIFIED : 11 November 2003

DESCRIPTION :
Returns the xi input made up of the specified <indices> for the
<variable_finite_element>.  If <number_of_indices> is zero or <indices> is NULL
then the input refers to all values.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_VARIABLE_NEW_FINITE_ELEMENT_H__ */
