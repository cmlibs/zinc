/*******************************************************************************
FILE : text_choose_fe_node.c

LAST MODIFIED : 20 March 2003

DESCRIPTION :
Control for choosing a node by typing its identifier in a text box, based
on the text_choose_from_fe_region macro.
==============================================================================*/

#include "choose/text_choose_fe_node.h"
#include "choose/text_choose_from_fe_region_private.h"
#include "finite_element/finite_element.h"
#include "user_interface/message.h"

/*
Module types
------------
*/
FULL_DECLARE_TEXT_CHOOSE_FROM_FE_REGION_TYPE(FE_node);

/*
Module functions
----------------
*/

DECLARE_TEXT_CHOOSE_FROM_FE_REGION_MODULE_FUNCTIONS(FE_node)

/*
Global functions
----------------
*/

DECLARE_TEXT_CHOOSE_FROM_FE_REGION_GLOBAL_FUNCTIONS(FE_node)
