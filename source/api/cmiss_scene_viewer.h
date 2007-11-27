/*******************************************************************************
FILE : cmiss_scene_viewer.h

LAST MODIFIED : 7 November 2007

DESCRIPTION :
The public interface to the Cmiss_scene_viewer object for rendering cmiss
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
#ifndef __CMISS_SCENE_VIEWER_H__
#define __CMISS_SCENE_VIEWER_H__

#if defined (GTK_USER_INTERFACE)
#include <gtk/gtk.h>
#endif /* defined (GTK_USER_INTERFACE) */
#if defined (CARBON_USER_INTERFACE)
#include <carbon/carbon.h>
#endif /* defined (CARBON_USER_INTERFACE) */
#if defined (WIN32_USER_INTERFACE)
#include <windows.h>
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (MOTIF)
#include <Xm/Xm.h>
#endif /* defined (MOTIF) */

#include "cmiss_texture.h"

/*
Global types
------------
*/
struct Cmiss_scene_viewer_package;

enum Cmiss_scene_viewer_buffering_mode
/*******************************************************************************
LAST MODIFIED : 16 September 2002

DESCRIPTION :
Describes the buffering mode of the scene viewer.  A DOUBLE_BUFFER allows the
graphics to be drawn offscreen before being displayed all at once, reducing the
apparent flicker.  A SINGLE_BUFFER may allow you a greater colour depth or 
other features unavailable on a single buffer scene_viewer.  Secifying
ANY_BUFFER_MODE will mean that with SINGLE_BUFFER or DOUBLE_BUFFER mode may
be selected depending on the other requirements of the scene_viewer.
==============================================================================*/
{
	CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE,
	CMISS_SCENE_VIEWER_BUFFERING_SINGLE,
	CMISS_SCENE_VIEWER_BUFFERING_DOUBLE
};

enum Cmiss_scene_viewer_interact_mode
/*******************************************************************************
LAST MODIFIED : 2 November 2006

DESCRIPTION :
Controls the way the mouse and keyboard are used to interact with the scene viewer.
CMISS_SCENE_VIEWER_INTERACT_STANDARD is the traditional cmgui mode. 
  Rotate: Left mouse button 
  Translate: Middle mouse button
  Zoom: Right mouse button
CMISS_SCENE_VIEWER_INTERACT_2D is a mode more suitable for 2D use
  Translate: Left mouse button
  Rotate: Middle mouse button 
  Zoom: Right mouse button
==============================================================================*/
{
	CMISS_SCENE_VIEWER_INTERACT_STANDARD,
	CMISS_SCENE_VIEWER_INTERACT_2D
};

enum Cmiss_scene_viewer_stereo_mode
/*******************************************************************************
LAST MODIFIED : 16 September 2002

DESCRIPTION :
Specifies whether a STEREO capable scene viewer is required.  This will
have to work in cooperation with your window manager and hardware.
ANY_STEREO_MODE means that either STEREO or MONO will may be chosen
depending on the other requirements of the scene_viewer.
==============================================================================*/
{
	CMISS_SCENE_VIEWER_STEREO_ANY_MODE,
	CMISS_SCENE_VIEWER_STEREO_MONO,
	CMISS_SCENE_VIEWER_STEREO_STEREO
};

enum Cmiss_scene_viewer_viewport_mode
/*******************************************************************************
LAST MODIFIED : 04 February 2005

DESCRIPTION :
Specifies the behaviour of the NDC co-ordinates with respect to the size of the
viewport.
In RELATIVE viewport mode the intended viewing volume is made as large as
possible in the physical viewport while maintaining the aspect ratio from
NDC_width and NDC_height. In ABSOLUTE viewport mode viewport_pixels_per_unit
values are used to give and exact mapping from user coordinates to pixels.
In DISTORTING_RELATIVE viewport mode the intended viewing volume is made as
large as possible in the physical viewport, and the aspect ratio may be
changed.
==============================================================================*/
{
	CMISS_SCENE_VIEWER_VIEWPORT_ABSOLUTE,
	CMISS_SCENE_VIEWER_VIEWPORT_RELATIVE,
	CMISS_SCENE_VIEWER_VIEWPORT_DISTORTING_RELATIVE
};

enum Cmiss_scene_viewer_projection_mode
/*******************************************************************************
LAST MODIFIED : 16 September 2002

DESCRIPTION :
Specifies the sort of projection matrix used to render the 3D scene.
==============================================================================*/
{
	CMISS_SCENE_VIEWER_PROJECTION_PARALLEL,
	CMISS_SCENE_VIEWER_PROJECTION_PERSPECTIVE
};

enum Cmiss_scene_viewer_transparency_mode
/*******************************************************************************
LAST MODIFIED : 13 September 2002

DESCRIPTION :
Controls the way partially transparent objects are rendered in scene viewer.
CMISS_SCENE_VIEWER_FAST_TRANSPARENCY just includes transparent objects in the
normal render, this causes them to obscure other objects behind if they are
drawn first.
CMISS_SCENE_VIEWER_SLOW_TRANSPARENCY puts out all the opaque geometry first
and then ignores the depth test while drawing all partially transparent objects,
this ensures everything is drawn but multiple layers of transparency will 
always draw on top of each other which means a surface that is behind another
may be drawn over the top of one that is supposed to be in front.
CMISS_SCENE_VIEWER_LAYERED_TRANSPARENCY divides the viewing volume between the
near and far clip planes into "transparency_layers" slices and renders each
slice from back to front.  This is very slow and can still have artefacts at 
the edges of the layers.  Best use of the slices is made if the near and far
clip planes are tight around the objects in the scene.
CMISS_SCENE_VIEWER_ORDER_INDEPENDENT_TRANSPARENCY uses some Nvidia extensions
to implement a full back to front perl pixel fragment sort correctly rendering
transparency with a small number of passes, specified by "transparency layers".
This uses all the texturing resources of the current Nvidia hardware and so
no materials used in the scene can contain textures.
==============================================================================*/
{
	CMISS_SCENE_VIEWER_TRANSPARENCY_FAST,
	CMISS_SCENE_VIEWER_TRANSPARENCY_SLOW,
	CMISS_SCENE_VIEWER_TRANSPARENCY_LAYERED,
	CMISS_SCENE_VIEWER_TRANSPARENCY_ORDER_INDEPENDENT
};

enum Cmiss_scene_viewer_blending_mode
/*******************************************************************************
LAST MODIFIED : 

DESCRIPTION :
SCENE_VIEWER_BLEND_NORMAL is src=GL_SRC_ALPHA and dest=GL_ONE_MINUS_SRC_ALPHA
SCENE_VIEWER_BLEND_TRUE_ALPHA is available for OpenGL version 1.4 and above
and is src=GL_SRC_ALPHA and dest=GL_ONE_MINUS_SRC_ALPHA
for rgb and src=GL_ONE and dest=GL_ONE_MINUS_SRC_ALPHA for alpha, which 
results in the correct final alpha value in a saved image.
==============================================================================*/
{
	CMISS_SCENE_VIEWER_BLENDING_NORMAL,
	CMISS_SCENE_VIEWER_BLENDING_NONE,
	CMISS_SCENE_VIEWER_BLENDING_TRUE_ALPHA
};

typedef struct Cmiss_scene_viewer *Cmiss_scene_viewer_id;

typedef void (*Cmiss_scene_viewer_callback)(Cmiss_scene_viewer_id scene_viewer,
	void *callback_data, void *user_data);

enum Cmiss_scene_viewer_input_event_type
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
==============================================================================*/
{
	CMISS_SCENE_VIEWER_INPUT_MOTION_NOTIFY,
	CMISS_SCENE_VIEWER_INPUT_BUTTON_PRESS,
	CMISS_SCENE_VIEWER_INPUT_BUTTON_RELEASE,
	CMISS_SCENE_VIEWER_INPUT_KEY_PRESS,
	CMISS_SCENE_VIEWER_INPUT_KEY_RELEASE
};

enum Cmiss_scene_viewer_input_modifier_flags
/*******************************************************************************
LAST MODIFIED : 12 September 2007

DESCRIPTION :
==============================================================================*/
{
	CMISS_SCENE_VIEWER_INPUT_MODIFIER_SHIFT = 1,
	CMISS_SCENE_VIEWER_INPUT_MODIFIER_CONTROL = 2,
	CMISS_SCENE_VIEWER_INPUT_MODIFIER_ALT = 4,
	CMISS_SCENE_VIEWER_INPUT_MODIFIER_BUTTON1 = 8
};

struct Cmiss_scene_viewer_input;
typedef struct Cmiss_scene_viewer_input *Cmiss_scene_viewer_input_id;

typedef int (*Cmiss_scene_viewer_input_callback)(
	Cmiss_scene_viewer_id scene_viewer,
	struct Cmiss_scene_viewer_input *, void *user_data);
/* The Cmiss_scene_viewer_input describes the input event */

/*
Global functions
----------------
*/

#if defined (GTK_USER_INTERFACE)
Cmiss_scene_viewer_id create_Cmiss_scene_viewer_gtk(
	struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package,
	GtkContainer *scene_viewer_widget,
	enum Cmiss_scene_viewer_buffering_mode buffer_mode,
	enum Cmiss_scene_viewer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Creates a Cmiss_scene_viewer by creating a GtkGlArea inside the specified 
<scene_viewer_widget> container.
If <minimum_colour_buffer_depth>, <minimum_depth_buffer_depth> or 
<minimum_accumulation_buffer_depth> are not zero then they are used to filter
out the possible visuals selected for graphics_buffers.  If they are zero then 
the accumulation_buffer_depth are not tested and the maximum colour buffer depth is
chosen.
==============================================================================*/
#endif /* defined (GTK_USER_INTERFACE) */

#if defined (CARBON_USER_INTERFACE)
Cmiss_scene_viewer_id create_Cmiss_scene_viewer_Carbon(
	struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package,
	CGrafPtr port, int port_x, int port_y,
	enum Cmiss_scene_viewer_buffering_mode buffer_mode,
	enum Cmiss_scene_viewer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 27 November 2006

DESCRIPTION :
Creates a Cmiss_scene_viewer by creating a graphics buffer on the specified 
<port> window handle.
If <minimum_colour_buffer_depth>, <minimum_depth_buffer_depth> or 
<minimum_accumulation_buffer_depth> are not zero then they are used to filter
out the possible visuals selected for graphics_buffers.  If they are zero then 
the accumulation_buffer_depth are not tested and the maximum colour buffer depth is
chosen.
==============================================================================*/
#endif /* defined (CARBON_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
Cmiss_scene_viewer_id create_Cmiss_scene_viewer_win32(
	struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package,
	HWND hWnd, HDC hDC,
	enum Cmiss_scene_viewer_buffering_mode buffer_mode,
	enum Cmiss_scene_viewer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 1 June 2007

DESCRIPTION :
Creates a Cmiss_scene_viewer by creating a graphics buffer on the specified 
<hWnd> window handle.  If the <hDC> is specified it is used to render.
Alternatively if <hWnd> is NULL and <hDC> is specified then no window functions
are performed but the graphics window will render into the supplied device context.
If <minimum_colour_buffer_depth>, <minimum_depth_buffer_depth> or 
<minimum_accumulation_buffer_depth> are not zero then they are used to filter
out the possible visuals selected for graphics_buffers.  If they are zero then 
the accumulation_buffer_depth are not tested and the maximum colour buffer depth is
chosen.
==============================================================================*/
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
int Cmiss_scene_viewer_handle_windows_event(Cmiss_scene_viewer_id scene_viewer,
	UINT event,WPARAM first_message,LPARAM second_message);
/*******************************************************************************
LAST MODIFIED : 31 May 2007

DESCRIPTION:
Passes the supplied windows event on to the graphics buffer.
==============================================================================*/
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
int Cmiss_scene_viewer_win32_set_window_size(Cmiss_scene_viewer_id scene_viewer,
	int width, int height, int x, int y);
/*******************************************************************************
LAST MODIFIED : 14 September 2007

DESCRIPTION :
Sets the maximum extent of the graphics window within which individual paints 
will be requested with handle_windows_event.
==============================================================================*/
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (MOTIF)
Cmiss_scene_viewer_id create_Cmiss_scene_viewer_motif(
	struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package,
	Widget parent,
	enum Cmiss_scene_viewer_buffering_mode buffer_mode,
	enum Cmiss_scene_viewer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 25 January 2006

DESCRIPTION :
Creates a Cmiss_scene_viewer by creating a graphics buffer on the specified 
<parent> widget.
If <minimum_colour_buffer_depth>, <minimum_depth_buffer_depth> or 
<minimum_accumulation_buffer_depth> are not zero then they are used to filter
out the possible visuals selected for graphics_buffers.  If they are zero then 
the accumulation_buffer_depth are not tested and the maximum colour buffer depth is
chosen.
==============================================================================*/
#endif /* defined (MOTIF) */

int Cmiss_scene_viewer_destroy(Cmiss_scene_viewer_id *scene_viewer_id_address);
/*******************************************************************************
LAST MODIFIED : 4 September 2007

DESCRIPTION :
Closes the scene_viewer.
==============================================================================*/

#if defined (CARBON_USER_INTERFACE)
int Cmiss_scene_viewer_carbon_set_window_size(Cmiss_scene_viewer_id scene_viewer,
	int width, int height, int portx, int porty, int clip_width, int clip_height);
/*******************************************************************************
LAST MODIFIED : 16 February 2007

DESCRIPTION :
Sets the coordinates within the graphics port which the scene_viewer should
respect.
==============================================================================*/

#endif /* defined (CARBON_USER_INTERFACE) */

int Cmiss_scene_viewer_get_interact_mode(Cmiss_scene_viewer_id scene_viewer,
 enum Cmiss_scene_viewer_interact_mode *interact_mode);
/*******************************************************************************
LAST MODIFIED : 2 November 2006

DESCRIPTION :
Returns the mouse and keyboard interaction mode of the Scene_viewer.  
See the definition of the
Cmiss_scene_viewer_interact_mode enumerator.
==============================================================================*/

int Cmiss_scene_viewer_set_interact_mode(Cmiss_scene_viewer_id scene_viewer,
	enum Cmiss_scene_viewer_interact_mode interact_mode);
/*******************************************************************************
LAST MODIFIED : 2 November 2006

DESCRIPTION :
Sets the interaction mode of the Scene_viewer.  See the definition of the
Cmiss_scene_viewer_interact_mode enumerator.
==============================================================================*/

int Cmiss_scene_viewer_get_lookat_parameters(
	Cmiss_scene_viewer_id scene_viewer,
	double *eyex,double *eyey,double *eyez,
	double *lookatx,double *lookaty,double *lookatz,
	double *upx,double *upy,double *upz);
/*******************************************************************************
LAST MODIFIED : 10 September 2002

DESCRIPTION :
Gets the view direction and orientation of the Scene_viewer.
==============================================================================*/

int Cmiss_scene_viewer_set_lookat_parameters_non_skew(
	Cmiss_scene_viewer_id scene_viewer,double eyex,double eyey,double eyez,
	double lookatx,double lookaty,double lookatz,
	double upx,double upy,double upz);
/*******************************************************************************
LAST MODIFIED : 11 September 2002

DESCRIPTION :
Normal function for controlling Scene_viewer_set_lookat_parameters that ensures
the up vector is orthogonal to the view direction - so projection is not skew.
==============================================================================*/

int Cmiss_scene_viewer_get_near_and_far_plane(Cmiss_scene_viewer_id scene_viewer,
	double *near_plane, double *far_plane);
/*******************************************************************************
LAST MODIFIED : 13 September 2002

DESCRIPTION :
Gets the distance from the eye_point to the <near> clip plane and to the <far>
clip plane in the <scene_viewer>.
==============================================================================*/

int Cmiss_scene_viewer_set_near_and_far_plane(Cmiss_scene_viewer_id scene_viewer,
	double near_plane, double far_plane);
/*******************************************************************************
LAST MODIFIED : 13 September 2002

DESCRIPTION :
Sets the distance from the eye_point to the <near> clip plane and to the <far>
clip plane in the <scene_viewer>.
==============================================================================*/

int Cmiss_scene_viewer_get_viewport_mode(Cmiss_scene_viewer_id scene_viewer,
	enum Cmiss_scene_viewer_viewport_mode *viewport_mode);
/*******************************************************************************
LAST MODIFIED : 04 February 2005

DESCRIPTION :
Gets the viewport mode(absolute/relative/distorting relative) for the
<scene_viewer>.
==============================================================================*/

int Cmiss_scene_viewer_set_viewport_mode(Cmiss_scene_viewer_id scene_viewer,
	enum Cmiss_scene_viewer_viewport_mode viewport_mode);
/*******************************************************************************
LAST MODIFIED : 04 February 2005

DESCRIPTION :
Sets the viewport mode(absolute/relative/distorting relative) for the
<scene_viewer>.
==============================================================================*/

int Cmiss_scene_viewer_get_projection_mode(Cmiss_scene_viewer_id scene_viewer,
	enum Cmiss_scene_viewer_projection_mode *projection_mode);
/*******************************************************************************
LAST MODIFIED : 17 September 2002

DESCRIPTION :
Returns the projection mode - parallel/perspective - of the Scene_viewer.
==============================================================================*/

int Cmiss_scene_viewer_set_projection_mode(Cmiss_scene_viewer_id scene_viewer,
	enum Cmiss_scene_viewer_projection_mode projection_mode);
/*******************************************************************************
LAST MODIFIED : 13 September 2002

DESCRIPTION :
Sets the projection mode - parallel/perspective - of the Scene_viewer.
==============================================================================*/

int Cmiss_scene_viewer_get_transparency_mode(Cmiss_scene_viewer_id scene_viewer,
 enum Cmiss_scene_viewer_transparency_mode *transparency_mode);
/*******************************************************************************
LAST MODIFIED : 17 September 2002

DESCRIPTION :
Returns the transparency mode of the Scene_viewer.  See the definition of the
Cmiss_scene_viewer_transparency_mode enumerator.
==============================================================================*/

int Cmiss_scene_viewer_set_transparency_mode(Cmiss_scene_viewer_id scene_viewer,
	enum Cmiss_scene_viewer_transparency_mode transparency_mode);
/*******************************************************************************
LAST MODIFIED : 13 September 2002

DESCRIPTION :
Sets the transparency mode of the Scene_viewer.  See the definition of the
Cmiss_scene_viewer_transparency_mode enumerator.
==============================================================================*/

int Cmiss_scene_viewer_get_blending_mode(Cmiss_scene_viewer_id scene_viewer,
 enum Cmiss_scene_viewer_blending_mode *blending_mode);
/*******************************************************************************
LAST MODIFIED : 7 November 2007

DESCRIPTION :
Returns the blending mode of the Scene_viewer.  See the definition of the
Cmiss_scene_viewer_blending_mode enumerator.
==============================================================================*/

int Cmiss_scene_viewer_set_blending_mode(Cmiss_scene_viewer_id scene_viewer,
	enum Cmiss_scene_viewer_blending_mode blending_mode);
/*******************************************************************************
LAST MODIFIED : 7 November 2007

DESCRIPTION :
Sets the blending mode of the Scene_viewer.  See the definition of the
Cmiss_scene_viewer_blending_mode enumerator.
==============================================================================*/

int Cmiss_scene_viewer_get_transparency_layers(Cmiss_scene_viewer_id scene_viewer,
	unsigned int *transparency_layers);
/*******************************************************************************
LAST MODIFIED : 17 September 2002

DESCRIPTION :
Gets the number of layers used in the CMISS_SCENE_VIEWER_LAYERED_TRANSPARENCY
transparency_mode.  See the definition of the
Cmiss_scene_viewer_transparency_mode enumerator.
==============================================================================*/

int Cmiss_scene_viewer_set_transparency_layers(Cmiss_scene_viewer_id scene_viewer,
	unsigned int layers);
/*******************************************************************************
LAST MODIFIED : 13 September 2002

DESCRIPTION :
Sets the number of layers used in the CMISS_SCENE_VIEWER_LAYERED_TRANSPARENCY
transparency_mode.  See the definition of the
Cmiss_scene_viewer_transparency_mode enumerator.
==============================================================================*/

int Cmiss_scene_viewer_get_view_angle(Cmiss_scene_viewer_id scene_viewer,
	double *view_angle);
/*******************************************************************************
LAST MODIFIED : 10 September 2002

DESCRIPTION :
Gets the diagonal view angle, in radians, of the <scene_viewer>.
View angle is measured across the largest square which fits inside the viewing
window.
==============================================================================*/

int Cmiss_scene_viewer_set_view_angle(Cmiss_scene_viewer_id scene_viewer,
	double view_angle);
/*******************************************************************************
LAST MODIFIED : 10 September 2002

DESCRIPTION :
Sets the diagonal view angle, in radians, of the <scene_viewer>.
View angle is measured across the largest square which fits inside the viewing
window.
==============================================================================*/

int Cmiss_scene_viewer_get_antialias_mode(Cmiss_scene_viewer_id scene_viewer,
	unsigned int *antialias);
/*******************************************************************************
LAST MODIFIED : 11 September 2002

DESCRIPTION :
Gets the number of jitter samples used to antialias the scene_viewer.
==============================================================================*/

int Cmiss_scene_viewer_set_antialias_mode(Cmiss_scene_viewer_id scene_viewer,
	unsigned int antialias_mode);
/*******************************************************************************
LAST MODIFIED : 11 September 2002

DESCRIPTION :
Sets the number of jitter samples used to antialias the scene_viewer.
Zero turns antialiasing off.
==============================================================================*/

int Cmiss_scene_viewer_get_depth_of_field(Cmiss_scene_viewer_id scene_viewer,
	double *depth_of_field, double *focal_depth);
/*******************************************************************************
LAST MODIFIED : 18 December 2006

DESCRIPTION :
==============================================================================*/

int Cmiss_scene_viewer_set_depth_of_field(Cmiss_scene_viewer_id scene_viewer,
	double depth_of_field, double focal_depth);
/*******************************************************************************
LAST MODIFIED : 18 December 2006

DESCRIPTION :
Set a simulated <depth_of_field> for the scene_viewer.
If <depth_of_field> is 0, then this is disabled, essentially an infinite depth.
Otherwise, <depth_of_field> is a normalised length in z space, so 1 is a
significant value, 0.1 is a small value causing significant distortion.
The <focal_depth> is depth in normalised device coordinates, -1 at near plane
and +1 at far plane.  At this <focal_depth> the image is in focus no matter
how small the <depth_of_field>. 
==============================================================================*/

int Cmiss_scene_viewer_get_perturb_lines(Cmiss_scene_viewer_id scene_viewer,
	int *perturb_lines);
/*******************************************************************************
LAST MODIFIED : 11 September 2002

DESCRIPTION :
Returns the <perturb_lines> flag which determines whether the 
GL_EXT_polygon_offset extension is used to offset the lines from the surfaces
in the z direction of the scene viewer.
==============================================================================*/

int Cmiss_scene_viewer_set_perturb_lines(Cmiss_scene_viewer_id scene_viewer,
	int perturb_lines);
/*******************************************************************************
LAST MODIFIED : 11 September 2002

DESCRIPTION :
When the line draw mode is turned on (set to one) the lines are raised in the
z direction when the GL_EXT_polygon_offset extension is available from the X
Server.  This means that the lines appear solid rather than interfering with a
surface in the same space.
==============================================================================*/

int Cmiss_scene_viewer_get_background_colour_r_g_b(
	Cmiss_scene_viewer_id scene_viewer, double *red, double *green, double *blue);
/*******************************************************************************
LAST MODIFIED : 11 September 2002

DESCRIPTION :
Returns the background_colour of the scene_viewer.
==============================================================================*/

int Cmiss_scene_viewer_set_background_colour_r_g_b(
	Cmiss_scene_viewer_id scene_viewer, double red, double green, double blue);
/*******************************************************************************
LAST MODIFIED : 11 September 2002

DESCRIPTION :
Sets the background_colour of the scene_viewer.
==============================================================================*/

int Cmiss_scene_viewer_get_interactive_tool_name(
	Cmiss_scene_viewer_id scene_viewer, char **tool_name);
/*******************************************************************************
LAST MODIFIED : 11 September 2002

DESCRIPTION :
Returns an allocated string which specifies the name of the current
interactive_tool.  You should call Cmiss_deallocate with the returned
pointer when it is no longer required.
==============================================================================*/

int Cmiss_scene_viewer_set_interactive_tool_by_name(
	Cmiss_scene_viewer_id scene_viewer, const char *tool_name);
/*******************************************************************************
LAST MODIFIED : 11 September 2002

DESCRIPTION :
Sets the currently active interactive tool for the scene_viewer if one that
matches the <tool_name> exists.
==============================================================================*/

int Cmiss_scene_viewer_get_scene_name(Cmiss_scene_viewer_id scene_viewer,
	char **scene_name);
/*******************************************************************************
LAST MODIFIED : 11 September 2002

DESCRIPTION :
Returns an allocated string which identifies the scene currently rendered
by the <scene_viewer>.  You should call Cmiss_deallocate with the returned
pointer when it is no longer required.
==============================================================================*/

int Cmiss_scene_viewer_set_scene_by_name(Cmiss_scene_viewer_id scene_viewer,
	const char *scene_name);
/*******************************************************************************
LAST MODIFIED : 11 September 2002

DESCRIPTION :
Sets the currently rendered scene in the scene_viewer if one that
matches the <scene_name> exists.
==============================================================================*/

int Cmiss_scene_viewer_set_overlay_scene_by_name(Cmiss_scene_viewer_id scene_viewer,
	const char *scene_name);
/*******************************************************************************
LAST MODIFIED : 10 September 2003

DESCRIPTION :
Sets the overlay scene rendered in the scene_viewer if one that
matches the <scene_name> exists.
==============================================================================*/

int Cmiss_scene_viewer_set_background_texture_by_name(Cmiss_scene_viewer_id scene_viewer,
	const char *texture_name);
/*******************************************************************************
LAST MODIFIED : 10 September 2003

DESCRIPTION :
Sets the background texture rendered in the scene_viewer if one that
matches the <scene_name> exists.
==============================================================================*/

int Cmiss_scene_viewer_set_background_texture_info(Cmiss_scene_viewer_id scene_viewer,
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

int Cmiss_scene_viewer_view_all(Cmiss_scene_viewer_id scene_viewer);
/*******************************************************************************
LAST MODIFIED : 12 September 2002

DESCRIPTION :
Finds the x, y and z ranges from the scene and sets the view parameters so
that everything can be seen, and with window's std_view_angle. Also adjusts
near and far clipping planes; if specific values are required, should follow
with commands for setting these.
==============================================================================*/

int Cmiss_scene_viewer_redraw_now(Cmiss_scene_viewer_id scene_viewer);
/*******************************************************************************
LAST MODIFIED : 11 September 2002

DESCRIPTION :
Forces a redraw of the given scene viewer to take place immediately
==============================================================================*/

int Cmiss_scene_viewer_get_translation_rate(Cmiss_scene_viewer_id scene_viewer,
	double *translation_rate);
/*******************************************************************************
LAST MODIFIED : 4 February 2005

DESCRIPTION :
Gets the scene viewer translation rate.
==============================================================================*/

int Cmiss_scene_viewer_set_translation_rate(Cmiss_scene_viewer_id scene_viewer,
	double translation_rate);
/*******************************************************************************
LAST MODIFIED : 4 February 2005

DESCRIPTION :
Sets the scene viewer translation rate.
==============================================================================*/

int Cmiss_scene_viewer_get_tumble_rate(Cmiss_scene_viewer_id scene_viewer,
	double *tumble_rate);
/*******************************************************************************
LAST MODIFIED : 4 February 2005

DESCRIPTION :
Gets the scene viewer tumble rate.
==============================================================================*/

int Cmiss_scene_viewer_set_tumble_rate(Cmiss_scene_viewer_id scene_viewer,
	double tumble_rate);
/*******************************************************************************
LAST MODIFIED : 4 February 2005

DESCRIPTION :
Sets the scene viewer tumble rate.
==============================================================================*/

int Cmiss_scene_viewer_get_zoom_rate(Cmiss_scene_viewer_id scene_viewer,
	double *zoom_rate);
/*******************************************************************************
LAST MODIFIED : 4 February 2005

DESCRIPTION :
Gets the scene viewer zoom rate.
==============================================================================*/

int Cmiss_scene_viewer_set_zoom_rate(Cmiss_scene_viewer_id scene_viewer,
	double zoom_rate);
/*******************************************************************************
LAST MODIFIED : 4 February 2005

DESCRIPTION :
Sets the scene viewer zoom rate.
==============================================================================*/

int Cmiss_scene_viewer_get_freespin_tumble_angle(Cmiss_scene_viewer_id scene_viewer,
	double *tumble_angle);
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Returns the <scene_viewer> freespin tool tumble angle.
==============================================================================*/

int Cmiss_scene_viewer_set_freespin_tumble_angle(Cmiss_scene_viewer_id scene_viewer,
	double tumble_angle);
/*******************************************************************************
LAST MODIFIED : 17 February 2005

DESCRIPTION :
Sets the <scene_viewer> freespin tool tumble angle.
==============================================================================*/

int Cmiss_scene_viewer_get_freespin_tumble_axis(Cmiss_scene_viewer_id scene_viewer,
	float *tumble_axis);
/*******************************************************************************
LAST MODIFIED : 9 October 2003

DESCRIPTION :
Gets the <scene_viewer> tumble axis.  The <tumble_axis> is the vector
about which the scene is turning relative to its lookat point.
==============================================================================*/

int Cmiss_scene_viewer_start_freespin(Cmiss_scene_viewer_id scene_viewer,
	float *tumble_axis, double tumble_angle);
/*******************************************************************************
LAST MODIFIED : 10 September 2003

DESCRIPTION :
Sets the <scene_viewer> spinning in idle time.  The <tumble_axis> is the vector
about which the scene is turning relative to its lookat point and the
<tumble_angle> controls how much it turns on each redraw.
==============================================================================*/

int Cmiss_scene_viewer_stop_animations(Cmiss_scene_viewer_id scene_viewer);
/*******************************************************************************
LAST MODIFIED : 29 September 2000

DESCRIPTION :
Tells the <scene_viewer> to stop all automatic informations that it produces,
eg. automatic tumble.
==============================================================================*/

int Cmiss_scene_viewer_write_image_to_file(Cmiss_scene_viewer_id scene_viewer,
	const char *file_name, int force_onscreen, int preferred_width,
	int preferred_height, int preferred_antialias, int preferred_transparency_layers);
/*******************************************************************************
LAST MODIFIED : 18 September 2002

DESCRIPTION :
Writes the view in the scene_viewer to the specified filename.
If <preferred_width>, <preferred_height>, <preferred_antialias> or
<preferred_transparency_layers> are non zero then they attempt to override the
default values for just this write.  The width and height cannot be overridden
when the <force_onscreen> flag is set.
==============================================================================*/

int Cmiss_scene_viewer_get_NDC_info(Cmiss_scene_viewer_id scene_viewer,
	double *NDC_left,double *NDC_top,double *NDC_width,double *NDC_height);
/*******************************************************************************
LAST MODIFIED : 15 November 2005

DESCRIPTION :
Gets the NDC information.
==============================================================================*/


int Cmiss_scene_viewer_set_NDC_info(Cmiss_scene_viewer_id scene_viewer,
	double NDC_left,double NDC_top,double NDC_width,double NDC_height);
/*******************************************************************************
LAST MODIFIED : 15 November 2005

DESCRIPTION :
Gets the NDC information.
==============================================================================*/

int Cmiss_scene_viewer_get_viewing_volume(Cmiss_scene_viewer_id scene_viewer,
	double *left,double *right,double *bottom,double *top,double *near_plane,
	double *far_plane);
/*******************************************************************************
LAST MODIFIED : 18 November 1997

DESCRIPTION :
Gets the viewing volume of the Scene_viewer.
==============================================================================*/

int Cmiss_scene_viewer_set_viewing_volume(Cmiss_scene_viewer_id scene_viewer,
	double left,double right,double bottom,double top,double near_plane,double far_plane);
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

int Cmiss_scene_viewer_get_frame_pixels(Cmiss_scene_viewer_id  scene_viewer,
	enum Cmiss_texture_storage_type storage, int *width, int *height,
	int preferred_antialias, int preferred_transparency_layers,
	unsigned char **frame_data, int force_onscreen);
/*******************************************************************************
LAST MODIFIED : 18 September 2002

DESCRIPTION :
Returns the contents of the scene viewer as pixels.  <width> and <height>
will be respected if the window is drawn offscreen and they are non zero,
otherwise they are set in accordance with current size of the scene viewer.
If <preferred_antialias> or <preferred_transparency_layers> are non zero then they
attempt to override the default values for just this call.
If <force_onscreen> is non zero then the pixels will always be grabbed from the
scene viewer on screen.
==============================================================================*/

int Cmiss_scene_viewer_add_transform_callback(
	Cmiss_scene_viewer_id scene_viewer,
	Cmiss_scene_viewer_callback function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 28 August 2007

DESCRIPTION :
==============================================================================*/

int Cmiss_scene_viewer_remove_transform_callback(
	Cmiss_scene_viewer_id scene_viewer,
	Cmiss_scene_viewer_callback function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 28 August 2007

DESCRIPTION :
Removes the callback calling <function> with <user_data> from
<scene_viewer>.
==============================================================================*/

int Cmiss_scene_viewer_default_input_callback(
	Cmiss_scene_viewer_id scene_viewer,
	Cmiss_scene_viewer_input_id input_data);
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
The callback for mouse or keyboard input in the Scene_viewer window. The
resulting behaviour depends on the <scene_viewer> input_mode. In Transform mode
mouse clicks and drags are converted to transformation; in Select mode OpenGL
picking is performed with picked objects and mouse click and drag information
returned to the scene.
==============================================================================*/

int Cmiss_scene_viewer_add_input_callback(
	Cmiss_scene_viewer_id scene_viewer,
	Cmiss_scene_viewer_input_callback function,
	void *user_data, int add_first);
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
Adds callback <function> that will be activated each time input is received
by the scene_viewer.
If <add_first> is true (non zero) then this callback will be added to the 
front of the list.
When a callback event is generated the list is processed as long as each
callback function returns true, so to stop processing and not call any more
of the callbacks registered after your handler then return false.
==============================================================================*/

int Cmiss_scene_viewer_remove_input_callback(
	Cmiss_scene_viewer_id scene_viewer,
	Cmiss_scene_viewer_input_callback function,
	void *user_data);
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
Removes the callback calling <function> with <user_data> from
<scene_viewer>.
==============================================================================*/

int Cmiss_scene_viewer_input_get_event_type(
	Cmiss_scene_viewer_input_id input_data,
	enum Cmiss_scene_viewer_input_event_type *event_type);
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
Returns the type of event that <input_data> represents.
==============================================================================*/

int Cmiss_scene_viewer_input_set_event_type(
	Cmiss_scene_viewer_input_id input_data,
	enum Cmiss_scene_viewer_input_event_type event_type);
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
Sets the type of event that <input_data> represents.
==============================================================================*/

int Cmiss_scene_viewer_input_get_button_number(
	Cmiss_scene_viewer_input_id input_data);
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
Returns the button number that generated the event.
This will be 1 to 3 for a button event and 0 for a non button event.
==============================================================================*/

int Cmiss_scene_viewer_input_set_button_number(
	Cmiss_scene_viewer_input_id input_data, int button_number);
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
Sets the button number that the event represents.
1 to 3 for a button event and 0 for a non button event.
==============================================================================*/

int Cmiss_scene_viewer_input_get_key_code(
	Cmiss_scene_viewer_input_id input_data);
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
Returns the key code that generated the event.
==============================================================================*/

int Cmiss_scene_viewer_input_set_key_code(
	Cmiss_scene_viewer_input_id input_data, int key_code);
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
Sets the key code that the event represents.
==============================================================================*/

int Cmiss_scene_viewer_input_get_x_position(
	Cmiss_scene_viewer_input_id input_data);
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
Returns the x position of the mouse when the event occured in pixels from top left corner.
==============================================================================*/

int Cmiss_scene_viewer_input_set_x_position(
	Cmiss_scene_viewer_input_id input_data, int x_position);
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
Sets the x position of the mouse when the event occured in pixels from top left corner.
==============================================================================*/

int Cmiss_scene_viewer_input_get_y_position(
	Cmiss_scene_viewer_input_id input_data);
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
Returns the y position of the mouse when the event occured in pixels from top left corner.
==============================================================================*/

int Cmiss_scene_viewer_input_set_y_position(
	Cmiss_scene_viewer_input_id input_data, int y_position);
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
Sets the y position of the mouse when the event occured in pixels from top left corner.
==============================================================================*/

int Cmiss_scene_viewer_input_get_modifier_flags(
	Cmiss_scene_viewer_input_id input_data,
	enum Cmiss_scene_viewer_input_modifier_flags *modifier_flags);
/*******************************************************************************
LAST MODIFIED : 12 September 2007

DESCRIPTION :
Returns the set of bit flags showing the whether the modifier inputs
were active when the event was generated.
==============================================================================*/

int Cmiss_scene_viewer_input_set_modifier_flags(
	Cmiss_scene_viewer_input_id input_data,
	enum Cmiss_scene_viewer_input_modifier_flags modifier_flags);
/*******************************************************************************
LAST MODIFIED : 12 September 2007

DESCRIPTION :
Sets the set of bit flags showing the whether the modifier inputs
were active when the event was generated.
==============================================================================*/

int Cmiss_scene_viewer_add_repaint_required_callback(
	Cmiss_scene_viewer_id scene_viewer,
	Cmiss_scene_viewer_callback function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 20 September 2007

DESCRIPTION :
This callback will be notified when a repaint is required by a windowless mode
scene_viewer, so that the host application can do the redraw.
==============================================================================*/

int Cmiss_scene_viewer_remove_repaint_required_callback(
	Cmiss_scene_viewer_id scene_viewer,
	Cmiss_scene_viewer_callback function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 28 August 2007

DESCRIPTION :
Removes the callback calling <function> with <user_data> from
<scene_viewer>.
==============================================================================*/


#endif /* __CMISS_SCENE_VIEWER_H__ */
