//******************************************************************************
// FILE : variable_matrix.hpp
//
// LAST MODIFIED : 4 February 2004
//
// DESCRIPTION :
//==============================================================================
#if !defined (__VARIABLE_MATRIX_HPP__)
#define __VARIABLE_MATRIX_HPP__

#include <list>
#include <utility>
#include "computed_variable/variable.hpp"
#include "computed_variable/variable_vector.hpp"

class Variable_matrix;

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_matrix> Variable_matrix_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_matrix> Variable_matrix_handle;
#else
typedef Variable_matrix * Variable_matrix_handle;
#endif

class Variable_matrix : public Variable
//******************************************************************************
// LAST MODIFIED : 4 February 2004
//
// DESCRIPTION :
// An identity variable whose input/output is a matrix
//==============================================================================
{
	// can be used by any function
	public:
		// constructor
		Variable_matrix(Matrix& values);
		// copy constructor
		Variable_matrix(const Variable_matrix&);
		// assignment
		Variable_matrix& operator=(const Variable_matrix&);
		// destructor
		~Variable_matrix();
		Scalar& operator()(Variable_size_type,Variable_size_type);
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
		// get the specified sub-matrix
		Variable_matrix_handle sub_matrix(Variable_size_type row_low,
			Variable_size_type row_high,Variable_size_type column_low,
			Variable_size_type column_high) const;
		Variable_size_type number_of_rows() const;
		Variable_size_type number_of_columns() const;
		// solve a system of linear equations
		Variable_vector_handle solve(const Variable_handle&);
		Variable_matrix_handle solve(const Variable_matrix_handle&);
		Variable_vector_handle solve(const Variable_vector_handle&);
		// input specifier
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			input_values(),
			input_values(Variable_size_type,Variable_size_type),
			input_values(const ublas::vector<
				std::pair<Variable_size_type,Variable_size_type> >);
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
		Matrix values;
};

#endif /* !defined (__VARIABLE_MATRIX_HPP__) */
