/*******************************************************************************
FILE : api/cmiss_variable_new_matrix.cpp

LAST MODIFIED : 2 February 2004

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

#include <new>
#include "api/cmiss_variable_new_matrix.h"
#include "computed_variable/variable_matrix.hpp"

/*
Global functions
----------------
*/

Cmiss_variable_new_id Cmiss_variable_new_matrix_create(
	unsigned int number_of_rows,unsigned int number_of_columns,Scalar *values)
/*******************************************************************************
LAST MODIFIED : 26 November 2003

DESCRIPTION :
Creates a Cmiss_variable_new matrix with the specified <number_of_rows>,
<number_of_columns> and <values>.  If <values> is NULL then the matrix is
initialized to zero.
==============================================================================*/
{
	Cmiss_variable_new_id result;

	result=0;
	if (0<number_of_rows*number_of_columns)
	{
		Matrix values_matrix(number_of_rows,number_of_columns);
		Variable_size_type i,j,k;

		if (values)
		{
			k=0;
			for (i=0;i<number_of_rows;i++)
			{
				for (j=0;j<number_of_columns;j++)
				{
					values_matrix(i,j)=values[k];
					k++;
				}
			}
		}
		else
		{
			for (i=0;i<number_of_rows;i++)
			{
				for (j=0;j<number_of_columns;j++)
				{
					values_matrix(i,j)=(Scalar)0;
				}
			}
		}
#if defined (USE_SMART_POINTER)
		result=reinterpret_cast<Cmiss_variable_new_id>(new Variable_handle(
			new Variable_matrix(values_matrix)));
#else /* defined (USE_SMART_POINTER) */
		result=reinterpret_cast<Cmiss_variable_new_id>(Variable_handle(
			new Variable_matrix(values_matrix)));
#endif /* defined (USE_SMART_POINTER) */
	}

	return (result);
}

int Cmiss_variable_new_matrix_get_dimensions(
	Cmiss_variable_new_id variable_matrix,unsigned int *number_of_rows_address,
	unsigned int *number_of_columns_address)
/*******************************************************************************
LAST MODIFIED : 6 November 2003

DESCRIPTION :
Gets the <*number_of_rows_address> and <*number_of_columns_address> for the
<variable_matrix>.
==============================================================================*/
{
	int return_code;
#if defined (USE_SMART_POINTER)
	Variable_matrix_handle *variable_matrix_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable_matrix *variable_matrix_address;
#endif /* defined (USE_SMART_POINTER) */

	return_code=0;
	if (
#if defined (USE_SMART_POINTER)
		variable_matrix_handle_address=
		reinterpret_cast<Variable_matrix_handle *>(variable_matrix)
#else /* defined (USE_SMART_POINTER) */
		variable_matrix_address=reinterpret_cast<Variable_matrix *>(variable_matrix)
#endif /* defined (USE_SMART_POINTER) */
		)
	{
		return_code=1;
		if (number_of_rows_address)
		{
			*number_of_rows_address=
#if defined (USE_SMART_POINTER)
				((*variable_matrix_handle_address)->number_of_rows)()
#else /* defined (USE_SMART_POINTER) */
				(variable_matrix_address->number_of_rows)()
#endif /* defined (USE_SMART_POINTER) */
			;
		}
		if (number_of_columns_address)
		{
			*number_of_columns_address=
#if defined (USE_SMART_POINTER)
				((*variable_matrix_handle_address)->number_of_columns)()
#else /* defined (USE_SMART_POINTER) */
				(variable_matrix_address->number_of_columns)()
#endif /* defined (USE_SMART_POINTER) */
			;
		}
	}

	return (return_code);
}

Cmiss_variable_new_id Cmiss_variable_new_matrix_get_sub_matrix(
	Cmiss_variable_new_id variable_matrix,unsigned int row_low,
	unsigned int row_high,unsigned int column_low,unsigned int column_high)
/*******************************************************************************
LAST MODIFIED : 6 November 2003

DESCRIPTION :
Returns a Cmiss_variable_new matrix which is the specified sub-matrix of
<variable_matrix>.
==============================================================================*/
{
	Cmiss_variable_new_id result;
#if defined (USE_SMART_POINTER)
	Variable_matrix_handle *variable_matrix_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable_derivative_matrix *variable_matrix_address;
#endif /* defined (USE_SMART_POINTER) */

	result=0;
	if (
#if defined (USE_SMART_POINTER)
		variable_matrix_handle_address=
		reinterpret_cast<Variable_matrix_handle *>(variable_matrix)
#else /* defined (USE_SMART_POINTER) */
		variable_matrix_address=reinterpret_cast<Variable_matrix *>(variable_matrix)
#endif /* defined (USE_SMART_POINTER) */
		)
	{
		result=reinterpret_cast<Cmiss_variable_new_id>(
#if defined (USE_SMART_POINTER)
			new Variable_handle(((*variable_matrix_handle_address)->sub_matrix)(
			row_low,row_high,column_low,column_high))
#else /* defined (USE_SMART_POINTER) */
			(variable_matrix_address->sub_matrix)(row_low,row_high,column_low,
			column_high)
#endif /* defined (USE_SMART_POINTER) */
			);
	}

	return (result);
}

Cmiss_variable_new_input_id Cmiss_variable_new_input_matrix_values(
	Cmiss_variable_new_id variable_matrix,unsigned int number_of_indices,
	unsigned int *row_indices,unsigned int *column_indices)
/*******************************************************************************
LAST MODIFIED : 2 February 2004

DESCRIPTION :
Returns the values input made up of the specified indices for the
<variable_matrix>.  If <number_of_indices> is zero or <row_indices> is NULL or
<column_indices> is NULL then the input refers to all values.
==============================================================================*/
{
	Cmiss_variable_new_input_id result;
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		input_values;
	Variable_matrix_handle variable_matrix_handle;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
#endif /* defined (USE_SMART_POINTER) */

	result=0;
	if (
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(
		variable_matrix))&&
		(variable_matrix_handle=boost::dynamic_pointer_cast<Variable_matrix,
		Variable>(*variable_handle_address))
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(variable_matrix))&&
		(variable_matrix_handle=dynamic_cast<Variable_matrix *>(variable_address))
#endif /* defined (USE_SMART_POINTER) */
		)
	{
		if ((0<number_of_indices)&&row_indices&&column_indices)
		{
			boost::numeric::ublas::vector< std::pair<Variable_size_type,
				Variable_size_type> > indices_matrix(number_of_indices);
			unsigned int i;

			for (i=0;i<number_of_indices;i++)
			{
				indices_matrix[i].first=row_indices[i];
				indices_matrix[i].second=column_indices[i];
			}
			input_values=variable_matrix_handle->input_values(indices_matrix);
		}
		else
		{
			input_values=variable_matrix_handle->input_values();
		}
		result=reinterpret_cast<Cmiss_variable_new_input_id>(
#if defined (USE_SMART_POINTER)
			new
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			(
#endif /* defined (USE_SMART_POINTER) */
			input_values
#if defined (USE_SMART_POINTER)
			)
#endif /* defined (USE_SMART_POINTER) */
			);
	}

	return (result);
}

Cmiss_variable_new_id Cmiss_variable_new_matrix_solve(
	Cmiss_variable_new_id variable_matrix,Cmiss_variable_new_id variable_rhs)
/*******************************************************************************
LAST MODIFIED : 12 December 2003

DESCRIPTION :
Returns the solution of the linear system <variable_matrix>*x=<variable_rhs>.
<variable_rhs> should be a matrix or a vector.
==============================================================================*/
{
	Cmiss_variable_new_id result;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_matrix_handle_address;
	Variable_handle *variable_rhs_handle_address;
#else
	Variable *variable_matrix_address;
#endif /* defined (USE_SMART_POINTER) */
	Variable_matrix_handle variable_matrix_handle;
	Variable_handle variable_rhs_handle;

	result=0;
	if (
#if defined (USE_SMART_POINTER)
		(variable_matrix_handle_address=
		reinterpret_cast<Variable_handle *>(variable_matrix))&&
		(*variable_matrix_handle_address)&&
		(variable_matrix_handle=boost::dynamic_pointer_cast<Variable_matrix,
		Variable>(*variable_matrix_handle_address))&&
		(variable_rhs_handle_address=
		reinterpret_cast<Variable_handle *>(variable_rhs))&&
		(variable_rhs_handle= *variable_rhs_handle_address)
#else /* defined (USE_SMART_POINTER) */
		(variable_matrix_address=
		reinterpret_cast<Variable *>(variable_matrix))&&
		(variable_matrix_handle=dynamic_cast<Variable_matrix *>(
		variable_matrix_address))&&
		(variable_rhs_address=reinterpret_cast<Variable *>(variable_rhs))
#endif /* defined (USE_SMART_POINTER) */
		)
	{
		Variable_matrix_handle variable_matrix_rhs_handle;
		Variable_vector_handle variable_vector_rhs_handle;

		if (variable_matrix_rhs_handle=
#if defined (USE_SMART_POINTER)
			boost::dynamic_pointer_cast<Variable_matrix,Variable>(variable_rhs_handle)
#else /* defined (USE_SMART_POINTER) */
			dynamic_cast<Variable_matrix *>(variable_rhs_handle)
#endif /* defined (USE_SMART_POINTER) */
			)
		{
			result=reinterpret_cast<Cmiss_variable_new_id>(
#if defined (USE_SMART_POINTER)
				new Variable_handle(
#endif /* defined (USE_SMART_POINTER) */
				variable_matrix_handle->solve(variable_matrix_rhs_handle)
#if defined (USE_SMART_POINTER)
				)
#endif /* defined (USE_SMART_POINTER) */
				);
		}
		else if (variable_vector_rhs_handle=
#if defined (USE_SMART_POINTER)
			boost::dynamic_pointer_cast<Variable_vector,Variable>(variable_rhs_handle)
#else /* defined (USE_SMART_POINTER) */
			dynamic_cast<Variable_vector *>(variable_rhs_handle)
#endif /* defined (USE_SMART_POINTER) */
			)
		{
			result=reinterpret_cast<Cmiss_variable_new_id>(
#if defined (USE_SMART_POINTER)
				new Variable_handle(
#endif /* defined (USE_SMART_POINTER) */
				variable_matrix_handle->solve(variable_vector_rhs_handle)
#if defined (USE_SMART_POINTER)
				)
#endif /* defined (USE_SMART_POINTER) */
				);
		}
		else
		{
			result=reinterpret_cast<Cmiss_variable_new_id>(
#if defined (USE_SMART_POINTER)
				new Variable_handle(
#endif /* defined (USE_SMART_POINTER) */
				variable_matrix_handle->solve(variable_rhs_handle)
#if defined (USE_SMART_POINTER)
				)
#endif /* defined (USE_SMART_POINTER) */
				);
		}
	}

	return (result);
}
