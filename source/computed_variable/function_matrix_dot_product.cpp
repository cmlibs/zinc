//******************************************************************************
// FILE : function_matrix_dot_product.cpp
//
// LAST MODIFIED : 12 November 2004
//
// DESCRIPTION :
//==============================================================================

#if defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_dot_product_implementation.cpp"
#else // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_dot_product.hpp"
#endif // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)

#if !defined (AIX)
#include "computed_variable/function_derivative_matrix.hpp"
#include "computed_variable/function_variable_composite.hpp"

template<>
Function_handle
	Function_variable_matrix_dot_product<Scalar>::evaluate_derivative(
	std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 12 November 2004
//
// DESCRIPTION :
// ???DB.  To be done
//==============================================================================
{
	boost::intrusive_ptr< Function_matrix_dot_product<Scalar> >
		function_matrix_dot_product;
	Function_handle result(0);
	Function_size_type order;

	if ((function_matrix_dot_product=boost::dynamic_pointer_cast<
		Function_matrix_dot_product<Scalar>,Function>(function()))&&
		(0<(order=independent_variables.size())))
	{
		Function_size_type number_of_columns,number_of_rows;
		boost::intrusive_ptr< Function_matrix<Scalar> > variable_1,variable_2;

		if ((variable_1=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_dot_product->variable_1_private->evaluate()))&&
			(variable_2=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_dot_product->variable_2_private->
			evaluate()))&&(0<(number_of_rows=variable_1->number_of_rows()))&&
			(0<(number_of_columns=variable_1->number_of_columns()))&&
			(number_of_rows==variable_2->number_of_rows())&&
			(number_of_columns==variable_2->number_of_columns()))
		{
			Function_variable_handle intermediate_variable(
				new Function_variable_composite(
				function_matrix_dot_product->variable_1_private,
				function_matrix_dot_product->variable_2_private));

			if (intermediate_variable)
			{
				Function_size_type offset=number_of_rows*number_of_columns;
				Function_size_type number_of_independent_values=2*offset;
				Function_derivative_matrix_handle derivative_f(0),derivative_g(0);
				Function_size_type i,j,k;
				Matrix Df(1,number_of_independent_values),
					D2f(1,number_of_independent_values*number_of_independent_values);
				std::list<Function_variable_handle>
					intermediate_independent_variables(order,intermediate_variable);
				std::list<Matrix> matrices(0);
				std::list<Matrix>::iterator matrix_iterator,matrix_iterator_end;

				Df.clear();
				k=0;
				for (i=1;i<=number_of_rows;i++)
				{
					for (j=1;j<=number_of_columns;j++)
					{
						Df(0,k)=(*variable_2)(i,j);
						Df(0,k+offset)=(*variable_1)(i,j);
						k++;
					}
				}
				if (1<order)
				{
					Function_size_type dependent_column,l,step;

					D2f.clear();
					dependent_column=0;
					step=offset*(number_of_independent_values-1);
					k=0;
					for (i=0;i<number_of_rows;i++)
					{
						for (j=0;j<number_of_columns;j++)
						{
							dependent_column += offset;
							for (l=0;l<k;l++)
							{
								D2f(0,dependent_column)=1;
								D2f(0,dependent_column+step)=1;
								dependent_column++;
							}
							dependent_column++;
							for (l=k+1;l<offset;l++)
							{
								D2f(0,dependent_column)=1;
								D2f(0,dependent_column+step)=1;
								dependent_column++;
							}
							k++;
						}
					}
				}
				for (i=0;i<order;i++)
				{
					matrices.push_back(Df);
					matrix_iterator_end=matrices.end();
					matrix_iterator_end--;
					matrix_iterator=matrices.begin();
					while (matrix_iterator!=matrix_iterator_end)
					{
						if (number_of_independent_values==matrix_iterator->size2())
						{
							matrices.push_back(D2f);
						}
						else
						{
							Matrix zero_matrix(1,
								(matrix_iterator->size2())*number_of_independent_values);

							zero_matrix.clear();
							matrices.push_back(zero_matrix);
						}
						matrix_iterator++;
					}
				}
				try
				{
					derivative_f=new Function_derivative_matrix(this,
						intermediate_independent_variables,matrices);
					derivative_g=new Function_derivative_matrix(intermediate_variable,
						independent_variables);
					if (derivative_f&&derivative_g)
					{
						Function_derivative_matrix_handle derivative_matrix=
							Function_derivative_matrix_compose(this,derivative_f,
							derivative_g);

						if (derivative_matrix)
						{
							result=derivative_matrix->matrix(independent_variables);
						}
					}
				}
				catch (Function_derivative_matrix::Construction_exception)
				{
					// do nothing
					//???debug
					std::cout << "Function_variable_matrix_dot_product<Scalar>::evaluate_derivative.  Failed" << std::endl;
				}
			}
		}
	}

	return (result);
}

template<>
bool Function_matrix_dot_product<Scalar>::evaluate_derivative(
	Scalar& derivative,Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 20 October 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_dot_product<Scalar> >
		atomic_variable_matrix_dot_product;

	result=false;
	if ((atomic_variable_matrix_dot_product=boost::dynamic_pointer_cast<
		Function_variable_matrix_dot_product<Scalar>,Function_variable>(
		atomic_variable))&&equivalent(Function_handle(this),
		atomic_variable_matrix_dot_product->function())&&
		(0<atomic_variable_matrix_dot_product->row())&&
		(0<atomic_variable_matrix_dot_product->column())&&
		(0<atomic_independent_variables.size()))
	{
		boost::intrusive_ptr< Function_matrix<Scalar> > derivative_matrix=
			boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
			atomic_variable_matrix_dot_product->evaluate_derivative(
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
