//******************************************************************************
// FILE : function_matrix.hpp
//
// LAST MODIFIED : 23 February 2005
//
// DESCRIPTION :
//==============================================================================
#if !defined (__FUNCTION_MATRIX_HPP__)
#define __FUNCTION_MATRIX_HPP__

#include <list>
#include <utility>
#include "computed_variable/function.hpp"

EXPORT template<typename Value_type>
class Function_matrix : public Function
//******************************************************************************
// LAST MODIFIED : 23 February 2005
//
// DESCRIPTION :
// An identity function whose input/output is a matrix
//==============================================================================
{
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	public:
		// constructor
		Function_matrix(ublas::matrix<Value_type,ublas::column_major>& values);
		// destructor
		~Function_matrix();
	// inherited
	public:
		virtual string_handle get_string_representation();
		virtual Function_variable_handle input();
		virtual Function_variable_handle output();
	// additional
	public:
		// get the matrix values
		const ublas::matrix<Value_type,ublas::column_major>& matrix();
		// get a matrix entry variable
		virtual Function_variable_handle entry(Function_size_type,
			Function_size_type);
		// get a matrix entry value.  NB. row and column start from 1
		virtual Value_type& operator()(Function_size_type row,
			Function_size_type column);
		// get the specified sub-matrix
		virtual boost::intrusive_ptr< Function_matrix<Value_type> >
			sub_matrix(Function_size_type row_low,
			Function_size_type row_high,Function_size_type column_low,
			Function_size_type column_high) const;
		virtual Function_size_type number_of_rows() const;
		virtual Function_size_type number_of_columns() const;
		// solve a system of linear equations
		boost::intrusive_ptr< Function_matrix<Value_type> > solve(
			const boost::intrusive_ptr< Function_matrix<Value_type> >&);
		// calculate the determinant (zero for non-square matrix)
		virtual bool determinant(Value_type&);
	private:
#if defined (EVALUATE_RETURNS_VALUE)
		virtual Function_handle evaluate(Function_variable_handle atomic_variable);
#else // defined (EVALUATE_RETURNS_VALUE)
		virtual bool evaluate(Function_variable_handle atomic_variable);
#endif // defined (EVALUATE_RETURNS_VALUE)
		virtual bool evaluate_derivative(Scalar& derivative,
			Function_variable_handle atomic_variable,
			std::list<Function_variable_handle>& atomic_independent_variables);
		virtual bool set_value(Function_variable_handle atomic_variable,
			Function_variable_handle atomic_value);
		virtual Function_handle get_value(Function_variable_handle atomic_variable);
	protected:
		// copy constructor
		Function_matrix(const Function_matrix&);
	private:
		// assignment
		Function_matrix& operator=(const Function_matrix&);
		// equality
		virtual bool operator==(const Function&) const;
	protected:
		ublas::matrix<Value_type,ublas::column_major> values;
};

#if !defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_implementation.cpp"
#endif // !defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)

#endif /* !defined (__FUNCTION_MATRIX_HPP__) */
