/*******************************************************************************
FILE : api/cmiss_value_element_xi.h

LAST MODIFIED : 30 July 2003

DESCRIPTION :
The public interface to the Cmiss_value_element_xi object.
==============================================================================*/
#ifndef __API_CMISS_VALUE_ELEMENT_XI_H__
#define __API_CMISS_VALUE_ELEMENT_XI_H__

#include "api/cmiss_value.h"
#include "api/cmiss_finite_element.h"
#include "general/object.h"

Cmiss_value_id CREATE(Cmiss_value_element_xi)(int dimension,
	struct Cmiss_element *element, float *xi_values);
/*******************************************************************************
LAST MODIFIED : 30 July 2003

DESCRIPTION :
Creates a Cmiss_value which contains a element_xi of values.
==============================================================================*/

#endif /* __API_CMISS_VALUE_ELEMENT_XI_H__ */
