/**
FILE : cad_tool.h

CREATED : 20 June 2010

DESCRIPTION :
Interactive tool for selecting cad primitives with a mouse and other devices.
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
#if !defined (CAD_TOOL_H)
#define CAD_TOOL_H

#include "interaction/interactive_tool.h"
#include "time/time_keeper.h"

/*
Global types
------------
*/

struct Cad_tool;

/*
Global functions
----------------
*/

struct Cad_tool *CREATE(Cad_tool)(
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct Cmiss_region *region,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Graphical_material *rubber_band_material,
	struct User_interface *user_interface,
	struct Time_keeper *time_keeper);

int DESTROY(Cad_tool)(struct Cad_tool **cad_tool_address);

int Cad_tool_pop_up_dialog(struct Cad_tool *cad_tool, struct Graphics_window *graphics_window);

int Cad_tool_pop_down_dialog(struct Cad_tool *cad_tool);

int Cad_tool_get_select_surfaces_enabled(struct Cad_tool *cad_tool);

int Cad_tool_set_select_surfaces_enabled(struct Cad_tool *cad_tool,
	int select_surfaces_enabled);

int Cad_tool_get_select_lines_enabled(struct Cad_tool *cad_tool);

int Cad_tool_set_select_lines_enabled(struct Cad_tool *cad_tool,
	int select_lines_enabled);

struct Computed_field *Cad_tool_get_command_field(
	struct Cad_tool *cad_tool);

int Cad_tool_set_command_field(struct Cad_tool *cad_tool,
	struct Computed_field *command_field);

struct Interactive_tool *Cad_tool_get_interactive_tool(
	struct Cad_tool *cad_tool);

int Cad_tool_set_execute_command(struct Cad_tool *cad_tool, 
	struct Execute_command *execute_command);

#endif /* !defined (CAD_TOOL_H) */
