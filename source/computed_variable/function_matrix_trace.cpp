//******************************************************************************
// FILE : function_matrix_trace.cpp
//
// LAST MODIFIED : 18 October 2004
//
// DESCRIPTION :
//==============================================================================

#if defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_trace_implementation.cpp"
#else // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_trace.hpp"
#endif // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)

#if !defined (AIX)
template<>
Function_handle Function_variable_matrix_trace<Scalar>::evaluate_derivative(
	std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 18 October 2004
//
// DESCRIPTION :
// ???DB.  To be done
//==============================================================================
{
	boost::intrusive_ptr< Function_matrix_trace<Scalar> > function_matrix_trace;
	Function_handle result(0);

	if ((function_matrix_trace=boost::dynamic_pointer_cast<
		Function_matrix_trace<Scalar>,Function>(function()))&&
		(0<independent_variables.size()))
	{
		Function_size_type number_of_independent_values,number_of_rows;
		boost::intrusive_ptr< Function_matrix<Scalar> > derivative,matrix;

		if ((matrix=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_trace->matrix_private->evaluate()))&&
			(matrix->number_of_columns()==(number_of_rows=matrix->number_of_rows()))&&
			(derivative=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_trace->matrix_private->evaluate_derivative(
			independent_variables)))&&(number_of_rows*number_of_rows==
			derivative->number_of_rows())&&
			(0<(number_of_independent_values=derivative->number_of_columns())))
		{
			Function_size_type dependent_row,i,j;
			Matrix result_matrix(1,number_of_independent_values);
			Scalar sum;

			for (j=1;j<=number_of_independent_values;j++)
			{
				sum=0;
				dependent_row=1;
				for (i=0;i<number_of_rows;i++)
				{
					sum += (*derivative)(dependent_row,j);
					dependent_row += number_of_rows+1;
				}
				result_matrix(0,j-1)=sum;
			}
			result=Function_handle(new Function_matrix<Scalar>(result_matrix));
		}
	}

	return (result);
}

template<>
bool Function_matrix_trace<Scalar>::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 1 October 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_trace<Scalar> >
		atomic_variable_matrix_trace;

	result=false;
	if ((atomic_variable_matrix_trace=boost::dynamic_pointer_cast<
		Function_variable_matrix_trace<Scalar>,Function_variable>(
		atomic_variable))&&equivalent(Function_handle(this),
		atomic_variable_matrix_trace->function())&&
		(0<atomic_variable_matrix_trace->row())&&
		(0<atomic_variable_matrix_trace->column())&&
		(0<atomic_independent_variables.size()))
	{
		boost::intrusive_ptr< Function_matrix<Scalar> > derivative_matrix=
			boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
			atomic_variable_matrix_trace->evaluate_derivative(
			atomic_independent_variables));

		if (derivative_matrix)
		{
			result=true;
			derivative=(*derivative_matrix)(1,1);
		}
	}

	return (result);
}

#if defined (OLD_CODE)
template<>
bool Function_matrix_trace<Scalar>::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 1 October 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_trace<Scalar> >
		atomic_variable_matrix_trace;

	result=false;
	if ((atomic_variable_matrix_trace=boost::dynamic_pointer_cast<
		Function_variable_matrix_trace<Scalar>,Function_variable>(
		atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_matrix_trace->function())&&
		(1==atomic_variable_matrix_trace->row())&&
		(1==atomic_variable_matrix_trace->column()))
	{
		boost::intrusive_ptr< Function_variable_matrix<Scalar> > temp_variable;
		Function_handle function;
		Function_size_type i,number_of_rows;
		Scalar sum,value;

		sum=0;
		number_of_rows=matrix_private->number_of_rows();
		i=1;
		while ((i<=number_of_rows)&&(temp_variable=(*matrix_private)(i,i))&&
			(function=temp_variable->function())&&(function->evaluate_derivative)(
			value,temp_variable,atomic_independent_variables))
		{
			sum += value;
			i++;
		}
		if (i>number_of_rows)
		{
			result=true;
			derivative=sum;
		}
	}

	return (result);
}
#endif // defined (OLD_CODE)
#endif // !defined (AIX)
