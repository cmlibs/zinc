/*******************************************************************************
FILE : cmiss_command.c

LAST MODIFIED : 5 April 2004

DESCRIPTION :
The public interface to the some of the internal functions of cmiss.
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#include <stdlib.h>
#include "api/cmiss_command_data.h"
#include "api/cmiss_time_keeper.h"
#include "command/cmiss.h"
#include "time/time_keeper.h"
#include "general/debug.h"
#include "graphics/graphics_window.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

int Cmiss_command_data_execute_command(struct Cmiss_command_data *command_data,
	const char *command)
/*******************************************************************************
LAST MODIFIED : 9 July 2007

DESCRIPTION :
Parses the supplied <command> using the command parser interpreter.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_command_data_execute_command);
	if (command)
	{
		return_code = cmiss_execute_command((char *)command,
			(void *)command_data);
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_command_data_execute_command */

struct Cmiss_scene_viewer *Cmiss_command_data_get_graphics_window_pane_by_name(
	struct Cmiss_command_data *command_data, const char *name, int pane_number)
/*******************************************************************************
LAST MODIFIED : 26 January 2007

DESCRIPTION :
Returns the a handle to the scene_viewer that inhabits the pane of a graphics_window.
No longer a method in api/cmiss_graphics_window where it relied upon a global
variable.
==============================================================================*/
{
	struct Scene_viewer *scene_viewer;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
	struct Graphics_window *window;
	struct MANAGER(Graphics_window) *graphics_window_manager;
#else
	USE_PARAMETER(pane_number);
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */

	ENTER(Cmiss_command_data_get_graphics_window_pane_by_name);
	if (command_data && name)
	{
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
		if (graphics_window_manager = 
			Cmiss_command_data_get_graphics_window_manager(command_data))
		{
			if (window=FIND_BY_IDENTIFIER_IN_MANAGER(Graphics_window,name)(
				(char *)name, graphics_window_manager))
			{
				scene_viewer = Graphics_window_get_Scene_viewer(window, pane_number);
			}
			else
			{
				display_message(WARNING_MESSAGE,"Could not find window named %s",
					name);
				scene_viewer = (struct Scene_viewer *)NULL;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"No graphics window manager in this cmgui");
			scene_viewer = (struct Scene_viewer *)NULL;
		}
#else /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
		display_message(WARNING_MESSAGE,"No graphics windows in this cmgui");
		scene_viewer = (struct Scene_viewer *)NULL;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_command_data_get_graphics_window_pane_by_name.  "
			"Invalid arguments.");
		scene_viewer = (struct Scene_viewer *)NULL;
	}
	LEAVE;

	return (scene_viewer);
} /* Cmiss_command_data_get_graphics_window_pane_by_name */

Cmiss_time_keeper_id Cmiss_command_data_get_time_keeper(
	Cmiss_command_data_id command_data)
{
	Cmiss_time_keeper_id time_keeper;

	ENTER(Cmiss_command_data_get_time_keeper);
	if (command_data)
	{
		time_keeper = ACCESS(Time_keeper)
			(Cmiss_command_data_get_default_time_keeper(command_data));
	}
	else
	{
		time_keeper = NULL;
	}
	LEAVE;

	return (time_keeper);
}
