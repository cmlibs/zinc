/*******************************************************************************
FILE : scene_viewer.h

LAST MODIFIED : 29 September 2000

DESCRIPTION :
Three_D_drawing derivative for viewing a Scene from an arbitrary position.
The scene viewer has the following modes for handling user input:
SCENE_VIEWER_NO_INPUT ignores any input, leaving it up to the owner of the
scene viewer to set viewing parameters.
SCENE_VIEWER_SELECT performs OpenGL picking and returns the picked objects
to the scene via a callback, along with mouse button press and motion
information in a view-independent format.
SCENE_VIEWER_TRANSFORM allows the view of the scene to be changed by tumbling,
translating and zooming with mouse button press and motion events.
==============================================================================*/
#if !defined (SCENE_VIEWER_H)
#define SCENE_VIEWER_H

#include "general/callback.h"
#include "general/image_utilities.h"
#include "general/object.h"
#include "graphics/colour.h"
#include "graphics/scene.h"
#include "graphics/light.h"
#include "graphics/light_model.h"
#include "interaction/interactive_tool.h"

/*
Global types
------------
*/
enum Scene_viewer_projection_mode
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
Be sure to implement any new modes in Scene_viewer_projection_mode_string.
==============================================================================*/
{
	SCENE_VIEWER_PARALLEL,
	SCENE_VIEWER_PERSPECTIVE,
	SCENE_VIEWER_CUSTOM
};

enum Scene_viewer_input_mode
/*******************************************************************************
LAST MODIFIED : 6 October 2000

DESCRIPTION :
==============================================================================*/
{
	SCENE_VIEWER_NO_INPUT_OR_DRAW,
	SCENE_VIEWER_UPDATE_ON_CLICK,
	SCENE_VIEWER_NO_INPUT,
	SCENE_VIEWER_SELECT,
	SCENE_VIEWER_TRANSFORM
};

enum Scene_viewer_buffer_mode
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
???RC May want to add left and right buffer options for 3-D display.
Be sure to implement any new modes in Scene_viewer_buffer_mode_string.
==============================================================================*/
{
	SCENE_VIEWER_PIXEL_BUFFER,
	SCENE_VIEWER_SINGLE_BUFFER,
	SCENE_VIEWER_DOUBLE_BUFFER
};

enum Scene_viewer_viewport_mode
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
In RELATIVE viewport mode the intended viewing volume is made as large as
possible in the physical viewport while maintaining the aspect ratio from
NDC_width and NDC_height. In ABSOLUTE viewport mode viewport_pixels_per_unit
values are used to give and exact mapping from user coordinates to pixels.
Be sure to implement any new modes in Scene_viewer_viewport_mode_string.
==============================================================================*/
{
	SCENE_VIEWER_ABSOLUTE_VIEWPORT,
	SCENE_VIEWER_RELATIVE_VIEWPORT
};

enum Scene_viewer_transparency_mode
/*******************************************************************************
LAST MODIFIED : 23 November 1998

DESCRIPTION :
In mode SCENE_VIEWER_SLOW_TRANSPARENCY the display list for the scene is
rendered twice. In the first pass only opaque objects (alpha=1.0) are rendered,
while these set the depth buffer. In the second pass writing to the depth buffer
is disabled, while only fragments for which alpha does not equal 1.0 are
rendered.
Be sure to implement any new modes in Scene_viewer_transparency_mode_string.
==============================================================================*/
{
	SCENE_VIEWER_FAST_TRANSPARENCY,
	SCENE_VIEWER_SLOW_TRANSPARENCY,
	SCENE_VIEWER_LAYERED_TRANSPARENCY
};

struct Scene_viewer;
/*******************************************************************************
LAST MODIFIED : 18 November 1997
 
DESCRIPTION :
Members of the Scene_viewer structure are private.
==============================================================================*/

DECLARE_CALLBACK_TYPES(Scene_viewer_transform, \
	struct Scene_viewer *, void *);

/*
Global functions
----------------
*/
struct Scene_viewer *CREATE(Scene_viewer)(Widget parent,
	struct Colour *background_colour,enum Scene_viewer_buffer_mode buffer_mode,
	struct MANAGER(Light) *light_manager,struct Light *default_light,
	struct MANAGER(Light_model) *light_model_manager,
	struct Light_model *default_light_model,
	struct MANAGER(Scene) *scene_manager,struct Scene *scene,
	struct MANAGER(Texture) *texture_manager,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 18 November 1998

DESCRIPTION :
Creates a Scene_viewer in the widget <parent> to display <scene>.
Note: the parent must be an XmForm since form constraints will be applied.
If any of light_manager, light_model_manager, scene_manager or texture_manager
are supplied, the scene_viewer will automatically redraw in response to changes
of objects from these managers that are in use by the scene_viewer. Redraws are
performed in idle time so that multiple redraws are avoided.
==============================================================================*/

int DESTROY(Scene_viewer)(struct Scene_viewer **scene_viewer_address);
/*******************************************************************************
LAST MODIFIED : 18 November 1998

DESCRIPTION :
Closes the scene_viewer and disposes of the scene_viewer data structure.
==============================================================================*/

int Scene_viewer_awaken(struct Scene_viewer *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 29 September 2000

DESCRIPTION :
Restores manager callbacks of previously inactive scene_viewer. Must call after
Scene_viewer_sleep to restore normal activity.
==============================================================================*/

int Scene_viewer_stop_animations(struct Scene_viewer *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 29 September 2000

DESCRIPTION :
Tells the <scene_viewer> to stop all automatic informations that it produces,
eg. automatic tumble.
==============================================================================*/

int Scene_viewer_sleep(struct Scene_viewer *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 29 September 2000

DESCRIPTION :
Turns off any pending automatic tumbles or redraws in idle time, and removes
any manager callbacks to minimise impact of inactive scene_viewer on rest of
program. Must call Scene_viewer_awaken to restore manager callbacks.
Must call this in DESTROY function.
==============================================================================*/

int Scene_viewer_get_antialias_mode(struct Scene_viewer *scene_viewer,
	int *antialias);
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
==============================================================================*/

int Scene_viewer_set_antialias_mode(struct Scene_viewer *scene_viewer,
	int antialias_mode);
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Sets the number of jitter samples used to antialias the scene_viewer.
Zero turns antialiasing off.
==============================================================================*/

int Scene_viewer_get_background_colour(struct Scene_viewer *scene_viewer,
	struct Colour *background_colour);
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
Returns the background_colour of the scene_viewer.
==============================================================================*/

int Scene_viewer_set_background_colour(struct Scene_viewer *scene_viewer,
	struct Colour *background_colour);
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
Sets the background_colour of the scene_viewer.
==============================================================================*/

struct Texture *Scene_viewer_get_background_texture(
	struct Scene_viewer *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
Retrieves the Scene_viewer's background_texture. Note that NULL is the valid
return if there is no background texture.
==============================================================================*/

int Scene_viewer_set_background_texture(struct Scene_viewer *scene_viewer,
	struct Texture *background_texture);
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
Sets the background_texture to be displayed in the Scene_viewer. Information
on how it will be displayed is set in Scene_viewer_set_background_texture_info.
==============================================================================*/

int Scene_viewer_get_background_texture_info(struct Scene_viewer *scene_viewer,
	double *bk_texture_left,double *bk_texture_top,
	double *bk_texture_width,double *bk_texture_height,
	int *bk_texture_undistort_on,double *bk_texture_max_pixels_per_polygon);
/*******************************************************************************
LAST MODIFIED : 28 September 1999

DESCRIPTION :
See Scene_viewer_set_background_texture_info for meaning of return values.
==============================================================================*/

int Scene_viewer_set_background_texture_info(struct Scene_viewer *scene_viewer,
	double bk_texture_left,double bk_texture_top,
	double bk_texture_width,double bk_texture_height,
	int bk_texture_undistort_on,double bk_texture_max_pixels_per_polygon);
/*******************************************************************************
LAST MODIFIED : 28 September 1999

DESCRIPTION :
If there is a background_texture in the scene_viewer, these values specify the
top,left corner, in user coordinates, where it will be displayed, while the
next two parameters specify the size it will have in these coordinates.
If the bk_texture_undistort_on flag is set, radial distortion parameters from the background texture are un-distorted when the
texture is displayed. It does this by drawing it as a collection of polygons;
the last parameter controls the size of polygons used to do this.
==============================================================================*/

int Scene_viewer_get_border_width(struct Scene_viewer *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 12 October 1998

DESCRIPTION :
Returns the border_width of the scene_viewer widget. Note that the border is
only shown on the bottom and right of each viewer in the graphics window.
==============================================================================*/

int Scene_viewer_set_border_width(struct Scene_viewer *scene_viewer,
	int border_width);
/*******************************************************************************
LAST MODIFIED : 12 October 1998

DESCRIPTION :
Sets the border_width of the scene_viewer widget. Note that the border is
only shown on the bottom and right of each viewer in the graphics window.
==============================================================================*/

enum Scene_viewer_buffer_mode Scene_viewer_get_buffer_mode(
	struct Scene_viewer *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Returns the buffer mode - single_buffer/double_buffer - of the Scene_viewer.
==============================================================================*/

struct Interactive_tool *Scene_viewer_get_interactive_tool(
	struct Scene_viewer *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 11 April 2000

DESCRIPTION :
Returns the interactive_tool used by the Scene_viewer.
The interactive_tool may be NULL, indicating that no overlay is in use.
==============================================================================*/

int Scene_viewer_set_interactive_tool(struct Scene_viewer *scene_viewer,
	struct Interactive_tool *interactive_tool);
/*******************************************************************************
LAST MODIFIED : 11 April 2000

DESCRIPTION :
Sets the interactive tool that will receive input if the Scene_viewer is in
SCENE_VIEWER_SELECT mode. A NULL value indicates no tool.
==============================================================================*/

enum Scene_viewer_input_mode Scene_viewer_get_input_mode(
	struct Scene_viewer *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Returns the input_mode of the Scene_viewer.
==============================================================================*/

int Scene_viewer_set_input_mode(struct Scene_viewer *scene_viewer,
	enum Scene_viewer_input_mode input_mode);
/*******************************************************************************
LAST MODIFIED : 19 December 1997

DESCRIPTION :
Sets the input_mode of the Scene_viewer.
==============================================================================*/

int Scene_viewer_add_light(struct Scene_viewer *scene_viewer,
	struct Light *light);
/*******************************************************************************
LAST MODIFIED : 3 December 1997

DESCRIPTION :
Adds a light to the Scene_viewer list_of_lights.
==============================================================================*/

int Scene_viewer_has_light(struct Scene_viewer *scene_viewer,
	struct Light *light);
/*******************************************************************************
LAST MODIFIED : 12 December 1997

DESCRIPTION :
Returns true if <Scene_viewer> has <light> in its list_of_lights, OR if <light>
is NULL, returns true if <scene_viewer> has any lights.
==============================================================================*/

int Scene_viewer_remove_light(struct Scene_viewer *scene_viewer,
	struct Light *light);
/*******************************************************************************
LAST MODIFIED : 3 December 1997

DESCRIPTION :
Removes a light from the Scene_viewer list_of_lights.
==============================================================================*/

struct Light_model *Scene_viewer_get_light_model(
	struct Scene_viewer *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 3 December 1997

DESCRIPTION :
Returns the Scene_viewer light_model.
==============================================================================*/

int Scene_viewer_set_light_model(struct Scene_viewer *scene_viewer,
	struct Light_model *light_model);
/*******************************************************************************
LAST MODIFIED : 3 December 1997

DESCRIPTION :
Sets the Scene_viewer light_model.
==============================================================================*/

int Scene_viewer_get_perturb_lines(struct Scene_viewer *scene_viewer,
	int *perturb_lines);
/*******************************************************************************
LAST MODIFIED : 13 June 2000

DESCRIPTION :
==============================================================================*/

int Scene_viewer_set_perturb_lines(struct Scene_viewer *scene_viewer,
	int perturb_lines);
/*******************************************************************************
LAST MODIFIED : 13 June 2000

DESCRIPTION :
When the line draw mode is turned on (set to one) the lines are raised in the
z direction when the GL_EXT_polygon_offset extension is available from the X
Server.  This means that the lines appear solid rather than interfering with a
surface in the same space.
==============================================================================*/

int Scene_viewer_get_lookat_parameters(struct Scene_viewer *scene_viewer,
	double *eyex,double *eyey,double *eyez,
	double *lookatx,double *lookaty,double *lookatz,
	double *upx,double *upy,double *upz);
/*******************************************************************************
LAST MODIFIED : 18 November 1997

DESCRIPTION :
Gets the view direction and orientation of the Scene_viewer.
==============================================================================*/

int Scene_viewer_set_lookat_parameters_non_skew(
	struct Scene_viewer *scene_viewer,double eyex,double eyey,double eyez,
	double lookatx,double lookaty,double lookatz,
	double upx,double upy,double upz);
/*******************************************************************************
LAST MODIFIED : 7 October 1998

DESCRIPTION :
Special version of Scene_viewer_set_lookat_parameters that ensures the up vector
is orthogonal to the view direction - so prejection is not skew.
==============================================================================*/

int Scene_viewer_set_lookat_parameters(struct Scene_viewer *scene_viewer,
	double eyex,double eyey,double eyez,
	double lookatx,double lookaty,double lookatz,
	double upx,double upy,double upz);
/*******************************************************************************
LAST MODIFIED : 18 November 1997

DESCRIPTION :
Sets the view direction and orientation of the Scene_viewer.
==============================================================================*/

int Scene_viewer_get_modelview_matrix(struct Scene_viewer *scene_viewer,
	double modelview_matrix[16]);
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
Reads the modelview matrix used if the SCENE_VIEWER_CUSTOM projection is in
effect. The format of the matrix is as in Scene_viewer_set_modelview_matrix.
==============================================================================*/

int Scene_viewer_set_modelview_matrix(struct Scene_viewer *scene_viewer,
	double modelview_matrix[16]);
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
Sets the modelview matrix used if the SCENE_VIEWER_CUSTOM projection is in
effect. The 4X4 matrix is stored in an array of 16 double values, with values
consecutive across rows, eg:
[x'] = |  m0  m1  m2  m3 |.[x]
[y']   |  m4  m5  m6  m7 | [y]
[z']   |  m8  m9 m10 m11 | [z]
[w']   | m12 m13 m14 m15 | [w]
==============================================================================*/

int Scene_viewer_get_NDC_info(struct Scene_viewer *scene_viewer,
	double *NDC_left,double *NDC_top,double *NDC_width,double *NDC_height);
/*******************************************************************************
LAST MODIFIED : 21 January 1998
 
DESCRIPTION :
Gets the NDC_info from the scene_viewer - see Scene_viewer_set_NDC_info.
==============================================================================*/

int Scene_viewer_set_NDC_info(struct Scene_viewer *scene_viewer,
	double NDC_left,double NDC_top,double NDC_width,double NDC_height);
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
The projection matrix converts the viewing volume into Normalised Device
Coordinates - NDCs - which range from -1.0 to +1.0 in each axis. However, the
shape of this area in the x,y plane of the screen will not be square in the
general case. The NDC_width, NDC_height, NDC_top and NDC_left values describe
the physical dimensions of the NDC cube, used to draw the image on the screen
without distortion. In RELATIVE viewport_mode, only the ratio of NDC_width to
NDC_height is important. In ABSOLUTE viewport_mode, the top and left values
are used to position the intended viewing volume in user coordinates.
==============================================================================*/

struct Scene *Scene_viewer_get_overlay_scene(struct Scene_viewer *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 18 November 1998

DESCRIPTION :
Returns the overlay_scene used by the Scene_viewer.
The overlay_scene may be NULL, indicating that no overlay is in use.
==============================================================================*/

int Scene_viewer_set_overlay_scene(struct Scene_viewer *scene_viewer,
	struct Scene *overlay_scene);
/*******************************************************************************
LAST MODIFIED : 18 November 1998

DESCRIPTION :
Sets the overlay_scene displayed in addition to the scene as a fixed parallel
projection with coordinates ranging from -1 to +1 in the largest square fitting
in the scene_viewer window, and -1 to +1 from far to near.
The overlay_scene may be NULL, indicating that no overlay is in use.
==============================================================================*/

enum Scene_viewer_projection_mode Scene_viewer_get_projection_mode(
	struct Scene_viewer *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 5 October 1998

DESCRIPTION :
Returns the projection mode - parallel/perspective - of the Scene_viewer.
==============================================================================*/

int Scene_viewer_set_projection_mode(struct Scene_viewer *scene_viewer,
	enum Scene_viewer_projection_mode projection_mode);
/*******************************************************************************
LAST MODIFIED : 21 November 1997

DESCRIPTION :
Sets the projection mode - parallel/perspective - of the Scene_viewer.
==============================================================================*/

int Scene_viewer_get_projection_matrix(struct Scene_viewer *scene_viewer,
	double projection_matrix[16]);
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
Reads the projection matrix used if the SCENE_VIEWER_CUSTOM projection is in
effect. The format of the matrix is as in Scene_viewer_set_projection_matrix.
==============================================================================*/

int Scene_viewer_set_projection_matrix(struct Scene_viewer *scene_viewer,
	double projection_matrix[16]);
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
Sets the projection matrix used if the SCENE_VIEWER_CUSTOM projection is in
effect. The 4X4 matrix is stored in an array of 16 double values, with values
consecutive across rows, eg:
[x'] = |  m0  m1  m2  m3 |.[x]
[y']   |  m4  m5  m6  m7 | [y]
[z']   |  m8  m9 m10 m11 | [z]
[w']   | m12 m13 m14 m15 | [w]
==============================================================================*/

struct Scene *Scene_viewer_get_scene(struct Scene_viewer *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 14 February 1998

DESCRIPTION :
Returns the Scene_viewer scene.
==============================================================================*/

int Scene_viewer_set_scene(struct Scene_viewer *scene_viewer,
	struct Scene *scene);
/*******************************************************************************
LAST MODIFIED : 14 February 1998

DESCRIPTION :
Sets the Scene_viewer scene.
==============================================================================*/

int Scene_viewer_get_transform_rate(struct Scene_viewer *scene_viewer,
	double *translate_rate,double *tumble_rate,double *zoom_rate);
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Returns the rate of translate, tumble and zoom transformations in
relation to mouse movements. Values of around 1.0 could be considered normal.
A value of zero turns off that transform capability.
==============================================================================*/

int Scene_viewer_set_transform_rate(struct Scene_viewer *scene_viewer,
	double translate_rate,double tumble_rate,double zoom_rate);
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Sets the rate of translate, tumble and zoom transformations in
relation to mouse movements. Values of around 1.0 could be considered normal.
A value of zero turns off that transform capability.
Negative values reverse the effects of mouse movement.
==============================================================================*/

int Scene_viewer_sync_add_callback(struct Scene_viewer *scene_viewer,
	CALLBACK_FUNCTION(Scene_viewer_transform) *function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Adds a callback to <element_selection> so that when it changes <function> is
called with <user_data>. <function> has 3 arguments, a
struct Scene_viewer *, a void* and the void *user_data.
==============================================================================*/

int Scene_viewer_sync_remove_callback(struct Scene_viewer *scene_viewer,
	CALLBACK_FUNCTION(Scene_viewer_transform) *function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Removes the callback calling <function> with <user_data> from
<scene_viewer>.
==============================================================================*/

int Scene_viewer_transform_add_callback(struct Scene_viewer *scene_viewer,
	CALLBACK_FUNCTION(Scene_viewer_transform) *function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Adds a callback to <element_selection> so that when it changes <function> is
called with <user_data>. <function> has 3 arguments, a
struct Scene_viewer *, a void* and the void *user_data.
==============================================================================*/

int Scene_viewer_transform_remove_callback(struct Scene_viewer *scene_viewer,
	CALLBACK_FUNCTION(Scene_viewer_transform) *function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Removes the callback calling <function> with <user_data> from
<scene_viewer>.
==============================================================================*/

enum Scene_viewer_transparency_mode Scene_viewer_get_transparency_mode(
	struct Scene_viewer *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 23 November 1998

DESCRIPTION :
See Scene_viewer_set_transparency_mode for explanation.
==============================================================================*/

int Scene_viewer_set_transparency_mode(struct Scene_viewer *scene_viewer,
	enum Scene_viewer_transparency_mode transparency_mode);
/*******************************************************************************
LAST MODIFIED : 23 November 1998

DESCRIPTION :
Sets the transparency_mode of the Scene_viewer. In fast transparency mode,
the scene is drawn as is, with depth buffer writing even for semi-transparent
objects. In slow transparency mode, opaque objects are rendered first, then
semi-transparent objects are rendered without writing the depth buffer. Hence,
you can even see through the first semi-transparent surface drawn.
==============================================================================*/

int Scene_viewer_get_transparency_layers(struct Scene_viewer *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 9 October 1999

DESCRIPTION :
See Scene_viewer_set_transparency_layers for explanation.
==============================================================================*/

int Scene_viewer_set_transparency_layers(struct Scene_viewer *scene_viewer,
	int layers);
/*******************************************************************************
LAST MODIFIED : 9 October 1999

DESCRIPTION :
When the transparency_mode of the Scene_viewer is layered_transparency then
the z depth is divided into <layers> slices.  From back to front for each layer
the clip planes are set to clip all other layers and then the entire scene is 
drawn.  This is very expensive but can get great results for transparent
surfaces.  Best use of the slices is made if the near and far clip planes are
tight around the objects in the scene.
==============================================================================*/

int Scene_viewer_get_view_angle(struct Scene_viewer *scene_viewer,
	double *view_angle);
/*******************************************************************************
LAST MODIFIED : 13 October 1998

DESCRIPTION :
Gets the diagonal view angle, in radians, of the <scene_viewer>.
For PARALLEL and PERSPECTIVE projection modes only.
==============================================================================*/

int Scene_viewer_set_view_angle(struct Scene_viewer *scene_viewer,
	double view_angle);
/*******************************************************************************
LAST MODIFIED : 13 October 1998

DESCRIPTION :
Sets the diagonal view angle, in radians, of the <scene_viewer>.
For PARALLEL and PERSPECTIVE projection modes only.
==============================================================================*/

int Scene_viewer_set_view_simple(struct Scene_viewer *scene_viewer,
	double centre_x,double centre_y,double centre_z,double radius,
	double view_angle,double clip_distance);
/*******************************************************************************
LAST MODIFIED : 28 February 1998

DESCRIPTION :
Adjusts the viewing parameters of <scene_viewer> so that it is looking at the
<centre_pt> of a sphere of the given <radius> with the given <view_angle>.
The function also adjusts the far clipping plane to be clip_distance behind
the interest point, and the near plane to by the minimum of clip_distance or
eye_distance*0.99 in front of it.
==============================================================================*/

int Scene_viewer_get_viewing_volume(struct Scene_viewer *scene_viewer,
	double *left,double *right,double *bottom,double *top,double *near,
	double *far);
/*******************************************************************************
LAST MODIFIED : 18 November 1997

DESCRIPTION :
Gets the viewing volume of the Scene_viewer.
==============================================================================*/

int Scene_viewer_set_viewing_volume(struct Scene_viewer *scene_viewer,
	double left,double right,double bottom,double top,double near,double far);
/*******************************************************************************
LAST MODIFIED : 15 December 1997

DESCRIPTION :
Sets the viewing volume of the Scene_viewer. Unless the viewing volume is the
same shape as the window, taking into account the aspect, the Scene_viewer will
enlarge it to maintain the desired aspect ratio. Hence, the values specified
represent the minimum viewing volume. The left, right, bottom and top values
are at the lookat point, not on the near plane as OpenGL assumes. This gives a
similar sized viewing_volume for both parallel and perspective projections.
The viewing volume can be made unsymmetric to create special effects such as
rendering a higher resolution image in parts.
==============================================================================*/

int Scene_viewer_get_viewport_info(struct Scene_viewer *scene_viewer,
	double *viewport_left,double *viewport_top,double *viewport_pixels_per_unit_x,
	double *viewport_pixels_per_unit_y);
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
See Scene_viewer_set_viewport_info for explanation of the values returned.
==============================================================================*/

int Scene_viewer_set_viewport_info(struct Scene_viewer *scene_viewer,
	double viewport_left,double viewport_top,double viewport_pixels_per_unit_x,
	double viewport_pixels_per_unit_y);
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
The 4 parameters of this routine define the user coordinate system in the
physical viewport. <viewport_left> and <viewport_top> define the exact value
of the user's x and y coordinate at these edges of the viewport.
Note that these are real numbers, and that they refer to the location at the
top-left of the top-left pixel.
???RC Check the above!
The remaining two values specify the scale in pixels per 1 unit of user
coordinates in the x and y direction.
The parameters to the Scene_viewer_set_background_texture_info and
Scene_viewer_set_NDC_info routines are to be given in the user coordinate
system established here. Furthermore, by adjusting the viewport_left and
viewport_top, the image can be translated, while changing the number of
pixels per unit enables zooming to be achieved.
???RC Later: send user coordinates with mouse events.
???RC How to handle y axis pointing down?
==============================================================================*/

enum Scene_viewer_viewport_mode Scene_viewer_get_viewport_mode(
	struct Scene_viewer *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
See Scene_viewer_set_viewport_mode for explanation.
==============================================================================*/

int Scene_viewer_set_viewport_mode(struct Scene_viewer *scene_viewer,
	enum Scene_viewer_viewport_mode viewport_mode);
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
Sets the viewport_mode of the Scene_viewer. A relative viewport scales the NDC
viewing volume to the maximum size that can fit in the window. An absolute
viewport uses the NDC_information to map the NDC viewing volume onto the
viewport coordinates, which are specified relative to the window.
==============================================================================*/

int Scene_viewer_get_viewport_size(struct Scene_viewer *scene_viewer,
	Dimension *width,Dimension *height);
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
Returns the width and height of the Scene_viewers drawing area.
==============================================================================*/

int Scene_viewer_get_window_projection_matrix(struct Scene_viewer *scene_viewer,
	double window_projection_matrix[16]);
/*******************************************************************************
LAST MODIFIED : 24 March 1998

DESCRIPTION :
Returns the actual projection matrix applied to fill the window.
==============================================================================*/

int Scene_viewer_set_viewport_size(struct Scene_viewer *scene_viewer,
	Dimension width,Dimension height);
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
Sets the width and height of the Scene_viewers drawing area.
==============================================================================*/

int Scene_viewer_rotate_about_lookat_point(struct Scene_viewer *scene_viewer,
	double axis[3],double angle);
/*******************************************************************************
LAST MODIFIED : 26 November 1997

DESCRIPTION :
Rotates the eye <angle> radians about unit vector axis <a> stemming from the
<scene_viewer> lookat point. Up vector is also reoriented to remain normal to
the eye-to-lookat direction. Rotation is in a clockwise sense. Also, if <a> is
not already a unit vector, it will be made one by this function.
==============================================================================*/

int for_each_Light_in_Scene_viewer(struct Scene_viewer *scene_viewer,
	LIST_ITERATOR_FUNCTION(Light) *iterator_function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 18 December 1997

DESCRIPTION :
Allows clients of the <scene_viewer> to perform functions with the lights in it.
The most common task will to list the lights in the scene with show_Light.
==============================================================================*/

int Scene_viewer_render_scene(struct Scene_viewer *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 23 July 1999

DESCRIPTION :
Called to redraw the Scene_viewer scene after changes in the display lists or
viewing transformations.
==============================================================================*/

int Scene_viewer_render_scene_with_picking(struct Scene_viewer *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 23 July 1999

DESCRIPTION :
Called to redraw the Scene_viewer scene after changes in the display lists or
viewing transformations.  Writes picking names with the primitives.
==============================================================================*/

int Scene_viewer_render_scene_in_viewport(struct Scene_viewer *scene_viewer,
	int left, int bottom, int right, int top);
/*******************************************************************************
LAST MODIFIED : 23 July 1999

DESCRIPTION :
Called to redraw the Scene_viewer scene after changes in the display lists or
viewing transformations.  Uses the specified viewport to draw into (unless
all the dimensions are zero).
==============================================================================*/

int Scene_viewer_redraw(struct Scene_viewer *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Call this after changing viewing parameters or display lists to have the
Scene_viewer redraw the image. Does this by putting a WorkProc on the queue
(if not already done for this Scene_viewer) which will force a redraw at the
next idle moment. If the scene_viewer is changed again before it is updated,
a new WorkProc will not be put in the queue, but the old one will update the
window to the new state.
Forces a full redraw rather than a fast_changing update.
==============================================================================*/

int Scene_viewer_redraw_now(struct Scene_viewer *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Forces a redraw of the given scene viewer to take place immediately - ie.
not just at the next idle moment as Scene_viewer_redraw does.
Forces a full redraw rather than a fast_changing update.
==============================================================================*/

int Scene_viewer_redraw_now_without_swapbuffers(
	struct Scene_viewer *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 12 June 1998

DESCRIPTION :
Forces a redraw of the given scene viewer to take place immediately but does
not swap the back and front buffers so that utilities such as the movie
extensions can get the undated frame from the backbuffer.
==============================================================================*/

int Scene_viewer_set_update_pixel_image(struct Scene_viewer *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 18 May 1998

DESCRIPTION :
Sets a flag so that the redraw will necessarily fully render the scene in
pixel buffer mode
==============================================================================*/

int Scene_viewer_set_pixel_image(struct Scene_viewer *scene_viewer,
	int width,int height,void *data);
/*******************************************************************************
LAST MODIFIED : 18 May 1998

DESCRIPTION :
Sets the RGB data in a scene viewer when buffer type
is SCENE_VIEWER_PIXEL_BUFFER.  The data is copied into the internal buffer.
It is expected to be byte sized values for each of Red Green and Blue only.
==============================================================================*/

int Scene_viewer_get_pixel_image(struct Scene_viewer *scene_viewer,
	int *width,int *height,void **data);
/*******************************************************************************
LAST MODIFIED : 18 May 1998

DESCRIPTION :
Returns RGB data grabbed from the scene viewer when buffer type
is SCENE_VIEWER_PIXEL_BUFFER.  The data is handed directly so it should
be used immediately and not DEALLOCATED.  It is expected to be byte sized
values for each of Red Green and Blue only.
==============================================================================*/

int Scene_viewer_view_all( struct Scene_viewer *scene_viewer);
/*******************************************************************************
LAST MODIFIED : 18 May 1998

DESCRIPTION :
Finds the x, y and z ranges from the scene and sets the view parameters so
that everything can be seen, and with window's std_view_angle. Also adjusts
near and far clipping planes; if specific values are required, should follow
with commands for setting these.
==============================================================================*/

int Scene_viewer_viewport_zoom(struct Scene_viewer *scene_viewer,
	double zoom_ratio);
/*******************************************************************************
LAST MODIFIED : 16 February 1998

DESCRIPTION :
Scales of the absolute image while keeping the same centre point.
==============================================================================*/

int Scene_viewer_set_input_callback(struct Scene_viewer *scene_viewer,
	struct Callback_data *callback );
/*******************************************************************************
LAST MODIFIED : 23 July 1998

DESCRIPTION :
Sets a callback that will be activated each time input is received by the 
scene_viewer.
==============================================================================*/

char *Scene_viewer_buffer_mode_string(
	enum Scene_viewer_buffer_mode buffer_mode);
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
Returns a string label for the <buffer_mode>.
NOTE: Calling function must not deallocate returned string.
==============================================================================*/

char *Scene_viewer_input_mode_string(
	enum Scene_viewer_input_mode input_mode);
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
Returns a string label for the <input_mode>.
NOTE: Calling function must not deallocate returned string.
==============================================================================*/

char *Scene_viewer_projection_mode_string(
	enum Scene_viewer_projection_mode projection_mode);
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
Returns a string label for the <projection_mode>.
NOTE: Calling function must not deallocate returned string.
==============================================================================*/

char *Scene_viewer_transparency_mode_string(
	enum Scene_viewer_transparency_mode transparency_mode);
/*******************************************************************************
LAST MODIFIED : 23 November 1998

DESCRIPTION :
Returns a string label for the <transparency_mode>.
NOTE: Calling function must not deallocate returned string.
==============================================================================*/

char *Scene_viewer_viewport_mode_string(
	enum Scene_viewer_viewport_mode viewport_mode);
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
Returns a string label for the <viewport_mode>.
NOTE: Calling function must not deallocate returned string.
==============================================================================*/
#endif /* !defined (SCENE_VIEWER_H) */
