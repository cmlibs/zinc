/*******************************************************************************
FILE : api/cmiss_value_derivative_matrix.h

LAST MODIFIED : 12 August 2003

DESCRIPTION :
The public interface to the Cmiss_value_derivative_matrix object.
==============================================================================*/
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
