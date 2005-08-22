/*******************************************************************************
FILE : api/cmiss_value_derivative_matrix.h

LAST MODIFIED : 12 August 2003

DESCRIPTION :
The public interface to the Cmiss_value_derivative_matrix object.
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
#ifndef __API_CMISS_VALUE_DERIVATIVE_MATRIX_H__
#define __API_CMISS_VALUE_DERIVATIVE_MATRIX_H__

#include "api/cmiss_value.h"
#include "api/cmiss_variable.h"

Cmiss_value_id CREATE(Cmiss_value_derivative_matrix)(Cmiss_variable_id dependent_variable,
	int order, Cmiss_variable_id *independent_variables,
	int number_of_matrices, Cmiss_value_id *matrices);
/*******************************************************************************
LAST MODIFIED : 12 August 2003

DESCRIPTION :
Creates a Cmiss_value which contains a set of derivative matrices.
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
#endif /* __API_CMISS_VALUE_DERIVATIVE_MATRIX_H__ */
