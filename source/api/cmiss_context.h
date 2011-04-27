/***************************************************************************//**
 * cmiss_context.h
 * 
 * API to access the main root structure of cmgui.
 */
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

#ifndef __CMISS_CONTEXT_H__
#define __CMISS_CONTEXT_H__

#include "api/cmiss_graphics_module.h"
#include "api/cmiss_scene_viewer.h"
#include "api/cmiss_region.h"
#include "api/cmiss_time_keeper.h"
#if defined (WIN32_USER_INTERFACE)
//#define WINDOWS_LEAN_AND_MEAN
#if !defined (NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#endif /* defined (WIN32_USER_INTERFACE) */

/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
==============================================================================*/

#ifndef CMISS_CONTEXT_ID_DEFINED
	struct Cmiss_context;
  typedef struct Cmiss_context * Cmiss_context_id;
  #define CMISS_CONTEXT_ID_DEFINED
#endif /* CMISS_CONTEXT_ID_DEFINED */


/***************************************************************************//**
 * Create a new cmgui context with an id <id>.
 *
 * @param id  The identifier given to the new context.
 * @return  a handle to a new Cmiss_context if successfully create, otherwise NULL.
 */
Cmiss_context_id Cmiss_context_create(const char *id);

/*******************************************************************************
 * Returns a new reference to the context with reference count incremented.
 * Caller is responsible for destroying the new reference.
 * 
 * @param context  The context to obtain a new reference to.
 * @return  New region reference with incremented reference count.
 */
Cmiss_context_id Cmiss_context_access(Cmiss_context_id context);

/***************************************************************************//**
 * Destroy a context.
 *
 * @param context_address  The address to the handle of the context
 *    to be destroyed Cmiss_context.
 * @return  1 if successfully destroy Cmiss_context otherwise 0.
 */
int Cmiss_context_destroy(Cmiss_context_id *context_address);

/***************************************************************************//**
 * Returns a handle to the default graphics module.
 *
 * @param context  Handle to a cmiss_context object.
 * @return  The handle to the default graphics module of the context if 
 *    successfully called, otherwise 0.
 */
Cmiss_graphics_module_id Cmiss_context_get_default_graphics_module(
	Cmiss_context_id context);

/***************************************************************************//**
 * Create a new graphics module and return the handle of it.
 *
 * @param context  Handle to a cmiss_context object.
 * @return  A newly created graphics_module if successfully called, otherwise 0.
 */
Cmiss_graphics_module_id Cmiss_context_create_graphics_module(Cmiss_context_id context);

/***************************************************************************//**
 * Returns the default region in the context.
 *
 * @param context  Handle to a cmiss_context object.
 * @return  The handle to the default region of the context if successfully 
 *    called, otherwise 0.
 */
Cmiss_region_id Cmiss_context_get_default_region(Cmiss_context_id context);

/***************************************************************************//**
 * Create a new region and return a reference to it. Use this function to create
 * a region forming the root of an independent region tree. To create regions
 * for addition to an existing region tree, use Cmiss_region_create_region.
 * 
 * @see Cmiss_region_create_region
 * @param context  Handle to a cmiss_context object.
 * @return  Reference to newly created region if successful, otherwise 0.
 */
Cmiss_region_id Cmiss_context_create_region(Cmiss_context_id context);

/***************************************************************************//**
 * Enable the internal user interface in cmgui.
 *
 * @param context  Handle to a cmiss_context object.
 * @return  1 if successfully initialized user interface, otherwise 0.
 */
#if !defined (WIN32_USER_INTERFACE)
int Cmiss_context_enable_user_interface(Cmiss_context_id context, 
	int in_argc, const char *in_argv[]);
#else
int Cmiss_context_enable_user_interface(
	struct Context *context, int in_argc, const char *in_argv[],
	HINSTANCE current_instance, HINSTANCE previous_instance, 
	LPSTR command_line,int initial_main_window_state);
#endif

/***************************************************************************//**
 * Execute cmgui command as in standalone cmgui application. 
 * User interface must be enabled before this function can be called successfully.
 *
 * @param context  Handle to a cmiss_context object.
 * @param command  Command to be executed.
 * @return  1 if command completed successfully, otherwise 0.
 */
int Cmiss_context_execute_command(Cmiss_context_id context, 
	const char *command);

/***************************************************************************//**
 * Returns the a handle to the scene_viewer that inhabits the pane of a 
 * graphics_window.
 * User interface must be enabled before this function can be called successfully.
 *
 * @param context  Handle to a cmiss_context object.
 * @return  The scene viewer if successfully called otherwise NULL.
 */
int Cmiss_context_run_main_loop(Cmiss_context_id context);

/***************************************************************************//**
 * Returns the a handle to the scene_viewer that inhabits the pane of a 
 * graphics_window.
 * User interface must be enabled before this function can be called successfully.
 *
 * @param context  Handle to a cmiss_context object.
 * @return  The scene viewer if successfully called otherwise NULL.
 */
Cmiss_scene_viewer_id Cmiss_context_get_graphics_window_pane_by_name(
	Cmiss_context_id context, const char *name, int pane_number);

/***************************************************************************//**
 * Returns the handle to time keeper and also increments the access count of 
 * the returned time keeper by one.
 * User interface must be enabled before this function can be called successfully.
 *
 * @param context  Handle to a cmiss_context object.
 * @return  The time keeper if successfully called otherwise NULL.
 */
Cmiss_time_keeper_id Cmiss_context_get_default_time_keeper(
	Cmiss_context_id context);

/***************************************************************************//**
 * Returns a handle to a scene viewer package
 * User interface must be enabled before this function can be called successfully.
 *
 * @param context  Handle to a cmiss_context object.
 * @return The scene viewer package if successfully called otherwise NULL.
 */
Cmiss_scene_viewer_package_id Cmiss_context_get_default_scene_viewer_package(
	Cmiss_context_id context);
#endif /* __CMISS_CONTEXT_H__ */
