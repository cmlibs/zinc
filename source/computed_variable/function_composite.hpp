//******************************************************************************
// FILE : function_composite.hpp
//
// LAST MODIFIED : 15 March 2004
//
// DESCRIPTION :
// A list of functions whose output is the composite of the functions' outputs
// and whose input is the union of the functions' inputs.
//==============================================================================
#if !defined (__FUNCTION_COMPOSITE_HPP__)
#define __FUNCTION_COMPOSITE_HPP__

#include <list>

#include "computed_variable/function.hpp"

class Function_composite : public Function
//******************************************************************************
// LAST MODIFIED : 5 March 2004
//
// DESCRIPTION :
// A composite of other function(s).
//==============================================================================
{
	public:
		// constructor
		Function_composite(const Function_handle& function_1,
			const Function_handle& function_2);
		Function_composite(std::list<Function_handle>& functions_list);
		// destructor
		~Function_composite();
	// inherited
	public:
		string_handle get_string_representation();
		Function_variable_handle input();
		Function_variable_handle output();
	private:
		Function_handle evaluate(Function_variable_handle atomic_variable);
		bool evaluate_derivative(Scalar& derivative,
			Function_variable_handle atomic_variable,
			std::list<Function_variable_handle>& atomic_independent_variables);
		bool set_value(Function_variable_handle atomic_variable,
			Function_variable_handle atomic_value);
	private:
		// copy constructor
		Function_composite(const Function_composite&);
		// assignment
		Function_composite& operator=(const Function_composite&);
	private:
		std::list<Function_handle> functions_list;
};

typedef boost::intrusive_ptr<Function_composite> Function_composite_handle;

#endif /* !defined (__FUNCTION_COMPOSITE_HPP__) */
