/*******************************************************************************
FILE : choose_node_group.c

LAST MODIFIED : 24 October 1998

DESCRIPTION :
Widget for choosing a GROUP(FE_node), based on choose_object macro.
==============================================================================*/

#include "choose/choose_node_group.h"
#include "choose/choose_object_private.h"
#include "finite_element/finite_element.h"
#include "user_interface/message.h"

/*
Module types
------------
*/
FULL_DECLARE_CHOOSE_OBJECT_STRUCT_TYPE(GROUP(FE_node));

/*
Module functions
----------------
*/
DECLARE_CHOOSE_OBJECT_MODULE_FUNCTIONS(GROUP(FE_node))

/*
Global functions
----------------
*/
DECLARE_CHOOSE_OBJECT_GLOBAL_FUNCTIONS(GROUP(FE_node))

