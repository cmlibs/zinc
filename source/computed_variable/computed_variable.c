/*******************************************************************************
FILE : computed_variable.c

LAST MODIFIED : 4 February 2003

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
???DB.  Do without components?
???DB.  Need Computed_variable_changed?
==============================================================================*/
#include "computed_variable/computed_variable.h"
#include "computed_variable/computed_variable_private.h"
#include "general/indexed_list_private.h"
#include "general/debug.h"
#include "general/list_private.h"
#include "general/manager_private.h"
#include "general/mystring.h"
#include "user_interface/message.h"

/*
Module types
------------
*/
struct Computed_variable
/*******************************************************************************
LAST MODIFIED : 4 February 2003

DESCRIPTION :
==============================================================================*/
{
	/* the name/identifier of the Computed_variable */
	char *name;
	/* the type string identifies the variable's type.  It points to a string that
		is shared by all variables of the same type and should not be copied */
	char *type_string;
	/* information that is specific to the type */
	void *type_specific_data;
	/* methods.   To add a new method, add
		- a field here in struct Computed_variable
		- a function type declaration in computed_variable_private.h
		- an argument and an assigment to Computed_variable_establish_methods
		The compiler will force the other changes because
		Computed_variable_establish_methods is used in
		- Computed_variable_clear_type
		- CREATE(Computed_variable)
		- MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_variable,name) */
	Computed_variable_clear_type_specific_function
		computed_variable_clear_type_specific_function;
	Computed_variable_copy_type_specific_function
		computed_variable_copy_type_specific_function;
	Computed_variable_get_independent_variable_value_type_specific_function
		computed_variable_get_independent_variable_value_type_specific_function;
	Computed_variable_get_value_type_type_specific_function
		computed_variable_get_value_type_type_specific_function;
	Computed_variable_is_defined_type_specific_function
		computed_variable_is_defined_type_specific_function;
	Computed_variable_is_independent_variable_of_type_specific_function
		computed_variable_is_independent_variable_of_type_specific_function;
	Computed_variable_not_in_use_function
		computed_variable_not_in_use_function;
	Computed_variable_overlap_type_specific_function
		computed_variable_overlap_type_specific_function;
	Computed_variable_same_variable_type_specific_function
		computed_variable_same_variable_type_specific_function;
	Computed_variable_set_independent_variable_value_type_specific_function
		computed_variable_set_independent_variable_value_type_specific_function;
	/* source variables for independent variables */
		/*???DB.  Have not done set up.  Starting with
			Computed_variable_set_source_variable */
	int number_of_source_variables;
	struct Computed_variable **independent_variables;
	struct Computed_variable **source_variables;

	int access_count;

#if defined (OLD_CODE)
	/* the name/identifier of the Computed_variable */
	char *name;
	int number_of_components;
	/* This is set for variables where the components have names other than
		the defaults of 1,2...number_of_components */
	char **component_names;

	/* if the following flag is set, the variable may not be modified or destroyed
		by the user. See Computed_variable_set_read_only function */
	int read_only;
	struct Coordinate_system coordinate_system;

	/* Value cache: */
	/* For all Computed_variable_types: computed values/derivatives.
		When the variable is computed its values and derivatives are first placed
		in these arrays. If the variable is then recomputed at element:xi, the values
		are returned immediately. The <values> array is allocated when the variable is
		evaluated and deallocated when the number_of_components is [re]established
		or the variable is copied over. The values array is made large enough to store
		the values of the variable while the derivatives fit those of the variable in an
		element of dimension MAXIMUM_ELEMENT_XI_DIMENSIONS. */
	/* ???RC note: separation of cache and variable probably necessary if computed
		variables are to be efficient under multiprocessing */
	FE_value *values,*derivatives,xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	/* flag saying whether derivatives were calculated */
	int derivatives_valid;
	/* Only one of element/node should be accessed at a time - if there is an
		element accessed, then values are guaranteed to be valid and derivatives
		are valid if the derivatives_valid flag is set - both at the above xi only.
		If node is accessed, then the values are valid at that node. Either modes
		of caching must be cleared by a call to Computed_variable_clear_cache once
		the variable is no longer of immediate use. */
	/* last element in which values/derivatives calculated - ACCESSed by variable */
	struct FE_element *element;
	/* last node at which values calculated - ACCESSed by variable */
	struct FE_node *node;
	/* last time at which values were calculated */
	FE_value time;

	/* cache used when doing find_xi calculations */
	struct Computed_variable_find_element_xi_cache *find_element_xi_cache;

	Computed_variable_clear_type_specific_function
		computed_variable_clear_type_specific_function;
	Computed_variable_copy_type_specific_function
		computed_variable_copy_type_specific_function;
	Computed_variable_clear_cache_type_specific_function
		computed_variable_clear_cache_type_specific_function;
	Computed_variable_type_specific_contents_match_function
		computed_variable_type_specific_contents_match_function;
	Computed_variable_is_defined_in_element_function
		computed_variable_is_defined_in_element_function;
	Computed_variable_is_defined_at_node_function
		computed_variable_is_defined_at_node_function;
	Computed_variable_has_numerical_components_function
		computed_variable_has_numerical_components_function;
	Computed_variable_not_in_use_function
		computed_variable_not_in_use_function;
	Computed_variable_evaluate_cache_at_node_function
		computed_variable_evaluate_cache_at_node_function;
	Computed_variable_evaluate_cache_in_element_function
		computed_variable_evaluate_cache_in_element_function;
	Computed_variable_evaluate_as_string_at_node_function
		computed_variable_evaluate_as_string_at_node_function;
	Computed_variable_evaluate_as_string_in_element_function
		computed_variable_evaluate_as_string_in_element_function;
	Computed_variable_set_values_at_node_function
		computed_variable_set_values_at_node_function;
	Computed_variable_set_values_in_element_function
		computed_variable_set_values_in_element_function;
	Computed_variable_get_native_discretization_in_element_function
		computed_variable_get_native_discretization_in_element_function;
	Computed_variable_find_element_xi_function
		computed_variable_find_element_xi_function;
	List_Computed_variable_function list_Computed_variable_function;
	Computed_variable_get_command_string_function
		computed_variable_get_command_string_function;
	Computed_variable_has_multiple_times_function
		computed_variable_has_multiple_times_function;

	void *type_specific_data;
	/* The type string identifies the type, it should not be copied but
		point to the common type string for that type */
	char *type_string;

	/* for all Computed_variable_types calculated from others */

	/* array of computed variables this variable is calculated from */
	int number_of_source_variables;
	struct Computed_variable **source_variables;
	/* array of constant values this variable is calculated from */
	int number_of_source_values;
	FE_value *source_values;

	int access_count;
#endif /* defined (OLD_CODE) */
}; /* struct Computed_variable */

FULL_DECLARE_INDEXED_LIST_TYPE(Computed_variable);

FULL_DECLARE_MANAGER_TYPE(Computed_variable);

struct Computed_variable_value
/*******************************************************************************
LAST MODIFIED : 27 January 2003

DESCRIPTION :
A variable/value pair for specifying the value of the variable.
==============================================================================*/
{
	struct Computed_variable *variable;
	struct Computed_value *value;
	int access_count;
}; /* struct Computed_variable_value */

FULL_DECLARE_LIST_TYPE(Computed_variable_value);

/*
Module functions
----------------
*/
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Computed_variable,name,char *,strcmp)

DECLARE_LOCAL_MANAGER_FUNCTIONS(Computed_variable)

static int Computed_variable_is_source_variable_conditional(
	struct Computed_variable *variable,void *other_variable_void)
/*******************************************************************************
LAST MODIFIED : 4 February 2003

DESCRIPTION :
List conditional function returning true if <variable> is a source variable of
<other_variable>, ie. <other_variable> depends on <variable>.

NB.  Because used for a list of source variable is not
Computed_variable_depends_on_Computed_variable_conditional (other way round).
==============================================================================*/
{
	int return_code;

	ENTER(Computed_variable_is_source_variable_conditional);
	return_code=Computed_variable_depends_on_Computed_variable(
		(struct Computed_variable *)other_variable_void,variable);
	LEAVE;

	return (return_code);
} /* Computed_variable_is_source_variable_conditional */

/*
Friend functions
----------------
*/
int Computed_variable_establish_methods(struct Computed_variable *variable,
	Computed_variable_clear_type_specific_function
	computed_variable_clear_type_specific_function,
	Computed_variable_copy_type_specific_function
	computed_variable_copy_type_specific_function,
	Computed_variable_get_independent_variable_value_type_specific_function
	computed_variable_get_independent_variable_value_type_specific_function,
	Computed_variable_get_value_type_type_specific_function
	computed_variable_get_value_type_type_specific_function,
	Computed_variable_is_defined_type_specific_function
	computed_variable_is_defined_type_specific_function,
	Computed_variable_is_independent_variable_of_type_specific_function
	computed_variable_is_independent_variable_of_type_specific_function,
	Computed_variable_not_in_use_function
	computed_variable_not_in_use_function,
	Computed_variable_overlap_type_specific_function
	computed_variable_overlap_type_specific_function,
	Computed_variable_same_variable_type_specific_function
	computed_variable_same_variable_type_specific_function,
	Computed_variable_set_independent_variable_value_type_specific_function
	computed_variable_set_independent_variable_value_type_specific_function)
/*******************************************************************************
LAST MODIFIED : 4 February 2003

DESCRIPTION :
Sets the methods for the <variable>.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_variable_establish_methods);
	return_code=0;
	if (variable)
	{
		variable->computed_variable_clear_type_specific_function=
			computed_variable_clear_type_specific_function;
		variable->computed_variable_copy_type_specific_function=
			computed_variable_copy_type_specific_function;
		variable->
			computed_variable_get_independent_variable_value_type_specific_function=
			computed_variable_get_independent_variable_value_type_specific_function;
		variable->computed_variable_get_value_type_type_specific_function=
			computed_variable_get_value_type_type_specific_function;
		variable->computed_variable_is_defined_type_specific_function=
			computed_variable_is_defined_type_specific_function;
		variable->
			computed_variable_is_independent_variable_of_type_specific_function=
			computed_variable_is_independent_variable_of_type_specific_function;
		variable->computed_variable_not_in_use_function=
			computed_variable_not_in_use_function;
		variable->computed_variable_overlap_type_specific_function=
			computed_variable_overlap_type_specific_function;
		variable->computed_variable_same_variable_type_specific_function=
			computed_variable_same_variable_type_specific_function;
		variable->
			computed_variable_set_independent_variable_value_type_specific_function=
			computed_variable_set_independent_variable_value_type_specific_function;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_variable_establish_methods.  "
			"Missing variable");
	}
	LEAVE;

	return (return_code);
} /* Computed_variable_establish_methods */

int Computed_variable_clear_type(struct Computed_variable *variable)
/*******************************************************************************
LAST MODIFIED : 4 February 2003

DESCRIPTION :
Used internally by DESTROY and Computed_variable_set_type_*() functions to
deallocate or deaccess data specific to any Computed_variable_type.  Functions
changing the type of the Computed_variable should
- allocate any dynamic data needed for the type
- call this function to clear what is currently in the variable
- then set values
to ensure that the variable is not left in an invalid state.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_variable_clear_type);
	return_code=0;
	/* check arguments */
	if (variable)
	{
		if (variable->type_specific_data)
		{
			if (variable->computed_variable_clear_type_specific_function)
			{
				return_code=
					(variable->computed_variable_clear_type_specific_function)(variable);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_variable_clear_type.  "
					"Type specific data but no function to clear it");
				return_code=0;
			}
			variable->type_specific_data=(void *)NULL;
		}
		/* clear all methods */
		Computed_variable_establish_methods(variable,
			(Computed_variable_clear_type_specific_function)NULL,
			(Computed_variable_copy_type_specific_function)NULL,
			(Computed_variable_get_independent_variable_value_type_specific_function)
			NULL,
			(Computed_variable_get_value_type_type_specific_function)NULL,
			(Computed_variable_is_defined_type_specific_function)NULL,
			(Computed_variable_is_independent_variable_of_type_specific_function)NULL,
			(Computed_variable_not_in_use_function)NULL,
			(Computed_variable_overlap_type_specific_function)NULL,
			(Computed_variable_same_variable_type_specific_function)NULL,
			(Computed_variable_set_independent_variable_value_type_specific_function)
			NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_variable_clear_type.  "
			"Missing variable");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_variable_clear_type */

/*
Global functions
----------------
*/
int Computed_value_set_type_jacobian_matrix(struct Computed_value *value,
	struct Matrix *matrix,struct LIST(Computed_variable) *row_variables,
	struct LIST(Computed_variable) *column_variables);
/*******************************************************************************
LAST MODIFIED : 24 January 2003

DESCRIPTION :
Makes <value> of type jacobian_matrix and sets its <matrix>, <row_variables>
and <column_variables>.  After success, the <value> is responsible for
DESTROYing <matrix>, <row_variables> and <column_variables>.
==============================================================================*/

int Computed_value_is_type_jacobian_matrix(struct Computed_value *value);
/*******************************************************************************
LAST MODIFIED : 24 January 2003

DESCRIPTION :
Returns a non-zero if <value> is a jacobian_matrix and zero otherwise.
==============================================================================*/

int Computed_value_get_type_jacobian_matrix(struct Computed_value *value,
	struct Matrix **matrix_address,
	struct LIST(Computed_variable) **row_variables_address,
	struct LIST(Computed_variable) **column_variables_address);
/*******************************************************************************
LAST MODIFIED : 24 January 2003

DESCRIPTION :
If <value> is of type jacobian_matrix, gets its <*matrix_address>,
<*row_variables_address> and <*column_variables_address>.

The calling program must not DEALLOCATE the returned structures.
==============================================================================*/

struct Computed_variable *CREATE(Computed_variable)(char *name)
/*******************************************************************************
LAST MODIFIED : 4 February 2003

DESCRIPTION :
Creates an empty variable with specified <name> and no type, dependent variables
or independent variables.  Each type of variable has its own "set_type"
function.  The <name> is copied.
==============================================================================*/
{
	struct Computed_variable *variable;

	ENTER(CREATE(Computed_variable));
	if (name)
	{
		variable=(struct Computed_variable *)NULL;
		if (ALLOCATE(variable,struct Computed_variable,1)&&
			(variable->name=duplicate_string(name)))
		{
			/* initialise data */
			variable->type_string=(char *)NULL;
			variable->type_specific_data=NULL;
			/* initialise methods */
			Computed_variable_establish_methods(variable,
				(Computed_variable_clear_type_specific_function)NULL,
				(Computed_variable_copy_type_specific_function)NULL,
				(Computed_variable_get_independent_variable_value_type_specific_function)NULL,
				(Computed_variable_get_value_type_type_specific_function)NULL,
				(Computed_variable_is_defined_type_specific_function)NULL,
				(Computed_variable_is_independent_variable_of_type_specific_function)
				NULL,
				(Computed_variable_not_in_use_function)NULL,
				(Computed_variable_overlap_type_specific_function)NULL,
				(Computed_variable_same_variable_type_specific_function)NULL,
				(Computed_variable_set_independent_variable_value_type_specific_function)NULL);
			/* initialize source variables */
			variable->number_of_source_variables=0;
			variable->independent_variables=(struct Computed_variable **)NULL;
			variable->source_variables=(struct Computed_variable **)NULL;
			/* initialise access_count */
			variable->access_count=0;
		}
		else
		{
			display_message(ERROR_MESSAGE,"CREATE(Computed_variable).  "
				"Insufficient memory");
			DEALLOCATE(variable);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Computed_variable).  "
			"Missing name");
		variable=(struct Computed_variable *)NULL;
	}
	LEAVE;

	return (variable);
} /* CREATE(Computed_variable) */

int DESTROY(Computed_variable)(struct Computed_variable **variable_address)
/*******************************************************************************
LAST MODIFIED : 3 February 2003

DESCRIPTION :
Frees memory/deaccess objects for Computed_variable at <*variable_address>.
==============================================================================*/
{
	int i,return_code;
	struct Computed_variable **independent_variable_address,
		**source_variable_address,*variable;

	ENTER(DESTROY(Computed_variable));
	return_code=0;
	if (variable_address&&(variable= *variable_address))
	{
		if (0>=variable->access_count)
		{
			DEALLOCATE(variable->name);
			Computed_variable_clear_type(variable);
			i=variable->number_of_source_variables;
			independent_variable_address=variable->independent_variables;
			source_variable_address=variable->source_variables;
			while (i>0)
			{
				DEACCESS(Computed_variable)(independent_variable_address);
				DEACCESS(Computed_variable)(source_variable_address);
				independent_variable_address++;
				source_variable_address++;
				i--;
			}
			DEALLOCATE(variable->independent_variables);
			DEALLOCATE(variable->source_variables);
			DEALLOCATE(*variable_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Computed_variable).  Positive access_count");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Computed_variable).  Missing variable");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Computed_variable) */

DECLARE_OBJECT_FUNCTIONS(Computed_variable)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Computed_variable)

DECLARE_INDEXED_LIST_FUNCTIONS(Computed_variable)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Computed_variable,name,
	char *,strcmp)

DECLARE_INDEXED_LIST_IDENTIFIER_CHANGE_FUNCTIONS(Computed_variable,name)

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(Computed_variable,name)
{
	char *name;
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(Computed_variable,name));
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
					"MANAGER_COPY_WITH_IDENTIFIER(Computed_variable,name).  "
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
			if (return_code=MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_variable,name)(
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
					"MANAGER_COPY_WITH_IDENTIFIER(Computed_variable,name).  "
					"Could not copy without identifier");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITH_IDENTIFIER(Computed_variable,name).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(Computed_variable,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Computed_variable,name)
/*******************************************************************************
LAST MODIFIED : 4 February 2003

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
	int i,return_code;
	struct Computed_variable **independent_variable_address,
		**source_variable_address;
	void *type_specific_data;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_variable,name));
	if (source&&destination)
	{
		/* check <source> does not depend on <destination> else infinite loop */
		if (Computed_variable_depends_on_Computed_variable(source,destination))
		{
			display_message(ERROR_MESSAGE,
				"MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_variable,name).  "
				"Cannot make variable depend on itself");
			return_code=0;
		}
		else
		{
			type_specific_data=NULL;
			return_code=1;
			if (source->type_specific_data)
			{
				if (source->computed_variable_copy_type_specific_function)
				{
					if (!(type_specific_data=
						(source->computed_variable_copy_type_specific_function)(source,
						destination)))
					{
						display_message(ERROR_MESSAGE,
							"MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_variable,name).  "
							"Type specific copy function failed.");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_variable,name).  "
						"Type specific data but no copy function.");
					return_code=0;
				}
			}
			if (return_code)
			{
				/* free current type-specific data */
				Computed_variable_clear_type(destination);
				/* deaccess current source variables */
				i=destination->number_of_source_variables;
				independent_variable_address=destination->independent_variables;
				source_variable_address=destination->source_variables;
				while (i>0)
				{
					DEACCESS(Computed_variable)(independent_variable_address);
					DEACCESS(Computed_variable)(source_variable_address);
					independent_variable_address++;
					source_variable_address++;
					i--;
				}
				if (0<source->number_of_source_variables)
				{
					if (REALLOCATE(independent_variable_address,
						destination->independent_variables,struct Computed_variable *,
						source->number_of_source_variables))
					{
						destination->independent_variables=independent_variable_address;
						if (REALLOCATE(source_variable_address,
							destination->source_variables,struct Computed_variable *,
							source->number_of_source_variables))
						{
							destination->source_variables=source_variable_address;
						}
						else
						{
							return_code=0;
						}
					}
					else
					{
						return_code=0;
					}
				}
				if (return_code)
				{
					/* establish the new type */
					destination->type_string=source->type_string;
					destination->type_specific_data=type_specific_data;
					for (i=source->number_of_source_variables-1;i>0;i--)
					{
						(destination->source_variables)[i]=
							ACCESS(Computed_variable)((source->source_variables)[i]);
						(destination->independent_variables)[i]=
							ACCESS(Computed_variable)((source->independent_variables)[i]);
					}
					return_code=Computed_variable_establish_methods(destination,
						source->computed_variable_clear_type_specific_function,
						source->computed_variable_copy_type_specific_function,
						source->
						computed_variable_get_independent_variable_value_type_specific_function,
						source->computed_variable_get_value_type_type_specific_function,
						source->computed_variable_is_defined_type_specific_function,
						source->
						computed_variable_is_independent_variable_of_type_specific_function,
						source->computed_variable_not_in_use_function,
						source->computed_variable_overlap_type_specific_function,
						source->computed_variable_same_variable_type_specific_function,
						source->
						computed_variable_set_independent_variable_value_type_specific_function);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_variable,name).  "
						"Could not REALLOCATE source variable storage");
					return_code=0;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_variable,name).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_variable,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Computed_variable,name,char *)
{
	char *destination_name;
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(Computed_variable,name));
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
					"MANAGER_COPY_IDENTIFIER(Computed_variable,name).  "
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
			"MANAGER_COPY_IDENTIFIER(Computed_variable,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(Computed_variable,name) */

DECLARE_MANAGER_FUNCTIONS(Computed_variable)

PROTOTYPE_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Computed_variable)
/*******************************************************************************
LAST MODIFIED : 27 January 2003

DESCRIPTION :
Computed_variable requires a special version of this function because it may
use objects (eg FE_fields) that are also used.  If the computed variable is not
itself in use, it calls the variable's optional computed_variable_not_in_use
function and bases its result on that.
==============================================================================*/
{
	int return_code;

	ENTER(MANAGED_OBJECT_NOT_IN_USE(Computed_variable));
	return_code=0;
	if (manager&&object)
	{
		if (!(manager->locked))
		{
			if (IS_OBJECT_IN_LIST(Computed_variable)(object, manager->object_list))
			{
				if ((1==object->access_count)||
					((2==object->access_count)&&
					IS_OBJECT_IN_LIST(Computed_variable)(object,
					manager->message->changed_object_list)))
				{
					if (object->computed_variable_not_in_use_function)
					{
						return_code=(object->computed_variable_not_in_use_function)(object);
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
					"MANAGED_OBJECT_NOT_IN_USE(Computed_variable).  "
					"Object is not managed");
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"MANAGED_OBJECT_NOT_IN_USE(Computed_variable).  Manager is locked");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGED_OBJECT_NOT_IN_USE(Computed_variable).  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* MANAGED_OBJECT_NOT_IN_USE(Computed_variable) */

DECLARE_ADD_OBJECT_TO_MANAGER_FUNCTION(Computed_variable,name)

DECLARE_MANAGER_MODIFY_FUNCTION(Computed_variable,name)
	/*???DB.  Don't need special version because don't have components */
DECLARE_MANAGER_MODIFY_NOT_IDENTIFIER_FUNCTION(Computed_variable,name)
	/*???DB.  Don't need special version because don't have components */

DECLARE_MANAGER_MODIFY_IDENTIFIER_FUNCTION(Computed_variable,name,char *)
DECLARE_FIND_BY_IDENTIFIER_IN_MANAGER_FUNCTION(Computed_variable,name,char *)

int Computed_variable_same_variable(struct Computed_variable *variable_1,
	struct Computed_variable *variable_2)
/*******************************************************************************
LAST MODIFIED : 31 January 2003

DESCRIPTION :
Returns nonzero if <variable_1> and <variable_2> are the same variable (eg. the
value at node 10 for the finite element field bob) and zero otherwise.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_variable_same_variable);
	return_code=0;
	if (variable_1&&variable_2)
	{
		if (variable_1->computed_variable_same_variable_type_specific_function)
		{
			return_code=
				(variable_1->computed_variable_same_variable_type_specific_function)(
				variable_1,variable_2);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Computed_variable_same_variable.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_variable_same_variable.  "
			"Invalid argument(s).  %p %p",variable_1,variable_2);
	}
	LEAVE;

	return (return_code);
} /* Computed_variable_same_variable */

int Computed_variable_overlap(struct Computed_variable *variable_1,
	struct Computed_variable *variable_2)
/*******************************************************************************
LAST MODIFIED : 31 January 2003

DESCRIPTION :
Returns nonzero if <variable_1> and <variable_2> overlap (eg d/ds1 and all
values) and zero otherwise.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_variable_overlap);
	return_code=0;
	if (variable_1&&variable_2)
	{
		if (variable_1->computed_variable_overlap_type_specific_function)
		{
			return_code=
				(variable_1->computed_variable_overlap_type_specific_function)(
				variable_1,variable_2);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Computed_variable_overlap.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_variable_overlap.  "
			"Invalid argument(s).  %p %p",variable_1,variable_2);
	}
	LEAVE;

	return (return_code);
} /* Computed_variable_overlap */

int Computed_variable_is_independent_variable_of(
	struct Computed_variable *dependent_variable,
	struct Computed_variable *independent_variable)
/*******************************************************************************
LAST MODIFIED : 31 January 2003

DESCRIPTION :
Returns nonzero if <independent_variable> is an independent variable of
<dependent_variable> and zero otherwise.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_variable_is_independent_variable_of);
	return_code=0;
	if (dependent_variable&&independent_variable)
	{
		if (dependent_variable->
			computed_variable_is_independent_variable_of_type_specific_function)
		{
			return_code=(dependent_variable->
				computed_variable_is_independent_variable_of_type_specific_function)(
				dependent_variable,independent_variable);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_variable_is_independent_variable_of.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_variable_is_independent_variable_of.  "
			"Invalid argument(s).  %p %p",dependent_variable,independent_variable);
	}
	LEAVE;

	return (return_code);
} /* Computed_variable_is_independent_variable_of */

char *Computed_variable_get_type(struct Computed_variable *variable)
/*******************************************************************************
LAST MODIFIED : 27 January 2003

DESCRIPTION :
Returns the string which identifies the type.  The calling function must not
DEALLOCATE the returned string.
==============================================================================*/
{
	char *return_string;

	ENTER(Computed_variable_get_type);
	return_string=(char *)NULL;
	if (variable)
	{
		return_string=variable->type_string;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_variable_get_type.  "
			"Missing variable");
	}
	LEAVE;

	return (return_string);
} /* Computed_variable_get_type */

int Computed_variable_get_value_type(struct Computed_variable *variable,
	struct Computed_value *type)
/*******************************************************************************
LAST MODIFIED : 31 January 2003

DESCRIPTION :
Sets the <type> to be the same as the type of the results of <variable>, but
does not set/calculate the actual value.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_variable_get_value_type);
	return_code=0;
	if (variable&&type)
	{
		if (variable->computed_variable_get_value_type_type_specific_function)
		{
			return_code=
				(variable->computed_variable_get_value_type_type_specific_function)(
				variable,type);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Computed_variable_get_value_type.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_variable_get_value_type.  "
			"Invalid argument(s).  %p %p",variable,type);
	}
	LEAVE;

	return (return_code);
} /* Computed_variable_get_value_type */

int Computed_variable_same_value_type(struct Computed_variable *variable_1,
	struct Computed_variable *variable_2)
/*******************************************************************************
LAST MODIFIED : 31 January 2003

DESCRIPTION :
Returns nonzero if <variable_1> and <variable_2> have the same value type and
zero otherwise.
==============================================================================*/
{
	int return_code;
	struct Computed_value *value_1,*value_2;

	ENTER(Computed_variable_same_value_type);
	return_code=0;
	if (variable_1&&variable_2)
	{
		value_1=CREATE(Computed_value)();
		value_2=CREATE(Computed_value)();
		if (value_1&&value_2)
		{
			if (Computed_variable_get_value_type(variable_1,value_1)&&
				Computed_variable_get_value_type(variable_2,value_2))
			{
				return_code=Computed_value_same_type(value_1,value_2);
			}
			else
			{
				display_message(ERROR_MESSAGE,"Computed_variable_same_value_type.  "
					"Could not get value types");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Computed_variable_same_value_type.  "
				"Could not create value types.  %p %p",value_1,value_2);
		}
		DESTROY(Computed_value)(&value_1);
		DESTROY(Computed_value)(&value_2);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_variable_same_value_type.  "
			"Invalid argument(s).  %p %p",variable_1,variable_2);
	}
	LEAVE;

	return (return_code);
} /* Computed_variable_same_value_type */

int Computed_variable_set_independent_variable_source_variable(
	struct Computed_variable *variable,
	struct Computed_variable *independent_variable,
	struct Computed_variable *source_variable)
/*******************************************************************************
LAST MODIFIED : 3 February 2003

DESCRIPTION :
Sets the values for <independent_variable> of the <variable> to come from the
<source_variable>.  The types of the values for <independent_variable> and
<source_variable> must match.  If <source_variable> is NULL then a previous
setting of the source variable for <independent_variable> is cleared.
==============================================================================*/
{
	int i,j,overlapping_variables,return_code;
	struct Computed_variable **independent_variable_address,
		**source_variable_address;

	ENTER(Computed_variable_set_independent_variable_source_variable);
	return_code=0;
	if (variable&&independent_variable)
	{
		/* check that results of <independent_variable> and <source_variable>
			match */
		if (!source_variable||Computed_variable_same_value_type(
			independent_variable,source_variable))
		{
			/* pass on to source variables */
			return_code=1;
			i=variable->number_of_source_variables;
			j=0;
			overlapping_variables=0;
			independent_variable_address=variable->independent_variables;
			source_variable_address=variable->source_variables;
			while (return_code&&(i>0))
			{
				if (j>0)
				{
					*(independent_variable_address-j)= *independent_variable_address;
					*(source_variable_address-j)= *source_variable_address;
				}
				if (Computed_variable_same_variable(*independent_variable_address,
					independent_variable))
				{
					DEACCESS(Computed_variable)(independent_variable_address);
					DEACCESS(Computed_variable)(source_variable_address);
					j++;
				}
				else
				{
					if (!source_variable&&Computed_variable_overlap(
						*independent_variable_address,independent_variable))
					{
						overlapping_variables=1;
					}
					return_code=
						Computed_variable_set_independent_variable_source_variable(
						*source_variable_address,independent_variable,source_variable);
				}
				i--;
				source_variable_address++;
				independent_variable_address++;
			}
			variable->number_of_source_variables -= j;
			if (return_code)
			{
				/* add <source_variable> if it is an <independent_variable> of
					<variable> */
					/*???DB.  Immediate independent variable */
				if ((source_variable||overlapping_variables)&&
					Computed_variable_is_independent_variable_of(variable,
					independent_variable))
				{
					if (j<1)
					{
						if (REALLOCATE(source_variable_address,variable->source_variables,
							struct Computed_variable *,
							(variable->number_of_source_variables)+1))
						{
							variable->source_variables=source_variable_address;
							if (REALLOCATE(independent_variable_address,
								variable->independent_variables,struct Computed_variable *,
								(variable->number_of_source_variables)+1))
							{
								variable->independent_variables=independent_variable_address;
							}
						}
					}
					else
					{
						independent_variable_address=variable->independent_variables;
						source_variable_address=variable->source_variables;
					}
					if (independent_variable_address&&source_variable_address)
					{
						independent_variable_address[variable->number_of_source_variables]=
							ACCESS(Computed_variable)(independent_variable);
						source_variable_address[variable->number_of_source_variables]=
							ACCESS(Computed_variable)(source_variable);
						(variable->number_of_source_variables)++;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_variable_set_independent_variable_source_variable.  "
							"Insufficient memory for extending source/independent variable "
							"pairs");
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_variable_set_independent_variable_source_variable.  "
				"Source and independent value type mis-match");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_variable_set_independent_variable_source_variable.  "
			"Invalid argument(s).  %p %p",variable,independent_variable);
	}
	LEAVE;

	return (return_code);
} /* Computed_variable_set_independent_variable_source_variable */

int Computed_variable_set_independent_variable_value(
	struct Computed_variable *variable,
	struct Computed_variable *independent_variable,
	struct Computed_value *value)
/*******************************************************************************
LAST MODIFIED : 3 February 2003

DESCRIPTION :
Sets the <value> of the <independent_variable> for the <variable>.
==============================================================================*/
{
	int i,return_code;
	struct Computed_variable **independent_variable_address;

	ENTER(Computed_variable_set_independent_variable_value);
	return_code=0;
	/* check arguments */
	if (variable&&independent_variable&&value)
	{
		if (variable->
			computed_variable_set_independent_variable_value_type_specific_function)
		{
			return_code=(variable->
				computed_variable_set_independent_variable_value_type_specific_function)
				(variable,independent_variable,value);
			i=variable->number_of_source_variables;
			independent_variable_address=variable->independent_variables;
			while (return_code&&(i>0))
			{
				return_code=Computed_variable_set_independent_variable_value(
					*independent_variable_address,independent_variable,value);
				independent_variable_address++;
				i--;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_variable_set_independent_variable_value.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_variable_set_independent_variable_value.  "
			"Invalid argument(s).  %p %p %p",variable,independent_variable,value);
	}
	LEAVE;

	return (return_code);
} /* Computed_variable_set_independent_variable_value */

int Computed_variable_get_independent_variable_value(
	struct Computed_variable *variable,
	struct Computed_variable *independent_variable,
	struct Computed_value *value)
/*******************************************************************************
LAST MODIFIED : 3 February 2003

DESCRIPTION :
Gets the <value> of the <independent_variable> for the <variable>.  If the
<value> has no type, then the default type for the <independent_variable> is
used otherwise a representation in <value>'s type is attempted.
==============================================================================*/
{
	int i,return_code;
	struct Computed_variable **independent_variable_address;

	ENTER(Computed_variable_get_independent_variable_value);
	return_code=0;
	/* check arguments */
	if (variable&&independent_variable&&value)
	{
		if (variable->
			computed_variable_get_independent_variable_value_type_specific_function)
		{
			return_code=(variable->
				computed_variable_get_independent_variable_value_type_specific_function)
				(variable,independent_variable,value);
			i=variable->number_of_source_variables;
			independent_variable_address=variable->independent_variables;
			while ((!return_code)&&(i>0))
			{
				return_code=Computed_variable_get_independent_variable_value(
					*independent_variable_address,independent_variable,value);
				independent_variable_address++;
				i--;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_variable_get_independent_variable_value.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_variable_get_independent_variable_value.  "
			"Invalid argument(s).  %p %p %p",variable,independent_variable,value);
	}
	LEAVE;

	return (return_code);
} /* Computed_variable_get_independent_variable_value */

int Computed_variable_depends_on_Computed_variable(
	struct Computed_variable *variable,struct Computed_variable *other_variable)
/*******************************************************************************
LAST MODIFIED : 4 February 2002

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
{
	int i,return_code;
	struct Computed_variable **source_variable_address;

	ENTER(Computed_variable_depends_on_Computed_variable);
	if (variable&&other_variable)
	{
		if (variable==other_variable)
		{
			return_code=1;
		}
		else
		{
			return_code=0;
			source_variable_address=variable->source_variables;
			i=variable->number_of_source_variables;
			while ((!return_code)&&(i>0))
			{
				return_code=Computed_variable_depends_on_Computed_variable(
					*source_variable_address,other_variable);
				i--;
				source_variable_address++;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_variable_depends_on_Computed_variable.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_variable_depends_on_Computed_variable */

int Computed_variable_depends_on_Computed_variable_in_list(
	struct Computed_variable *variable,
	struct LIST(Computed_variable) *variable_list)
/*******************************************************************************
LAST MODIFIED : 4 February 2003

DESCRIPTION :
Returns nonzero if <variable> depends on any variable in <variable_list>.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_variable_depends_on_Computed_variable_in_list);
	return_code=0;
	if (variable && variable_list)
	{
		if (FIRST_OBJECT_IN_LIST_THAT(Computed_variable)(
			Computed_variable_is_source_variable_conditional,
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
			"Computed_variable_depends_on_Computed_variable_in_list.  "
			"Invalid argument(s).  %p %p",variable,variable_list);
	}
	LEAVE;

	return (return_code);
} /* Computed_variable_depends_on_Computed_variable_in_list */

struct Computed_variable_value *CREATE(Computed_variable_value)(
	struct Computed_variable *variable,struct Computed_value *value)
/*******************************************************************************
LAST MODIFIED : 27 January 2003

DESCRIPTION :
Creates a <variable>/<value> pair.
==============================================================================*/
{
	struct Computed_variable_value *variable_value;

	ENTER(CREATE(Computed_variable_value));
	if (ALLOCATE(variable_value,struct Computed_variable_value,1))
	{
		variable_value->variable=variable;
		variable_value->value=value;
		variable_value->access_count=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Computed_variable_value).  "
			"Insufficient memory");
	}
	LEAVE;

	return (variable_value);
} /* CREATE(Computed_variable_value) */

int DESTROY(Computed_variable_value)(
	struct Computed_variable_value **variable_value_address)
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Frees memory/deaccess objects for Computed_variable_value at
<*variable_value_address>.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Computed_variable_value));
	return_code=0;
	if (variable_value_address&&(*variable_value_address))
	{
		if ((*variable_value_address)->access_count<=0)
		{
			DEALLOCATE(*variable_value_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"DESTROY(Computed_variable_value).  "
				"Positive access_count");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Computed_variable_value).  "
			"Missing variable_value");
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Computed_variable_value) */

DECLARE_OBJECT_FUNCTIONS(Computed_variable_value)

DECLARE_LIST_FUNCTIONS(Computed_variable_value)

/*???DB.  Where I'm up to */
int Computed_variable_is_defined(struct Computed_variable *variable,
	struct LIST(Computed_variable_value) *values)
/*******************************************************************************
LAST MODIFIED : 4 February 2003

DESCRIPTION :
Returns true if <variable> can be calculated with the specified <values>
over-riding, but not setting, the current values.

???DB.  Could use evaluate, but that would be less efficient and currently
	would write error messages if it failed
==============================================================================*/
{
	int i,return_code;
	struct Computed_variable **source_variable_address;

	ENTER(Computed_variable_is_defined);
	return_code=0;
	/* check arguments */
	if (variable&&values)
	{
		if (variable->computed_variable_is_defined_type_specific_function)
		{
			return_code=1;
			source_variable_address=variable->source_variables;
			i=variable->number_of_source_variables;
			while (return_code&&(i>0))
			{
				return_code=Computed_variable_is_defined(*source_variable_address,
					values);
				i--;
				source_variable_address++;
			}
			if (return_code)
			{
				return_code=
					(variable->computed_variable_is_defined_type_specific_function)(
					variable,values);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Computed_variable_is_defined.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_variable_is_defined.  "
			"Invalid argument(s).  %p %p",variable,values);
	}
	LEAVE;

	return (return_code);
} /* Computed_variable_is_defined */

int Computed_variable_evaluate(struct Computed_variable_value *variable_value,
	struct LIST(Computed_variable_value) *values);
/*******************************************************************************
LAST MODIFIED : 4 February 2003

DESCRIPTION :
Calculates the <variable_value> with the specified <values> over-riding, but not
setting, the current values.
==============================================================================*/

int Computed_variable_set_type_derivative(
	struct Computed_variable *derivative,
	struct Computed_variable *variable,
	struct LIST(Computed_variable) *independent_variables);
/*******************************************************************************
LAST MODIFIED : 29 January 2003

DESCRIPTION :
Sets <derivative> to be the derivative of the <variable> with respect to the
<independent_variables>.
==============================================================================*/

int Computed_variable_set_type_divergence(
	struct Computed_variable *divergence,
	struct Computed_variable *variable,
	struct LIST(Computed_variable) *independent_variables);
/*******************************************************************************
LAST MODIFIED : 24 January 2003

DESCRIPTION :
Sets <divergence> to be the divergence of the <variable> with respect to the
<independent_variables>.
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
