/*******************************************************************************
FILE : graphical_element.h

LAST MODIFIED : 15 February 2000

DESCRIPTION :
Graphical element group data structure.
==============================================================================*/
#if !defined (GRAPHICAL_ELEMENT_H)
#define GRAPHICAL_ELEMENT_H

#include "graphics/element_group_settings.h"

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
	struct GROUP(FE_element) *element_group,struct GROUP(FE_node) *node_group,
	struct GROUP(FE_node) *data_group);
/*******************************************************************************
LAST MODIFIED : 22 April 1999

DESCRIPTION :
Allocates memory and assigns fields for a graphical finite element group for
the given <element_group>. Its partner <node_group> must also be supplied, and
is expected to be of the same name. The rest of the application must ensure
it contains at least the nodes referenced by the elements in the element group.
Likewise, the <data_group> is expected to be supplied and of the same name.
The GT_element_group does not access the element group, but it does access the
node and data groups. It must therefore be destroyed in response to element
group manager delete messages (currently handled by a Scene), which must
precede removing the node and data groups from their respective managers.
==============================================================================*/

int DESTROY(GT_element_group)(struct GT_element_group **gt_element_group);
/*******************************************************************************
LAST MODIFIED : 24 October 1997

DESCRIPTION :
Frees the memory for <**gt_element_group> and sets <*gt_element_group> to NULL.
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

int GET_NAME(GT_element_group)(struct GT_element_group *gt_element_group,
	char **name_ptr);
/*******************************************************************************
LAST MODIFIED : 6 July 1999

DESCRIPTION :
Returns the name of the <gt_element_group>.
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

int GT_element_group_get_settings_position(struct GT_element_group
	*gt_element_group,struct GT_element_settings *settings);
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Returns the position of <settings> in <gt_element_group>.
==============================================================================*/

int GT_element_group_get_circle_discretization(
	struct GT_element_group *gt_element_group);
/*******************************************************************************
LAST MODIFIED : 3 March 1999

DESCRIPTION :
Returns the circle discretization of the gt_element_group.
==============================================================================*/

int GT_element_group_set_circle_discretization(
	struct GT_element_group *gt_element_group,int circle_discretization,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 3 March 1999

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
	struct Element_discretization *element_discretization,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 3 March 1999

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

int GT_element_group_build_graphics_objects(
	struct GT_element_group *gt_element_group,struct FE_element *changed_element,
	struct FE_node *changed_node);
/*******************************************************************************
LAST MODIFIED : 12 June 1998

DESCRIPTION :
Adds or edits a graphics object for each settings in <gt_element_group> without
a graphics object or affected by <changed_element> or <changed_node>.
==============================================================================*/

int GT_element_group_copy(struct GT_element_group *destination,
	struct GT_element_group *source);
/*******************************************************************************
LAST MODIFIED : 11 June 1998

DESCRIPTION :
Copies the GT_element_group contents from source to destination.
Pointers to graphics_objects are cleared in the destination list of settings.
???RC Not a full COPY object function: copies only general and specific
settings while element_group, node_group and other members are not copied.
Otherwise need to work out how to handle node_groups that are maintained by the
GT_element_group. Current routine is sufficient for the graphical element
attribute editor.
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

struct GROUP(FE_element) *GT_element_group_get_element_group(
	struct GT_element_group *gt_element_group);
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Returns the element_group used by <gt_element_group>.
==============================================================================*/

struct GROUP(FE_node) *GT_element_group_get_node_group(
	struct GT_element_group *gt_element_group);
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Returns the node_group used by <gt_element_group>.
==============================================================================*/

struct GROUP(FE_node) *GT_element_group_get_data_group(
	struct GT_element_group *gt_element_group);
/*******************************************************************************
LAST MODIFIED : 10 September 1998

DESCRIPTION :
Returns the data_group used by <gt_element_group>.
==============================================================================*/

int GT_element_group_list_commands(struct GT_element_group *gt_element_group,
	char *command_prefix,char *command_suffix);
/*******************************************************************************
LAST MODIFIED : 22 July 1998

DESCRIPTION :
Lists the general settings and graphics defined for <gt_element_group> - as a
set of commands that can be used to reproduce the groups appearance. The
<command_prefix> should generally contain "gfx modify g_element" while the
optional <command_suffix> may describe the scene (eg. "scene default").
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

int GT_element_group_clear_changed(struct GT_element_group *gt_element_group);
/*******************************************************************************
LAST MODIFIED : 17 June 1998

DESCRIPTION :
Clears the changed flag in <gt_element_group>. The flag can subsequently be
checked with GT_element_group_has_changed to see whether the display list for
the graphics object enclosing <gt_element_group> needs updating. Once it is
updated, this routine should be called.
==============================================================================*/

int GT_element_group_has_changed(struct GT_element_group *gt_element_group);
/*******************************************************************************
LAST MODIFIED : 17 June 1998

DESCRIPTION :
Returns true if <gt_element_group> is flagged as having been changed.
See also GT_element_group_clear_changed.
==============================================================================*/

int GT_element_group_has_embedded_field(struct GT_element_group *gt_element_group,
	struct FE_element *changed_element, struct FE_node *changed_node);
/*******************************************************************************
LAST MODIFIED : 28 April 1999

DESCRIPTION :
Returns true if <gt_element_group> contains settings which use embedded fields which
are affected by the <changed_node> or <changed_element>.
If <changed_node> and <changed_element> are both NULL then this returns true 
if the group contains any settings using embedded fields.
==============================================================================*/

int GT_element_group_computed_field_change(
	struct GT_element_group *gt_element_group,
	struct Computed_field *changed_field);
/*******************************************************************************
LAST MODIFIED : 3 May 1999

DESCRIPTION :
Tells the <gt_element_group> about the <changed_field>, where NULL means all
fields have changed. If any of the graphics objects in the settings need
rebuilding, this function takes care of that, and returns true so that the
scene knows it must invoke a scene redraw.
???RC This should be done with callbacks... get GT_element_group out of
graphics_object.c!
==============================================================================*/

int compile_GT_element_group(struct GT_element_group *gt_element_group,
	float time);
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Rebuilds the display list for graphical_element_group. 
The object is compiled at the time pointed to by <time_void>.
==============================================================================*/

int execute_GT_element_group(struct GT_element_group *gt_element_group,
	float time);
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
==============================================================================*/

int GT_element_group_clear_picked(struct GT_element_group *gt_element_group);
/*******************************************************************************
LAST MODIFIED : 14 February 2000

DESCRIPTION :
Clears all the picked objects in the <gt_element_group>.
Only parent scene is authorised to do this.
Should only be called by Scene_object_clear_picked, otherwise need callback to
inform it of change.
==============================================================================*/

int GT_element_group_clear_selected(struct GT_element_group *gt_element_group);
/*******************************************************************************
LAST MODIFIED : 14 February 2000

DESCRIPTION :
Clears all the selected objects in the <gt_element_group>.
Only parent scene is authorised to do this.
Should only be called by Scene_object_clear_selected, otherwise need callback to
inform it of change.
==============================================================================*/

int GT_element_group_pick_subobject(struct GT_element_group *gt_element_group,
	int number_of_subobject_names,int *subobject_names);
/*******************************************************************************
LAST MODIFIED : 15 February 2000

DESCRIPTION :
Finds the settings with the position/identifier given in the first position in
the <subobject_names> in <gt_element_group>, picking any objects it refers to
based on the remainder of the subobject names.
For example, if there are "node_points" in the first position, and there are
three subobject_names 1 0 5, node 5 will be added to the picked list.
==============================================================================*/

int GT_element_group_get_picked_nodes(
	struct GT_element_group *gt_element_group,struct LIST(FE_node) *node_list);
/*******************************************************************************
LAST MODIFIED : 15 February 2000

DESCRIPTION :
Ensures all the picked nodes in <gt_element_group> are in <node_list>. Does
not clear <node_list> first.
==============================================================================*/

int GT_element_group_get_selected_nodes(
	struct GT_element_group *gt_element_group,struct LIST(FE_node) *node_list);
/*******************************************************************************
LAST MODIFIED : 15 February 2000

DESCRIPTION :
Ensures all the selected nodes in <gt_element_group> are in <node_list>. Does
not clear <node_list> first.
==============================================================================*/

int GT_element_group_modify_selected_nodes(
	struct GT_element_group *gt_element_group,
	enum GT_element_group_select_modify_mode modify_mode,
	struct LIST(FE_node) *node_list);
/*******************************************************************************
LAST MODIFIED : 15 February 2000

DESCRIPTION :
Modifies the list of selected nodes in <gt_element_group> with <node_list>
according to the <modify_mode>: add, remove, replace, toggle etc.
==============================================================================*/

struct FE_node *GT_element_group_get_nearest_node(
	struct GT_element_group *gt_element_group,FE_value centre[3],
	struct GT_element_settings **settings_address,int *edit_vector);
/*******************************************************************************
LAST MODIFIED : 15 February 2000

DESCRIPTION :
Returns the picked node in <gt_element_group> closest to <centre>. Evaluates the
coordinate field for each node with every NODE_POINTS settings, as well as the
end point of the vector if an orientation_scale field is used. Note that the end
of the node vector will be chosen first if it is at the same location as the
node coordinates. The <settings> with the nearest node are returned, and the
<edit_vector> flag will be set if the end of the node vector was selected over
the node coordinates.
Note that the centre given must be in the coordinate area of <gt_element_group>,
ie. any transformations of the graphics must be inverted.
==============================================================================*/
#endif /* !defined (GRAPHICAL_ELEMENT_H) */
