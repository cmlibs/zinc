/*******************************************************************************
FILE : api/cmiss_variable_new_composite.cpp

LAST MODIFIED : 16 November 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new composite object.
==============================================================================*/

#include <new>
#include "api/cmiss_variable_new_composite.h"
#include "computed_variable/variable_composite.hpp"

/*
Global functions
----------------
*/
Cmiss_variable_new_id Cmiss_variable_new_composite_create(
	Cmiss_variable_new_list_id variables)
/*******************************************************************************
LAST MODIFIED : 16 November 2003

DESCRIPTION :
Creates a Cmiss_variable_new composite with the supplied <variables>.
==============================================================================*/
{
	Cmiss_variable_new_id result;
	std::list<Variable_handle> *variables_address;

	result=0;
	if (variables_address=reinterpret_cast<std::list<Variable_handle> *>(
    variables))
	{
		result=reinterpret_cast<Cmiss_variable_new_id>(
#if defined (USE_SMART_POINTER)
			new Variable_composite_handle(new Variable_composite(*variables_address))
#else /* defined (USE_SMART_POINTER) */
			new Variable_composite(*variables_address)
#endif /* defined (USE_SMART_POINTER) */
			);
	}

	return (result);
}
