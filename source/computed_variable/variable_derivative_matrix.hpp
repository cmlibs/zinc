//******************************************************************************
// FILE : variable_derivative_matrix.hpp
//
// LAST MODIFIED : 9 February 2004
//
// DESCRIPTION :
//==============================================================================
#if !defined (__VARIABLE_DERIVATIVE_MATRIX_HPP__)
#define __VARIABLE_DERIVATIVE_MATRIX_HPP__

#include <list>

#include "computed_variable/variable.hpp"
#include "computed_variable/variable_composition.hpp"
#include "computed_variable/variable_input_composite_union.hpp"
#include "computed_variable/variable_inverse.hpp"
#include "computed_variable/variable_matrix.hpp"

class Variable_derivative_matrix;

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_derivative_matrix>
	Variable_derivative_matrix_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_derivative_matrix>
	Variable_derivative_matrix_handle;
#else
typedef Variable_derivative_matrix * Variable_derivative_matrix_handle;
#endif

class Variable_derivative_matrix : public Variable
//******************************************************************************
// LAST MODIFIED : 9 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	//???DB.  Where I'm up to
	//???DB.  - can't keep adding friends/includes for new composite
	//???DB.  Bad having all these friends and having to keep adding
#if defined (GENERALIZE_COMPOSITE_INPUT)
	friend class Variable_input_composite_union_evaluate_derivative_functor;
#else // defined (GENERALIZE_COMPOSITE_INPUT)
	friend class Variable_input_composite_evaluate_derivative_functor;
#endif // defined (GENERALIZE_COMPOSITE_INPUT)
	friend class Variable_composite_evaluate_derivative_functor;
#if defined (GENERALIZE_COMPOSITE_INPUT)
	friend Variable_handle Variable_input_composite_union::evaluate_derivative(
		Variable_handle dependent_variable,
		std::list<
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		>& independent_variables,
		std::list<
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		>::iterator& composite_independent_variable,
		std::list<Variable_input_value_handle>& values);
#endif // defined (GENERALIZE_COMPOSITE_INPUT)
	friend Variable_handle Variable::evaluate_derivative(
		std::list<
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		>& independent_variables,
		std::list<Variable_input_value_handle>& values);
	friend Variable_handle Variable_inverse::evaluate_derivative(
		std::list<
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		>& independent_variables,std::list<Variable_input_value_handle>& values);
	friend Variable_derivative_matrix_handle 
		Variable_derivative_matrix_compose(
		const Variable_handle& dependent_variable,
		const Variable_derivative_matrix_handle& derivative_f,
		const Variable_derivative_matrix_handle& derivative_g);
	public:
		// copy constructor
		Variable_derivative_matrix(const Variable_derivative_matrix&);
		// assignment
		Variable_derivative_matrix& operator=(const Variable_derivative_matrix&);
		// destructor
		virtual ~Variable_derivative_matrix();
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
		virtual Variable_handle clone() const;
		// get the specified partial derivative
		Variable_matrix_handle matrix(
			std::list<
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			>& partial_independent_variables);
		// calculate the composition inverse
		Variable_handle inverse(const Variable_inverse_handle&);
	protected:
		Variable_derivative_matrix(const Variable_handle& dependent_variable,
			const std::list<
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			>& independent_variables,
			const std::list<Matrix>& matrices);
		// calls dependent_variable->evaluate_derivative_local to fill in the
		//   matrices
		Variable_derivative_matrix(const Variable_handle& dependent_variable,
			const std::list<
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			>& independent_variables);
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
		//???DB.  Should be Variable rather Variable_handle because when Variable
		//  changes derivative matrix doesn't automatically change?
		Variable_handle dependent_variable;
		std::list<
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			> independent_variables;
		std::list<Matrix> matrices;
};

#endif /* !defined (__VARIABLE_DERIVATIVE_MATRIX_HPP__) */
