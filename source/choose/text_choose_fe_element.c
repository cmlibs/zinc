/*******************************************************************************
FILE : text_choose_fe_element.c

LAST MODIFIED : 28 January 2003

DESCRIPTION :
Control for choosing an element by typing its identifier in a text box, based
on the text_choose_from_fe_region macro.
==============================================================================*/

#include "choose/text_choose_fe_element.h"
#include "choose/text_choose_from_fe_region_private.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "user_interface/message.h"

/*
Module types
------------
*/
FULL_DECLARE_TEXT_CHOOSE_FROM_FE_REGION_TYPE(FE_element);

/*
Module functions
----------------
*/

DECLARE_TEXT_CHOOSE_FROM_FE_REGION_MODULE_FUNCTIONS(FE_element)

/*
Global functions
----------------
*/
DECLARE_TEXT_CHOOSE_FROM_FE_REGION_GLOBAL_FUNCTIONS(FE_element)

