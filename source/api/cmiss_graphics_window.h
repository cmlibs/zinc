/*******************************************************************************
FILE : cmiss_graphics_window.h

LAST MODIFIED : 18 September 2002

DESCRIPTION :
The public interface to the Cmiss_graphics_window object.
==============================================================================*/
#ifndef __CMISS_GRAPHICS_WINDOW_H__
#define __CMISS_GRAPHICS_WINDOW_H__

#include "api/cmiss_scene_viewer.h"

int Cmiss_graphics_window_get_scene_viewer_by_name(char *graphics_window_name,
	int pane_number, Cmiss_scene_viewer_id *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 18 September 2002

DESCRIPTION :
Returns the <cmiss_scene_viewer_id> that corresponds to the <graphics_window_name>
and the <pane_number>.  The first pane is <pane_number> 0.
==============================================================================*/
#endif /* __CMISS_GRAPHICS_WINDOW_H__ */
