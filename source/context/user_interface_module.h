/*******************************************************************************
FILE : User_interface_module.h

LAST MODIFIED : 7 January 2003

DESCRIPTION :
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

#if !defined (USER_INTERFACE_MODULE_H)
#define USER_INTERFACE_MODULE_H

#include "comfile/comfile.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "graphics/colour.h"
#include "graphics/graphics_window.h"
#include "user_interface/event_dispatcher.h"
#include "user_interface/message.h"

struct User_interface_module
{
	int access_count;
	int argc, cleanup_argc;
	char **argv, **cleanup_argv;
	struct Colour background_colour,foreground_colour;
#if defined (CELL)
	struct Cell_interface *cell_interface;
#endif /* defined (CELL) */
	struct Event_dispatcher *event_dispatcher;
	struct Cmiss_scene_viewer_package *scene_viewer_package;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
	struct MANAGER(Graphics_window) *graphics_window_manager;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
#if defined (SGI_MOVIE_FILE) && defined (MOTIF_USER_INTERFACE)
	struct MANAGER(Movie_graphics) *movie_graphics_manager;
#endif /* defined (SGI_MOVIE_FILE) && defined (MOTIF_USER_INTERFACE) */
#if defined (MOTIF_USER_INTERFACE)
	struct Select_tool *select_tool;
	struct Prompt_window *prompt_window;
	struct Projection_window *projection_window;
	Widget curve_editor_dialog,data_grabber_dialog,
		grid_field_calculator_dialog,input_module_dialog,
		sync_2d_3d_dialog;
	struct Element_creator *element_creator;
	struct Time_editor_dialog *time_editor_dialog;
#endif /* defined (MOTIF_USER_INTERFACE) */
	struct Time_keeper *default_time_keeper;
	struct User_interface *user_interface;
	struct Emoter_dialog *emoter_slider_dialog;
#if defined (MOTIF_USER_INTERFACE) || defined (WX_USER_INTERFACE)
	struct MANAGER(Comfile_window) *comfile_window_manager;
	struct Node_viewer *data_viewer,*node_viewer;
	struct Element_point_viewer *element_point_viewer;
	struct Material_editor_dialog *material_editor_dialog;
	struct Scene_editor *scene_editor;
	struct Spectrum_editor_dialog *spectrum_editor_dialog;
#endif /* defined (MOTIF_USER_INTERFACE) || defined (WX_USER_INTERFACE) */
	struct Graphics_buffer_package *graphics_buffer_package;
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
};

/***************************************************************************//**
 * Create a new user_interaface_module, access count will be set to 1.
 *
 * @param context  pointer to a context object which this user interface module
 *   will be built on.
 * @param in_argc  number of arguments
 * @param in_argv  array to the value of each argument
 * @return  handle to an user_interface_module.
 */
#if !defined (WIN32_USER_INTERFACE)
struct User_interface_module *User_interface_module_create(
	struct Context *context, int in_argc, const char *in_argv[]);
#else
struct User_interface_module *User_interface_module_create(
	struct Context *context, int in_argc, const char *in_argv[],
	HINSTANCE current_instance, HINSTANCE previous_instance, 
	LPSTR command_line,int initial_main_window_state);
#endif

/***************************************************************************//**
 * Access the user_interaface_module, increase the access count of the module 
 * by one.
 *
 * @param UI_module  pointer to the "to be accessed" user interface module.
 * @return  handle to an user_interface_module if successfully called, 
 *   otherwise NULL.
 */
struct User_interface_module *User_interface_module_access(
	struct User_interface_module *UI_module);

/***************************************************************************//**
 * Dereference a user_interaface_module, decrease the access count of the module 
 * by one.
 *
 * @param UI_module  address to a pointer to the "to be accessed" 
 *   User_interface_module.
 * @return  address of the handle of an User_interface_module.
 */
int User_interface_module_destroy(
	struct User_interface_module **UI_module_address);

#endif
