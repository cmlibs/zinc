//******************************************************************************
// FILE : variable_derivative.hpp
//
// LAST MODIFIED : 11 December 2003
//
// DESCRIPTION :
//==============================================================================
#if !defined (__VARIABLE_DERIVATIVE_HPP__)
#define __VARIABLE_DERIVATIVE_HPP__

#include <list>
#include "computed_variable/variable.hpp"

class Variable_derivative : public Variable
//******************************************************************************
// LAST MODIFIED : 11 December 2003
//
// DESCRIPTION :
// A derivative of another variable.
//==============================================================================
{
	// so that Variable_derivative_matrix::Variable_derivative_matrix can do the
	//   merging of independent_variables instead of
	//   Variable_derivative::evaluate_derivative_local
	friend class Variable_derivative_matrix;
	public:
		// constructor
		Variable_derivative(const Variable_handle& dependent_variable,
			std::list<Variable_input_handle>& independent_variables);
		// copy constructor
		Variable_derivative(const Variable_derivative&);
		// assignment
		Variable_derivative& operator=(const Variable_derivative&);
		// destructor
		~Variable_derivative();
		// get the number of scalars in the result
		Variable_size_type size() const;
		// get the scalars in the result
		Vector *scalars();
		// input specifier - use dependent variable inputs
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
		Variable_handle dependent_variable;
		std::list<Variable_input_handle> independent_variables;
};

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_derivative> Variable_derivative_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_derivative> Variable_derivative_handle;
#else
typedef Variable_derivative * Variable_derivative_handle;
#endif

#endif /* !defined (__VARIABLE_DERIVATIVE_HPP__) */
