//******************************************************************************
// FILE : function_linear_span.hpp
//
// LAST MODIFIED : 13 January 2005
//
// DESCRIPTION :
//==============================================================================
#if !defined (__FUNCTION_LINEAR_SPAN_HPP__)
#define __FUNCTION_LINEAR_SPAN_HPP__

#include <list>
#include "computed_variable/function.hpp"
#include "computed_variable/function_matrix.hpp"

class Function_linear_span;

typedef boost::intrusive_ptr<Function_linear_span> Function_linear_span_handle;

class Function_linear_span : public Function_matrix<Scalar>
//******************************************************************************
// LAST MODIFIED : 13 January 2005
//
// DESCRIPTION :
// Evaluates to a vector whose entries are the result of evaluating the spanned
// variable at each of the vectors in the unit basis for the spanned variable's
// values.
//==============================================================================
{
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	friend class Function_variable_linear_span;
	public:
		// for construction exception
		class Construction_exception {};
		// constructor
		Function_linear_span(const Function_variable_handle& spanned_variable,
			const Function_variable_handle& spanning_variable);
		// destructor
		~Function_linear_span();
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
		Function_linear_span(const Function_linear_span&);
		// assignment
		Function_linear_span& operator=(const Function_linear_span&);
		// equality
		bool operator==(const Function&) const;
	private:
		Function_variable_handle spanned_variable_private,spanning_variable_private;
		static ublas::matrix<Scalar,ublas::column_major> constructor_values;
};

#endif /* !defined (__FUNCTION_LINEAR_SPAN_HPP__) */
