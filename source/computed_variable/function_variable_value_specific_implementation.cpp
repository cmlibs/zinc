//******************************************************************************
// FILE : function_variable_value_specific_implementation.cpp
//
// LAST MODIFIED : 18 August 2004
//
// DESCRIPTION :
//==============================================================================
//
#include "computed_variable/function_variable_wrapper.hpp"

// global classes
// ==============

// template class Function_variable_value_specific
// ===============================================

EXPORT template<typename Value_type>
Function_variable_value_specific<Value_type>::
	Function_variable_value_specific(
	bool (*set_function)(Value_type&,const Function_variable_handle)):
	set_function(set_function){}

EXPORT template<typename Value_type>
Function_variable_value_specific<Value_type>::
	~Function_variable_value_specific<Value_type>(){}

EXPORT template<typename Value_type>
const std::string Function_variable_value_specific<Value_type>::type()
{
	return (type_string);
}

EXPORT template<typename Value_type>
bool Function_variable_value_specific<Value_type>::set(Value_type& value,
	const Function_variable_handle variable)
{
	bool result;

	result=false;
	if (set_function)
	{
		Function_variable_handle wrapped_variable;
		Function_variable_wrapper_handle temp_variable;

		wrapped_variable=variable;
		while (temp_variable=boost::dynamic_pointer_cast<Function_variable_wrapper,
			Function_variable>(wrapped_variable))
		{
			wrapped_variable=temp_variable->get_wrapped();
		}
		result=(*set_function)(value,wrapped_variable);
	}

	return (result);
}
