/*******************************************************************************
FILE : api/cmiss_value_fe_value.h

LAST MODIFIED : 10 September 2003

DESCRIPTION :
The public interface to the Cmiss_value_fe_value object.
==============================================================================*/
#ifndef __API_CMISS_VALUE_FE_VALUE_H__
#define __API_CMISS_VALUE_FE_VALUE_H__

#include "api/cmiss_value.h"
/* SAB These will need to have an equivalent in the api. */
#include "general/object.h"
#include "general/value.h"

Cmiss_value_id CREATE(Cmiss_value_FE_value)(FE_value value);
/*******************************************************************************
LAST MODIFIED : 20 August 2003

DESCRIPTION :
Creates a Cmiss_value which contains an fe_value.
==============================================================================*/

Cmiss_value_id CREATE(Cmiss_value_FE_value_vector)(int number_of_values,
	FE_value *values);
/*******************************************************************************
LAST MODIFIED : 20 August 2003

DESCRIPTION :
Creates a Cmiss_value which contains a vector of fe_values.
==============================================================================*/

Cmiss_value_id CREATE(Cmiss_value_FE_value_matrix)(int number_of_rows,
	int number_of_columns, FE_value *values);
/*******************************************************************************
LAST MODIFIED : 10 September 2003

DESCRIPTION :
Creates a Cmiss_value which contains a vector of fe_values.
==============================================================================*/

#endif /* __API_CMISS_VALUE_FE_VALUE_H__ */
