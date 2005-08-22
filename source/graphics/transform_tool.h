/*******************************************************************************
FILE : transform_tool.h

LAST MODIFIED : 12 June 2000

DESCRIPTION :
Icon/tool representing the transform function on a graphics window.
Eventually use to store parameters for the transform function.
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
#if !defined (TRANSFORM_TOOL_H)
#define TRANSFORM_TOOL_H

#include "interaction/interactive_tool.h"

/*
Global types
------------
*/

struct Transform_tool;
/*******************************************************************************
LAST MODIFIED : 12 June 2000

DESCRIPTION :
The contents of this object are private.
==============================================================================*/

/*
Global functions
----------------
*/

int Interactive_tool_is_Transform_tool(struct Interactive_tool *interactive_tool);
/*******************************************************************************
LAST MODIFIED : 6 October 2000

DESCRIPTION :
Identifies whether an Interactive_tool is a Transform_tool.
==============================================================================*/

int Interactive_tool_transform_get_free_spin(
	struct Interactive_tool *interactive_tool);
/*******************************************************************************
LAST MODIFIED : 9 October 2000

DESCRIPTION :
If the interactive tool is of type Transform this function specifies whether 
the window should spin freely when tumbling.
==============================================================================*/

int Interactive_tool_transform_set_free_spin(struct Interactive_tool *interactive_tool,
	int free_spin);
/*******************************************************************************
LAST MODIFIED : 9 October 2000

DESCRIPTION :
If the interactive tool is of type Transform this function controls whether 
the window will spin freely when tumbling.
==============================================================================*/

struct Interactive_tool *create_Interactive_tool_transform(
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 9 October 2000

DESCRIPTION :
Creates a transform type Interactive_tool which control the transformation of
scene_viewers.
==============================================================================*/
#endif /* !defined (TRANSFORM_TOOL_H) */
