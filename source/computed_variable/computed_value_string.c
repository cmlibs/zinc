/*******************************************************************************
FILE : computed_value_string.c

LAST MODIFIED : 25 July 2003

DESCRIPTION :
The beginnings of a Cmiss_value_string type, NOT IMPLEMENTED
==============================================================================*/
#include "computed_variable/computed_value.h"
#include "computed_variable/computed_value_private.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Module constants
----------------
*/
static char Cmiss_value_string_type_string[]="String";

int Cmiss_value_string_set_type(Cmiss_value_id value,char *string);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Makes <value> of type string and sets its <string>.  After success, the <value>
is responsible for DEALLOCATEing <string>.
==============================================================================*/

DECLARE_CMISS_VALUE_IS_TYPE_FUNCTION(string)

int Cmiss_value_string_get_type(Cmiss_value_id value,char **string_address);
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
If <value> is of type string, gets its <*string_address>.

The calling program must not DEALLOCATE the returned <*string_address>.
==============================================================================*/
