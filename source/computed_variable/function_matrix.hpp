//******************************************************************************
// FILE : function_matrix.hpp
//
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
//==============================================================================
#if !defined (__FUNCTION_MATRIX_HPP__)
#define __FUNCTION_MATRIX_HPP__

#include <list>
#include <utility>
#include "computed_variable/function.hpp"

EXPORT template<typename Value_type>
	class Function_variable_matrix_coefficients;

EXPORT template<typename Value_type>
class Function_matrix : public Function
//******************************************************************************
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
// An identity function whose input/output is a matrix
//==============================================================================
{
	friend class Function_variable_matrix_coefficients<Value_type>;
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
		string_handle get_string_representation();
		Function_variable_handle input();
		Function_variable_handle output();
	// additional
	public:
		// get a matrix entry variable
		Function_variable_handle entry(Function_size_type,Function_size_type);
		// get a matrix entry value
		Value_type& operator()(Function_size_type,Function_size_type);
		// get the specified sub-matrix
		boost::intrusive_ptr< Function_matrix<Value_type> >
			sub_matrix(Function_size_type row_low,
			Function_size_type row_high,Function_size_type column_low,
			Function_size_type column_high) const;
		Function_size_type number_of_rows() const;
		Function_size_type number_of_columns() const;
		// solve a system of linear equations
		boost::intrusive_ptr< Function_matrix<Value_type> > solve(
			const boost::intrusive_ptr< Function_matrix<Value_type> >&);
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
		Function_matrix(const Function_matrix&);
		// assignment
		Function_matrix& operator=(const Function_matrix&);
		// equality
		bool operator==(const Function&) const;
	private:
		ublas::matrix<Value_type,ublas::column_major> values;
};

#if !defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_implementation.cpp"
#endif // !defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)

#endif /* !defined (__FUNCTION_MATRIX_HPP__) */
