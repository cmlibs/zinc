/*******************************************************************************
FILE : api/cmiss_variable_new_input_composite.cpp

LAST MODIFIED : 12 January 2004

DESCRIPTION :
The public interface to the Cmiss_variable_new_input composite objects.
==============================================================================*/

#include <new>
#include "api/cmiss_variable_new_input_composite.h"
#include "computed_variable/variable_input_composite.hpp"

#if defined (GENERALIZE_COMPOSITE_INPUT)
#include "computed_variable/variable_input_composite_union.hpp"
#endif // defined (GENERALIZE_COMPOSITE_INPUT)

/*
Global functions
----------------
*/
Cmiss_variable_new_input_id Cmiss_variable_new_input_composite_create(
	Cmiss_variable_new_input_list_id inputs)
/*******************************************************************************
LAST MODIFIED : 12 January 2004

DESCRIPTION :
Creates a Cmiss_variable_new_input composite with the supplied <inputs>.
==============================================================================*/
{
	Cmiss_variable_new_input_id result;
	std::list<Variable_input_handle> *inputs_address;

	result=0;
	if (inputs_address=reinterpret_cast<std::list<Variable_input_handle> *>(
    inputs))
	{
		result=reinterpret_cast<Cmiss_variable_new_input_id>(
#if defined (USE_SMART_POINTER)
			new
#if defined (GENERALIZE_COMPOSITE_INPUT)
			Variable_input_composite_union_handle
#else // defined (GENERALIZE_COMPOSITE_INPUT)
			Variable_input_composite_handle
#endif // defined (GENERALIZE_COMPOSITE_INPUT)
			(
#endif /* defined (USE_SMART_POINTER) */
			new
#if defined (GENERALIZE_COMPOSITE_INPUT)
			Variable_input_composite_union
#else // defined (GENERALIZE_COMPOSITE_INPUT)
			Variable_input_composite
#endif // defined (GENERALIZE_COMPOSITE_INPUT)
			(*inputs_address)
#if defined (USE_SMART_POINTER)
			)
#endif /* defined (USE_SMART_POINTER) */
			);
	}

	return (result);
}
