//******************************************************************************
// FILE : function_variable_value.hpp
//
// LAST MODIFIED : 21 July 2004
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

EXPORT template<typename Value_type>
class Function_variable_value_specific : public Function_variable_value
//******************************************************************************
// LAST MODIFIED : 13 July 2004
//
// DESCRIPTION :
// A variable's specific value type.
//==============================================================================
{
	public:
		// constructor
		Function_variable_value_specific(
			bool (*set_function)(Value_type&,const Function_variable_handle));
		// destructor
		virtual ~Function_variable_value_specific();
		const std::string type();
		bool set(Value_type& value,const Function_variable_handle variable);
	private:
		// copy operations are private and undefined to prevent copying
		Function_variable_value_specific(
			const Function_variable_value_specific<Value_type>&);
		void operator=(const Function_variable_value_specific<Value_type>&);
	private:
		static const std::string type_string;
		bool (*set_function)(Value_type&,const Function_variable_handle);
};

#if !defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_variable_value_specific.cpp"
#endif // !defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)

#endif /* !defined (__FUNCTION_VARIABLE_VALUE_HPP__) */
