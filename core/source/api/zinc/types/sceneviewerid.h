/**
 * FILE : sceneviewerid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_SCENEVIEWERID_H__
#define CMZN_SCENEVIEWERID_H__

struct cmzn_sceneviewermodule;
typedef struct cmzn_sceneviewermodule * cmzn_sceneviewermodule_id;

struct cmzn_sceneviewer;
typedef struct cmzn_sceneviewer *cmzn_sceneviewer_id;

/* The cmzn_sceneviewerinput describes the input event */
typedef int (*cmzn_sceneviewerinput_callback)(
	cmzn_sceneviewer_id sceneviewer,
	struct cmzn_sceneviewerinput *, void *user_data);

typedef void (*cmzn_sceneviewer_callback)(cmzn_sceneviewer_id sceneviewer,
	void *callback_data, void *user_data);

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
	CMZN_SCENEVIEWER_BUFFERING_MODE_INVALID,
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
	CMZN_SCENEVIEWER_INTERACT_MODE_INVALID,
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
	CMZN_SCENEVIEWER_STEREO_MODE_INVALID,
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
	CMZN_SCENEVIEWER_VIEWPORT_MODE_INVALID,
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
	CMZN_SCENEVIEWER_PROJECTION_MODE_INVALID,
	CMZN_SCENEVIEWER_PROJECTION_PARALLEL,
	CMZN_SCENEVIEWER_PROJECTION_PERSPECTIVE
};

/**
 * Scene viewer GL blending mode.
 */
enum cmzn_sceneviewer_blending_mode
{
	CMZN_SCENEVIEWER_BLENDING_MODE_INVALID,
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

#endif
