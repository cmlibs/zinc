/*******************************************************************************
FILE : computed_variable_identity.h

LAST MODIFIED : 17 July 2003

DESCRIPTION :
Implements the identity computed variable - takes a variable to itself.  Used
for calculating derivatives.
==============================================================================*/
#if !defined (__CMISS_VARIABLE_IDENTITY_H__)
#define __CMISS_VARIABLE_IDENTITY_H__

#include "computed_variable/computed_variable.h"

/*
Global functions
----------------
*/
int Cmiss_variable_identity_set_type(Cmiss_variable_id identity,
	Cmiss_variable_id variable);
/*******************************************************************************
LAST MODIFIED : 17 July 2003

DESCRIPTION :
Sets <identity> to be a variable whose dependent variable is <variable> and
whose independent variable is <variable>.
==============================================================================*/

PROTOTYPE_CMISS_VARIABLE_IS_TYPE_FUNCTION(identity);

#endif /* !defined (__CMISS_VARIABLE_IDENTITY_H__) */
