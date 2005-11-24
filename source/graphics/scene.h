/*******************************************************************************
FILE : scene.h

LAST MODIFIED : 4 December 2003

DESCRIPTION :
Structure for storing the collections of objects that make up a 3-D graphical
model - lights, materials, primitives, etc.
Also contains interface routines for having these converted to display lists,
and for these to be assembled into a single display list of the whole scene.

HISTORY :
November 1997. Created from Scene description part of Drawing.
December 1997. Created MANAGER(Scene).
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
#if !defined (SCENE_H)
#define SCENE_H

#include "general/any_object.h"
#include "general/callback.h"
#include "general/enumerator.h"
#include "graphics/element_point_ranges.h"
#include "graphics/graphical_element.h"
#include "graphics/graphics_library.h"
#include "graphics/graphics_object.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "graphics/light.h"
#include "graphics/material.h"
#include "graphics/spectrum.h"
#include "interaction/interaction_volume.h"
#include "region/cmiss_region.h"
#include "selection/element_point_ranges_selection.h"
#include "selection/element_selection.h"
#include "selection/node_selection.h"
#include "time/time_keeper.h"
/* #include "graphics/texture.h" */

/*
Global constants
----------------
*/
#define SCENE_INPUT_MODIFY_SHIFT   1
#define SCENE_INPUT_MODIFY_CONTROL 2
#define SCENE_INPUT_MODIFY_ALT     4

/*
Global types
------------
*/
enum Scene_graphical_element_mode
/*******************************************************************************
LAST MODIFIED : 16 March 2001

DESCRIPTION :
Controls whether graphical element groups are placed in the scene and if so,
how they are rendered by default.
Note: the first value will be 0 by the ANSI standard, with each subsequent entry
incremented by 1. This pattern is expected by the ENUMERATOR macros.
Must ensure the ENUMERATOR_STRING function returns a string for each value here.
==============================================================================*/
{
	GRAPHICAL_ELEMENT_EMPTY,
	GRAPHICAL_ELEMENT_INVISIBLE,
	GRAPHICAL_ELEMENT_LINES,
	GRAPHICAL_ELEMENT_MANUAL,
	GRAPHICAL_ELEMENT_NONE
}; /* enum Scene_graphical_element_mode */

enum Scene_object_type
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
==============================================================================*/
{
	SCENE_OBJECT_TYPE_INVALID,
	SCENE_OBJECT_GRAPHICS_OBJECT,
	SCENE_OBJECT_GRAPHICAL_ELEMENT_GROUP,
	SCENE_OBJECT_SCENE
}; /* enum Scene_object_type */

enum Scene_change_status
/*******************************************************************************
LAST MODIFIED : 12 July 2000

DESCRIPTION :
Describes the nature of the change message received by scene clients.
==============================================================================*/
{
	SCENE_NO_CHANGE,
	SCENE_CHANGE,
	SCENE_FAST_CHANGE
};

struct Scene_object;
/*******************************************************************************
LAST MODIFIED : 19 November 1997

DESCRIPTION :
Scenes store a list of these wrappers to GT_objects so that the same
graphics object may have different visibility on different scenes.
Members of this object are private.
==============================================================================*/

DECLARE_LIST_TYPES(Scene_object);

struct Scene_picked_object;
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Describes a single picked item in a format compatible with objects in our
display hierarchy.
The contents of this object are private.
==============================================================================*/

DECLARE_LIST_TYPES(Scene_picked_object);

struct Scene;
/*******************************************************************************
LAST MODIFIED : 8 December 1997

DESCRIPTION :
Stores the collections of objects that make up a 3-D graphical model.
The contents of this object are private.
==============================================================================*/

DECLARE_LIST_TYPES(Scene);

DECLARE_MANAGER_TYPES(Scene);

struct Modify_scene_data
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Structure to pass to modify_Scene.
==============================================================================*/
{
	struct MANAGER(Light) *light_manager;
	struct MANAGER(Scene) *scene_manager;
	struct Scene *default_scene;
	/* following used for enabling GFEs */
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Cmiss_region *root_region;
	struct Cmiss_region *data_root_region;
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct FE_element_selection *element_selection;
	struct FE_node_selection *data_selection,*node_selection;
	struct User_interface *user_interface;
}; /* struct Modify_scene_data */

enum Scene_input_type
{
	SCENE_BUTTON_PRESS,
	SCENE_MOTION_NOTIFY,
	SCENE_BUTTON_RELEASE
}; /* enum Scene_input_type */

struct Scene_input_callback_data
/*******************************************************************************
LAST MODIFIED : 18 May 1998

DESCRIPTION :
Package of data to pass to clients of scene input callbacks, describing the
type of the [mouse] event, the line in space under the mouse at the time of the
event as defined by points near and far, the button number and shift-key down
state for button press and release events, and the list of picked objects. The
viewing direction is also necessary for determining the editing response with
non-parallel projections.
==============================================================================*/
{
	double viewx,viewy,viewz,nearx,neary,nearz,farx,fary,farz;
	enum Scene_input_type input_type;
	int button_number;
	/* flags indicating the state of the shift, control and alt keys -
		to use logical OR input_modifier with SCENE_INPUT_MODIFY_SHIFT etc. */
	int input_modifier;
	struct LIST(Scene_picked_object) *picked_object_list;
}; /* Scene_input_callback_data */

typedef void Scene_input_callback_procedure(struct Scene *,void *, \
	struct Scene_input_callback_data *);

struct Scene_input_callback
/*******************************************************************************
LAST MODIFIED : 26 February 1998

DESCRIPTION :
Contains all information necessary for an input callback from the scene.
==============================================================================*/
{
	Scene_input_callback_procedure *procedure;
	void *data;
}; /* struct Scene_input_callback */

DECLARE_CMISS_CALLBACK_TYPES(Scene_object_transformation, struct Scene_object *, \
	gtMatrix *);

struct Scene_get_data_range_for_spectrum_data
{
	struct Spectrum *spectrum;
	struct Graphics_object_data_range_struct range;
};

/*
Global functions
----------------
*/

PROTOTYPE_ENUMERATOR_FUNCTIONS(Scene_graphical_element_mode);

PROTOTYPE_OBJECT_FUNCTIONS(Scene_object);
PROTOTYPE_LIST_FUNCTIONS(Scene_object);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Scene_object, \
	position,int);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Scene_object);

int Scene_object_is_fast_changing(struct Scene_object *scene_object,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 12 July 2000

DESCRIPTION :
Returns true if the fast_changing flag of <scene_object> is set.
==============================================================================*/

int Scene_object_get_fast_changing(struct Scene_object *scene_object);
/*******************************************************************************
LAST MODIFIED : 12 July 2000

DESCRIPTION :
Returns the fast_changing flag of <scene_object>.
==============================================================================*/

int Scene_object_set_fast_changing(struct Scene_object *scene_object,
	int fast_changing);
/*******************************************************************************
LAST MODIFIED : 12 July 2000

DESCRIPTION :
Sets the fast_changing flag of <scene_object>.
==============================================================================*/

enum GT_visibility_type Scene_object_get_visibility(
	struct Scene_object *scene_object);
/*******************************************************************************
LAST MODIFIED : 9 December 1997

DESCRIPTION :
Returns the visibility of <scene_object>.
==============================================================================*/

int Scene_object_set_visibility(struct Scene_object *scene_object,
	enum GT_visibility_type visibility);
/*******************************************************************************
LAST MODIFIED : 19 November 1997

DESCRIPTION :
Sets the visibility of <scene_object>.
==============================================================================*/

int Scene_object_add_transformation_callback(struct Scene_object *scene_object,
	CMISS_CALLBACK_FUNCTION(Scene_object_transformation) *function, void *user_data);
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
Adds a callback to <scene_object> for when its transformation is changed.
<function> should have 3 arguments: struct Scene_object *, a gtMatrix * and the
given void *user_data.
==============================================================================*/

int Scene_object_remove_transformation_callback(
	struct Scene_object *scene_object,
	CMISS_CALLBACK_FUNCTION(Scene_object_transformation) *function, void *user_data);
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
Removes the transformation callback calling <function> with <user_data> from
<scene_object>.
==============================================================================*/

int Scene_object_has_transformation(struct Scene_object *scene_object);
/*******************************************************************************
LAST MODIFIED : 8 October 1998

DESCRIPTION :
Returns 1 if the <scene_object> has a nonidentity transformation matrix
==============================================================================*/

int Scene_object_get_transformation(struct Scene_object *scene_object,
	gtMatrix *transformation);
/*******************************************************************************
LAST MODIFIED : 8 October 1998

DESCRIPTION :
Returns the transformation of <scene_object>.
==============================================================================*/

int Scene_object_set_transformation(struct Scene_object *scene_object,
	gtMatrix *transformation);
/*******************************************************************************
LAST MODIFIED : 8 October 1998

DESCRIPTION :
Sets the transformation of <scene_object>.
==============================================================================*/

int Scene_object_iterator_set_visibility(struct Scene_object *scene_object,
	void *visibility_void);
/*******************************************************************************
LAST MODIFIED : 19 November 1997

DESCRIPTION :
List iterator function for setting the visibility of <scene_object>.
Note: second parameter should be passed as (void *)visibility, where
visibility is of type enum GT_visibility_type.
==============================================================================*/

int Scene_object_has_name(struct Scene_object *scene_object,
	void *name_void);
/*******************************************************************************
LAST MODIFIED : 12 October 1998

DESCRIPTION :
Scene_object iterator function returning true if <scene_object> has the 
specified <name>.
==============================================================================*/

struct GT_object *Scene_object_get_gt_object(
	struct Scene_object *scene_object);
/*******************************************************************************
LAST MODIFIED : 19 November 1997

DESCRIPTION :
Returns the GT_object referenced by <scene_object>.
==============================================================================*/

int Scene_object_set_gt_object(struct Scene_object *scene_object,
	struct GT_object *gt_object);
/*******************************************************************************
LAST MODIFIED : 16 October 2001

DESCRIPTION :
Changes the GT_object referenced by <scene_object>.
==============================================================================*/

int Scene_object_has_gt_object(struct Scene_object *scene_object,
	void *gt_object_void);
/*******************************************************************************
LAST MODIFIED : 7 July 1999

DESCRIPTION :
Scene_object iterator function returning true if <scene_object> is of type
SCENE_OBJECT_GRAPHICS_OBJECT and references <gt_object>.  If <gt_object_void>
is NULL then returns true if <scene_object> contains any graphics_object.
==============================================================================*/

int Scene_object_has_child_scene(struct Scene_object *scene_object,
	void *child_scene_void);
/*******************************************************************************
LAST MODIFIED : 20 November 1998

DESCRIPTION :
Scene_object iterator function returning true if <scene_object> references
<child_scene> (or any child scene if NULL).
==============================================================================*/

struct Scene *Scene_object_get_parent_scene(struct Scene_object *scene_object);
/*******************************************************************************
LAST MODIFIED : 29 October 2001

DESCRIPTION :
Returns the <scene> that the <scene_object> is in.
==============================================================================*/

struct Any_object *Scene_object_get_represented_object(
	struct Scene_object *scene_object);
/*******************************************************************************
LAST MODIFIED : 23 August 2000

DESCRIPTION :
Returns the global object that <scene_object> represents, if any.
==============================================================================*/

int Scene_object_set_represented_object(struct Scene_object *scene_object,
	struct Any_object *represented_object);
/*******************************************************************************
LAST MODIFIED : 23 August 2000

DESCRIPTION :
Sets the global object that <scene_object> will represent, if any.
==============================================================================*/

double Scene_object_get_time(struct Scene_object *scene_object);
/*******************************************************************************
LAST MODIFIED : 5 October 1998

DESCRIPTION :
Returns the actual time referenced by <scene_object>.
==============================================================================*/

int Scene_object_has_time(struct Scene_object *scene_object);
/*******************************************************************************
LAST MODIFIED : 5 October 1998

DESCRIPTION :
Returns 1 if the Scene object has a time dependence, 0 if not
==============================================================================*/

struct Time_object *Scene_object_get_time_object(struct Scene_object *scene_object);
/*******************************************************************************
LAST MODIFIED : 5 October 1998

DESCRIPTION :
Returns the Time_object object referenced by <scene_object>.
==============================================================================*/

int Scene_object_set_time_object(struct Scene_object *scene_object,
	struct Time_object *time);
/*******************************************************************************
LAST MODIFIED : 5 October 1998

DESCRIPTION :
Changes the Time_object object referenced by <scene_object>.
==============================================================================*/

int Scene_object_has_Cmiss_region(struct Scene_object *scene_object,
	void *cmiss_region_void);
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Scene_object iterator function returning true if <scene_object> contains a
graphical element group for the given <cmiss_region>.
==============================================================================*/

int Scene_object_has_data_Cmiss_region(struct Scene_object *scene_object,
	void *data_cmiss_region_void);
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Scene_object iterator function returning true if <scene_object> contains a
g_ELEMENT_GROUP gt_object referencing the given <data_cmiss_region>.
==============================================================================*/

int Scene_object_has_graphical_element_group(struct Scene_object *scene_object,
	void *gt_element_group_void);
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Scene_object iterator function returning true if <scene_object> contains a
gt_ELEMENT_GROUP which matches <gt_element_group_void>.  If <gt_element_group_void>
is NULL the function returns true if the scene_object contains any gt_element_group.
==============================================================================*/

struct GT_element_group *Scene_object_get_graphical_element_group(
	struct Scene_object *scene_object);
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Returns the GT_element_group referenced by <scene_object>.
==============================================================================*/

int Scene_object_get_range(struct Scene_object *scene_object,
	void *graphics_object_range_void);
/*******************************************************************************
LAST MODIFIED : 19 November 1997

DESCRIPTION :
Scene_object list iterator function. If <scene_object> is visible, expands
the <graphics_object_range> to include the range of the linked list of
graphics objects in scene_object.
==============================================================================*/

int Scene_object_get_time_range(struct Scene_object *scene_object,
	void *graphics_object_time_range_void);
/*******************************************************************************
LAST MODIFIED : 19 November 1997

DESCRIPTION :
Scene_object list iterator function. Enlarges the minimum and maximum time
ranges by those of the graphics_objects contained in the <scene_object>.
==============================================================================*/

enum Scene_object_type Scene_object_get_type(struct Scene_object *scene_object);
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Returns the Scene_object_type of <scene_object>.
==============================================================================*/

int list_Scene_object(struct Scene_object *scene_object,void *dummy);
/*******************************************************************************
LAST MODIFIED : 26 April 1999

DESCRIPTION :
Iterator function called by list_Scene. Writes the scene object position,
name, visibility and information about the object in it to the command window.
???RC list transformation? Have separate gfx list transformation commands.
==============================================================================*/

int list_Scene_object_transformation(struct Scene_object *scene_object,
	void *dummy);
/*******************************************************************************
LAST MODIFIED : 26 April 1999

DESCRIPTION :
Iterator function for writing the transformation in effect for <scene_object>
in an easy-to-interpret matrix multiplication form.
==============================================================================*/

int list_Scene_object_transformation_commands(struct Scene_object *scene_object,
	void *command_prefix_void);
/*******************************************************************************
LAST MODIFIED : 26 April 1999

DESCRIPTION :
Iterator function for writing the transformation in effect for <scene_object>
as a command, using the given <command_prefix>.
==============================================================================*/

struct Scene *CREATE(Scene)(char *name);
/*******************************************************************************
LAST MODIFIED : 8 February 1998

DESCRIPTION :
Scene now has pointer to its scene_manager, and it uses manager modify
messages to inform its clients of changes. The pointer to the scene_manager
is set and unset by the add/remove object from manager routines, overwritten
from the default versions of these functions.
==============================================================================*/

int DESTROY(Scene)(struct Scene **scene);
/*******************************************************************************
LAST MODIFIED : 19 November 1997

DESCRIPTION :
==============================================================================*/

int Scene_begin_cache(struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 6 June 2001

DESCRIPTION :
Call before making several changes to the scene so only a single change message
is sent. Call Scene_end_cache at the end of the changes.
==============================================================================*/

int Scene_end_cache(struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 6 June 2001

DESCRIPTION :
Call after making changes preceded by a call to Scene_begin_cache to enable a
final message to be sent to clients.
==============================================================================*/

int Scene_enable_graphics(struct Scene *scene,
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_material,
	struct Graphics_font *default_font,
	struct MANAGER(Light) *light_manager,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *default_spectrum,
	struct MANAGER(Texture) *texture_manager);
/*******************************************************************************
LAST MODIFIED : 24 November 2005

DESCRIPTION :
The scene is initially incapable of generating any graphics, since it does not
have access to the material, light and spectrum managers. This routine must be
called soon after creating the scene to give the scene these managers.
Do not have to call this routine if MANAGER_COPY_WITHOUT_IDENTIFIER is used to
create a scene from an existing scene with graphics enabled.
NOTE: The light_manager is not currently used by the scene.
==============================================================================*/

int Scene_disable_graphics(struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 20 July 1998

DESCRIPTION :
Removes links to all objects required to display graphics.
==============================================================================*/

int Scene_enable_time_behaviour(struct Scene *scene,
	struct Time_keeper *default_time_keeper);
/*******************************************************************************
LAST MODIFIED : 12 February 1999

DESCRIPTION :
The scene is initially incapable of varying objects with time as it has no
time_keeper to put time_objects into.
Do not have to call this routine if MANAGER_COPY_WITHOUT_IDENTIFIER is used to
create a scene from an existing scene with graphics enabled.
==============================================================================*/

struct Time_keeper *Scene_get_default_time_keeper(struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 18 February 1999

DESCRIPTION :
==============================================================================*/

int Scene_disable_time_behaviour(struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 12 February 1999

DESCRIPTION :
Removes links to all objects required to vary graphics objects with time.
==============================================================================*/

enum Scene_graphical_element_mode Scene_get_graphical_element_mode(
	struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 4 February 2000

DESCRIPTION :
Returns the mode controlling how graphical element groups are displayed in the
scene.
==============================================================================*/

int Scene_set_graphical_element_mode(struct Scene *scene,
	enum Scene_graphical_element_mode graphical_element_mode,
	struct MANAGER(Computed_field) *computed_field_manager,
	struct Cmiss_region *root_region,
	struct Cmiss_region *data_root_region,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct FE_element_selection *element_selection,
	struct FE_node_selection *node_selection,
	struct FE_node_selection *data_selection,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Sets the mode controlling how graphical element groups are displayed in the
scene. Passes the managers and other data required to create and update the
graphical elements.
Must be called after Scene_enable_graphics since GFEs require the default
material and spectrum.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Scene);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Scene);

PROTOTYPE_LIST_FUNCTIONS(Scene);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Scene,name,char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(Scene,name,char *);
PROTOTYPE_MANAGER_FUNCTIONS(Scene);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Scene,name,char *);

float Scene_time(struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 8 December 1997

DESCRIPTION :
Returns the current time from <scene>.
==============================================================================*/

int Scene_set_time(struct Scene *scene, float time);
/*******************************************************************************
LAST MODIFIED : 19 February 1998

DESCRIPTION :
Sets the current time in <scene>.
==============================================================================*/

int Scene_get_number_of_scene_objects(struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 13 July 1999

DESCRIPTION :
Gets the number of scene_objects in a scene.
==============================================================================*/

int for_each_Scene_object_in_Scene(struct Scene *scene,
	LIST_ITERATOR_FUNCTION(Scene_object) *iterator_function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 23 December 1997

DESCRIPTION :
Allows clients of the <scene> to perform functions with the scene_objects in
it. For example, rendervrml.c needs to output all the scene objects in a scene.
==============================================================================*/

struct Scene_object *first_Scene_object_in_Scene_that(struct Scene *scene,
	LIST_CONDITIONAL_FUNCTION(Scene_object) *conditional_function,
	void *user_data);
/*******************************************************************************
LAST MODIFIED : 15 May 2000

DESCRIPTION :
Wrapper for FIRST_OBJECT_IN_LIST_THAT function for Scene_object.
==============================================================================*/

int Scene_for_each_material(struct Scene *scene,
	MANAGER_ITERATOR_FUNCTION(Graphical_material) *iterator_function,
	void *user_data);
/*******************************************************************************
LAST MODIFIED : 3 May 2005

DESCRIPTION :
Iterates through every material used by the scene.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Scene_picked_object);
PROTOTYPE_LIST_FUNCTIONS(Scene_picked_object);

struct Scene_picked_object *CREATE(Scene_picked_object)(int hit_no);
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Creates a Scene_picked_object for storing picking information in a format
compatible with objects in our display hierarchy. Creates a blank object that
must be filled with appropriate data.
==============================================================================*/

int DESTROY(Scene_picked_object)(
	struct Scene_picked_object **scene_picked_object_address);
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Destroys the Scene_picked_object.
==============================================================================*/

int Scene_picked_object_add_Scene_object(
	struct Scene_picked_object *scene_picked_object,
	struct Scene_object *scene_object);
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Adds the <scene_object> to the end of the list specifying the path to the
picked graphic represented by the <scene_picked_object>.
==============================================================================*/

int Scene_picked_object_get_number_of_scene_objects(
	struct Scene_picked_object *scene_picked_object);
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Returns the number of scene objects in the path of our display heirarchy to the
<scene_picked_object>.
==============================================================================*/

struct Scene_object *Scene_picked_object_get_Scene_object(
	struct Scene_picked_object *scene_picked_object,int scene_object_no);
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Returns the scene_object at position <scene_object_no> - where 0 is the first -
in the list of scene_objects in the path of our display heirarchy to the
<scene_picked_object>.
==============================================================================*/

int Scene_picked_object_add_subobject(
	struct Scene_picked_object *scene_picked_object,int subobject);
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Adds the <subobject> name to the end of the list of names identifying the
particular picked graphic represented by the <scene_picked_object>.
==============================================================================*/

int Scene_picked_object_get_number_of_subobjects(
	struct Scene_picked_object *scene_picked_object);
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Returns the number of integer subobject names identifying the
<scene_picked_object>.
==============================================================================*/

int Scene_picked_object_get_subobject(
	struct Scene_picked_object *scene_picked_object,int subobject_no);
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Returns the subobject at position <subobject_no> - where 0 is the first - in
the list of integer subobject names identifying the <scene_picked_object>.
==============================================================================*/

double Scene_picked_object_get_farthest(
	struct Scene_picked_object *scene_picked_object);
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Returns the <farthest> position at which the <scene_picked_object> was picked.
==============================================================================*/

int Scene_picked_object_set_farthest(
	struct Scene_picked_object *scene_picked_object,double farthest);
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Sets the <farthest> position at which the <scene_picked_object> was picked.
==============================================================================*/

double Scene_picked_object_get_nearest(
	struct Scene_picked_object *scene_picked_object);
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Returns the <nearest> position at which the <scene_picked_object> was picked.
==============================================================================*/

int Scene_picked_object_set_nearest(
	struct Scene_picked_object *scene_picked_object,double nearest);
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Sets the <nearest> position at which the <scene_picked_object> was picked.
==============================================================================*/

int Scene_picked_object_write(struct Scene_picked_object *scene_picked_object);
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Writes the contents of the <scene_picked_object> as:
scene_object_name[.scene_object_name] subobject_number[ subobject_number...]
==============================================================================*/

int Scene_picked_objects_have_same_transformation(
	struct Scene_picked_object *scene_picked_object1,
	struct Scene_picked_object *scene_picked_object2);
/*******************************************************************************
LAST MODIFIED : 23 July 1999

DESCRIPTION :
Returns true if <scene_picked_object1> and <scene_picked_object2> have the
same total transformation.
==============================================================================*/

int Scene_picked_object_get_total_transformation_matrix(
	struct Scene_picked_object *scene_picked_object,int *transformation_required,
	double *transformation_matrix);
/*******************************************************************************
LAST MODIFIED : 23 July 1999

DESCRIPTION :
Multiplies the transformation matrices for all the scene_objects in the
<scene_picked_object>, returning the overall <matrix>. The matrix has 16 values
in the order of along rows first, which operate on the untransformed homogeneous
coordinates [x y z h(=1)] to give [x' y' z' h'], with xm = x'/h', etc. as in:
|x'| |M11 M12 M13 M14| |x|
|y'|=|M21 M22 M23 M24|.|y|
|z'| |M31 M32 M33 M34| |z|
|h'| |M41 M42 M43 M44| |h|
However, if none of the scene objects have transformations, the flag
<transformation_required> will be set to 0 and the <transformation_matrix> will
be set to the identity.
==============================================================================*/

struct Any_object *Scene_picked_object_list_get_nearest_any_object(
	struct LIST(Scene_picked_object) *scene_picked_object_list,
	struct Scene_picked_object **scene_picked_object_address);
/*******************************************************************************
LAST MODIFIED : 24 August 2000

DESCRIPTION :
Returns the nearest picked any_object in <scene_picked_object_list>.
If <scene_picked_object_address> is supplied, the pointer to the
Scene_picked_object referring to the nearest any_object is put there.
==============================================================================*/

struct LIST(Any_object) *Scene_picked_object_list_get_picked_any_objects(
	struct LIST(Scene_picked_object) *scene_picked_object_list);
/*******************************************************************************
LAST MODIFIED : 24 August 2000

DESCRIPTION :
Returns the list of all any_objects in the <scene_picked_object_list>. 
==============================================================================*/

struct FE_element *Scene_picked_object_list_get_nearest_element(
	struct LIST(Scene_picked_object) *scene_picked_object_list,
	struct Cmiss_region *cmiss_region,
	int select_elements_enabled,int select_faces_enabled,int select_lines_enabled,
	struct Scene_picked_object **scene_picked_object_address,
	struct GT_element_group **gt_element_group_address,
	struct GT_element_settings **gt_element_settings_address);
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Returns the nearest picked element in <scene_picked_object_list> that is in
<cmiss_region> (or in root_region if NULL). If any of the remaining address
arguments are not NULL, they are filled with the appropriate information
pertaining to the nearest element.
<select_elements_enabled> allows top-level/3-D elements to be selected.
<select_faces_enabled> allows face and 2-D elements to be selected.
<select_lines_enabled> allows line and 1-D elements to be selected.
==============================================================================*/

struct LIST(FE_element) *Scene_picked_object_list_get_picked_elements(
	struct LIST(Scene_picked_object) *scene_picked_object_list,
	int select_elements_enabled,int select_faces_enabled,
	int select_lines_enabled);
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Returns the list of all elements identified in the <scene_picked_object_list>. 
<select_elements_enabled> allows top-level/3-D elements to be selected.
<select_faces_enabled> allows face and 2-D elements to be selected.
<select_lines_enabled> allows line and 1-D elements to be selected.
==============================================================================*/

struct Element_point_ranges *Scene_picked_object_list_get_nearest_element_point(
	struct LIST(Scene_picked_object) *scene_picked_object_list,
	struct Cmiss_region *cmiss_region,
	struct Scene_picked_object **scene_picked_object_address,
	struct GT_element_group **gt_element_group_address,
	struct GT_element_settings **gt_element_settings_address);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Returns the nearest picked element point in <scene_picked_object_list> that is
in <cmiss_region> (or in root_region if NULL). If any of the remaining address
arguments are not NULL, they are filled with the appropriate information
pertaining to the nearest element point.
The returned Element_point_ranges structure should be used or destroyed by the
calling function.
==============================================================================*/

struct LIST(Element_point_ranges) *Scene_picked_object_list_get_picked_element_points(
	struct LIST(Scene_picked_object) *scene_picked_object_list);
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
Returns the list of all element_points in the <scene_picked_object_list>.
==============================================================================*/

struct FE_node *Scene_picked_object_list_get_nearest_node(
	struct LIST(Scene_picked_object) *scene_picked_object_list,
	int use_data, struct Cmiss_region *cmiss_region,
	struct Scene_picked_object **scene_picked_object_address,
	struct GT_element_group **gt_element_group_address,
	struct GT_element_settings **gt_element_settings_address);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Returns the nearest picked node in <scene_picked_object_list> that is in
<cmiss_region> (or any region if NULL). If any of the remaining address
arguments are not NULL, they are filled with the appropriate information
pertaining to the nearest node.
The <use_data> flag indicates that we are searching for a data point instead of
a node, needed since different settings type used for each, plus it uses the
data_Cmiss_region from the GT_element_group. <cmiss_region> must be a data
region if <use_data> set.
==============================================================================*/

struct LIST(FE_node) *Scene_picked_object_list_get_picked_nodes(
	struct LIST(Scene_picked_object) *scene_picked_object_list,int use_data);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Returns the list of all nodes in the <scene_picked_object_list>. 
The <use_data> flag indicates that we are searching for data points instead of
nodes, needed since different settings type used for each.
==============================================================================*/

int Scene_get_input_callback(struct Scene *scene,
	struct Scene_input_callback *scene_input_callback);
/*******************************************************************************
LAST MODIFIED : 25 March 1999

DESCRIPTION :
Fills <scene_input_callback> with the current scene input_callback information.
==============================================================================*/

int Scene_set_input_callback(struct Scene *scene,
	struct Scene_input_callback *scene_input_callback);
/*******************************************************************************
LAST MODIFIED : 25 March 1999

DESCRIPTION :
Sets the function that will be called to pass on input information, and the data
that that function wants to receive.
==============================================================================*/

int Scene_input(struct Scene *scene,enum Scene_input_type input_type,
	int button_number,int input_modifier,double viewx,double viewy,double viewz,
	double nearx,double neary,double nearz,double farx,double fary,double farz,
	int num_hits
#if defined (OPENGL_API)
	,GLuint *select_buffer
#endif /* defined (OPENGL_API) */
	);
/*******************************************************************************
LAST MODIFIED : 25 July 1998

DESCRIPTION :
Routine called by Scene_viewers - and in future possibly other Scene clients -
to tell about input. At present only mouse input is handled; the <input_type>
says whether it is a mouse button press, motion or button release event, and the
button_number and whether modifier keys were depressed at the time of the
event are given in subsequent parameters. The coordinates of two points in
space, near and far, are given to identify the ray under the mouse pointer at
the time of the event. The view direction is also needed for clients of pick and
drag information. Finally, if the event involved selection/picking, the number
of hits and the select buffer will be set accordingly. The main function of
this routine is to convert picking information into a list of easy-to-interpret
Scene_picked_objects to pass to clients of the scene, eg. node editor.
==============================================================================*/

struct LIST(Scene_picked_object) *Scene_pick_objects(struct Scene *scene,
	struct Interaction_volume *interaction_volume,
	struct Graphics_buffer *graphics_buffer);
/*******************************************************************************
LAST MODIFIED : 18 November 2005

DESCRIPTION :
Returns a list of all the graphical entities in the <interaction_volume> of
<scene>. The nearest member of each scene_picked_object will be adjusted as
understood for the type of <interaction_volume> passed.
==============================================================================*/

int Scene_add_light(struct Scene *scene,struct Light *light);
/*******************************************************************************
LAST MODIFIED : 12 December 1997

DESCRIPTION :
Adds a light to the Scene list_of_lights.
==============================================================================*/

int Scene_has_light_in_list(struct Scene *scene,
	struct LIST(Light) *light_list);
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Returns true if the list_of_lights in <Scene> intersects <light_list>.
==============================================================================*/

int Scene_remove_light(struct Scene *scene,struct Light *light);
/*******************************************************************************
LAST MODIFIED : 12 December 1997

DESCRIPTION :
Removes a light from the Scene list_of_lights.
==============================================================================*/

int for_each_Light_in_Scene(struct Scene *scene,
	LIST_ITERATOR_FUNCTION(Light) *iterator_function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 18 December 1997

DESCRIPTION :
Allows clients of the <scene> to perform functions with the lights in it. The
most common task will be to call execute_Light.
==============================================================================*/

int Scene_has_fast_changing_objects(struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Returns true if the scene may require special rendering because it has
fast_changing objects in it, involving separately calling
execute_Scene_non_fast_changing and execute_Scene_fast_changing, instead of
execute_Scene.
==============================================================================*/

enum Scene_change_status Scene_get_change_status(struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 12 July 2000

DESCRIPTION :
Returns the change state of the scene; SCENE_NO_CHANGE, SCENE_FAST_CHANGE or
SCENE_CHANGE. Clients may respond to SCENE_FAST_CHANGE more efficiently.
==============================================================================*/

int build_Scene(struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 31 May 2001

DESCRIPTION :
To speed up messaging response, graphical_elements put off building
graphics objects for their settings until requested. This function should be
called to request builds for all objects used by <scene>. It should be called
before the scene is output to OpenGL, VRML and wavefront objs. In particular,
this function must be called before compile_Scene.
==============================================================================*/

int compile_Scene(struct Scene *scene, struct Graphics_buffer *graphics_buffer);
/*******************************************************************************
LAST MODIFIED : 31 May 2001

DESCRIPTION :
Assembles the display list containing the whole scene. Before that, however, it
compiles the display lists of objects that will be executed in the scene.
The <graphics_buffer> is used to provide rendering contexts.
Note that lights are not included in the scene and must be handled separately!
Must also call build_Scene before this functions.
==============================================================================*/

int execute_Scene(struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 9 March 2001

DESCRIPTION :
Calls the display list for <scene>. If the display list is not current, an
an error is reported. Version calls both the normal and fast_changing lists.
Note that lights are not included in the scene and must be handled separately!
Initialises the name stack then calls execute_child_Scene.
==============================================================================*/

int execute_Scene_non_fast_changing(struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Calls just the normal non-fast_changing display list for <scene>, if any.
==============================================================================*/

int execute_Scene_fast_changing(struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Calls the just fast_changing display list for <scene>, if any.
==============================================================================*/

int Scene_remove_Scene_object(struct Scene *scene,
	struct Scene_object *scene_object);
/*******************************************************************************
LAST MODIFIED : 14 March 2001

DESCRIPTION :
Removes <scene object> from the list of objects on <scene>.
==============================================================================*/

int Scene_add_graphics_object(struct Scene *scene,
	struct GT_object *graphics_object, int position, char *scene_object_name,
	int fast_changing);
/*******************************************************************************
LAST MODIFIED : 15 March 2001

DESCRIPTION :
Adds <graphics_object> to the list of objects on <scene> at <position>.
A position of 1 indicates the top of the list, while less than 1 or greater
than the number of graphics objects in the list puts it at the end.
The optional <scene_object_name> allows the scene_object to be given a different
name from that of the <graphics_object>, and must be unique for the scene.
Also set the <fast_changing> flag on creation to avoid wrong updates if on.
==============================================================================*/

int Scene_remove_graphics_object(struct Scene *scene,
	struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 15 March 2001

DESCRIPTION :
Removes all scene objects containing <graphics object> from <scene>.
Does not complain if <graphics_object> is not used in <scene>.
==============================================================================*/

int Scene_add_child_scene(struct Scene *scene, struct Scene *child_scene,
	int position, char *scene_object_name, struct MANAGER(Scene) *scene_manager);
/*******************************************************************************
LAST MODIFIED : 15 March 2001

DESCRIPTION :
Adds <child_scene> to the list of objects on <scene> at <position>.
A position of 1 indicates the top of the list, while less than 1 or greater
than the number of graphics objects in the list puts it at the end.
The optional <scene_object_name> allows the scene_object to be given a different
name from that of the <child_scene>, and must be unique for the scene.
==============================================================================*/

int Scene_remove_child_scene(struct Scene *scene, struct Scene *child_scene);
/*******************************************************************************
LAST MODIFIED : 15 March 2001

DESCRIPTION :
Removes all scene objects containing <child_scene> from <scene>.
Does not complain if <child_scene> is not used in <scene>.
==============================================================================*/

int Scene_add_graphical_element_group(struct Scene *scene,
	struct Cmiss_region *cmiss_region, 	struct Cmiss_region *data_cmiss_region,
	int position, char *scene_object_name);
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Adds a graphical element group for <cmiss_region>, with data from
<data_cmiss_region> to the list of objects in <scene> at <position>.
The group will be given a default rendition depending on the scene's current
graphical_element_mode.
The new Scene_object will take the <scene_object_name>; an error is
reported if this name is already in use in <scene>.
A position of 1 indicates the top of the list, while less than 1 or greater
than the number of graphics objects in the list puts it at the end.
Note if the scene is in GRAPHICAL_ELEMENT_MANUAL mode, a group may be added
more than once with a different name, however, it will share the underlying
GT_element_group and therefore have the same rendition.
==============================================================================*/

int Scene_remove_graphical_element_group(struct Scene *scene,
	struct Cmiss_region *cmiss_region);
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Removes all scene objects containing a graphical rendition of <cmiss_region>
from <scene>. Does not complain if <cmiss_region> is not used in <scene>.
==============================================================================*/

int Scene_update_time_behaviour(struct Scene *scene, struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 5 October 1998

DESCRIPTION :
If the graphics_object has more than one time, this function ensures that the
corresponding Scene_objects in this scene have a Time_object.
==============================================================================*/

int Scene_update_time_behaviour_with_gt_element_group(struct Scene *scene,
  struct GT_element_group *gt_element_group);
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
If the <gt_element_group> has more than one time, this function ensures that the
Scene_object has a Time_object.
==============================================================================*/

int Scene_set_time_behaviour(struct Scene *scene, char *scene_object_name,
	char *time_object_name, struct Time_keeper *time_keeper);
/*******************************************************************************
LAST MODIFIED : 12 October 1998

DESCRIPTION :
Creates a Time_object with name <time_object_name> and sets that as the time
object for the scene_object named <scene_object_name>.
==============================================================================*/

int Scene_get_graphics_range(struct Scene *scene,
	double *centre_x, double *centre_y, double *centre_z,
	double *size_x, double *size_y, double *size_z);
/*******************************************************************************
LAST MODIFIED : 16 October 2001

DESCRIPTION :
Finds the range of all visible graphics objects in scene.
Returns 0 without error if scene is empty.
==============================================================================*/

int Scene_get_element_group_position(struct Scene *scene,
	struct Cmiss_region *cmiss_region);
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
The order in which objects are drawn is important for OpenGL transparency.
This function returns the position of <cmiss_region> in <scene>, starting
from 1 at the top. A return value of 0 indicates an error - probably saying
that the GFE for element_group is not in the scene.
==============================================================================*/

int Scene_set_element_group_position(struct Scene *scene,
	struct Cmiss_region *cmiss_region, int position);
/*******************************************************************************
LAST MODIFIED : 2 December 200

DESCRIPTION :
The order in which objects are drawn is important for OpenGL transparency.
This function sets the position of <cmiss_region> in <scene>, starting
from 1 at the top. A value less than 1 or greater than the number of graphics
objects in the list puts <cmiss_region> at the end.
Scene_object for the group keeps the same name.
==============================================================================*/

int Scene_get_graphics_object_position(struct Scene *scene,
	struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 9 December 1997

DESCRIPTION :
The order in which objects are drawn is important for OpenGL transparency.
This function returns the position of <graphics_object> in <scene>, starting
from 1 at the top. A return value of 0 indicates an error - probably saying
that the graphics object is not in the scene.
==============================================================================*/

int Scene_get_scene_object_position(struct Scene *scene,
	struct Scene_object *scene_object);
/*******************************************************************************
LAST MODIFIED : 12 November 2001

DESCRIPTION :
Returns the position of <scene_object> in the <scene>->scene_object_list.
==============================================================================*/

int Scene_set_scene_object_position(struct Scene *scene,
	struct Scene_object *scene_object,int position);
/*******************************************************************************
LAST MODIFIED : 12 October 1998

DESCRIPTION :
The order in which objects are drawn is important for OpenGL transparency.
This function sets the position of <scene_object> in <scene>, starting
from 1 at the top. A value less than 1 or greater than the number of graphics
objects in the list puts <scene_object> at the end.
==============================================================================*/

int Scene_set_graphics_object_position(struct Scene *scene,
	struct GT_object *graphics_object,int position);
/*******************************************************************************
LAST MODIFIED : 9 December 1997

DESCRIPTION :
The order in which objects are drawn is important for OpenGL transparency.
This function sets the position of <graphics_object> in <scene>, starting
from 1 at the top. A value less than 1 or greater than the number of graphics
objects in the list puts <graphics_object> at the end.
==============================================================================*/

enum GT_visibility_type Scene_get_element_group_visibility(
	struct Scene *scene, struct Cmiss_region *cmiss_region);
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Returns the visibility of the GFE for <cmiss_region> in <scene>.
==============================================================================*/

int Scene_set_element_group_visibility(struct Scene *scene,
	struct Cmiss_region *cmiss_region, enum GT_visibility_type visibility);
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Sets the visibility of all scene objects that are graphical element groups for
<cmiss_region> in <scene>.
==============================================================================*/

enum GT_visibility_type Scene_get_graphics_object_visibility(
	struct Scene *scene,struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 9 December 1997

DESCRIPTION :
Returns the visibility of <graphics_object> in <scene>.
==============================================================================*/

int Scene_set_graphics_object_visibility(struct Scene *scene,
	struct GT_object *graphics_object,enum GT_visibility_type visibility);
/*******************************************************************************
LAST MODIFIED : 15 March 2001

DESCRIPTION :
Sets the visibility of all instances of <graphics_object> in <scene>.
==============================================================================*/

int Scene_has_graphics_object(struct Scene *scene,
	struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 8 December 1997

DESCRIPTION :
Returns true if <graphics object> is in the list of objects on <scene>.
==============================================================================*/

struct Scene *Scene_object_get_child_scene(struct Scene_object *scene_object);
/*******************************************************************************
LAST MODIFIED : 7 July 1999

DESCRIPTION :
==============================================================================*/

int Scene_has_child_scene(struct Scene *scene,struct Scene *child_scene);
/*******************************************************************************
LAST MODIFIED : 20 November 1998

DESCRIPTION :
Returns true if <child_scene> is in the list of scenes in <scene>.
==============================================================================*/

struct Scene_object *Scene_get_Scene_object_by_name(struct Scene *scene,
	char *name);
/*******************************************************************************
LAST MODIFIED : 14 March 2001

DESCRIPTION :
Returns the Scene_object called <name> in <scene>, or NULL if not found.
==============================================================================*/

int Scene_has_Cmiss_region(struct Scene *scene,
	struct Cmiss_region *cmiss_region);
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Returns true if <scene> contains a graphical element for <cmiss_region>.
==============================================================================*/

int Scene_has_data_Cmiss_region(struct Scene *scene,
	struct Cmiss_region *cmiss_region);
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Returns true if <scene> contains a graphical element for data <cmiss_region>.
==============================================================================*/

struct GT_element_group *Scene_get_graphical_element_group(
	struct Scene *scene, struct Cmiss_region *cmiss_region);
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Returns the graphical element_group for <cmiss_region> in <scene>.
==============================================================================*/

struct Scene_object *Scene_get_scene_object_with_Cmiss_region(
	struct Scene *scene, struct Cmiss_region *cmiss_region);
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Returns the scene_object for <element_group> in <scene>.
==============================================================================*/

int set_Scene(struct Parse_state *state,
	void *material_address_void,void *scene_manager_void);
/*******************************************************************************
LAST MODIFIED : 9 December 1997

DESCRIPTION :
Modifier function to set the scene from a command.
==============================================================================*/

int set_Scene_including_sub_objects(struct Parse_state *state,
	void *scene_address_void,void *scene_manager_void);
/*******************************************************************************
LAST MODIFIED : 7 July 1999

DESCRIPTION :
Modifier function to set the scene from a command.  This function understands
the use of a period '.' to delimit sub objects and will automatically create
a Scene that wraps a graphics_object from either the scene or a 
GT_element_settings or a whole GT_element_settings so that export commands can
work on these sub_elements.  These created scenes are not added to the manager.
==============================================================================*/

int modify_Scene(struct Parse_state *state,void *scene_void,
	void *modify_scene_data_void);
/*******************************************************************************
LAST MODIFIED : 18 December 1997

DESCRIPTION :
Parser commands for modifying scenes - lighting, etc.
==============================================================================*/

int list_Scene(struct Scene *scene,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 21 September 1998

DESCRIPTION :
Writes the properties of the <scene> to the command window.
==============================================================================*/

int gfx_modify_g_element_general(struct Parse_state *state,
	void *cmiss_region_void, void *scene_void);
/*******************************************************************************
LAST MODIFIED : 2 December 2002

DESCRIPTION :
Executes a GFX MODIFY G_ELEMENT GENERAL command.
Allows general element_group settings to be changed (eg. discretization) and
updates graphics of settings affected by the changes (probably all).
==============================================================================*/

#if !defined (NEW_ALIAS)
int export_to_alias(struct Scene *scene);
/******************************************************************************
LAST MODIFIED : 17 July 1997

DESCRIPTION :
Exports visible volumes to alias
==============================================================================*/
#endif /* !defined (NEW_ALIAS) */

typedef int(*graphics_object_tree_iterator_function)(
	struct GT_object *graphics_object, double time, void *user_data);

int for_each_graphics_object_in_scene(struct Scene *scene,
	graphics_object_tree_iterator_function iterator_function,
	void *user_data);
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
This function iterates through every graphics object in the scene
including those in each individual settings of the graphical finite
elements and those chained together with other graphics objects
==============================================================================*/

int Scene_get_data_range_for_spectrum(struct Scene *scene,
	struct Spectrum *spectrum, float *minimum, float *maximum,
	int *range_set);
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
Sets the range to include the data values of any of the graphics object
in the <scene> which point to this spectrum.  The <range_set> flag is set if
any valid graphics objects using this spectrum were found.
==============================================================================*/
#endif /* !defined (SCENE_H) */
