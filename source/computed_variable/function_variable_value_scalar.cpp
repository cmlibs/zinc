//******************************************************************************
// FILE : function_variable_value_scalar.cpp
//
// LAST MODIFIED : 20 February 2004
//
// DESCRIPTION :
//==============================================================================

#include "computed_variable/function_variable_value_scalar.hpp"

const std::string Function_variable_value_scalar::type_string("Scalar");

Function_variable_value_scalar::Function_variable_value_scalar(
	Function_variable_value_scalar_set_function set_function):
	set_function(set_function){}

Function_variable_value_scalar::~Function_variable_value_scalar(){};

const std::string Function_variable_value_scalar::type()
{
	return (type_string);
}

bool Function_variable_value_scalar::set(Scalar& value,
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
