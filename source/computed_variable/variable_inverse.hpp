//******************************************************************************
// FILE : variable_inverse.hpp
//
// LAST MODIFIED : 4 February 2004
//
// DESCRIPTION :
//==============================================================================
#if !defined (__VARIABLE_INVERSE_HPP__)
#define __VARIABLE_INVERSE_HPP__

#include <list>
#include "computed_variable/variable.hpp"

class Variable_inverse : public Variable
//******************************************************************************
// LAST MODIFIED : 4 February 2004
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
		Variable_inverse(const
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			& dependent_variable,Variable_handle& independent_variable);
		// copy constructor
		Variable_inverse(const Variable_inverse&);
		// assignment
		Variable_inverse& operator=(const Variable_inverse&);
		// destructor
		~Variable_inverse();
		// components are indivisible
#if defined (USE_ITERATORS)
		// returns the number of components that are differentiable
		virtual Variable_size_type number_differentiable() const;
#if defined (USE_VARIABLES_AS_COMPONENTS)
		virtual bool is_component();
#endif // defined (USE_VARIABLES_AS_COMPONENTS)
		// for stepping through the components that make up the Variable
#if defined (USE_VARIABLES_AS_COMPONENTS)
#if defined (USE_ITERATORS_NESTED)
		virtual Iterator begin_components();
		virtual Iterator end_components();
#else // defined (USE_ITERATORS_NESTED)
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		virtual Handle_iterator<Variable_handle> begin_components();
		virtual Handle_iterator<Variable_handle> end_components();
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#endif // defined (USE_ITERATORS_NESTED)
#else // defined (USE_VARIABLES_AS_COMPONENTS)
		virtual Handle_iterator<Variable_io_specifier_handle> begin_components();
		virtual Handle_iterator<Variable_io_specifier_handle> end_components();
#endif // defined (USE_VARIABLES_AS_COMPONENTS)
#else // defined (USE_ITERATORS)
		// get the number of scalars in the result
		Variable_size_type size() const;
		// get the scalars in the result
		Vector *scalars();
#endif // defined (USE_VARIABLE_ITERATORS)
		// input specifier
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			//???DB.  Extend to specify parts of independent_variable?
			input_independent(),
			input_step_tolerance(),
			input_value_tolerance(),
			input_maximum_iterations(),
			input_dependent_estimate();
		// overload derivative evaluation
		virtual Variable_handle evaluate_derivative(
			std::list<
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			>& independent_variables,
			std::list<Variable_input_value_handle>& values);
		virtual Variable_handle clone() const;
	private:
		Variable_handle evaluate_local();
		bool evaluate_derivative_local(Matrix& matrix,
			std::list<
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			>& independent_variables);
		Variable_handle get_input_value_local(
			const
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			& input_atomic);
		bool set_input_value_local(const
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			& input_atomic,
			const
#if defined (USE_VARIABLES_AS_COMPONENTS)
			Variable_handle
#else // defined (USE_VARIABLES_AS_COMPONENTS)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLES_AS_COMPONENTS)
			& value);
		string_handle get_string_representation_local();
	private:
		Scalar step_tolerance,value_tolerance;
		Variable_handle dependent_variable_estimate;
		Variable_handle independent_value;
		Variable_handle independent_variable;
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			dependent_variable;
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
