/*******************************************************************************
FILE : choose_node_group.h

LAST MODIFIED : 23 October 1998

DESCRIPTION :
Widget for choosing a GROUP(FE_node), based on choose_object macro.
==============================================================================*/
#if !defined (CHOOSE_NODE_GROUP_H)
#define CHOOSE_NODE_GROUP_H

#include "finite_element/finite_element.h"
#include "choose/choose_object.h"

PROTOTYPE_CHOOSE_OBJECT_GLOBAL_FUNCTIONS(GROUP(FE_node));

#endif /* !defined (CHOOSE_NODE_GROUP_H) */
