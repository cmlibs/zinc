/*******************************************************************************
FILE : select_finite_element.c

LAST MODIFIED : 20 December 2000

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
LAST MODIFIED : 20 April 2000

DESCRIPTION :
Creates a new struct FE_node with a unique identifier in <object_manager>.
It <template_object> is supplied, the new_object will be a copy of it and its
identifier may be derived from it.
FE_node version uses the integer cm_node_identifier as an identifier.
???RC Should be part of manager.h
============================================================================*/
{
	struct FE_node *new_object;

	ENTER(SELECT_MANAGER_CREATE(FE_node));
	if (object_manager)
	{
		new_object=CREATE(FE_node)(get_next_FE_node_number(object_manager,1),
			template_object);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"SELECT_MANAGER_CREATE(FE_node).  Invalid argument(s)");
		new_object=(struct FE_node *)NULL;
	}
	LEAVE;

	return (new_object);
} /* SELECT_MANAGER_CREATE(FE_node) */

PROTOTYPE_SELECT_MANAGER_MODIFY_IDENTIFIER_AS_NAME_FUNCTION(FE_node)
/*****************************************************************************
LAST MODIFIED : 20 December 2000

DESCRIPTION :
Version for FE_node objects converts object_name into a node number and calls
MANAGER_MODIFY_IDENTIFIER function.
???RC Should this really know about object->cm_node_identifier?
============================================================================*/
{
	int return_code;
	int new_number;

	ENTER(SELECT_MANAGER_MODIFY_IDENTIFIER_AS_NAME(FE_node));
#if defined (OLD_CODE)
	USE_PARAMETER(object_name);
	USE_PARAMETER(object);
	USE_PARAMETER(object_manager);
	display_message(ERROR_MESSAGE,
		"SELECT_MANAGER_MODIFY_IDENTIFIER_AS_NAME(FE_node).  "
		"Not allowed to rename nodes");
	return_code=0;
#endif /* defined (OLD_CODE) */
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

