//******************************************************************************
// FILE : function_variable_value_element.hpp
//
// LAST MODIFIED : 25 March 2004
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

typedef bool (*Function_variable_value_element_set_function)(
	struct FE_element*& value,const Function_variable_handle variable);

class Function_variable_value_element : public Function_variable_value
//******************************************************************************
// LAST MODIFIED : 25 March 2004
//
// DESCRIPTION :
// A variable's element value type.
//==============================================================================
{
	public:
		// constructor
		Function_variable_value_element(
			Function_variable_value_element_set_function set_function);
		// destructor
		virtual ~Function_variable_value_element();
		const std::string type();
		bool set(struct FE_element*& value,const Function_variable_handle variable);
	private:
		// copy operations are private and undefined to prevent copying
		Function_variable_value_element(const Function_variable_value_element&);
		void operator=(const Function_variable_value_element&);
	private:
		static const std::string type_string;
		Function_variable_value_element_set_function set_function;
};

typedef boost::intrusive_ptr<Function_variable_value_element>
	Function_variable_value_element_handle;

#endif /* !defined (__FUNCTION_VARIABLE_VALUE_ELEMENT_HPP__) */
