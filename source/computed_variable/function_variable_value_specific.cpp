//******************************************************************************
// FILE : function_variable_value_specific.cpp
//
// LAST MODIFIED : 15 July 2004
//
// DESCRIPTION :
//==============================================================================

// global classes
// ==============

// template class Function_variable_value_specific
// ===============================================

EXPORT template<typename Value_type>
Function_variable_value_specific<Value_type>::
	Function_variable_value_specific<Value_type>(
	bool (*set_function)(Value_type&,const Function_variable_handle)):
	set_function(set_function){};

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
		result=(*set_function)(value,variable);
	}

	return (result);
}
