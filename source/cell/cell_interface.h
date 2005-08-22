/*******************************************************************************
FILE : cell_interface.h

LAST MODIFIED : 07 May 2001

DESCRIPTION :
The Cell Interface.
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
#if !defined (CELL_INTERFACE_H)
#define CELL_INTERFACE_H

#include "general/object.h"
#include "general/debug.h"
#include "time/time.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

#include "graphics/light_model.h"
#include "interaction/interactive_tool.h"
#include "selection/any_object_selection.h"

#define ROOT_ELEMENT_ID "ROOT_ELEMENT_UNIQUE_IDENTIFIER_13579"

#define WRITE_INDENT( indent_level ) \
{ \
  int write_indent_counter = 0; \
  while(write_indent_counter < indent_level) \
  { \
    display_message(INFORMATION_MESSAGE," "); \
    write_indent_counter++; \
  } \
}

/*
Module objects
--------------
*/
struct Cell_interface;
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
The Cell Interface main structure.
==============================================================================*/

/*
Global functions
----------------
*/
struct Cell_interface *CREATE(Cell_interface)(
	struct Any_object_selection *any_object_selection,
  struct Colour *background_colour,
  struct Graphical_material *default_graphical_material,
  struct Light *default_light,
  struct Light_model *default_light_model,
  struct Scene *default_scene,
  struct Spectrum *default_spectrum,
  struct Time_keeper *time_keeper,
  struct LIST(GT_object) *graphics_object_list,
  struct LIST(GT_object) *glyph_list,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
  struct MANAGER(Light) *light_manager,
  struct MANAGER(Light_model) *light_model_manager,
  struct MANAGER(Graphical_material) *graphical_material_manager,
  struct MANAGER(Scene) *scene_manager,
  struct MANAGER(Spectrum) *spectrum_manager,
  struct MANAGER(Texture) *texture_manager,
  struct User_interface *user_interface,
  XtCallbackProc exit_callback
#if defined (CELL_DISTRIBUTED)
  ,struct Element_point_ranges_selection *element_point_ranges_selection,
  struct Computed_field_package *computed_field_package,
  struct MANAGER(FE_element) *element_manager,
  struct MANAGER(GROUP(FE_element)) *element_group_manager,
  struct MANAGER(FE_field) *fe_field_manager
#endif /* defined (CELL_DISTRIBUTED) */
  );
/*******************************************************************************
LAST MODIFIED : 16 January 2001

DESCRIPTION :
Creates the Cell interface object.
==============================================================================*/
int DESTROY(Cell_interface)(struct Cell_interface **cell_interface_address);
/*******************************************************************************
LAST MODIFIED : 26 June 2000

DESCRIPTION :
Destroys the Cell_interface object.
==============================================================================*/
int Cell_interface_close_model(struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 19 October 2000

DESCRIPTION :
Destroys all the current components and variables in the interface.
==============================================================================*/
void Cell_interface_terminate_XMLParser(struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Terminates the XML Parser. Need to do this so that the parser is only ever
initialised once and terminated once for the enitre life of this application
instance. Required because the Xerces library can only be initialised and
terminated once!!
==============================================================================*/
int Cell_interface_read_model(struct Cell_interface *cell_interface,
  char *filename);
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Reads in a Cell model. If no <filename> is given, then prompts for a file name.
==============================================================================*/
int Cell_interface_write_model(struct Cell_interface *cell_interface,
  char *filename);
/*******************************************************************************
LAST MODIFIED : 13 November 2000

DESCRIPTION :
Writes out a Cell model.
==============================================================================*/
int Cell_interface_write_model_to_ipcell_file(
  struct Cell_interface *cell_interface,char *filename);
/*******************************************************************************
LAST MODIFIED : 11 January 2001

DESCRIPTION :
Writes out a Cell model to a ipcell file
==============================================================================*/
int Cell_interface_list_components(
  struct Cell_interface *cell_interface,int full);
/*******************************************************************************
LAST MODIFIED : 09 July 2000

DESCRIPTION :
Lists out the current set of cell components. If <full> is not 0, then a full
listing is given, otherwise simply gives a list of names.
==============================================================================*/
int Cell_interface_list_XMLParser_properties(
  struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Lists out the current set of XML parser properties
==============================================================================*/
int *Cell_interface_get_XMLParser_properties(
  struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 02 July 2000

DESCRIPTION :
Gets the current set of XML parser properties
==============================================================================*/
int Cell_interface_set_XMLParser_properties(
  struct Cell_interface *cell_interface,int *properties);
/*******************************************************************************
LAST MODIFIED : 02 July 2000

DESCRIPTION :
Sets the current set of XML parser properties
==============================================================================*/
int Cell_interface_list_copy_tags(struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Lists out the current set of copy tags
==============================================================================*/
char **Cell_interface_get_copy_tags(
  struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 09 July 2000

DESCRIPTION :
Gets a copy of the current set of copy tags.
==============================================================================*/
int Cell_interface_set_copy_tags(
  struct Cell_interface *cell_interface,char **copy_tags);
/*******************************************************************************
LAST MODIFIED : 09 July 2000

DESCRIPTION :
Sets the current set of copy tags.
==============================================================================*/
int Cell_interface_list_ref_tags(struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
Lists out the current set of ref tags
==============================================================================*/
int Cell_interface_list_hierarchy(struct Cell_interface *cell_interface,
  int full,char *name);
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Lists out the hierarchy described by the component with the given <name>, or
the root level component if no name is given.
==============================================================================*/
int Cell_interface_calculate_model(struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 26 October 2000

DESCRIPTION :
Calculates the current cell model.
==============================================================================*/
int Cell_interface_pop_up_calculate_dialog(
  struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 15 November 2000

DESCRIPTION :
Brings up the calculate dialog.
==============================================================================*/
int Cell_interface_pop_up_unemap(struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 01 November 2000

DESCRIPTION :
Pops up the UnEmap windows.
==============================================================================*/
int Cell_interface_clear_unemap(struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 05 November 2000

DESCRIPTION :
Clears the UnEmap windows.
==============================================================================*/
int Cell_interface_set_save_signals(struct Cell_interface *cell_interface,
  int save);
/*******************************************************************************
LAST MODIFIED : 05 November 2000

DESCRIPTION :
Sets the value of the save signals toggle in the UnEmap interface object.
==============================================================================*/
int Cell_interface_list_root_component(struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 06 November 2000

DESCRIPTION :
Lists out the root component from the cell interface.
==============================================================================*/
int Cell_interface_edit_component_variables(
  struct Cell_interface *cell_interface,char *name,int reset);
/*******************************************************************************
LAST MODIFIED : 07 November 2000

DESCRIPTION :
Pops up the cell varaible editing dialog with the variables from the component
given by <name>. If <name> is NULL, then the root component is used. <reset>
specifies whether the variable editing dialog is cleared before adding the
component <name> to the dialog.
==============================================================================*/
int Cell_interface_pop_up(struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 07 November 2000

DESCRIPTION :
Pops up the cell interface
==============================================================================*/
int Cell_interface_pop_up_distributed_editing_dialog(
  struct Cell_interface *cell_interface,Widget activation);
/*******************************************************************************
LAST MODIFIED : 12 January 2001

DESCRIPTION :
Pops up the distributed editing dialog. <activation> is the toggle button
widget which activated the dialog.
==============================================================================*/
int Cell_interface_destroy_distributed_editing_dialog(
  struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 12 January 2001

DESCRIPTION :
Destroys the distributed editing dialog.
==============================================================================*/
struct LIST(Cell_variable) *Cell_interface_get_variable_list(
  struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
Returns the <cell_interface>'s variable list.
==============================================================================*/
int Cell_interface_close(struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 22 January 2001

DESCRIPTION :
Closes all the Cell interface's.
==============================================================================*/
int Cell_interface_pop_up_export_dialog(struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 31 January 2001

DESCRIPTION :
Brings up the export (to CMISS) dialog.
==============================================================================*/
int Cell_interface_export_to_ipcell(struct Cell_interface *cell_interface,
  char *filename);
/*******************************************************************************
LAST MODIFIED : 02 February 2001

DESCRIPTION :
Exports an IPCELL file
==============================================================================*/
int Cell_interface_export_to_ipmatc(struct Cell_interface *cell_interface,
  char *filename,void *element_group_void,void *grid_field_void);
/*******************************************************************************
LAST MODIFIED : 02 February 2001

DESCRIPTION :
Exports an IPMATC file
==============================================================================*/
int Cell_interface_list_variables(struct Cell_interface *cell_interface,
  char *component_name,char *variable_name,int full);
/*******************************************************************************
LAST MODIFIED : 03 April 2001

DESCRIPTION :
If found in the given <component_name> component, variable <variable_name> will
be listed out.
==============================================================================*/
int Cell_interface_set_variable_value_from_string(
  struct Cell_interface *cell_interface,char *component_name,
  char *variable_name,char *value_string);
/*******************************************************************************
LAST MODIFIED : 03 April 2001

DESCRIPTION :
If a <component_name> and a <variable_name> are specified and the given variable
is found in the variable list for the given component, the variable's value is
set to the given <value_string>. If no <component_name> is specified, the root
component is assumed.
==============================================================================*/
int Cell_interface_list_calculate(struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Lists out the current calculate object.
==============================================================================*/
int Cell_interface_set_calculate(struct Cell_interface *cell_interface,
  float Tstart,float Tend,float dT,float tabT,char *model_routine_name,
  char *model_dso_name,char *intg_routine_name,char *intg_dso_name,
  char *data_file_name);
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Sets the values of the <cell_interface>'s calculate objec to those specified.
==============================================================================*/
float Cell_interface_get_start_time(struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Returns the integration start time as a float.
==============================================================================*/
float Cell_interface_get_end_time(struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Returns the integration end time as a float.
==============================================================================*/
float Cell_interface_get_dt(struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Returns the integration time step as a float.
==============================================================================*/
float Cell_interface_get_tabt(struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Returns the integration tabulation interval as a float.
==============================================================================*/
char *Cell_interface_get_model_routine_name(
  struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Returns the model routine name.
==============================================================================*/
char *Cell_interface_get_model_dso_name(struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Returns the model DSO name.
==============================================================================*/
char *Cell_interface_get_intg_routine_name(
  struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Returns the integrator routine name.
==============================================================================*/
char *Cell_interface_get_intg_dso_name(struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Returns the integrator DSO name.
==============================================================================*/
char *Cell_interface_get_data_file_name(struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 04 April 2001

DESCRIPTION :
Returns the data file name.
==============================================================================*/
Widget Cell_interface_get_shell(struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 07 May 2001

DESCRIPTION :
Returns the shell widget for the cell window.
==============================================================================*/
struct User_interface *Cell_interface_get_user_interface(
  struct Cell_interface *cell_interface);
/*******************************************************************************
LAST MODIFIED : 07 May 2001

DESCRIPTION :
Returns a pointer to the main user interface object.
==============================================================================*/

#endif /* !defined (CELL_INTERFACE_H) */
