//******************************************************************************
// FILE : function_composite.hpp
//
// LAST MODIFIED : 13 January 2005
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
// LAST MODIFIED : 13 January 2005
//
// DESCRIPTION :
// A composite of other function(s).
//==============================================================================
{
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	public:
		// for construction exception
		class Construction_exception {};
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
#if defined (EVALUATE_RETURNS_VALUE)
		Function_handle evaluate(Function_variable_handle atomic_variable);
#else // defined (EVALUATE_RETURNS_VALUE)
		bool evaluate(Function_variable_handle atomic_variable);
#endif // defined (EVALUATE_RETURNS_VALUE)
		bool evaluate_derivative(Scalar& derivative,
			Function_variable_handle atomic_variable,
			std::list<Function_variable_handle>& atomic_independent_variables);
		bool set_value(Function_variable_handle atomic_variable,
			Function_variable_handle atomic_value);
		Function_handle get_value(Function_variable_handle atomic_variable);
	private:
		// copy constructor
		Function_composite(const Function_composite&);
		// assignment
		Function_composite& operator=(const Function_composite&);
		// equality
		bool operator==(const Function&) const;
	private:
		std::list<Function_handle> functions_list;
};

typedef boost::intrusive_ptr<Function_composite> Function_composite_handle;

#endif /* !defined (__FUNCTION_COMPOSITE_HPP__) */
