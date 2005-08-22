//******************************************************************************
// FILE : function_matrix.cpp
//
// LAST MODIFIED : 28 June 2005
//
// DESCRIPTION :
//==============================================================================
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

//#define DO_NOT_EXPLICITLY_INCLUDE_function_matrix_implementation

#if defined (DO_NOT_EXPLICITLY_INCLUDE_function_matrix_implementation)
#if defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_implementation.cpp"
#else // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix.hpp"
#endif // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#else // defined (DO_NOT_EXPLICITLY_INCLUDE_function_matrix_implementation)
#include "computed_variable/function_matrix.hpp"
#include "computed_variable/function_matrix_implementation.cpp"

template class Function_matrix<Scalar>;
#endif // defined (DO_NOT_EXPLICITLY_INCLUDE_function_matrix_implementation)

template<>
boost::intrusive_ptr< Function_matrix<Scalar> >
	Function_matrix<Scalar>::solve(
	const boost::intrusive_ptr< Function_matrix<Scalar> >& rhs)
//******************************************************************************
// LAST MODIFIED : 14 September 2004
//
// DESCRIPTION :
// A x = B
// n by n  n by m  n by m
//==============================================================================
{
	boost::intrusive_ptr< Function_matrix<Scalar> > result(0);
	Function_size_type number_of_rhss,size_A;

	if (this&&(0<(size_A=number_of_rows()))&&(number_of_columns()==size_A)&&
		rhs&&(rhs->number_of_rows()==size_A)&&
		(0<(number_of_rhss=rhs->number_of_columns())))
	{
		ublas::matrix<Scalar,ublas::column_major>
			A(size_A,size_A),X(size_A,number_of_rhss);
		std::vector<int> ipiv(size_A);

		A=values;
		X=rhs->values;
		lapack::gesv(A,ipiv,X);
		result=boost::intrusive_ptr< Function_matrix<Scalar> >(
			new Function_matrix<Scalar>(X));
	}

	return (result);
}

template<>
bool Function_matrix<Scalar>::determinant(Scalar& det)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
// Zero for a non-square matrix.
//==============================================================================
{
	bool result(false);
	Function_size_type size_A;

	if (this&&(0<(size_A=number_of_rows()))&&(number_of_columns()==size_A))
	{
		Function_size_type i;
		std::vector<int> ipiv(size_A);
		ublas::matrix<Scalar,ublas::column_major> A(size_A,size_A);

		A=values;
		lapack::getrf(A,ipiv);
		result=true;
		det=1;
		for (i=0;i<size_A;++i)
		{
			if (i+1==(Function_size_type)ipiv[i])
			{
				det *= A(i,i);
			}
			else
			{
				det *= -A(i,i);
			}
		}
	}

	return (result);
}

template<>
bool Function_matrix<Scalar>::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 4 March 2005
//
// DESCRIPTION :
//==============================================================================
{
#if defined (Function_matrix_INPUT_AND_OUTPUT_ARE_DISTINCT)
	bool result;
	boost::intrusive_ptr< Function_variable_matrix<Scalar> >
		atomic_dependent_variable,atomic_independent_variable;
#if defined (OLD_CODE)
	boost::intrusive_ptr< Function_variable_matrix_output<Scalar> >
		atomic_dependent_variable;
	boost::intrusive_ptr< Function_variable_matrix_input<Scalar> >
		atomic_independent_variable;
#endif // defined (OLD_CODE)

	result=false;
	if ((atomic_dependent_variable=boost::dynamic_pointer_cast<
		Function_variable_matrix_output<Scalar>,Function_variable>(
		atomic_variable))&&
		equivalent(Function_handle(this),atomic_dependent_variable->function())&&
		(1==atomic_dependent_variable->number_differentiable()))
	{
		result=true;
		if ((1==atomic_independent_variables.size())&&
			((atomic_independent_variable=boost::dynamic_pointer_cast<
			Function_variable_matrix_input<Scalar>,Function_variable>(
			atomic_independent_variables.front()))||
			(atomic_independent_variable=boost::dynamic_pointer_cast<
			Function_variable_matrix_output<Scalar>,Function_variable>(
			atomic_independent_variables.front())))&&
			equivalent(Function_handle(this),
			atomic_independent_variable->function())&&
			(atomic_dependent_variable->row()==atomic_independent_variable->row())&&
			(atomic_dependent_variable->column()==
			atomic_independent_variable->column()))
		{
			derivative=1;
		}
		else
		{
			derivative=0;
		}
	}
	else if ((atomic_dependent_variable=boost::dynamic_pointer_cast<
		Function_variable_matrix_input<Scalar>,Function_variable>(
		atomic_variable))&&
		equivalent(Function_handle(this),atomic_dependent_variable->function())&&
		(1==atomic_dependent_variable->number_differentiable()))
	{
		result=true;
		if ((1==atomic_independent_variables.size())&&
			(atomic_independent_variable=boost::dynamic_pointer_cast<
			Function_variable_matrix_input<Scalar>,Function_variable>(
			atomic_independent_variables.front()))&&
			equivalent(Function_handle(this),
			atomic_independent_variable->function())&&
			(atomic_dependent_variable->row()==atomic_independent_variable->row())&&
			(atomic_dependent_variable->column()==
			atomic_independent_variable->column()))
		{
			derivative=1;
		}
		else
		{
			derivative=0;
		}
	}
#else // defined (Function_matrix_INPUT_AND_OUTPUT_ARE_DISTINCT)
	bool result;
	boost::intrusive_ptr< Function_variable_matrix<Scalar> >
		atomic_dependent_variable,atomic_independent_variable;

	result=false;
	if ((atomic_dependent_variable=boost::dynamic_pointer_cast<
		Function_variable_matrix<Scalar>,Function_variable>(
		atomic_variable))&&
		equivalent(Function_handle(this),atomic_dependent_variable->function())&&
		(1==atomic_dependent_variable->number_differentiable()))
	{
		result=true;
		if ((1==atomic_independent_variables.size())&&
			(atomic_independent_variable=boost::dynamic_pointer_cast<
			Function_variable_matrix<Scalar>,Function_variable>(
			atomic_independent_variables.front()))&&
			equivalent(atomic_dependent_variable,atomic_independent_variable))
		{
			derivative=1;
		}
		else
		{
			derivative=0;
		}
	}
#endif // defined (Function_matrix_INPUT_AND_OUTPUT_ARE_DISTINCT)

	return (result);
}
