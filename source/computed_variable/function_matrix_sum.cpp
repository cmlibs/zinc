//******************************************************************************
// FILE : function_matrix_sum.cpp
//
// LAST MODIFIED : 1 October 2004
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
Function_handle Function_variable_matrix_sum<Scalar>::evaluate_derivative(
	std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 1 October 2004
//
// DESCRIPTION :
// ???DB.  To be done
//==============================================================================
{
	boost::intrusive_ptr< Function_matrix_sum<Scalar> > function_matrix_sum;
	Function_handle result(0);
	Function_size_type order;

	if ((function_matrix_sum=boost::dynamic_pointer_cast<
		Function_matrix_sum<Scalar>,Function>(function()))&&
		(0<(order=independent_variables.size())))
	{
		Function_size_type number_of_columns,number_of_dependent_values,
			number_of_independent_values,number_of_rows;
		boost::intrusive_ptr< Function_matrix<Scalar> > derivative_1,derivative_2,
			summand_1,summand_2;

		if ((summand_1=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_sum->summand_1_private->evaluate()))&&
			(summand_2=boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
			function_matrix_sum->summand_2_private->evaluate()))&&
			(row_private<=(number_of_rows=summand_1->number_of_rows()))&&
			(number_of_rows==summand_2->number_of_rows())&&
			(column_private<=(number_of_columns=summand_1->number_of_columns()))&&
			(number_of_columns==summand_2->number_of_columns())&&
			(derivative_1=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_sum->summand_1_private->
			evaluate_derivative(independent_variables)))&&
			(derivative_2=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_sum->summand_2_private->
			evaluate_derivative(independent_variables)))&&
			(number_of_rows*number_of_columns==
			(number_of_dependent_values=derivative_1->number_of_rows()))&&
			(number_of_dependent_values==derivative_2->number_of_rows())&&
			(derivative_1->number_of_columns()==(number_of_independent_values=
			derivative_2->number_of_columns())))
		{
			Function_size_type dependent_row,i,j;

			if (0==row_private)
			{
				if (0==column_private)
				{
					for (i=1;i<=number_of_dependent_values;i++)
					{
						for (j=1;j<=number_of_independent_values;j++)
						{
							(*derivative_1)(i,j) += (*derivative_2)(i,j);
						}
					}
					result=derivative_1;
				}
				else
				{
					Matrix result_matrix(number_of_rows,number_of_independent_values);

					dependent_row=column_private;
					for (i=0;i<number_of_rows;i++)
					{
						for (j=1;j<=number_of_independent_values;j++)
						{
							result_matrix(i,j-1)=(*derivative_1)(dependent_row,j)+
								(*derivative_2)(dependent_row,j);
						}
						dependent_row += number_of_columns;
					}
					result=Function_handle(new Function_matrix<Scalar>(result_matrix));
				}
			}
			else
			{
				if (0==column_private)
				{
					Matrix result_matrix(number_of_columns,number_of_independent_values);

					dependent_row=(row_private-1)*number_of_columns+1;
					for (i=0;i<number_of_columns;i++)
					{
						for (j=1;j<=number_of_independent_values;j++)
						{
							result_matrix(i,j-1)=(*derivative_1)(dependent_row,j)+
								(*derivative_2)(dependent_row,j);
						}
						dependent_row++;
					}
					result=Function_handle(new Function_matrix<Scalar>(result_matrix));
				}
				else
				{
					Matrix result_matrix(1,number_of_independent_values);

					dependent_row=(row_private-1)*number_of_columns+column_private;
					for (j=1;j<=number_of_independent_values;j++)
					{
						result_matrix(0,j-1)=(*derivative_1)(dependent_row,j)+
							(*derivative_2)(dependent_row,j);
					}
					result=Function_handle(new Function_matrix<Scalar>(result_matrix));
				}
			}
		}
	}

	return (result);
}

template<>
bool Function_matrix_sum<Scalar>::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 1 October 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_sum<Scalar> >
		atomic_variable_matrix_sum;

	result=false;
	if ((atomic_variable_matrix_sum=boost::dynamic_pointer_cast<
		Function_variable_matrix_sum<Scalar>,Function_variable>(
		atomic_variable))&&equivalent(Function_handle(this),
		atomic_variable_matrix_sum->function())&&
		(0<atomic_variable_matrix_sum->row())&&
		(0<atomic_variable_matrix_sum->column())&&
		(0<atomic_independent_variables.size()))
	{
		boost::intrusive_ptr< Function_matrix<Scalar> > derivative_matrix=
			boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
			atomic_variable_matrix_sum->evaluate_derivative(
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
#endif // defined (OLD_CODE)
#endif // !defined (AIX)
