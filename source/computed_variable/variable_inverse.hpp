//******************************************************************************
// FILE : variable_inverse.hpp
//
// LAST MODIFIED : 16 December 2003
//
// DESCRIPTION :
//==============================================================================
#if !defined (__VARIABLE_INVERSE_HPP__)
#define __VARIABLE_INVERSE_HPP__

#include <list>
#include "computed_variable/variable.hpp"

class Variable_inverse : public Variable
//******************************************************************************
// LAST MODIFIED : 16 December 2003
//
// DESCRIPTION :
// An inverse of another variable.  Evaluates using a Newton-Raphson iteration
// Evaluates derivative by calculating the derivative of independent_variable
// and "multiplying" by the inverse of the derivative, of the same order, of
// independent_variable with respect to dependent_variable.  Note that the
// derivative of independent_variable with respect to independent_variable is
// the identity and the second derivative of independent_variable with respect
// to independent_variable is zero.
//==============================================================================
{
	friend class Variable_derivative_matrix;
	public:
		// constructor
		Variable_inverse(const Variable_input_handle& dependent_variable,
			Variable_handle& independent_variable);
		// copy constructor
		Variable_inverse(const Variable_inverse&);
		// assignment
		Variable_inverse& operator=(const Variable_inverse&);
		// destructor
		~Variable_inverse();
		// get the number of scalars in the result
		Variable_size_type size() const;
		// get the scalars in the result
		Vector *scalars();
		// input specifier
		Variable_input_handle input_independent();
			//???DB.  Extend to specify parts of independent_variable?
		Variable_input_handle input_step_tolerance();
		Variable_input_handle input_value_tolerance();
		Variable_input_handle input_maximum_iterations();
		Variable_input_handle input_dependent_estimate();
		// overload derivative evaluation
		virtual Variable_handle evaluate_derivative(
			std::list<Variable_input_handle>& independent_variables,
			std::list<Variable_input_value_handle>& values);
		virtual Variable_handle clone() const;
	private:
		Variable_handle evaluate_local();
		void evaluate_derivative_local(Matrix& matrix,
			std::list<Variable_input_handle>& independent_variables);
		Variable_handle get_input_value_local(const Variable_input_handle& input);
		int set_input_value_local(const Variable_input_handle& input,
			const Variable_handle& value);
		string_handle get_string_representation_local();
	private:
		Scalar step_tolerance,value_tolerance;
		Variable_handle dependent_variable_estimate;
		Variable_handle independent_value;
		Variable_handle independent_variable;
		Variable_input_handle dependent_variable;
		Variable_size_type maximum_iterations;
};

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_inverse> Variable_inverse_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_inverse> Variable_inverse_handle;
#else
typedef Variable_inverse * Variable_inverse_handle;
#endif

#endif /* !defined (__VARIABLE_INVERSE_HPP__) */
