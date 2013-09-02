/*******************************************************************************
FILE : sceneviewer.h

LAST MODIFIED : 7 November 2007

DESCRIPTION :
The public interface to the cmzn_scene_viewer object for rendering cmiss
scenes.
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
#ifndef CMZN_SCENEVIEWER_H__
#define CMZN_SCENEVIEWER_H__

#include "types/fieldid.h"
#include "types/fieldimageid.h"
#include "types/graphicsfilterid.h"
#include "types/interactivetoolid.h"
#include "types/sceneid.h"
#include "types/sceneviewerid.h"
#include "types/sceneid.h"
#include "types/scenecoordinatesystem.h"
#include "types/sceneviewerinputid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Describes the buffering mode of the scene viewer.  A DOUBLE_BUFFER allows the
 * graphics to be drawn offscreen before being displayed all at once, reducing the
 * apparent flicker.  A SINGLE_BUFFER may allow you a greater colour depth or
 * other features unavailable on a single buffer scene_viewer.  Secifying
 * ANY_BUFFER_MODE will mean that with SINGLE_BUFFER or DOUBLE_BUFFER mode may
 * be selected depending on the other requirements of the scene_viewer.
 * The special modes RENDER_OFFSCREEN_AND_COPY and RENDER_OFFSCREEN_AND_BLEND
 * are used when an OpenGL context cannot be activated directly on the supplied
 * window, such as when the graphics are to be composited by an external program.
 * These are currently only implemeneted for winapi.
 * The graphics will be drawn offscreen and only rendered on screen when requested,
 * such as with the cmzn_scene_viewer_handle_windows_event.  The COPY version will
 * overwrite any existing pixels when drawing and the BLEND version will use the
 * alpha channel of the rendered scene to blend itself with the existing pixels.
 */
enum cmzn_scene_viewer_buffering_mode
{
	CMZN_SCENE_VIEWER_BUFFERING_ANY_MODE,
	CMZN_SCENE_VIEWER_BUFFERING_SINGLE,
	CMZN_SCENE_VIEWER_BUFFERING_DOUBLE,
	CMZN_SCENE_VIEWER_BUFFERING_RENDER_OFFSCREEN_AND_COPY,
	CMZN_SCENE_VIEWER_BUFFERING_RENDER_OFFSCREEN_AND_BLEND
};

/**
 * Controls the way the mouse and keyboard are used to interact with the scene viewer.
 * CMZN_SCENE_VIEWER_INTERACT_STANDARD is the traditional cmgui mode.
 *   Rotate: Left mouse button
 *   Translate: Middle mouse button
 *   Zoom: Right mouse button
 * CMZN_SCENE_VIEWER_INTERACT_2D is a mode more suitable for 2D use
 *   Translate: Left mouse button
 *   Rotate: Middle mouse button
 *   Zoom: Right mouse button
 */
enum cmzn_scene_viewer_interact_mode
{
	CMZN_SCENE_VIEWER_INTERACT_STANDARD,
	CMZN_SCENE_VIEWER_INTERACT_2D
};

/**
 * Specifies whether a STEREO capable scene viewer is required.  This will
 * have to work in cooperation with your window manager and hardware.
 * ANY_STEREO_MODE means that either STEREO or MONO will may be chosen
 * depending on the other requirements of the scene_viewer.
 */
enum cmzn_scene_viewer_stereo_mode
{
	CMZN_SCENE_VIEWER_STEREO_ANY_MODE,
	CMZN_SCENE_VIEWER_STEREO_MONO,
	CMZN_SCENE_VIEWER_STEREO_STEREO
};

/**
 * Specifies the behaviour of the NDC co-ordinates with respect to the size of the
 * viewport.
 * In RELATIVE viewport mode the intended viewing volume is made as large as
 * possible in the physical viewport while maintaining the aspect ratio from
 * NDC_width and NDC_height. In ABSOLUTE viewport mode viewport_pixels_per_unit
 * values are used to give and exact mapping from user coordinates to pixels.
 * In DISTORTING_RELATIVE viewport mode the intended viewing volume is made as
 * large as possible in the physical viewport, and the aspect ratio may be
 * changed.
 */
enum cmzn_scene_viewer_viewport_mode
{
	CMZN_SCENE_VIEWER_VIEWPORT_ABSOLUTE,
	CMZN_SCENE_VIEWER_VIEWPORT_RELATIVE,
	CMZN_SCENE_VIEWER_VIEWPORT_DISTORTING_RELATIVE
};

/**
 * Specifies the sort of projection matrix used to render the 3D scene.
 */
enum cmzn_scene_viewer_projection_mode
{
	CMZN_SCENE_VIEWER_PROJECTION_PARALLEL,
	CMZN_SCENE_VIEWER_PROJECTION_PERSPECTIVE
};

/**
 * SCENE_VIEWER_BLEND_NORMAL is src=GL_SRC_ALPHA and dest=GL_ONE_MINUS_SRC_ALPHA
 * SCENE_VIEWER_BLEND_TRUE_ALPHA is available for OpenGL version 1.4 and above
 * and is src=GL_SRC_ALPHA and dest=GL_ONE_MINUS_SRC_ALPHA
 * for rgb and src=GL_ONE and dest=GL_ONE_MINUS_SRC_ALPHA for alpha, which
 * results in the correct final alpha value in a saved image.
 */
enum cmzn_scene_viewer_blending_mode
{
	CMZN_SCENE_VIEWER_BLENDING_NORMAL,
	CMZN_SCENE_VIEWER_BLENDING_NONE,
	CMZN_SCENE_VIEWER_BLENDING_TRUE_ALPHA
};

/**
 * Specifies the scene viewer input event type.
 */
enum cmzn_scene_viewer_input_event_type
{
	CMZN_SCENE_VIEWER_INPUT_INVALID,
	CMZN_SCENE_VIEWER_INPUT_MOTION_NOTIFY,
	CMZN_SCENE_VIEWER_INPUT_BUTTON_PRESS,
	CMZN_SCENE_VIEWER_INPUT_BUTTON_RELEASE,
	CMZN_SCENE_VIEWER_INPUT_KEY_PRESS,
	CMZN_SCENE_VIEWER_INPUT_KEY_RELEASE
};

/**
 * Specifies the scene viewer input modifier.
 */
enum cmzn_scene_viewer_input_modifier_flags
{
	CMZN_SCENE_VIEWER_INPUT_MODIFIER_NONE = 0,
	CMZN_SCENE_VIEWER_INPUT_MODIFIER_SHIFT = 1,
	CMZN_SCENE_VIEWER_INPUT_MODIFIER_CONTROL = 2,
	CMZN_SCENE_VIEWER_INPUT_MODIFIER_ALT = 4,
	CMZN_SCENE_VIEWER_INPUT_MODIFIER_BUTTON1 = 8
};

/*
Global functions
----------------
*/

/**
 * Returns a new reference to the scene viewer with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param scene_viewer  The scene_viewer to obtain a new reference to.
 * @return  New scene_viewer reference with incremented reference count.
 */
ZINC_API cmzn_scene_viewer_id cmzn_scene_viewer_access(cmzn_scene_viewer_id scene_viewer);

/**
 * Destroys this reference to the scene_viewer (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param scene_viewer_id_address  The address to the handle of the scene_viewer
 *    to be destroyed.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_scene_viewer_destroy(cmzn_scene_viewer_id *scene_viewer_id_address);

/**
 * Returns the mouse and keyboard interaction mode of the Scene_viewer.
 * See the definition of the
 * cmzn_scene_viewer_interact_mode enumerator.
 */
ZINC_API int cmzn_scene_viewer_get_interact_mode(cmzn_scene_viewer_id scene_viewer,
 enum cmzn_scene_viewer_interact_mode *interact_mode);

/**
 * Sets the interaction mode of the Scene_viewer.  See the definition of the
cmzn_scene_viewer_interact_mode enumerator.

 */
ZINC_API int cmzn_scene_viewer_set_interact_mode(cmzn_scene_viewer_id scene_viewer,
	enum cmzn_scene_viewer_interact_mode interact_mode);

/**
 * Set the eye position of the scene viewer.
 *
 * @param scene_viewer  Handle to the scene viewer.
 * @param eye  Array of three values containing the new eye position.
 * @return Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_scene_viewer_set_eye_position(cmzn_scene_viewer_id scene_viewer,
	double const *eye);

/**
 * Get the eye position of the scene viewer.
 *
 * @param  scene_viewer  Handle to the scene viewer.
 * @param  eye  Array of size 3 to hold the values of the eye position.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_scene_viewer_get_eye_position(cmzn_scene_viewer_id scene_viewer,
	double *eye);

/**
 * Set the lookat position of the scene viewer.
 *
 * @param scene_viewer  Handle to the scene viewer.
 * @param lookat  Array of three values containing the new lookat position.
 * @return Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_scene_viewer_set_lookat_position(cmzn_scene_viewer_id scene_viewer,
	double const *lookat);

/**
 * Get the lookat position of the scene viewer.
 *
 * @param  scene_viewer  Handle to the scene viewer.
 * @param  lookat  Array of size 3 to hold the values of the lookat position.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_scene_viewer_get_lookat_position(cmzn_scene_viewer_id scene_viewer,
	double *lookat);

/**
 * Set the up vector of the scene viewer.
 *
 * @param scene_viewer  Handle to the scene viewer.
 * @param upVector  Array of three values containing the new up vector.
 * @return Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_scene_viewer_set_up_vector(cmzn_scene_viewer_id scene_viewer,
	double const *upVector);

/**
 * Get the up vector of the scene viewer.
 *
 * @param  scene_viewer  Handle to the scene viewer.
 * @param  upVector  Array of size 3 to hold the values of the up vector.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_scene_viewer_get_up_vector(cmzn_scene_viewer_id scene_viewer,
	double *upVector);

/**
 * Gets the view direction and orientation of the Scene_viewer.
 */
ZINC_API int cmzn_scene_viewer_get_lookat_parameters(
	cmzn_scene_viewer_id scene_viewer,
	double *eyex,double *eyey,double *eyez,
	double *lookatx,double *lookaty,double *lookatz,
	double *upx,double *upy,double *upz);

/**
 * Normal function for controlling Scene_viewer_set_lookat_parameters that ensures
 * the up vector is orthogonal to the view direction - so projection is not skew.
 *
 */
ZINC_API int cmzn_scene_viewer_set_lookat_parameters_non_skew(
	cmzn_scene_viewer_id scene_viewer,double eyex,double eyey,double eyez,
	double lookatx,double lookaty,double lookatz,
	double upx,double upy,double upz);

/**
 * Gets the distance from the eye_point to the <near> clip plane and to the <far>
 * clip plane in the <scene_viewer>.
 */
ZINC_API int cmzn_scene_viewer_get_near_and_far_plane(cmzn_scene_viewer_id scene_viewer,
	double *near_plane, double *far_plane);

/**
 * Sets the distance from the eye_point to the <near> clip plane and to the <far>
 * clip plane in the <scene_viewer>.
 */
ZINC_API int cmzn_scene_viewer_set_near_and_far_plane(cmzn_scene_viewer_id scene_viewer,
	double near_plane, double far_plane);

/**
 * Gets the viewport mode(absolute/relative/distorting relative) for the
 * <scene_viewer>.
 */
ZINC_API int cmzn_scene_viewer_get_viewport_mode(cmzn_scene_viewer_id scene_viewer,
	enum cmzn_scene_viewer_viewport_mode *viewport_mode);

/**
 * Sets the width and height of the Scene_viewers drawing area.
 */
ZINC_API int cmzn_scene_viewer_set_viewport_size(cmzn_scene_viewer_id scene_viewer,
	int width, int height);

/**
 * Sets the viewport mode(absolute/relative/distorting relative) for the
 * <scene_viewer>.
 */
ZINC_API int cmzn_scene_viewer_set_viewport_mode(cmzn_scene_viewer_id scene_viewer,
	enum cmzn_scene_viewer_viewport_mode viewport_mode);

/**
 *Returns the projection mode - parallel/perspective - of the Scene_viewer.
 */
ZINC_API int cmzn_scene_viewer_get_projection_mode(cmzn_scene_viewer_id scene_viewer,
	enum cmzn_scene_viewer_projection_mode *projection_mode);

/**
 * Sets the projection mode - parallel/perspective - of the Scene_viewer.
 */
ZINC_API int cmzn_scene_viewer_set_projection_mode(cmzn_scene_viewer_id scene_viewer,
	enum cmzn_scene_viewer_projection_mode projection_mode);

/**
 * Returns the blending mode of the Scene_viewer.  See the definition of the
 * cmzn_scene_viewer_blending_mode enumerator.
 */
ZINC_API int cmzn_scene_viewer_get_blending_mode(cmzn_scene_viewer_id scene_viewer,
 enum cmzn_scene_viewer_blending_mode *blending_mode);

/**
 * Sets the blending mode of the Scene_viewer.  See the definition of the
 * cmzn_scene_viewer_blending_mode enumerator.
 */
ZINC_API int cmzn_scene_viewer_set_blending_mode(cmzn_scene_viewer_id scene_viewer,
	enum cmzn_scene_viewer_blending_mode blending_mode);

/**
 * Gets the diagonal view angle, in radians, of the <scene_viewer>.
 * View angle is measured across the largest square which fits inside the viewing
 * window.
 */
ZINC_API int cmzn_scene_viewer_get_view_angle(cmzn_scene_viewer_id scene_viewer,
	double *view_angle);

/**
 * Sets the diagonal view angle, in radians, of the <scene_viewer>.
 * View angle is measured across the largest square which fits inside the viewing
 * window.
 */
ZINC_API int cmzn_scene_viewer_set_view_angle(cmzn_scene_viewer_id scene_viewer,
	double view_angle);

/**
 * Set the width of the graphics buffer.
 */
ZINC_API int cmzn_scene_viewer_set_graphics_buffer_width(cmzn_scene_viewer_id scene_viewer,
	unsigned int width);

/**
 * Set the height of the graphics buffer.
 */
ZINC_API int cmzn_scene_viewer_set_graphics_buffer_height(cmzn_scene_viewer_id scene_viewer,
	unsigned int height);

/**
 * Gets the number of jitter samples used to antialias the scene_viewer.
 */
ZINC_API int cmzn_scene_viewer_get_antialias_mode(cmzn_scene_viewer_id scene_viewer,
	unsigned int *antialias);

/**
 * Sets the number of jitter samples used to antialias the scene_viewer.
 * Zero turns antialiasing off.
 */
ZINC_API int cmzn_scene_viewer_set_antialias_mode(cmzn_scene_viewer_id scene_viewer,
	unsigned int antialias_mode);

/**
 * Get the depth of field and focal depth of the scene viewer.
 */
ZINC_API int cmzn_scene_viewer_get_depth_of_field(cmzn_scene_viewer_id scene_viewer,
	double *depth_of_field, double *focal_depth);

/**
 * Set a simulated <depth_of_field> for the scene_viewer.
 * If <depth_of_field> is 0, then this is disabled, essentially an infinite depth.
 * Otherwise, <depth_of_field> is a normalised length in z space, so 1 is a
 * significant value, 0.1 is a small value causing significant distortion.
 * The <focal_depth> is depth in normalised device coordinates, -1 at near plane
 * and +1 at far plane.  At this <focal_depth> the image is in focus no matter
 * how small the <depth_of_field>.
 */
ZINC_API int cmzn_scene_viewer_set_depth_of_field(cmzn_scene_viewer_id scene_viewer,
	double depth_of_field, double focal_depth);

/**
 * Returns the <perturb_lines> flag which determines whether the
 * GL_EXT_polygon_offset extension is used to offset the lines from the surfaces
 * in the z direction of the scene viewer.
 */
ZINC_API int cmzn_scene_viewer_get_perturb_lines(cmzn_scene_viewer_id scene_viewer,
	int *perturb_lines);

/**
 * When the line draw mode is turned on (set to one) the lines are raised in the
 * z direction when the GL_EXT_polygon_offset extension is available from the X
 * Server.  This means that the lines appear solid rather than interfering with a
 * surface in the same space.
 */
ZINC_API int cmzn_scene_viewer_set_perturb_lines(cmzn_scene_viewer_id scene_viewer,
	int perturb_lines);

/**
 * Returns the background_colour of the scene_viewer.
 * The component order in the array is [red, green, blue]
 *
 * @param scene_viewer The handle to the scene viewer
 * @param valuesOut3 The rgb components of the colour with values between [0, 1.0]
 * @return CMZN_OK if successful, any other value otherwise
 */
ZINC_API int cmzn_scene_viewer_get_background_colour_rgb(
	cmzn_scene_viewer_id scene_viewer, double *valuesOut3);

/**
 * Sets the background_colour of the scene_viewer by individual component.
 * Each component should be in the range [0, 1.0].
 *
 * @param scene_viewer The handle to the scene viewer
 * @param red The red component value between [0, 1.0]
 * @param green The green component value between [0, 1.0]
 * @param blue The blue component value between [0, 1.0]
 * @return CMZN_OK if successful, any other value otherwise
 */
ZINC_API int cmzn_scene_viewer_set_background_colour_component_rgb(
	cmzn_scene_viewer_id scene_viewer, double red, double green, double blue);

/**
 * Sets the background_colour of the scene_viewer.
 * Each component should be in the range [0, 1.0].
 * The component order is [red, green, blue]
 *
 * @param scene_viewer The handle to the scene viewer
 * @param valuesIn3 The rgb components of the colour
 * @return CMZN_OK if successful, any other value otherwise
 */
ZINC_API int cmzn_scene_viewer_set_background_colour_rgb(
	cmzn_scene_viewer_id scene_viewer, const double *valuesIn3);

/**
 * If there is a background_texture in the scene_viewer, these values specify the
 * top,left corner, in user coordinates, where it will be displayed, while the
 * next two parameters specify the size it will have in these coordinates.
 * If the bk_texture_undistort_on flag is set, radial distortion parameters from the background texture are un-distorted when the
 * texture is displayed. It does this by drawing it as a collection of polygons;
 * the last parameter controls the size of polygons used to do this.
 */
ZINC_API int cmzn_scene_viewer_set_background_texture_info(cmzn_scene_viewer_id scene_viewer,
	double bk_texture_left,double bk_texture_top,
	double bk_texture_width,double bk_texture_height,
	int bk_texture_undistort_on,double bk_texture_max_pixels_per_polygon);

/**
 * Finds the x, y and z ranges from the scene and sets the view parameters so
 * that everything can be seen, and with window's std_view_angle. Also adjusts
 * near and far clipping planes; if specific values are required, should follow
 * with commands for setting these.
 */
ZINC_API int cmzn_scene_viewer_view_all(cmzn_scene_viewer_id scene_viewer);

/**
 * Forces a redraw of the given scene viewer to take place immediately
 */
ZINC_API int cmzn_scene_viewer_render_scene(cmzn_scene_viewer_id scene_viewer);

/**
 * Gets the scene viewer translation rate.
 */
ZINC_API int cmzn_scene_viewer_get_translation_rate(cmzn_scene_viewer_id scene_viewer,
	double *translation_rate);

/**
 * Sets the scene viewer translation rate.
 */
ZINC_API int cmzn_scene_viewer_set_translation_rate(cmzn_scene_viewer_id scene_viewer,
	double translation_rate);

/**
 * Gets the scene viewer tumble rate.
 */
ZINC_API int cmzn_scene_viewer_get_tumble_rate(cmzn_scene_viewer_id scene_viewer,
	double *tumble_rate);

/**
 * Sets the scene viewer tumble rate.
 */
ZINC_API int cmzn_scene_viewer_set_tumble_rate(cmzn_scene_viewer_id scene_viewer,
	double tumble_rate);

/**
 * Gets the scene viewer zoom rate.
 */
ZINC_API int cmzn_scene_viewer_get_zoom_rate(cmzn_scene_viewer_id scene_viewer,
	double *zoom_rate);

/**
 * Sets the scene viewer zoom rate.
 */
ZINC_API int cmzn_scene_viewer_set_zoom_rate(cmzn_scene_viewer_id scene_viewer,
	double zoom_rate);

/**
 * Returns the scene viewer freespin tool tumble angle.
 */
ZINC_API int cmzn_scene_viewer_get_freespin_tumble_angle(cmzn_scene_viewer_id scene_viewer,
	double *tumble_angle);

/**
 * Sets the <scene_viewer> freespin tool tumble angle.
 */
ZINC_API int cmzn_scene_viewer_set_freespin_tumble_angle(cmzn_scene_viewer_id scene_viewer,
	double tumble_angle);

/**
 * Gets the <scene_viewer> tumble axis.  The <tumble_axis> is the vector
 * about which the scene is turning relative to its lookat point.
 */
ZINC_API int cmzn_scene_viewer_get_freespin_tumble_axis(cmzn_scene_viewer_id scene_viewer,
	double *tumble_axis);

/**
 * Sets the <scene_viewer> spinning in idle time.  The <tumble_axis> is the vector
 * about which the scene is turning relative to its lookat point and the
 * <tumble_angle> controls how much it turns on each redraw.
 */
ZINC_API int cmzn_scene_viewer_start_freespin(cmzn_scene_viewer_id scene_viewer,
	double *tumble_axis, double tumble_angle);

/**
 * Tells the <scene_viewer> to stop all automatic informations that it produces,
 * eg. automatic tumble.
 */
ZINC_API int cmzn_scene_viewer_stop_animations(cmzn_scene_viewer_id scene_viewer);

/**
 * Writes the view in the scene_viewer to the specified filename.
 * If <preferred_width>, <preferred_height>, <preferred_antialias> or
 * <preferred_transparency_layers> are non zero then they attempt to override the
 * default values for just this write.  The width and height cannot be overridden
 * when the <force_onscreen> flag is set.
 */
ZINC_API int cmzn_scene_viewer_write_image_to_file(cmzn_scene_viewer_id scene_viewer,
	const char *file_name, int force_onscreen, int preferred_width,
	int preferred_height, int preferred_antialias, int preferred_transparency_layers);

/**
 * Gets the NDC information.
 */
ZINC_API int cmzn_scene_viewer_get_NDC_info(cmzn_scene_viewer_id scene_viewer,
	double *NDC_left,double *NDC_top,double *NDC_width,double *NDC_height);

/**
 * Gets the NDC information.
 */
ZINC_API int cmzn_scene_viewer_set_NDC_info(cmzn_scene_viewer_id scene_viewer,
	double NDC_left,double NDC_top,double NDC_width,double NDC_height);

/**
 * Gets the viewing volume of the scene viewer.
 */
ZINC_API int cmzn_scene_viewer_get_viewing_volume(cmzn_scene_viewer_id scene_viewer,
	double *left,double *right,double *bottom,double *top,double *near_plane,
	double *far_plane);

/**
 * Sets the viewing volume of the Scene_viewer. Unless the viewing volume is the
 * same shape as the window, taking into account the aspect, the Scene_viewer will
 * enlarge it to maintain the desired aspect ratio. Hence, the values specified
 * represent the minimum viewing volume. The left, right, bottom and top values
 * are at the lookat point, not on the near plane as OpenGL assumes. This gives a
 * similar sized viewing_volume for both parallel and perspective projections.
 * The viewing volume can be made unsymmetric to create special effects such as
 * rendering a higher resolution image in parts.
 */
ZINC_API int cmzn_scene_viewer_set_viewing_volume(cmzn_scene_viewer_id scene_viewer,
	double left,double right,double bottom,double top,double near_plane,double far_plane);

/**
 * Returns the contents of the scene viewer as pixels.  <width> and <height>
 * will be respected if the window is drawn offscreen and they are non zero,
 * otherwise they are set in accordance with current size of the scene viewer.
 * If <preferred_antialias> or <preferred_transparency_layers> are non zero then they
 * attempt to override the default values for just this call.
 * If <force_onscreen> is non zero then the pixels will always be grabbed from the
 * scene viewer on screen.
 */
ZINC_API int cmzn_scene_viewer_get_frame_pixels(cmzn_scene_viewer_id  scene_viewer,
	enum cmzn_stream_information_image_pixel_format storage, int *width, int *height,
	int preferred_antialias, int preferred_transparency_layers,
	unsigned char **frame_data, int force_onscreen);

/**
 * Add the callback <function> with <user_data> to <scene_viewer>.
 */
ZINC_API int cmzn_scene_viewer_add_transform_callback(
	cmzn_scene_viewer_id scene_viewer,
	cmzn_scene_viewer_callback function,void *user_data);

/**
 * Removes the callback calling <function> with <user_data> from
 * <scene_viewer>.
 */
ZINC_API int cmzn_scene_viewer_remove_transform_callback(
	cmzn_scene_viewer_id scene_viewer,
	cmzn_scene_viewer_callback function,void *user_data);

/**
 * Destroys this handle to the scene viewer inpit, and sets it to NULL.
 *
 * @param input_address  The address to the handle of the input
 *    to be destroyed.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
//-- ZINC_API int cmzn_scene_viewer_input_destroy(cmzn_scene_viewer_input_id *input_address);

/**
 * Manually calls the scene viewer's list of input callbacks with the supplied
 * input data.
 *
 * @param scene_viewer  Handle to cmzn_scene_viewer object.
 * @param input_data  Description of the input event.
 * @return  Status CMZN_OK on success, any other value if failed.
 */
ZINC_API int cmzn_scene_viewer_process_input(cmzn_scene_viewer_id scene_viewer,
	cmzn_scene_viewer_input_id input_data);

/**
 * Adds callback <function> that will be activated each time input is received
 * by the scene_viewer.
 * If <add_first> is true (non zero) then this callback will be added to the
 * front of the list.
 * When a callback event is generated the list is processed as long as each
 * callback function returns true, so to stop processing and not call any more
 * of the callbacks registered after your handler then return false.
 */
ZINC_API int cmzn_scene_viewer_add_input_callback(
	cmzn_scene_viewer_id scene_viewer,
	cmzn_scene_viewer_input_callback function,
	void *user_data, int add_first);

/**
 * Remove the input callback <function> with <user_data> from <scene_viewer>.
 */
ZINC_API int cmzn_scene_viewer_remove_input_callback(
	cmzn_scene_viewer_id scene_viewer,
	cmzn_scene_viewer_input_callback function,
	void *user_data);

/**
 * Set the scene for the scene viewer.
 */
ZINC_API int cmzn_scene_viewer_set_scene(cmzn_scene_viewer_id scene_viewer,
	cmzn_scene_id scene);

ZINC_API cmzn_scene_id cmzn_scene_viewer_get_scene(cmzn_scene_viewer_id scene_viewer);

/**
 * Set the filter to be used in <scene_viewer>. All graphics will be shown
 * until a filter showing graphic is set.
 *
 * @param scene viewer  Scene viewer to set filter for.
 * @param filter  Filter to be set for scene viewer.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_scene_viewer_set_filter(cmzn_scene_viewer_id scene_viewer,
	cmzn_graphics_filter_id filter);

/**
 * Get the filter currently used in <scene_viewer>.
 *
 * @param scene_viewer  Scene_viewer to get the filters from.
 * @return  filter if successful, otherwise NULL.
 */
ZINC_API cmzn_graphics_filter_id cmzn_scene_viewer_get_filter(
	cmzn_scene_viewer_id scene_viewer);

/**
 * This callback will be notified when a repaint is required by a windowless mode
 * scene_viewer, so that the host application can do the redraw.
 */
ZINC_API int cmzn_scene_viewer_add_repaint_required_callback(
	cmzn_scene_viewer_id scene_viewer,
	cmzn_scene_viewer_callback function,void *user_data);

/**
 * Removes the callback calling <function> with <user_data> from
 * <scene_viewer>.
 */
ZINC_API int cmzn_scene_viewer_remove_repaint_required_callback(
	cmzn_scene_viewer_id scene_viewer,
	cmzn_scene_viewer_callback function,void *user_data);

/**
 * Returns a count of the number of scene viewer redraws.
 */
ZINC_API unsigned int cmzn_scene_viewer_get_frame_count(cmzn_scene_viewer_id scene_viewer);

/**
 * Returns a handle to a scene viewer object.  The scene viewer attributes for
 * buffering mode and stereo mode are also set.
 *
 * @param scene_viewer_module  Handle to a scene_viewer_module object.
 * @param buffer_mode  The buffering mode in use for the OpenGL context.
 * @param stereo_mode  The stereo mode in use for the OpenGL context.
 * @return  A handle to a scene viewer if successfully created otherwise 0.
 */
ZINC_API cmzn_scene_viewer_id cmzn_scene_viewer_module_create_scene_viewer(
	cmzn_scene_viewer_module_id scene_viewer_module,
	enum cmzn_scene_viewer_buffering_mode buffer_mode,
	enum cmzn_scene_viewer_stereo_mode stereo_mode);

/**
 * Returns a new reference to the scene viewer module with the reference counter
 * incremented.  The caller is responsible for destroying the new reference.
 *
 * @param scene_viewer_module  The scene viewer module to obtain a reference to.
 * @return New scene viewer module reference with incremented reference count.
 */
ZINC_API cmzn_scene_viewer_module_id cmzn_scene_viewer_module_access(cmzn_scene_viewer_module_id scene_viewer_module);

/**
 * Destroys the scene viewer module and sets the pointer to 0.
 *
 * @param scene_viewer_module_address  The pointer to the handle of the scene viewer module.
 * @return  Status CMZN_OK if handle successfully destroyed, otherwise any other value.
 */
ZINC_API int cmzn_scene_viewer_module_destroy(cmzn_scene_viewer_module_id *scene_viewer_module_address);

/**
 * Controls the way partially transparent objects are rendered in scene viewer.
 */
enum cmzn_scene_viewer_transparency_mode
{
	CMZN_SCENE_VIEWER_TRANSPARENCY_INVALID = 0,
	CMZN_SCENE_VIEWER_TRANSPARENCY_FAST = 1,
	/*!< CMZN_CMZN_SCENE_VIEWER_TRANSPARENCY_FAST just includes
	 * transparent objects in the normal render, this causes them
	 * to obscure other objects behind if they are drawn first.
	 */
	CMZN_SCENE_VIEWER_TRANSPARENCY_SLOW = 2,
	/*!< CMZN_CMZN_SCENE_VIEWER_TRANSPARENCY_SLOW puts out all the
	 * opaque geometry first and then ignores the depth test while
	 * drawing all partially transparent objects, this ensures everything
	 * is drawn but multiple layers of transparency will always draw
	 * on top of each other which means a surface that is behind another
	 * may be drawn over the top of one that is supposed to be in front.
	 */
	CMZN_SCENE_VIEWER_TRANSPARENCY_ORDER_INDEPENDENT = 3
	/*!< CMZN_CMZN_SCENE_VIEWER_TRANSPARENCY_ORDER_INDEPENDENT uses
	 * some Nvidia extensions to implement a full back to front perl pixel
	 * fragment sort correctly rendering transparency with a small number
	 * of passes, specified by "transparency layers". This uses all the
	 * texturing resources of the current Nvidia hardware and so
	 * no materials used in the scene can contain textures.
	 */
};


/**
 * Get the transparency_mode of the Scene_viewer. In fast transparency mode,
 * the scene is drawn as it is, with depth buffer writing even for semi-transparent
 * objects. In slow transparency mode, opaque objects are rendered first, then
 * semi-transparent objects are rendered without writing the depth buffer. Hence,
 * you can even see through the first semi-transparent surface drawn.
 *
 * @See cmzn_scene_viewer_transparency_mode
 * @See cmzn_scene_viewer_set_transparency_mode
 *
 * @param scene_viewer  Handle to the scene_viewer.
 *
 * @return  transparency_mode set for this scene_viewer.
 *   CMZN_SCENE_VIEWER_TRANSPARENCY_ORDER_INDEPENDENT if failed or
 *   mode is not set correctly
 */
ZINC_API enum cmzn_scene_viewer_transparency_mode cmzn_scene_viewer_get_transparency_mode(
	cmzn_scene_viewer_id scene_viewer);

/**
 * Set the transparency_mode of the Scene_viewer.
 *
 * @See cmzn_scene_viewer_transparency_mode
 * @See cmzn_scene_viewer_get_transparency_mode
 *
 * @param scene_viewer  Handle to the scene_viewer.
 * @param transparency_mode  Transparency mode to be set for scene_viewer
 *
 * @return  CMZN_OK if value is set successfully, any other value if
 * 	failed.
 */
ZINC_API int cmzn_scene_viewer_set_transparency_mode(cmzn_scene_viewer_id scene_viewer,
	enum cmzn_scene_viewer_transparency_mode transparency_mode);


/**
 * Get the number of layers used in the CMZN_SCENE_VIEWER_TRANSPARENCY_ORDER_INDEPENDENT
 * transparency_mode.
 *
 * @See cmzn_scene_viewer_transparency_mode
 *
 * @param scene_viewer  Handle to the zinc scene_viewer component.
 *
 * @return  number of layers for this scene viewer. Any otehr value if failed or
 *   it is not set correctly.
 */
ZINC_API int cmzn_scene_viewer_get_transparency_layers(cmzn_scene_viewer_id scene_viewer);

/**
 * Set the number of layers used in the CMZN_SCENE_VIEWER_TRANSPARENCY_ORDER_INDEPENDENT
 * transparency_mode.
 *
 * @See cmzn_scene_viewer_transparency_mode
 *
 * @param scene_viewer  Handle to the zinc scene_viewer component.
 * @param layers  number of layers to be set for this scene viewer.
 *
 * @return  CMZN_OK if value is set successfully, any other value if
 * 	failed.
 */
ZINC_API int cmzn_scene_viewer_set_transparency_layers(cmzn_scene_viewer_id scene_viewer,
	int layers);

#ifdef __cplusplus
}
#endif

#endif
