//******************************************************************************
// FILE : function_variable_value_scalar.hpp
//
// LAST MODIFIED : 25 February 2004
//
// DESCRIPTION :
//==============================================================================
#if !defined (__FUNCTION_VARIABLE_VALUE_SCALAR_HPP__)
#define __FUNCTION_VARIABLE_VALUE_SCALAR_HPP__

#include "computed_variable/function_variable_value.hpp"

typedef bool (*Function_variable_value_scalar_set_function)(Scalar& value,
	const Function_variable_handle variable);

class Function_variable_value_scalar : public Function_variable_value
//******************************************************************************
// LAST MODIFIED : 20 February 2004
//
// DESCRIPTION :
// A variable's scalar value type.
//==============================================================================
{
	public:
		// constructor
		Function_variable_value_scalar(
			Function_variable_value_scalar_set_function set_function);
		// destructor
		virtual ~Function_variable_value_scalar();
		const std::string type();
		bool set(Scalar& value,const Function_variable_handle variable);
	private:
		// copy operations are private and undefined to prevent copying
		Function_variable_value_scalar(const Function_variable_value_scalar&);
		void operator=(const Function_variable_value_scalar&);
	private:
		static const std::string type_string;
		Function_variable_value_scalar_set_function set_function;
};

typedef boost::intrusive_ptr<Function_variable_value_scalar>
	Function_variable_value_scalar_handle;

#endif /* !defined (__FUNCTION_VARIABLE_VALUE_SCALAR_HPP__) */
