//******************************************************************************
// FILE : function_variable_value_function_size_type.hpp
//
// LAST MODIFIED : 15 July 2004
//
// DESCRIPTION :
//==============================================================================
#if !defined (__FUNCTION_VARIABLE_VALUE_FUNCTION_SIZE_TYPE_HPP__)
#define __FUNCTION_VARIABLE_VALUE_FUNCTION_SIZE_TYPE_HPP__

#include "computed_variable/function_variable_value.hpp"

#if defined (OLD_CODE)
typedef bool (*Function_variable_value_function_size_type_set_function)(
	Function_size_type& value,const Function_variable_handle variable);

class Function_variable_value_function_size_type :
	public Function_variable_value
//******************************************************************************
// LAST MODIFIED : 24 June 2004
//
// DESCRIPTION :
// A variable's function_size_type value type.
//==============================================================================
{
	public:
		// constructor
		Function_variable_value_function_size_type(
			Function_variable_value_function_size_type_set_function set_function);
		// destructor
		virtual ~Function_variable_value_function_size_type();
		const std::string type();
		bool set(Function_size_type& value,const Function_variable_handle variable);
	private:
		// copy operations are private and undefined to prevent copying
		Function_variable_value_function_size_type(
			const Function_variable_value_function_size_type&);
		void operator=(const Function_variable_value_function_size_type&);
	private:
		static const std::string type_string;
		Function_variable_value_function_size_type_set_function set_function;
};
#endif // defined (OLD_CODE)

typedef Function_variable_value_specific<Function_size_type>
	Function_variable_value_function_size_type;

typedef boost::intrusive_ptr<Function_variable_value_function_size_type>
	Function_variable_value_function_size_type_handle;

#endif /* !defined (__FUNCTION_VARIABLE_VALUE_FUNCTION_SIZE_TYPE_HPP__) */
