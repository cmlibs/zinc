/*******************************************************************************
FILE : text_choose_fe_element.h

LAST MODIFIED : 28 January 2003

DESCRIPTION :
Control for choosing an element by typing its identifier in a text box, based
on the text_choose_object macro.
==============================================================================*/
#if !defined (TEXT_CHOOSE_ELEMENT_H)
#define TEXT_CHOOSE_ELEMENT_H

#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "choose/text_choose_from_fe_region.h"

PROTOTYPE_TEXT_CHOOSE_FROM_FE_REGION_GLOBAL_FUNCTIONS(FE_element);

#endif /* !defined (TEXT_CHOOSE_ELEMENT_H) */
