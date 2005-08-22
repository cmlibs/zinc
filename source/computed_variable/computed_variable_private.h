/*******************************************************************************
FILE : computed_variable_private.h

LAST MODIFIED : 9 July 2003

DESCRIPTION :
???DB.  Move structure into .c .  Means that have to change macro into a
	function.
???DB.  Also means that macros don't know struct Cmiss_variable definition
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#if !defined (__CMISS_VARIABLE_PRIVATE_H__)
#define __CMISS_VARIABLE_PRIVATE_H__

#include "computed_variable/computed_variable.h"

/*
Method types
------------
*/
typedef void Cmiss_variable_type_specific_data;

typedef int (*Cmiss_variable_clear_type_specific_function)(
	Cmiss_variable_id variable);
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Clear the type specific data for <variable> (passed into the _set_type_
function), but don't DEALLOCATE the data.
==============================================================================*/

#define START_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION( variable_type ) \
int Cmiss_variable_ ## variable_type ## _clear_type_specific( \
	Cmiss_variable_id variable) \
{ \
	int return_code; \
	struct Cmiss_variable_ ## variable_type ## _type_specific_data *data; \
\
	ENTER(Cmiss_variable_ ## variable_type ## _clear_type_specific); \
	return_code=0; \
	data=(struct Cmiss_variable_ ## variable_type ## _type_specific_data *) \
		Cmiss_variable_get_type_specific_data(variable); \
	ASSERT_IF(data,return_code,0) \
	ASSERT_IF(Cmiss_variable_ ## variable_type ## _type_string== \
		Cmiss_variable_get_type_id_string(variable),return_code,0)

#define END_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION( variable_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Cmiss_variable_ ## variable_type ## _clear_type_specific */

typedef Cmiss_variable_type_specific_data*
	(*Cmiss_variable_duplicate_data_type_specific_function)(
	Cmiss_variable_id variable);
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Returns a duplicate of the <variable>'s type specific data.

???DB.  A copy with, source and destination, but this makes it difficult for
	MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_variable,name) to recover if this
	fails
==============================================================================*/

#define START_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
Cmiss_variable_type_specific_data * \
	Cmiss_variable_ ## variable_type ## _duplicate_data_type_specific( \
	Cmiss_variable_id variable) \
{ \
	struct Cmiss_variable_ ## variable_type ## _type_specific_data \
		*destination,*source; \
\
	ENTER(Cmiss_variable_ ## variable_type ## _duplicate_data_type_specific); \
	/* check arguments */ \
	ASSERT_IF(variable&&(Cmiss_variable_ ## variable_type ## _type_string== \
		Cmiss_variable_get_type_id_string(variable)),destination,NULL) \
	{ \
		source=Cmiss_variable_get_type_specific_data(variable); \
		ASSERT_IF(source,destination,NULL) \
		if (!ALLOCATE(destination, \
			struct Cmiss_variable_ ## variable_type ## _type_specific_data,1)) \
		{ \
			display_message(ERROR_MESSAGE,"Cmiss_variable_" #variable_type \
				"_duplicate_data_type_specific.  Could not ALLOCATE destination"); \
		} \
		else

#define END_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
	} \
	LEAVE; \
\
	return (destination); \
} /* Cmiss_variable_ ## variable_type ## _duplicate_data_type_specific */

typedef int (*Cmiss_variable_evaluate_derivative_type_specific_function)(
	Cmiss_variable_id variable,int order,
	Cmiss_variable_id *independent_variables,
	Cmiss_value_id value);
/*******************************************************************************
LAST MODIFIED : 2 July 2003

DESCRIPTION :
Evaluates the <order> degree derivative of <variable> with respect to
<independent_variables> and sets <value> to the result.
==============================================================================*/

#define START_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
int Cmiss_variable_ ## variable_type ## _evaluate_derivative_type_specific( \
	Cmiss_variable_id variable,int order, \
	Cmiss_variable_id *independent_variables, \
	Cmiss_value_id value) \
{ \
	int return_code; \
\
	ENTER(Cmiss_variable_ ## variable_type ## \
		_evaluate_derivative_type_specific); \
	return_code=0; \
	/* check arguments */ \
	ASSERT_IF(variable&&(Cmiss_variable_ ## variable_type ## _type_string== \
		Cmiss_variable_get_type_id_string(variable)&&(0<order)&& \
		independent_variables&&value),return_code,0)

#define END_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Cmiss_variable_ ## variable_type ## \
	_evaluate_derivative_type_specific */

typedef int (*Cmiss_variable_evaluate_type_specific_function)(
	struct Cmiss_variable_value *variable_value);
/*******************************************************************************
LAST MODIFIED : 3 July 2003

DESCRIPTION :
Evaluate the <variable_value>.
==============================================================================*/

#define START_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
int Cmiss_variable_ ## variable_type ## _evaluate_type_specific( \
	struct Cmiss_variable_value *variable_value) \
{ \
	int return_code; \
	Cmiss_value_id value; \
	Cmiss_variable_id variable; \
\
	ENTER(Cmiss_variable_ ## variable_type ## _evaluate_type_specific); \
	return_code=0; \
	/* check arguments */ \
	variable=Cmiss_variable_value_get_variable(variable_value); \
	value=Cmiss_variable_value_get_value(variable_value); \
	ASSERT_IF(variable&&(Cmiss_variable_ ## variable_type ## _type_string== \
		Cmiss_variable_get_type_id_string(variable))&&value,return_code,0) \

#define END_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION( variable_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Cmiss_variable_ ## variable_type ## _evaluate_type_specific */

typedef int
	(*Cmiss_variable_get_independent_variable_value_type_specific_function)(
	Cmiss_variable_id dependent_variable,
	Cmiss_variable_id independent_variable,
	Cmiss_value_id value);
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Gets the current <value> for the <independent_variable> of the
<dependent_variable>.
==============================================================================*/

#define START_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
int Cmiss_variable_ ## variable_type ## \
	_get_independent_variable_value_type_specific( \
	Cmiss_variable_id dependent_variable, \
	Cmiss_variable_id independent_variable, \
	Cmiss_value_id value) \
{ \
	int return_code; \
\
	ENTER(Cmiss_variable_ ## variable_type ## \
		_get_independent_variable_value_type_specific); \
	return_code=0; \
	/* check arguments */ \
	ASSERT_IF(dependent_variable&&(Cmiss_variable_ ## variable_type ## \
		_type_string==Cmiss_variable_get_type_id_string(dependent_variable))&& \
		independent_variable&&value,return_code,0)

#define END_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Cmiss_variable_ ## variable_type ## \
	_get_independent_variable_value_type_specific */

typedef int
	(*Cmiss_variable_get_set_independent_variable_value_type_specific_function)(
	Cmiss_variable_id dependent_variable,
	Cmiss_variable_id independent_variable,
	Cmiss_value_id value);
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Swaps the <value> and the current value for the <independent_variable> of the
<dependent_variable>.
==============================================================================*/

#define START_CMISS_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
int Cmiss_variable_ ## variable_type ## \
	_get_set_independent_variable_value_type_specific( \
	Cmiss_variable_id dependent_variable, \
	Cmiss_variable_id independent_variable, \
	Cmiss_value_id value) \
{ \
	int return_code; \
\
	ENTER(Cmiss_variable_ ## variable_type ## \
		_get_set_independent_variable_value_type_specific); \
	return_code=0; \
	/* check arguments */ \
	ASSERT_IF(dependent_variable&&(Cmiss_variable_ ## variable_type ## \
		_type_string==Cmiss_variable_get_type_id_string(dependent_variable))&& \
		independent_variable&&value,return_code,0)

#define END_CMISS_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Cmiss_variable_ ## variable_type ## \
	_get_set_independent_variable_value_type_specific */

typedef int (*Cmiss_variable_get_value_type_type_specific_function)(
	Cmiss_variable_id variable,Cmiss_value_id type);
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Gets the <type> (doesn't fill in a value) that the <variable> evaluates to.
==============================================================================*/

#define START_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
int Cmiss_variable_ ## variable_type ## _get_value_type_type_specific( \
	Cmiss_variable_id variable,Cmiss_value_id type) \
{ \
	int return_code; \
\
	ENTER(Cmiss_variable_ ## variable_type ## _get_value_type_type_specific); \
	return_code=0; \
	/* check arguments */ \
	ASSERT_IF(variable&&(Cmiss_variable_ ## variable_type ## _type_string== \
		Cmiss_variable_get_type_id_string(variable))&&type,return_code,0)

#define END_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Cmiss_variable_ ## variable_type ## _get_value_type_type_specific */

typedef int (*Cmiss_variable_is_defined_type_specific_function)(
	Cmiss_variable_id variable,struct LIST(Cmiss_variable_value) *values);
/*******************************************************************************
LAST MODIFIED : 2 July 2003

DESCRIPTION :
If the <variable> can be evaluated assuming that all its source variables can be
evaluated and that the <values> are specified, then this method returns a
non-zero.  This method is not recursive - checking through tree of source
variables is done by Cmiss_variable_is_defined.

???DB.  Which one (above or below)?  A derivative computed variable does not
	need to differentiate its source variables.  It needs do differentiate the
	source variables of the computed variable it is a derivative of.

If source variables are defined and the <variable> can be evaluated assuming
that all its source variables can be evaluated and that the <values> are
specified, then this method returns a non-zero.  This method does the is
defined checking for source variables.

???DB.  Could change so that doesn't need to look at independent/source
	variable pairs at all - Cmiss_variable_is_defined incorporates the
	independent variables into a values list and change how source/independent
	pairs are stored?
==============================================================================*/

#define START_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
int Cmiss_variable_ ## variable_type ## _is_defined_type_specific( \
	Cmiss_variable_id variable,struct LIST(Cmiss_variable_value) *values) \
{ \
	int return_code; \
\
	ENTER(Cmiss_variable_ ## variable_type ## _is_defined_type_specific); \
	return_code=0; \
	/*???DB.  temp */ \
	USE_PARAMETER(values); \
	/* check arguments */ \
	ASSERT_IF(variable&&(Cmiss_variable_ ## variable_type ## _type_string== \
		Cmiss_variable_get_type_id_string(variable)),return_code,0)

#define END_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Cmiss_variable_ ## variable_type ## _is_defined_type_specific */

typedef int
	(*Cmiss_variable_is_independent_variable_of_type_specific_function)(
	Cmiss_variable_id dependent_variable,
	Cmiss_variable_id independent_variable);
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Returns non-zero if <dependent_variable> has the <independent_variable> and zero
otherwise.
==============================================================================*/

#define START_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
int Cmiss_variable_ ## variable_type ## \
	_is_independent_variable_of_type_specific( \
	Cmiss_variable_id dependent_variable, \
	Cmiss_variable_id independent_variable) \
{ \
	int return_code; \
\
	ENTER(Cmiss_variable_ ## variable_type ## \
		_is_independent_variable_of_type_specific); \
	return_code=0; \
	/* check arguments */ \
	ASSERT_IF(dependent_variable&&(Cmiss_variable_ ## variable_type \
		## _type_string==Cmiss_variable_get_type_id_string(dependent_variable))&& \
		independent_variable,return_code,0)

#define END_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Cmiss_variable_ ## variable_type ## \
	_is_independent_variable_of_type_specific */

typedef int (*Cmiss_variable_not_in_use_type_specific_function)(
	Cmiss_variable_id variable);
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Returns non-zero if none of its type specific objects (in data) are in use
(ACCESSed) and zero otherwise.
==============================================================================*/

#define START_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
int Cmiss_variable_ ## variable_type ## _not_in_use_type_specific( \
	Cmiss_variable_id variable) \
{ \
	int return_code; \
\
	ENTER(Cmiss_variable_ ## variable_type ## _not_in_use_type_specific); \
	return_code=0; \
	/* check arguments */ \
	ASSERT_IF(variable&&(Cmiss_variable_ ## variable_type ## _type_string== \
		Cmiss_variable_get_type_id_string(variable)),return_code,0)

#define END_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Cmiss_variable_ ## variable_type ## _not_in_use_type_specific */

typedef int (*Cmiss_variable_overlap_type_specific_function)(
	Cmiss_variable_id variable_1,Cmiss_variable_id variable_2);
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Returns non-zero if <variable_1> and <variable_2> share variables (eg [value,
d/ds1] and [d/ds1,d/ds2] overlap) and zero otherwise.
==============================================================================*/

#define START_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
int Cmiss_variable_ ## variable_type ## _overlap_type_specific( \
	Cmiss_variable_id variable_1,Cmiss_variable_id variable_2) \
{ \
	int return_code; \
\
	ENTER(Cmiss_variable_ ## variable_type ## _overlap_type_specific); \
	return_code=0; \
	/* check arguments */ \
	ASSERT_IF(variable_1&&(Cmiss_variable_ ## variable_type ## _type_string== \
		Cmiss_variable_get_type_id_string(variable_1))&&variable_2&& \
		(Cmiss_variable_ ## variable_type ## _type_string== \
		Cmiss_variable_get_type_id_string(variable_2)),return_code,0)

#define END_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Cmiss_variable_ ## variable_type ## _overlap_type_specific */

typedef int (*Cmiss_variable_same_variable_type_specific_function)(
	Cmiss_variable_id variable_1,Cmiss_variable_id variable_2);
/*******************************************************************************
LAST MODIFIED : 19 May 2003

DESCRIPTION :
Returns nonzero if <variable_1> and <variable_2> are the same variable (eg. the
value at node 10 for the finite element field bob) and zero otherwise.
==============================================================================*/

#define START_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
int Cmiss_variable_ ## variable_type ## _same_variable_type_specific( \
	Cmiss_variable_id variable_1,Cmiss_variable_id variable_2) \
{ \
	int return_code; \
\
	ENTER(Cmiss_variable_ ## variable_type ## _same_variable_type_specific); \
	return_code=0; \
	/* check arguments */ \
	ASSERT_IF(variable_1&&variable_2,return_code,0) \
	if ((Cmiss_variable_ ## variable_type ## _type_string== \
		Cmiss_variable_get_type_id_string(variable_1))&& \
		(Cmiss_variable_ ## variable_type ## _type_string== \
		Cmiss_variable_get_type_id_string(variable_2)))

#define END_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Cmiss_variable_ ## variable_type ## _same_variable_type_specific */

typedef int
	(*Cmiss_variable_set_independent_variable_value_type_specific_function)(
	Cmiss_variable_id dependent_variable,
	Cmiss_variable_id independent_variable,
	Cmiss_value_id value);
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Sets the <value> of the <independent_variable> for the <variable>.
==============================================================================*/

#define START_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
int Cmiss_variable_ ## variable_type ## \
	_set_independent_variable_value_type_specific( \
	Cmiss_variable_id dependent_variable, \
	Cmiss_variable_id independent_variable,Cmiss_value_id value) \
{ \
	int return_code; \
\
	ENTER(Cmiss_variable_ ## variable_type ## \
		_set_independent_variable_value_type_specific); \
	return_code=0; \
	/* check arguments */ \
	ASSERT_IF(dependent_variable&& \
		(Cmiss_variable_ ## variable_type ## _type_string== \
		Cmiss_variable_get_type_id_string(dependent_variable))&& \
		independent_variable&&value,return_code,0)

#define END_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION( \
	variable_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Cmiss_variable_ ## variable_type ## \
	_set_independent_variable_value_type_specific */


/*
Friend macros
-------------
*/
#define DECLARE_CMISS_VARIABLE_IS_TYPE_FUNCTION( variable_type ) \
PROTOTYPE_CMISS_VARIABLE_IS_TYPE_FUNCTION(variable_type) \
{ \
	int return_code; \
\
	ENTER(CMISS_VARIABLE_IS_TYPE(variable_type)); \
	return_code=0; \
	/* check argument */ \
	if (variable) \
	{ \
		if (Cmiss_variable_ ## variable_type ## _type_string== \
			Cmiss_variable_get_type_id_string(variable)) \
		{ \
			return_code=1; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"CMISS_VARIABLE_IS_TYPE(" #variable_type \
			").  " "Missing variable"); \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CMISS_VARIABLE_IS_TYPE(variable_type) */

#define CMISS_VARIABLE_ESTABLISH_METHODS( variable, variable_type ) \
/***************************************************************************** \
LAST MODIFIED : 10 February 2003 \
\
DESCRIPTION : \
Each Cmiss_variable_set_type function should call this macro to establish \
the virtual functions that give the variable its particular behaviour.  Each \
function must therefore be defined for each variable type, even if it is set \
to NULL or some default function. \
============================================================================*/ \
Cmiss_variable_establish_methods(variable, \
	Cmiss_variable_ ## variable_type ## _clear_type_specific, \
	Cmiss_variable_ ## variable_type ## _duplicate_data_type_specific, \
	Cmiss_variable_ ## variable_type ## _evaluate_derivative_type_specific, \
	Cmiss_variable_ ## variable_type ## _evaluate_type_specific, \
	Cmiss_variable_ ## variable_type ## _get_independent_variable_value_type_specific, \
	Cmiss_variable_ ## variable_type ## _get_set_independent_variable_value_type_specific, \
	Cmiss_variable_ ## variable_type ## _get_value_type_type_specific, \
	Cmiss_variable_ ## variable_type ## _is_defined_type_specific, \
	Cmiss_variable_ ## variable_type ## _is_independent_variable_of_type_specific, \
	Cmiss_variable_ ## variable_type ## _not_in_use_type_specific, \
	Cmiss_variable_ ## variable_type ## _overlap_type_specific, \
	Cmiss_variable_ ## variable_type ## _same_variable_type_specific, \
	Cmiss_variable_ ## variable_type ## _set_independent_variable_value_type_specific)


/*
Friend functions
----------------
*/
int Cmiss_variable_establish_methods(Cmiss_variable_id variable,
	Cmiss_variable_clear_type_specific_function clear_type_specific_function,
	Cmiss_variable_duplicate_data_type_specific_function
	duplicate_data_type_specific_function,
	Cmiss_variable_evaluate_derivative_type_specific_function
	evaluate_derivative_type_specific_function,
	Cmiss_variable_evaluate_type_specific_function
	evaluate_type_specific_function,
	Cmiss_variable_get_independent_variable_value_type_specific_function
	get_independent_variable_value_type_specific_function,
	Cmiss_variable_get_set_independent_variable_value_type_specific_function
	get_set_independent_variable_value_type_specific_function,
	Cmiss_variable_get_value_type_type_specific_function
	get_value_type_type_specific_function,
	Cmiss_variable_is_defined_type_specific_function
	is_defined_type_specific_function,
	Cmiss_variable_is_independent_variable_of_type_specific_function
	is_independent_variable_of_type_specific_function,
	Cmiss_variable_not_in_use_type_specific_function
	not_in_use_type_specific_function,
	Cmiss_variable_overlap_type_specific_function overlap_type_specific_function,
	Cmiss_variable_same_variable_type_specific_function
	same_variable_type_specific_function,
	Cmiss_variable_set_independent_variable_value_type_specific_function
	set_independent_variable_value_type_specific_function);
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Sets the methods for the <variable>.
==============================================================================*/

Cmiss_variable_type_specific_data *Cmiss_variable_get_type_specific_data(
	Cmiss_variable_id variable);
/*******************************************************************************
LAST MODIFIED : 6 February 2003

DESCRIPTION :
Returns the type specific data for the <variable>.
==============================================================================*/

int Cmiss_variable_set_type_specific_information(
	Cmiss_variable_id variable,char *type_string,
	void *type_specific_data);
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
Sets the type specific information for the <variable>.
==============================================================================*/

int Cmiss_variable_clear_type(Cmiss_variable_id variable);
/*******************************************************************************
LAST MODIFIED : 27 January 2003

DESCRIPTION :
Used internally by DESTROY and Cmiss_variable_set_type_*() functions to
deallocate or deaccess data specific to any Cmiss_variable_type.  Functions
changing the type of the Cmiss_variable should
- allocate any dynamic data needed for the type
- call this function to clear what is currently in the variable
- then set values
to ensure that the variable is not left in an invalid state.
==============================================================================*/

int Cmiss_variable_not_in_use(Cmiss_variable_id variable);
/*******************************************************************************
LAST MODIFIED : 11 February 2003

DESCRIPTION :
To give access to Cmiss_variable_not_in_use_type_specific_function method.
==============================================================================*/
#endif /* !defined (__CMISS_VARIABLE_PRIVATE_H__) */
