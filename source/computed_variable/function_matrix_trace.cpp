//******************************************************************************
// FILE : function_matrix_trace.cpp
//
// LAST MODIFIED : 10 September 2004
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
bool Function_matrix_trace<Scalar>::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 10 September 2004
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
#endif // !defined (AIX)
