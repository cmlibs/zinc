//******************************************************************************
// FILE : function_variable_value_function_size_type.hpp
//
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
#if !defined (__FUNCTION_VARIABLE_VALUE_FUNCTION_SIZE_TYPE_HPP__)
#define __FUNCTION_VARIABLE_VALUE_FUNCTION_SIZE_TYPE_HPP__

#include "computed_variable/function_variable_value.hpp"

typedef Function_variable_value_specific<Function_size_type>
	Function_variable_value_function_size_type;

typedef boost::intrusive_ptr<Function_variable_value_function_size_type>
	Function_variable_value_function_size_type_handle;

#endif /* !defined (__FUNCTION_VARIABLE_VALUE_FUNCTION_SIZE_TYPE_HPP__) */
