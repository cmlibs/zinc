/*******************************************************************************
FILE : computed_variable_private.h

LAST MODIFIED : 20 March 2003

DESCRIPTION :
???DB.  Move structure into .c .  Means that have to change macro into a
	function.
???DB.  Also means that macros don't know struct Computed_variable definition
==============================================================================*/
#if !defined (COMPUTED_VARIABLE_PRIVATE_H)
#define COMPUTED_VARIABLE_PRIVATE_H

#include "computed_variable/computed_variable.h"

/*
Friend types
------------
*/
typedef int (*Computed_variable_source_variable_iterator)(
	struct Computed_variable *variable,struct Computed_variable *source_variable,
	struct Computed_variable *independent_variable,void *data_void);

/*
Method types
------------
*/
typedef void Computed_variable_type_specific_data;

typedef int (*Computed_variable_clear_type_specific_function)(
	struct Computed_variable *variable);
/*******************************************************************************
LAST MODIFIED : 6 February 2003

DESCRIPTION :
Clear the type specific data for <variable> (passed into the _set_type_
function), but don't DEALLOCATE the data.
==============================================================================*/

#define START_COMPUTED_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION( variable_type ) \
int Computed_variable_ ## variable_type ## _clear_type_specific( \
	struct Computed_variable *variable) \
{ \
	int return_code; \
	struct Computed_variable_ ## variable_type ## _type_specific_data *data; \
\
	ENTER(Computed_variable_ ## variable_type ## _clear_type_specific); \
	return_code=0; \
	data=(struct Computed_variable_ ## variable_type ## _type_specific_data *) \
		Computed_variable_get_type_specific_data(variable); \
	ASSERT_IF(data,return_code,0) \
	ASSERT_IF(computed_variable_ ## variable_type ## _type_string== \
		Computed_variable_get_type_id_string(variable),return_code,0)

#define END_COMPUTED_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION( variable_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Computed_variable_ ## variable_type ## _clear_type_specific */

typedef Computed_variable_type_specific_data*
	(*Computed_variable_duplicate_data_type_specific_function)(
	struct Computed_variable *variable);
/*******************************************************************************
LAST MODIFIED : 7 February 2003

DESCRIPTION :
Returns a duplicate of the <variable>'s type specific data.

???DB.  A copy with, source and destination, but this makes it difficult for
	MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_variable,name) to recover if this
	fails
==============================================================================*/

#define START_COMPUTED_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
Computed_variable_type_specific_data * \
	Computed_variable_ ## variable_type ## _duplicate_data_type_specific( \
	struct Computed_variable *variable) \
{ \
	struct Computed_variable_ ## variable_type ## _type_specific_data \
		*destination,*source; \
\
	ENTER(Computed_variable_ ## variable_type ## _duplicate_data_type_specific); \
	/* check arguments */ \
	ASSERT_IF(variable&&(computed_variable_ ## variable_type ## _type_string== \
		Computed_variable_get_type_id_string(variable)),destination,NULL) \
	{ \
		source=Computed_variable_get_type_specific_data(variable); \
		ASSERT_IF(source,destination,NULL) \
		if (!ALLOCATE(destination, \
			struct Computed_variable_ ## variable_type ## _type_specific_data,1)) \
		{ \
			display_message(ERROR_MESSAGE,"Computed_variable_" #variable_type \
				"_duplicate_data_type_specific.  Could not ALLOCATE destination"); \
		} \
		else

#define END_COMPUTED_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
	} \
	LEAVE; \
\
	return (destination); \
} /* Computed_variable_ ## variable_type ## _duplicate_data_type_specific */

typedef int (*Computed_variable_evaluate_derivative_type_specific_function)(
	struct Computed_variable *variable,int order,
	struct Computed_variable **independent_variables,
	struct Computed_value *value);
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
Evaluates the <order> degree derivative of <variable> with respect to
<independent_variables> and sets <value> to the result.
==============================================================================*/

#define START_COMPUTED_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
int Computed_variable_ ## variable_type ## _evaluate_derivative_type_specific( \
	struct Computed_variable *variable,int order, \
	struct Computed_variable **independent_variables, \
	struct Computed_value *value) \
{ \
	int return_code; \
\
	ENTER(Computed_variable_ ## variable_type ## \
		_evaluate_derivative_type_specific); \
	return_code=0; \
	/* check arguments */ \
	ASSERT_IF(variable&&(computed_variable_ ## variable_type ## _type_string== \
		Computed_variable_get_type_id_string(variable)&&(0<order)&& \
		independent_variables&&value),return_code,0)

#define END_COMPUTED_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Computed_variable_ ## variable_type ## \
	_evaluate_derivative_type_specific */

typedef int (*Computed_variable_evaluate_type_specific_function)(
	struct Computed_variable_value *variable_value);
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
Evaluate the <variable_value> using the independent variable <values>.
==============================================================================*/

#define START_COMPUTED_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
int Computed_variable_ ## variable_type ## _evaluate_type_specific( \
	struct Computed_variable_value *variable_value) \
{ \
	int return_code; \
	struct Computed_variable *variable; \
\
	ENTER(Computed_variable_ ## variable_type ## _evaluate_type_specific); \
	return_code=0; \
	/* check arguments */ \
	variable=Computed_variable_value_get_variable(variable_value); \
	ASSERT_IF(variable&&(computed_variable_ ## variable_type ## _type_string== \
		Computed_variable_get_type_id_string(variable)),return_code,0)

#define END_COMPUTED_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION( variable_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Computed_variable_ ## variable_type ## _evaluate_type_specific */

typedef int
	(*Computed_variable_get_independent_variable_value_type_specific_function)(
	struct Computed_variable *dependent_variable,
	struct Computed_variable *independent_variable,
	struct Computed_value *value);
/*******************************************************************************
LAST MODIFIED : 7 February 2003

DESCRIPTION :
Gets the current <value> for the <independent_variable> of the
<dependent_variable>.
==============================================================================*/

#define START_COMPUTED_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
int Computed_variable_ ## variable_type ## \
	_get_independent_variable_value_type_specific( \
	struct Computed_variable *dependent_variable, \
	struct Computed_variable *independent_variable, \
	struct Computed_value *value) \
{ \
	int return_code; \
\
	ENTER(Computed_variable_ ## variable_type ## \
		_get_independent_variable_value_type_specific); \
	return_code=0; \
	/* check arguments */ \
	ASSERT_IF(dependent_variable&&(computed_variable_ ## variable_type ## \
		_type_string==Computed_variable_get_type_id_string(dependent_variable))&& \
		independent_variable&&value,return_code,0)

#define END_COMPUTED_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Computed_variable_ ## variable_type ## \
	_get_independent_variable_value_type_specific */

typedef int
	(*Computed_variable_get_set_independent_variable_value_type_specific_function)(
	struct Computed_variable *dependent_variable,
	struct Computed_variable *independent_variable,
	struct Computed_value *value);
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
Swaps the <value> and the current value for the <independent_variable> of the
<dependent_variable>.
==============================================================================*/

#define START_COMPUTED_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
int Computed_variable_ ## variable_type ## \
	_get_set_independent_variable_value_type_specific( \
	struct Computed_variable *dependent_variable, \
	struct Computed_variable *independent_variable, \
	struct Computed_value *value) \
{ \
	int return_code; \
\
	ENTER(Computed_variable_ ## variable_type ## \
		_get_set_independent_variable_value_type_specific); \
	return_code=0; \
	/* check arguments */ \
	ASSERT_IF(dependent_variable&&(computed_variable_ ## variable_type ## \
		_type_string==Computed_variable_get_type_id_string(dependent_variable))&& \
		independent_variable&&value,return_code,0)

#define END_COMPUTED_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Computed_variable_ ## variable_type ## \
	_get_set_independent_variable_value_type_specific */

typedef int (*Computed_variable_get_value_type_type_specific_function)(
	struct Computed_variable *variable,struct Computed_value *type);
/*******************************************************************************
LAST MODIFIED : 7 February 2003

DESCRIPTION :
Gets the <type> (doesn't fill in a value) that the <variable> evaluates to.
==============================================================================*/

#define START_COMPUTED_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
int Computed_variable_ ## variable_type ## _get_value_type_type_specific( \
	struct Computed_variable *variable,struct Computed_value *type) \
{ \
	int return_code; \
\
	ENTER(Computed_variable_ ## variable_type ## _get_value_type_type_specific); \
	return_code=0; \
	/* check arguments */ \
	ASSERT_IF(variable&&(computed_variable_ ## variable_type ## _type_string== \
		Computed_variable_get_type_id_string(variable))&&type,return_code,0)

#define END_COMPUTED_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Computed_variable_ ## variable_type ## _get_value_type_type_specific */

typedef int
	(*Computed_variable_is_defined_type_specific_function)(
	struct Computed_variable *variable);
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
If the <variable> can be evaluated assuming that all its source variables can be
evaluated and that the <values> are specified, then this method returns a
non-zero.  This method is not recursive - checking through tree of source
variables is done by Computed_variable_is_defined.

???DB.  Which one (above or below)?  A derivative computed variable does not
	need to differentiate its source variables.  It needs do differentiate the
	source variables of the computed variable it is a derivative of.

If source variables are defined and the <variable> can be evaluated assuming
that all its source variables can be evaluated and that the <values> are
specified, then this method returns a non-zero.  This method does the is
defined checking for source variables.

???DB.  Could change so that doesn't need to look at independent/source
	variable pairs at all - Computed_variable_is_defined incorporates the
	independent variables into a values list and change how source/independent
	pairs are stored?
==============================================================================*/

#define START_COMPUTED_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
int Computed_variable_ ## variable_type ## _is_defined_type_specific( \
	struct Computed_variable *variable) \
{ \
	int return_code; \
\
	ENTER(Computed_variable_ ## variable_type ## _is_defined_type_specific); \
	return_code=0; \
	/* check arguments */ \
	ASSERT_IF(variable&&(computed_variable_ ## variable_type ## _type_string== \
		Computed_variable_get_type_id_string(variable)),return_code,0)

#define END_COMPUTED_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Computed_variable_ ## variable_type ## _is_defined_type_specific */

typedef int
	(*Computed_variable_is_independent_variable_of_type_specific_function)(
	struct Computed_variable *dependent_variable,
	struct Computed_variable *independent_variable);
/*******************************************************************************
LAST MODIFIED : 7 February 2003

DESCRIPTION :
Returns non-zero if <dependent_variable> has the <independent_variable> and zero
otherwise.
==============================================================================*/

#define START_COMPUTED_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
int Computed_variable_ ## variable_type ## \
	_is_independent_variable_of_type_specific( \
	struct Computed_variable *dependent_variable, \
	struct Computed_variable *independent_variable) \
{ \
	int return_code; \
\
	ENTER(Computed_variable_ ## variable_type ## \
		_is_independent_variable_of_type_specific); \
	return_code=0; \
	/* check arguments */ \
	ASSERT_IF(dependent_variable&&(computed_variable_ ## variable_type \
		## _type_string==Computed_variable_get_type_id_string(dependent_variable))&& \
		independent_variable,return_code,0)

#define END_COMPUTED_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Computed_variable_ ## variable_type ## \
	_is_independent_variable_of_type_specific */

typedef int (*Computed_variable_not_in_use_type_specific_function)(
	struct Computed_variable *variable);
/*******************************************************************************
LAST MODIFIED : 7 February 2003

DESCRIPTION :
Returns non-zero if none of its type specific objects (in data) are in use
(ACCESSed) and zero otherwise.
==============================================================================*/

#define START_COMPUTED_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
int Computed_variable_ ## variable_type ## _not_in_use_type_specific( \
	struct Computed_variable *variable) \
{ \
	int return_code; \
\
	ENTER(Computed_variable_ ## variable_type ## _not_in_use_type_specific); \
	return_code=0; \
	/* check arguments */ \
	ASSERT_IF(variable&&(computed_variable_ ## variable_type ## _type_string== \
		Computed_variable_get_type_id_string(variable)),return_code,0)

#define END_COMPUTED_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Computed_variable_ ## variable_type ## _not_in_use_type_specific */

typedef int (*Computed_variable_overlap_type_specific_function)(
	struct Computed_variable *variable_1,struct Computed_variable *variable_2);
/*******************************************************************************
LAST MODIFIED : 7 February 2003

DESCRIPTION :
Returns non-zero if <variable_1> and <variable_2> share variables (eg [value,
d/ds1] and [d/ds1,d/ds2] overlap) and zero otherwise.
==============================================================================*/

#define START_COMPUTED_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
int Computed_variable_ ## variable_type ## _overlap_type_specific( \
	struct Computed_variable *variable_1,struct Computed_variable *variable_2) \
{ \
	int return_code; \
\
	ENTER(Computed_variable_ ## variable_type ## _overlap_type_specific); \
	return_code=0; \
	/* check arguments */ \
	ASSERT_IF(variable_1&&(computed_variable_ ## variable_type ## _type_string== \
		Computed_variable_get_type_id_string(variable_1))&&variable_2&& \
		(computed_variable_ ## variable_type ## _type_string== \
		Computed_variable_get_type_id_string(variable_2)),return_code,0)

#define END_COMPUTED_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Computed_variable_ ## variable_type ## _overlap_type_specific */

typedef int (*Computed_variable_same_variable_type_specific_function)(
	struct Computed_variable *variable_1,struct Computed_variable *variable_2);
/*******************************************************************************
LAST MODIFIED : 7 February 2003

DESCRIPTION :
Returns nonzero if <variable_1> and <variable_2> are the same variable (eg. the
value at node 10 for the finite element field bob) and zero otherwise.
==============================================================================*/

#define START_COMPUTED_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
int Computed_variable_ ## variable_type ## _same_variable_type_specific( \
	struct Computed_variable *variable_1,struct Computed_variable *variable_2) \
{ \
	int return_code; \
\
	ENTER(Computed_variable_ ## variable_type ## _same_variable_type_specific); \
	return_code=0; \
	/* check arguments */ \
	ASSERT_IF(variable_1&&(computed_variable_ ## variable_type ## _type_string== \
		Computed_variable_get_type_id_string(variable_1))&&variable_2&& \
		(computed_variable_ ## variable_type ## _type_string== \
		Computed_variable_get_type_id_string(variable_2)),return_code,0)

#define END_COMPUTED_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Computed_variable_ ## variable_type ## _same_variable_type_specific */

typedef int
	(*Computed_variable_set_independent_variable_value_type_specific_function)(
	struct Computed_variable *dependent_variable,
	struct Computed_variable *independent_variable,
	struct Computed_value *value);
/*******************************************************************************
LAST MODIFIED : 7 February 2003

DESCRIPTION :
Sets the <value> of the <independent_variable> for the <variable>.
==============================================================================*/

#define START_COMPUTED_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
int Computed_variable_ ## variable_type ## \
	_set_independent_variable_value_type_specific( \
	struct Computed_variable *dependent_variable, \
	struct Computed_variable *independent_variable,struct Computed_value *value) \
{ \
	int return_code; \
\
	ENTER(Computed_variable_ ## variable_type ## _set_independent_variable_value_type_specific); \
	return_code=0; \
	/* check arguments */ \
	ASSERT_IF(dependent_variable&& \
		(computed_variable_ ## variable_type ## _type_string== \
		Computed_variable_get_type_id_string(dependent_variable))&& \
		independent_variable&&value,return_code,0)

#define END_COMPUTED_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Computed_variable_ ## variable_type ## \
	_set_independent_variable_value_type_specific */


/*
Friend macros
-------------
*/
#define DECLARE_COMPUTED_VARIABLE_IS_TYPE_FUNCTION( variable_type ) \
PROTOTYPE_COMPUTED_VARIABLE_IS_TYPE_FUNCTION(variable_type) \
{ \
	int return_code; \
\
	ENTER(Computed_variable_is_type_ ## variable_type); \
	return_code=0; \
	/* check argument */ \
	if (variable) \
	{ \
		if (computed_variable_ ## variable_type ## _type_string== \
			Computed_variable_get_type_id_string(variable)) \
		{ \
			return_code=1; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"Computed_variable_is_type_" #variable_type \
			".  " "Missing variable"); \
	} \
	LEAVE; \
\
	return (return_code); \
} /* Computed_variable_is_type_ ## variable_type */

#define COMPUTED_VARIABLE_ESTABLISH_METHODS( variable, variable_type ) \
/***************************************************************************** \
LAST MODIFIED : 10 February 2003 \
\
DESCRIPTION : \
Each Computed_variable_set_type function should call this macro to establish \
the virtual functions that give the variable its particular behaviour.  Each \
function must therefore be defined for each variable type, even if it is set \
to NULL or some default function. \
============================================================================*/ \
Computed_variable_establish_methods(variable, \
	Computed_variable_ ## variable_type ## _clear_type_specific, \
	Computed_variable_ ## variable_type ## _duplicate_data_type_specific, \
	Computed_variable_ ## variable_type ## _evaluate_derivative_type_specific, \
	Computed_variable_ ## variable_type ## _evaluate_type_specific, \
	Computed_variable_ ## variable_type ## _get_independent_variable_value_type_specific, \
	Computed_variable_ ## variable_type ## _get_set_independent_variable_value_type_specific, \
	Computed_variable_ ## variable_type ## _get_value_type_type_specific, \
	Computed_variable_ ## variable_type ## _is_defined_type_specific, \
	Computed_variable_ ## variable_type ## _is_independent_variable_of_type_specific, \
	Computed_variable_ ## variable_type ## _not_in_use_type_specific, \
	Computed_variable_ ## variable_type ## _overlap_type_specific, \
	Computed_variable_ ## variable_type ## _same_variable_type_specific, \
	Computed_variable_ ## variable_type ## _set_independent_variable_value_type_specific)


/*
Friend functions
----------------
*/
int Computed_variable_establish_methods(struct Computed_variable *variable,
	Computed_variable_clear_type_specific_function
	computed_variable_clear_type_specific_function,
	Computed_variable_duplicate_data_type_specific_function
	computed_variable_duplicate_data_type_specific_function,
	Computed_variable_evaluate_derivative_type_specific_function
	computed_variable_evaluate_derivative_type_specific_function,
	Computed_variable_evaluate_type_specific_function
	computed_variable_evaluate_type_specific_function,
	Computed_variable_get_independent_variable_value_type_specific_function
	computed_variable_get_independent_variable_value_type_specific_function,
	Computed_variable_get_set_independent_variable_value_type_specific_function
	computed_variable_get_set_independent_variable_value_type_specific_function,
	Computed_variable_get_value_type_type_specific_function
	computed_variable_get_value_type_type_specific_function,
	Computed_variable_is_defined_type_specific_function
	computed_variable_is_defined_type_specific_function,
	Computed_variable_is_independent_variable_of_type_specific_function
	computed_variable_is_independent_variable_of_type_specific_function,
	Computed_variable_not_in_use_type_specific_function
	computed_variable_not_in_use_type_specific_function,
	Computed_variable_overlap_type_specific_function
	computed_variable_overlap_type_specific_function,
	Computed_variable_same_variable_type_specific_function
	computed_variable_same_variable_type_specific_function,
	Computed_variable_set_independent_variable_value_type_specific_function
	computed_variable_set_independent_variable_value_type_specific_function);
/*******************************************************************************
LAST MODIFIED : 10 February 2003

DESCRIPTION :
Sets the methods for the <variable>.
==============================================================================*/

Computed_variable_type_specific_data *Computed_variable_get_type_specific_data(
	struct Computed_variable *variable);
/*******************************************************************************
LAST MODIFIED : 6 February 2003

DESCRIPTION :
Returns the type specific data for the <variable>.
==============================================================================*/

int Computed_variable_set_type_specific_information(
	struct Computed_variable *variable,char *type_string,
	void *type_specific_data);
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
Sets the type specific information for the <variable>.
==============================================================================*/

int Computed_variable_clear_type(struct Computed_variable *variable);
/*******************************************************************************
LAST MODIFIED : 27 January 2003

DESCRIPTION :
Used internally by DESTROY and Computed_variable_set_type_*() functions to
deallocate or deaccess data specific to any Computed_variable_type.  Functions
changing the type of the Computed_variable should
- allocate any dynamic data needed for the type
- call this function to clear what is currently in the variable
- then set values
to ensure that the variable is not left in an invalid state.
==============================================================================*/

int Computed_variable_not_in_use(struct Computed_variable *variable);
/*******************************************************************************
LAST MODIFIED : 11 February 2003

DESCRIPTION :
To give access to Computed_variable_not_in_use_type_specific_function method.
==============================================================================*/

int Computed_variable_for_each_source_variable(
	struct Computed_variable *variable,
	Computed_variable_source_variable_iterator iterator,void *user_data);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Applies the <iterator> to each of the <variable>s source/independent variable
pairs until finish or the <iterator> returns zero.
==============================================================================*/

#if defined (OLD_CODE)
int Computed_variable_changed(struct Computed_variable *variable,
	struct MANAGER(Computed_variable) *computed_variable_manager);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Notifies the <computed_variable_manager> that the <variable> has changed.
==============================================================================*/

struct Computed_variable_type_data;

struct Computed_variable_type_data *CREATE(Computed_variable_type_data)
	(char *name, Define_Computed_variable_type_function
	define_Computed_variable_type_function, void *define_type_user_data);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Creates a structure representing a type of computed variable.  The <name> should
point to a static string which is used as the identifier of that type
throughout the program.  The <define_Computed_variable_type_function> is added to
the define_computed_variable option table when needed.
==============================================================================*/

int DESTROY(Computed_variable_type_data)
	(struct Computed_variable_type_data **data_address);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Frees memory/deaccess data at <*data_address>.
==============================================================================*/

int Computed_variable_package_add_type(
	struct Computed_variable_package *computed_variable_package, char *name,
	Define_Computed_variable_type_function define_Computed_variable_type_function,
	void *define_type_user_data);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Adds the type of Computed_variable described by <name> and
<define_Computed_variable_type_function> to those in the LIST held by the
<computed_variable_package>.  This type is then added to the
define_Computed_variable_type option table when parsing commands.
==============================================================================*/

int Computed_variable_clear_type(struct Computed_variable *variable);
/*******************************************************************************
LAST MODIFIED : 18 June 1999

DESCRIPTION :
Used internally by DESTROY and Computed_variable_set_type_*() functions to
deallocate or deaccess data specific to any Computed_variable_type. Functions
changing the type of the Computed_variable should allocate any dynamic data needed
for the type, call this function to clear what is currently in the variable and
then set values - that way the variable will never be left in an invalid state.
Calls Computed_variable_clear_cache before clearing the type.
==============================================================================*/

int Computed_variable_contents_match(struct Computed_variable *variable,
	void *other_computed_variable_void);
/*******************************************************************************
LAST MODIFIED : 22 January 1999

DESCRIPTION :
Iterator/conditional function returning true if contents of <variable> other than
its name matches the contents of the <other_computed_variable_void>.
==============================================================================*/

int Computed_variable_default_clear_type_specific(struct Computed_variable *variable);
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
A default implementation of this function to use when there is no type
specific data.
==============================================================================*/

void *Computed_variable_default_copy_type_specific(
	struct Computed_variable *source, struct Computed_variable *destination);
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
A default implementation of this function to use when there is no type
specific data.
==============================================================================*/

int Computed_variable_default_type_specific_contents_match(
	struct Computed_variable *variable, struct Computed_variable *other_computed_variable);
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
A default implementation of this function to use when there is no type
specific data.
==============================================================================*/

int Computed_variable_default_is_defined_in_element(struct Computed_variable *variable,
	struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Returns 1 if the all the source variables are defined in the supplied <element>.
==============================================================================*/

int Computed_variable_default_is_defined_at_node(struct Computed_variable *variable,
	struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Returns 1 if all the source variables are defined at the supplied <node>.
==============================================================================*/

int Computed_variable_default_has_multiple_times(struct Computed_variable *variable);
/*******************************************************************************
LAST MODIFIED : 22 November 2001

DESCRIPTION :
Returns 1 if any of the source variables have multiple times.
==============================================================================*/

int Computed_variable_set_coordinate_system_from_sources(
	struct Computed_variable *variable);
/*******************************************************************************
LAST MODIFIED : 3 July 2000

DESCRIPTION :
Sets the coordinate system of the <variable> to match that of it's sources.
==============================================================================*/

int Computed_variable_default_has_numerical_components(struct Computed_variable *variable);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Most computed variables have numerical components so this function returns 1.
==============================================================================*/

int Computed_variable_evaluate_cache_at_node(
	struct Computed_variable *variable,struct FE_node *node, FE_value time);
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Calculates the values of <variable> at <node>, if it is defined over the element.
Upon successful return the node values of the <variable> are stored in its cache.

???RC Could have a separate values cache for node computations. I am thinking of
cases where we have wrappers for calculating a coordinate variable at element:xi
taken from a variable or variables at a node - for showing the projection of a data
point during mesh fitting. At present the coordinate variable of data pt. position
may be the same as that of the element, but the position is quite different.
Ideally, they should have distinct coordinate variables, but 3-component coordinate
variables with the name 'coordinates' are quite pervasive.
==============================================================================*/

int Computed_variable_evaluate_cache_in_element(
	struct Computed_variable *variable,struct FE_element *element,FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives);
/*******************************************************************************
LAST MODIFIED : 30 November 2001

DESCRIPTION :
Calculates the values and derivatives (if <calculate_derivatives> set) of
<variable> at <element>:<xi>, if it is defined over the element. Upon successful
return values and derivatives of the variable are stored in the internal cache for
the <variable>. <xi> is assumed to contain the same number of values as the
dimension of the element.

The optional <top_level_element> may be supplied for the benefit of this or
any source variables that may require calculation on it instead of a face or line.
FIBRE_AXES and GRADIENT are examples of such variables, since they require
top-level coordinate derivatives. The term "top_level" refers to an ultimate
parent element for the face or line, eg. the 3-D element parent to 2-D faces.
If no such top level element is supplied and one is required, then the first
available parent element will be chosen - if the user requires a top-level
element in the same group as the face or with the face on the correct side,
then they should supply the top_level_element here. Once a variable has switched
to being calculated on the top_level_element, all its source variables will be
too - this should be understood when supplying source variables to such functions.
==============================================================================*/

int Computed_variable_evaluate_source_variables_cache_at_node(
	struct Computed_variable *variable,struct FE_node *node, FE_value time);
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Calculates the cache values of each source variable in <variable> at <node>, if it
is defined over the element.
Upon successful return the node values of the source variables are stored in their
cache.
==============================================================================*/

int Computed_variable_evaluate_source_variables_cache_in_element(
	struct Computed_variable *variable,struct FE_element *element,FE_value *xi,
	FE_value time,struct FE_element *top_level_element,int calculate_derivatives);
/*******************************************************************************
LAST MODIFIED : 30 November 2001

DESCRIPTION :
Calculates the cache values of each source variable in <variable> in <element>, if it
is defined over the element.
Upon successful return the element values of the source variables are stored in their
cache.
==============================================================================*/

char *Computed_variable_default_evaluate_as_string_at_node(
	struct Computed_variable *variable, int component_number, struct FE_node *node,
	FE_value time);
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Returns a string describing the value/s of the <variable> at the <node>. A string
built up of comma separated values evaluated
for the variable in Computed_variable_evaluate_cache_at_node. The FE_value exception
is used since it is likely the values are already in the cache in most cases,
or can be used by other variables again if calculated now.
The <component_number> indicates which component to calculate.  Use -1 to
create a string which represents all the components.
It is up to the calling function to DEALLOCATE the returned string.
==============================================================================*/

char *Computed_variable_default_evaluate_as_string_in_element(
	struct Computed_variable *variable,int component_number,
	struct FE_element *element,FE_value *xi,FE_value time,
	struct FE_element *top_level_element);
/*******************************************************************************
LAST MODIFIED : 30 November 2001

DESCRIPTION :
Returns a string representing the value of <variable>.<component_number> at
<element>:<xi>. Calls Computed_variable_evaluate_cache_in_element and converts
the value for <component_number> to a string (since result may already be in
cache).

Use -1 as the <component_number> if you want all the components.

The <top_level_element> parameter has the same use as in
Computed_variable_evaluate_cache_in_element.
==============================================================================*/

int Computed_variable_default_get_native_discretization_in_element(
	struct Computed_variable *variable,struct FE_element *element,int *number_in_xi);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Inherits its result from the first source variable.
==============================================================================*/

int Computed_variable_extract_rc(struct Computed_variable *variable,
	int element_dimension,FE_value *rc_coordinates,FE_value *rc_derivatives);
/*******************************************************************************
LAST MODIFIED : 10 February 1999

DESCRIPTION :
Takes the values in <variable> and converts them from their current coordinate
system into rectangular cartesian, returning them in the 3 component
<rc_coordinates> array. If <rc_derivatives> is not NULL, the derivatives are
also converted to rc and returned in that 9-component FE_value array.
Note that odd coordinate systems, such as FIBRE are treated as if they are
RECTANGULAR_CARTESIAN, which just causes a copy of values.
If <element_dimension> or the number of components in <variable> are less than 3,
the missing places in the <rc_coordinates> and <rc_derivatives> arrays are
cleared to zero.
???RC Uses type float for in-between values x,y,z and jacobian for future
compatibility with coordinate system transformation functions in geometry.c.
This causes a slight drop in performance.

Note the order of derivatives:
1. All the <element_dimension> derivatives of component 1.
2. All the <element_dimension> derivatives of component 2.
3. All the <element_dimension> derivatives of component 3.
==============================================================================*/

int Computed_variable_get_top_level_element_and_xi(struct FE_element *element,
	FE_value *xi, int element_dimension, struct FE_element **top_level_element,
	FE_value *top_level_xi, int *top_level_element_dimension);
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Finds the <top_level_element>, <top_level_xi> and <top_level_element_dimension>
for the given <element> and <xi>.  If <top_level_element> is already set it
is checked and the <top_level_xi> calculated.
==============================================================================*/
#endif /* defined (OLD_CODE) */

#endif /* !defined (COMPUTED_VARIABLE_PRIVATE_H) */
