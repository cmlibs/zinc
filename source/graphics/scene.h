/***************************************************************************//**
 * FILE : scene.h
 *
 * Internal interface to Cmiss_scene which describes a collection of graphics
 * able to be output to a Cmiss_scene_viewer or other outputs/devices.
 * It broadly comprises a reference to a region sub-tree and filters controlling
 * which graphics are displayed from its renditions.
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
#if !defined (SCENE_H)
#define SCENE_H

#if defined (BUILD_WITH_CMAKE)
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */

#include "api/zn_scene.h"
#include "general/any_object.h"
#include "general/callback.h"
#include "general/enumerator.h"
#include "graphics/element_point_ranges.h"
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
#include "time/time_keeper.h"
/* #include "graphics/texture.h" */
#if defined(USE_OPENCASCADE)
#	include "api/zn_field_cad.h"
#endif /* defined(USE_OPENCASCADE) */

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

struct Scene_picked_object;
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Describes a single picked item in a format compatible with objects in our
display hierarchy.
The contents of this object are private.
==============================================================================*/

DECLARE_LIST_TYPES(Scene_picked_object);

/* 
The Cmiss_scene which is Public is currently the same object as the 
cmgui internal Scene.  The Public interface is contained in 
api/zn_scene.h however most of the functions come directly from
this module.  So that these functions match the public declarations the 
struct Scene is declared to be the same as Cmiss_scene here
and the functions given their public names.
*/
/* Convert the type */
#define Scene Cmiss_scene

struct Scene;
/*******************************************************************************
LAST MODIFIED : 8 December 1997

DESCRIPTION :
Stores the collections of objects that make up a 3-D graphical model.
The contents of this object are private.
==============================================================================*/

DECLARE_LIST_TYPES(Scene);

DECLARE_MANAGER_TYPES(Scene);

/***************************************************************************//**
 * Structure to pass to define_Scene.
 */
struct Define_scene_data
{
	struct MANAGER(Light) *light_manager;
	struct MANAGER(Scene) *scene_manager;
	struct Cmiss_region *root_region;
	struct Cmiss_graphics_module *graphics_module;
}; /* struct Define_scene_data */

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

struct Scene_get_data_range_for_spectrum_data
{
	struct Spectrum *spectrum;
	struct Graphics_object_data_range_struct range;
};

struct Cmiss_graphic;
struct Cmiss_rendition;

/*
Global functions
----------------
*/

struct Scene *CREATE(Scene)(void);
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

PROTOTYPE_OBJECT_FUNCTIONS(Scene);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Scene);

PROTOTYPE_LIST_FUNCTIONS(Scene);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Scene,name,const char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(Scene,name,const char *);
PROTOTYPE_MANAGER_FUNCTIONS(Scene);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Scene,name,const char *);

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

void *Scene_picked_object_list_get_picked_region_sorted_nodes(
	struct LIST(Scene_picked_object) *scene_picked_object_list,int use_data);

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
	struct Cmiss_rendition **rendition_address,
	struct Cmiss_graphic **graphic_address);
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

struct Element_point_ranges *Scene_picked_object_list_get_nearest_element_point(
	struct LIST(Scene_picked_object) *scene_picked_object_list,
	struct Cmiss_region *cmiss_region,
	struct Scene_picked_object **scene_picked_object_address,
	struct Cmiss_rendition **rendition_address,
	struct Cmiss_graphic **graphic_address);
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

#if defined (USE_OPENCASCADE)
void *Scene_picked_object_list_get_picked_region_cad_primitives(
	struct LIST(Scene_picked_object) *scene_picked_object_list,
	int select_surfaces_enabled, int select_lines_enabled);
#endif /* defined (USE_OPENCASCADE) */

void *Scene_picked_object_list_get_picked_region_sorted_elements(
	struct LIST(Scene_picked_object) *scene_picked_object_list,
	int select_elements_enabled,int select_faces_enabled,
	int select_lines_enabled);

struct FE_node *Scene_picked_object_list_get_nearest_node(
	struct LIST(Scene_picked_object) *scene_picked_object_list,
	int use_data, struct Cmiss_region *cmiss_region,
	struct Scene_picked_object **scene_picked_object_address,
	struct Cmiss_rendition **rendition_address,
	struct Cmiss_graphic **graphic_address);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Returns the nearest picked node in <scene_picked_object_list> that is in
<cmiss_region> (or any region if NULL). If any of the remaining address
arguments are not NULL, they are filled with the appropriate information
pertaining to the nearest node.
The <use_data> flag indicates that we are searching for a data point instead of
a node, needed since different settings type used for each.
==============================================================================*/

#if defined (USE_OPENCASCADE)
Cmiss_cad_identifier_id Scene_picked_object_list_get_cad_primitive(
	struct LIST(Scene_picked_object) *scene_picked_object_list,
	struct Cmiss_region *cmiss_region,
	int select_surfaces_enabled, int select_lines_enabled,
	struct Scene_picked_object **scene_picked_object_address,
	struct Cmiss_rendition **rendition_address,
	struct Cmiss_graphic **graphic_address);
#endif /* defined (USE_OPENCASCADE) */

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

int Scene_update_time_behaviour(struct Scene *scene, struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 5 October 1998

DESCRIPTION :
If the graphics_object has more than one time, this function ensures that the
corresponding Scene_objects in this scene have a Time_object.
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

/***************************************************************************//**
 * Parser commands for defining scenes - filters, lighting, etc.
 * @param define_scene_data_void  void pointer to a struct Define_scene_data
 * with contents filled.
 */
int define_Scene(struct Parse_state *state, void *scene_void,
	void *define_scene_data_void);

int list_Scene(struct Scene *scene,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 21 September 1998

DESCRIPTION :
Writes the properties of the <scene> to the command window.
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

/***************************************************************************//**
 * This function iterates through every graphics object in the scene in region
 * order calling the iterator_function with the optional user_data.
 */
int for_each_graphics_object_in_scene(struct Scene *scene,
	graphics_object_tree_iterator_function iterator_function,
	void *user_data);

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

/***************************************************************************//** 
 * Get the top region of the scene.
 * Caller must clean up handle with Cmiss_scene_destroy.
 * 
 * @param scene  The scene
 * @return  Handle to the top region of the scene, or NULL if none.
 */
struct Cmiss_region *Cmiss_scene_get_region(struct Scene *scene);

int Cmiss_scene_add_rendition(struct Scene *scene, struct Cmiss_rendition *rendition);

int Scene_rendition_changed(
	struct Scene *scene,struct Cmiss_rendition *rendition);

int Cmiss_scene_remove_rendition(struct Scene *scene, 
	struct Cmiss_rendition *rendition);

//int Cmiss_scene_rendition_list_set_show(Scene *scene,
//	struct Cmiss_rendition *rendition, int visibility_flag);

int Scene_export_region_graphics_object(struct Scene *scene,
	struct Cmiss_region *region, const char *graphic_name,
	graphics_object_tree_iterator_function iterator_function,
	void *user_data);

int Scene_add_graphics_object(struct Scene *scene,
	struct GT_object *graphics_object, const char *cmiss_graphic_name);
/*******************************************************************************
Adds <graphics_object> to the list of objects on <scene> at <position>.
A position of 1 indicates the top of the list, while less than 1 or greater
than the number of graphics objects in the list puts it at the end.
The optional <scene_object_name> allows the scene_object to be given a different
name from that of the <graphics_object>, and must be unique for the scene.
Also set the <fast_changing> flag on creation to avoid wrong updates if on.
==============================================================================*/

int Scene_picked_object_get_number_of_renditions(
	struct Scene_picked_object *scene_picked_object);
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Returns the number of scene objects in the path of our display heirarchy to the
<scene_picked_object>.
==============================================================================*/

int Scene_picked_object_add_rendition(
	struct Scene_picked_object *scene_picked_object,
	struct Cmiss_rendition *rendition);

struct Cmiss_rendition *Scene_picked_object_get_rendition(
	struct Scene_picked_object *scene_picked_object,int rendition_no);
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Returns the scene_object at position <scene_object_no> - where 0 is the first -
in the list of scene_objects in the path of our display heirarchy to the
<scene_picked_object>.
==============================================================================*/

int Cmiss_scene_graphics_filter_change(struct Scene *scene,	void *message_void);

/***************************************************************************//**
 * Clean up the callback used by scene_viewer_projection callback.
 */
int Cmiss_scene_cleanup_top_rendition_scene_projection_callback(
	struct Scene *scene, void *dummy_void);

#endif /* !defined (SCENE_H) */
