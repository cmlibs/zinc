/*******************************************************************************
FILE : computed_value_matrix.h

LAST MODIFIED : 16 July 2003

DESCRIPTION :
Implements a matrix computed value.
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
#if !defined (__CMISS_VALUE_MATRIX_H__)
#define __CMISS_VALUE_MATRIX_H__

#include "computed_variable/computed_value.h"
#include "matrix/matrix.h"

/*
Global functions
----------------
*/
int Cmiss_value_matrix_set_type(Cmiss_value_id value,struct Matrix *matrix);
/*******************************************************************************
LAST MODIFIED : 4 May 2003

DESCRIPTION :
Makes <value> of type matrix and sets its <matrix>.  After success, the <value>
is responsible for DESTROYing <matrix>.
==============================================================================*/

PROTOTYPE_CMISS_VALUE_IS_TYPE_FUNCTION(matrix);

int Cmiss_value_matrix_get_type(Cmiss_value_id value,
	struct Matrix **matrix_address);
/*******************************************************************************
LAST MODIFIED : 4 May 2003

DESCRIPTION :
If <value> is of type matrix, gets its <*matrix_address>.

The calling program must not DESTROY the returned <*matrix_address>.
==============================================================================*/

int Cmiss_value_matrix_get_dimensions(Cmiss_value_id matrix,
	int *number_of_rows_address,int *number_of_columns_address);
/*******************************************************************************
LAST MODIFIED : 7 May 2003

DESCRIPTION :
Gets the <*number_of_rows_address> and <*number_of_columns_address> for the
<matrix>.
==============================================================================*/

int Cmiss_value_matrix_get_value(Cmiss_value_id matrix,int row_number,
	int column_number,Matrix_value *value_address);
/*******************************************************************************
LAST MODIFIED : 7 May 2003

DESCRIPTION :
Gets the <*value_address> at the <row_number> and <column_number> of the
<matrix>.
==============================================================================*/

int Cmiss_value_matrix_set_value(Cmiss_value_id matrix,int row_number,
	int column_number,Matrix_value value);
/*******************************************************************************
LAST MODIFIED : 7 May 2003

DESCRIPTION :
Sets the <value> at the <row_number> and <column_number> of the <matrix>.
==============================================================================*/

Cmiss_value_id Cmiss_value_matrix_get_submatrix(Cmiss_value_id matrix,
	int row_low,int row_high,int column_low,int column_high);
/*******************************************************************************
LAST MODIFIED : 11 May 2003

DESCRIPTION :
Returns a Cmiss value which is the specified sub-matrix of <matrix>.
==============================================================================*/

int Cmiss_value_matrix_set_submatrix(Cmiss_value_id matrix,int row_low,
	int row_high,int column_low,int column_high,Cmiss_value_id submatrix);
/*******************************************************************************
LAST MODIFIED : 16 July 2003

DESCRIPTION :
Copies the <submatrix> into the specified sub-matrix of <matrix>.
==============================================================================*/
#endif /* !defined (__CMISS_VALUE_MATRIX_H__) */
