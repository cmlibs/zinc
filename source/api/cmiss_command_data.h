/*******************************************************************************
FILE : cmiss_command_data.h

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
#ifndef __CMISS_COMMAND_H__
#define __CMISS_COMMAND_H__

#include "api/cmiss_field.h"
#include "api/cmiss_time_keeper.h"
#if defined (WIN32_USER_INTERFACE)
#include <windows.h>
#endif /* defined (WIN32_USER_INTERFACE) */

/*
Global types
------------
*/
struct Cmiss_command_data;
typedef struct Cmiss_command_data *Cmiss_command_data_id;

struct manager_Cmiss_texture;
typedef struct manager_Cmiss_texture *Cmiss_texture_manager_id;

struct Cmiss_scene_viewer_package;
typedef struct Cmiss_scene_viewer_package *Cmiss_scene_viewer_package_id;

struct Cmiss_graphics_module;
#ifndef CMISS_GRAPHICS_MODULE_ID_DEFINED
typedef struct Cmiss_graphics_module * Cmiss_graphics_module_id;
#define CMISS_GRAPHICS_MODULE_ID_DEFINED
#endif /* CMISS_GRAPHICS_ID_DEFINED */

/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
Shifted the Cmiss_command_data to be internal to cmiss.c
==============================================================================*/

/*
Global functions
----------------
*/

#if !defined (WIN32_USER_INTERFACE)
Cmiss_command_data_id Cmiss_command_data_create(int argc, const char *argv[],
	const char *version_string);
#else /* !defined (WIN32_USER_INTERFACE) */
struct Cmiss_command_data *Cmiss_command_data_create(int argc, const char *argv[],
	const char *version_string, HINSTANCE current_instance, 
        HINSTANCE previous_instance, LPSTR command_line,int initial_main_window_state);
#endif /* !defined (WIN32_USER_INTERFACE) */
/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
Initialise all the subcomponents of cmgui and create the Cmiss_command_data
==============================================================================*/

/*******************************************************************************
 * Returns a new reference to the command data with reference count incremented.
 * Caller is responsible for destroying the new reference.
 * 
 * @param command_data  The command data to obtain a new reference to.
 * @return  New command data reference with incremented reference count.
 */
Cmiss_command_data_id Cmiss_command_data_access(Cmiss_command_data_id command_data);

/***************************************************************************//**
 * Destroys reference to the command data and sets pointer/handle to NULL.
 * Internally this just decrements the reference count.
 *
 * @param command_data_address  Address of command data reference.
 * @return  1 on success, 0 if invalid arguments.
 */
int Cmiss_command_data_destroy(Cmiss_command_data_id *command_data_address);

int Cmiss_command_data_execute_command(Cmiss_command_data_id command_data,
	const char *command);
/*******************************************************************************
LAST MODIFIED : 9 July 2007

DESCRIPTION :
Parses the supplied <command> using the command parser interpreter.
==============================================================================*/

struct Cmiss_region *Cmiss_command_data_get_root_region(
	Cmiss_command_data_id command_data);
/*******************************************************************************
LAST MODIFIED : 18 April 2003

DESCRIPTION :
Returns the root region from the <command_data>.
==============================================================================*/

int Cmiss_command_data_main_loop(Cmiss_command_data_id command_data);
/*******************************************************************************
LAST MODIFIED : 19 December 2002

DESCRIPTION :
Process events until some events request the program to finish.
==============================================================================*/

struct Cmiss_scene_viewer *Cmiss_command_data_get_graphics_window_pane_by_name(
	Cmiss_command_data_id command_data, const char *name, int pane_number);
/*******************************************************************************
LAST MODIFIED : 26 January 2007

DESCRIPTION :
Returns the a handle to the scene_viewer that inhabits the pane of a graphics_window.
==============================================================================*/

/***************************************************************************//**
 * Returns the handle to time keeper and also increments the access count of 
 * the returned time keeper by one.
 *
 * @param command_data  The Cmiss command data object.
 * @return  The time keeper if successfully called otherwise NULL.
 */
Cmiss_time_keeper_id Cmiss_command_data_get_time_keeper(
	Cmiss_command_data_id command_data);

/***************************************************************************//**
 * Returns a handle to the texture manager.
 *
 * @param command_data  The Cmiss command data object.
 * @return  The Texture manager.
 */
Cmiss_texture_manager_id Cmiss_command_data_get_texture_manager(
 Cmiss_command_data_id command_data);

/***************************************************************************//**
 * Returns a handle to a scene viewer package
 *
 * @param command_data The Cmiss command data object
 * @return The scene viewer package
 */
Cmiss_scene_viewer_package_id Cmiss_command_data_get_scene_viewer_package(
	Cmiss_command_data_id command_data);

/***************************************************************************//**
 * Returns a handle to the rendition graphics package.
 *
 * @param command_data The Cmiss command data object
 * @return The rendition graphics package
 */
Cmiss_graphics_module_id Cmiss_command_data_get_graphics_module(
	Cmiss_command_data_id command_data);

#endif /* __CMISS_COMMAND_H__ */
