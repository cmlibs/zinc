/*******************************************************************************
FILE : cmiss_graphics_window.h

LAST MODIFIED : 9 September 2002

DESCRIPTION :
The public interface to the Cmiss_graphics_window object for rendering cmiss
scenes.
==============================================================================*/
#ifndef __CMISS_GRAPHICS_WINDOW_PRIVATE_H__
#define __CMISS_GRAPHICS_WINDOW_PRIVATE_H__

#include "api/cmiss_graphics_window.h"
#include "general/debug.h"
#include "general/manager.h"
#include "general/object.h"
#include "graphics/graphics_window.h"

int Cmiss_graphics_window_free_data(void);
/*******************************************************************************
LAST MODIFIED : 10 September 2002

DESCRIPTION:
Frees all the data used to scene viewer objects.
==============================================================================*/

int Cmiss_graphics_window_set_data(
	struct MANAGER(Graphics_window) *window_manager);
/*******************************************************************************
LAST MODIFIED : 10 September 2002

DESCRIPTION:
Initialises all the data used to create scene viewer objects.
==============================================================================*/
#endif /* __CMISS_GRAPHICS_WINDOW_PRIVATE_H__ */
