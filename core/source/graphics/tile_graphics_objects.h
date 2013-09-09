/*******************************************************************************
FILE : tile_graphics_object.h

LAST MODIFIED : 29 November 2007

DESCRIPTION :
Header file for rendergl.c, GL rendering calls (API specific)
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (TILE_GRAPHICS_OBJECTS_H)
#define TILE_GRAPHICS_OBJECTS_H

#include "graphics/graphics_object.h"

struct GT_surface *tile_GT_surface(struct GT_surface *surface, 
	struct Texture_tiling *texture_tiling);
/*******************************************************************************
LAST MODIFIED : 29 November 2007

DESCRIPTION :
Split a GT_surface <surface> based on its texture coordinates and 
<texture_tiling> boundaries.  Returns a surface or linked list of surfaces
that have equivalent geometry separated into separate surfaces for separate
tiles.
==============================================================================*/

#endif /* !defined (TILE_GRAPHICS_OBJECTS_H) */
