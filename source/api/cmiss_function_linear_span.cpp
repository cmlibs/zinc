/*******************************************************************************
FILE : api/cmiss_function_linear_span.cpp

LAST MODIFIED : 10 November 2004

DESCRIPTION :
The public interface to the Cmiss_function linear span object.
==============================================================================*/

#include <new>
#include "api/cmiss_function_linear_span.h"
#include "computed_variable/function_linear_span.hpp"

/*
Global functions
----------------
*/

Cmiss_function_id Cmiss_function_linear_span_create(
	Cmiss_function_variable_id spanned_variable,
	Cmiss_function_variable_id spanning_variable)
/*******************************************************************************
LAST MODIFIED : 10 November 2004

DESCRIPTION :
Creates a Cmiss_function linear span.
==============================================================================*/
{
	Cmiss_function_id result;
	Function_variable_handle *spanned_variable_handle_address,
		*spanning_variable_handle_address;

	result=0;
	if ((spanned_variable_handle_address=reinterpret_cast<
		Function_variable_handle *>(spanned_variable))&&
		(spanning_variable_handle_address=reinterpret_cast<
		Function_variable_handle *>(spanning_variable)))
	{
		try
		{
			result=reinterpret_cast<Cmiss_function_id>(
				new boost::intrusive_ptr<Function_linear_span>(new Function_linear_span(
				*spanned_variable_handle_address,*spanning_variable_handle_address)));
		}
		catch (Function_linear_span::Construction_exception)
		{
			result=0;
		}
	}

	return (result);
}
