/*******************************************************************************
FILE : select_graphical_material.c

LAST MODIFIED : 20 April 2000

DESCRIPTION :
Declares select widget functions for Graphical_material objects.
==============================================================================*/

#include "graphics/material.h"
#include "select/select_graphical_material.h"
#include "select/select_private.h"

/*
Module types
------------
*/
FULL_DECLARE_SELECT_STRUCT_TYPE(Graphical_material);

/*
Module functions
----------------
*/
DECLARE_DEFAULT_SELECT_MANAGER_MODIFY_IDENTIFIER_AS_NAME_FUNCTION(
	Graphical_material)
DECLARE_DEFAULT_SELECT_FIND_BY_IDENTIFIER_AS_NAME_IN_MANAGER_FUNCTION(
	Graphical_material)
DECLARE_DEFAULT_SELECT_MANAGER_CREATE_FUNCTION(Graphical_material)

DECLARE_SELECT_MODULE_FUNCTIONS(Graphical_material,"Material named:")

/*
Global functions
----------------
*/
DECLARE_SELECT_GLOBAL_FUNCTIONS(Graphical_material)

