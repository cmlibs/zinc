//******************************************************************************
// FILE : function_matrix_resize.cpp
//
// LAST MODIFIED : 14 January 2005
//
// DESCRIPTION :
//==============================================================================

#if defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_resize_implementation.cpp"
#else // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_resize.hpp"
#endif // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)

#if !defined (AIX)
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#include "computed_variable/function_derivative.hpp"
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

template<>
Function_handle Function_variable_matrix_resize<Scalar>::evaluate_derivative(
	std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 14 January 2005
//
// DESCRIPTION :
//==============================================================================
{
	boost::intrusive_ptr< Function_matrix_resize<Scalar> >
		function_matrix_resize;
	Function_handle result(0);

	if ((function_matrix_resize=boost::dynamic_pointer_cast<
		Function_matrix_resize<Scalar>,Function>(function()))&&
		(0<independent_variables.size()))
	{
		Function_size_type number_of_columns,number_of_independent_values,
			number_of_rows,size;
		boost::intrusive_ptr< Function_matrix<Scalar> > derivative,matrix;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_derivatnew_handle temp_function;
		Function_variable_handle temp_variable;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

		if (
#if defined (EVALUATE_RETURNS_VALUE)
			(matrix=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_resize->matrix_private->evaluate()))&&
#else // defined (EVALUATE_RETURNS_VALUE)
			(function_matrix_resize->matrix_private->evaluate)()&&
			(matrix=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_resize->matrix_private->get_value()))&&
#endif // defined (EVALUATE_RETURNS_VALUE)
			(0<(size=(matrix->number_of_columns())*(matrix->number_of_rows())))&&
			(0<(number_of_columns=
			function_matrix_resize->number_of_columns_private))&&
			(0==size%number_of_columns)&&
			(row_private<=(number_of_rows=size/number_of_columns))&&
			(column_private<=number_of_columns)&&
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(derivative=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_resize->matrix_private->
			evaluate_derivative(independent_variables)))&&
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(temp_function=boost::dynamic_pointer_cast<Function_derivatnew,
			Function>(function_matrix_resize->matrix_private->derivative(
			independent_variables)))&&(temp_variable=temp_function->output())&&
			(temp_variable->evaluate())&&(temp_variable=temp_function->matrix(
			independent_variables))&&(derivative=boost::dynamic_pointer_cast<
			Function_matrix<Scalar>,Function>(temp_variable->get_value()))&&
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(number_of_rows*number_of_columns==
			derivative->number_of_rows()))
		{
			Function_size_type dependent_row,i,j;

			number_of_independent_values=derivative->number_of_columns();
			if (0==row_private)
			{
				if (0==column_private)
				{
					result=derivative;
				}
				else
				{
					Matrix result_matrix(number_of_rows,number_of_independent_values);

					dependent_row=column_private;
					for (i=0;i<number_of_rows;i++)
					{
						for (j=1;j<=number_of_independent_values;j++)
						{
							result_matrix(i,j-1)=(*derivative)(dependent_row,j);
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
							result_matrix(i,j-1)=(*derivative)(dependent_row,j);
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
						result_matrix(0,j-1)=(*derivative)(dependent_row,j);
					}
					result=Function_handle(new Function_matrix<Scalar>(result_matrix));
				}
			}
		}
	}

	return (result);
}

template<>
bool Function_matrix_resize<Scalar>::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 7 October 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_resize<Scalar> >
		atomic_variable_matrix_resize;

	result=false;
	if ((atomic_variable_matrix_resize=boost::dynamic_pointer_cast<
		Function_variable_matrix_resize<Scalar>,Function_variable>(
		atomic_variable))&&equivalent(Function_handle(this),
		atomic_variable_matrix_resize->function())&&
		(0<atomic_variable_matrix_resize->row())&&
		(0<atomic_variable_matrix_resize->column())&&
		(0<atomic_independent_variables.size()))
	{
		boost::intrusive_ptr< Function_matrix<Scalar> > derivative_matrix=
			boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
			atomic_variable_matrix_resize->evaluate_derivative(
			atomic_independent_variables));

		if (derivative_matrix)
		{
			result=true;
			derivative=(*derivative_matrix)(1,1);
		}
	}

	return (result);
}
#endif // !defined (AIX)
