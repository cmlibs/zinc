/*******************************************************************************
FILE : computed_value_finite_element.h

LAST MODIFIED : 11 July 2003

DESCRIPTION :
Implements computed values which interface to finite elements:
- element_xi
==============================================================================*/
#if !defined (__CMISS_VALUE_FINITE_ELEMENT_H__)
#define __CMISS_VALUE_FINITE_ELEMENT_H__

#include "finite_element/finite_element.h"
#include "computed_variable/computed_value.h"

/*
Global functions
----------------
*/
int Cmiss_value_element_xi_set_type(Cmiss_value_id value,int dimension,
	struct FE_element *element,FE_value *xi);
/*******************************************************************************
LAST MODIFIED : 11 July 2003

DESCRIPTION :
Makes <value> of type element_xi and sets its <dimension>, <element> and <xi).
<dimension> must be positive or <element> must be non-NULL.  If <dimension> is
positive and <element> is non-NULL then <dimension> should equal the dimension
of the element.  After success, the <value> is responsible for DEALLOCATEing
<xi>.

???DB.  Assuming that the <element> knows its FE_region (can get manager)
==============================================================================*/

PROTOTYPE_CMISS_VALUE_IS_TYPE_FUNCTION(element_xi);

int Cmiss_value_element_xi_get_type(Cmiss_value_id value,int *dimension_address,
	struct FE_element **element_address,FE_value **xi_address);
/*******************************************************************************
LAST MODIFIED : 11 July 2003

DESCRIPTION :
If <value> is of type element_xi, gets its <*dimension_address>,
<*element_address> and <*xi_address).

The calling program must not DEALLOCATE the returned <*xi_address>.
==============================================================================*/
#endif /* !defined (__CMISS_VALUE_FINITE_ELEMENT_H__) */
