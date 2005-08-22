/*******************************************************************************
FILE : computed_value_finite_element.h

LAST MODIFIED : 11 July 2003

DESCRIPTION :
Implements computed values which interface to finite elements:
- element_xi
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
#if !defined (__CMISS_VALUE_FINITE_ELEMENT_H__)
#define __CMISS_VALUE_FINITE_ELEMENT_H__

#include "finite_element/finite_element.h"
#include "computed_variable/computed_value.h"

/*
Global functions
----------------
*/
int Cmiss_value_element_xi_set_type(Cmiss_value_id value,int dimension,
	struct FE_element *element,FE_value *xi);
/*******************************************************************************
LAST MODIFIED : 11 July 2003

DESCRIPTION :
Makes <value> of type element_xi and sets its <dimension>, <element> and <xi).
<dimension> must be positive or <element> must be non-NULL.  If <dimension> is
positive and <element> is non-NULL then <dimension> should equal the dimension
of the element.  After success, the <value> is responsible for DEALLOCATEing
<xi>.

???DB.  Assuming that the <element> knows its FE_region (can get manager)
==============================================================================*/

PROTOTYPE_CMISS_VALUE_IS_TYPE_FUNCTION(element_xi);

int Cmiss_value_element_xi_get_type(Cmiss_value_id value,int *dimension_address,
	struct FE_element **element_address,FE_value **xi_address);
/*******************************************************************************
LAST MODIFIED : 11 July 2003

DESCRIPTION :
If <value> is of type element_xi, gets its <*dimension_address>,
<*element_address> and <*xi_address).

The calling program must not DEALLOCATE the returned <*xi_address>.
==============================================================================*/
#endif /* !defined (__CMISS_VALUE_FINITE_ELEMENT_H__) */
