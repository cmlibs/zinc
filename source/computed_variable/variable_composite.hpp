//******************************************************************************
// FILE : variable_composite.hpp
//
// LAST MODIFIED : 24 November 2003
//
// DESCRIPTION :
//==============================================================================
#if !defined (__VARIABLE_COMPOSITE_HPP__)
#define __VARIABLE_COMPOSITE_HPP__

#include <list>
#include "computed_variable/variable.hpp"

class Variable_composite : public Variable
//******************************************************************************
// LAST MODIFIED : 24 November 2003
//
// DESCRIPTION :
// A composite of other variable(s).
//
// Composite variables are "flat" in the sense that there list of variables does
// not contain composite variables.  This means that the constructor(s) has to
// flatten the list.
//==============================================================================
{
	friend class Variable_composite_evaluate_functor;
	public:
		// constructor
		Variable_composite(std::list<Variable_handle>& variables_list);
		// copy constructor
		Variable_composite(const Variable_composite&);
		// assignment
		Variable_composite& operator=(const Variable_composite&);
		// destructor
		~Variable_composite();
		// get the number of scalars in the result
		Variable_size_type size();
		// get the scalars in the result
		Vector *scalars();
		// input specifier - use inputs for variables in list
		// overload evaluation
		virtual Variable_handle evaluate(
			std::list<Variable_input_value_handle>& values);
		// overload derivative evaluation
		virtual Variable_handle evaluate_derivative(
			std::list<Variable_input_handle>& independent_variables,
			std::list<Variable_input_value_handle>& values);
	private:
		Variable_handle evaluate_local();
		void evaluate_derivative_local(Matrix& matrix,
			std::list<Variable_input_handle>& independent_variables);
		Variable_handle get_input_value_local(const Variable_input_handle& input);
		int set_input_value_local(const Variable_input_handle& input,
			const Variable_handle& value);
		string_handle get_string_representation_local();
	private:
		std::list<Variable_handle> variables_list;
};

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_composite> Variable_composite_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_composite> Variable_composite_handle;
#else
typedef Variable_composite * Variable_composite_handle;
#endif

#endif /* !defined (__VARIABLE_COMPOSITE_HPP__) */
