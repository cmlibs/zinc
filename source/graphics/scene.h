/*******************************************************************************
FILE : scene.h

LAST MODIFIED : 22 February 2000

DESCRIPTION :
Structure for storing the collections of objects that make up a 3-D graphical
model - lights, materials, primitives, etc.
Also contains interface routines for having these converted to display lists,
and for these to be assembled into a single display list of the whole scene.

HISTORY :
November 1997. Created from Scene description part of Drawing.
December 1997. Created MANAGER(Scene).
==============================================================================*/
#if !defined (SCENE_H)
#define SCENE_H

#include "general/callback.h"
#include "graphics/graphical_element.h"
#include "graphics/graphics_object.h"
#include "finite_element/finite_element_to_streamlines.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "graphics/light.h"
#include "graphics/material.h"
#include "graphics/spectrum.h"
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
LAST MODIFIED : 4 February 2000

DESCRIPTION :
Controls whether graphical element groups are placed in the scene and if so,
how they are rendered by default.
Have members BEFORE_FIRST and AFTER_LAST to enable iterating through the list
for automatic creation of choose_enumerator widgets.
==============================================================================*/
{
	GRAPHICAL_ELEMENT_MODE_INVALID,
	GRAPHICAL_ELEMENT_MODE_BEFORE_FIRST,
	GRAPHICAL_ELEMENT_NONE,
	GRAPHICAL_ELEMENT_INVISIBLE,
	GRAPHICAL_ELEMENT_EMPTY,
	GRAPHICAL_ELEMENT_LINES,
	GRAPHICAL_ELEMENT_MODE_AFTER_LAST
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
LAST MODIFIED : 16 February 1999

DESCRIPTION :
Structure to pass to modify_Scene.
==============================================================================*/
{
	struct MANAGER(Light) *light_manager;
	struct MANAGER(Scene) *scene_manager;
	struct Scene *default_scene;
	/* following used for enabling GFEs */
	struct Computed_field_package *computed_field_package;
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(GROUP(FE_element)) *element_group_manager;
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
	struct MANAGER(FE_node) *data_manager;
	struct MANAGER(GROUP(FE_node)) *data_group_manager;
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

/*
Global functions
----------------
*/

char *Scene_graphical_element_mode_string(enum Scene_graphical_element_mode graphical_element_mode);
/*******************************************************************************
LAST MODIFIED : 4 February 2000

DESCRIPTION :
Returns a pointer to a static string describing the <graphical_element_mode>.
This string should match the command used to create that type of settings.
The returned string must not be DEALLOCATEd!
==============================================================================*/

char **Scene_graphical_element_mode_get_valid_strings(int *number_of_valid_strings);
/*******************************************************************************
LAST MODIFIED : 4 February 2000

DESCRIPTION :
Returns and allocated array of pointers to all static strings for valid
Scene_graphical_element_modes - obtained from function Scene_graphical_element_mode_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/

enum Scene_graphical_element_mode Scene_graphical_element_mode_from_string(
	char *graphical_element_mode_string);
/*******************************************************************************
LAST MODIFIED : 4 February 2000

DESCRIPTION :
Returns the <Scene_graphical_element_mode> described by <graphical_element_mode_string>, or
INVALID if not recognized.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Scene_object);
PROTOTYPE_LIST_FUNCTIONS(Scene_object);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Scene_object, \
	position,int);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Scene_object);

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
LAST MODIFIED : 21 July 1998

DESCRIPTION :
Changes the GT_object referenced by <scene_object>. Use to point copied scene
objects to graphics object specific to a scene, eg. scene->axis_object.
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

int Scene_object_has_element_group(struct Scene_object *scene_object,
	void *element_group_void);
/*******************************************************************************
LAST MODIFIED : 19 September 1997

DESCRIPTION :
Scene_object iterator function returning true if <scene_object> contains a
g_ELEMENT_GROUP gt_object referencing the given element_group.
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

int Scene_object_has_unmanaged_element_group(
	struct Scene_object *scene_object,void *element_group_manager_void);
/*******************************************************************************
LAST MODIFIED : 18 February 1998

DESCRIPTION :
Returns true if the <scene_object> contains a GFE for an element group no
longer in <element_group_manager>.
???RC Should be static?
==============================================================================*/

int Scene_object_has_unmanaged_node_group(
	struct Scene_object *scene_object,void *node_group_manager_void);
/*******************************************************************************
LAST MODIFIED : 21 April 1999

DESCRIPTION :
Returns true if the <scene_object> contains a GFE for an element group no
longer in <node_group_manager>.
???RC Should be static?
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

int Scene_object_clear_selected(struct Scene_object *scene_object,void *dummy);
/*******************************************************************************
LAST MODIFIED : 10 February 2000

DESCRIPTION :
Clears the selected flag of the <scene_object>, and unselects any selected
objects it contains.
???RC Later only allow change if current input_client passed to authorise it.
==============================================================================*/

int Scene_object_is_selected(struct Scene_object *scene_object,void *dummy);
/*******************************************************************************
LAST MODIFIED : 10 February 2000

DESCRIPTION :
Returns true if the selected flag of the <scene_object> is set.
==============================================================================*/

int Scene_object_set_selected(struct Scene_object *scene_object);
/*******************************************************************************
LAST MODIFIED : 10 February 2000

DESCRIPTION :
Sets the <selected> flag of the <scene_object>.
???RC Later only allow change if current input_client passed to authorise it.
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

int Scene_clear_selected(struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 11 February 2000

DESCRIPTION :
Unselects all objects in <scene>.
???RC Later only allow change if current input_client passed to authorise it.
==============================================================================*/

int Scene_enable_graphics(struct Scene *scene,
	struct LIST(GT_object) *glyph_list,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_material,
	struct MANAGER(Light) *light_manager,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *default_spectrum,
	struct MANAGER(Texture) *texture_manager);
/*******************************************************************************
LAST MODIFIED : 12 February 1999

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

int Scene_enable_interactive_streamlines(struct Scene *scene,
	struct MANAGER(Interactive_streamline) *streamline_manager);
/*******************************************************************************
LAST MODIFIED : 8 February 1998

DESCRIPTION :
Allows scenes to automatically draw interactive streamlines when they are
modified.
==============================================================================*/

int Scene_disable_interactive_streamlines(struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 8 February 1998

DESCRIPTION :
==============================================================================*/

enum Scene_graphical_element_mode Scene_get_graphical_element_mode(struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 4 February 2000

DESCRIPTION :
Returns the mode controlling how graphical element groups are displayed in the
scene.
==============================================================================*/

int Scene_set_graphical_element_mode(struct Scene *scene,
	enum Scene_graphical_element_mode graphical_element_mode,
	struct Computed_field_package *computed_field_package,
	struct MANAGER(FE_element) *element_manager,
	struct MANAGER(GROUP(FE_element)) *element_group_manager,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(FE_node) *data_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 4 February 2000

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

struct Scene_object *Scene_get_first_scene_object_that(struct Scene *scene,
	LIST_CONDITIONAL_FUNCTION(Scene_object) *conditional_function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 13 July 1999

DESCRIPTION :
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

unsigned int Scene_picked_object_get_farthest(
	struct Scene_picked_object *scene_picked_object);
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Returns the <farthest> position at which the <scene_picked_object> was picked.
==============================================================================*/

int Scene_picked_object_set_farthest(
	struct Scene_picked_object *scene_picked_object,unsigned int farthest);
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Sets the <farthest> position at which the <scene_picked_object> was picked.
==============================================================================*/

unsigned int Scene_picked_object_get_nearest(
	struct Scene_picked_object *scene_picked_object);
/*******************************************************************************
LAST MODIFIED : 15 July 1999

DESCRIPTION :
Returns the <nearest> position at which the <scene_picked_object> was picked.
==============================================================================*/

int Scene_picked_object_set_nearest(
	struct Scene_picked_object *scene_picked_object,unsigned int nearest);
/*******************************************************************************
LAST MODIFIED : 15 July 1999

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

struct FE_node *Scene_picked_object_list_get_nearest_node(
	struct LIST(Scene_picked_object) *scene_picked_object_list,
	struct MANAGER(FE_node) *node_manager,struct GROUP(FE_node) *node_group,
	struct Scene_picked_object **scene_picked_object_address,
	struct GT_element_group **gt_element_group_address,
	struct GT_element_settings **gt_element_settings_address);
/*******************************************************************************
LAST MODIFIED : 22 February 2000

DESCRIPTION :
Returns the nearest picked node in <scene_picked_object_list> that is in
<node_group> (or any group if NULL). If any of the remaining address arguments
are not NULL, they are filled with the appropriate information pertaining to
the nearest node.
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

int Scene_add_light(struct Scene *scene,struct Light *light);
/*******************************************************************************
LAST MODIFIED : 12 December 1997

DESCRIPTION :
Adds a light to the Scene list_of_lights.
==============================================================================*/

int Scene_has_light(struct Scene *scene,struct Light *light);
/*******************************************************************************
LAST MODIFIED : 12 December 1997

DESCRIPTION :
Returns true if <Scene> has <light> in its list_of_lights, OR if <light>
is NULL, returns true if <scene> has any lights.
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

int compile_Scene(struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Assembles the display list containing the whole scene. Before that, however, it
compiles the display lists of objects that will be executed in the scene.
Note that lights are not included in the scene and must be handled separately!
==============================================================================*/

int execute_Scene(struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Calls the display list for <scene>. If the display list is not current, an
an error is reported.
Note that lights are not included in the scene and must be handled separately!
==============================================================================*/

int Scene_add_graphics_object(struct Scene *scene,
	struct GT_object *graphics_object,int position, char *name);
/*******************************************************************************
LAST MODIFIED : 12 October 1998

DESCRIPTION :
Adds <graphics_object> to the list of objects on <scene> at <position>.
A position of 1 indicates the top of the list, while less than 1 or greater
than the number of graphics objects in the list puts it at the end.
The <name> is used for the scene_object and must be unique for the scene.
==============================================================================*/

int Scene_remove_graphics_object(struct Scene *scene,
	struct GT_object *graphics_object);
/*******************************************************************************
LAST MODIFIED : 8 December 1997

DESCRIPTION :
Removes <graphics object> from the list of objects on <scene>.
==============================================================================*/

int Scene_add_child_scene(struct Scene *scene, struct Scene *child_scene,
	int position, char *name, struct MANAGER(Scene) *scene_manager);
/*******************************************************************************
LAST MODIFIED : 20 November 1998

DESCRIPTION :
Adds <child_scene> to the list of objects on <scene> at <position>.
A position of 1 indicates the top of the list, while less than 1 or greater
than the number of graphics objects in the list puts it at the end.
The <name> is used for the scene_object and must be unique for the scene.
==============================================================================*/

int Scene_remove_child_scene(struct Scene *scene,struct Scene *child_scene);
/*******************************************************************************
LAST MODIFIED : 20 November 1998

DESCRIPTION :
Removes <child_scene> from the list of scenes in <scene>.
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

int Scene_get_axis_lengths(struct Scene *scene,Triple axis_lengths);
/*******************************************************************************
LAST MODIFIED : 9 May 1999

DESCRIPTION :
Returns the length of the axes - iff graphics are enabled.
==============================================================================*/

int Scene_set_axis_lengths(struct Scene *scene,Triple axis_lengths);
/*******************************************************************************
LAST MODIFIED : 9 May 1999

DESCRIPTION :
Sets the length of the axes - iff graphics are enabled.
Each axis length can now be set separately.
==============================================================================*/

struct Graphical_material *Scene_get_axis_material(struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 20 July 1998

DESCRIPTION :
Returns the material used to draw the axes - iff graphics are enabled.
==============================================================================*/

int Scene_set_axis_material(struct Scene *scene,
	struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 20 July 1998

DESCRIPTION :
Sets the material used to draw the axes - iff graphics are enabled.
==============================================================================*/

int Scene_get_axis_origin(struct Scene *scene,Triple axis_origin);
/*******************************************************************************
LAST MODIFIED : 31 July 1998

DESCRIPTION :
Returns the origin of the axes - iff graphics are enabled.
==============================================================================*/

int Scene_set_axis_origin(struct Scene *scene,Triple axis_origin);
/*******************************************************************************
LAST MODIFIED : 31 July 1998

DESCRIPTION :
Sets the length of the axes - iff graphics are enabled.
==============================================================================*/

enum GT_visibility_type Scene_get_axis_visibility(struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 21 July 1998

DESCRIPTION :
Returns the visibility of the axes - iff graphics are enabled.
==============================================================================*/

int Scene_set_axis_visibility(struct Scene *scene,
	enum GT_visibility_type visibility);
/*******************************************************************************
LAST MODIFIED : 21 July 1998

DESCRIPTION :
Sets the visibility of the axes - iff graphics are enabled.
==============================================================================*/

int Scene_get_element_group_position(struct Scene *scene,
	struct GROUP(FE_element) *element_group);
/*******************************************************************************
LAST MODIFIED : 25 February 1998

DESCRIPTION :
The order in which objects are drawn is important for OpenGL transparency.
This function returns the position of <element_group> in <scene>, starting
from 1 at the top. A return value of 0 indicates an error - probably saying
that the GFE for element_group is not in the scene.
==============================================================================*/

int Scene_set_element_group_position(struct Scene *scene,
	struct GROUP(FE_element) *element_group,int position);
/*******************************************************************************
LAST MODIFIED : 25 February 1998

DESCRIPTION :
The order in which objects are drawn is important for OpenGL transparency.
This function sets the position of <element_group> in <scene>, starting
from 1 at the top. A value less than 1 or greater than the number of graphics
objects in the list puts <element_group> at the end.
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
	struct Scene *scene,struct GROUP(FE_element) *element_group);
/*******************************************************************************
LAST MODIFIED : 16 February 1998

DESCRIPTION :
Returns the visibility of the GFE for <element_group> in <scene>.
==============================================================================*/

int Scene_set_element_group_visibility(struct Scene *scene,
	struct GROUP(FE_element) *element_group,enum GT_visibility_type visibility);
/*******************************************************************************
LAST MODIFIED : 16 February 1998

DESCRIPTION :
Sets the visibility of the GFE for <element_group> in <scene>.
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
LAST MODIFIED : 9 December 1997

DESCRIPTION :
Sets the visibility of <graphics_object> in <scene>.
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

struct Scene_object *Scene_get_scene_object_by_name(struct Scene *scene,
	char *graphics_object_name);
/*******************************************************************************
LAST MODIFIED : 12 October 1998

DESCRIPTION :
==============================================================================*/

int Scene_has_graphical_element_group(struct Scene *scene,
	struct GROUP(FE_element) *element_group);
/*******************************************************************************
LAST MODIFIED : 8 December 1997

DESCRIPTION :
Returns true if <element_group> is in the list of objects on <scene>.
==============================================================================*/

struct GT_element_group *Scene_get_graphical_element_group(
	struct Scene *scene,struct GROUP(FE_element) *element_group);
/*******************************************************************************
LAST MODIFIED : 8 December 1997

DESCRIPTION :
Returns the graphical element_group for <element_group> in <scene>.
==============================================================================*/

#if defined (OLD_CODE)
int regenerate_element_group_in_Scene(
	struct GROUP(FE_element) *element_group,struct Scene *scene,
	struct FE_element *changed_element,struct FE_node *changed_node);
/*******************************************************************************
LAST MODIFIED : 3 July 1997

DESCRIPTION :
Regenerates the graphics_objects of any settings on the specified window which
have their settings_changed flag set, then puts them in that windows linked-list
of objects attached to the gt_element_group.
==============================================================================*/
#endif /* defined (OLD_CODE) */

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
	void *element_group_void,void *modify_g_element_general_data_void);
/*******************************************************************************
LAST MODIFIED : 8 December 1997

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
