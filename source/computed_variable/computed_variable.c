/*******************************************************************************
FILE : computed_variable.c

LAST MODIFIED : 20 July 2003

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

???DB.  Do without components?

???DB.  Need Cmiss_variable_changed?

???DB.  Can Cmiss_values be got rid of by moving data and methods into
	corresponding Cmiss_variable (ie. a variable always has a value)?

???DB.  Great similarity between EVALUATE_DERIVATIVEs (and other methods) for
	"simple" variable types such as element_xi and coordinates.  Make
	macro/function?
	Setting up derivative matrix needs tidying because need to know about too many
	classes - have to do testing of value_type after Cmiss_variable_get_value_type

???DB.  Combine CREATE/DESTROY and ACCESS/DEACCESS into one pair?

???DB.  Need composite and identity variables to do composition variable
	derivatives
	- will need to include computed_variable_composite.h in this file so that
		composite independent variables can be handled in
		Cmiss_variable_evaluate_derivative.  This would be a circular include if
		there wasn't the #if defined's at the beginning of the .h's
	- identity will also need to know about composite to get identity matrix for
		derivative?

???DB.  Why does IS_DEFINED have values?

???DB.  Should CMISS_VARIABLE_NOT_IN_USE just look at access_count?  Description
	says that it checks for any type specific data being in use
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
/* computed_value_derivative_matrix, computed_value_matrix and
	computed_variable_composite are needed for splitting up independent variables
	in Cmiss_variable_evaluate_derivative */
#include "computed_variable/computed_value_derivative_matrix.h"
#include "computed_variable/computed_value_matrix.h"
#include "computed_variable/computed_variable_composite.h"
#include "computed_variable/computed_variable_private.h"
#include "general/indexed_list_private.h"
#include "general/debug.h"
#include "general/list_private.h"
#include "general/manager_private.h"
#include "general/mystring.h"
#include "user_interface/message.h"

/*
Module constants
----------------
*/

/*
Module types
------------
*/
struct Cmiss_variable
/*******************************************************************************
LAST MODIFIED : 9 July 2003

DESCRIPTION :
1 What to do about operators?
	- every variable is a function xor an operator (function and operator is too
		complicated)
	- composition is an operator as well as derivative, divergence, gradient,
		integral.  Get rid of
		Cmiss_variable_set_independent_variable_source_variable, because part of
		set_type for composition?
	- could probably make work with source/independent moved into compose type
		specific data, by having the set value put the variable-value pair into a
		list inside the operators type specific data - trouble is double handling.
		Alternative is to put variable-value pairs into the evaluate arguments and
		have different START and END macros for functions and operators
2 Combine variables and values?  Like a variable-value pair (since variables are
	having room for a value)?
==============================================================================*/
{
	/* the name/identifier of the Cmiss_variable */
	char *name;
	/* the type string identifies the variable's type.  It points to a string that
		is shared by all variables of the same type and should not be copied */
	char *type_string;
	/* information that is specific to the type */
	Cmiss_variable_type_specific_data *type_specific_data;
	/* methods.   To add a new method, add
		- a field here in struct Cmiss_variable
		- a function type declaration in computed_variable_private.h
		- an argument and an assigment to Cmiss_variable_establish_methods
		The compiler will force the other changes because
		Cmiss_variable_establish_methods is used in
		- Cmiss_variable_clear_type
		- CREATE(Cmiss_variable)
		- MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_variable,name) */
	Cmiss_variable_clear_type_specific_function clear_type_specific_function;
	Cmiss_variable_duplicate_data_type_specific_function
		duplicate_data_type_specific_function;
	Cmiss_variable_evaluate_derivative_type_specific_function
		evaluate_derivative_type_specific_function;
	Cmiss_variable_evaluate_type_specific_function
		evaluate_type_specific_function;
	Cmiss_variable_get_independent_variable_value_type_specific_function
		get_independent_variable_value_type_specific_function;
	Cmiss_variable_get_set_independent_variable_value_type_specific_function
		get_set_independent_variable_value_type_specific_function;
	Cmiss_variable_get_value_type_type_specific_function
		get_value_type_type_specific_function;
	Cmiss_variable_is_defined_type_specific_function
		is_defined_type_specific_function;
	Cmiss_variable_is_independent_variable_of_type_specific_function
		is_independent_variable_of_type_specific_function;
	Cmiss_variable_not_in_use_type_specific_function
		not_in_use_type_specific_function;
	Cmiss_variable_overlap_type_specific_function overlap_type_specific_function;
	Cmiss_variable_same_variable_type_specific_function
		same_variable_type_specific_function;
	Cmiss_variable_set_independent_variable_value_type_specific_function
		set_independent_variable_value_type_specific_function;

	int access_count;
}; /* struct Cmiss_variable */

FULL_DECLARE_INDEXED_LIST_TYPE(Cmiss_variable);

FULL_DECLARE_MANAGER_TYPE(Cmiss_variable);

struct Cmiss_variable_value
/*******************************************************************************
LAST MODIFIED : 27 January 2003

DESCRIPTION :
A variable/value pair for specifying the value of the variable.
==============================================================================*/
{
	Cmiss_variable_id variable;
	Cmiss_value_id value;
	int access_count;
}; /* struct Cmiss_variable_value */

FULL_DECLARE_LIST_TYPE(Cmiss_variable_value);

struct Cmiss_variable_package
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Allows sharing of Cmiss_variables with the same name.
==============================================================================*/
{
	struct MANAGER(Cmiss_variable) *variable_manager;
}; /* struct Cmiss_variable_package */

/*
Module functions
----------------
*/
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Cmiss_variable,name,char *,strcmp)

DECLARE_LOCAL_MANAGER_FUNCTIONS(Cmiss_variable)

static int Cmiss_variable_is_source_variable_conditional(
	Cmiss_variable_id variable,void *other_variable_void)
/*******************************************************************************
LAST MODIFIED : 4 February 2003

DESCRIPTION :
List conditional function returning true if <variable> is a source variable of
<other_variable>, ie. <other_variable> depends on <variable>.

NB.  Because used for a list of source variable is not
Cmiss_variable_depends_on_Cmiss_variable_conditional (other way round).
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_is_source_variable_conditional);
	return_code=Cmiss_variable_depends_on_Cmiss_variable(
		(Cmiss_variable_id )other_variable_void,variable);
	LEAVE;

	return (return_code);
} /* Cmiss_variable_is_source_variable_conditional */

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
	set_independent_variable_value_type_specific_function)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Sets the methods for the <variable>.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_establish_methods);
	return_code=0;
	if (variable)
	{
		variable->clear_type_specific_function=clear_type_specific_function;
		variable->duplicate_data_type_specific_function=
			duplicate_data_type_specific_function;
		variable->evaluate_derivative_type_specific_function=
			evaluate_derivative_type_specific_function;
		variable->evaluate_type_specific_function=evaluate_type_specific_function;
		variable->get_independent_variable_value_type_specific_function=
			get_independent_variable_value_type_specific_function;
		variable->get_set_independent_variable_value_type_specific_function=
			get_set_independent_variable_value_type_specific_function;
		variable->get_value_type_type_specific_function=
			get_value_type_type_specific_function;
		variable->is_defined_type_specific_function=
			is_defined_type_specific_function;
		variable->is_independent_variable_of_type_specific_function=
			is_independent_variable_of_type_specific_function;
		variable->not_in_use_type_specific_function=
			not_in_use_type_specific_function;
		variable->overlap_type_specific_function=overlap_type_specific_function;
		variable->same_variable_type_specific_function=
			same_variable_type_specific_function;
		variable->set_independent_variable_value_type_specific_function=
			set_independent_variable_value_type_specific_function;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_establish_methods.  "
			"Missing variable");
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_establish_methods */

Cmiss_variable_type_specific_data *Cmiss_variable_get_type_specific_data(
	Cmiss_variable_id variable)
/*******************************************************************************
LAST MODIFIED : 6 February 2003

DESCRIPTION :
Returns the type specific data for the <variable>.
==============================================================================*/
{
	Cmiss_variable_type_specific_data *data;

	ENTER(Cmiss_variable_get_type_specific_data);
	data=(Cmiss_variable_type_specific_data *)NULL;
	ASSERT_IF(variable,data,(Cmiss_variable_type_specific_data *)NULL)
	{
		data=variable->type_specific_data;
	}
	LEAVE;

	return (data);
} /* Cmiss_variable_get_type_specific_data */

int Cmiss_variable_set_type_specific_information(
	Cmiss_variable_id variable,char *type_string,
	void *type_specific_data)
/*******************************************************************************
LAST MODIFIED : 24 April 2003

DESCRIPTION :
Sets the type specific information for the <variable>.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_set_type_specific_information);
	return_code=0;
	/* check arguments */
	if (variable&&type_string&&type_specific_data)
	{
		variable->type_specific_data=type_specific_data;
		variable->type_string=type_string;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_variable_set_type_specific_information.  "
			"Invalid argument(s).  %p %p %p",variable,type_string,type_specific_data);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_set_type_specific_information */

int Cmiss_variable_clear_type(Cmiss_variable_id variable)
/*******************************************************************************
LAST MODIFIED : 20 July 2003

DESCRIPTION :
Used internally by DESTROY and Cmiss_variable_set_type_*() functions to
deallocate or deaccess data specific to any Cmiss_variable_type.  Functions
changing the type of the Cmiss_variable should
- allocate any dynamic data needed for the type
- call this function to clear what is currently in the variable
- then set values
to ensure that the variable is not left in an invalid state.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_clear_type);
	return_code=0;
	/* check arguments */
	if (variable)
	{
		/* clear the type specific data first */
		if (variable->type_specific_data)
		{
			if (variable->clear_type_specific_function)
			{
				return_code=(variable->clear_type_specific_function)(variable);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_variable_clear_type.  "
					"Type specific data but no function to clear it");
				return_code=0;
			}
			DEALLOCATE(variable->type_specific_data);
		}
		else
		{
			return_code=1;
		}
		/* clear all methods */
		Cmiss_variable_establish_methods(variable,
			(Cmiss_variable_clear_type_specific_function)NULL,
			(Cmiss_variable_duplicate_data_type_specific_function)NULL,
			(Cmiss_variable_evaluate_derivative_type_specific_function)NULL,
			(Cmiss_variable_evaluate_type_specific_function)NULL,
			(Cmiss_variable_get_independent_variable_value_type_specific_function)NULL,
			(Cmiss_variable_get_set_independent_variable_value_type_specific_function)NULL,
			(Cmiss_variable_get_value_type_type_specific_function)NULL,
			(Cmiss_variable_is_defined_type_specific_function)NULL,
			(Cmiss_variable_is_independent_variable_of_type_specific_function)NULL,
			(Cmiss_variable_not_in_use_type_specific_function)NULL,
			(Cmiss_variable_overlap_type_specific_function)NULL,
			(Cmiss_variable_same_variable_type_specific_function)NULL,
			(Cmiss_variable_set_independent_variable_value_type_specific_function)
			NULL);
		variable->type_string=(char *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_clear_type.  "
			"Missing variable");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_clear_type */

int Cmiss_variable_not_in_use(Cmiss_variable_id variable)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
To give access to Cmiss_variable_not_in_use_type_specific_function method.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_not_in_use);
	return_code=0;
	/* check arguments */
	if (variable)
	{
		if (variable->not_in_use_type_specific_function)
		{
			return_code=(variable->not_in_use_type_specific_function)(variable);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_not_in_use.  "
				"Type specific data but no function to clear it");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_not_in_use.  "
			"Missing variable");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_not_in_use */

/*
Global functions
----------------
*/
Cmiss_variable_id CREATE(Cmiss_variable)(
	struct Cmiss_variable_package *package,char *name)
/*******************************************************************************
LAST MODIFIED : 10 July 2003

DESCRIPTION :
If there is a <package> and a Cmiss_variable in the <package> with the
specified <name> then the Cmiss_variable is returned.

Otherwise, an empty variable with specified <name> and no type, dependent
variables or independent variables is created and returned.  If there is a
<package> then the created Cmiss_variables is put in the <package>.  The
<name> is copied.

Each type of variable has its own "set_type" function.
==============================================================================*/
{
	Cmiss_variable_id variable;

	ENTER(CREATE(Cmiss_variable));
	variable=(Cmiss_variable_id )NULL;
	if (name&&(!package||(package&&(package->variable_manager))))
	{
		if (package)
		{
			variable=FIND_BY_IDENTIFIER_IN_MANAGER(Cmiss_variable,name)(name,
				package->variable_manager);
		}
		if (!variable)
		{
			if (ALLOCATE(variable,struct Cmiss_variable,1)&&
				(variable->name=duplicate_string(name)))
			{
				/* initialise data */
				variable->type_string=(char *)NULL;
				variable->type_specific_data=NULL;
				/* initialise methods */
				Cmiss_variable_establish_methods(variable,
					(Cmiss_variable_clear_type_specific_function)NULL,
					(Cmiss_variable_duplicate_data_type_specific_function)NULL,
					(Cmiss_variable_evaluate_derivative_type_specific_function)NULL,
					(Cmiss_variable_evaluate_type_specific_function)NULL,
					(Cmiss_variable_get_independent_variable_value_type_specific_function)NULL,
					(Cmiss_variable_get_set_independent_variable_value_type_specific_function)NULL,
					(Cmiss_variable_get_value_type_type_specific_function)NULL,
					(Cmiss_variable_is_defined_type_specific_function)NULL,
					(Cmiss_variable_is_independent_variable_of_type_specific_function)
					NULL,
					(Cmiss_variable_not_in_use_type_specific_function)NULL,
					(Cmiss_variable_overlap_type_specific_function)NULL,
					(Cmiss_variable_same_variable_type_specific_function)NULL,
					(Cmiss_variable_set_independent_variable_value_type_specific_function)NULL);
				/* initialise access_count */
				variable->access_count=0;
				if (package)
				{
					if (!ADD_OBJECT_TO_MANAGER(Cmiss_variable)(variable,
						package->variable_manager))
					{
						display_message(ERROR_MESSAGE,"CREATE(Cmiss_variable).  "
							"Could not add variable to manager");
						DESTROY(Cmiss_variable)(&variable);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"CREATE(Cmiss_variable).  "
					"Insufficient memory");
				DEALLOCATE(variable);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Cmiss_variable).  "
			"Missing name or invalid package.  %p %p",name,package);
	}
	LEAVE;

	return (variable);
} /* CREATE(Cmiss_variable) */

int DESTROY(Cmiss_variable)(Cmiss_variable_id *variable_address)
/*******************************************************************************
LAST MODIFIED : 5 February 2003

DESCRIPTION :
Frees memory/deaccess objects for Cmiss_variable at <*variable_address>.
==============================================================================*/
{
	int return_code;
	Cmiss_variable_id variable;

	ENTER(DESTROY(Cmiss_variable));
	return_code=0;
	if (variable_address&&(variable= *variable_address))
	{
		if (0>=variable->access_count)
		{
			DEALLOCATE(variable->name);
			Cmiss_variable_clear_type(variable);
			DEALLOCATE(*variable_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Cmiss_variable).  Positive access_count");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Cmiss_variable).  Missing variable");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Cmiss_variable) */

int Cmiss_variable_copy(Cmiss_variable_id destination,
	Cmiss_variable_id source)
/*******************************************************************************
LAST MODIFIED : 10 July 2003

DESCRIPTION :
Copies the type and contents from <source> to <destination>.

???DB.  What if the access_count>0?  Put in Cmiss_variable_clear_type?
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_copy);
	return_code=0;
	/* check arguments */
	if (destination&&source)
	{
		if (Cmiss_variable_clear_type(destination))
		{
			/* initialise data */
			destination->type_string=source->type_string;
			if (source->duplicate_data_type_specific_function)
			{
				destination->type_specific_data=
					(source->duplicate_data_type_specific_function)(source);
			}
			/* initialise methods */
			Cmiss_variable_establish_methods(destination,
				source->clear_type_specific_function,
				source->duplicate_data_type_specific_function,
				source->evaluate_derivative_type_specific_function,
				source->evaluate_type_specific_function,
				source->get_independent_variable_value_type_specific_function,
				source->get_set_independent_variable_value_type_specific_function,
				source->get_value_type_type_specific_function,
				source->is_defined_type_specific_function,
				source->is_independent_variable_of_type_specific_function,
				source->not_in_use_type_specific_function,
				source->overlap_type_specific_function,
				source->same_variable_type_specific_function,
				source->set_independent_variable_value_type_specific_function);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_copy.  "
			"Invalid argument(s).  %p %p",destination,source);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_copy */

DECLARE_OBJECT_FUNCTIONS(Cmiss_variable)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Cmiss_variable)

DECLARE_INDEXED_LIST_FUNCTIONS(Cmiss_variable)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Cmiss_variable,name,
	char *,strcmp)

DECLARE_INDEXED_LIST_IDENTIFIER_CHANGE_FUNCTIONS(Cmiss_variable,name)

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(Cmiss_variable,name)
{
	char *name;
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(Cmiss_variable,name));
	if (source&&destination)
	{
		if (source->name)
		{
			if (ALLOCATE(name,char,strlen(source->name)+1))
			{
				strcpy(name,source->name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_WITH_IDENTIFIER(Cmiss_variable,name).  "
					"Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			if (return_code=MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_variable,name)(
				destination,source))
			{
				/* copy values */
				DEALLOCATE(destination->name);
				destination->name=name;
			}
			else
			{
				DEALLOCATE(name);
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_WITH_IDENTIFIER(Cmiss_variable,name).  "
					"Could not copy without identifier");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITH_IDENTIFIER(Cmiss_variable,name).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(Cmiss_variable,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Cmiss_variable,name)
/*******************************************************************************
LAST MODIFIED : 10 July 2003

DESCRIPTION :
Do not allow copy if:
- it creates a self-referencing variable (one that depends on itself) which will
	result in an infinite loop;

???RC.  Previously denied copy if read_only flag was set in the destination
	variable.  However, this makes it impossible to modify computed variables that
	wrap FE_fields when the latter changes.  Hence, now leave it up to
	define/destroy functions to check if read_only flag is set.
==============================================================================*/
{
	int return_code;
	void *type_specific_data;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_variable,name));
	if (source&&destination)
	{
		/* check <source> does not depend on <destination> else infinite loop */
		if (Cmiss_variable_depends_on_Cmiss_variable(source,destination))
		{
			display_message(ERROR_MESSAGE,
				"MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_variable,name).  "
				"Cannot make variable depend on itself");
			return_code=0;
		}
		else
		{
			type_specific_data=NULL;
			return_code=1;
			if (source->type_specific_data)
			{
				if (source->duplicate_data_type_specific_function)
				{
					if (!(type_specific_data=
						(source->duplicate_data_type_specific_function)(source)))
					{
						display_message(ERROR_MESSAGE,
							"MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_variable,name).  "
							"Type specific duplicate data function failed.");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_variable,name).  "
						"Type specific data but no duplicate data function.");
					return_code=0;
				}
			}
			if (return_code)
			{
				/* free current type-specific data */
				Cmiss_variable_clear_type(destination);
				/* establish the new type */
				destination->type_string=source->type_string;
				destination->type_specific_data=type_specific_data;
				return_code=Cmiss_variable_establish_methods(destination,
					source->clear_type_specific_function,
					source->duplicate_data_type_specific_function,
					source->evaluate_derivative_type_specific_function,
					source->evaluate_type_specific_function,
					source->get_independent_variable_value_type_specific_function,
					source->get_set_independent_variable_value_type_specific_function,
					source->get_value_type_type_specific_function,
					source->is_defined_type_specific_function,
					source->is_independent_variable_of_type_specific_function,
					source->not_in_use_type_specific_function,
					source->overlap_type_specific_function,
					source->same_variable_type_specific_function,
					source->set_independent_variable_value_type_specific_function);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_variable,name).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_variable,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Cmiss_variable,name,char *)
{
	char *destination_name;
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(Cmiss_variable,name));
	return_code=0;
	if (name&&destination)
	{
		if (name)
		{
			if (ALLOCATE(destination_name,char,strlen(name)+1))
			{
				strcpy(destination_name,name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_IDENTIFIER(Cmiss_variable,name).  "
					"Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			/* copy name */
			DEALLOCATE(destination->name);
			destination->name=destination_name;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Cmiss_variable,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(Cmiss_variable,name) */

DECLARE_MANAGER_FUNCTIONS(Cmiss_variable)

PROTOTYPE_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Cmiss_variable)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Cmiss_variable requires a special version of this function because it may
use objects (eg FE_fields) that are also used.  If the computed variable is not
itself in use, it calls the variable's optional not_in_use function and bases
its result on that.

???DB.  What about source fields?
==============================================================================*/
{
	int return_code;

	ENTER(MANAGED_OBJECT_NOT_IN_USE(Cmiss_variable));
	return_code=0;
	if (manager&&object)
	{
		if (!(manager->locked))
		{
			if (IS_OBJECT_IN_LIST(Cmiss_variable)(object,manager->object_list))
			{
				if ((1==object->access_count)||
					((2==object->access_count)&&
					IS_OBJECT_IN_LIST(Cmiss_variable)(object,
					manager->message->changed_object_list)))
				{
					if (object->not_in_use_type_specific_function)
					{
						return_code=(object->not_in_use_type_specific_function)(object);
					}
					else
					{
						return_code=1;
					}
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"MANAGED_OBJECT_NOT_IN_USE(Cmiss_variable).  "
					"Object is not managed");
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"MANAGED_OBJECT_NOT_IN_USE(Cmiss_variable).  Manager is locked");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGED_OBJECT_NOT_IN_USE(Cmiss_variable).  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* MANAGED_OBJECT_NOT_IN_USE(Cmiss_variable) */

DECLARE_ADD_OBJECT_TO_MANAGER_FUNCTION(Cmiss_variable,name)

DECLARE_MANAGER_MODIFY_FUNCTION(Cmiss_variable,name)
	/*???DB.  Don't need special version because don't have components */
DECLARE_MANAGER_MODIFY_NOT_IDENTIFIER_FUNCTION(Cmiss_variable,name)
	/*???DB.  Don't need special version because don't have components */

DECLARE_MANAGER_MODIFY_IDENTIFIER_FUNCTION(Cmiss_variable,name,char *)
DECLARE_FIND_BY_IDENTIFIER_IN_MANAGER_FUNCTION(Cmiss_variable,name,char *)

int Cmiss_variable_same_variable(Cmiss_variable_id variable_1,
	Cmiss_variable_id variable_2)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Returns nonzero if <variable_1> and <variable_2> are the same variable (eg. the
value at node 10 for the finite element field bob) and zero otherwise.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_same_variable);
	return_code=0;
	if (variable_1&&variable_2)
	{
		if (variable_1->same_variable_type_specific_function)
		{
			return_code=(variable_1->same_variable_type_specific_function)(variable_1,
				variable_2);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_variable_same_variable.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_same_variable.  "
			"Invalid argument(s).  %p %p",variable_1,variable_2);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_same_variable */

int Cmiss_variable_overlap(Cmiss_variable_id variable_1,
	Cmiss_variable_id variable_2)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Returns nonzero if <variable_1> and <variable_2> overlap (eg d/ds1 and all
values) and zero otherwise.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_overlap);
	return_code=0;
	if (variable_1&&variable_2)
	{
		if (variable_1->overlap_type_specific_function)
		{
			return_code=(variable_1->overlap_type_specific_function)(variable_1,
				variable_2);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_variable_overlap.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_overlap.  "
			"Invalid argument(s).  %p %p",variable_1,variable_2);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_overlap */

int Cmiss_variable_is_independent_variable_of(
	Cmiss_variable_id dependent_variable,
	Cmiss_variable_id independent_variable)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Returns nonzero if <independent_variable> is an independent variable of
<dependent_variable> and zero otherwise.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_is_independent_variable_of);
	return_code=0;
	if (dependent_variable&&independent_variable)
	{
		if (dependent_variable->is_independent_variable_of_type_specific_function)
		{
			return_code=
				(dependent_variable->is_independent_variable_of_type_specific_function)(
				dependent_variable,independent_variable);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_is_independent_variable_of.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_variable_is_independent_variable_of.  "
			"Invalid argument(s).  %p %p",dependent_variable,independent_variable);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_is_independent_variable_of */

char *Cmiss_variable_get_type_id_string(Cmiss_variable_id variable)
/*******************************************************************************
LAST MODIFIED : 7 February 2003

DESCRIPTION :
Returns the string which identifies the type.  The calling function must not
DEALLOCATE the returned string.
==============================================================================*/
{
	char *return_string;

	ENTER(Cmiss_variable_get_type_id_string);
	return_string=(char *)NULL;
	if (variable)
	{
		return_string=variable->type_string;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_get_type_id_string.  "
			"Missing variable");
	}
	LEAVE;

	return (return_string);
} /* Cmiss_variable_get_type_id_string */

int Cmiss_variable_get_value_type(Cmiss_variable_id variable,
	Cmiss_value_id type)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Sets the <type> to be the same as the type of the results of <variable>, but
does not set/calculate the actual value.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_get_value_type);
	return_code=0;
	if (variable&&type)
	{
		if (variable->get_value_type_type_specific_function)
		{
			return_code=(variable->get_value_type_type_specific_function)(variable,
				type);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_variable_get_value_type.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_get_value_type.  "
			"Invalid argument(s).  %p %p",variable,type);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_get_value_type */

int Cmiss_variable_same_value_type(Cmiss_variable_id variable_1,
	Cmiss_variable_id variable_2)
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
Returns nonzero if <variable_1> and <variable_2> have the same value type and
zero otherwise.
==============================================================================*/
{
	int return_code;
	Cmiss_value_id value_1,value_2;

	ENTER(Cmiss_variable_same_value_type);
	return_code=0;
	if (variable_1&&variable_2)
	{
		value_1=CREATE(Cmiss_value)();
		value_2=CREATE(Cmiss_value)();
		if (value_1&&value_2)
		{
			if (Cmiss_variable_get_value_type(variable_1,value_1)&&
				Cmiss_variable_get_value_type(variable_2,value_2))
			{
				return_code=Cmiss_value_same_sub_type(value_1,value_2);
			}
			else
			{
				display_message(ERROR_MESSAGE,"Cmiss_variable_same_value_type.  "
					"Could not get value types");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_variable_same_value_type.  "
				"Could not create value types.  %p %p",value_1,value_2);
		}
		DESTROY(Cmiss_value)(&value_1);
		DESTROY(Cmiss_value)(&value_2);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_same_value_type.  "
			"Invalid argument(s).  %p %p",variable_1,variable_2);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_same_value_type */

int Cmiss_variable_set_independent_variable_value(
	Cmiss_variable_id variable,
	Cmiss_variable_id independent_variable,
	Cmiss_value_id value)
/*******************************************************************************
LAST MODIFIED : 10 July 2003

DESCRIPTION :
Sets the <value> of the <independent_variable> for the <variable>.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_set_independent_variable_value);
	return_code=0;
	/* check arguments */
	if (variable&&independent_variable&&value)
	{
		if (variable->set_independent_variable_value_type_specific_function)
		{
			return_code=
				(variable->set_independent_variable_value_type_specific_function)(
				variable,independent_variable,value);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_set_independent_variable_value.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_variable_set_independent_variable_value.  "
			"Invalid argument(s).  %p %p %p",variable,independent_variable,value);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_set_independent_variable_value */

int Cmiss_variable_get_independent_variable_value(
	Cmiss_variable_id variable,
	Cmiss_variable_id independent_variable,
	Cmiss_value_id value)
/*******************************************************************************
LAST MODIFIED : 10 July 2003

DESCRIPTION :
Gets the <value> of the <independent_variable> for the <variable>.  If the
<value> has no type, then the default type for the <independent_variable> is
used otherwise a representation in <value>'s type is attempted.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_get_independent_variable_value);
	return_code=0;
	/* check arguments */
	if (variable&&independent_variable&&value)
	{
		if (variable->get_independent_variable_value_type_specific_function)
		{
			return_code=
				(variable->get_independent_variable_value_type_specific_function)(
				variable,independent_variable,value);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_get_independent_variable_value.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_variable_get_independent_variable_value.  "
			"Invalid argument(s).  %p %p %p",variable,independent_variable,value);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_get_independent_variable_value */

int Cmiss_variable_get_set_independent_variable_value(
	Cmiss_variable_id variable,
	Cmiss_variable_id independent_variable,
	Cmiss_value_id value)
/*******************************************************************************
LAST MODIFIED : 10 July 2003

DESCRIPTION :
Swaps the <value> and the current value for the <independent_variable> of the
<variable>.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_get_set_independent_variable_value);
	return_code=0;
	/* check arguments */
	if (variable&&independent_variable&&value)
	{
		if (variable->get_set_independent_variable_value_type_specific_function)
		{
			return_code=
				(variable->get_set_independent_variable_value_type_specific_function)(
				variable,independent_variable,value);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_get_set_independent_variable_value.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_variable_get_set_independent_variable_value.  "
			"Invalid argument(s).  %p %p %p",variable,independent_variable,value);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_get_set_independent_variable_value */

int Cmiss_variable_depends_on_Cmiss_variable(
	Cmiss_variable_id variable,Cmiss_variable_id other_variable)
/*******************************************************************************
LAST MODIFIED : 10 July 2002

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

???DB.  Needs work
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_depends_on_Cmiss_variable);
	if (variable&&other_variable)
	{
		if (variable==other_variable)
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_variable_depends_on_Cmiss_variable.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_depends_on_Cmiss_variable */

int Cmiss_variable_depends_on_Cmiss_variable_in_list(
	Cmiss_variable_id variable,
	struct LIST(Cmiss_variable) *variable_list)
/*******************************************************************************
LAST MODIFIED : 4 February 2003

DESCRIPTION :
Returns nonzero if <variable> depends on any variable in <variable_list>.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_depends_on_Cmiss_variable_in_list);
	return_code=0;
	if (variable && variable_list)
	{
		if (FIRST_OBJECT_IN_LIST_THAT(Cmiss_variable)(
			Cmiss_variable_is_source_variable_conditional,
			(void *)variable,variable_list))
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_variable_depends_on_Cmiss_variable_in_list.  "
			"Invalid argument(s).  %p %p",variable,variable_list);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_depends_on_Cmiss_variable_in_list */

struct Cmiss_variable_value *CREATE(Cmiss_variable_value)(
	Cmiss_variable_id variable,Cmiss_value_id value)
/*******************************************************************************
LAST MODIFIED : 22 April 2003

DESCRIPTION :
Creates a <variable>/<value> pair.
==============================================================================*/
{
	struct Cmiss_variable_value *variable_value;

	ENTER(CREATE(Cmiss_variable_value));
	if (ALLOCATE(variable_value,struct Cmiss_variable_value,1))
	{
		variable_value->variable=ACCESS(Cmiss_variable)(variable);
		variable_value->value=ACCESS(Cmiss_value)(value);
		variable_value->access_count=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Cmiss_variable_value).  "
			"Insufficient memory");
	}
	LEAVE;

	return (variable_value);
} /* CREATE(Cmiss_variable_value) */

int DESTROY(Cmiss_variable_value)(
	struct Cmiss_variable_value **variable_value_address)
/*******************************************************************************
LAST MODIFIED : 22 April 2003

DESCRIPTION :
Frees memory/deaccess objects for Cmiss_variable_value at
<*variable_value_address>.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Cmiss_variable_value));
	return_code=0;
	if (variable_value_address&&(*variable_value_address))
	{
		if ((*variable_value_address)->access_count<=0)
		{
			DEACCESS(Cmiss_variable)(&((*variable_value_address)->variable));
			DEACCESS(Cmiss_value)(&((*variable_value_address)->value));
			DEALLOCATE(*variable_value_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"DESTROY(Cmiss_variable_value).  "
				"Positive access_count");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Cmiss_variable_value).  "
			"Missing variable_value");
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Cmiss_variable_value) */

DECLARE_OBJECT_FUNCTIONS(Cmiss_variable_value)

DECLARE_LIST_FUNCTIONS(Cmiss_variable_value)

Cmiss_value_id Cmiss_variable_value_get_value(
	struct Cmiss_variable_value *variable_value)
/*******************************************************************************
LAST MODIFIED : 7 February 2003

DESCRIPTION :
Returns the value for the <variable_value>.
==============================================================================*/
{
	Cmiss_value_id value;

	ENTER(Cmiss_variable_value_get_value);
	value=(Cmiss_value_id)NULL;
	/* check arguments */
	if (variable_value)
	{
		value=variable_value->value;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_value_get_value.  "
			"Missing variable_value");
	}
	LEAVE;

	return (value);
} /* Cmiss_variable_value_get_value */

Cmiss_variable_id Cmiss_variable_value_get_variable(
	struct Cmiss_variable_value *variable_value)
/*******************************************************************************
LAST MODIFIED : 7 February 2003

DESCRIPTION :
Returns the variable for the <variable_value>.
==============================================================================*/
{
	Cmiss_variable_id variable;

	ENTER(Cmiss_variable_value_get_variable);
	variable=(Cmiss_variable_id )NULL;
	/* check arguments */
	if (variable_value)
	{
		variable=variable_value->variable;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_value_get_variable.  "
			"Missing variable_value");
	}
	LEAVE;

	return (variable);
} /* Cmiss_variable_value_get_variable */

int Cmiss_variable_get_set_independent_variable_value_iterator(
	struct Cmiss_variable_value *independent_variable_value,
	void *dependent_variable_void)
/*******************************************************************************
LAST MODIFIED : 9 July 2003

DESCRIPTION :
List iterator function that swaps the <independent_variable_value> with the
current one for the <dependent_variable_void>.  Returns true whether or not the
<independent_variable_value> is for the <dependent_variable_void>.
==============================================================================*/
{
	int return_code;
	Cmiss_variable_id dependent_variable;

	ENTER(Cmiss_variable_get_set_independent_variable_value_iterator);
	return_code=0;
	dependent_variable=(Cmiss_variable_id )dependent_variable_void;
	ASSERT_IF(independent_variable_value&&dependent_variable&&
		(dependent_variable->
		get_set_independent_variable_value_type_specific_function),return_code,0)
	{
		return_code=(dependent_variable->
			get_set_independent_variable_value_type_specific_function)(
			dependent_variable,independent_variable_value->variable,
			independent_variable_value->value);
		/* not all the independent_variable_value pairs will be for the
			dependent_variable */
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_get_set_independent_variable_value_iterator */

int Cmiss_variable_is_defined(Cmiss_variable_id variable,
	struct LIST(Cmiss_variable_value) *values)
/*******************************************************************************
LAST MODIFIED : 2 July 2003

DESCRIPTION :
Returns true if <variable> can be calculated with the specified <values>
over-riding, but not setting, the current values.

???DB.  Could use evaluate, but that would be less efficient and currently
	would write error messages if it failed
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_is_defined);
	return_code=0;
	/* check arguments */
	if (variable&&values)
	{
		if (variable->is_defined_type_specific_function)
		{
			return_code=(variable->is_defined_type_specific_function)(variable,
				values);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_variable_is_defined.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_is_defined.  "
			"Invalid argument(s).  %p %p",variable,values);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_is_defined */

int Cmiss_variable_evaluate(struct Cmiss_variable_value *variable_value,
	struct LIST(Cmiss_variable_value) *values)
/*******************************************************************************
LAST MODIFIED : 10 July 2003

DESCRIPTION :
Calculates the <variable_value> with the specified <values> over-riding, but not
setting, the current values.
==============================================================================*/
{
	Cmiss_variable_id variable;
	int return_code;

	ENTER(Cmiss_variable_evaluate);
	return_code=0;
	/* checking arguments */
	if (variable_value&&(variable=variable_value->variable))
	{
		if (variable->evaluate_type_specific_function)
		{
			/* swap the specified <values> with the current values */
			if (!values||FOR_EACH_OBJECT_IN_LIST(Cmiss_variable_value)(
				Cmiss_variable_get_set_independent_variable_value_iterator,
				(void *)variable,values))
			{
				return_code=(variable->evaluate_type_specific_function)(
					variable_value);
				if (values&&!FOR_EACH_OBJECT_IN_LIST(Cmiss_variable_value)(
					Cmiss_variable_get_set_independent_variable_value_iterator,
					(void *)variable,values))
				{
					display_message(ERROR_MESSAGE,"Cmiss_variable_evaluate.  "
						"Could not reset <values>");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Cmiss_variable_evaluate.  "
					"Could not over-ride <values>");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_variable_evaluate.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_evaluate.  "
			"Missing <variable_value> to evaluate");
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_evaluate */

int Cmiss_variable_evaluate_derivative(
	Cmiss_variable_id dependent_variable,int order,
	Cmiss_variable_id *independent_variables,
	struct LIST(Cmiss_variable_value) *values,
	Cmiss_value_id derivative_matrix)
/*******************************************************************************
LAST MODIFIED : 20 July 2003

DESCRIPTION :
Evaluates the <derivative_matrix> for the <order> degree derivative of
<dependent_variable> with respect to the <independent_variables>.
==============================================================================*/
{
	Cmiss_value_id *derivative_matrices,*matrices,**sub_matrices,sub_matrix,
		value_temp;
	Cmiss_variable_id *composite_variables,*local_independent_variables;
	int column_number,composite_independent_variable_factor,
		composite_independent_variable_number,composite_independent_variable_step,
		derivative_index,j,k,matrix_number,number_of_columns,
		number_of_composite_variables,number_of_matrices,number_of_rows,
		*numbers_of_composite_values,*numbers_of_independent_values,return_code,
		total_number_of_columns;
	struct Matrix *matrix_temp;

	ENTER(Cmiss_variable_evaluate_derivative);
	return_code=0;
	/* checking arguments */
	if (dependent_variable&&(0<order)&&independent_variables&&derivative_matrix)
	{
		if (dependent_variable->evaluate_derivative_type_specific_function)
		{
			/* check for composite independent variables */
			composite_independent_variable_number=0;
			while ((composite_independent_variable_number<order)&&
				!CMISS_VARIABLE_IS_TYPE(composite)(
				independent_variables[composite_independent_variable_number]))
			{
				composite_independent_variable_number++;
			}
			if (composite_independent_variable_number==order)
			{
				/* none of the independent variables are composite */
				/* swap the specified <values> with the current values */
				if (!values||FOR_EACH_OBJECT_IN_LIST(Cmiss_variable_value)(
					Cmiss_variable_get_set_independent_variable_value_iterator,
					(void *)dependent_variable,values))
				{
					return_code=(dependent_variable->
						evaluate_derivative_type_specific_function)(
						dependent_variable,order,independent_variables,derivative_matrix);
					if (values&&!FOR_EACH_OBJECT_IN_LIST(Cmiss_variable_value)(
						Cmiss_variable_get_set_independent_variable_value_iterator,
						(void *)dependent_variable,values))
					{
						display_message(ERROR_MESSAGE,
							"Cmiss_variable_evaluate_derivative.  "
							"Could not reset <values>");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Cmiss_variable_evaluate_derivative.  "
						"Could not over-ride <values>");
				}
			}
			else
			{
				/* an independent variable is composite */
				if (Cmiss_variable_composite_get_type(
					independent_variables[composite_independent_variable_number],
					&number_of_composite_variables,&composite_variables)&&
					(0<number_of_composite_variables)&&composite_variables)
				{
					/* working storage */
					ALLOCATE(local_independent_variables,Cmiss_variable_id,order);
					ALLOCATE(derivative_matrices,Cmiss_value_id,
						number_of_composite_variables);
					ALLOCATE(sub_matrices,Cmiss_value_id *,number_of_composite_variables);
					ALLOCATE(numbers_of_independent_values,int,order);
					ALLOCATE(numbers_of_composite_values,int,
						number_of_composite_variables);
					if (value_temp=CREATE(Cmiss_value)())
					{
						ACCESS(Cmiss_value)(value_temp);
					}
					if (local_independent_variables&&derivative_matrices&&sub_matrices&&
						numbers_of_independent_values&&numbers_of_composite_values&&
						value_temp)
					{
						/* calculate the derivatives for the variables making up the
							composite */
						j=0;
						return_code=1;
						while (return_code&&(j<order))
						{
							local_independent_variables[j]=independent_variables[j];
							if (return_code=Cmiss_variable_get_value_type(
								independent_variables[j],value_temp))
							{
								return_code=Cmiss_value_get_reals(value_temp,
									numbers_of_independent_values+j,(FE_value **)NULL);
							}
							j++;
						}
						j=0;
						while (return_code&&(j<number_of_composite_variables))
						{
							local_independent_variables[
								composite_independent_variable_number]=composite_variables[j];
							if ((return_code=Cmiss_variable_get_value_type(
								composite_variables[j],value_temp))&&
								(return_code=Cmiss_value_get_reals(value_temp,
								numbers_of_composite_values+j,(FE_value **)NULL)))
							{
								if (derivative_matrices[j]=CREATE(Cmiss_value)())
								{
									ACCESS(Cmiss_value)(derivative_matrices[j]);
									if (return_code=Cmiss_variable_evaluate_derivative(
										dependent_variable,order,local_independent_variables,values,
										derivative_matrices[j]))
									{
										return_code=Cmiss_value_derivative_matrix_get_type(
											derivative_matrices[j],(Cmiss_variable_id *)NULL,
											(int *)NULL,(Cmiss_variable_id **)NULL,sub_matrices+j);
									}
									j++;
								}
							}
						}
						if (return_code)
						{
							/* assemble the derivative matrix */
							number_of_matrices=1;
							for (j=0;j<order;j++)
							{
								number_of_matrices *= 2;
							}
							number_of_matrices -= 1;
							ALLOCATE(matrices,Cmiss_value_id,number_of_matrices);
							if (matrices)
							{
								composite_independent_variable_factor=1;
								for (j=0;j<composite_independent_variable_number;j++)
								{
									composite_independent_variable_factor *= 2;
								}
								matrix_number=0;
								while (return_code&&(matrix_number<number_of_matrices))
								{
									if (((matrix_number+1)/composite_independent_variable_factor)%
										2)
									{
										/* derivative involves composite independent variable */
										/* calculate the size of the assembled matrix */
										return_code=Cmiss_value_matrix_get_dimensions(
											sub_matrices[0][matrix_number],&number_of_rows,
											&total_number_of_columns);
										j=1;
										while (return_code&&(j<number_of_composite_variables))
										{
											if (return_code=Cmiss_value_matrix_get_dimensions(
												sub_matrices[j][matrix_number],(int *)NULL,
												&number_of_columns))
											{
												total_number_of_columns += number_of_columns;
											}
											j++;
										}
										if (return_code&&(matrix_temp=CREATE(Matrix)("matrix",DENSE,
											number_of_rows,total_number_of_columns))&&
											(matrices[matrix_number]=CREATE(Cmiss_value)())&&
											Cmiss_value_matrix_set_type(matrices[matrix_number],
											matrix_temp))
										{
											ACCESS(Cmiss_value)(matrices[matrix_number]);
											derivative_index=matrix_number+1;
											composite_independent_variable_step=1;
											j=0;
											while (j<composite_independent_variable_number)
											{
												if (derivative_index%2)
												{
													composite_independent_variable_step *=
														numbers_of_independent_values[j];
												}
												j++;
												derivative_index /= 2;
											}
											column_number=0;
											k=0;
											while (return_code&&
												(column_number<total_number_of_columns))
											{
												j=0;
												while (return_code&&(j<number_of_composite_variables))
												{
													number_of_columns=composite_independent_variable_step*
														numbers_of_composite_values[j];
													if (sub_matrix=Cmiss_value_matrix_get_submatrix(
														sub_matrices[j][matrix_number],1,number_of_rows,
														number_of_columns*k+1,number_of_columns*(k+1)))
													{
														ACCESS(Cmiss_value)(sub_matrix);
														if (return_code=Cmiss_value_matrix_set_submatrix(
															matrices[matrix_number],1,number_of_rows,
															column_number+1,column_number+number_of_columns,
															sub_matrix))
														{
															column_number += number_of_columns;
														}
														DEACCESS(Cmiss_value)(&sub_matrix);
													}
													else
													{
														return_code=0;
													}
													j++;
												}
												k++;
											}
											matrix_number++;
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"Cmiss_variable_evaluate_derivative.  "
												"Could not set up matrices[%d]. %d %d %p %p",
												matrix_number,return_code,j,matrix_temp,
												matrices[matrix_number]);
											if (return_code)
											{
												if (matrix_temp)
												{
													if (matrices[matrix_number])
													{
														DESTROY(Cmiss_value)(matrices+matrix_number);
													}
													DESTROY(Matrix)(&matrix_temp);
												}
												return_code=0;
											}
										}
									}
									else
									{
										/* derivative does not involve composite independent
											variable */
										if (matrices[matrix_number]=CREATE(Cmiss_value)())
										{
											ACCESS(Cmiss_value)(matrices[matrix_number]);
											return_code=Cmiss_value_copy(matrices[matrix_number],
												sub_matrices[0][matrix_number]);
											matrix_number++;
										}
										else
										{
											return_code=0;
										}
									}
								}
								if (return_code)
								{
									/* duplicate independent variables */
									if (ALLOCATE(local_independent_variables,Cmiss_variable_id,
										order))
									{
										for (j=0;j<order;j++)
										{
											local_independent_variables[j]=independent_variables[j];
										}
										if (return_code=Cmiss_value_derivative_matrix_set_type(
											derivative_matrix,dependent_variable,order,
											local_independent_variables,matrices))
										{
											/* matrices and local_independent_variables are now
												managed by derivative_matrix */
											local_independent_variables=(Cmiss_variable_id *)NULL;
											matrices=(Cmiss_value_id *)NULL;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"Cmiss_variable_evaluate_derivative.  "
											"Could not allocate local_independent_variables");
										return_code=0;
									}
									if (matrices)
									{
										for (j=0;j<number_of_matrices;j++)
										{
											DEACCESS(Cmiss_value)(matrices+j);
										}
									}
								}
								else
								{
									while (matrix_number>0)
									{
										matrix_number--;
										DEACCESS(Cmiss_value)(matrices+matrix_number);
									}
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Cmiss_variable_evaluate_derivative.  "
									"Could not allocate matrices (%p)",
									matrices);
								return_code=0;
							}
							if (matrices)
							{
								DEALLOCATE(matrices);
							}
							for (j=0;j<number_of_composite_variables;j++)
							{
								DEACCESS(Cmiss_value)(derivative_matrices+j);
							}
						}
						else
						{
							while (j>0)
							{
								j--;
								DEACCESS(Cmiss_value)(derivative_matrices+j);
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Cmiss_variable_evaluate_derivative.  "
							"Could not allocate working storage.  %p %p %p %p %p %p",
							local_independent_variables,derivative_matrices,sub_matrices,
							numbers_of_independent_values,numbers_of_composite_values,
							value_temp);
					}
					if (value_temp)
					{
						DEACCESS(Cmiss_value)(&value_temp);
					}
					if (numbers_of_composite_values)
					{
						DEALLOCATE(numbers_of_composite_values);
					}
					if (numbers_of_independent_values)
					{
						DEALLOCATE(numbers_of_independent_values);
					}
					if (sub_matrices)
					{
						DEALLOCATE(sub_matrices);
					}
					if (derivative_matrices)
					{
						DEALLOCATE(derivative_matrices);
					}
					if (local_independent_variables)
					{
						DEALLOCATE(local_independent_variables);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Cmiss_variable_evaluate_derivative.  "
						"Could not get composite_variables");
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_variable_evaluate_derivative.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_evaluate_derivative.  "
			"Invalid argument(s).  %p %d %p %p",dependent_variable,order,
			independent_variables,derivative_matrix);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_evaluate_derivative */

struct Cmiss_variable_package *CREATE(Cmiss_variable_package)(void)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Creates a Cmiss_variable_package which is used for sharing of
Cmiss_variables with the same name.

???DB.  What about FE_ managers?  Do FE computed variables need there own
	package (part of set_type_ ?)
==============================================================================*/
{
	struct Cmiss_variable_package *package;

	ENTER(CREATE(Cmiss_variable_package));
	package=(struct Cmiss_variable_package *)NULL;
	if (ALLOCATE(package,struct Cmiss_variable_package,1))
	{
		if (!(package->variable_manager=CREATE_MANAGER(Cmiss_variable)()))
		{
			display_message(ERROR_MESSAGE,"CREATE(Cmiss_variable_package).  "
				"Could not create variable_manager");
			DEALLOCATE(package);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Cmiss_variable_package).  "
			"Could not allocate package");
	}
	LEAVE;

	return (package);
} /* CREATE(Cmiss_variable_package) */

int DESTROY(Cmiss_variable_package)(
	struct Cmiss_variable_package **package_address)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Frees memory/deaccess objects in Cmiss_variable_package at <*package_address>.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Cmiss_variable_package));
	return_code=0;
	if (package_address)
	{
		if (*package_address)
		{
			DESTROY_MANAGER(Cmiss_variable)(&((*package_address)->variable_manager));
			DEALLOCATE(*package_address);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Cmiss_variable_package).  "
			"Could not allocate package");
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Cmiss_variable_package) */
