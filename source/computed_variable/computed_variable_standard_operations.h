/*******************************************************************************
FILE : computed_variable_standard_operations.h

LAST MODIFIED : 3 August 2003

DESCRIPTION :
Implements computed variables which carry out standard operations.
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
#if !defined (__CMISS_VARIABLE_STANDARD_OPERATIONS_H__)
#define __CMISS_VARIABLE_STANDARD_OPERATIONS_H__

#include "computed_variable/computed_variable.h"

/*
Global functions
----------------
*/
int Cmiss_variable_dot_product_input_1_set_type(Cmiss_variable_id variable,
	int dimension);
/*******************************************************************************
LAST MODIFIED : 3 August 2003

DESCRIPTION :
Converts the <variable> into the first input for a dot product (=1.2).

Only used to name independent variables and so can't be evaluated.
==============================================================================*/

PROTOTYPE_CMISS_VARIABLE_IS_TYPE_FUNCTION(dot_product_input_1);

int Cmiss_variable_dot_product_input_2_set_type(Cmiss_variable_id variable,
	int dimension);
/*******************************************************************************
LAST MODIFIED : 3 August 2003

DESCRIPTION :
Converts the <variable> into the second input for a dot product (=1.2).

Only used to name independent variables and so can't be evaluated.
==============================================================================*/

PROTOTYPE_CMISS_VARIABLE_IS_TYPE_FUNCTION(dot_product_input_2);

int Cmiss_variable_dot_product_set_type(Cmiss_variable_id variable,
	int dimension);
/*******************************************************************************
LAST MODIFIED : 3 August 2003

DESCRIPTION :
Converts the <variable> into a variable that calculates the dot product of its
inputs.

Independent variables are: dot_product_input_1, dot_product_input_2.
==============================================================================*/

PROTOTYPE_CMISS_VARIABLE_IS_TYPE_FUNCTION(dot_product);

#endif /* !defined (__CMISS_VARIABLE_STANDARD_OPERATIONS_H__) */
