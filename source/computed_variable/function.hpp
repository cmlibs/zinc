//******************************************************************************
// FILE : function.hpp
//
// LAST MODIFIED : 11 June 2004
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
// LAST MODIFIED : 11 June 2004
//
// DESCRIPTION :
// ???DB.  A function maintains storage for all its inputs and outputs.
//
// ???DB.  Is this a multimethod - virtual function dispatch based on more
//   than one object (Alexandrescu, Chapter 11)?
//==============================================================================
{
	friend class Function_composite;
	friend class Function_composition;
	friend class Function_derivative;
	friend class Function_derivative_matrix;
	friend class Function_variable;
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
	private:
		// if <atomic_variable> is not a variable of the function, then a zero
		//   handle is returned.  Otherwise, evaluate returns a new Function which
		//   is the value of the <atomic_variable>.  The new Function is an identity
		//   function (outputs the same as inputs)
		virtual Function_handle evaluate(
			Function_variable_handle atomic_variable)=0;
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
	private:
		int reference_count;
		friend void intrusive_ptr_add_ref(Function *);
		friend void intrusive_ptr_release(Function *);
};

#endif /* !defined (__FUNCTION_HPP__) */
