//******************************************************************************
// FILE : function_variable_value_function_size_type.cpp
//
// LAST MODIFIED : 15 July 2004
//
// DESCRIPTION :
//==============================================================================

#include "computed_variable/function_variable_value_function_size_type.hpp"

#if defined (OLD_CODE)
const std::string
	Function_variable_value_function_size_type::type_string("Function_size_type");

Function_variable_value_function_size_type::
	Function_variable_value_function_size_type(
	Function_variable_value_function_size_type_set_function set_function):
	set_function(set_function){}

Function_variable_value_function_size_type::
	~Function_variable_value_function_size_type(){}

const std::string Function_variable_value_function_size_type::type()
{
	return (type_string);
}

bool Function_variable_value_function_size_type::set(Function_size_type& value,
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
#endif // defined (OLD_CODE)

const std::string Function_variable_value_specific<Function_size_type>::
	type_string("Function_size_type");
