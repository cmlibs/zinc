/*******************************************************************************
FILE : comp_new_field.h

LAST MODIFIED : 20 January 2003

DESCRIPTION :
Comp_new_field's are expressions that are constructed for:
- display eg. difference between measured and calculated positions
- minimization eg. fitting by minimizing the difference between measured and
	calculated positions
- solution eg. solving a FEM variational formulation equals zero

A variable is any value which is used in or is the result of the calculation of
a Comp_new_field and so can include element/xi, time, nodal parameters and
element scale factors.  When differentiating the dependent and independent
variables are specified.  Any unspecified variables are constant.

Comp_new_field's are able to be:
- evaluated at a point (specific choice of values for independent variables)
- differentiated at a point (specific choice of values for independent
	variables ie. not symbolic)
- composed ie. the results of one Comp_new_field can replace independent
	variables for another Comp_new_field

The shared interface to Comp_new_field's is defined here, but there are separate
modules for specific types eg finite element, gradient and coordinate
transformations.

NOTES :
1 To be able to use this with the current version, have swapped
	- computed_field -> comp_new_field
	- Computed_field -> Comp_new_field
	- COMPUTED_FIELD -> COMP_NEW_FIELD
	When finished can change back
2 To allow compilation, caching and reuse, there should be a
	Comp_new_field_calculation object to be used in the following manner
		calculation=CREATE(Comp_new_field_calculation)(comp_new_field)
		j=1,p
			Comp_new_field_calculation_set_variable(calculation,variable_ij),ij=1,nj
			Comp_new_field_calculation_set_derivative(calculation,tuple_ij),ij=1,mj
			Comp_new_field_calculation_evaluate(calculation)
		DESTROY(Comp_new_field_calculation)(&calculation)
	The Comp_new_field_calculation_set's are not changing the comp_new_field, just
	changing the overrides.  Assuming that comp_new_field is not being changed,
	this allows calculation shortcuts to be made.  It also allows the cache to be
	outside of the Comp_new_field.
3 To specify a derivative, a list of Comp_new_field_variable's is used eg
	d^3/dxi1dxi3^2 is represented by the list (xi1,xi3,xi3).
4 Why are Comp_new_field_variable's needed?  For the current Computed_field's,
	the source_fields are specified in the Computed_field_set_type function and
	you can only differentiate with respect to xi.  With the Comp_new_field's
	there are ways of specifying the source fields which overlap, eg first order
	derivative nodal values and all nodal values, and you can differentiate with
	respect to any independent variable.  Comp_new_field_variable's are a flexible
	way of identifying the inputs and outputs of a Comp_new_field.
5 Have split off comp_new_field_utilities and comp_new_field_commands in order
	to focus on core of Comp_new_field's.
6 To say that the results of one computed field should replace independent
	variables for a second computed field (composition of mathematical functions),
	use Comp_new_field_variable_set_source and Comp_new_field_evaluate (temporary
	over-riding) or Comp_new_field_set_variable (permanent).
???DB.  Seems a bit long-winded.  May be simpler if had
	- the source for Comp_new_field_variable_set_source being a Comp_new_field
		(rather than a Comp_new_field_variable).  This would mean that all the
		results of a Comp_new_field would have to be used
	- get rid of Comp_new_fields and just have Comp_new_field_variables (or
		equivalently just have Comp_new_fields).  Have the actual computation as
		part of the result Comp_new_field_variables.  What about specifying
		element/xi?
7 Specifying variables in evaluate is only for leaf dofs?
8 Comp_new_fields are not invertable ie. if the results are specified for an
	evaluation, the dofs are not calculated.  Instead
	- Newton-Raphson can be done explicitly in the calling program or command file
	- use Comp_new_field_set_type_inverse


???DB.  Label outputs (results) in same way that label inputs?
	- allows use of part of output for input
	- if make in and out the same, would probably have evaluate (with no result
		argument) and then get the particular output.  No, still have results to
		specify what is wanted

???DB.  What about using a Comp_new_field multiple times within another?
	- different ways of replacing independent variables each time?
	- how does this affect naming of variables?
	- duplicate Comp_new_field's that are used?  Makes caching less useful?
		- interface allows this if add Comp_new_field_duplicate?
		- what happens currently?
	- separate caching and connection of Comp_new_fields (another object layer)?

???DB.  How should element/xi and time be "shared" between Comp_new_field's?
	- have Mesh_location which is element/xi or node
	- have a FE computed field which is identity returning Mesh_location and
		others replace their Mesh_location independent variables with its result?
		How do you set Mesh_location when only have top?
		???DB.  Most transparent?
	- have concept of a shared variable?
	- use independent/dependent somehow?
	- force element/xi and time to be supplied?
	- have variables come from parent
	  - results come down tree (towards root)
		- variables go up tree (towards leaves)
	- could change Comp_new_field of Comp_new_field_variable when passing
		Mesh_location on?  What about where field is important eg nodal values?

???DB.  This is CellML in terms of having an interface with in and out and
	saying how everything is connected.  CellML has only one model/Comp_new_field

???DB.  Want to be able to say that some independent variables have to be
	supplied (from another field)?  Force element/xi and time to be supplied?

???DB.  Have a Comp_new_field type which is a Comp_new_field_variable?

???DB.  Something needs doing about FE_value - has spread too far?
	Set up in general/value.h .  Maybe just needs renaming?
==============================================================================*/
#if !defined (COMP_NEW_FIELD_H)
#define COMP_NEW_FIELD_H

#include "general/geometry.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "general/value.h"

/*
Global types
------------
*/
struct Comp_new_field_variable;
/*******************************************************************************
LAST MODIFIED : 17 January 2003

DESCRIPTION :
A variable or collection of variables used in the calculation of a
Comp_new_field.  Can be used to:
- identify eg. when differentiating with respect to
- specify actual values
- specify a Comp_new_field to use to calculate
- request and return values/field

A Comp_new_field_variable is an identifier for an input to or output from a
Comp_new_field and not the actual input/output.  So when a Comp_new_field is
given a Comp_new_field_variable it can decide whether it identifies any of its
actual input(s)/output(s).

???DB.  What about names for results?  Unlike independent variables, name is
	dependent on Comp_new_field name?
	- Comp_new_field_variable includes pointer to Comp_new_field?
==============================================================================*/

/* a list of variables.  This can be used to specify the variables to
	differentiate with respect to, ie. a derivative, with the number of variables
	in the list being the order of the derivative */
DECLARE_LIST_TYPES(Comp_new_field_variable);

/* a list of derivatives */
DECLARE_LIST_TYPES(LIST(Comp_new_field_variable));

struct Comp_new_field_matrix;
/*******************************************************************************
LAST MODIFIED : 16 January 2003

DESCRIPTION :
Contains a possibly sparse matrix and identifies the variable(s) corresponding
to each row and each column.

???DB.  Need functions/methods
???DB.  Need matrix object
==============================================================================*/

struct Comp_new_field;
/*******************************************************************************
LAST MODIFIED : 2 January 2003

DESCRIPTION :
An expression that can be displayed, minimized or used in an equation.
==============================================================================*/

DECLARE_LIST_TYPES(Comp_new_field);

DECLARE_MANAGER_TYPES(Comp_new_field);

struct Comp_new_field_package;
/*******************************************************************************
LAST MODIFIED : 2 January 2003

DESCRIPTION :
Contains all information for editing and maintaining Comp_new_fields, including
the MANAGER(Comp_new_field).

???RC Make macro PACKAGE(Comp_new_field) etc.?
==============================================================================*/

/*
Global functions
----------------
*/
struct Comp_new_field_variable *CREATE(Comp_new_field_variable)(void);
/*******************************************************************************
LAST MODIFIED : 16 January 2003

DESCRIPTION :
Creates an empty variable.  Each type of Comp_new_field can have functions for
changing a Comp_new_field_variable to refer to their own variables.
==============================================================================*/

int DESTROY(Comp_new_field_variable)(
	struct Comp_new_field_variable **variable_address);
/*******************************************************************************
LAST MODIFIED : 16 January 2003

DESCRIPTION :
Frees memory/deaccess objects in Comp_new_field_variable at <*variable_address>.
==============================================================================*/

PROTOTYPE_LIST_FUNCTIONS(Comp_new_field_variable);
PROTOTYPE_LIST_FUNCTIONS(LIST(Comp_new_field_variable));

struct Comp_new_field_variable *Comp_new_field_variable_duplicate(
	struct Comp_new_field_variable *variable);
/*******************************************************************************
LAST MODIFIED : 16 January 2003

DESCRIPTION :
Makes a copy of the <variable>.
==============================================================================*/

int Comp_new_field_variable_set_source(struct Comp_new_field_variable *variable,
	struct Comp_new_field_variable *source);
/*******************************************************************************
LAST MODIFIED : 20 January 2003

DESCRIPTION :
Sets the values for <variable> to come from the values <source>.  The types of
the values for <variable> and <source> must match.
==============================================================================*/

int Comp_new_field_variable_set_type_result(
	struct Comp_new_field_variable *variable,struct Comp_new_field *field,
	int component_number);
/*******************************************************************************
LAST MODIFIED : 19 January 2003

DESCRIPTION :
Converts the <variable> into the result for the specified <field> (all computed
fields if NULL) and <component_number> (all components if -1).
==============================================================================*/

int Comp_new_field_variable_set_type_independent_variable(
	struct Comp_new_field_variable *variable,struct Comp_new_field *field);
/*******************************************************************************
LAST MODIFIED : 19 January 2003

DESCRIPTION :
Converts the <variable> into all independent variables for the specified <field>
(all computed fields if NULL).
==============================================================================*/

int Comp_new_field_variable_set_type_FE_value_vector(
	struct Comp_new_field_variable *variable,int number_of_components,
	FE_value *values);
/*******************************************************************************
LAST MODIFIED : 16 January 2003

DESCRIPTION :
Converts the <variable> into a vector of FE_values with the specified
<number_of_components> and <values> (possibly NULL, copied).
==============================================================================*/

int Comp_new_field_variable_is_FE_value_vector(
	struct Comp_new_field_variable *variable,int *number_of_components_address);
/*******************************************************************************
LAST MODIFIED : 16 January 2003

DESCRIPTION :
If the <variable> is a vector of FE_values, then returns nonzero and sets
<*number_of_components_address> to the number of components in the vector,
otherwise returns zero.
==============================================================================*/

int Comp_new_field_variable_get_FE_value_vector(
	struct Comp_new_field_variable *variable,int *number_of_components_address,
	FE_value **values_address);
/*******************************************************************************
LAST MODIFIED : 16 January 2003

DESCRIPTION :
If <variable> is representable as a vector of FE_values, then returns nonzero
and
- if <number_of_components_address> is not NULL sets
	<*number_of_components_address> to the number of values in the vector
- if <values_address> is not NULL sets <*values_address> to point at the values
otherwise returns zero.

The calling routine is NOT responsible for DEALLOCATEing <*values_address>.
==============================================================================*/

int Comp_new_field_variable_is_string(struct Comp_new_field *variable);
/*******************************************************************************
LAST MODIFIED : 16 January 2003

DESCRIPTION :
If the <variable> is a string, then returns nonzero, otherwise returns zero.
==============================================================================*/

char *Comp_new_field_variable_as_string(
	struct Comp_new_field_variable *variable);
/*******************************************************************************
LAST MODIFIED : 16 January 2003

DESCRIPTION :
If possible, returns a string representing the <variable>, otherwise returns
NULL.

The calling function is responsible for DEALLOCATEing the returned string.
==============================================================================*/

struct Comp_new_field *CREATE(Comp_new_field)(char *name);
/*******************************************************************************
LAST MODIFIED : 16 January 2003

DESCRIPTION :
Creates a Comp_new_field with the given <name> and no type.  The type specific
modules have a "set_type" function.
==============================================================================*/

int DESTROY(Comp_new_field)(struct Comp_new_field **field_address);
/*******************************************************************************
LAST MODIFIED : 2 January 2003

DESCRIPTION :
Frees memory/deaccess objects in Comp_new_field at <*field_address>.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Comp_new_field);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Comp_new_field);

PROTOTYPE_LIST_FUNCTIONS(Comp_new_field);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Comp_new_field,name,char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(Comp_new_field,name,char *);
PROTOTYPE_MANAGER_FUNCTIONS(Comp_new_field);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Comp_new_field,name,char *);

struct Comp_new_field *Comp_new_field_duplicate(struct Comp_new_field *field);
/*******************************************************************************
LAST MODIFIED : 16 January 2003

DESCRIPTION :
Makes a copy of the <field>.  Does not duplicate underlying fields (eg. finite
element).
==============================================================================*/

int Comp_new_field_set_variable(struct Comp_new_field *field,
	struct LIST(Comp_new_field_variable) *values);
/*******************************************************************************
LAST MODIFIED : 19 January 2003

DESCRIPTION :
Sets the specified variables for the <field> to the given <values>.  A value can
be another field which means that the variable would be calculated from the
other field.
==============================================================================*/

int Comp_new_field_get_variable(struct Comp_new_field *field,
	struct Comp_new_field_variable **values);
/*******************************************************************************
LAST MODIFIED : 19 January 2003

DESCRIPTION :
Gets the specified variables for the <field> and puts them in <values>.  If a
specified variable is calculated from another field then the other field is
returned in <values>.
==============================================================================*/

int Comp_new_field_set_type_inverse(struct Comp_new_field *inverse_field,
	struct Comp_new_field *field,struct LIST(Comp_new_field_variable) *results);
/*******************************************************************************
LAST MODIFIED : 20 January 2003

DESCRIPTION :
Sets <inverse_field> to be the inverse of the <field>.  Its independent
variables are the results of the <field> and its <results> are independent
variables of the <field>.
==============================================================================*/

int Comp_new_field_depends_on_Comp_new_field(struct Comp_new_field *field,
	struct Comp_new_field *other_field);
/*******************************************************************************
LAST MODIFIED : 8 January 2002

DESCRIPTION :
Returns true if the two fields are identical or if <other_field> needs to be
evaluated in order to evaluate <field>.

This function is used by MANAGER_COPY_WITHOUT_IDENTIFIER to ensure that the
destination field will not depend on itself, thus leading to an infinite loop,
and also to prevent copying a field over itself.

Parts of the program receiving manager messages for Comp_new_fields should call
this function with the field=field in use and other_field=modified field to
determine if the field in use needs updating.
==============================================================================*/

int Comp_new_field_depends_on_Comp_new_field_in_list(
	struct Comp_new_field *field,struct LIST(Comp_new_field) *field_list);
/*******************************************************************************
LAST MODIFIED : 2 January 2002

DESCRIPTION :
Returns true if <field> depends on any field in <field_list>.
==============================================================================*/

int Comp_new_field_is_defined(struct Comp_new_field *field,
	struct LIST(Comp_new_field_variable) *values,
	struct LIST(LIST(Comp_new_field_variable)) *derivatives,
	struct LIST(Comp_new_field_variable) *results);
/*******************************************************************************
LAST MODIFIED : 19 January 2003

DESCRIPTION :
Returns true if <results> of the <field> can be calculated with the specified
<values> over-riding, but not setting, the current values and the <derivatives>
of the <results> can be calculated.  Each entry of <derivatives> is a list,
specifying the variables to differentiate with respect to.

???DB.  Could use evaluate, but that would be less efficient and currently
	would write error messages if it failed
==============================================================================*/

int Comp_new_field_evaluate(struct Comp_new_field *field,
	struct LIST(Comp_new_field_variable) *values,
	struct LIST(LIST(Comp_new_field_variable)) *derivatives,
	struct LIST(Comp_new_field_variable) *results,
	struct Comp_new_field_variable_matrix *derivative_results);
/*******************************************************************************
LAST MODIFIED : 19 January 2003

DESCRIPTION :
Calculates the <results> of the <field> and the <derivatives> of the <results>
(placed in <derivative_results>) with the specified <values> over-riding, but
not setting, the current values.  Each entry of <derivatives> is a list,
specifying the variables to differentiate with respect to.
==============================================================================*/

struct Coordinate_system *Comp_new_field_get_coordinate_system(
	struct Comp_new_field *field);
/*******************************************************************************
LAST MODIFIED : 2 January 2003

DESCRIPTION :
Returns the coordinate system <field> is to be interpreted under.  See function
Comp_new_field_set_coordinate_system for further details.
==============================================================================*/

int Comp_new_field_set_coordinate_system(struct Comp_new_field *field,
	struct Coordinate_system *coordinate_system);
/*******************************************************************************
LAST MODIFIED : 2 January 2003

DESCRIPTION :
Sets the coordinate system <field> is to be interpreted under.  Note the careful
choice of words here: the coordinate system merely tells the rest of the program
what needs to be done to transform the field values into any other coordinate
system.  It does not have to be "correct" for the values in the field, eg. you
can describe prolate spheroidal values as RC to "open out" the heart model.
<focus> must be given with prolate and oblate spheroidal coordinate systems.
==============================================================================*/

char *Comp_new_field_get_type_string(struct Comp_new_field *field);
/*******************************************************************************
LAST MODIFIED : 2 January 2003

DESCRIPTION :
Returns the string which identifies the type.

The calling function must not DEALLOCATE the returned string.
==============================================================================*/

int Comp_new_field_set_read_only(struct Comp_new_field *field);
/*******************************************************************************
LAST MODIFIED : 2 January 2003

DESCRIPTION :
Marks <field> as read-only, telling the program that the user is not permitted
to modify or destroy it.

???DB.  Is this part of the package?
==============================================================================*/

int Comp_new_field_set_read_write(struct Comp_new_field *field);
/*******************************************************************************
LAST MODIFIED : 2 January 2003

DESCRIPTION :
Clears read-only status of <field>, telling the program that the user is allowed
to modify and destroy it.

???DB.  Is this part of the package?
==============================================================================*/

struct Comp_new_field *Comp_new_field_manager_get_component_wrapper(
	struct MANAGER(Comp_new_field) *comp_new_field_manager,
	struct Comp_new_field *field,int component_no);
/*******************************************************************************
LAST MODIFIED : 3 December 1999

DESCRIPTION :
If a COMPONENT wrapper for <field> <component_no> exists in the
<comp_new_field_manager>, it is returned, otherwise a new one is made in the
manager and returned.
==============================================================================*/

struct Comp_new_field_package *CREATE(Comp_new_field_package)(
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_element) *fe_element_manager);
/*******************************************************************************
LAST MODIFIED : 17 January 2003

DESCRIPTION :
Creates a Comp_new_field_package which is used by the rest of the program to
access everything to do with computed fields. The comp_new_field_manager is
created here, and callbacks for changes in the fe_field_manager are established
so that wrapper Comp_new_fields are automatically created and updated to match
FE_fields.

???DB.  Should this be in comp_new_field_finite_element?  Don't want FE here?
==============================================================================*/

int DESTROY(Comp_new_field_package)(
	struct Comp_new_field_package **package_address);
/*******************************************************************************
LAST MODIFIED : 3 February 1999

DESCRIPTION :
Frees memory/deaccess objects in comp_new_field_package at <*package_address>.
Cancels any further messages from the MANAGER(FE_field).
==============================================================================*/

struct MANAGER(Comp_new_field)
	*Comp_new_field_package_get_comp_new_field_manager(
	struct Comp_new_field_package *comp_new_field_package);
/*******************************************************************************
LAST MODIFIED : 3 February 1999

DESCRIPTION :
Extracts the comp_new_field_manager from the comp_new_field_package. Note that
the rest of the program should use this sparingly - it is really only here to
allow interfacing to the choose_object widgets.
==============================================================================*/

int Comp_new_field_can_be_destroyed(struct Comp_new_field *field);
/*******************************************************************************
LAST MODIFIED : 10 May 2000

DESCRIPTION :
Returns true if the <field> is only accessed once - assumed by its manager. If
it is of type COMP_NEW_FIELD_FINITE_ELEMENT further tests that its fe_field can
be destroyed, assuming it is only accessed by this field and its manager.
==============================================================================*/
#endif /* !defined (COMP_NEW_FIELD_H) */
