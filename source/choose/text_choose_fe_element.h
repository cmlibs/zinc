/*******************************************************************************
FILE : text_choose_fe_element.h

LAST MODIFIED : 9 September 1999

DESCRIPTION :
Control for choosing an element by typing its identifier in a text box, based
on the text_choose_object macro.
==============================================================================*/
#if !defined (TEXT_CHOOSE_ELEMENT_H)
#define TEXT_CHOOSE_ELEMENT_H

#include "finite_element/finite_element.h"
#include "choose/text_choose_object.h"

PROTOTYPE_TEXT_CHOOSE_OBJECT_GLOBAL_FUNCTIONS(FE_element);

#endif /* !defined (TEXT_CHOOSE_ELEMENT_H) */
