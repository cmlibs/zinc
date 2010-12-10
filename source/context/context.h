/***************************************************************************//**
 * context.cpp
 * 
 * The main root structure of cmgui.
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

#if !defined (CONTEXT_H)
#define CONTEXT_H

struct Context;

#include "api/cmiss_context.h"
#include "command/cmiss.h"
#include "context/user_interface_module.h"

/***************************************************************************//**
 * Return the default command data object in context.
 *
 * @param context  Pointer to a cmiss_context object.
 * @return  the default command data if successfully, otherwise NULL.
 */
struct Cmiss_command_data *Cmiss_context_get_default_command_interpreter(
	struct Context *context);

/***************************************************************************//**
 * Return the any object selection in context.
 *
 * @param context  Pointer to a cmiss_context object.
 * @return  the any_object_selection if successfully, otherwise NULL.
 */
struct Any_object_selection *Cmiss_context_get_any_object_selection(
	struct Context *context);

/***************************************************************************//**
 * Return the element point ranges selection in context.
 *
 * @param context  Pointer to a cmiss_context object.
 * @return  the Element_point_ranges_selection if successfully, otherwise NULL.
 */
struct Element_point_ranges_selection *Cmiss_context_get_element_point_ranges_selection(
	struct Context *context);

/***************************************************************************//**
 * Return the event dispatcher in context.
 *
 * @param context  Pointer to a cmiss_context object.
 * @return  the default event_dispatcher if successfully, otherwise NULL.
 */
struct Event_dispatcher *Cmiss_context_get_default_event_dispatcher(
	struct Context *context);

/***************************************************************************//**
 * Return the IO_stream_package in context.
 *
 * @param context  Pointer to a cmiss_context object.
 * @return  the default IO_stream_package if successfully, otherwise NULL.
 */
struct IO_stream_package *Cmiss_context_get_default_IO_stream_package(
	struct Context *context);

/***************************************************************************//**
 * Return the default curve manager in context.
 *
 * @param context  Pointer to a cmiss_context object.
 * @return  the default curve_manager if successfully, otherwise NULL.
 */
struct MANAGER(Curve) *Cmiss_context_get_default_curve_manager(
	Cmiss_context_id context);

/***************************************************************************//**
 * Create and returns the internal user interface module in cmgui.
 *
 * @param context  Pointer to a cmiss_context object.
 * @return  user_interface_module if successfully create the user interface, 
 *    otherwise NULL.
 */
#if !defined (WIN32_USER_INTERFACE)
struct User_interface_module *Cmiss_context_create_user_interface(
	struct Context *context, int in_argc, const char *in_argv[]);
#else
struct User_interface_module *Cmiss_context_create_user_interface(
	struct Context *context, int in_argc, const char *in_argv[],
	HINSTANCE current_instance, HINSTANCE previous_instance, 
	LPSTR command_line,int initial_main_window_state);
#endif
#endif /* !defined (CONTEXT_H) */
