/***************************************************************************//**
 * FILE : node_viewer_wx.h
 *
 * Dialog for selecting nodes and viewing and/or editing field values. Works
 * with selection to display the last selected node, or set it if entered in
 * this dialog.
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
#if !defined (NODE_VIEWER_WX_H)
#define NODE_VIEWER_WX_H
 
#include "api/types/cmiss_graphics_module_id.h"
#include "api/types/cmiss_region_id.h"
#include "api/types/cmiss_time_keeper_id.h"

/*
Global Types
------------
*/

/***************************************************************************//**
 * Node viewer/editor dialog object.
 */
struct Node_viewer;

/*
Global Functions
----------------
*/

/***************************************************************************//**
 * Creates a dialog for choosing nodes and displaying and editing their fields.
 */
struct Node_viewer *Node_viewer_create(
	struct Node_viewer **node_viewer_address,
	const char *dialog_title,
	Cmiss_region_id root_region, int use_data,
	Cmiss_graphics_module_id graphics_module,
	Cmiss_time_keeper_id time_keeper);

/***************************************************************************//**
 * Closes and destroys the Node_viewer.
 */
int Node_viewer_destroy(struct Node_viewer **node_viewer_address);

/***************************************************************************//**
 * Pops the window for <node_viewer> to the front of those visible.
 */
int Node_viewer_bring_window_to_front(struct Node_viewer *node_viewer);

#endif /* !defined (NODE_VIEWER_WX_H) */
