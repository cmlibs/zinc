/*******************************************************************************
FILE : interaction_graphics.h

LAST MODIFIED : 10 July 2000

DESCRIPTION :
Functions for building graphics assisting interaction, eg. rubber-band effect.
==============================================================================*/
#if !defined (INTERACTION_GRAPHICS_H)
#define INTERACTION_GRAPHICS_H

#include "graphics/graphics_object.h"
#include "interaction/interaction_volume.h"

/*
Global functions
----------------
*/

int Interaction_volume_make_polyline_extents(
	struct Interaction_volume *interaction_volume,
	struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
Fills <graphics_object> - of type g_POLYLINE with lines marking the box
enclosing the <interaction volume>, used for rubber-banding. Lines are put at
time 0 in the graphics object; any other primitives at that time are cleared.
==============================================================================*/

#endif /* !defined (INTERACTION_GRAPHICS_H) */
