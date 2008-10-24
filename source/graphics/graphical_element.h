/*******************************************************************************
FILE : graphical_element.h

LAST MODIFIED : 14 March 2003

DESCRIPTION :
Graphical element group data structure.
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
#if !defined (GRAPHICAL_ELEMENT_H)
#define GRAPHICAL_ELEMENT_H

#include "finite_element/finite_element.h"
#include "graphics/element_group_settings.h"
#include "region/cmiss_region.h"
#include "selection/element_point_ranges_selection.h"
#include "selection/element_selection.h"
#include "selection/node_selection.h"

/*
Global types
------------
*/

struct GT_element_group;
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Private structure for maintaining a graphical rendition of an element group.
???RC This probably should not be a GT_object primitive type.
???RC Move to a module by itself?
==============================================================================*/

typedef int(*GT_element_group_callback)(struct GT_element_group *gt_element_group,
	void *user_data);

enum GT_element_group_select_modify_mode
/*******************************************************************************
LAST MODIFIED : 15 February 2000

DESCRIPTION :
Controls how a list of selected objects is modified by a new list.
==============================================================================*/
{
	GT_ELEMENT_GROUP_SELECT_ADD,
	GT_ELEMENT_GROUP_SELECT_REMOVE,
	GT_ELEMENT_GROUP_SELECT_REPLACE,
	GT_ELEMENT_GROUP_SELECT_TOGGLE
};

/*
Global functions
----------------
*/

PROTOTYPE_OBJECT_FUNCTIONS(GT_element_group);

struct GT_element_group *CREATE(GT_element_group)(
	struct Cmiss_region *cmiss_region,
	struct Cmiss_region *data_cmiss_region,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct FE_element_selection *element_selection,
	struct FE_node_selection *node_selection,
	struct FE_node_selection *data_selection);
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Allocates memory and assigns fields for a graphical finite element group for
the given <cmiss_region> and <data_cmiss_region>. Neither region is accessed;
instead the Scene is required to destroy GT_element_group if either region is
destroyed.
If supplied, callbacks are requested from the <element_selection> and
<node_selection> to enable automatic highlighting of selected graphics.
==============================================================================*/

struct GT_element_group *create_editor_copy_GT_element_group(
	struct GT_element_group *existing_gt_element_group);
/*******************************************************************************
LAST MODIFIED : 6 April 2000

DESCRIPTION :
Creates a GT_element_group that is a copy of <existing_gt_element_group> -
WITHOUT copying graphics objects, and WITHOUT manager and selection callbacks.
==============================================================================*/

int DESTROY(GT_element_group)(struct GT_element_group **gt_element_group);
/*******************************************************************************
LAST MODIFIED : 24 October 1997

DESCRIPTION :
Frees the memory for <**gt_element_group> and sets <*gt_element_group> to NULL.
==============================================================================*/

int GT_element_groups_match(struct GT_element_group *gt_element_group1,
	struct GT_element_group *gt_element_group2);
/*******************************************************************************
LAST MODIFIED : 14 November 2001

DESCRIPTION :
Returns true if <gt_element_group1> and <gt_element_group2> match in every way.
==============================================================================*/

int GT_element_group_begin_cache(struct GT_element_group *gt_element_group);
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
Call before making several changes to the gt_element_group so only a single
change message is sent. Call GT_element_group_end_cache at the end of the
changes.
==============================================================================*/

int GT_element_group_end_cache(struct GT_element_group *gt_element_group);
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
Call after making changes preceded by a call to GT_element_group_begin_cache to
enable a final message to be sent to clients.
==============================================================================*/

int GT_element_group_add_callback(struct GT_element_group *GT_element_group, 
	GT_element_group_callback callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 6 July 1999

DESCRIPTION :
Adds a callback routine which is called whenever a GT_element_group is aware of
changes.
==============================================================================*/

int GT_element_group_remove_callback(struct GT_element_group *GT_element_group,
	GT_element_group_callback callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 6 July 1999

DESCRIPTION :
Removes a callback which was added previously
==============================================================================*/

int GT_element_group_add_settings(struct GT_element_group *gt_element_group,
	struct GT_element_settings *settings,int position);
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Adds the <settings> to <gt_element_group> at the given <position>, where 1 is
the top of the list (rendered first), and values less than 1 or greater than the
last position in the list cause the settings to be added at its end, with a
position one greater than the last.
==============================================================================*/

int GT_element_group_remove_settings(struct GT_element_group *gt_element_group,
	struct GT_element_settings *settings);
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Removes the <settings> from <gt_element_group> and decrements the position
of all subsequent settings.
==============================================================================*/

int GT_element_group_modify_settings(struct GT_element_group *gt_element_group,
	struct GT_element_settings *settings,
	struct GT_element_settings *new_settings);
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Changes the contents of <settings> to match <new_settings>, with no change in
position in <gt_element_group>.
==============================================================================*/

int GT_element_group_get_settings_position(
	struct GT_element_group *gt_element_group,
	struct GT_element_settings *settings);
/*******************************************************************************
LAST MODIFIED : 13 November 2001

DESCRIPTION :
Returns the position of <settings> in <gt_element_group>.
==============================================================================*/

int GT_element_group_get_number_of_settings(
	struct GT_element_group *gt_element_group);
/*******************************************************************************
LAST MODIFIED : 13 November 2001

DESCRIPTION :
Returns the number of settings in <gt_element_group>.
==============================================================================*/

int GT_element_group_get_circle_discretization(
	struct GT_element_group *gt_element_group);
/*******************************************************************************
LAST MODIFIED : 3 March 1999

DESCRIPTION :
Returns the circle discretization of the gt_element_group.
==============================================================================*/

int GT_element_group_set_circle_discretization(
	struct GT_element_group *gt_element_group, int circle_discretization);
/*******************************************************************************
LAST MODIFIED : 28 September 2005

DESCRIPTION :
Sets the circle discretization of the gt_element_group.
==============================================================================*/

struct Computed_field *GT_element_group_get_default_coordinate_field(
	struct GT_element_group *gt_element_group);
/*******************************************************************************
LAST MODIFIED : 16 February 1999

DESCRIPTION :
Returns the default coordinate field of the <gt_element_group>.
==============================================================================*/

int GT_element_group_set_default_coordinate_field(
	struct GT_element_group *gt_element_group,
	struct Computed_field *default_coordinate_field);
/*******************************************************************************
LAST MODIFIED : 16 February 1999

DESCRIPTION :
Sets the <default_coordinate_field> used by <gt_element_group>. Settings without
a specific coordinate field will use this one.
==============================================================================*/

int GT_element_group_get_element_discretization(
	struct GT_element_group *gt_element_group,
	struct Element_discretization *element_discretization);
/*******************************************************************************
LAST MODIFIED : 3 March 1999

DESCRIPTION :
Returns the element discretization of the gt_element_group.
==============================================================================*/

int GT_element_group_set_element_discretization(
	struct GT_element_group *gt_element_group,
	struct Element_discretization *element_discretization);
/*******************************************************************************
LAST MODIFIED : 28 September 2005

DESCRIPTION :
Sets the element discretization of the gt_element_group.
==============================================================================*/

struct FE_field *GT_element_group_get_native_discretization_field(
	struct GT_element_group *gt_element_group);
/*******************************************************************************
LAST MODIFIED : 3 March 1999

DESCRIPTION :
Returns the default coordinate field of the <gt_element_group>.
==============================================================================*/

int GT_element_group_set_native_discretization_field(
	struct GT_element_group *gt_element_group,
	struct FE_field *native_discretization_field);
/*******************************************************************************
LAST MODIFIED : 3 March 1999

DESCRIPTION :
Sets the <native_discretization_field> used by <gt_element_group>. If the field
is not NULL and is element-based in a given element, its native discretization
is used in preference to the global element_discretization.
==============================================================================*/

int GT_element_group_copy(struct GT_element_group *destination,
	struct GT_element_group *source);
/*******************************************************************************
LAST MODIFIED : 21 March 2000

DESCRIPTION :
Copies the GT_element_group contents from source to destination.
Pointers to graphics_objects are cleared in the destination list of settings.
NOTES:
- not a full copy; does not copy groups, selection etc. Use copy_create for
this task so that callbacks can be set up for these.
- does not copy graphics objects to settings in destination.
==============================================================================*/

int GT_element_group_modify(struct GT_element_group *destination,
	struct GT_element_group *source);
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Copies the GT_element_group contents from source to destination, keeping any
graphics objects from the destination that will not change with the new settings
from source. Used to apply the changed GT_element_group from the editor to the
actual GT_element_group.
==============================================================================*/

struct Cmiss_region *GT_element_group_get_Cmiss_region(
	struct GT_element_group *gt_element_group);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Returns the Cmiss_region used by <gt_element_group>.
==============================================================================*/

struct Cmiss_region *GT_element_group_get_data_Cmiss_region(
	struct GT_element_group *gt_element_group);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Returns the data_Cmiss_region used by <gt_element_group>.
==============================================================================*/

int GT_element_group_list_commands(struct GT_element_group *gt_element_group,
	 char *command_prefix,char *command_suffix);
/*******************************************************************************
LAST MODIFIED : 16 August 2007

DESCRIPTION :
Lists the general settings and graphics defined for <gt_element_group> - as a
set of commands that can be used to reproduce the groups appearance. The
<command_prefix> should generally contain "gfx modify g_element" while the
optional <command_suffix> may describe the scene (eg. "scene default").
Note the command prefix is expected to contain the name of the region.
==============================================================================*/

int GT_element_group_write_commands_to_comfile(struct GT_element_group *gt_element_group,
	 char *command_prefix,char *command_suffix);
/*******************************************************************************
LAST MODIFIED : 16 August 2007

DESCRIPTION:
Similar to the GT_element_group_list_commands, but pass a class to
GT_element_group_process_list_or_write_window_commands which will
write commadns into a comfile.
==============================================================================*/

int GT_element_group_list_contents(struct GT_element_group *gt_element_group);
/*******************************************************************************
LAST MODIFIED : 22 July 1998

DESCRIPTION :
Lists the general settings and graphics defined for <gt_element_group>.
==============================================================================*/

struct GT_element_settings *get_settings_at_position_in_GT_element_group(
	struct GT_element_group *gt_element_group,int position);
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Wrapper for accessing the list of settings in <gt_element_group>.
==============================================================================*/

struct GT_element_settings *first_settings_in_GT_element_group_that(
	struct GT_element_group *gt_element_group,
	LIST_CONDITIONAL_FUNCTION(GT_element_settings) *conditional_function,
	void *data);
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Wrapper for accessing the list of settings in <gt_element_group>.
==============================================================================*/

int for_each_settings_in_GT_element_group(
	struct GT_element_group *gt_element_group,
	LIST_ITERATOR_FUNCTION(GT_element_settings) *iterator_function,void *data);
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Wrapper for accessing the list of settings in <gt_element_group>.
==============================================================================*/

int GT_element_group_time_changed(struct GT_element_group *gt_element_group);
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
Invalidate any components of a GT_element group that depend on time
==============================================================================*/

int GT_element_group_has_embedded_field(
	struct GT_element_group *gt_element_group,
	struct LIST(FE_element) *changed_element_list,
	struct LIST(FE_node) *changed_node_list);
/*******************************************************************************
LAST MODIFIED : 25 May 2001

DESCRIPTION :
Returns true if <gt_element_group> contains settings which use embedded fields
which are affected by the <changed_node> or <changed_element>.
If <changed_element_list> and <changed_node_list> are both NULL then this
returns true if the group contains any settings using embedded fields.
==============================================================================*/

int GT_element_group_has_multiple_times(
	struct GT_element_group *gt_element_group);
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
Returns true if <gt_element_group> contains settings which depend on time.
==============================================================================*/

int GT_element_group_Graphical_material_change(
	struct GT_element_group *gt_element_group,
	struct LIST(Graphical_material) *changed_material_list);
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
NOTE: TEMPORARY UNTIL GRAPHICAL ELEMENTS RECEIVE THEIR OWN MATERIAL MESSAGES.
If any of the settings in <gt_element_group> use materials with spectrums in the
<changed_material_list>, clear their graphics objects and call
GT_element_group_changed.
==============================================================================*/

int GT_element_group_Spectrum_change(struct GT_element_group *gt_element_group,
	struct LIST(Spectrum) *changed_spectrum_list);
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
NOTE: TEMPORARY UNTIL GRAPHICAL ELEMENTS RECEIVE THEIR OWN SPECTRUM MESSAGES.
If any of the settings in <gt_element_group> use spectrums in the
<changed_spectrum_list>, clear their graphics objects and call
GT_element_group_changed.
==============================================================================*/

#endif /* !defined (GRAPHICAL_ELEMENT_H) */
