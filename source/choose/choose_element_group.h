/*******************************************************************************
FILE : choose_element_group.h

LAST MODIFIED : 23 JULY 1997

DESCRIPTION :
Widget for choosing a GROUP(FE_element), based on choose_object macro.
==============================================================================*/
#if !defined (CHOOSE_ELEMENT_GROUP_H)
#define CHOOSE_ELEMENT_GROUP_H

#include "finite_element/finite_element.h"
#include "choose/choose_object.h"

PROTOTYPE_CHOOSE_OBJECT_GLOBAL_FUNCTIONS(GROUP(FE_element));

#endif /* !defined (CHOOSE_ELEMENT_GROUP_H) */
