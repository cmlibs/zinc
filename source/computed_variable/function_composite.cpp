//******************************************************************************
// FILE : function_composite.cpp
//
// LAST MODIFIED : 11 April 2004
//
// DESCRIPTION :
//==============================================================================

#include <sstream>

#include "computed_variable/function_composite.hpp"
#include "computed_variable/function_variable_composite.hpp"
#include "computed_variable/function_variable_union.hpp"
#include "computed_variable/function_variable_value_scalar.hpp"

// global classes
// ==============

// class Function_composite
// ------------------------

Function_composite::Function_composite(const Function_handle& function_1,
	const Function_handle& function_2):Function(),functions_list(0)
//******************************************************************************
// LAST MODIFIED : 10 March 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	functions_list.push_back(function_1);
	functions_list.push_back(function_2);
}

Function_composite::Function_composite(
	std::list<Function_handle>& functions_list):Function(),
	functions_list(functions_list)
//******************************************************************************
// LAST MODIFIED : 10 March 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Function_composite::~Function_composite()
//******************************************************************************
// LAST MODIFIED : 9 March 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

string_handle Function_composite::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 10 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Function_handle>::iterator function_iterator=functions_list.begin();
	std::ostringstream out;
	string_handle return_string,temp_string;
	Function_size_type i=functions_list.size();

	if (return_string=new std::string)
	{
		out << "[" << i << "](";
		while (i>0)
		{
			if (temp_string=(*function_iterator)->get_string_representation())
			{
				out << *temp_string;
				delete temp_string;
			}
			function_iterator++;
			i--;
			if (i>0)
			{
				out << ",";
			}
		}
		out << ")";
		*return_string=out.str();
	}

	return (return_string);
}

Function_variable_handle Function_composite::input()
//******************************************************************************
// LAST MODIFIED : 15 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Function_handle>::iterator function_iterator=functions_list.begin();
	std::list<Function_variable_handle> variables_list(0);
	Function_size_type i;

	for (i=functions_list.size();i>0;i--)
	{
		variables_list.push_back((*function_iterator)->input());
		function_iterator++;
	}

	return (Function_variable_handle(new Function_variable_union(
		Function_handle(this),variables_list)));
}

Function_variable_handle Function_composite::output()
//******************************************************************************
// LAST MODIFIED : 9 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Function_handle>::iterator function_iterator=functions_list.begin();
	std::list<Function_variable_handle> variables_list(0);
	Function_size_type i;

	for (i=functions_list.size();i>0;i--)
	{
		variables_list.push_back((*function_iterator)->output());
		function_iterator++;
	}

	return (Function_variable_handle(new Function_variable_composite(
		Function_handle(this),variables_list)));
}

Function_handle Function_composite::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 10 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
	Function_size_type i=functions_list.size();
	std::list<Function_handle>::iterator function_iterator=functions_list.begin();

	while ((!result)&&(i>0))
	{
		if (*function_iterator)
		{
			result=(*function_iterator)->evaluate(atomic_variable);
		}
		i--;
		function_iterator++;
	}

	return (result);
}

bool Function_composite::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 10 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_size_type i=functions_list.size();
	std::list<Function_handle>::iterator function_iterator=functions_list.begin();

	result=false;
	while ((!result)&&(i>0))
	{
		if (*function_iterator)
		{
			result=(*function_iterator)->evaluate_derivative(derivative,
				atomic_variable,atomic_independent_variables);
		}
		i--;
		function_iterator++;
	}

	return (result);
}

bool Function_composite::set_value(Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 10 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_size_type i=functions_list.size();
	std::list<Function_handle>::iterator function_iterator=functions_list.begin();

	result=false;
	while ((!result)&&(i>0))
	{
		if (*function_iterator)
		{
			result=(*function_iterator)->set_value(atomic_variable,atomic_value);
		}
		i--;
		function_iterator++;
	}

	return (result);
}

Function_composite::Function_composite(
	const Function_composite& function_composite):Function(),
	functions_list(function_composite.functions_list)
//******************************************************************************
// LAST MODIFIED : 10 March 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
}

Function_composite& Function_composite::operator=(
	const Function_composite& function_composite)
//******************************************************************************
// LAST MODIFIED : 10 March 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	this->functions_list=function_composite.functions_list;

	return (*this);
}
