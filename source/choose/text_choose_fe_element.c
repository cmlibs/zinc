/*******************************************************************************
FILE : text_choose_fe_element.c

LAST MODIFIED : 9 February 2000

DESCRIPTION :
Control for choosing an element by typing its identifier in a text box, based
on the text_choose_object macro.
==============================================================================*/

#include "choose/text_choose_fe_element.h"
#include "choose/text_choose_object_private.h"
#include "finite_element/finite_element.h"
#include "user_interface/message.h"

/*
Module types
------------
*/
FULL_DECLARE_TEXT_CHOOSE_OBJECT_TYPE(FE_element);

/*
Module functions
----------------
*/
DECLARE_TEXT_CHOOSE_OBJECT_MODULE_FUNCTIONS(FE_element)

/*
Global functions
----------------
*/
DECLARE_TEXT_CHOOSE_OBJECT_GLOBAL_FUNCTIONS(FE_element)

