/*******************************************************************************
FILE : selected_graphic.h

LAST MODIFIED : 18 February 2000

DESCRIPTION :
Indexed lists for storing selected objects in graphics by number, and ranges
of subobject numbers if required.
==============================================================================*/
#if !defined (SELECTED_GRAPHIC_H)
#define SELECTED_GRAPHIC_H

#include "general/list.h"
#include "general/multi_range.h"
#include "general/object.h"

/*
Global types
------------
*/

struct Selected_graphic;
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
The contents of this object are private.
==============================================================================*/

DECLARE_LIST_TYPES(Selected_graphic);

/*
Global functions
----------------
*/

struct Selected_graphic *CREATE(Selected_graphic)(int number);
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Creates a Selected_graphics for the given number. Its subranges will be NULL.
==============================================================================*/

int DESTROY(Selected_graphic)(
	struct Selected_graphic **selected_graphic_address);
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Destroys the Selected_graphic.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Selected_graphic);
PROTOTYPE_LIST_FUNCTIONS(Selected_graphic);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Selected_graphic,number,int);

struct Multi_range *Selected_graphic_get_subranges(
	struct Selected_graphic *selected_graphic);
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Returns the subranges in <selected_graphic>. Note the return value will be the
pointer to the subranges Multi_range in the selected_graphics; this may be NULL,
indicating there are no selected subranges. Also, it should not be modified -
its availability in this way is purely for efficiency of rendering.
==============================================================================*/

int Selected_graphic_set_subranges(struct Selected_graphic *selected_graphic,
	struct Multi_range *subranges);
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
DESTROYs any subranges in <selected_graphic> and replaces them with the given
<subranges>, which may be NULL. Note that this function does not make a copy of
the given <subranges> - they will now be owned by the <selected_graphic>.
==============================================================================*/

#endif /* !defined (SELECTED_GRAPHIC_H) */
