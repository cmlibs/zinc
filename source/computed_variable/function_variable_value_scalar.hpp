//******************************************************************************
// FILE : function_variable_value_scalar.hpp
//
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
#if !defined (__FUNCTION_VARIABLE_VALUE_SCALAR_HPP__)
#define __FUNCTION_VARIABLE_VALUE_SCALAR_HPP__

#include "computed_variable/function_variable_value.hpp"

typedef Function_variable_value_specific<Scalar> Function_variable_value_scalar;

typedef boost::intrusive_ptr<Function_variable_value_scalar>
	Function_variable_value_scalar_handle;

#endif /* !defined (__FUNCTION_VARIABLE_VALUE_SCALAR_HPP__) */
