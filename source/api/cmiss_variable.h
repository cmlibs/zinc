/*******************************************************************************
FILE : api/cmiss_variable.h

LAST MODIFIED : 30 July 2003

DESCRIPTION :
The public interface to the Cmiss_variable object.
==============================================================================*/
#ifndef __API_CMISS_VARIABLE_H__
#define __API_CMISS_VARIABLE_H__

#include "api/cmiss_value.h"
/* If this is going to be in the API then it needs to have an interface there */
#include "general/object.h"
#include "general/list.h"

/*
Global types
------------
*/

typedef struct Cmiss_variable *Cmiss_variable_id;
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
An object that can be evaluated from and differentiated with respect to other
variables.  It can be displayed, minimized or used in an equation.
==============================================================================*/

struct Cmiss_variable_value;
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
A variable/value pair for specifying the value of the variable.
==============================================================================*/

DECLARE_LIST_TYPES(Cmiss_variable_value);

/*
Global functions
----------------
*/

/* SAB Temporarily mangle the external name until we decide to mangle the 
	internal one instead */
Cmiss_variable_id CREATE(Cmiss_variable_API)(char *name);
/*******************************************************************************
LAST MODIFIED : 30 July 2003

DESCRIPTION :
Creates a Cmiss_variable with the supplied <name>.
==============================================================================*/

struct Cmiss_variable_value *CREATE(Cmiss_variable_value)(
	Cmiss_variable_id variable,Cmiss_value_id value);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Creates a <variable>/<value> pair.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Cmiss_variable_value);

PROTOTYPE_LIST_FUNCTIONS(Cmiss_variable_value);

int Cmiss_variable_evaluate(struct Cmiss_variable_value *variable_value,
	struct LIST(Cmiss_variable_value) *values);
/*******************************************************************************
LAST MODIFIED : 4 February 2003

DESCRIPTION :
Calculates the <variable_value> with the specified <values> over-riding, but not
setting, the current values.
==============================================================================*/

int Cmiss_variable_evaluate_derivative(
	Cmiss_variable_id dependent_variable,int order,
	Cmiss_variable_id *independent_variables,
	struct LIST(Cmiss_variable_value) *values,
	Cmiss_value_id derivative_matrix);
/*******************************************************************************
LAST MODIFIED : 7 March 2003

DESCRIPTION :
Evaluates the <derivative_matrix> for the <order> degree derivative of
<dependent_variable> with respect to the <independent_variables>.
==============================================================================*/
#endif /* __API_CMISS_VARIABLE_H__ */
