/*******************************************************************************
FILE : select_environment_map.c

LAST MODIFIED : 18 May 1998

DESCRIPTION :
Declares select widget functions for Environment_map objects.
==============================================================================*/
#include "graphics/environment_map.h"
#include "select/select_environment_map.h"
#include "select/select_private.h"

/*
Module types
------------
*/
FULL_DECLARE_SELECT_STRUCT_TYPE(Environment_map);

/*
Module functions
----------------
*/
DECLARE_DEFAULT_SELECT_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(
	Environment_map)
DECLARE_DEFAULT_SELECT_MANAGER_MODIFY_IDENTIFIER_AS_NAME_FUNCTION(
	Environment_map)
DECLARE_DEFAULT_SELECT_FIND_BY_IDENTIFIER_AS_NAME_IN_MANAGER_FUNCTION(
	Environment_map)
DECLARE_DEFAULT_SELECT_MANAGER_CREATE_FUNCTION(Environment_map)

DECLARE_SELECT_MODULE_FUNCTIONS(Environment_map,"Env. map named:")

/*
Global functions
----------------
*/
DECLARE_SELECT_GLOBAL_FUNCTIONS(Environment_map)
