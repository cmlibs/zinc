/*******************************************************************************
FILE : api/cmiss_function_integral.cpp

LAST MODIFIED : 4 November 2004

DESCRIPTION :
The public interface to the Cmiss_function integral object.
==============================================================================*/

#include <new>
#include "api/cmiss_function_integral.h"
#include "computed_variable/function_integral.hpp"

/*
Global functions
----------------
*/

Cmiss_function_id Cmiss_function_integral_create(
	Cmiss_function_variable_id integrand_output,
	Cmiss_function_variable_id integrand_input,
	Cmiss_function_variable_id independent_output,
	Cmiss_function_variable_id independent_input,
	Cmiss_region_id domain,char *quadrature_scheme)
/*******************************************************************************
LAST MODIFIED : 5 November 2004

DESCRIPTION :
Creates a Cmiss_function integral.
==============================================================================*/
{
	Cmiss_function_id result;
	Function_variable_handle *independent_input_handle_address,
		*independent_output_handle_address,*integrand_input_handle_address,
		*integrand_output_handle_address;

	result=0;
	if ((integrand_output_handle_address=reinterpret_cast<
		Function_variable_handle *>(integrand_output))&&
		(integrand_input_handle_address=reinterpret_cast<
		Function_variable_handle *>(integrand_input)))
	{
		try
		{
			if ((independent_output_handle_address=reinterpret_cast<
				Function_variable_handle *>(independent_output))&&
				(independent_input_handle_address=reinterpret_cast<
				Function_variable_handle *>(independent_input)))
			{
				result=reinterpret_cast<Cmiss_function_id>(
					new boost::intrusive_ptr<Function_integral>(new Function_integral(
					*integrand_output_handle_address,*integrand_input_handle_address,
					*independent_output_handle_address,*independent_input_handle_address,
					domain,quadrature_scheme)));
			}
			else
			{
				result=reinterpret_cast<Cmiss_function_id>(
					new boost::intrusive_ptr<Function_integral>(new Function_integral(
					*integrand_output_handle_address,*integrand_input_handle_address,
					Function_variable_handle(0),Function_variable_handle(0),
					domain,quadrature_scheme)));
			}
		}
		catch (Function_integral::Construction_exception)
		{
			result=0;
		}
	}

	return (result);
}
