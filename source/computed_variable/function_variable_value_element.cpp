//******************************************************************************
// FILE : function_variable_value_element.cpp
//
// LAST MODIFIED : 25 March 2004
//
// DESCRIPTION :
//==============================================================================

#include "computed_variable/function_variable_value_element.hpp"

const std::string Function_variable_value_element::type_string("Element");

Function_variable_value_element::Function_variable_value_element(
	Function_variable_value_element_set_function set_function):
	set_function(set_function){}

Function_variable_value_element::~Function_variable_value_element(){};

const std::string Function_variable_value_element::type()
{
	return (type_string);
}

bool Function_variable_value_element::set(struct FE_element*& value,
	const Function_variable_handle variable)
{
	bool result;

	result=false;
	if (set_function)
	{
		result=(*set_function)(value,variable);
	}

	return (result);
}
