/*******************************************************************************
FILE : choose_fe_field.c

LAST MODIFIED : 10 January 2003

DESCRIPTION :
Widget for choosing a FE_field, based on FE_REGION_CHOOSE_OBJECT macro.
==============================================================================*/

#include "choose/choose_fe_field.h"
#include "choose/fe_region_choose_object_private.h"
#include "finite_element/finite_element.h"

/*
Module types
------------
*/

FULL_DECLARE_FE_REGION_CHOOSE_OBJECT_TYPE(FE_field);

/*
Module functions
----------------
*/

DECLARE_FE_REGION_CHOOSE_OBJECT_MODULE_FUNCTIONS(FE_field)

/*
Global functions
----------------
*/

DECLARE_FE_REGION_CHOOSE_OBJECT_GLOBAL_FUNCTIONS(FE_field)
