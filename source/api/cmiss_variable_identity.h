/*******************************************************************************
FILE : api/cmiss_variable_identity.h

LAST MODIFIED : 1 September 2003

DESCRIPTION :
The public interface to the Cmiss_variable_identity object.
==============================================================================*/
#ifndef __API_CMISS_VARIABLE_IDENTITY_H__
#define __API_CMISS_VARIABLE_IDENTITY_H__

/* If this is going to be in the API then it needs to have an interface there */
#include "general/object.h"
#include "api/cmiss_variable.h"

Cmiss_variable_id CREATE(Cmiss_variable_identity)(char *name,
	Cmiss_variable_id variable);
/*******************************************************************************
LAST MODIFIED : 1 September 2003

DESCRIPTION :
Creates a Cmiss_variable which represents an identity.
==============================================================================*/
#endif /* __API_CMISS_VARIABLE_IDENTITY_H__ */
