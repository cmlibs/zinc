/*******************************************************************************
FILE : graphical_element.h

LAST MODIFIED : 20 July 1999

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
#endif /* !defined (GRAPHICAL_ELEMENT_H) */
