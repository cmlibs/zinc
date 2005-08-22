/*******************************************************************************
FILE : computed_variable_derivative.h

LAST MODIFIED : 9 July 2003

DESCRIPTION :
Implements the derivative computed variable.

???DB.  divergence and inverse are here to get them out of computed_variable and
	they will end up in their own modules
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
#if !defined (__CMISS_VARIABLE_DERIVATIVE_H__)
#define __CMISS_VARIABLE_DERIVATIVE_H__

#include "computed_variable/computed_value.h"
#include "computed_variable/computed_variable.h"

/*
Global functions
----------------
*/
int Cmiss_variable_derivative_set_type(Cmiss_variable_id derivative,
	Cmiss_variable_id dependent_variable,int order,
	Cmiss_variable_id *independent_variables);
/*******************************************************************************
LAST MODIFIED : 21 March 2003

DESCRIPTION :
Sets <derivative> to be the derivative of the <dependent_variable> with respect
to the <independent_variables>.  This function ACCESSes the <dependent_variable>
and <independent_variables>.  After success, the <derivative> is responsible for
DEACCESS/DEALLOCATEing <dependent_variable> and <independent_variables>.
==============================================================================*/

int Cmiss_variable_divergence_set_type(Cmiss_variable_id divergence,
	Cmiss_variable_id dependent_variable,Cmiss_variable_id independent_variable);
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
Sets <divergence> to be the divergence of the <dependent_variable> with respect
to the <independent_variable>.
==============================================================================*/

int Cmiss_variable_inverse_set_type(Cmiss_variable_id inverse_variable,
	Cmiss_variable_id variable,struct LIST(Cmiss_variable) *dependent_variables);
/*******************************************************************************
LAST MODIFIED : 24 January 2003

DESCRIPTION :
Sets <inverse_variable> to be the inverse of the <variable>.  Its independent
variables are the dependent variables of the <variable> and its
<dependent_variables> are independent variables of the <variable>.
==============================================================================*/
#endif /* !defined (__CMISS_VARIABLE_DERIVATIVE_H__) */
