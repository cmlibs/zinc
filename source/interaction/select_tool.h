/*******************************************************************************
FILE : select_tool.h

LAST MODIFIED : 24 August 2000

DESCRIPTION :
Interactive tool for selecting Any_objects associated with Scene_objects with
mouse and other devices.
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
#if !defined (SELECT_TOOL_H)
#define SELECT_TOOL_H

#include "graphics/material.h"
#include "interaction/interactive_tool.h"
#include "selection/any_object_selection.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/

struct Select_tool;
/*******************************************************************************
LAST MODIFIED : 24 August 2000

DESCRIPTION :
The contents of this structure are private.
==============================================================================*/

/*
Global functions
----------------
*/

struct Select_tool *CREATE(Select_tool)(
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct Any_object_selection *any_object_selection,
	struct Graphical_material *rubber_band_material,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Creates an Select_tool with Interactive_tool in
<interactive_tool_manager>. Selects Any_objects represented by scene_objects in
<any_object_selection> in response to interactive_events.
==============================================================================*/

int DESTROY(Select_tool)(struct Select_tool **select_tool_address);
/*******************************************************************************
LAST MODIFIED : 24 August 2000

DESCRIPTION :
Frees and deaccesses objects in the <select_tool> and deallocates the
structure itself.
==============================================================================*/

#endif /* !defined (SELECT_TOOL_H) */
