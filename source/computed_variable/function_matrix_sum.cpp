//******************************************************************************
// FILE : function_matrix_sum.cpp
//
// LAST MODIFIED : 6 September 2004
//
// DESCRIPTION :
//==============================================================================

#if defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_sum_implementation.cpp"
#else // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_sum.hpp"
#endif // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)

#if !defined (AIX)
template<>
bool Function_matrix_sum<Scalar>::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 1 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_sum<Scalar> >
		atomic_variable_matrix_sum;
	Function_size_type column,row;

	result=false;
	if ((atomic_variable_matrix_sum=boost::dynamic_pointer_cast<
		Function_variable_matrix_sum<Scalar>,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_matrix_sum->function())&&
		(0<(row=atomic_variable_matrix_sum->row()))&&
		(0<(column=atomic_variable_matrix_sum->column())))
	{
		boost::intrusive_ptr< Function_variable_matrix<Scalar> > temp_variable;
		Function_handle function;
		Scalar value_1,value_2;

		if ((temp_variable=(*summand_1_private)(row,column))&&
			(function=temp_variable->function())&&(function->evaluate_derivative)(
			value_1,temp_variable,atomic_independent_variables)&&
			(temp_variable=(*summand_2_private)(row,column))&&
			(function=temp_variable->function())&&(function->evaluate_derivative)(
			value_2,temp_variable,atomic_independent_variables))
		{
			result=true;
			derivative=value_1+value_2;
		}
	}

	return (result);
}
#endif // !defined (AIX)
