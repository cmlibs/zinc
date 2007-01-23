/*******************************************************************************
FILE : cmiss.h

LAST MODIFIED : 16 September 2004

DESCRIPTION :
Functions and types for executing cmiss commands.

This should only be included in cmgui.c and command/cmiss.c
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
#if !defined (COMMAND_CMISS_H)
#define COMMAND_CMISS_H

#include "command/command.h"
#include "general/io_stream.h"
#include "general/manager.h"
#include "region/cmiss_region.h"
#if defined (WIN32_USER_INTERFACE)
#include <windows.h>
#endif /* defined (WIN32_USER_INTERFACE) */

/*
Global types
------------
*/
struct Cmiss_command_data;
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Shifted the Cmiss_command_data to be internal to cmiss.c
==============================================================================*/

/*
Global functions
----------------
*/

int cmiss_execute_command(char *command_string,void *command_data);
/*******************************************************************************
LAST MODIFIED : 9 November 1998

DESCRIPTION :
Execute a <command_string>.
==============================================================================*/

#if defined (F90_INTERPRETER) || defined (PERL_INTERPRETER)
void execute_command(char *command_string,void *command_data_void, int *quit,
  int *error);
/*******************************************************************************
LAST MODIFIED : 28 March 2000

DESCRIPTION:
==============================================================================*/
#endif /* defined (F90_INTERPRETER) || defined (PERL_INTERPRETER) */

int cmiss_set_command(char *command_string,void *command_data_void);
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION:
Sets the <command_string> in the command box of the CMISS command_window, ready
for editing or entering. If there is no command_window, does nothing.
==============================================================================*/

#if !defined (WIN32_USER_INTERFACE)
struct Cmiss_command_data *CREATE(Cmiss_command_data)(int argc,char *argv[],
	char *version_string);
#else /* !defined (WIN32_USER_INTERFACE) */
struct Cmiss_command_data *CREATE(Cmiss_command_data)(int argc,char *argv[],
	char *version_string, HINSTANCE current_instance, 
        HINSTANCE previous_instance, LPSTR command_line,int initial_main_window_state);
#endif /* !defined (WIN32_USER_INTERFACE) */
/*******************************************************************************
LAST MODIFIED : 19 December 2002

DESCRIPTION :
Initialise all the subcomponents of cmgui and create the Cmiss_command_data
==============================================================================*/

int Cmiss_command_data_main_loop(struct Cmiss_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 19 December 2002

DESCRIPTION :
Process events until some events request the program to finish.
==============================================================================*/

int DESTROY(Cmiss_command_data)(struct Cmiss_command_data **command_data_address);
/*******************************************************************************
LAST MODIFIED : 19 December 2002

DESCRIPTION :
Clean up the command_data, deallocating all the associated memory and resources.
==============================================================================*/

struct Cmiss_region *Cmiss_command_data_get_root_region(
	struct Cmiss_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 18 April 2003

DESCRIPTION :
Returns the root region from the <command_data>.
==============================================================================*/

struct Time_keeper *Cmiss_command_data_get_default_time_keeper(
	struct Cmiss_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 30 July 2003

DESCRIPTION :
Returns the default time_keeper from the <command_data>.
==============================================================================*/

struct Execute_command *Cmiss_command_data_get_execute_command(
	struct Cmiss_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 28 May 2003

DESCRIPTION :
Returns the execute command structure from the <command_data>, useful for 
executing cmiss commands from C.
==============================================================================*/

struct IO_stream_package *Cmiss_command_data_get_IO_stream_package(
	struct Cmiss_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 16 September 2004

DESCRIPTION :
Returns the io_stream_package structure from the <command_data>
==============================================================================*/

struct Cmiss_fdio_package* Cmiss_command_data_get_fdio_package(
	struct Cmiss_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 10 March 2005

DESCRIPTION :
Gets an Fdio_package for this <command_data>
==============================================================================*/

struct Cmiss_idle_package* Cmiss_command_data_get_idle_package(
	struct Cmiss_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
Gets an Idle_package for this <command_data>
==============================================================================*/

struct Cmiss_timer_package* Cmiss_command_data_get_timer_package(
	struct Cmiss_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 11 April 2005

DESCRIPTION :
Gets a Timer_package for this <command_data>
==============================================================================*/

struct FE_element_selection *Cmiss_command_data_get_element_selection(
	struct Cmiss_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 4 July 2005

DESCRIPTION :
Returns the selected_element object from the <command_data>.
==============================================================================*/

struct FE_node_selection *Cmiss_command_data_get_node_selection(
	struct Cmiss_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 4 July 2005

DESCRIPTION :
Returns the selected_node object from the <command_data>.
==============================================================================*/

struct User_interface *Cmiss_command_data_get_user_interface(
	struct Cmiss_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 25 January 2006

DESCRIPTION :
Gets the user_interface for this <command_data>
==============================================================================*/

struct MANAGER(Texture) *Cmiss_command_data_get_texture_manager(
	struct Cmiss_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 7 November 2006

DESCRIPTION :
Returns the texture manager from the <command_data>.
==============================================================================*/

struct Cmiss_scene_viewer_package *Cmiss_command_data_get_scene_viewer_package(
	struct Cmiss_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 7 November 2006

DESCRIPTION :
Returns the texture manager from the <command_data>.
==============================================================================*/
#endif /* !defined (COMMAND_CMISS_H) */
