/***************************************************************************//**
 * cmiss_context.c
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

#include "api/cmiss_context.h"
#include "context/context.h"
#include "context/user_interface_module.h"

#if !defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER)
int Cmiss_context_enable_user_interface(Cmiss_context_id context, 
	int in_argc, const char *in_argv[])
#else
int Cmiss_context_enable_user_interface(
	struct Context *context, int in_argc, const char *in_argv[],
	HINSTANCE current_instance, HINSTANCE previous_instance, 
	LPSTR command_line,int initial_main_window_state)
#endif
{
	int return_code = 0;
#if !defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER)
	struct User_interface_module *UI_module = Cmiss_context_create_user_interface(
		context, in_argc, in_argv);
#else
	struct User_interface_module *UI_module=  Cmiss_context_create_user_interface(
		context, in_argc, in_argv, current_instance, previous_instance, 
		command_line, initial_main_window_state);
#endif
	if (UI_module)
	{
		return_code = 1;
		User_interface_module_destroy(&UI_module);
	}

	return return_code;
}
