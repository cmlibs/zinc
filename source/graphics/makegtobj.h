/*******************************************************************************
FILE : makegtobj.h

LAST MODIFIED : 18 February 2000

DESCRIPTION :
Prototypes for graphics routines in the API.
==============================================================================*/
#if !defined (MAKEGTOBJ_H)
#define MAKEGTOBJ_H

#include "graphics/graphics_object.h"
#include "general/multi_range.h"

int makegtobject(gtObject *object,float time,int draw_selected);
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Convert graphical object into API object.
If <draw_selected> is set, only selected graphics are drawn, otherwise only
un-selected graphics are drawn.
==============================================================================*/
#endif
