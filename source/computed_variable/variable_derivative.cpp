//******************************************************************************
// FILE : variable_derivative.cpp
//
// LAST MODIFIED : 11 December 2003
//
// DESCRIPTION :
//==============================================================================

#include <new>
#include <sstream>
#include <string>
#include <stdio.h>

//???DB.  Put in include?
const bool Assert_on=true;

template<class Assertion,class Exception>inline void Assert(
	Assertion assertion,Exception exception)
{
	if (Assert_on&&!(assertion)) throw exception;
}

#include "computed_variable/variable_derivative.hpp"

// global classes
// ==============

// class Variable_derivative
// -------------------------

Variable_derivative::Variable_derivative(
	const Variable_handle& dependent_variable,
	std::list<Variable_input_handle>& independent_variables):Variable(),
	dependent_variable(dependent_variable),
	independent_variables(independent_variables)
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
//==============================================================================
{
}

Variable_derivative::Variable_derivative(
	const Variable_derivative& variable_derivative):Variable(),
	dependent_variable(variable_derivative.dependent_variable),
	independent_variables(variable_derivative.independent_variables)
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
}

Variable_derivative& Variable_derivative::operator=(
	const Variable_derivative& variable_derivative)
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	dependent_variable=variable_derivative.dependent_variable;
	independent_variables=variable_derivative.independent_variables;

	return (*this);
}

Variable_derivative::~Variable_derivative()
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

Variable_handle Variable_derivative::evaluate_derivative(
	std::list<Variable_input_handle>& independent_variables,
	std::list<Variable_input_value_handle>& values)
//******************************************************************************
// LAST MODIFIED : 16 November 2003
//
// DESCRIPTION :
// Overload Variable::evaluate_derivative
//==============================================================================
{
	std::list<Variable_input_handle>
		merged_independent_variables=this->independent_variables;

	merged_independent_variables.insert(merged_independent_variables.end(),
		independent_variables.begin(),independent_variables.end());

	return (dependent_variable->evaluate_derivative(merged_independent_variables,
		values));
}

class Variable_derivative_calculate_size_functor
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
// A unary function (functor) for calculating the size of a derivative variable.
//==============================================================================
{
	public:
		Variable_derivative_calculate_size_functor(
			const Variable_handle& dependent_variable,Variable_size_type& size):
			size(size),dependent_variable_size(dependent_variable->size())
		{
			size=0;
		};
		int operator() (const Variable_input_handle& input)
		{
			size += (size+dependent_variable_size)*(input->size());
			return (0);
		};
	private:
		Variable_size_type& size;
		Variable_size_type dependent_variable_size;
};

Variable_size_type Variable_derivative::size() const
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_size_type size;

	// get the specified values
	std::for_each(independent_variables.begin(),independent_variables.end(),
		Variable_derivative_calculate_size_functor(dependent_variable,size));

	return (size);
}

Vector *Variable_derivative::scalars()
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (evaluate_local()->scalars());
}

Variable_handle Variable_derivative::clone() const
//******************************************************************************
// LAST MODIFIED : 8 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (Variable_derivative_handle(new Variable_derivative(*this)));
}

Variable_handle Variable_derivative::evaluate_local()
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Variable_input_value_handle> values(0);

	return (dependent_variable->evaluate_derivative(independent_variables,
		values));
}

void Variable_derivative::evaluate_derivative_local(Matrix&,
	std::list<Variable_input_handle>&)
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	// should not come here - handled by overloading Variable::evaluate_derivative
	Assert(false,std::logic_error(
		"Variable_derivative::evaluate_derivative_local.  "
		"Should not come here"));
}

Variable_handle Variable_derivative::get_input_value_local(
	const Variable_input_handle& input)
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
//???DB.  Could use get_input_value_local
//==============================================================================
{
	return (dependent_variable->get_input_value(input));
}

int Variable_derivative::set_input_value_local(
	const Variable_input_handle& input,const Variable_handle& values)
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (dependent_variable->set_input_value(input,values));
}

string_handle Variable_derivative::get_string_representation_local()
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
//???DB.  Overload << instead of get_string_representation?
//???DB.  Need get_string_representation for inputs to do properly?
//==============================================================================
{
	string_handle return_string;
	std::ostringstream out;

	if (return_string=new std::string)
	{
		out << "derivative";
		*return_string=out.str();
	}

	return (return_string);
}
