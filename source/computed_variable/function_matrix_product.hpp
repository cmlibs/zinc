//******************************************************************************
// FILE : function_matrix_product.hpp
//
// LAST MODIFIED : 21 April 2005
//
// DESCRIPTION :
//==============================================================================
#if !defined (__FUNCTION_MATRIX_PRODUCT_HPP__)
#define __FUNCTION_MATRIX_PRODUCT_HPP__

#include <list>
#include <utility>
#include "computed_variable/function_matrix.hpp"
#include "computed_variable/function_variable_matrix.hpp"

template<typename Value_type>
class Function_variable_matrix_product;

EXPORT template<typename Value_type>
class Function_matrix_product : public Function_matrix<Value_type>
//******************************************************************************
// LAST MODIFIED : 21 April 2005
//
// DESCRIPTION :
// Output is the product of two matrix variables.  Input is the union of the
// matrix variables.
//==============================================================================
{
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	friend class Function_variable_matrix_product<Value_type>;
	friend class Function_derivatnew_matrix_product;
	public:
		// for construction exception
		class Invalid_multiplier_multiplicand {};
		// constructor
		Function_matrix_product(const Function_variable_handle& multiplier,
			const Function_variable_handle& multiplicand);
		// destructor
		~Function_matrix_product();
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
		Function_matrix_product(const Function_matrix_product&);
		// assignment
		Function_matrix_product& operator=(const Function_matrix_product&);
		// equality
		bool operator==(const Function&) const;
	private:
		Function_variable_handle multiplicand_private,multiplier_private;
		static ublas::matrix<Value_type,ublas::column_major> constructor_values;
};

#if !defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_product_implementation.cpp"
#endif // !defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)

#endif /* !defined (__FUNCTION_MATRIX_PRODUCT_HPP__) */
