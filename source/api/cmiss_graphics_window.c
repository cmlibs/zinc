/*******************************************************************************
FILE : cmiss_graphics_window.c

LAST MODIFIED : 10 September 2002

DESCRIPTION :
The public interface to the Cmiss_graphics_window object.
==============================================================================*/
#include <stdarg.h>
#include "api/cmiss_graphics_window.h"
#include "api/cmiss_graphics_window_private.h"
#include "general/debug.h"
#include "general/manager.h"
#include "general/object.h"
#include "graphics/graphics_window.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

struct Cmiss_graphics_window_data
/*******************************************************************************
LAST MODIFIED : 9 September 2002

DESCRIPTION:
The default data used to create Cmiss_graphics_windows.
==============================================================================*/
{
	struct MANAGER(Graphics_window) *window_manager;
};

/*
Module variables
----------------
*/

static struct Cmiss_graphics_window_data *cmiss_graphics_window_data = NULL;

/*
Global functions
----------------
*/

int Cmiss_graphics_window_free_data(void)
/*******************************************************************************
LAST MODIFIED : 10 September 2002

DESCRIPTION:
Frees all the data used to scene viewer objects.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_graphics_window_free_data);
	if (cmiss_graphics_window_data)
	{
		DEALLOCATE(cmiss_graphics_window_data);
		cmiss_graphics_window_data = (struct Cmiss_graphics_window_data *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_graphics_window_free_data.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphics_window_free_data */

int Cmiss_graphics_window_set_data(
	struct MANAGER(Graphics_window) *window_manager)
/*******************************************************************************
LAST MODIFIED : 10 September 2002

DESCRIPTION:
Initialises all the data used to scene viewer objects.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_graphics_window_set_data);
	if (window_manager)
	{
		if (cmiss_graphics_window_data)
		{
			/* Free the previous copy */
			Cmiss_graphics_window_free_data();
		}
		if (ALLOCATE(cmiss_graphics_window_data, struct Cmiss_graphics_window_data, 1))
		{
			cmiss_graphics_window_data->window_manager = window_manager;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_graphics_window_set_data.  "
				"Unable to allocate data storage");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_graphics_window_set_data.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphics_window_set_data */

int Cmiss_graphics_window_get_scene_viewer_by_name(char *graphics_window_name,
	int pane_number, Cmiss_scene_viewer_id *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 18 September 2002

DESCRIPTION :
Returns the <cmiss_scene_viewer_id> that corresponds to the <graphics_window_name>
and the <pane_number>.  The first pane is <pane_number> 0.
==============================================================================*/
{
	int return_code;
	struct Graphics_window *window;

	ENTER(Cmiss_graphics_window_get_scene_viewer_by_name);
	if (cmiss_graphics_window_data)
	{
		if (window=FIND_BY_IDENTIFIER_IN_MANAGER(Graphics_window,name)(
			graphics_window_name, cmiss_graphics_window_data->window_manager))
		{
			*scene_viewer = Graphics_window_get_Scene_viewer(window, pane_number);
			return_code = 1;
		}
		else
		{
			display_message(WARNING_MESSAGE,"Could not find window named %s",
				graphics_window_name);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_graphics_window_get_scene_viewer_by_name.  "
			"The Cmiss_graphics_window data must be initialised before any api "
			"calls can be made.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphics_window_get_scene_viewer_by_name */


