//******************************************************************************
// FILE : function_inverse.hpp
//
// LAST MODIFIED : 9 July 2004
//
// DESCRIPTION :
//==============================================================================
#if !defined (__FUNCTION_INVERSE_HPP__)
#define __FUNCTION_INVERSE_HPP__

#include "computed_variable/function.hpp"

class Function_inverse;

typedef boost::intrusive_ptr<Function_inverse> Function_inverse_handle;

class Function_inverse : public Function
//******************************************************************************
// LAST MODIFIED : 24 June 2004
//
// DESCRIPTION :
// An inverse of another function.  Evaluates using a Newton-Raphson iteration
// Evaluates derivative by calculating the derivative of independent_variable
// and "multiplying" by the inverse of the derivative, of the same order, of
// independent_variable with respect to dependent_variable.  Note that the
// derivative of independent_variable with respect to independent_variable is
// the identity and the second derivative of independent_variable with respect
// to independent_variable is zero.
//==============================================================================
{
	friend class Function_variable_independent;
	friend class Function_variable_iterator_representation_atomic_independent;
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	friend class Function_variable_inverse;
	friend class Function_variable_iterator_representation_atomic_inverse;
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	friend class Function_variable_dependent;
	friend class Function_variable_dependent_estimate;
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	public:
		// constructor
		Function_inverse(const Function_variable_handle & dependent_variable,
			Function_variable_handle& independent_variable);
		// destructor
		~Function_inverse();
	// inherited
	public:
		string_handle get_string_representation();
		Function_variable_handle input();
		Function_variable_handle output();
	// additional
	public:
		Function_variable_handle
			independent(),
				//???DB.  Extend to components?
			step_tolerance(),
			value_tolerance(),
			maximum_iterations(),
			dependent_estimate();
		Scalar step_tolerance_value();
		Scalar value_tolerance_value();
		Function_size_type maximum_iterations_value();
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
		Function_inverse(const Function_inverse&);
		// assignment
		Function_inverse& operator=(const Function_inverse&);
	private:
		Function_size_type maximum_iterations_private;
		Function_variable_handle dependent_variable,independent_variable;
		Scalar step_tolerance_private,value_tolerance_private;
};

#endif /* !defined (__FUNCTION_INVERSE_HPP__) */
