//******************************************************************************
// FILE : variable_matrix.hpp
//
// LAST MODIFIED : 11 December 2003
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
// LAST MODIFIED : 11 December 2003
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
		// get the number of scalars in the result
		Variable_size_type size() const;
		// get the scalars in the result
		Vector *scalars();
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
		Variable_input_handle input_values();
		Variable_input_handle input_values(Variable_size_type,Variable_size_type);
		Variable_input_handle input_values(const ublas::vector<
			std::pair<Variable_size_type,Variable_size_type> >);
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
		Matrix values;
};

#endif /* !defined (__VARIABLE_MATRIX_HPP__) */
