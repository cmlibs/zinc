//******************************************************************************
// FILE : function_matrix.hpp
//
// LAST MODIFIED : 17 March 2004
//
// DESCRIPTION :
//==============================================================================
#if !defined (__FUNCTION_MATRIX_HPP__)
#define __FUNCTION_MATRIX_HPP__

#include <list>
#include <utility>
#include "computed_variable/function.hpp"

class Function_matrix;

typedef boost::intrusive_ptr<Function_matrix> Function_matrix_handle;

class Function_matrix : public Function
//******************************************************************************
// LAST MODIFIED : 17 March 2004
//
// DESCRIPTION :
// An identity function whose input/output is a matrix
//==============================================================================
{
	friend class Function_variable_matrix;
	public:
		// constructor
		Function_matrix(Matrix& values);
		// destructor
		~Function_matrix();
	// inherited
	public:
		string_handle get_string_representation();
		Function_variable_handle input();
		Function_variable_handle output();
	// additional
	public:
		// get a matrix entry variable
		Function_variable_handle entry(Function_size_type,Function_size_type);
		// get a matrix entry value
		Scalar& operator()(Function_size_type,Function_size_type);
		// get the specified sub-matrix
		Function_matrix_handle sub_matrix(Function_size_type row_low,
			Function_size_type row_high,Function_size_type column_low,
			Function_size_type column_high) const;
		Function_size_type number_of_rows() const;
		Function_size_type number_of_columns() const;
		// solve a system of linear equations
		Function_matrix_handle solve(const Function_matrix_handle&);
	private:
		Function_handle evaluate(Function_variable_handle atomic_variable);
		bool evaluate_derivative(Scalar& derivative,
			Function_variable_handle atomic_variable,
			std::list<Function_variable_handle>& atomic_independent_variables);
		bool set_value(Function_variable_handle atomic_variable,
			Function_variable_handle atomic_value);
	private:
		// copy constructor
		Function_matrix(const Function_matrix&);
		// assignment
		Function_matrix& operator=(const Function_matrix&);
	private:
		Matrix values;
};

#endif /* !defined (__FUNCTION_MATRIX_HPP__) */
