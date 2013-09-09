/***************************************************************************//**
 * FILE : interactivetool.h
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
