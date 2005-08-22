/*******************************************************************************
FILE : computed_value_fe_value.h

LAST MODIFIED : 30 July 2003

DESCRIPTION :
computed_value types for FE_value, FE_value vector and FE_value_matrix
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
#if !defined (__CMISS_VALUE_FE_VALUE_H__)
#define __CMISS_VALUE_FE_VALUE_H__

#include "computed_variable/computed_value.h"

/*
Global functions
----------------
*/
int Cmiss_value_FE_value_set_type(Cmiss_value_id value,
	FE_value fe_value);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Makes <value> of type FE_value and sets its <fe_value>.
==============================================================================*/

PROTOTYPE_CMISS_VALUE_IS_TYPE_FUNCTION(FE_value);

int Cmiss_value_FE_value_get_type(Cmiss_value_id value,
	FE_value *fe_value_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <value> is of type FE_value, gets its <*fe_value_address>.
==============================================================================*/

int Cmiss_value_FE_value_vector_set_type(Cmiss_value_id value,
	int number_of_fe_values,FE_value *fe_value_vector);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Makes <value> of type FE_value_vector and sets its <number_of_fe_values> and
<fe_value_vector>.  After success, the <value> is responsible for DEALLOCATEing
<fe_value_vector>.
==============================================================================*/

PROTOTYPE_CMISS_VALUE_IS_TYPE_FUNCTION(FE_value_vector);

int Cmiss_value_FE_value_vector_get_type(Cmiss_value_id value,
	int *number_of_fe_values_address,FE_value **fe_value_vector_address);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
If <value> is of type FE_value_vector, gets its <*number_of_fe_values_address>
and <*fe_value_vector_address>.

The calling program must not DEALLOCATE the returned <*fe_value_vector_address>.
==============================================================================*/

int Cmiss_value_FE_value_matrix_set_type(Cmiss_value_id value,
	int number_of_rows,int number_of_columns,FE_value *fe_value_matrix);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Makes <value> of type FE_value_matrix and sets its <number_of_rows>,
<number_of_columns> and <fe_value_matrix> (column number varying fastest).
After success, the <value> is responsible for DEALLOCATEing <fe_value_matrix>.
==============================================================================*/

PROTOTYPE_CMISS_VALUE_IS_TYPE_FUNCTION(FE_value_matrix);

int Cmiss_value_FE_value_matrix_get_type(Cmiss_value_id value,
	int *number_of_rows_address,int *number_of_columns_address,
	FE_value **fe_value_matrix_address);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
If <value> is of type FE_value_matrix, gets its <*number_of_rows_address>,
<*number_of_columns_address> and <*fe_value_matrix_address>.

The calling program must not DEALLOCATE the returned <*fe_value_matrix_address>.
==============================================================================*/
#endif /* !defined (__CMISS_VALUE_FE_VALUE_H__) */
