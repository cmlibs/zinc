/*******************************************************************************
FILE : cmiss.h

LAST MODIFIED : 17 August 2005

DESCRIPTION :
Functions required to hook in a unemap application to cmgui.
Could be generalised to support other external applications.

These routines are implemented in command/cmiss.c. 

This should only be included in cmgui.c and command/cmiss.c
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
#if !defined (COMMAND_CMISS_UNEMAP_LINK_H)
#define COMMAND_CMISS_UNEMAP_LINK_H

#include "command/cmiss.h"

/*
Global types
------------
*/

struct Unemap_command_data;
struct System_window;
typedef struct Unemap_command_data * (*Create_unemap_command_function)(
	struct Event_dispatcher *event_dispatcher,
	struct Execute_command *execute_command,
	struct User_interface *user_interface,
#if defined (MOTIF)
	struct Node_tool *node_tool,
	struct Interactive_tool *transform_tool,
#endif /* defined (MOTIF) */
	struct LIST(GT_object) *glyph_list,
	struct Computed_field_package *computed_field_package,
	struct MANAGER(FE_basis) *basis_manager,
	struct Cmiss_region *root_cmiss_region,
	struct Cmiss_region *data_root_cmiss_region,
	struct Graphics_buffer_package *graphics_buffer_package,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_graphical_material,
	struct IO_stream_package *io_stream_package,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct MANAGER(Light) *light_manager,
	struct Light *default_light,
	struct MANAGER(Light_model) *light_model_manager,
	struct Light_model *default_light_model,
	struct MANAGER(Texture) *texture_manager,
	struct MANAGER(Scene) *scene_manager,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct FE_element_selection *element_selection,
	struct FE_node_selection *data_selection,
	struct FE_node_selection *node_selection,
	struct System_window *unemap_system_window,
	struct Time_keeper *default_time_keeper
	);
typedef int (*Execute_unemap_command_function)(struct Parse_state *state,
	void *prompt_void, void *unemap_command_data_void);
typedef int (*Destroy_unemap_command_function)(
	struct Unemap_command_data **unemap_command_data_address);

/*
Global functions
----------------
*/

int Cmiss_command_data_create_unemap(struct Cmiss_command_data *command_data,
	Create_unemap_command_function create_function,
	Execute_unemap_command_function execute_function,
	Destroy_unemap_command_function destroy_function);
/*******************************************************************************
LAST MODIFIED : 17 August 2005

DESCRIPTION :
Registers the <create_function>, <execute_function> and <destroy_function> 
with the <command_data>.
This allows unemap to operate integrated into cmgui.  The <create_function> will
be called immediately by this function, and the <destroy_function> handle kept
to be called when the Cmiss_command_data is being destroyed.
==============================================================================*/

#endif /* !defined (COMMAND_CMISS_UNEMAP_LINK_H) */
