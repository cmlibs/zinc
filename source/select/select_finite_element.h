/*******************************************************************************
FILE : select_finite_element.h

LAST MODIFIED : 13 May 1997

DESCRIPTION :
Version of select widget using FE_node and GROUP(FE_node) objects.
==============================================================================*/
#if !defined (SELECT_FINITE_ELEMENT_H)
#define SELECT_FINITE_ELEMENT_H

#include "select/select.h"
/*#include "finite_element/finite_element.h"*/

PROTOTYPE_SELECT_GLOBAL_FUNCTIONS(FE_node);
PROTOTYPE_SELECT_GLOBAL_FUNCTIONS(GROUP(FE_node));

#endif /* !defined (SELECT_FINITE_ELEMENT_H) */
