/*******************************************************************************
FILE : cmiss_scene_viewer.h

LAST MODIFIED : 19 September 2002

DESCRIPTION :
The public interface to the Cmiss_scene_viewer object for rendering cmiss
scenes.
==============================================================================*/
#ifndef __CMISS_SCENE_VIEWER_H__
#define __CMISS_SCENE_VIEWER_H__

#if defined (GTK_USER_INTERFACE)
#include <gtk/gtk.h>
#endif /* defined (GTK_USER_INTERFACE) */
#include "general/object.h"

/*
Global types
------------
*/

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
	CMISS_SCENE_VIEWER_ANY_BUFFERING_MODE,
	CMISS_SCENE_VIEWER_SINGLE_BUFFERING,
	CMISS_SCENE_VIEWER_DOUBLE_BUFFERING
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
	CMISS_SCENE_VIEWER_ANY_STEREO_MODE,
	CMISS_SCENE_VIEWER_MONO,
	CMISS_SCENE_VIEWER_STEREO
};

enum Cmiss_scene_viewer_projection_mode
/*******************************************************************************
LAST MODIFIED : 16 September 2002

DESCRIPTION :
Specifies the sort of projection matrix used to render the 3D scene.
==============================================================================*/
{
	CMISS_SCENE_VIEWER_PARALLEL,
	CMISS_SCENE_VIEWER_PERSPECTIVE
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
	CMISS_SCENE_VIEWER_FAST_TRANSPARENCY,
	CMISS_SCENE_VIEWER_SLOW_TRANSPARENCY,
	CMISS_SCENE_VIEWER_LAYERED_TRANSPARENCY,
	CMISS_SCENE_VIEWER_ORDER_INDEPENDENT_TRANSPARENCY
};

typedef struct Cmiss_scene_viewer *Cmiss_scene_viewer_id;

/*
Global functions
----------------
*/

#if defined (GTK_USER_INTERFACE)
Cmiss_scene_viewer_id create_Cmiss_scene_viewer_gtk(
	GtkContainer *scene_viewer_widget,
	enum Cmiss_scene_viewer_buffering_mode buffer_mode,
	enum Cmiss_scene_viewer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth, int specified_visual_id);
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
If <specified_visual_id> is nonzero then this overrides all other visual
selection mechanisms and this visual will be used if possible or the create will
fail.
==============================================================================*/
#endif /* defined (GTK_USER_INTERFACE) */

int DESTROY(Cmiss_scene_viewer)(Cmiss_scene_viewer_id *scene_viewer_id_address);
/*******************************************************************************
LAST MODIFIED : 10 September 2002

DESCRIPTION :
Closes the scene_viewer.
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
	double *near, double *far);
/*******************************************************************************
LAST MODIFIED : 13 September 2002

DESCRIPTION :
Gets the distance from the eye_point to the <near> clip plane and to the <far>
clip plane in the <scene_viewer>.
==============================================================================*/

int Cmiss_scene_viewer_set_near_and_far_plane(Cmiss_scene_viewer_id scene_viewer,
	double near, double far);
/*******************************************************************************
LAST MODIFIED : 13 September 2002

DESCRIPTION :
Sets the distance from the eye_point to the <near> clip plane and to the <far>
clip plane in the <scene_viewer>.
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

int Cmiss_scene_viewer_get_transparency_layers(Cmiss_scene_viewer_id scene_viewer,
	int *transparency_layers);
/*******************************************************************************
LAST MODIFIED : 17 September 2002

DESCRIPTION :
Gets the number of layers used in the CMISS_SCENE_VIEWER_LAYERED_TRANSPARENCY
transparency_mode.  See the definition of the
Cmiss_scene_viewer_transparency_mode enumerator.
==============================================================================*/

int Cmiss_scene_viewer_set_transparency_layers(Cmiss_scene_viewer_id scene_viewer,
	int layers);
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
	int *antialias);
/*******************************************************************************
LAST MODIFIED : 11 September 2002

DESCRIPTION :
Gets the number of jitter samples used to antialias the scene_viewer.
==============================================================================*/

int Cmiss_scene_viewer_set_antialias_mode(Cmiss_scene_viewer_id scene_viewer,
	int antialias_mode);
/*******************************************************************************
LAST MODIFIED : 11 September 2002

DESCRIPTION :
Sets the number of jitter samples used to antialias the scene_viewer.
Zero turns antialiasing off.
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

int Cmiss_scene_viewer_get_background_colour_rgb(
	Cmiss_scene_viewer_id scene_viewer, double *red, double *green, double *blue);
/*******************************************************************************
LAST MODIFIED : 11 September 2002

DESCRIPTION :
Returns the background_colour of the scene_viewer.
==============================================================================*/

int Cmiss_scene_viewer_set_background_colour_rgb(
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
	Cmiss_scene_viewer_id scene_viewer, char *tool_name);
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
	char *scene_name);
/*******************************************************************************
LAST MODIFIED : 11 September 2002

DESCRIPTION :
Sets the currently scene rendered in the scene_viewer if one that
matches the <scene_name> exists.
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

int Cmiss_scene_viewer_write_image_to_file(Cmiss_scene_viewer_id scene_viewer,
	char *file_name, int force_onscreen, int preferred_width,
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
#endif /* __CMISS_SCENE_VIEWER_H__ */
