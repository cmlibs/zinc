//******************************************************************************
// FILE : variable_composition.hpp
//
// LAST MODIFIED : 17 November 2003
//
// DESCRIPTION :
//==============================================================================
#if !defined (__VARIABLE_COMPOSITION_HPP__)
#define __VARIABLE_COMPOSITION_HPP__

#include <list>
#include "computed_variable/variable.hpp"

class Variable_composition : public Variable
//******************************************************************************
// LAST MODIFIED : 17 November 2003
//
// DESCRIPTION :
// A composition of other variable(s).
//==============================================================================
{
	public:
		// constructor
		Variable_composition(const Variable_handle,
			const std::list<Variable_input_value_handle>);
		// copy constructor
		Variable_composition(const Variable_composition&);
		// assignment
		Variable_composition& operator=(const Variable_composition&);
		// destructor
		~Variable_composition();
		// get the number of scalars in the result
		Variable_size_type size();
		// get the scalars in the result
		Vector *scalars();
		// input specifier - use inputs for dependent and source variables
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
		Variable_handle dependent_variable;
		std::list<Variable_input_value_handle> input_source_list;
};

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_composition> Variable_composition_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_composition> Variable_composition_handle;
#else
typedef Variable_composition * Variable_composition_handle;
#endif

#endif /* !defined (__VARIABLE_COMPOSITION_HPP__) */
