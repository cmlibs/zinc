/*******************************************************************************
FILE : choose_element_group.c

LAST MODIFIED : 21 January 2000

DESCRIPTION :
Widget for choosing a GROUP(FE_element), based on choose_object macro.
==============================================================================*/

#include "choose/choose_element_group.h"
#include "choose/choose_object_private.h"
#include "finite_element/finite_element.h"
#include "user_interface/message.h"

/*
Module types
------------
*/
FULL_DECLARE_CHOOSE_OBJECT_TYPE(GROUP(FE_element));

/*
Module functions
----------------
*/
DECLARE_CHOOSE_OBJECT_MODULE_FUNCTIONS(GROUP(FE_element))

/*
Global functions
----------------
*/
DECLARE_CHOOSE_OBJECT_GLOBAL_FUNCTIONS(GROUP(FE_element))

