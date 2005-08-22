/*******************************************************************************
FILE : api/cmiss_function_matrix.h

LAST MODIFIED : 26 April 2004

DESCRIPTION :
The public interface to the Cmiss_function matrix object.
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
#ifndef __API_CMISS_FUNCTION_MATRIX_H__
#define __API_CMISS_FUNCTION_MATRIX_H__

#include "api/cmiss_function.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_function_id Cmiss_function_matrix_create(unsigned int number_of_rows,
	unsigned int number_of_columns,Scalar *values);
/*******************************************************************************
LAST MODIFIED : 24 February 2004

DESCRIPTION :
Creates a Cmiss_function matrix with the specified <number_of_rows>,
<number_of_columns> and <values>.  If <values> is NULL then the matrix is
initialized to zero.
==============================================================================*/

Scalar Cmiss_function_matrix_value(Cmiss_function_id function_matrix,
	unsigned int row,unsigned int column);
/*******************************************************************************
LAST MODIFIED : 3 February 2005

DESCRIPTION :
Returns the scalar value at the specified <row> and <column>.
==============================================================================*/

Cmiss_function_variable_id Cmiss_function_matrix_entry(
	Cmiss_function_id function_matrix,unsigned int row,unsigned int column);
/*******************************************************************************
LAST MODIFIED : 17 March 2004

DESCRIPTION :
Returns a variable that refers to the entry at the specified <row> and <column>.
If <row> is zero, then the variable refers to the <column>.  If <column> is
zero, then the variable refers to the <row>.  If <row> and <column> are both
zero, the variable refers to the whole matrix.
==============================================================================*/

int Cmiss_function_matrix_get_dimensions(Cmiss_function_id function_matrix,
	unsigned int *number_of_rows_address,unsigned int *number_of_columns_address);
/*******************************************************************************
LAST MODIFIED : 24 February 2004

DESCRIPTION :
Gets the <*number_of_rows_address> and <*number_of_columns_address> for the
<function_matrix>.  Returns a non-zero for success.
==============================================================================*/

Cmiss_function_id Cmiss_function_matrix_get_sub_matrix(
	Cmiss_function_id function_matrix,unsigned int row_low,unsigned int row_high,
	unsigned int column_low,unsigned int column_high);
/*******************************************************************************
LAST MODIFIED : 24 February 2004

DESCRIPTION :
Returns a Cmiss_function matrix which is the specified sub-matrix of
<function_matrix>.
==============================================================================*/

Cmiss_function_id Cmiss_function_matrix_solve(Cmiss_function_id function_matrix,
	Cmiss_function_id function_rhs);
/*******************************************************************************
LAST MODIFIED : 24 February 2004

DESCRIPTION :
Returns the solution of the linear system <function_matrix>*x=<function_rhs>.
<function_rhs> should be a matrix or a vector.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_FUNCTION_MATRIX_H__ */
