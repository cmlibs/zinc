//******************************************************************************
// FILE : function_variable_composite.hpp
//
// LAST MODIFIED : 21 February 2005
//
// DESCRIPTION :
// A list of specifiers joined together end on end.  There can be repeats in the
// list of atomic specifiers [begin_atomic(),end_atomic()).
//==============================================================================
#if !defined (__FUNCTION_VARIABLE_COMPOSITE_HPP__)
#define __FUNCTION_VARIABLE_COMPOSITE_HPP__

#include <list>

#include "computed_variable/function_variable.hpp"

#define USE_FUNCTION_VARIABLE_COMPOSITE_EVALUATE

class Function_variable_composite : public Function_variable
//******************************************************************************
// LAST MODIFIED : 21 February 2005
//
// DESCRIPTION :
// A composite of other variable(s).
//
#if defined (COMPOSITE_FLATTENING)
// Composite variables are "flat" in the sense that the list of
// variables does not contain composite variables.  This means that the
// constructors have to flatten the list.
//???DB.  Not sure if this "flattening" is useful
#endif // defined (COMPOSITE_FLATTENING)
//==============================================================================
{
	friend class Function_variable_iterator_representation_atomic_composite;
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	public:
		// constructor
		Function_variable_composite(const Function_variable_handle& variable_1,
			const Function_variable_handle& variable_2);
		Function_variable_composite(const Function_handle& function,
			const Function_variable_handle& variable_1,
			const Function_variable_handle& variable_2);
		Function_variable_composite(
			std::list<Function_variable_handle>& variables_list);
		Function_variable_composite(const Function_handle& function,
			std::list<Function_variable_handle>& variables_list);
	// inherited
	public:
		Function_variable_handle clone() const;
#if defined (USE_FUNCTION_VARIABLE_COMPOSITE_EVALUATE)
#if defined (EVALUATE_RETURNS_VALUE)
		// evaluate creates a new Function which is the variable's value.  For a
		//   dependent variable, this will involve evaluating the variable's
		//   function
		virtual Function_handle evaluate();
#else // defined (EVALUATE_RETURNS_VALUE)
		// for a dependent variable, the variable's function will be evaluated.  For
		//   an independent variable, nothing will happen
		virtual bool evaluate();
#endif // defined (EVALUATE_RETURNS_VALUE)
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		// evaluate_derivative creates a new Function which is the value of the
		//   variable differentiated with respect to the <independent_variables>
		virtual Function_handle evaluate_derivative(
			std::list<Function_variable_handle>& independent_variables);
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		// derivative creates a new Function which calculates the value of this
		//   variable differentiated with respect to the <independent_variables>
		virtual Function_handle derivative(
			const std::list<Function_variable_handle>& independent_variables);
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (USE_FUNCTION_VARIABLE_COMPOSITE_EVALUATE)
		string_handle get_string_representation();
		Function_variable_iterator begin_atomic() const;
		Function_variable_iterator end_atomic() const;
		std::reverse_iterator<Function_variable_iterator> rbegin_atomic() const;
		std::reverse_iterator<Function_variable_iterator> rend_atomic() const;
		Function_size_type number_differentiable();
#if defined (CIRCULAR_SMART_POINTERS)
		virtual void add_dependent_function(const Function_handle);
		virtual void remove_dependent_function(const Function_handle);
#else // defined (CIRCULAR_SMART_POINTERS)
		virtual void add_dependent_function(Function*);
		virtual void remove_dependent_function(Function*);
#endif // defined (CIRCULAR_SMART_POINTERS)
	private:
		virtual bool equality_atomic(const Function_variable_handle&) const;
	private:
		// copy constructor
		Function_variable_composite(const Function_variable_composite&);
		// assignment
		Function_variable_composite& operator=(const Function_variable_composite&);
	private:
		std::list<Function_variable_handle> variables_list;
};

typedef boost::intrusive_ptr<Function_variable_composite>
	Function_variable_composite_handle;

#endif /* !defined (__FUNCTION_VARIABLE_COMPOSITE_HPP__) */
