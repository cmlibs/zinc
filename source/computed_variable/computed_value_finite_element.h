/*******************************************************************************
FILE : computed_value_finite_element.h

LAST MODIFIED : 19 February 2003

DESCRIPTION :
Implements computed values which interface to finite elements:
- element_xi
==============================================================================*/
#if !defined (COMPUTED_VALUE_FINITE_ELEMENT_H)
#define COMPUTED_VALUE_FINITE_ELEMENT_H

#include "finite_element/finite_element.h"
#include "computed_variable/computed_value.h"

/*
Global functions
----------------
*/
int Computed_value_set_type_element_xi(struct Computed_value *value,
	struct FE_element *element,FE_value *xi);
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
Makes <value> of type element_xi and sets its <element> and <xi).  After
success, the <value> is responsible for DEALLOCATEing <xi>.

???DB.  Assuming that the <element> knows its FE_region (can get manager)
???DB.  Is it necessary to add a dimension or insist on non-NULL element?
==============================================================================*/

PROTOTYPE_COMPUTED_VALUE_IS_TYPE_FUNCTION(element_xi);

int Computed_value_get_type_element_xi(struct Computed_value *value,
	struct FE_element **element_address,FE_value **xi_address);
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
If <value> is of type element_xi, gets its <*element_address> and <*xi_address).

The calling program must not DEALLOCATE the returned <*xi_address>.
==============================================================================*/
#endif /* !defined (COMPUTED_VALUE_FINITE_ELEMENT_H) */
