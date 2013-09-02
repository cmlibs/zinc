/***************************************************************************//**
 * FILE : interactivetool.h
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
 * Portions created by the Initial Developer are Copyright (C) 2011
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

#ifndef CMZN_INTERACTIVETOOL_H__
#define CMZN_INTERACTIVETOOL_H__

#include "types/interactivetoolid.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * Execute cmgui command as in standalone cmgui application however this execute
 * command function will apply to the interactive tool being passed into this
 * function only. It takes a string of command as gfx <interactive_tool> does.
 *
 * NOTE: This function may be removed in the future once more API functions are
 * made available to the users.
 *
 * @param interactive_tool  Handle to a cmzn_interactive_tool object.
 * @param command  Command to be executed.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
int cmzn_interactive_tool_execute_command(cmzn_interactive_tool_id interactive_tool,
	const char *command);

/*******************************************************************************
 * Returns a new handle to the interactive tool with reference count incremented.
 * Caller is responsible for destroying the new handle.
 *
 * @param mesh  The interactive tool to obtain a new reference to.
 * @return  New interactive tool handle with incremented reference count.
 */
cmzn_interactive_tool_id cmzn_interactive_tool_access(
	cmzn_interactive_tool_id interactive_tool);

/***************************************************************************//**
 * Destroy the interactive_tool.
 *
 * @param interactive_tool  address to the handle to the "to be destroyed"
 *   zinc interactive_tool.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
int cmzn_interactive_tool_destroy(cmzn_interactive_tool_id *interactive_tool);

#ifdef __cplusplus
}
#endif

#endif /* CMZN_INTERACTIVETOOL_H__ */
