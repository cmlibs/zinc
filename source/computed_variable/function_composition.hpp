//******************************************************************************
// FILE : function_composition.hpp
//
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
//==============================================================================
#if !defined (__FUNCTION_COMPOSITION_HPP__)
#define __FUNCTION_COMPOSITION_HPP__

#include <list>
#include "computed_variable/function.hpp"

class Function_composition;

typedef boost::intrusive_ptr<Function_composition> Function_composition_handle;

class Function_composition : public Function
//******************************************************************************
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
// A composition of other function(s).
//==============================================================================
{
	friend class Function_variable_composition;
	friend class Function_variable_iterator_representation_atomic_composition;
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	public:
		// constructor
		Function_composition(const Function_variable_handle output,
			const Function_variable_handle input,
			const Function_variable_handle value);
		// destructor
		~Function_composition();
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
		Function_composition(const Function_composition&);
		// assignment
		Function_composition& operator=(const Function_composition&);
		// equality
		bool operator==(const Function&) const;
	private:
		Function_variable_handle input_private,output_private,value_private;
};

#endif /* !defined (__FUNCTION_COMPOSITION_HPP__) */
