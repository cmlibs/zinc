//******************************************************************************
// FILE : function_matrix_trace.hpp
//
// LAST MODIFIED : 10 September 2004
//
// DESCRIPTION :
//==============================================================================
#if !defined (__FUNCTION_MATRIX_TRACE_HPP__)
#define __FUNCTION_MATRIX_TRACE_HPP__

#include <list>
#include <utility>
#include "computed_variable/function_matrix.hpp"
#include "computed_variable/function_variable_matrix.hpp"

EXPORT template<typename Value_type>
class Function_matrix_trace : public Function_matrix<Value_type>
//******************************************************************************
// LAST MODIFIED : 10 September 2004
//
// DESCRIPTION :
// Output is the trace of the matrix variable.  Input is the input of the matrix
// variable's function.
//==============================================================================
{
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	public:
		// for construction exception
		class Invalid_matrix {};
		// constructor
		Function_matrix_trace(const Function_variable_handle& matrix);
		// destructor
		~Function_matrix_trace();
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
		Function_matrix_trace(const Function_matrix_trace&);
		// assignment
		Function_matrix_trace& operator=(const Function_matrix_trace&);
		// equality
		bool operator==(const Function&) const;
	private:
		boost::intrusive_ptr< Function_variable_matrix<Value_type> > matrix_private;
		static ublas::matrix<Value_type,ublas::column_major> constructor_values;
};

#if !defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_trace_implementation.cpp"
#endif // !defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)

#endif /* !defined (__FUNCTION_MATRIX_TRACE_HPP__) */
