//******************************************************************************
// FILE : function_matrix_determinant.cpp
//
// LAST MODIFIED : 16 September 2004
//
// DESCRIPTION :
//==============================================================================

#if defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_determinant_implementation.cpp"
#else // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_determinant.hpp"
#endif // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)

#if !defined (AIX)
#include "computed_variable/function_derivative_matrix.hpp"

template<>
bool Function_matrix_determinant<Scalar>::evaluate_derivative(
	Scalar& derivative,Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 16 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_determinant<Scalar> >
		atomic_variable_matrix_determinant;
	Function_derivative_matrix_handle derivative_g(0);
	Function_handle matrix_current(0);
	boost::intrusive_ptr< Function_matrix<Scalar> > matrix(0);
	Function_size_type number_of_rows,order;

	result=false;
	if ((atomic_variable_matrix_determinant=boost::dynamic_pointer_cast<
		Function_variable_matrix_determinant<Scalar>,Function_variable>(
		atomic_variable))&&equivalent(Function_handle(this),
		atomic_variable_matrix_determinant->function())&&
		(1==atomic_variable_matrix_determinant->row())&&
		(1==atomic_variable_matrix_determinant->column())&&
		(0<(order=atomic_independent_variables.size()))&&matrix_private&&
		(0<(number_of_rows=matrix_private->number_of_rows()))&&
		(matrix_current=matrix_private->get_value())&&
		(matrix=boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
		matrix_private->evaluate()))&&(derivative_g=new Function_derivative_matrix(
		matrix_private,atomic_independent_variables)))
	{
		bool valid;
		Function_size_type i,j,k,l,number_of_columns,size;
		std::vector<Function_size_type> entries(2*order);
		std::vector<Matrix> Df(order);

		size=number_of_rows*number_of_rows;
		number_of_columns=size;
		i=0;
		valid=true;
		while (valid&&(i<order))
		{
			Matrix& Df_i=Df[i];

			Df_i.resize(1,number_of_columns);
			if (i<size)
			{
				bool zero_derivative;

				for (j=0;j<=2*i+1;j++)
				{
					entries[j]=1;
				}
				j=0;
				while (valid&&(j<number_of_columns))
				{
					// check for repeated rows/columns in derivative
					zero_derivative=false;
					k=0;
					while (!zero_derivative&&(k<=i))
					{
						l=0;
						while (!zero_derivative&&(l<k))
						{
							zero_derivative=((entries[2*l]==entries[2*k])||
								(entries[2*l+1]==entries[2*k+1]));
							l++;
						}
						k++;
					}
					if (zero_derivative)
					{
						Df_i(0,j)=0;
					}
					else
					{
						// doing a copy
						boost::intrusive_ptr< Function_matrix<Scalar> >
							matrix_temp(boost::dynamic_pointer_cast<Function_matrix<Scalar>,
							Function>((matrix->output())->get_value()));
						Function_size_type column,row;

						if (matrix_temp)
						{
							for (k=0;k<=i;k++)
							{
								row=entries[2*k];
								column=entries[2*k+1];
								for (l=1;l<=number_of_rows;l++)
								{
									(*matrix_temp)(row,l)=0;
								}
								for (l=1;l<=number_of_rows;l++)
								{
									(*matrix_temp)(l,column)=0;
								}
								(*matrix_temp)(row,column)=1;
							}
							valid=matrix_temp->determinant(Df_i(0,j));
						}
						else
						{
							valid=false;
						}
					}
					// move to next
					k=2*i+1;
					entries[k]++;
					while ((0!=k)&&(number_of_rows<entries[k]))
					{
						entries[k]=1;
						k--;
						entries[k]++;
					}
					if ((0==k)&&(number_of_rows<entries[k]))
					{
						entries[k]=1;
					}
					j++;
				}
			}
			else
			{
				Df_i.clear();
			}
			number_of_columns *= size;
			i++;
		}
		if (valid)
		{
			Function_derivative_matrix_handle derivative_f(0);
			std::list<Function_variable_handle> intermediate_independent_variables(
				order,matrix_private);
			std::list<Matrix> matrices(0);
			std::list<Matrix>::iterator matrix_iterator,matrix_iterator_end;

			for (i=0;i<order;i++)
			{
				matrices.push_back(Df[0]);
				matrix_iterator_end=matrices.end();
				matrix_iterator_end--;
				matrix_iterator=matrices.begin();
				number_of_columns=size;
				j=1;
				while (matrix_iterator!=matrix_iterator_end)
				{
					if (number_of_columns!=(k=matrix_iterator->size2()))
					{
						if (size==k)
						{
							j=1;
						}
						else
						{
							if (number_of_columns*size==k)
							{
								j++;
								number_of_columns=k;
							}
						}
					}
					matrices.push_back(Df[j]);
					matrix_iterator++;
				}
			}
			derivative_f=new Function_derivative_matrix(atomic_variable,
				intermediate_independent_variables,matrices);
			if (derivative_f)
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
	if (matrix_current)
	{
		matrix_private->rset_value(matrix_current);
	}

	return (result);
}
#endif // !defined (AIX)
