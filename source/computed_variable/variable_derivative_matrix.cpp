//******************************************************************************
// FILE : variable_derivative_matrix.cpp
//
// LAST MODIFIED : 19 November 2003
//
// DESCRIPTION :
//==============================================================================

#include <algorithm>
#include <iterator>
#include <new>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>

//???DB.  Put in include?
const bool Assert_on=true;

template<class Assertion,class Exception>inline void Assert(
	Assertion assertion,Exception exception)
{
	if (Assert_on&&!(assertion)) throw exception;
}

#include "computed_variable/variable_derivative_matrix.hpp"

// class Variable_derivative_matrix
// --------------------------------

Variable_derivative_matrix::Variable_derivative_matrix(
	const Variable_handle& dependent_variable,
	const std::list<Variable_input_handle>& independent_variables,
	const std::list<Matrix>& matrices) : Variable(),
	dependent_variable(dependent_variable),
	independent_variables(independent_variables),matrices(matrices)
//******************************************************************************
// LAST MODIFIED : 19 November 2003
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	//???DB.  Check that consistent?
}

class Variable_derivative_matrix_create_matrices_inner_functor
//******************************************************************************
// LAST MODIFIED : 12 October 2003
//
// DESCRIPTION :
// A unary function (Functor) for creating and zeroing the list of matrices for
// a derivative matrix variable from a list of independent variables.  Inner
// loop.
//==============================================================================
{
	public:
		Variable_derivative_matrix_create_matrices_inner_functor(
			const Variable_handle& dependent_variable,
			const Variable_input_handle& independent_variable,
			int number_of_independent_values,std::list<Matrix>& matrices,
			std::list< std::list<Variable_input_handle> >&
			matrix_independent_variables):
			number_of_independent_values(number_of_independent_values),
			matrices(matrices),
			matrix_independent_variables(matrix_independent_variables),
			matrix_independent_variables_iterator(
			matrix_independent_variables.begin()),
			dependent_variable(dependent_variable),
			independent_variable(independent_variable)
		{
		};
		~Variable_derivative_matrix_create_matrices_inner_functor() {};
		int operator() (Matrix& matrix)
		{
			Matrix new_matrix((matrix.size1)(),
				number_of_independent_values*(matrix.size2)());
			std::list<Variable_input_handle> new_matrix_independent_variables;

			new_matrix_independent_variables= *matrix_independent_variables_iterator;
			new_matrix_independent_variables.push_back(independent_variable);
			dependent_variable->evaluate_derivative_local(new_matrix,
				new_matrix_independent_variables);
			matrices.push_back(new_matrix);
			matrix_independent_variables.push_back(new_matrix_independent_variables);
			matrix_independent_variables_iterator++;

			return (1);
		};
	private:
		int number_of_independent_values;
		std::list<Matrix>& matrices;
		std::list< std::list<Variable_input_handle> >& matrix_independent_variables;
		std::list< std::list<Variable_input_handle> >::iterator
			matrix_independent_variables_iterator;
		const Variable_handle& dependent_variable;
		const Variable_input_handle& independent_variable;
};

class Variable_derivative_matrix_create_matrices_outer_functor
//******************************************************************************
// LAST MODIFIED : 12 October 2003
//
// DESCRIPTION :
// A unary function (Functor) for creating and zeroing the list of matrices for
// a derivative matrix variable from a list of independent variables.  Outer
// loop.
//==============================================================================
{
	public:
		Variable_derivative_matrix_create_matrices_outer_functor(
			const Variable_handle& dependent_variable,std::list<Matrix>& matrices):
			number_of_dependent_values(dependent_variable->size()),matrices(matrices),
			matrix_independent_variables(),dependent_variable(dependent_variable)
		{};
		~Variable_derivative_matrix_create_matrices_outer_functor() {};
		int operator() (const Variable_input_handle& independent_variable)
		{
			int number_of_independent_values=independent_variable->size();
			Matrix new_matrix(number_of_dependent_values,
				number_of_independent_values);
			std::list<Variable_input_handle> new_matrix_independent_variables;
			std::list<Matrix>::iterator last;

			new_matrix_independent_variables.push_back(independent_variable);
			dependent_variable->evaluate_derivative_local(new_matrix,
				new_matrix_independent_variables);
			matrices.push_back(new_matrix);
			matrix_independent_variables.push_back(new_matrix_independent_variables);
			last=matrices.end();
			last--;
			for_each(matrices.begin(),last,
				Variable_derivative_matrix_create_matrices_inner_functor(
				dependent_variable,independent_variable,number_of_independent_values,
				matrices,matrix_independent_variables));

			return (1);
		};
	private:
		int number_of_dependent_values;
		std::list<Matrix>& matrices;
		std::list< std::list<Variable_input_handle> > matrix_independent_variables;
		const Variable_handle& dependent_variable;
};

Variable_derivative_matrix::Variable_derivative_matrix(
	const Variable_handle& dependent_variable,
	const std::list<Variable_input_handle>& independent_variables) :
	Variable(),dependent_variable(dependent_variable),
	independent_variables(independent_variables),matrices()
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	for_each(independent_variables.begin(),independent_variables.end(),
		Variable_derivative_matrix_create_matrices_outer_functor(
		dependent_variable,matrices));
}

Variable_derivative_matrix::Variable_derivative_matrix(
	const Variable_derivative_matrix& derivative_matrix) : Variable(),
	dependent_variable(derivative_matrix.dependent_variable),
	independent_variables(derivative_matrix.independent_variables),
	matrices(derivative_matrix.matrices){}
//******************************************************************************
// LAST MODIFIED : 16 October 2003
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================

Variable_derivative_matrix::~Variable_derivative_matrix()
//******************************************************************************
// LAST MODIFIED : 2 October 2003
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	//???DB.  To be done
}

class Variable_derivative_matrix_calculate_size_functor
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	public:
		Variable_derivative_matrix_calculate_size_functor(
			Variable_size_type& result):result(result){};
		int operator() (Matrix& matrix)
		{
			result += (matrix.size1())*(matrix.size2());
			return (0);
		}
		Variable_size_type& result;
};

Variable_size_type Variable_derivative_matrix::size()
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_size_type result=0;

	std::for_each(matrices.begin(),matrices.end(),
		Variable_derivative_matrix_calculate_size_functor(result));

	return (result);
}

class Variable_derivative_matrix_get_scalars_functor
//******************************************************************************
// LAST MODIFIED : 24 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	public:
		Variable_derivative_matrix_get_scalars_functor(Vector& values):
			index(0),values(values){};
		int operator() (Matrix& matrix)
		{
			Variable_size_type i,j,number_of_columns=matrix.size2(),
				number_of_rows=matrix.size1();

			for (i=0;i<number_of_rows;i++)
			{
				for (j=0;j<number_of_columns;j++)
				{
					values[index]=matrix(i,j);
					index++;
				}
			}

			return (0);
		}
	private:
		Variable_size_type index;
		Vector& values;
};

Vector *Variable_derivative_matrix::scalars()
//******************************************************************************
// LAST MODIFIED : 24 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	Vector *values_vector;

	if (values_vector=new Vector(size()))
	{
		std::for_each(matrices.begin(),matrices.end(),
			Variable_derivative_matrix_get_scalars_functor(*values_vector));
	}

	return (values_vector);
}

class Variable_derivative_matrix_get_matrix_functor
//******************************************************************************
// LAST MODIFIED : 6 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	public:
		Variable_derivative_matrix_get_matrix_functor(Variable_size_type& index,
			std::list<Matrix>::reverse_iterator& matrix_iterator,
			std::list<Matrix>& matrices,
			std::list<Variable_input_handle>& partial_independent_variables):
			matrix_iterator(matrix_iterator),partial_independent_variable_iterator(
			partial_independent_variables.rbegin()),index(index),
			offset(1+matrices.size())
		{
			matrix_iterator=matrices.rbegin();
			index=partial_independent_variables.size();
		};
		int operator() (Variable_input_handle& independent_variable)
		{
			bool found;

			offset /=2;
			if (index>0)
			{
				found=(**partial_independent_variable_iterator== *independent_variable);
			}
			else
			{
				found=false;
			}
			if (found)
			{
				index--;
				partial_independent_variable_iterator++;
			}
			else
			{
				int i;

				for (i=offset;i>0;i--)
				{
					matrix_iterator++;
				}
			}

			return (0);
		}
	private:
		std::list<Matrix>::reverse_iterator& matrix_iterator;
		std::list<Variable_input_handle>::reverse_iterator
			partial_independent_variable_iterator;
		Variable_size_type& index;
		Variable_size_type offset;
};

Variable_matrix_handle Variable_derivative_matrix::matrix(
	std::list<Variable_input_handle>& partial_independent_variables)
//******************************************************************************
// LAST MODIFIED : 6 November 2003
//
// DESCRIPTION :
// Returns the specified partial derivative (<partial_independent_variables>).
//
// ???DB.  Extend so that can have an independent variable that is a subset of
//   one of the independent variables for the derivative matrix.  eg nodal
//   values for a particular node as a subset of all nodal values
//==============================================================================
{
	std::list<Matrix>::reverse_iterator matrix_iterator;
	Variable_matrix_handle result;
	Variable_size_type index;

	std::for_each((this->independent_variables).rbegin(),
		(this->independent_variables).rend(),
		Variable_derivative_matrix_get_matrix_functor(index,matrix_iterator,
		matrices,partial_independent_variables));
	if (0==index)
	{
		result=Variable_matrix_handle(new Variable_matrix(*matrix_iterator));
	}
	else
	{
		Matrix temp_matrix(0,0);

		result=Variable_matrix_handle(new Variable_matrix(temp_matrix));
	}

	return (result);
}

Variable_handle Variable_derivative_matrix::evaluate_local()
//******************************************************************************
// LAST MODIFIED : 5 November 2003
//
// DESCRIPTION :
// Duplicate <this> so that <this> isn't changed by operations on the result.
//==============================================================================
{
	return (Variable_handle(new Variable_derivative_matrix(*this)));
}

void Variable_derivative_matrix::evaluate_derivative_local(Matrix &matrix,
	std::list<Variable_input_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 12 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	//???DB.  To be done
	// matrix is zero'd on entry
	//???DB.  Need to set up input for Variable_derivative_matrix
	if (1==independent_variables.size())
	{
		//???DB.  Really want to return identity matrix
		//???DB.  Need to check matrix size
		matrix(0,0)=1;
	}
}

Variable_handle Variable_derivative_matrix::get_input_value_local(
	const Variable_input_handle& input)
//******************************************************************************
// LAST MODIFIED : 16 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_handle value;

	//???DB.  To be done
	//???DB.  Temporary code to use up arguments
	if (0<(input->size()))
	{
		value=Variable_derivative_matrix_handle(new Variable_derivative_matrix(
			*this));
	}
	else
	{
		value=Variable_handle((Variable *)0);
	}

	return (value);
}

int Variable_derivative_matrix::set_input_value_local(
	const Variable_input_handle& input,const Variable_handle& value)
//******************************************************************************
// LAST MODIFIED : 2 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	int return_code;

	//???DB.  To be done
	//???DB.  Temporary code to use up arguments
	return_code=0;
	if ((0<(input->size()))&&(value->get_string_representation()))
	{
		return_code=1;
	}

	return (return_code);
}

string_handle Variable_derivative_matrix::get_string_representation_local()
//******************************************************************************
// LAST MODIFIED : 17 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string;
	std::ostringstream out;

	if (return_string=new std::string)
	{
		out << matrices.back();
		*return_string=out.str();
	}

	return (return_string);
}
