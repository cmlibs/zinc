//******************************************************************************
// FILE : function_variable.hpp
//
// LAST MODIFIED : 27 June 2005
//
// DESCRIPTION :
// An abstract class for specifying input/independent and output/dependent
// variables of a function.
//
// Function_variables can be evaluated, differentiated or set to another value.
//==============================================================================
#if !defined (__FUNCTION_VARIABLE_HPP__)
#define __FUNCTION_VARIABLE_HPP__

#include <iterator>
#include <list>

#include "computed_variable/function_base.hpp"
#include "computed_variable/function_variable_value.hpp"

#define USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE

class Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 2 March 2004
//
// DESCRIPTION :
//
// NOTES:
// To get iterators for a sub-class of Function_variable, derive a represenation
// class from this one.
//==============================================================================
{
	friend class Function_variable_iterator;
	public:
		// a "virtual" constructor
		virtual Function_variable_iterator_representation *clone()=0;
	protected:
		// destructor.  Virtual for proper destruction of derived classes
		virtual ~Function_variable_iterator_representation();
	private:
		// increment.  Do not want to return an object derived from
		//   Function_variable_iterator_representation.  Used by
		//   Function_variable_iterator::operator++() and ++(int)
		virtual void increment()=0;
		// decrement.  Do not want to return an object derived from
		//   Function_variable_iterator_representation.  Needs to be able to step
		//   from one past the last to the last.  Used by
		//   Function_variable_iterator::operator--() and --(int)
		virtual void decrement()=0;
		// equality.  Used by Function_variable_iterator::operator==() and !=()
		virtual bool equality(const Function_variable_iterator_representation*)=0;
		// dereference.  Used by Function_variable_iterator::operator*()
		virtual Function_variable_handle dereference() const=0;
	protected:
		// constructor.  Protected so that can't create "plain"
		//   Function_variable_iterator_representations
		Function_variable_iterator_representation();
	private:
		// copy operations are private and undefined to prevent copying
		Function_variable_iterator_representation(
			const Function_variable_iterator_representation&);
		Function_variable_iterator_representation& operator=(
			const Function_variable_iterator_representation&);
};

class Function_variable_iterator:
	public std::iterator<std::bidirectional_iterator_tag,Function_variable_handle,
	ptrdiff_t,Function_variable_handle*,Function_variable_handle>
//******************************************************************************
// LAST MODIFIED : 2 March 2004
//
// DESCRIPTION :
//
// NOTES:
// Do not derive from this class.  Derive from
// Function_variable_iterator_representation.
//
// The reference is Function_variable_handle rather the
// Function_variable_handle& (the default for std::iterator<>) because using
// smart pointers and because the objects in the pool being iterated are created
// by the iterator as required.
//==============================================================================
{
	public:
		// constructors
		Function_variable_iterator();
		Function_variable_iterator(Function_variable_iterator_representation *);
		// copy constructor
		Function_variable_iterator(const Function_variable_iterator&);
		// assignment
		Function_variable_iterator& operator=(const Function_variable_iterator&);
		// destructor
		~Function_variable_iterator();
		// increment (prefix)
		Function_variable_iterator& operator++();
		// increment (postfix)
		Function_variable_iterator operator++(int);
		// decrement (prefix)
		Function_variable_iterator& operator--();
		// decrement (postfix)
		Function_variable_iterator operator--(int);
		// equality
		bool operator==(const Function_variable_iterator&) const;
		// inequality
		bool operator!=(const Function_variable_iterator&) const;
		// dereference
		Function_variable_handle operator*() const;
		// don't have a operator-> because its not needed and it would return a
		//   Function_variable_handle*
	private:
		Function_variable_iterator_representation *representation;
};

class Function_variable
//******************************************************************************
// LAST MODIFIED : 27 June 2005
//
// DESCRIPTION :
// A specification for an input/independent and/or output/dependent variable of
// a Function.
//==============================================================================
{
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	public:
		virtual Function_variable_handle clone() const=0;
		// if the variable is for a single function, the function is returned,
		//   otherwise a zero handle is returned
		virtual Function_handle function() const;
		// returns a specification for the type of variable's value
		virtual Function_variable_value_handle value();
#if defined (EVALUATE_RETURNS_VALUE)
		// evaluate creates a new Function which is the variable's value with the
		//   specified <input> replaced by the given <value>.  For a dependent
		//   variable, this will involve evaluating the variable's function
		virtual Function_handle evaluate(void);
		virtual Function_handle evaluate(Function_variable_handle input,
			Function_handle value);
#else // defined (EVALUATE_RETURNS_VALUE)
		// for a dependent variable, the variable's function will be evaluated.  For
		//   an independent variable, nothing will happen
		virtual bool evaluate();
#endif // defined (EVALUATE_RETURNS_VALUE)
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		// evaluate_derivative creates a new Function which is the value of the
		//   variable differentiated with respect to the <independent_variables>
		//   and the specified <input> replaced with given the <value>
		virtual Function_handle evaluate_derivative(
			std::list<Function_variable_handle>& independent_variables);
		virtual Function_handle evaluate_derivative(
			std::list<Function_variable_handle>& independent_variables,
			Function_variable_handle input,Function_handle value);
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		// derivative creates a new Function which calculates the value of this
		//   variable differentiated with respect to the <independent_variables>
		virtual Function_handle derivative(
			const std::list<Function_variable_handle>& independent_variables)=0;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		// set_value changes the variable to have the <value> in the same order as
		//   evaluate.  Returns true if the variable is changed and false otherwise
		virtual bool set_value(Function_handle value);
		// rset_value changes the variable to have the <value> in the opposite order
		//   to evaluate (back to front).  Returns true if the variable is changed
		//   and false otherwise
		virtual bool rset_value(Function_handle value);
		// get_value creates a new Function which is the variable's value.  The
		//   variable's function is not evaluated
		virtual Function_handle get_value() const;
		// returns a string the represents the variable
		virtual string_handle get_string_representation()=0;
		// for stepping through the atomic variables that make up the variable.
		//   Atomic variables are indivisible, that is, two atomic variables are
		//   either the same or disjoint.  This is needed for determining the
		//   overlap between variables.  Atomic variables are for a single function
		virtual Function_variable_iterator begin_atomic() const=0;
		virtual Function_variable_iterator end_atomic() const=0;
		virtual std::reverse_iterator<Function_variable_iterator>
			rbegin_atomic() const=0;
		virtual std::reverse_iterator<Function_variable_iterator>
			rend_atomic() const=0;
		// returns the number of atomic variables that it makes sense to
		//   differentiate (output/dependent) or differentiate with respect to
		//   (input/independent)
		virtual Function_size_type number_differentiable();
		// the norm of the variable.  A negative result means that the norm is not
		//   defined
		virtual Scalar norm() const;
		// a zero handle indicates an error.  -= and += work in place.  - and +
		//   create new Functions and return their output variables
		virtual Function_variable_handle operator-(const Function_variable&) const;
		virtual Function_variable_handle operator-=(const Function_variable&);
		virtual Function_variable_handle operator+(const Function_variable&) const;
		virtual Function_variable_handle operator+=(const Function_variable&);
	public:
		// for caching function evaluations:
		// adds <dependent_function> to the list of functions that have to be
		//   re-evaluated if this variable's function(s) has/have to be re-evaluated
#if defined (CIRCULAR_SMART_POINTERS)
		virtual void add_dependent_function(
			const Function_handle dependent_function);
#else // defined (CIRCULAR_SMART_POINTERS)
		virtual void add_dependent_function(
			Function *dependent_function);
#endif // defined (CIRCULAR_SMART_POINTERS)
		// removes <dependent_function> to the list of functions that have to be
		//   re-evaluated if this variable's function(s) has/have to be re-evaluated
#if defined (CIRCULAR_SMART_POINTERS)
		virtual void remove_dependent_function(
			const Function_handle dependent_function);
#else // defined (CIRCULAR_SMART_POINTERS)
		virtual void remove_dependent_function(
			Function *dependent_function);
#endif // defined (CIRCULAR_SMART_POINTERS)
	private:
		virtual bool equality_atomic(const Function_variable_handle&) const=0;
		// equality operator.  To be used in equivalent
		virtual bool operator==(const Function_variable&) const;
	protected:
		// constructors.  Protected so that can't create "plain" Function_variables
		Function_variable(const Function_handle& function);
		// copy constructor
		Function_variable(const Function_variable&);
		// destructor.  Virtual for proper destruction of derived classes
		virtual ~Function_variable();
	private:
		// copy operations are private and undefined to prevent copying
		void operator=(const Function_variable&);
	protected:
		Function_handle function_private;
		Function_variable_value_handle value_private;
	private:
		mutable int reference_count;
		friend void intrusive_ptr_add_ref(Function_variable *);
		friend void intrusive_ptr_release(Function_variable *);
};

#endif /* !defined (__FUNCTION_VARIABLE_HPP__) */
