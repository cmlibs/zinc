/*******************************************************************************
FILE : select_finite_element.c

LAST MODIFIED : 6 November 1998

DESCRIPTION :
Declares select widget functions for FE_node and GROUP(FE_node) objects.
Note that FE_node overrides all default identifier functions since it uses its
integer cm_node_identifier member as the identifier rather than a name string.
==============================================================================*/
#include "finite_element/finite_element.h"
#include "select/select_finite_element.h"
#include "select/select_private.h"

/* Must be large enough to store a cm_node_identifier in: */
#define NODE_STRING_SIZE 20

/*
Module types
------------
*/
FULL_DECLARE_SELECT_STRUCT_TYPE(FE_node);
FULL_DECLARE_SELECT_STRUCT_TYPE(GROUP(FE_node));

/*
Module functions
----------------
*/
PROTOTYPE_SELECT_MANAGER_CREATE_FUNCTION(FE_node)
/*****************************************************************************
LAST MODIFIED : 3 November 1998

DESCRIPTION :
Creates and returns a pointer to an empty struct FE_node.
The new object is given a unique identifier for the manager - and may base
it on the current object. Returns NULL if a unique identifier cannot be
found.
FE_node version uses the integer cm_node_identifier as an identifier.
???RC Should be part of manager.h
============================================================================*/
{
	struct FE_node *new_object;
	int new_node_number;

	ENTER(SELECT_MANAGER_CREATE(FE_node));
	/* 1. Create a blank new object: */
	new_object=CREATE(FE_node)(0,(struct FE_node *)NULL);
	/*???DB.  Can I get away with this ? */
	/*CREATE(FE_node)(0,default_FE_node_field_info,default_FE_node_values)) */
	/* 2. Create an identifier to be given to the new object: */
	new_node_number=get_next_FE_node_number(object_manager,1);
	/* 3. Ensure the new identifier is not used by some object in the manager */
	/*    and copy it to the new object. */
	/*???RC This check is redundant if get_next_FE_node_number works! */
	if (FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,cm_node_identifier)(
		new_node_number,object_manager))
	{
		/* Destroy the new object: */
		DESTROY(FE_node)(&new_object);
	}
	else
	{
		/* Copy new name to object: */
		if (!MANAGER_COPY_IDENTIFIER(FE_node,cm_node_identifier)(
			new_object,new_node_number))
		{
			/* Destroy the new object: */
			DESTROY(FE_node)(&new_object);
			display_message(ERROR_MESSAGE,
				"SELECT_MANAGER_CREATE(FE_node).  Could not set identifier.");
		}
	}
	LEAVE;

	return (new_object);
} /* SELECT_MANAGER_CREATE(FE_node) */

PROTOTYPE_SELECT_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(FE_node)
/*****************************************************************************
LAST MODIFIED : 15 April 1998

DESCRIPTION :
Calls MANAGER_COPY_WITHOUT_IDENTIFIER function with a cm_node_identifier member.
???RC Should this really know about object->cm_node_identifier?
============================================================================*/
{
	int return_code;

	ENTER(SELECT_MANAGER_COPY_WITHOUT_IDENTIFIER(FE_node));
	return_code = MANAGER_COPY_WITHOUT_IDENTIFIER(FE_node,cm_node_identifier)(
		object,new_data);
	LEAVE;

	return return_code;
} /* SELECT_MANAGER_COPY_WITHOUT_IDENTIFIER(FE_node) */

PROTOTYPE_SELECT_MANAGER_MODIFY_IDENTIFIER_AS_NAME_FUNCTION(FE_node)
/*****************************************************************************
LAST MODIFIED : 9 May 1997

DESCRIPTION :
Version for FE_node objects converts object_name into a node number and calls
MANAGER_MODIFY_IDENTIFIER function.
???RC Should this really know about object->cm_node_identifier?
============================================================================*/
{
	int return_code;
	int new_number;

	ENTER(SELECT_MANAGER_MODIFY_IDENTIFIER_AS_NAME(FE_node));
	sscanf(object_name,"%i",&new_number);
	return_code = MANAGER_MODIFY_IDENTIFIER(FE_node,cm_node_identifier)(
		object,new_number,object_manager);
	LEAVE;

	return return_code;
} /* SELECT_MANAGER_MODIFY_IDENTIFIER_AS_NAME(FE_node) */

PROTOTYPE_SELECT_FIND_BY_IDENTIFIER_AS_NAME_IN_MANAGER_FUNCTION(FE_node)
/*****************************************************************************
LAST MODIFIED : 9 May 1997

DESCRIPTION :
Macro interface for FIND_BY_IDENTIFIER_IN_MANAGER(object_type,identifier)
Version for FE_node objects converts object_name into a node number and calls
FIND_BY_IDENTIFIER_IN_MANAGER function.
???RC Should this really know about object->cm_node_identifier?
============================================================================*/
{
	struct FE_node *return_object_ptr;
	int new_number;

	ENTER(SELECT_FIND_BY_IDENTIFIER_AS_NAME_IN_MANAGER(FE_node));
	sscanf(object_name,"%i",&new_number);
	return_object_ptr = FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,cm_node_identifier)(
		new_number,object_manager);
	LEAVE;

	return return_object_ptr;
} /* SELECT_FIND_BY_IDENTIFIER_AS_NAME_IN_MANAGER(FE_node) */

DECLARE_SELECT_MODULE_FUNCTIONS(FE_node,"Number:")

DECLARE_DEFAULT_SELECT_MANAGER_CREATE_FUNCTION(GROUP(FE_node))
DECLARE_DEFAULT_SELECT_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(GROUP(FE_node))
DECLARE_DEFAULT_SELECT_MANAGER_MODIFY_IDENTIFIER_AS_NAME_FUNCTION(
	GROUP(FE_node))
DECLARE_DEFAULT_SELECT_FIND_BY_IDENTIFIER_AS_NAME_IN_MANAGER_FUNCTION(
	GROUP(FE_node))

DECLARE_SELECT_MODULE_FUNCTIONS(GROUP(FE_node),"Group named:")

/*
Global functions
----------------
*/
DECLARE_SELECT_GLOBAL_FUNCTIONS(FE_node)
DECLARE_SELECT_GLOBAL_FUNCTIONS(GROUP(FE_node))

