//******************************************************************************
// FILE : function_matrix_divide_by_scalar.hpp
//
// LAST MODIFIED : 6 March 2005
//
// DESCRIPTION :
//==============================================================================
#if !defined (__FUNCTION_MATRIX_DIVIDE_BY_SCALAR_HPP__)
#define __FUNCTION_MATRIX_DIVIDE_BY_SCALAR_HPP__

#include <list>
#include <utility>
#include "computed_variable/function_matrix.hpp"
#include "computed_variable/function_variable_matrix.hpp"

template<typename Value_type>
class Function_variable_matrix_divide_by_scalar;

EXPORT template<typename Value_type>
class Function_matrix_divide_by_scalar : public Function_matrix<Value_type>
//******************************************************************************
// LAST MODIFIED : 6 March 2005
//
// DESCRIPTION :
// Output is the dividend (has matrix value) divided by the divisor (has 1x1
// matrix value) .  Input is the union of the dividend and the divisor.
//==============================================================================
{
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	friend class Function_variable_matrix_divide_by_scalar<Value_type>;
	public:
		// for construction exception
		class Invalid_dividend_divisor {};
		// constructor
		Function_matrix_divide_by_scalar(const Function_variable_handle& dividend,
			const Function_variable_handle& divisor);
		// destructor
		~Function_matrix_divide_by_scalar();
	// inherited
	public:
		string_handle get_string_representation();
		Function_variable_handle input();
		Function_variable_handle output();
	private:
#if defined (EVALUATE_RETURNS_VALUE)
		Function_handle evaluate(Function_variable_handle atomic_variable);
#else // defined (EVALUATE_RETURNS_VALUE)
		bool evaluate(Function_variable_handle atomic_variable);
#endif // defined (EVALUATE_RETURNS_VALUE)
		bool evaluate_derivative(Scalar& derivative,
			Function_variable_handle atomic_variable,
			std::list<Function_variable_handle>& atomic_independent_variables);
		bool set_value(Function_variable_handle atomic_variable,
			Function_variable_handle atomic_value);
		Function_handle get_value(Function_variable_handle atomic_variable);
	private:
		// copy constructor
		Function_matrix_divide_by_scalar(const Function_matrix_divide_by_scalar&);
		// assignment
		Function_matrix_divide_by_scalar& operator=(
			const Function_matrix_divide_by_scalar&);
		// equality
		bool operator==(const Function&) const;
	private:
		Function_variable_handle dividend_private,divisor_private;
		static ublas::matrix<Value_type,ublas::column_major> constructor_values;
};

#if !defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_divide_by_scalar_implementation.cpp"
#endif // !defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)

#endif /* !defined (__FUNCTION_MATRIX_DIVIDE_BY_SCALAR_HPP__) */
