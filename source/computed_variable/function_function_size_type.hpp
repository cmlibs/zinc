//******************************************************************************
// FILE : function_function_size_type.hpp
//
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
//==============================================================================
#if !defined (__FUNCTION_FUNCTION_SIZE_TYPE_HPP__)
#define __FUNCTION_FUNCTION_SIZE_TYPE_HPP__

#include <list>
#include <utility>
#include "computed_variable/function.hpp"

class Function_function_size_type;

typedef boost::intrusive_ptr<Function_function_size_type>
	Function_function_size_type_handle;

class Function_function_size_type : public Function
//******************************************************************************
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
// An identity function whose input/output is a function_size_type
//==============================================================================
{
	friend class Function_variable_function_size_type;
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	public:
		// constructor
		Function_function_size_type(Function_size_type& value);
		// destructor
		~Function_function_size_type();
	// inherited
	public:
		string_handle get_string_representation();
		Function_variable_handle input();
		Function_variable_handle output();
	// additional
	public:
		Function_size_type value() const;
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
		Function_function_size_type(const Function_function_size_type&);
		// assignment
		Function_function_size_type& operator=(const Function_function_size_type&);
		// equality
		bool operator==(const Function&) const;
	private:
		Function_size_type value_private;
};

#endif /* !defined (__FUNCTION_FUNCTION_SIZE_TYPE_HPP__) */
