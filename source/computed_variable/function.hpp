//******************************************************************************
// FILE : function.hpp
//
// LAST MODIFIED : 13 January 2005
//
// DESCRIPTION :
// Functions are expressions that are constructed for:
// - display eg. difference between measured and calculated positions
// - minimization eg. fitting by minimizing the difference between measured and
// 	 calculated positions
// - solution eg. solving a FEM variational formulation equals zero
//
// Functions calculate output/dependent variables from input/independent
// variables.
//==============================================================================
#if !defined (__FUNCTION_HPP__)
#define __FUNCTION_HPP__

#include <list>

#include "computed_variable/function_base.hpp"

class Function
//******************************************************************************
// LAST MODIFIED : 13 January 2005
//
// DESCRIPTION :
// A function maintains storage for all its inputs and the outputs that can be
// inverted.
//
// ???DB.  Is this a multimethod - virtual function dispatch based on more
//   than one object (Alexandrescu, Chapter 11)?
//==============================================================================
{
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	public:
		// returns a string the represents the function
		virtual string_handle get_string_representation()=0;
		// all the inputs/independents.  Derived classes may have additional methods
		//   for specifying subsets of the inputs
		virtual Function_variable_handle input()=0;
		// all the outputs/dependents.  Derived classes may have additional methods
		//   for specifying subsets of the outputs.  Components can be iterated
		//   using output()->begin_atomic() and output()->end_atomic()
		virtual Function_variable_handle output()=0;
#if defined (EVALUATE_RETURNS_VALUE)
		// if <atomic_variable> is not a variable of the function, then a zero
		//   handle is returned.  Otherwise, evaluate returns a new Function which
		//   is the value of the <atomic_variable>.  For a dependent variable, this
		//   will involve evaluating the function.  The new Function is an identity
		//   function (outputs the same as inputs)
		virtual Function_handle evaluate(
			Function_variable_handle atomic_variable)=0;
#else // defined (EVALUATE_RETURNS_VALUE)
		// for a dependent variable, this function is evaluated and true returned
		//   for success, false for failure).  For other variables, nothing is done
		//   and true is returned.
		// ???DB.  Distinguish between other variables?
		//   For a independent variable, true is
		//   returned.  For a variable which is not for this function, false is
		//   returned
		virtual bool evaluate(Function_variable_handle atomic_variable)=0;
#endif // defined (EVALUATE_RETURNS_VALUE)
		// if <atomic_variable> is not a differentiable atomic variable, that is
		//   1!=atomic_variable->number_differentiable(), of the function or
		//   <atomic_independent_variables> are not differentiable atomic variables,
		//   then false is returned.  Otherwise, evaluate_derivative returns true
		//   and sets <derivative> to the value of the <atomic_variable>
		//   differentiated with respect to the <atomic_independent_variables>
		virtual bool evaluate_derivative(Scalar& derivative,
			Function_variable_handle atomic_variable,
			std::list<Function_variable_handle>& atomic_independent_variables)=0;
		// changes the <atomic_variable> to have the <atomic_value>.  Returns true
		//   if the <atomic_variable>'s value was changed and false otherwise
		virtual bool set_value(Function_variable_handle atomic_variable,
			Function_variable_handle atomic_value)=0;
		// if <atomic_variable> is not a variable of the function, then a zero
		//   handle is returned.  Otherwise, get_value returns a new Function which
		//   is the value of the <atomic_variable>.  The function is not evaluated.
		//   The new Function is an identity function (outputs the same as inputs)
		virtual Function_handle get_value(
			Function_variable_handle atomic_variable)=0;
	public:
		//???DB.  Not safe - can forget to call and the function could be marked
		//  evaluated when it still need evaluating.
		// for caching function evaluations:
		// returns true if the function has been evaluated and false otherwise
		virtual bool evaluated() const;
		// sets this function to evaluated
		virtual void set_evaluated();
		// sets this function and its dependent functions to not evaluated
		virtual void set_not_evaluated();
		// adds <dependent_function> to the list of functions that have to be
		//   re-evaluated if this function has to be re-evaluated
#if defined (CIRCULAR_SMART_POINTERS)
		virtual void add_dependent_function(
			const Function_handle dependent_function);
#else // defined (CIRCULAR_SMART_POINTERS)
		virtual void add_dependent_function(
			Function *dependent_function);
#endif // defined (CIRCULAR_SMART_POINTERS)
		// removes <dependent_function> from the list of functions that have to be
		//   re-evaluated if this function has to be re-evaluated
#if defined (CIRCULAR_SMART_POINTERS)
		virtual void remove_dependent_function(
			const Function_handle dependent_function);
#else // defined (CIRCULAR_SMART_POINTERS)
		virtual void remove_dependent_function(
			Function *dependent_function);
#endif // defined (CIRCULAR_SMART_POINTERS)
	protected:
		// constructor.  Protected so that can't create "plain" Functions
		Function();
		// copy constructor
		Function(const Function&);
		// destructor.  Virtual for proper destruction of derived classes
			//???DB.  Would like to be protected, but have some operations where need
			//  to create Function_handles which means need destructor for smart
			//  pointers
			//???DB.  Trying protected again
		virtual ~Function();
	private:
		// copy operations are private and undefined to prevent copying
		void operator=(const Function&);
		// equality operator.  To be used in equivalent
		virtual bool operator==(const Function&) const=0;
	private:
		bool evaluated_private;
		int reference_count;
#if defined (CIRCULAR_SMART_POINTERS)
		std::list<Function_handle> dependent_functions;
#else // defined (CIRCULAR_SMART_POINTERS)
		std::list<Function *> dependent_functions;
#endif // defined (CIRCULAR_SMART_POINTERS)
		friend void intrusive_ptr_add_ref(Function *);
		friend void intrusive_ptr_release(Function *);
};

#endif /* !defined (__FUNCTION_HPP__) */
