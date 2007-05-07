/*******************************************************************************
FILE : emoter_dialog.h

LAST MODIFIED : 4 May 2004

DESCRIPTION :
This module creates a emoter_slider input device.  A node group slider is
used to scale the distance between a fixed node and a group of nodes.
???DB.  Extend to other options
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
#if !defined (EMOTER_SLIDER_DIALOG_H)
#define EMOTER_SLIDER_DIALOG_H

#include "general/io_stream.h"
#include "graphics/graphics_window.h"
#include "region/cmiss_region.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/
struct Emoter_dialog;
/*******************************************************************************
LAST MODIFIED : 9 December 2003

DESCRIPTION :
==============================================================================*/

struct Create_emoter_slider_data
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
==============================================================================*/
{
	struct Execute_command *execute_command;
	struct Cmiss_region *root_region;
	struct MANAGER(FE_basis) *basis_manager;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
	struct MANAGER(Graphics_window) *graphics_window_manager;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
	struct MANAGER(Curve) *curve_manager;
	struct MANAGER(Scene) *scene_manager;
	struct IO_stream_package *io_stream_package;
	struct Scene *viewer_scene;
	struct Colour viewer_background_colour;
	struct Graphics_buffer_package *graphics_buffer_package;
	struct Light *viewer_light;
	struct Light_model *viewer_light_model;
#if defined (MOTIF)
	Widget parent, *curve_editor_dialog_address;
#endif /* defined (MOTIF) */
	struct Emoter_dialog **emoter_dialog_address;
	struct User_interface *user_interface;
}; /* struct Create_emoter_slider_data */

/*
Global functions
----------------
*/
int gfx_create_emoter(struct Parse_state *state,void *dummy_to_be_modified,
	void *create_emoter_slider_data_void);
/*******************************************************************************
LAST MODIFIED : 9 December 2003

DESCRIPTION :
Executes a GFX CREATE EMOTER command.  If there is a emoter dialog
in existence, then bring it to the front, otherwise create new one.
==============================================================================*/

int set_emoter_slider_value(struct Parse_state *state,
	void *dummy_to_be_modified,void *emoter_slider_dialog_widget_void);
/*******************************************************************************
LAST MODIFIED : 1 June 1997

DESCRIPTION :
==============================================================================*/

int gfx_modify_emoter(struct Parse_state *state,
	void *dummy_to_be_modified, void *emoter_dialog_widget_void);
/*******************************************************************************
LAST MODIFIED : 7 September 1999

DESCRIPTION :
Executes a GFX MODIFY EMOTER command.
==============================================================================*/

int DESTROY(Emoter_dialog)(struct Emoter_dialog **emoter_dialog_address);
/*******************************************************************************
LAST MODIFIED : 9 December 2003

DESCRIPTION :
Callback for the emoter dialog - tidies up all details - mem etc
==============================================================================*/
#endif /* !defined (EMOTER_SLIDER_DIALOG_H) */
