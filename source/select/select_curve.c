/*******************************************************************************
FILE : select_control_curve.c

LAST MODIFIED : 8 November 1999

DESCRIPTION :
Declares select widget functions for Control_curve objects.
==============================================================================*/

#include "curve/control_curve.h"
#include "select/select_control_curve.h"
#include "select/select_private.h"

/*
Module types
------------
*/
FULL_DECLARE_SELECT_STRUCT_TYPE(Control_curve);

/*
Module functions
----------------
*/
/*
Module functions
----------------
*/
DECLARE_DEFAULT_SELECT_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Control_curve)
DECLARE_DEFAULT_SELECT_MANAGER_MODIFY_IDENTIFIER_AS_NAME_FUNCTION(Control_curve)
DECLARE_DEFAULT_SELECT_FIND_BY_IDENTIFIER_AS_NAME_IN_MANAGER_FUNCTION(
	Control_curve)

PROTOTYPE_SELECT_MANAGER_CREATE_FUNCTION(Control_curve)
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Creates and returns a pointer to an empty struct object_type.
The new object is given a unique identifier for the manager - and may base
it on the current object. Returns NULL if a unique identifier cannot be
found.
???RC Should really be part of manager.h
==============================================================================*/
{
	char *new_curve_name, *temp_name;
	static char copy_string[]="+";
	static char default_name[]="new";
	struct Control_curve *new_curve;

	ENTER(SELECT_MANAGER_CREATE(Control_curve));
	/* 1. Get a new, unique identifier for the curve */
	if (current_object)
	{
		GET_NAME(Control_curve)(current_object,&new_curve_name);
	}
	else
	{
		/* Give the new curve some default name */
		if (ALLOCATE(new_curve_name,char,strlen(default_name)+1))
		{
			strcpy(new_curve_name,default_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"SELECT_MANAGER_CREATE(Control_curve).  Could not allocate new name");
		}
	}
	/* 3. Ensure the new identifier is not used by some curve in the manager */
	/*    and copy it to the new curve. */
	/* Keep on appending the copy_string onto the name until it is unique: */
	while ((new_curve_name)&&(FIND_BY_IDENTIFIER_IN_MANAGER(
		Control_curve,name)(new_curve_name,object_manager)))
	{
		if (REALLOCATE(temp_name,new_curve_name,char,
			strlen(new_curve_name)+strlen(copy_string)+1))
		{
			new_curve_name=temp_name;
			strcat(new_curve_name,copy_string);
		}
		else
		{
			DEALLOCATE(new_curve_name);
			display_message(ERROR_MESSAGE,
				"SELECT_MANAGER_CREATE(Control_curve"
				").  Could not give curve a unique name");
		}
	}
	if (new_curve_name)
	{
		new_curve=CREATE(Control_curve)(new_curve_name,LINEAR_LAGRANGE,1);
		DEALLOCATE(new_curve_name);
	}
	else
	{
		new_curve=(struct Control_curve *)NULL;
	}
	if (!new_curve)
	{
		display_message(ERROR_MESSAGE,
			"SELECT_MANAGER_CREATE(Control_curve).  Unable to create new curve");
	}
	LEAVE;

	return (new_curve);
} /* SELECT_MANAGER_CREATE(Control_curve) */

DECLARE_SELECT_MODULE_FUNCTIONS(Control_curve,"Curve named:")

/*
Global functions
----------------
*/
DECLARE_SELECT_GLOBAL_FUNCTIONS(Control_curve)

