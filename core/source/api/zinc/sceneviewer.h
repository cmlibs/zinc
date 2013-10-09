/**
 * FILE : sceneviewer.h
 *
 * The public interface to the cmzn_sceneviewer object for rendering scenes.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_SCENEVIEWER_H__
#define CMZN_SCENEVIEWER_H__

#include "types/fieldid.h"
#include "types/fieldimageid.h"
#include "types/scenefilterid.h"
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
 * other features unavailable on a single buffer scene viewer.  Secifying
 * ANY_BUFFER_MODE will mean that with SINGLE_BUFFER or DOUBLE_BUFFER mode may
 * be selected depending on the other requirements of the scene viewer.
 * The special modes RENDER_OFFSCREEN_AND_COPY and RENDER_OFFSCREEN_AND_BLEND
 * are used when an OpenGL context cannot be activated directly on the supplied
 * window, such as when the graphics are to be composited by an external program.
 * These are currently only implemeneted for winapi.
 * The graphics will be drawn offscreen and only rendered on screen when requested,
 * such as with the cmzn_sceneviewer_handle_windows_event.  The COPY version will
 * overwrite any existing pixels when drawing and the BLEND version will use the
 * alpha channel of the rendered scene to blend itself with the existing pixels.
 */
enum cmzn_sceneviewer_buffering_mode
{
	CMZN_SCENEVIEWER_BUFFERING_INVALID,
	CMZN_SCENEVIEWER_BUFFERING_ANY_MODE,
	CMZN_SCENEVIEWER_BUFFERING_SINGLE,
	CMZN_SCENEVIEWER_BUFFERING_DOUBLE,
	CMZN_SCENEVIEWER_BUFFERING_RENDER_OFFSCREEN_AND_COPY,
	CMZN_SCENEVIEWER_BUFFERING_RENDER_OFFSCREEN_AND_BLEND
};

/**
 * Controls the way the mouse and keyboard are used to interact with the scene viewer.
 */
enum cmzn_sceneviewer_interact_mode
{
	CMZN_SCENEVIEWER_INTERACT_INVALID,
	CMZN_SCENEVIEWER_INTERACT_STANDARD,
		/*!< CMZN_SCENEVIEWER_INTERACT_STANDARD is the traditional cmgui mode.
		 *   Rotate: Left mouse button
		 *   Translate: Middle mouse button
		 *   Zoom: Right mouse button */
	CMZN_SCENEVIEWER_INTERACT_2D
		/*!< CMZN_SCENEVIEWER_INTERACT_2D is a mode more suitable for 2D use
		 *   Translate: Left mouse button
		 *   Rotate: Middle mouse button
		 *   Zoom: Right mouse button */
};

/**
 * Specifies whether a STEREO capable scene viewer is required.  This will
 * have to work in cooperation with your window manager and hardware.
 */
enum cmzn_sceneviewer_stereo_mode
{
	CMZN_SCENEVIEWER_STEREO_INVALID,
	CMZN_SCENEVIEWER_STEREO_ANY_MODE,
		/*!< either STEREO or MONO depending on other scene viewer requirements */
	CMZN_SCENEVIEWER_STEREO_MONO,
		/*!< Normal 2-D Monoscopic display */
	CMZN_SCENEVIEWER_STEREO_STEREO
		/*!< Stereoscopic display */
};

/**
 * Specifies the behaviour of the NDC coordinates with respect to the size of the
 * viewport.
 */
enum cmzn_sceneviewer_viewport_mode
{
	CMZN_SCENEVIEWER_VIEWPORT_INVALID,
	CMZN_SCENEVIEWER_VIEWPORT_ABSOLUTE,
		/*!< viewport_pixels_per_unit values are used to give and exact mapping from
		 *   user coordinates to pixels. */
	CMZN_SCENEVIEWER_VIEWPORT_RELATIVE,
		/*!< the intended viewing volume is made as large as possible in the physical
		 *   viewport while maintaining the aspect ratio from NDC_width and NDC_height. */
	CMZN_SCENEVIEWER_VIEWPORT_DISTORTING_RELATIVE
		/*!< the intended viewing volume is made as large as possible in the physical
		 *   viewport, and the aspect ratio may be changed. */
};

/**
 * Specifies the sort of projection matrix used to render the 3D scene.
 */
enum cmzn_sceneviewer_projection_mode
{
	CMZN_SCENEVIEWER_PROJECTION_INVALID,
	CMZN_SCENEVIEWER_PROJECTION_PARALLEL,
	CMZN_SCENEVIEWER_PROJECTION_PERSPECTIVE
};

/**
 * Scene viewer GL blending mode.
 */
enum cmzn_sceneviewer_blending_mode
{
	CMZN_SCENEVIEWER_BLENDING_INVALID,
	CMZN_SCENEVIEWER_BLENDING_NORMAL,
		/*!< src=GL_SRC_ALPHA and dest=GL_ONE_MINUS_SRC_ALPHA */
	CMZN_SCENEVIEWER_BLENDING_NONE,
		/*!<  */
	CMZN_SCENEVIEWER_BLENDING_TRUE_ALPHA
		/*!< Available for OpenGL version 1.4 and above:
		 * src=GL_SRC_ALPHA and dest=GL_ONE_MINUS_SRC_ALPHA for rgb,
		 * src=GL_ONE and dest=GL_ONE_MINUS_SRC_ALPHA for alpha,
		 * which results in the correct final alpha value in a saved image. */
};

/*
Global functions
----------------
*/

/**
 * Returns a new reference to the scene viewer with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param sceneviewer  The scene viewer to obtain a new reference to.
 * @return  New scene viewer reference with incremented reference count.
 */
ZINC_API cmzn_sceneviewer_id cmzn_sceneviewer_access(cmzn_sceneviewer_id sceneviewer);

/**
 * Destroys this reference to the scene viewer (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param sceneviewer_address  The address to the handle of the scene viewer
 *    to be destroyed.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_sceneviewer_destroy(cmzn_sceneviewer_id *sceneviewer_address);

/**
 * Gets the mouse and keyboard interaction mode of the scene viewer.
 * @see cmzn_sceneviewer_interact_mode
 *
 * @param sceneviewer  The scene viewer to query.
 * @return  The interact mode or CMZN_SCENEVIEWER_INTERACT_INVALID on error.
 */
ZINC_API enum cmzn_sceneviewer_interact_mode cmzn_sceneviewer_get_interact_mode(
	cmzn_sceneviewer_id sceneviewer);

/**
 * Sets the mouse and keyboard interaction mode of the scene viewer.
 * @see cmzn_sceneviewer_interact_mode
 *
 * @param sceneviewer  The scene viewer to modify.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_sceneviewer_set_interact_mode(cmzn_sceneviewer_id sceneviewer,
	enum cmzn_sceneviewer_interact_mode interact_mode);

/**
 * Set the eye position of the scene viewer.
 *
 * @param sceneviewer  Handle to the scene viewer.
 * @param eye  Array of three values containing the new eye position.
 * @return Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_sceneviewer_set_eye_position(cmzn_sceneviewer_id sceneviewer,
	double const *eye);

/**
 * Get the eye position of the scene viewer.
 *
 * @param  sceneviewer  Handle to the scene viewer.
 * @param  eye  Array of size 3 to hold the values of the eye position.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_sceneviewer_get_eye_position(cmzn_sceneviewer_id sceneviewer,
	double *eye);

/**
 * Set the lookat position of the scene viewer.
 *
 * @param sceneviewer  Handle to the scene viewer.
 * @param lookat  Array of three values containing the new lookat position.
 * @return Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_sceneviewer_set_lookat_position(cmzn_sceneviewer_id sceneviewer,
	double const *lookat);

/**
 * Get the lookat position of the scene viewer.
 *
 * @param  sceneviewer  Handle to the scene viewer.
 * @param  lookat  Array of size 3 to hold the values of the lookat position.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_sceneviewer_get_lookat_position(cmzn_sceneviewer_id sceneviewer,
	double *lookat);

/**
 * Set the up vector of the scene viewer.
 *
 * @param sceneviewer  Handle to the scene viewer.
 * @param upVector  Array of three values containing the new up vector.
 * @return Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_sceneviewer_set_up_vector(cmzn_sceneviewer_id sceneviewer,
	double const *upVector);

/**
 * Get the up vector of the scene viewer.
 *
 * @param  sceneviewer  Handle to the scene viewer.
 * @param  upVector  Array of size 3 to hold the values of the up vector.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_sceneviewer_get_up_vector(cmzn_sceneviewer_id sceneviewer,
	double *upVector);

/**
 * Gets the view direction and orientation of the scene viewer.
 */
ZINC_API int cmzn_sceneviewer_get_lookat_parameters(
	cmzn_sceneviewer_id sceneviewer,
	double *eyex,double *eyey,double *eyez,
	double *lookatx,double *lookaty,double *lookatz,
	double *upx,double *upy,double *upz);

/**
 * Normal function for controlling Scene_viewer_set_lookat_parameters that ensures
 * the up vector is orthogonal to the view direction - so projection is not skew.
 *
 */
ZINC_API int cmzn_sceneviewer_set_lookat_parameters_non_skew(
	cmzn_sceneviewer_id sceneviewer,double eyex,double eyey,double eyez,
	double lookatx,double lookaty,double lookatz,
	double upx,double upy,double upz);

/**
 * Gets the distance from the eye_point to the <near> clip plane and to the <far>
 * clip plane in the scene viewer.
 */
ZINC_API int cmzn_sceneviewer_get_near_and_far_plane(cmzn_sceneviewer_id sceneviewer,
	double *near_plane, double *far_plane);

/**
 * Sets the distance from the eye_point to the <near> clip plane and to the <far>
 * clip plane in the scene viewer.
 */
ZINC_API int cmzn_sceneviewer_set_near_and_far_plane(cmzn_sceneviewer_id sceneviewer,
	double near_plane, double far_plane);

/**
 * Gets the viewport mode (absolute/relative/distorting relative) for the
 * scene viewer.
 *
 * @param sceneviewer  The scene viewer to query.
 * @return  The viewport mode or CMZN_SCENEVIEWER_VIEWPORT_INVALID on error.
 */
ZINC_API enum cmzn_sceneviewer_viewport_mode cmzn_sceneviewer_get_viewport_mode(
	cmzn_sceneviewer_id sceneviewer);

/**
 * Sets the viewport mode(absolute/relative/distorting relative) for the
 * scene viewer.
 *
 * @param sceneviewer  The scene viewer to modify.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_sceneviewer_set_viewport_mode(cmzn_sceneviewer_id sceneviewer,
	enum cmzn_sceneviewer_viewport_mode viewport_mode);

/**
 * Sets the width and height of the scene viewer's drawing area.
 */
ZINC_API int cmzn_sceneviewer_set_viewport_size(cmzn_sceneviewer_id sceneviewer,
	int width, int height);

/**
 * Gets the projection mode - parallel/perspective - of the scene viewer.
 *
 * @param sceneviewer  The scene viewer to query.
 * @return  The viewport mode or CMZN_SCENEVIEWER_PROJECTION_INVALID on error.
 */
ZINC_API enum cmzn_sceneviewer_projection_mode
	cmzn_sceneviewer_get_projection_mode(cmzn_sceneviewer_id sceneviewer);

/**
 * Sets the projection mode - parallel/perspective - of the scene viewer.
 *
 * @param sceneviewer  The scene viewer to modify.
 * @param projection_mode  The new projection mode.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_sceneviewer_set_projection_mode(cmzn_sceneviewer_id sceneviewer,
	enum cmzn_sceneviewer_projection_mode projection_mode);

/**
 * Gets the blending mode of the scene viewer.
 * @see cmzn_sceneviewer_blending_mode
 *
 * @param sceneviewer  The scene viewer to query.
 * @return  The blending mode or CMZN_SCENEVIEWER_BLENDING_INVALID on error.
 */
ZINC_API enum cmzn_sceneviewer_blending_mode cmzn_sceneviewer_get_blending_mode(
	cmzn_sceneviewer_id sceneviewer);

/**
 * Sets the blending mode of the scene viewer.
 * @see cmzn_sceneviewer_blending_mode
 *
 * @param sceneviewer  The scene viewer to modify.
 * @param blending_mode  The new blending mode.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_sceneviewer_set_blending_mode(cmzn_sceneviewer_id sceneviewer,
	enum cmzn_sceneviewer_blending_mode blending_mode);

/**
 * Gets the diagonal view angle, in radians, of the scene viewer.
 * View angle is measured across the largest square which fits inside the viewing
 * window.
 *
 * @param sceneviewer  The scene viewer to query.
 * @return  The view angle in radians, or 0 if invalid arguments.
 */
ZINC_API double cmzn_sceneviewer_get_view_angle(cmzn_sceneviewer_id sceneviewer);

/**
 * Sets the diagonal view angle, in radians, of the scene viewer.
 * View angle is measured across the largest square which fits inside the viewing
 * window.
 *
 * @param sceneviewer  The scene viewer to modify.
 * @param view_angle  The view angle in radians, > 0.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_sceneviewer_set_view_angle(cmzn_sceneviewer_id sceneviewer,
	double view_angle);

/**
 * Set the width of the graphics buffer.
 */
ZINC_API int cmzn_sceneviewer_set_graphics_buffer_width(cmzn_sceneviewer_id sceneviewer,
	unsigned int width);

/**
 * Set the height of the graphics buffer.
 */
ZINC_API int cmzn_sceneviewer_set_graphics_buffer_height(cmzn_sceneviewer_id sceneviewer,
	unsigned int height);

/**
 * Gets the number of jitter samples used to antialias the scene viewer.
 *
 * @param sceneviewer  The scene viewer to query.
 * @return  The number of jitter samples used to antialias the graphics, or
 * 0 if antialiasing is off or bad argument.
 */
ZINC_API int cmzn_sceneviewer_get_antialias_sampling(
	cmzn_sceneviewer_id sceneviewer);

/**
 * Sets the number of jitter samples used to antialias the scene viewer.
 *
 * @param sceneviewer  The scene viewer to modify.
 * @param number_of_samples  The number of jitter samples used to antialias the
 * graphics. Only 2, 4 or 8 samples are supported. Zero disables antialiasing.
 * @return  On success CMZN_OK, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_sceneviewer_set_antialias_sampling(
	cmzn_sceneviewer_id sceneviewer, int number_of_samples);

/**
 * Get the depth of field and focal depth of the scene viewer.
 */
ZINC_API int cmzn_sceneviewer_get_depth_of_field(cmzn_sceneviewer_id sceneviewer,
	double *depth_of_field, double *focal_depth);

/**
 * Set a simulated <depth_of_field> for the scene viewer.
 * If <depth_of_field> is 0, then this is disabled, essentially an infinite depth.
 * Otherwise, <depth_of_field> is a normalised length in z space, so 1 is a
 * significant value, 0.1 is a small value causing significant distortion.
 * The <focal_depth> is depth in normalised device coordinates, -1 at near plane
 * and +1 at far plane.  At this <focal_depth> the image is in focus no matter
 * how small the <depth_of_field>.
 */
ZINC_API int cmzn_sceneviewer_set_depth_of_field(cmzn_sceneviewer_id sceneviewer,
	double depth_of_field, double focal_depth);

/**
 * Query whether lines are perturbed to appear in front of surfaces at the same
 * depth to avoid visual stitching artefacts.
 *
 * @param sceneviewer  The scene viewer to query.
 * @return  Boolean true if perturbing lines, otherwise false.
 */
ZINC_API bool cmzn_sceneviewer_get_perturb_lines_flag(
	cmzn_sceneviewer_id sceneviewer);

/**
 * Set whether lines are perturbed to appear in front of surfaces at the same
 * depth to avoid visual stitching artefacts. Uses GL_EXT_polygon_offset.
 * Note that quality of surface-only graphics can be impacted with this mode.
 *
 * @param sceneviewer  The scene viewer to modify.
 * @param value  New value of perturb lines flag.
 * @return  On success CMZN_OK, otherwise CMZN_ERROR_ARGUMENT.
*/
ZINC_API int cmzn_sceneviewer_set_perturb_lines_flag(
	cmzn_sceneviewer_id sceneviewer, bool value);

/**
 * Returns the background_colour of the scene viewer.
 * The component order in the array is [red, green, blue]
 *
 * @param sceneviewer The handle to the scene viewer
 * @param valuesOut3 The rgb components of the colour with values between [0, 1.0]
 * @return CMZN_OK if successful, any other value otherwise
 */
ZINC_API int cmzn_sceneviewer_get_background_colour_rgb(
	cmzn_sceneviewer_id sceneviewer, double *valuesOut3);

/**
 * Sets the background_colour of the scene viewer by individual component.
 * Each component should be in the range [0, 1.0].
 *
 * @param sceneviewer The handle to the scene viewer
 * @param red The red component value between [0, 1.0]
 * @param green The green component value between [0, 1.0]
 * @param blue The blue component value between [0, 1.0]
 * @return CMZN_OK if successful, any other value otherwise
 */
ZINC_API int cmzn_sceneviewer_set_background_colour_component_rgb(
	cmzn_sceneviewer_id sceneviewer, double red, double green, double blue);

/**
 * Sets the background_colour of the scene viewer.
 * Each component should be in the range [0, 1.0].
 * The component order is [red, green, blue]
 *
 * @param sceneviewer The handle to the scene viewer
 * @param valuesIn3 The rgb components of the colour
 * @return CMZN_OK if successful, any other value otherwise
 */
ZINC_API int cmzn_sceneviewer_set_background_colour_rgb(
	cmzn_sceneviewer_id sceneviewer, const double *valuesIn3);

/**
 * If there is a background_texture in the scene viewer, these values specify the
 * top,left corner, in user coordinates, where it will be displayed, while the
 * next two parameters specify the size it will have in these coordinates.
 * If the bk_texture_undistort_on flag is set, radial distortion parameters from the background texture are un-distorted when the
 * texture is displayed. It does this by drawing it as a collection of polygons;
 * the last parameter controls the size of polygons used to do this.
 */
ZINC_API int cmzn_sceneviewer_set_background_texture_info(cmzn_sceneviewer_id sceneviewer,
	double bk_texture_left,double bk_texture_top,
	double bk_texture_width,double bk_texture_height,
	int bk_texture_undistort_on,double bk_texture_max_pixels_per_polygon);

/**
 * Finds the x, y and z ranges from the scene and sets the view parameters so
 * that everything can be seen, and with window's std_view_angle. Also adjusts
 * near and far clipping planes; if specific values are required, should follow
 * with commands for setting these.
 */
ZINC_API int cmzn_sceneviewer_view_all(cmzn_sceneviewer_id sceneviewer);

/**
 * Forces a redraw of the given scene viewer to take place immediately
 */
ZINC_API int cmzn_sceneviewer_render_scene(cmzn_sceneviewer_id sceneviewer);

/**
 * Gets the rate at which translation occurs in relation to mouse movement using
 * standard sceneviewerinput transformation processing.
 *
 * @param sceneviewer  The scene viewer to query.
 * @return  The translation rate, or 0 if invalid arguments.
 */
ZINC_API double cmzn_sceneviewer_get_translation_rate(cmzn_sceneviewer_id sceneviewer);

/**
 * Sets the rate at which translation occurs in relation to mouse movement using
 * standard sceneviewerinput transformation processing.
 *
 * @param sceneviewer  The scene viewer to modify.
 * @param translation_rate  The translation rate;
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_sceneviewer_set_translation_rate(cmzn_sceneviewer_id sceneviewer,
	double translation_rate);

/**
 * Gets the rate at which tumble occurs in relation to mouse movement using
 * standard sceneviewerinput transformation processing.
 *
 * @param sceneviewer  The scene viewer to query.
 * @return  The tumble rate, or 0 if invalid arguments.
 */
ZINC_API double cmzn_sceneviewer_get_tumble_rate(cmzn_sceneviewer_id sceneviewer);

/**
 * Sets the rate at which tumble occurs in relation to mouse movement using
 * standard sceneviewerinput transformation processing.
 *
 * @param sceneviewer  The scene viewer to modify.
 * @param tumble_rate  The tumble rate;
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_sceneviewer_set_tumble_rate(cmzn_sceneviewer_id sceneviewer,
	double tumble_rate);

/**
 * Gets the rate at which zoom occurs in relation to mouse movement using
 * standard sceneviewerinput transformation processing.
 *
 * @param sceneviewer  The scene viewer to query.
 * @return  The zoom rate, or 0 if invalid arguments.
 */
ZINC_API double cmzn_sceneviewer_get_zoom_rate(cmzn_sceneviewer_id sceneviewer);

/**
 * Sets the rate at which zoom occurs in relation to mouse movement using
 * standard sceneviewerinput transformation processing.
 *
 * @param sceneviewer  The scene viewer to modify.
 * @param zoom_rate  The zoom rate;
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_sceneviewer_set_zoom_rate(cmzn_sceneviewer_id sceneviewer,
	double zoom_rate);

/**
 * Returns the scene viewer freespin tool tumble angle.
 */
ZINC_API int cmzn_sceneviewer_get_freespin_tumble_angle(cmzn_sceneviewer_id sceneviewer,
	double *tumble_angle);

/**
 * Sets the scene viewer freespin tool tumble angle.
 */
ZINC_API int cmzn_sceneviewer_set_freespin_tumble_angle(cmzn_sceneviewer_id sceneviewer,
	double tumble_angle);

/**
 * Gets the scene viewer tumble axis.  The <tumble_axis> is the vector
 * about which the scene is turning relative to its lookat point.
 */
ZINC_API int cmzn_sceneviewer_get_freespin_tumble_axis(cmzn_sceneviewer_id sceneviewer,
	double *tumble_axis);

/**
 * Sets the scene viewer spinning in idle time.  The <tumble_axis> is the vector
 * about which the scene is turning relative to its lookat point and the
 * <tumble_angle> controls how much it turns on each redraw.
 */
ZINC_API int cmzn_sceneviewer_start_freespin(cmzn_sceneviewer_id sceneviewer,
	double *tumble_axis, double tumble_angle);

/**
 * Tells the scene viewer to stop all automatic informations that it produces,
 * eg. automatic tumble.
 */
ZINC_API int cmzn_sceneviewer_stop_animations(cmzn_sceneviewer_id sceneviewer);

/**
 * Writes the view in the scene viewer to the specified filename.
 * If <preferred_width>, <preferred_height>, <preferred_antialias> or
 * <preferred_transparency_layers> are non zero then they attempt to override the
 * default values for just this write.  The width and height cannot be overridden
 * when the <force_onscreen> flag is set.
 */
ZINC_API int cmzn_sceneviewer_write_image_to_file(cmzn_sceneviewer_id sceneviewer,
	const char *file_name, int force_onscreen, int preferred_width,
	int preferred_height, int preferred_antialias, int preferred_transparency_layers);

/**
 * Gets the NDC information.
 */
ZINC_API int cmzn_sceneviewer_get_NDC_info(cmzn_sceneviewer_id sceneviewer,
	double *NDC_left,double *NDC_top,double *NDC_width,double *NDC_height);

/**
 * Gets the NDC information.
 */
ZINC_API int cmzn_sceneviewer_set_NDC_info(cmzn_sceneviewer_id sceneviewer,
	double NDC_left,double NDC_top,double NDC_width,double NDC_height);

/**
 * Gets the viewing volume of the scene viewer.
 */
ZINC_API int cmzn_sceneviewer_get_viewing_volume(cmzn_sceneviewer_id sceneviewer,
	double *left,double *right,double *bottom,double *top,double *near_plane,
	double *far_plane);

/**
 * Sets the viewing volume of the scene viewer. Unless the viewing volume is the
 * same shape as the window, taking into account the aspect, the scene viewer will
 * enlarge it to maintain the desired aspect ratio. Hence, the values specified
 * represent the minimum viewing volume. The left, right, bottom and top values
 * are at the lookat point, not on the near plane as OpenGL assumes. This gives a
 * similar sized viewing_volume for both parallel and perspective projections.
 * The viewing volume can be made unsymmetric to create special effects such as
 * rendering a higher resolution image in parts.
 */
ZINC_API int cmzn_sceneviewer_set_viewing_volume(cmzn_sceneviewer_id sceneviewer,
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
ZINC_API int cmzn_sceneviewer_get_frame_pixels(cmzn_sceneviewer_id  sceneviewer,
	enum cmzn_streaminformation_image_pixel_format storage, int *width, int *height,
	int preferred_antialias, int preferred_transparency_layers,
	unsigned char **frame_data, int force_onscreen);

/**
 * Add the callback <function> with <user_data> to scene viewer.
 */
ZINC_API int cmzn_sceneviewer_add_transform_callback(
	cmzn_sceneviewer_id sceneviewer,
	cmzn_sceneviewer_callback function,void *user_data);

/**
 * Removes the callback calling <function> with <user_data> from
 * scene viewer.
 */
ZINC_API int cmzn_sceneviewer_remove_transform_callback(
	cmzn_sceneviewer_id sceneviewer,
	cmzn_sceneviewer_callback function,void *user_data);

/**
 * Destroys this handle to the scene viewer inpit, and sets it to NULL.
 *
 * @param input_address  The address to the handle of the input
 *    to be destroyed.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
//-- ZINC_API int cmzn_sceneviewerinput_destroy(cmzn_sceneviewerinput_id *input_address);

/**
 * Manually calls the scene viewer's list of input callbacks with the supplied
 * input data.
 *
 * @param sceneviewer  Handle to cmzn_sceneviewer object.
 * @param input_data  Description of the input event.
 * @return  Status CMZN_OK on success, any other value if failed.
 */
ZINC_API int cmzn_sceneviewer_process_sceneviewerinput(cmzn_sceneviewer_id sceneviewer,
	cmzn_sceneviewerinput_id input_data);

/**
 * Adds callback <function> that will be activated each time input is received
 * by the scene viewer.
 * If <add_first> is true (non zero) then this callback will be added to the
 * front of the list.
 * When a callback event is generated the list is processed as long as each
 * callback function returns true, so to stop processing and not call any more
 * of the callbacks registered after your handler then return false.
 */
ZINC_API int cmzn_sceneviewer_add_input_callback(
	cmzn_sceneviewer_id sceneviewer,
	cmzn_sceneviewerinput_callback function,
	void *user_data, int add_first);

/**
 * Remove the input callback <function> with <user_data> from scene viewer.
 */
ZINC_API int cmzn_sceneviewer_remove_input_callback(
	cmzn_sceneviewer_id sceneviewer,
	cmzn_sceneviewerinput_callback function,
	void *user_data);

/**
 * Set the scene for the scene viewer.
 */
ZINC_API int cmzn_sceneviewer_set_scene(cmzn_sceneviewer_id sceneviewer,
	cmzn_scene_id scene);

ZINC_API cmzn_scene_id cmzn_sceneviewer_get_scene(cmzn_sceneviewer_id sceneviewer);

/**
 * Get the filter currently used in scene viewer.
 *
 * @param sceneviewer  scene viewer to get the filters from.
 * @return  filter if successful, otherwise NULL.
 */
ZINC_API cmzn_scenefilter_id cmzn_sceneviewer_get_scenefilter(
	cmzn_sceneviewer_id sceneviewer);

/**
 * Set the filter to be used in scene viewer. All graphics will be shown
 * if no filter is set.
 *
 * @param sceneviewer  Scene viewer to set filter for.
 * @param filter  Scene filter to set for scene viewer, or NULL to show all
 * graphics in scene.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_sceneviewer_set_scenefilter(cmzn_sceneviewer_id sceneviewer,
	cmzn_scenefilter_id filter);

/**
 * This callback will be notified when a repaint is required by a windowless mode
 * sceneviewer, so that the host application can do the redraw.
 */
ZINC_API int cmzn_sceneviewer_add_repaint_required_callback(
	cmzn_sceneviewer_id sceneviewer,
	cmzn_sceneviewer_callback function,void *user_data);

/**
 * Removes the callback calling <function> with <user_data> from
 * scene viewer.
 */
ZINC_API int cmzn_sceneviewer_remove_repaint_required_callback(
	cmzn_sceneviewer_id sceneviewer,
	cmzn_sceneviewer_callback function,void *user_data);

/**
 * Returns a count of the number of scene viewer redraws.
 */
ZINC_API unsigned int cmzn_sceneviewer_get_frame_count(cmzn_sceneviewer_id sceneviewer);

/**
 * Returns a handle to a scene viewer object.  The scene viewer attributes for
 * buffering mode and stereo mode are also set.
 *
 * @param sceneviewermodule  Handle to a scene viewer module object.
 * @param buffer_mode  The buffering mode in use for the OpenGL context.
 * @param stereo_mode  The stereo mode in use for the OpenGL context.
 * @return  A handle to a scene viewer if successfully created otherwise 0.
 */
ZINC_API cmzn_sceneviewer_id cmzn_sceneviewermodule_create_sceneviewer(
	cmzn_sceneviewermodule_id sceneviewermodule,
	enum cmzn_sceneviewer_buffering_mode buffer_mode,
	enum cmzn_sceneviewer_stereo_mode stereo_mode);

/**
 * Returns a new reference to the scene viewer module with the reference counter
 * incremented.  The caller is responsible for destroying the new reference.
 *
 * @param sceneviewermodule  The scene viewer module to obtain a reference to.
 * @return  New scene viewer module reference with incremented reference count.
 */
ZINC_API cmzn_sceneviewermodule_id cmzn_sceneviewermodule_access(cmzn_sceneviewermodule_id sceneviewermodule);

/**
 * Destroys the scene viewer module and sets the pointer to 0.
 *
 * @param sceneviewermodule_address  The pointer to the handle of the scene viewer module.
 * @return  Status CMZN_OK if handle successfully destroyed, otherwise any other value.
 */
ZINC_API int cmzn_sceneviewermodule_destroy(cmzn_sceneviewermodule_id *sceneviewermodule_address);

/**
 * Controls the way partially transparent objects are rendered in scene viewer.
 */
enum cmzn_sceneviewer_transparency_mode
{
	CMZN_SCENEVIEWER_TRANSPARENCY_INVALID = 0,
	CMZN_SCENEVIEWER_TRANSPARENCY_FAST = 1,
	/*!< CMZN_CMZN_SCENEVIEWER_TRANSPARENCY_FAST just includes
	 * transparent objects in the normal render, this causes them
	 * to obscure other objects behind if they are drawn first.
	 */
	CMZN_SCENEVIEWER_TRANSPARENCY_SLOW = 2,
	/*!< CMZN_CMZN_SCENEVIEWER_TRANSPARENCY_SLOW puts out all the
	 * opaque geometry first and then ignores the depth test while
	 * drawing all partially transparent objects, this ensures everything
	 * is drawn but multiple layers of transparency will always draw
	 * on top of each other which means a surface that is behind another
	 * may be drawn over the top of one that is supposed to be in front.
	 */
	CMZN_SCENEVIEWER_TRANSPARENCY_ORDER_INDEPENDENT = 3
	/*!< CMZN_CMZN_SCENEVIEWER_TRANSPARENCY_ORDER_INDEPENDENT uses
	 * some Nvidia extensions to implement a full back to front perl pixel
	 * fragment sort correctly rendering transparency with a small number
	 * of passes, specified by "transparency layers". This uses all the
	 * texturing resources of the current Nvidia hardware and so
	 * no materials used in the scene can contain textures.
	 */
};

/**
 * Get the transparency_mode of the scene viewer. In fast transparency mode,
 * the scene is drawn as it is, with depth buffer writing even for semi-transparent
 * objects. In slow transparency mode, opaque objects are rendered first, then
 * semi-transparent objects are rendered without writing the depth buffer. Hence,
 * you can even see through the first semi-transparent surface drawn.
 *
 * @See cmzn_sceneviewer_transparency_mode
 * @See cmzn_sceneviewer_set_transparency_mode
 *
 * @param sceneviewer  Handle to the scene viewer.
 *
 * @return  transparency_mode set for this sceneviewer.
 *   CMZN_SCENEVIEWER_TRANSPARENCY_ORDER_INDEPENDENT if failed or
 *   mode is not set correctly
 */
ZINC_API enum cmzn_sceneviewer_transparency_mode cmzn_sceneviewer_get_transparency_mode(
	cmzn_sceneviewer_id sceneviewer);

/**
 * Set the transparency_mode of the scene viewer.
 *
 * @See cmzn_sceneviewer_transparency_mode
 * @See cmzn_sceneviewer_get_transparency_mode
 *
 * @param sceneviewer  Handle to the scene viewer.
 * @param transparency_mode  Transparency mode to be set for sceneviewer
 *
 * @return  CMZN_OK if value is set successfully, any other value if
 * 	failed.
 */
ZINC_API int cmzn_sceneviewer_set_transparency_mode(cmzn_sceneviewer_id sceneviewer,
	enum cmzn_sceneviewer_transparency_mode transparency_mode);


/**
 * Get the number of layers used in the CMZN_SCENEVIEWER_TRANSPARENCY_ORDER_INDEPENDENT
 * transparency_mode.
 *
 * @See cmzn_sceneviewer_transparency_mode
 *
 * @param sceneviewer  The scene viewer to query.
 * @return  number of layers for this scene viewer. Any otehr value if failed or
 *   it is not set correctly.
 */
ZINC_API int cmzn_sceneviewer_get_transparency_layers(cmzn_sceneviewer_id sceneviewer);

/**
 * Set the number of layers used in the CMZN_SCENEVIEWER_TRANSPARENCY_ORDER_INDEPENDENT
 * transparency_mode.
 *
 * @See cmzn_sceneviewer_transparency_mode
 *
 * @param sceneviewer  The scene viewer to modify.
 * @param layers  number of layers to be set for this scene viewer.
 *
 * @return  CMZN_OK if value is set successfully, any other value if
 * 	failed.
 */
ZINC_API int cmzn_sceneviewer_set_transparency_layers(cmzn_sceneviewer_id sceneviewer,
	int layers);

#ifdef __cplusplus
}
#endif

#endif
