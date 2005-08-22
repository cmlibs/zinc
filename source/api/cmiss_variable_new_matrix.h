/*******************************************************************************
FILE : api/cmiss_variable_new_matrix.h

LAST MODIFIED : 6 November 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new matrix object.
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
#ifndef __API_CMISS_VARIABLE_NEW_MATRIX_H__
#define __API_CMISS_VARIABLE_NEW_MATRIX_H__

#include "api/cmiss_variable_new.h"

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_variable_new_id Cmiss_variable_new_matrix_create(
	unsigned int number_of_rows,unsigned int number_of_columns,Scalar *values);
/*******************************************************************************
LAST MODIFIED : 24 October 2003

DESCRIPTION :
Creates a Cmiss_variable_new matrix with the specified <number_of_rows>,
<number_of_columns> and <values>.  If <values> is NULL then the matrix is
initialized to zero.
==============================================================================*/

int Cmiss_variable_new_matrix_get_dimensions(
	Cmiss_variable_new_id variable_matrix,unsigned int *number_of_rows_address,
	unsigned int *number_of_columns_address);
/*******************************************************************************
LAST MODIFIED : 6 November 2003

DESCRIPTION :
Gets the <*number_of_rows_address> and <*number_of_columns_address> for the
<variable_matrix>.
==============================================================================*/

Cmiss_variable_new_id Cmiss_variable_new_matrix_get_sub_matrix(
	Cmiss_variable_new_id variable_matrix,unsigned int row_low,
	unsigned int row_high,unsigned int column_low,unsigned int column_high);
/*******************************************************************************
LAST MODIFIED : 6 November 2003

DESCRIPTION :
Returns a Cmiss_variable_new matrix which is the specified sub-matrix of
<variable_matrix>.
==============================================================================*/

Cmiss_variable_new_input_id Cmiss_variable_new_input_matrix_values(
	Cmiss_variable_new_id variable_matrix,unsigned int number_of_indices,
	unsigned int *row_indices,unsigned int *column_indices);
/*******************************************************************************
LAST MODIFIED : 24 October 2003

DESCRIPTION :
Returns the values input made up of the specified indices for the
<variable_matrix>.  If <number_of_indices> is zero or <row_indices> is NULL or
<column_indices> is NULL then the input refers to all values.
==============================================================================*/

Cmiss_variable_new_id Cmiss_variable_new_matrix_solve(
	Cmiss_variable_new_id variable_matrix,Cmiss_variable_new_id variable_rhs);
/*******************************************************************************
LAST MODIFIED : 11 December 2003

DESCRIPTION :
Returns the solution of the linear system <variable_matrix>*x=<variable_rhs>.
<variable_rhs> should be a matrix or a vector.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_VARIABLE_NEW_MATRIX_H__ */
