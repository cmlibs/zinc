//******************************************************************************
// FILE : function_identity.hpp
//
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
// Used when calculating derivatives eg. composition and inverse.
//==============================================================================
#if !defined (__FUNCTION_IDENTITY_HPP__)
#define __FUNCTION_IDENTITY_HPP__

#include "computed_variable/function.hpp"

class Function_identity;

typedef boost::intrusive_ptr<Function_identity> Function_identity_handle;

class Function_identity : public Function
//******************************************************************************
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
// An identity function for a specified variable.  Used in calculating
// derivatives.
//==============================================================================
{
	friend class Function_variable_identity;
	friend class Function_variable_iterator_representation_atomic_identity;
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	public:
		// constructor
		Function_identity(const Function_variable_handle& variable);
		// destructor
		~Function_identity();
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
		Function_identity(const Function_identity&);
		// assignment
		Function_identity& operator=(const Function_identity&);
		// equality
		bool operator==(const Function&) const;
	private:
		Function_variable_handle variable_private;
};

#endif // !defined (__FUNCTION_IDENTITY_HPP__)
