/*******************************************************************************
FILE : choose_fe_field.c

LAST MODIFIED : 21 January 2000

DESCRIPTION :
Widget for choosing a FE_field, based on choose_object macro.
==============================================================================*/

#include "choose/choose_fe_field.h"
#include "choose/choose_object_private.h"
#include "finite_element/finite_element.h"

/*
Module types
------------
*/
FULL_DECLARE_CHOOSE_OBJECT_TYPE(FE_field);

/*
Module functions
----------------
*/
DECLARE_CHOOSE_OBJECT_MODULE_FUNCTIONS(FE_field)

/*
Global functions
----------------
*/
DECLARE_CHOOSE_OBJECT_GLOBAL_FUNCTIONS(FE_field)
