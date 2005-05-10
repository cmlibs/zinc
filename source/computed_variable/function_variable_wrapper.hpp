//******************************************************************************
// FILE : function_variable_wrapper.hpp
//
// LAST MODIFIED : 10 May 2005
//
// DESCRIPTION :
// A variable that is a wrapper for another variable eg. the input/output
// variable for Function_identity.
//==============================================================================
#if !defined (__FUNCTION_VARIABLE_WRAPPER_HPP__)
#define __FUNCTION_VARIABLE_WRAPPER_HPP__

#include "computed_variable/function_variable.hpp"

class Function_variable_wrapper;

typedef boost::intrusive_ptr<Function_variable_wrapper>
	Function_variable_wrapper_handle;

class Function_variable_wrapper : public Function_variable
//******************************************************************************
// LAST MODIFIED : 10 May 2005
//
// DESCRIPTION :
// An identifier for another variable.
//==============================================================================
{
	friend class Function_derivatnew_wrapper;
	friend class Function_variable_iterator_representation_atomic_wrapper;
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	public:
		// constructors
		Function_variable_wrapper(const Function_handle& wrapping_function,
			const Function_variable_handle& wrapped_variable);
		// destructor.  Virtual for proper destruction of derived classes
		virtual ~Function_variable_wrapper();
	// inherited
	public:
		virtual Function_variable_handle clone() const;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		// derivative creates a new Function which calculates the value of this
		//   variable differentiated with respect to the <independent_variables>
		virtual Function_handle derivative(
			const std::list<Function_variable_handle>& independent_variables);
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		virtual Function_variable_value_handle value();
		virtual bool set_value(Function_handle value);
		virtual bool rset_value(Function_handle value);
		virtual Function_handle get_value() const;
		virtual string_handle get_string_representation();
		virtual Function_variable_iterator begin_atomic() const;
		virtual Function_variable_iterator end_atomic() const;
		virtual std::reverse_iterator<Function_variable_iterator> rbegin_atomic()
			const;
		virtual std::reverse_iterator<Function_variable_iterator> rend_atomic()
			const;
		virtual Function_size_type number_differentiable();
		virtual Function_variable_handle operator-(const Function_variable&) const;
	// additional
	public:
		// get the wrapped variable
		virtual Function_variable_handle get_wrapped() const;
	private:
		virtual bool equality_atomic(const Function_variable_handle&) const;
	protected:
		// copy constructor
		Function_variable_wrapper(const Function_variable_wrapper&);
	protected:
		Function_variable_handle working_variable;
};

#endif /* !defined (__FUNCTION_VARIABLE_WRAPPER_HPP__) */
