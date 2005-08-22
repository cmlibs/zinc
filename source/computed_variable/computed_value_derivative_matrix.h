/*******************************************************************************
FILE : computed_value_derivative_matrix.h

LAST MODIFIED : 3 July 2003

DESCRIPTION :
Implements the derivative matrix computed value.
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
#if !defined (__CMISS_VALUE_DERIVATIVE_MATRIX_H__)
#define __CMISS_VALUE_DERIVATIVE_MATRIX_H__

#include "computed_variable/computed_value.h"
#include "computed_variable/computed_variable.h"

/*
Global functions
----------------
*/
int Cmiss_value_derivative_matrix_set_type(Cmiss_value_id value,
	Cmiss_variable_id dependent_variable,int order,
	Cmiss_variable_id *independent_variables,
	Cmiss_value_id *matrices);
/*******************************************************************************
LAST MODIFIED : 5 May 2003

DESCRIPTION :
Makes <value> of type derivative_matrix and sets its <matrices>,
<dependent_variable>, <order> and <independent_variables>.  This function
ACCESSes the <dependent_variable>, <matrices> and <independent_variables>.
After success, the <value> is responsible for DEALLOCATE/DEACCESSing <matrices>,
<dependent_variable> and <independent_variables>.

<matrices> may be NULL.  If not, it should have 2**order-1 entries which are:
	d(dependent_variable)/d(independent_variables[0])
	d(dependent_variable)/d(independent_variables[1])
	d2(dependent_variable)/d(independent_variables[0])d(independent_variables[1])
	d(dependent_variable)/d(independent_variables[2])
	d2(dependent_variable)/d(independent_variables[0])d(independent_variables[2])
	d2(dependent_variable)/d(independent_variables[1])d(independent_variables[2])
	d3(dependent_variable)/d(independent_variables[0])d(independent_variables[1])
		d(independent_variables[2])
	...
	d(dependent_variable)/d(independent_variables[order-1])
	d2(dependent_variable)/d(independent_variables[0])
		d(independent_variables[order-1])
	d2(dependent_variable)/d(independent_variables[1])
		d(independent_variables[order-1])
	d3(dependent_variable)/d(independent_variables[0])d(independent_variables[1])
		d(independent_variables[order-1])
	d2(dependent_variable)/d(independent_variables[2])
		d(independent_variables[order-1])
	d3(dependent_variable)/d(independent_variables[0])d(independent_variables[2])
		d(independent_variables[order-1])
	d3(dependent_variable)/d(independent_variables[1])d(independent_variables[2])
		d(independent_variables[order-1])
	d4(dependent_variable)/d(independent_variables[0])d(independent_variables[1])
		d(independent_variables[2])d(independent_variables[order-1])
	...
	d{order}(dependent_variable)/d(independent_variables[0])
		d(independent_variables[1]) ... d(independent_variables[order-1])

The number of rows for each entry is the number of values for the
<dependent_variable>.  The number of columns for each entry is
	product(1+number_of_values, for each <independent_variable> involved in the
		entry)
==============================================================================*/

PROTOTYPE_CMISS_VALUE_IS_TYPE_FUNCTION(derivative_matrix);

int Cmiss_value_derivative_matrix_get_type(Cmiss_value_id value,
	Cmiss_variable_id *dependent_variable_address,int *order_address,
	Cmiss_variable_id **independent_variables_address,
	Cmiss_value_id **matrices_address);
/*******************************************************************************
LAST MODIFIED : 5 May 2003

DESCRIPTION :
If <value> is of type derivative_matrix, gets its <*dependent_variable_address>,
<*order_address>, <*independent_variables_address> and <*matrices_address>.

The calling program must not DEALLOCATE the returned structures.
==============================================================================*/

int Cmiss_value_derivative_matrix_get_matrix(Cmiss_value_id value,int order,
	Cmiss_variable_id *independent_variables,Cmiss_value_id *matrix_address);
/*******************************************************************************
LAST MODIFIED : 9 May 2003

DESCRIPTION :
If <value> is of type derivative_matrix, this function returns the specified
partial derivative (<order> and <independent_variables>) in <*matrix_address>.

???DB.  Extend so that can have an independent varible that is a subset of
	one of the independent variables for the derivative matrix.  eg nodal values
	for a particular node as a subset of all nodal values
==============================================================================*/
#endif /* !defined (__CMISS_VALUE_DERIVATIVE_MATRIX_H__) */
