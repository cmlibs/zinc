//******************************************************************************
// FILE : function_matrix_product.cpp
//
// LAST MODIFIED : 8 September 2004
//
// DESCRIPTION :
//==============================================================================

#if defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_product_implementation.cpp"
#else // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_product.hpp"
#endif // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)

#if !defined (AIX)
#include "computed_variable/function_derivative_matrix.hpp"
#include "computed_variable/function_variable_composite.hpp"

template<>
bool Function_matrix_product<Scalar>::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 8 September 2004
//
// DESCRIPTION :
// ???DB.  To be done
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_product<Scalar> >
		atomic_variable_matrix_product;
	Function_size_type column,order,row;

	result=false;
	if ((atomic_variable_matrix_product=boost::dynamic_pointer_cast<
		Function_variable_matrix_product<Scalar>,Function_variable>(
		atomic_variable))&&equivalent(Function_handle(this),
		atomic_variable_matrix_product->function())&&
		(0<(row=atomic_variable_matrix_product->row()))&&
		(0<(column=atomic_variable_matrix_product->column()))&&
		(0<(order=atomic_independent_variables.size())))
	{
		bool valid;
		boost::intrusive_ptr< Function_variable_matrix<Scalar> > temp_variable;
		Function_size_type i,number_in_sum=multiplier_private->number_of_columns();
		Function_variable_handle intermediate_variable(
			new Function_variable_composite((*multiplier_private)(row,0),
			(*multiplicand_private)(0,column)));
		Matrix Df(1,2*number_in_sum),D2f(1,4*number_in_sum*number_in_sum);
		std::list<Function_variable_handle> intermediate_independent_variables(
			order,intermediate_variable);
		std::list<Matrix> matrices(0);

		i=1;
		valid=true;
		while (valid&&(i<=number_in_sum))
		{
			valid=(temp_variable=(*multiplier_private)(row,i))&&
				(temp_variable->evaluate())&&
				(temp_variable->get_entry(Df(0,number_in_sum+i-1)))&&
				(temp_variable=(*multiplicand_private)(i,column))&&
				(temp_variable->evaluate())&&(temp_variable->get_entry(Df(0,i-1)));
			i++;
		}
		if (valid&&(1<order))
		{
			Function_size_type column_1,column_2,step;

			D2f.clear();
			column_1=number_in_sum;
			column_2=2*number_in_sum*number_in_sum;
			step=2*number_in_sum+1;
			for (i=0;i<number_in_sum;i++)
			{
				D2f(0,column_1)=1;
				column_1 += step;
				D2f(0,column_2)=1;
				column_2 += step;
			}
		}
		if (valid)
		{
			Function_derivative_matrix_handle derivative_f(0),derivative_g(0);
			std::list<Matrix>::iterator matrix_iterator,matrix_iterator_end;

			for (i=0;i<order;i++)
			{
				matrices.push_back(Df);
				matrix_iterator_end=matrices.end();
				matrix_iterator_end--;
				matrix_iterator=matrices.begin();
				while (matrix_iterator!=matrix_iterator_end)
				{
					if (2*number_in_sum==matrix_iterator->size2())
					{
						matrices.push_back(D2f);
					}
					else
					{
						Matrix zero_matrix(1,(matrix_iterator->size2())*2*number_in_sum);

						zero_matrix.clear();
						matrices.push_back(zero_matrix);
					}
					matrix_iterator++;
				}
			}
			derivative_f=new Function_derivative_matrix(atomic_variable,
				intermediate_independent_variables,matrices);
			derivative_g=new Function_derivative_matrix(intermediate_variable,
				atomic_independent_variables);
			if (derivative_f&&derivative_g)
			{
				Function_derivative_matrix_handle derivative_matrix=
					Function_derivative_matrix_compose(atomic_variable,derivative_f,
					derivative_g);
				boost::intrusive_ptr< Function_matrix<Scalar> > function_matrix;

				if (derivative_matrix&&(function_matrix=boost::dynamic_pointer_cast<
					Function_matrix<Scalar>,Function>(derivative_matrix->matrix(
					atomic_independent_variables))))
				{
					derivative=(*function_matrix)(1,1);
					result=true;
				}
			}
		}
	}

	return (result);
}
#endif // !defined (AIX)
