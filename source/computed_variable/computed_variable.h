/*******************************************************************************
FILE : computed_variable.h

LAST MODIFIED : 3 July 2003

DESCRIPTION :
Cmiss_variable's are expressions that are constructed for:
- display eg. difference between measured and calculated positions
- minimization eg. fitting by minimizing the difference between measured and
	calculated positions
- solution eg. solving a FEM variational formulation equals zero

A Cmiss_variable is a variable which is used in or is the result of a
calculation so can include element/xi, time, nodal parameters and element scale
factors.

When differentiating Cmiss_variable's the dependent and independent variables
are specified.  Any unspecified variables are constant.

Cmiss_variable's are able to be:
- evaluated at a point (specific choice of values for independent variables)
- differentiated at a point (specific choice of values for independent
	variables ie. not symbolic)
- composed ie. the results of one Cmiss_variable can replace independent
	variables for another Cmiss_variable

The shared interface to Cmiss_variable's is defined here, but there are
separate modules for specific types eg finite element, gradient and coordinate
transformations.

NOTES :
1 To allow compilation, caching and reuse, there should be a
	Cmiss_variable_calculation object to be used in the following manner
		calculation=CREATE(Cmiss_variable_calculation)(variable)
		j=1,p
			Cmiss_variable_calculation_set_variable_value(calculation,variable_ij
				value_ij),i=1,nj
			Cmiss_variable_calculation_set_derivative(calculation,derivative_ij),
				i=1,mj
			Cmiss_variable_calculation_evaluate(calculation)
		DESTROY(Cmiss_variable_calculation)(&calculation)
	The Cmiss_variable_calculation_set's are not changing the variable, just
	changing the overrides.  Assuming that the variable is not being changed,
	this allows calculation shortcuts to be made.  The
	Cmiss_variable_calculation object allows the cache to be outside of the
	Cmiss_variable.
2 To specify a derivative, a list of Cmiss_variable's is used eg
	d^3/dxi1dxi3^2 is represented by the list (xi1,xi3,xi3).
3 Have split off computed_variable_utilities and computed_variable_commands in
	order to focus on core of Cmiss_variable's.
4 To say that the results of one computed field should replace independent
	variables for a second computed field (composition of mathematical functions),
	use Cmiss_variable_set_source.
5 Specifying variable values in evaluate/is_defined is only for independent
	variables.  If a source has been specified for a variable, it is no longer
	independent.
6 Cmiss_variables are not invertable ie. if the results are specified for an
	evaluation, the dofs are not calculated.  Instead
	- Newton-Raphson can be done explicitly in the calling program or command file
	- use Cmiss_variable_inverse_set_type
7 A Cmiss_variable can be generic eg a mesh location or a time for any finite
	element computed variable.
8 To get a string representation of a variable use Cmiss_variable_get_value
	with a value of type string.

???DB.  Something needs doing about FE_value - has spread too far?  Currently
	set up in general/value.h .  Maybe just needs renaming?  Do as part of moving
	Cmiss_value to value?
???DB.  Do without component number?
==============================================================================*/
#if !defined (__CMISS_VARIABLE_H__)
#define __CMISS_VARIABLE_H__

#include "general/geometry.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "computed_variable/computed_value.h"

/*
Global macros
-------------
*/
#define CMISS_VARIABLE_IS_TYPE( variable_type ) \
	Cmiss_variable_ ## variable_type ## _is_type

#define PROTOTYPE_CMISS_VARIABLE_IS_TYPE_FUNCTION( variable_type ) \
int CMISS_VARIABLE_IS_TYPE(variable_type)( \
	Cmiss_variable_id variable) \
/***************************************************************************** \
LAST MODIFIED : 27 March 2003 \
\
DESCRIPTION : \
Returns a non-zero if <variable> is a #variable_type and zero otherwise. \
==============================================================================*/

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

DECLARE_LIST_TYPES(Cmiss_variable);

DECLARE_MANAGER_TYPES(Cmiss_variable);

struct Cmiss_variable_value;
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
A variable/value pair for specifying the value of the variable.
==============================================================================*/

DECLARE_LIST_TYPES(Cmiss_variable_value);

struct Cmiss_variable_package;
/*******************************************************************************
LAST MODIFIED : 8 April 2003

DESCRIPTION :
Allows sharing of Cmiss_variables with the same name.
==============================================================================*/

/*
Global functions
----------------
*/
Cmiss_variable_id CREATE(Cmiss_variable)(
	struct Cmiss_variable_package *package,char *name);
/*******************************************************************************
LAST MODIFIED : 8 April 2003

DESCRIPTION :
If there is a <package> and a Cmiss_variable in the <package> with the
specified <name> then the Cmiss_variable is returned.

Otherwise, an empty variable with specified <name> and no type, dependent
variables or independent variables is created and returned.  If there is a
<package> then the created Cmiss_variables is put in the <package>.  The
<name> is copied.

Each type of variable has its own "set_type" function.
==============================================================================*/

int DESTROY(Cmiss_variable)(Cmiss_variable_id *variable_address);
/*******************************************************************************
LAST MODIFIED : 21 January 2003

DESCRIPTION :
Frees memory/deaccess objects for Cmiss_variable at <*variable_address>.
==============================================================================*/

int Cmiss_variable_copy(Cmiss_variable_id destination,
	Cmiss_variable_id source);
/*******************************************************************************
LAST MODIFIED : 23 March 2003

DESCRIPTION :
Copies the type and contents from <source> to <destination>.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Cmiss_variable);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Cmiss_variable);

PROTOTYPE_LIST_FUNCTIONS(Cmiss_variable);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Cmiss_variable,name,char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(Cmiss_variable,name,char *);
PROTOTYPE_MANAGER_FUNCTIONS(Cmiss_variable);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Cmiss_variable,name,char *);

int Cmiss_variable_same_variable(Cmiss_variable_id variable_1,
	Cmiss_variable_id variable_2);
/*******************************************************************************
LAST MODIFIED : 31 January 2003

DESCRIPTION :
Returns nonzero if <variable_1> and <variable_2> are the same variable (eg. the
value at node 10 for the finite element field bob) and zero otherwise.
==============================================================================*/

int Cmiss_variable_overlap(Cmiss_variable_id variable_1,
	Cmiss_variable_id variable_2);
/*******************************************************************************
LAST MODIFIED : 31 January 2003

DESCRIPTION :
Returns nonzero if <variable_1> and <variable_2> overlap (eg d/ds1 and all
values) and zero otherwise.
==============================================================================*/

int Cmiss_variable_is_independent_variable_of(
	Cmiss_variable_id dependent_variable,
	Cmiss_variable_id independent_variable);
/*******************************************************************************
LAST MODIFIED : 31 January 2003

DESCRIPTION :
Returns nonzero if <independent_variable> is an independent variable of
<dependent_variable> and zero otherwise.
==============================================================================*/

char *Cmiss_variable_get_type_id_string(Cmiss_variable_id variable);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Returns the string which identifies the type.  The calling function must not
DEALLOCATE the returned string.
==============================================================================*/

int Cmiss_variable_get_value_type(Cmiss_variable_id variable,
	Cmiss_value_id type);
/*******************************************************************************
LAST MODIFIED : 29 January 2003

DESCRIPTION :
Sets the <type> to be the same as the type of the results of <variable>, but
does not set/calculate the actual value.
==============================================================================*/

int Cmiss_variable_same_value_type(Cmiss_variable_id variable_1,
	Cmiss_variable_id variable_2);
/*******************************************************************************
LAST MODIFIED : 31 January 2003

DESCRIPTION :
Returns nonzero if <variable_1> and <variable_2> have the same value type and
zero otherwise.
==============================================================================*/

int Cmiss_variable_set_independent_variable_source_variable(
	Cmiss_variable_id variable,
	Cmiss_variable_id independent_variable,
	Cmiss_variable_id source_variable);
/*******************************************************************************
LAST MODIFIED : 3 February 2003

DESCRIPTION :
Sets the values for <independent_variable> of the <variable> to come from the
<source_variable>.  The types of the values for <independent_variable> and
<source_variable> must match.  If <source_variable> is NULL then a previous
setting of the source variable for <independent_variable> is cleared.
==============================================================================*/

int Cmiss_variable_set_independent_variable_value(
	Cmiss_variable_id variable,
	Cmiss_variable_id independent_variable,
	Cmiss_value_id value);
/*******************************************************************************
LAST MODIFIED : 3 February 2003

DESCRIPTION :
Sets the <value> of the <independent_variable> for the <variable>.
==============================================================================*/

int Cmiss_variable_get_independent_variable_value(
	Cmiss_variable_id variable,
	Cmiss_variable_id independent_variable,
	Cmiss_value_id value);
/*******************************************************************************
LAST MODIFIED : 3 February 2003

DESCRIPTION :
Gets the <value> of the <independent_variable> for the <variable>.  If the
<value> has no type, then the default type for the <independent_variable> is
used otherwise a representation in <value>'s type is attempted.
==============================================================================*/

int Cmiss_variable_get_set_independent_variable_value(
	Cmiss_variable_id variable,
	Cmiss_variable_id independent_variable,
	Cmiss_value_id value);
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Swaps the <value> and the current value for the <independent_variable> of the
<variable>.
==============================================================================*/

int Cmiss_variable_depends_on_Cmiss_variable(
	Cmiss_variable_id variable,Cmiss_variable_id other_variable);
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Returns nonzero if the two variables are identical or if <other_variable> needs
to be evaluated in order to evaluate <variable>.

This function is used by MANAGER_COPY_WITHOUT_IDENTIFIER to ensure that the
destination variable will not depend on itself, thus leading to an infinite
loop, and also to prevent copying a variable over itself.

Parts of the program receiving manager messages for Cmiss_variables should
call this function with the variable=variable in use and
other_variable=modified variable to determine if the variable in use needs
updating.
==============================================================================*/

int Cmiss_variable_depends_on_Cmiss_variable_in_list(
	Cmiss_variable_id variable,
	struct LIST(Cmiss_variable) *variable_list);
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Returns nonzero if <variable> depends on any variable in <variable_list>.
==============================================================================*/

struct Cmiss_variable_value *CREATE(Cmiss_variable_value)(
	Cmiss_variable_id variable,Cmiss_value_id value);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Creates a <variable>/<value> pair.
==============================================================================*/

int DESTROY(Cmiss_variable_value)(
	struct Cmiss_variable_value **variable_value_address);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Frees memory/deaccess objects for Cmiss_variable_value at
<*variable_value_address>.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Cmiss_variable_value);

PROTOTYPE_LIST_FUNCTIONS(Cmiss_variable_value);

Cmiss_value_id Cmiss_variable_value_get_value(
	struct Cmiss_variable_value *variable_value);
/*******************************************************************************
LAST MODIFIED : 7 February 2003

DESCRIPTION :
Returns the value for the <variable_value>.
==============================================================================*/

Cmiss_variable_id Cmiss_variable_value_get_variable(
	struct Cmiss_variable_value *variable_value);
/*******************************************************************************
LAST MODIFIED : 7 February 2003

DESCRIPTION :
Returns the variable for the <variable_value>.
==============================================================================*/

int Cmiss_variable_get_set_independent_variable_value_iterator(
	struct Cmiss_variable_value *independent_variable_value,
	void *dependent_variable_void);
/*******************************************************************************
LAST MODIFIED : 9 July 2003

DESCRIPTION :
List iterator function that swaps the <independent_variable_value> with the
current one for the <dependent_variable_void>.  Returns true whether or not the
<independent_variable_value> is for the <dependent_variable_void>.
==============================================================================*/

int Cmiss_variable_is_defined(Cmiss_variable_id variable,
	struct LIST(Cmiss_variable_value) *values);
/*******************************************************************************
LAST MODIFIED : 4 February 2003

DESCRIPTION :
Returns true if <variable> can be calculated with the specified <values>
over-riding, but not setting, the current values.

???DB.  Could use evaluate, but that would be less efficient and currently
	would write error messages if it failed
==============================================================================*/

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

struct Cmiss_variable_package *CREATE(Cmiss_variable_package)(void);
/*******************************************************************************
LAST MODIFIED : 8 April 2003

DESCRIPTION :
Creates a Cmiss_variable_package which is used for sharing of
Cmiss_variables with the same name.

???DB.  What about FE_ managers?  Do FE computed variables need there own
	package (part of set_type_ ?)
==============================================================================*/

int DESTROY(Cmiss_variable_package)(
	struct Cmiss_variable_package **package_address);
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Frees memory/deaccess objects in Cmiss_variable_package at <*package_address>.
==============================================================================*/
#endif /* !defined (__CMISS_VARIABLE_H__) */
