//******************************************************************************
// FILE : function_identity.cpp
//
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
//???DB.  Need to be able to get to the variable it wraps so that can do
//  operations like += for variables?
//???DB.  Are union, intersection, composite wrappers?
//==============================================================================

#include <sstream>

#include "computed_variable/function_identity.hpp"
#include "computed_variable/function_variable_wrapper.hpp"

// global classes
// ==============

// class Function_identity
// -----------------------

Function_identity::Function_identity(const Function_variable_handle& variable):
	Function(),variable_private(variable)
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	if (variable_private)
	{
		variable_private->add_dependent_function(this);
	}
}

Function_identity::~Function_identity()
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
#if defined (CIRCULAR_SMART_POINTERS)
	// do nothing
#else // defined (CIRCULAR_SMART_POINTERS)
	if (variable_private)
	{
		variable_private->remove_dependent_function(this);
	}
#endif // defined (CIRCULAR_SMART_POINTERS)a
}

string_handle Function_identity::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//???DB.  Overload << instead of get_string_representation?
//==============================================================================
{
	string_handle return_string(0);

	if (return_string=new std::string)
	{
		std::ostringstream out;

		out << "identity(";
		if (variable_private)
		{
			out << *(variable_private->get_string_representation());
		}
		out << ")";
		*return_string=out.str();
	}

	return (return_string);
}

Function_variable_handle Function_identity::input()
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new
		Function_variable_wrapper(Function_handle(this),variable_private)));
}

Function_variable_handle Function_identity::output()
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new
		Function_variable_wrapper(Function_handle(this),variable_private)));
}

bool Function_identity::operator==(const Function& function) const
//******************************************************************************
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
// Equality operator.
//==============================================================================
{
	bool result;

	result=false;
	if (this)
	{
		try
		{
			const Function_identity& function_identity=
				dynamic_cast<const Function_identity&>(function);

			result=equivalent(variable_private,function_identity.variable_private);
		}
		catch (std::bad_cast)
		{
			// do nothing
		}
	}

	return (result);
}

Function_handle Function_identity::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
	Function_variable_wrapper_handle atomic_identity_variable;

	if ((atomic_identity_variable=boost::dynamic_pointer_cast<
		Function_variable_wrapper,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_identity_variable->function()))
	{
		result=(atomic_identity_variable->get_wrapped()->get_value)();
	}

	return (result);
}

bool Function_identity::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 20 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_handle atomic_variable_local,atomic_independent_variable;
	Function_variable_wrapper_handle atomic_variable_identity;

	result=false;
	if ((atomic_variable_identity=boost::dynamic_pointer_cast<
		Function_variable_wrapper,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_identity->function())&&
		(1==atomic_variable_identity->number_differentiable())&&
		(atomic_variable_local=atomic_variable_identity->get_wrapped()))
	{
		result=true;
		if ((1==atomic_independent_variables.size())&&
			(atomic_independent_variable=atomic_independent_variables.front())&&
			equivalent(atomic_variable_local,atomic_independent_variable))
		{
			derivative=1;
		}
		else
		{
			derivative=0;
		}
	}

	return (result);
}

bool Function_identity::set_value(Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_handle function;

	result=false;
	if (variable_private&&(function=(variable_private->function)()))
	{
		result=(function->set_value)(atomic_variable,atomic_value);
	}

	return (result);
}

Function_handle Function_identity::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle function,result;

	result=0;
	if (variable_private&&(function=(variable_private->function)()))
	{
		result=(function->get_value)(atomic_variable);
	}

	return (result);
}

Function_identity::Function_identity(
	const Function_identity& function_identity):Function(),
	variable_private(function_identity.variable_private)
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (variable_private)
	{
		variable_private->add_dependent_function(this);
	}
}

Function_identity& Function_identity::operator=(
	const Function_identity& function_identity)
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	if (function_identity.variable_private)
	{
		function_identity.variable_private->add_dependent_function(this);
	}
	if (variable_private)
	{
		variable_private->remove_dependent_function(this);
	}
	variable_private=function_identity.variable_private;

	return (*this);
}
