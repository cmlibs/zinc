//******************************************************************************
// FILE : function_variable_value_element.hpp
//
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
#if !defined (__FUNCTION_VARIABLE_VALUE_ELEMENT_HPP__)
#define __FUNCTION_VARIABLE_VALUE_ELEMENT_HPP__

extern "C"
{
#include "finite_element/finite_element.h"
}
#include "computed_variable/function_variable_value.hpp"

typedef struct FE_element* Element;

typedef Function_variable_value_specific<Element>
	Function_variable_value_element;

typedef boost::intrusive_ptr<Function_variable_value_element>
	Function_variable_value_element_handle;

#endif /* !defined (__FUNCTION_VARIABLE_VALUE_ELEMENT_HPP__) */
