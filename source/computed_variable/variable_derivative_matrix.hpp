//******************************************************************************
// FILE : variable_derivative_matrix.hpp
//
// LAST MODIFIED : 16 December 2003
//
// DESCRIPTION :
//==============================================================================
#if !defined (__VARIABLE_DERIVATIVE_MATRIX_HPP__)
#define __VARIABLE_DERIVATIVE_MATRIX_HPP__

#include <list>

#include "computed_variable/variable.hpp"
#include "computed_variable/variable_composition.hpp"
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
// LAST MODIFIED : 16 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	//???DB.  Bad having all these friends and having to keep adding
	friend class Variable_input_composite_evaluate_derivative_functor;
	friend class Variable_composite_evaluate_derivative_functor;
	friend Variable_handle Variable::evaluate_derivative(
		std::list<Variable_input_handle>& independent_variables,
		std::list<Variable_input_value_handle>& values);
	friend Variable_handle Variable_inverse::evaluate_derivative(
		std::list<Variable_input_handle>& independent_variables,
		std::list<Variable_input_value_handle>& values);
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
		// get the number of scalars in the result
		Variable_size_type size() const;
		// get the scalars in the result
		Vector *scalars();
		virtual Variable_handle clone() const;
		// get the specified partial derivative
		Variable_matrix_handle matrix(
			std::list<Variable_input_handle>& partial_independent_variables);
		// calculate the composition inverse
		Variable_handle inverse(const Variable_inverse_handle&);
	protected:
		Variable_derivative_matrix(const Variable_handle& dependent_variable,
			const std::list<Variable_input_handle>& independent_variables,
			const std::list<Matrix>& matrices);
		// calls dependent_variable->evaluate_derivative_local to fill in the
		//   matrices
		Variable_derivative_matrix(const Variable_handle& dependent_variable,
			const std::list<Variable_input_handle>& independent_variables);
	private:
		Variable_handle evaluate_local();
		void evaluate_derivative_local(Matrix& matrix,
			std::list<Variable_input_handle>& independent_variables);
		Variable_handle get_input_value_local(const Variable_input_handle& input);
		int set_input_value_local(const Variable_input_handle& input,
			const Variable_handle& value);
		string_handle get_string_representation_local();
	private:
		//???DB.  Should be Variable rather Variable_handle because when Variable
		//  changes derivative matrix doesn't automatically change?
		Variable_handle dependent_variable;
		std::list<Variable_input_handle> independent_variables;
		std::list<Matrix> matrices;
};

#endif /* !defined (__VARIABLE_DERIVATIVE_MATRIX_HPP__) */
