/*******************************************************************************
FILE : text_choose_fe_node.c

LAST MODIFIED : 9 February 2000

DESCRIPTION :
Control for choosing a node by typing its identifier in a text box, based
on the text_choose_object macro.
==============================================================================*/

#include "choose/text_choose_fe_node.h"
#include "choose/text_choose_object_private.h"
#include "finite_element/finite_element.h"
#include "user_interface/message.h"

/*
Module types
------------
*/
FULL_DECLARE_TEXT_CHOOSE_OBJECT_TYPE(FE_node);

/*
Module functions
----------------
*/
DECLARE_TEXT_CHOOSE_OBJECT_MODULE_FUNCTIONS(FE_node)

/*
Global functions
----------------
*/
DECLARE_TEXT_CHOOSE_OBJECT_GLOBAL_FUNCTIONS(FE_node)

