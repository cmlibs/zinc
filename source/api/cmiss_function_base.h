/*******************************************************************************
FILE : api/cmiss_function_base.h

LAST MODIFIED : 5 March 2004

DESCRIPTION :
==============================================================================*/
#ifndef __API_CMISS_FUNCTION_BASE_H__
#define __API_CMISS_FUNCTION_BASE_H__

// needs to be the same as typedef in computed_variable/function_base.hpp, but
//   will not compile if differ
typedef double Scalar;

/*
Global types
------------
*/
typedef struct Cmiss_function * Cmiss_function_id;
/*******************************************************************************
LAST MODIFIED : 23 February 2004

DESCRIPTION :
An identifier for a function.
==============================================================================*/

typedef struct Cmiss_function_variable * Cmiss_function_variable_id;
/*******************************************************************************
LAST MODIFIED : 23 February 2004

DESCRIPTION :
An identifier for a variable.
==============================================================================*/

#endif /* __API_CMISS_FUNCTION_BASE_H__ */
