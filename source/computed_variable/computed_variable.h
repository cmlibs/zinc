/*******************************************************************************
FILE : computed_variable.h

LAST MODIFIED : 24 March 2003

DESCRIPTION :
Computed_variable's are expressions that are constructed for:
- display eg. difference between measured and calculated positions
- minimization eg. fitting by minimizing the difference between measured and
	calculated positions
- solution eg. solving a FEM variational formulation equals zero

A Computed_variable is a variable which is used in or is the result of a
calculation so can include element/xi, time, nodal parameters and element scale
factors.

When differentiating Computed_variable's the dependent and independent variables
are specified.  Any unspecified variables are constant.

Computed_variable's are able to be:
- evaluated at a point (specific choice of values for independent variables)
- differentiated at a point (specific choice of values for independent
	variables ie. not symbolic)
- composed ie. the results of one Computed_variable can replace independent
	variables for another Computed_variable

The shared interface to Computed_variable's is defined here, but there are
separate modules for specific types eg finite element, gradient and coordinate
transformations.

NOTES :
1 To allow compilation, caching and reuse, there should be a
	Computed_variable_calculation object to be used in the following manner
		calculation=CREATE(Computed_variable_calculation)(variable)
		j=1,p
			Computed_variable_calculation_set_variable_value(calculation,variable_ij
				value_ij),i=1,nj
			Computed_variable_calculation_set_derivative(calculation,derivative_ij),
				i=1,mj
			Computed_variable_calculation_evaluate(calculation)
		DESTROY(Computed_variable_calculation)(&calculation)
	The Computed_variable_calculation_set's are not changing the variable, just
	changing the overrides.  Assuming that the variable is not being changed,
	this allows calculation shortcuts to be made.  The
	Computed_variable_calculation object allows the cache to be outside of the
	Computed_variable.
2 To specify a derivative, a list of Computed_variable's is used eg
	d^3/dxi1dxi3^2 is represented by the list (xi1,xi3,xi3).
3 Have split off computed_variable_utilities and computed_variable_commands in
	order to focus on core of Computed_variable's.
4 To say that the results of one computed field should replace independent
	variables for a second computed field (composition of mathematical functions),
	use Computed_variable_set_source.
5 Specifying variable values in evaluate/is_defined is only for independent
	variables.  If a source has been specified for a variable, it is no longer
	independent.
6 Computed_variables are not invertable ie. if the results are specified for an
	evaluation, the dofs are not calculated.  Instead
	- Newton-Raphson can be done explicitly in the calling program or command file
	- use Computed_variable_set_type_inverse
7 A Computed_variable can be generic eg a mesh location or a time for any finite
	element computed variable.
8 To get a string representation of a variable use Computed_variable_get_value
	with a value of type string.

???DB.  Something needs doing about FE_value - has spread too far?  Currently
	set up in general/value.h .  Maybe just needs renaming?  Do as part of moving
	computed_value to value?
???DB.  Do without component number?
==============================================================================*/
#if !defined (COMPUTED_VARIABLE_H)
#define COMPUTED_VARIABLE_H

#include "general/geometry.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "computed_variable/computed_value.h"

/*
Global macros
-------------
*/
#define PROTOTYPE_COMPUTED_VARIABLE_IS_TYPE_FUNCTION( variable_type ) \
int Computed_variable_is_type_ ## variable_type ( \
	struct Computed_variable *variable) \
/***************************************************************************** \
LAST MODIFIED : 24 March 2003 \
\
DESCRIPTION : \
Returns a non-zero if <variable> is a #variable_type and zero otherwise. \
==============================================================================*/

/*
Global types
------------
*/
struct Computed_variable;
/*******************************************************************************
LAST MODIFIED : 21 January 2003

DESCRIPTION :
An object that can be evaluated from and differentiated with respect to other
variables.  It can be displayed, minimized or used in an equation.
==============================================================================*/

DECLARE_LIST_TYPES(Computed_variable);

DECLARE_MANAGER_TYPES(Computed_variable);

struct Computed_variable_value;
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
A variable/value pair for specifying the value of the variable.
==============================================================================*/

DECLARE_LIST_TYPES(Computed_variable_value);

struct Computed_variable_package;
/*******************************************************************************
LAST MODIFIED : 21 January 2003

DESCRIPTION :
Contains all information for editing and maintaining Computed_variables,
including the MANAGER(Computed_variable).

???RC Make macro PACKAGE(Computed_variable) etc.?
==============================================================================*/

/*
Global functions
----------------
*/
int Computed_value_set_type_derivative_matrix(struct Computed_value *value,
	struct Computed_variable *dependent_variable,int order,
	struct Computed_variable **independent_variables,
	struct Computed_value *fe_value_matrix);
/*******************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
Makes <value> of type derivative_matrix and sets its <fe_value_matrix>,
<dependent_variable>, <order> and <independent_variables>.  This function
ACCESSes the <dependent_variable> and <independent_variables>.  After success,
the <value> is responsible for DESTROY/DEACCESSing <fe_value_matrix>,
<dependent_variable> and <independent_variables>.

The number of rows is the number of values for the <dependent_variable>.  The
number of columns is:
	product(1+number_of_values,for each independent_variable)-1
The columns are:
	d(dependent_variable)/d(independent_variables[0])
	d(dependent_variable)/d(independent_variables[1])
	d2(dependent_variable)/d(independent_variables[0])d(independent_variables[1])
	d(dependent_variable)/d(independent_variables[2])
	d2(dependent_variable)/d(independent_variables[0])d(independent_variables[2])
	d2(dependent_variable)/d(independent_variables[1])d(independent_variables[2])
	d3(dependent_variable)/d(independent_variables[0])d(independent_variables[1])
		d(independent_variables[2])
	...
	d(dependent_variable)/d(independent_variables[order-1])
	d2(dependent_variable)/d(independent_variables[0])
		d(independent_variables[order-1])
	d2(dependent_variable)/d(independent_variables[1])
		d(independent_variables[order-1])
	d3(dependent_variable)/d(independent_variables[0])d(independent_variables[1])
		d(independent_variables[order-1])
	d2(dependent_variable)/d(independent_variables[2])
		d(independent_variables[order-1])
	d3(dependent_variable)/d(independent_variables[0])d(independent_variables[2])
		d(independent_variables[order-1])
	d3(dependent_variable)/d(independent_variables[1])d(independent_variables[2])
		d(independent_variables[order-1])
	d4(dependent_variable)/d(independent_variables[0])d(independent_variables[1])
		d(independent_variables[2])d(independent_variables[order-1])
	...
	d{order}(dependent_variable)/d(independent_variables[0])
		d(independent_variables[1]) ... d(independent_variables[order-1])
==============================================================================*/

PROTOTYPE_COMPUTED_VALUE_IS_TYPE_FUNCTION(derivative_matrix);

int Computed_value_get_type_derivative_matrix(struct Computed_value *value,
	struct Computed_variable **dependent_variable_address,int *order_address,
	struct Computed_variable ***independent_variables_address,
	struct Computed_value **fe_value_matrix_address);
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
If <value> is of type derivative_matrix, gets its <*fe_value_matrix_address>,
<*dependent_variable_address>, <*order_address> and
<*independent_variables_address>.

The calling program must not DEALLOCATE the returned structures.
==============================================================================*/

struct Computed_variable *CREATE(Computed_variable)(char *name);
/*******************************************************************************
LAST MODIFIED : 27 January 2003

DESCRIPTION :
Creates an empty variable with specified <name> and no type, dependent variables
or independent variables.  Each type of variable has its own "set_type"
function.  The <name> is copied.
==============================================================================*/

int DESTROY(Computed_variable)(struct Computed_variable **variable_address);
/*******************************************************************************
LAST MODIFIED : 21 January 2003

DESCRIPTION :
Frees memory/deaccess objects for Computed_variable at <*variable_address>.
==============================================================================*/

int Computed_variable_copy(struct Computed_variable *destination,
	struct Computed_variable *source);
/*******************************************************************************
LAST MODIFIED : 23 March 2003

DESCRIPTION :
Copies the type and contents from <source> to <destination>.
==============================================================================*/

#if defined (OLD_CODE)
struct Computed_variable *Computed_variable_duplicate(
	struct Computed_variable *variable);
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
Returns a copy of the <variable>.
==============================================================================*/
#endif /* defined (OLD_CODE) */

PROTOTYPE_OBJECT_FUNCTIONS(Computed_variable);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Computed_variable);

PROTOTYPE_LIST_FUNCTIONS(Computed_variable);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Computed_variable,name,char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(Computed_variable,name,char *);
PROTOTYPE_MANAGER_FUNCTIONS(Computed_variable);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Computed_variable,name,char *);

int Computed_variable_same_variable(struct Computed_variable *variable_1,
	struct Computed_variable *variable_2);
/*******************************************************************************
LAST MODIFIED : 31 January 2003

DESCRIPTION :
Returns nonzero if <variable_1> and <variable_2> are the same variable (eg. the
value at node 10 for the finite element field bob) and zero otherwise.
==============================================================================*/

int Computed_variable_overlap(struct Computed_variable *variable_1,
	struct Computed_variable *variable_2);
/*******************************************************************************
LAST MODIFIED : 31 January 2003

DESCRIPTION :
Returns nonzero if <variable_1> and <variable_2> overlap (eg d/ds1 and all
values) and zero otherwise.
==============================================================================*/

int Computed_variable_is_independent_variable_of(
	struct Computed_variable *dependent_variable,
	struct Computed_variable *independent_variable);
/*******************************************************************************
LAST MODIFIED : 31 January 2003

DESCRIPTION :
Returns nonzero if <independent_variable> is an independent variable of
<dependent_variable> and zero otherwise.
==============================================================================*/

char *Computed_variable_get_type_id_string(struct Computed_variable *variable);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Returns the string which identifies the type.  The calling function must not
DEALLOCATE the returned string.
==============================================================================*/

int Computed_variable_get_value_type(struct Computed_variable *variable,
	struct Computed_value *type);
/*******************************************************************************
LAST MODIFIED : 29 January 2003

DESCRIPTION :
Sets the <type> to be the same as the type of the results of <variable>, but
does not set/calculate the actual value.
==============================================================================*/

int Computed_variable_same_value_type(struct Computed_variable *variable_1,
	struct Computed_variable *variable_2);
/*******************************************************************************
LAST MODIFIED : 31 January 2003

DESCRIPTION :
Returns nonzero if <variable_1> and <variable_2> have the same value type and
zero otherwise.
==============================================================================*/

int Computed_variable_set_independent_variable_source_variable(
	struct Computed_variable *variable,
	struct Computed_variable *independent_variable,
	struct Computed_variable *source_variable);
/*******************************************************************************
LAST MODIFIED : 3 February 2003

DESCRIPTION :
Sets the values for <independent_variable> of the <variable> to come from the
<source_variable>.  The types of the values for <independent_variable> and
<source_variable> must match.  If <source_variable> is NULL then a previous
setting of the source variable for <independent_variable> is cleared.
==============================================================================*/

int Computed_variable_set_independent_variable_value(
	struct Computed_variable *variable,
	struct Computed_variable *independent_variable,
	struct Computed_value *value);
/*******************************************************************************
LAST MODIFIED : 3 February 2003

DESCRIPTION :
Sets the <value> of the <independent_variable> for the <variable>.
==============================================================================*/

int Computed_variable_get_independent_variable_value(
	struct Computed_variable *variable,
	struct Computed_variable *independent_variable,
	struct Computed_value *value);
/*******************************************************************************
LAST MODIFIED : 3 February 2003

DESCRIPTION :
Gets the <value> of the <independent_variable> for the <variable>.  If the
<value> has no type, then the default type for the <independent_variable> is
used otherwise a representation in <value>'s type is attempted.
==============================================================================*/

int Computed_variable_depends_on_Computed_variable(
	struct Computed_variable *variable,struct Computed_variable *other_variable);
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Returns nonzero if the two variables are identical or if <other_variable> needs
to be evaluated in order to evaluate <variable>.

This function is used by MANAGER_COPY_WITHOUT_IDENTIFIER to ensure that the
destination variable will not depend on itself, thus leading to an infinite
loop, and also to prevent copying a variable over itself.

Parts of the program receiving manager messages for Computed_variables should
call this function with the variable=variable in use and
other_variable=modified variable to determine if the variable in use needs
updating.
==============================================================================*/

int Computed_variable_depends_on_Computed_variable_in_list(
	struct Computed_variable *variable,
	struct LIST(Computed_variable) *variable_list);
/*******************************************************************************
LAST MODIFIED : 23 January 2002

DESCRIPTION :
Returns nonzero if <variable> depends on any variable in <variable_list>.
==============================================================================*/

struct Computed_variable_value *CREATE(Computed_variable_value)(
	struct Computed_variable *variable,struct Computed_value *value);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Creates a <variable>/<value> pair.
==============================================================================*/

int DESTROY(Computed_variable_value)(
	struct Computed_variable_value **variable_value_address);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Frees memory/deaccess objects for Computed_variable_value at
<*variable_value_address>.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Computed_variable_value);

PROTOTYPE_LIST_FUNCTIONS(Computed_variable_value);

struct Computed_value *Computed_variable_value_get_value(
	struct Computed_variable_value *variable_value);
/*******************************************************************************
LAST MODIFIED : 7 February 2003

DESCRIPTION :
Returns the value for the <variable_value>.
==============================================================================*/

struct Computed_variable *Computed_variable_value_get_variable(
	struct Computed_variable_value *variable_value);
/*******************************************************************************
LAST MODIFIED : 7 February 2003

DESCRIPTION :
Returns the variable for the <variable_value>.
==============================================================================*/

int Computed_variable_is_defined(struct Computed_variable *variable,
	struct LIST(Computed_variable_value) *values);
/*******************************************************************************
LAST MODIFIED : 4 February 2003

DESCRIPTION :
Returns true if <variable> can be calculated with the specified <values>
over-riding, but not setting, the current values.

???DB.  Could use evaluate, but that would be less efficient and currently
	would write error messages if it failed
==============================================================================*/

int Computed_variable_evaluate(struct Computed_variable_value *variable_value,
	struct LIST(Computed_variable_value) *values);
/*******************************************************************************
LAST MODIFIED : 4 February 2003

DESCRIPTION :
Calculates the <variable_value> with the specified <values> over-riding, but not
setting, the current values.
==============================================================================*/

int Computed_variable_evaluate_derivative(
	struct Computed_variable *dependent_variable,int order,
	struct Computed_variable **independent_variables,
	struct LIST(Computed_variable_value) *values,
	struct Computed_value *derivative_matrix);
/*******************************************************************************
LAST MODIFIED : 7 March 2003

DESCRIPTION :
Evaluates the <derivative_matrix> for the <order> degree derivative of
<dependent_variable> with respect to the <independent_variables>.
==============================================================================*/

int Computed_variable_set_type_derivative(
	struct Computed_variable *derivative,
	struct Computed_variable *dependent_variable,int order,
	struct Computed_variable **independent_variables);
/*******************************************************************************
LAST MODIFIED : 21 March 2003

DESCRIPTION :
Sets <derivative> to be the derivative of the <dependent_variable> with respect
to the <independent_variables>.  This function ACCESSes the <dependent_variable>
and <independent_variables>.  After success, the <derivative> is responsible for
DEACCESS/DEALLOCATEing <dependent_variable> and <independent_variables>.
==============================================================================*/

int Computed_variable_set_type_divergence(
	struct Computed_variable *divergence,
	struct Computed_variable *dependent_variable,
	struct Computed_variable *independent_variable);
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
Sets <divergence> to be the divergence of the <dependent_variable> with respect
to the <independent_variable>.
==============================================================================*/

int Computed_variable_set_type_inverse(
	struct Computed_variable *inverse_variable,
	struct Computed_variable *variable,
	struct LIST(Computed_variable) *dependent_variables);
/*******************************************************************************
LAST MODIFIED : 24 January 2003

DESCRIPTION :
Sets <inverse_variable> to be the inverse of the <variable>.  Its independent
variables are the dependent variables of the <variable> and its
<dependent_variables> are independent variables of the <variable>.

???DB.  Just one <independent_variable>?
==============================================================================*/

struct Coordinate_system *Computed_variable_get_coordinate_system(
	struct Computed_variable *variable);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Returns the coordinate system <variable> is to be interpreted under.  See
function Computed_variable_set_coordinate_system for further details.

???DB.  Not sure if coordinate system is a property of a Computed_variable
==============================================================================*/

int Computed_variable_set_coordinate_system(struct Computed_variable *variable,
	struct Coordinate_system *coordinate_system);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Sets the coordinate system <variable> is to be interpreted under.  Note the
careful choice of words here: the coordinate system merely tells the rest of the
program what needs to be done to transform the variable values into any other
coordinate system.  It does not have to be "correct" for the values in the
variable, eg. you can describe prolate spheroidal values as RC to "open out" the
heart model.

???DB.  Not sure if coordinate system is a property of a Computed_variable
==============================================================================*/

int Computed_variable_set_read_only(struct Computed_variable *variable);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Marks <variable> as read-only, telling the program that the user is not
permitted to modify or destroy it.

???DB.  Is this part of the package?
==============================================================================*/

int Computed_variable_set_read_write(struct Computed_variable *variable);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Clears read-only status of <variable>, telling the program that the user is
allowed to modify and destroy it.

???DB.  Is this part of the package?
==============================================================================*/

struct Computed_variable *Computed_variable_manager_get_component_wrapper(
	struct MANAGER(Computed_variable) *computed_variable_manager,
	struct Computed_variable *variable,int component_no);
/*******************************************************************************
LAST MODIFIED : 3 December 1999

DESCRIPTION :
If a COMPONENT wrapper for <variable> <component_no> exists in the
<computed_variable_manager>, it is returned, otherwise a new one is made in the
manager and returned.

???DB.  Don't know what <component_no> means for a variable unless it returns a
	vector
==============================================================================*/

#if defined (OLD_CODE)
struct Computed_variable_package *CREATE(Computed_variable_package)(
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_element) *fe_element_manager);
/*******************************************************************************
LAST MODIFIED : 17 January 2003

DESCRIPTION :
Creates a Computed_variable_package which is used by the rest of the program to
access everything to do with computed variables. The computed_variable_manager
is created here, and callbacks for changes in the fe_field_manager are
established so that wrapper Computed_variables are automatically created and
updated to match FE_fields.

???DB.  Not sure
???DB.  Should this be in computed_variable_finite_element?  Don't want FE here?
==============================================================================*/
#endif /* defined (OLD_CODE) */

int DESTROY(Computed_variable_package)(
	struct Computed_variable_package **package_address);
/*******************************************************************************
LAST MODIFIED : 3 February 1999

DESCRIPTION :
Frees memory/deaccess objects in computed_variable_package at
<*package_address>.  Cancels any further messages from the MANAGER(FE_field).

???DB.  Not sure
==============================================================================*/

struct MANAGER(Computed_variable)
	*Computed_variable_package_get_computed_variable_manager(
	struct Computed_variable_package *computed_variable_package);
/*******************************************************************************
LAST MODIFIED : 3 February 1999

DESCRIPTION :
Extracts the computed_variable_manager from the computed_variable_package.  Note
that the rest of the program should use this sparingly - it is really only here
to allow interfacing to the choose_object widgets.

???DB.  Not sure
==============================================================================*/

int Computed_variable_can_be_destroyed(struct Computed_variable *variable);
/*******************************************************************************
LAST MODIFIED : 10 May 2000

DESCRIPTION :
Returns nonzero if the <variable> is only accessed once - assumed by its
manager.  If it is of type COMPUTED_VARIABLE_FINITE_ELEMENT further tests that
its fe_variable can be destroyed, assuming it is only accessed by this variable
and its manager.

???DB.  Not sure
==============================================================================*/
#endif /* !defined (COMPUTED_VARIABLE_H) */
