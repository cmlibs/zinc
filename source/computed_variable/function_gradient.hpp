//******************************************************************************
// FILE : function_gradient.hpp
//
// LAST MODIFIED : 25 August 2004
//
// DESCRIPTION :
//==============================================================================
#if !defined (__FUNCTION_GRADIENT_HPP__)
#define __FUNCTION_GRADIENT_HPP__

#include <list>
#include "computed_variable/function.hpp"

class Function_gradient;

typedef boost::intrusive_ptr<Function_gradient> Function_gradient_handle;

class Function_gradient : public Function
//******************************************************************************
// LAST MODIFIED : 25 August 2004
//
// DESCRIPTION :
// A gradient of another function.
//
// No storage for result and so can't be inverted.
// ???DB.  Could compose with a Function_matrix and then would be invertable?
//==============================================================================
{
	friend class Function_variable_gradient;
	friend class Function_variable_iterator_representation_atomic_gradient;
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	public:
		// constructor
		Function_gradient(const Function_variable_handle& dependent_variable,
			const Function_variable_handle& independent_variable);
		// destructor
		~Function_gradient();
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
		Function_handle get_value(Function_variable_handle atomic_variable);
	private:
		// copy constructor
		Function_gradient(const Function_gradient&);
		// assignment
		Function_gradient& operator=(const Function_gradient&);
		// equality
		bool operator==(const Function&) const;
	private:
		Function_variable_handle dependent_variable_private;
		Function_variable_handle independent_variable_private;
};

#endif /* !defined (__FUNCTION_GRADIENT_HPP__) */
