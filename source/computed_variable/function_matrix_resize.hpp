//******************************************************************************
// FILE : function_matrix_resize.hpp
//
// LAST MODIFIED : 7 October 2004
//
// DESCRIPTION :
//==============================================================================
#if !defined (__FUNCTION_MATRIX_TRANSPOSE_HPP__)
#define __FUNCTION_MATRIX_TRANSPOSE_HPP__

#include <list>
#include <utility>
#include "computed_variable/function_matrix.hpp"
#include "computed_variable/function_variable_matrix.hpp"

template<typename Value_type>
class Function_variable_matrix_resize;

EXPORT template<typename Value_type>
class Function_matrix_resize : public Function_matrix<Value_type>
//******************************************************************************
// LAST MODIFIED : 7 October 2004
//
// DESCRIPTION :
// Output is the resize of the matrix variable.  Input is the matrix
// variable's input.
//==============================================================================
{
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	friend class Function_variable_matrix_resize<Value_type>;
	public:
		// for construction exception
		class Invalid_argument {};
		// constructor
		Function_matrix_resize(const Function_variable_handle& matrix,
			const Function_size_type number_of_columns);
		// destructor
		~Function_matrix_resize();
	// inherited
	public:
		string_handle get_string_representation();
		Function_variable_handle input();
		Function_variable_handle output();
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
		Function_matrix_resize(const Function_matrix_resize&);
		// assignment
		Function_matrix_resize& operator=(const Function_matrix_resize&);
		// equality
		bool operator==(const Function&) const;
	private:
		Function_size_type number_of_columns_private;
		Function_variable_handle matrix_private;
		static ublas::matrix<Value_type,ublas::column_major> constructor_values;
};

#if !defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_resize_implementation.cpp"
#endif // !defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)

#endif /* !defined (__FUNCTION_MATRIX_TRANSPOSE_HPP__) */
