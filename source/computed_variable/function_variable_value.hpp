//******************************************************************************
// FILE : function_variable_value.hpp
//
// LAST MODIFIED : 25 February 2004
//
// DESCRIPTION :
// An abstract class for accessing the value of variable.  A mediator which
// allows a variable to set its value from another variable only having to know
// about its value type.
//==============================================================================
#if !defined (__FUNCTION_VARIABLE_VALUE_HPP__)
#define __FUNCTION_VARIABLE_VALUE_HPP__

#include <string>

#include "computed_variable/function_base.hpp"

class Function_variable_value
//******************************************************************************
// LAST MODIFIED : 20 February 2004
//
// DESCRIPTION :
// A variable's value type.
//==============================================================================
{
	public:
		// destructor
		virtual ~Function_variable_value();
		virtual const std::string type()=0;
	protected:
		// constructor
		Function_variable_value();
	private:
		// copy operations are private and undefined to prevent copying
		Function_variable_value(const Function_variable_value&);
		void operator=(const Function_variable_value&);
	private:
		int reference_count;
		friend void intrusive_ptr_add_ref(Function_variable_value *);
		friend void intrusive_ptr_release(Function_variable_value *);
};

typedef boost::intrusive_ptr<Function_variable_value>
	Function_variable_value_handle;

#endif /* !defined (__FUNCTION_VARIABLE_VALUE_HPP__) */
