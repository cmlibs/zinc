/*******************************************************************************
FILE : graphics_window.h

LAST MODIFIED : 5 March 2002

DESCRIPTION :
Interface file for opening and closing and working a CMISS 3D display window.

Have get/set routines for parameters specific to window and/or which have
widgets that are automatically updated if you set them. Use these functions
if supplied, otherwise use Graphics_window_get_Scene_viewer() for the pane_no of
interest and set scene_viewer values directly.
==============================================================================*/
#if !defined (GRAPHICS_WINDOW_H)
#define GRAPHICS_WINDOW_H

#include "general/image_utilities.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "graphics/scene.h"
#include "graphics/scene_viewer.h"
#include "interaction/interactive_tool.h"
#include "user_interface/user_interface.h"

/*
Global/Public types
-------------------
*/

enum Graphics_window_layout_mode
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Must ensure all the types defined here are handled by functions referring to
the layout - these generally contain the word 'layout' in their names.
Have members BEFORE_FIRST and AFTER_LAST to enable iterating through the list
without knowing which order the types are in.
==============================================================================*/
{
	GRAPHICS_WINDOW_LAYOUT_MODE_INVALID,
	GRAPHICS_WINDOW_LAYOUT_MODE_BEFORE_FIRST,
	GRAPHICS_WINDOW_LAYOUT_2D,
	GRAPHICS_WINDOW_LAYOUT_FREE_ORTHO,
	GRAPHICS_WINDOW_LAYOUT_FRONT_BACK,
	GRAPHICS_WINDOW_LAYOUT_FRONT_SIDE,
	GRAPHICS_WINDOW_LAYOUT_ORTHOGRAPHIC,
	GRAPHICS_WINDOW_LAYOUT_PSEUDO_3D,
	GRAPHICS_WINDOW_LAYOUT_SIMPLE,
	GRAPHICS_WINDOW_LAYOUT_MODE_AFTER_LAST
};

struct Graphics_window;
/*******************************************************************************
LAST MODIFIED : 10 December 1997

DESCRIPTION :
The contents of this object are private.
==============================================================================*/

DECLARE_LIST_TYPES(Graphics_window);

DECLARE_MANAGER_TYPES(Graphics_window);

struct Modify_graphics_window_data
/*******************************************************************************
LAST MODIFIED : 13 June 2000

DESCRIPTION :
Structure to pass to modify_Graphics_window.
==============================================================================*/
{
	struct MANAGER(Graphics_window) *graphics_window_manager;
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	struct MANAGER(Light) *light_manager;
	struct MANAGER(Light_model) *light_model_manager;
	struct MANAGER(Scene) *scene_manager;
	struct MANAGER(Texture) *texture_manager;
}; /* struct Modify_graphics_window_data */

/*
Global/Public functions
-----------------------
*/
struct Graphics_window *CREATE(Graphics_window)(char *name,
	enum Scene_viewer_buffer_mode buffer_mode,
	struct Colour *background_colour,
	struct MANAGER(Light) *light_manager,
	struct Light *default_light,
	struct MANAGER(Light_model) *light_model_manager,
	struct Light_model *default_light_model,
	struct MANAGER(Scene) *scene_manager,struct Scene *scene,
	struct MANAGER(Texture) *texture_manager,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 5 October 1997

DESCRIPTION:
Creates a Graphics_window object, window shell and widgets. Returns a pointer
to the newly created object. The Graphics_window maintains a pointer to the
manager it is to live in, since users will want to close windows with the
window manager widgets.
Each window has a unique <name> that can be used to identify it, and which
will be printed on the windows title bar.
==============================================================================*/

int DESTROY(Graphics_window)(struct Graphics_window **graphics_window_address);
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION:
Frees the contents of the Graphics_window structure and then the object itself,
then closes down the window shell and widgets it uses. Note that responsibility
for removing the graphics_window from a global list of windows is left with the
calling routine. See also Graphics_window_close_CB and
Graphics_window_destroy_CB.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Graphics_window);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Graphics_window);

PROTOTYPE_LIST_FUNCTIONS(Graphics_window);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Graphics_window,name,char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(Graphics_window,name,char *);
PROTOTYPE_MANAGER_FUNCTIONS(Graphics_window);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Graphics_window,name,char *);

char *Graphics_window_manager_get_new_name(
	struct MANAGER(Graphics_window) *graphics_window_manager);
/*******************************************************************************
LAST MODIFIED : 13 October 1998

DESCRIPTION :
Makes up a default name string for a graphics window, based on numbers and
starting at "1". Up to the calling routine to deallocate the returned string.
==============================================================================*/

int Graphics_window_get_current_pane(struct Graphics_window *window);
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Returns the current_pane of the <window>, from 0 to number_of_panes-1.
==============================================================================*/

int Graphics_window_set_current_pane(struct Graphics_window *window,
	int pane_no);
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Sets the current_pane of the <window> to <pane_no>, from 0 to number_of_panes-1.
==============================================================================*/

double Graphics_window_get_eye_spacing(struct Graphics_window *graphics_window);
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Returns the eye_spacing from the <graphics_window> used for 3-D viewing.
==============================================================================*/

int Graphics_window_set_eye_spacing(struct Graphics_window *graphics_window,
	double eye_spacing);
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Sets the <eye_spacing> for the <graphics_window> used for 3-D viewing.
==============================================================================*/

enum Scene_viewer_input_mode Graphics_window_get_input_mode(
	struct Graphics_window *window);
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Returns the current input mode of the graphics window. Valid return values are
SCENE_VIEWER_NO_INPUT, SCENE_VIEWER_SELECT and SCENE_VIEWER_TRANSFORM.
==============================================================================*/

int Graphics_window_set_input_mode(struct Graphics_window *window,
	enum Scene_viewer_input_mode input_mode);
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Sets the current input mode of the <window> to <input_mode>. Valid input_modes
are SCENE_VIEWER_NO_INPUT, SCENE_VIEWER_SELECT and SCENE_VIEWER_TRANSFORM.
==============================================================================*/

enum Graphics_window_layout_mode Graphics_window_get_layout_mode(
	struct Graphics_window *window);
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Returns the layout mode in effect on the <window>.
==============================================================================*/

int Graphics_window_set_layout_mode(struct Graphics_window *window,
	enum Graphics_window_layout_mode layout_mode);
/*******************************************************************************
LAST MODIFIED : 9 October 1998

DESCRIPTION :
Returns the layout mode in effect on the <window>.
==============================================================================*/

int Graphics_window_get_orthographic_axes(struct Graphics_window *window,
	int *ortho_up_axis,int *ortho_front_axis);
/*******************************************************************************
LAST MODIFIED : 13 October 1998

DESCRIPTION :
Returns the "up" and "front" axes of the graphics window.
Axis numbers are from 1 to 6, where 1=x, 2=y, 3=z, 4=-x, 5=-y and 6=-z.
==============================================================================*/

int Graphics_window_set_orthographic_axes(struct Graphics_window *window,
	int ortho_up_axis,int ortho_front_axis);
/*******************************************************************************
LAST MODIFIED : 8 October 1998

DESCRIPTION :
Sets the "up" and "front" axes of the graphics window. Used for layout_modes
such as GRAPHICS_WINDOW_LAYOUT_ORTHOGRAPHIC.
Axis numbers are from 1 to 6, where 1=x, 2=y, 3=z, 4=-x, 5=-y and 6=-z.
==============================================================================*/

enum Scene_viewer_projection_mode Graphics_window_get_projection_mode(
	struct Graphics_window *window,int pane_no);
/*******************************************************************************
LAST MODIFIED : 5 October 1998

DESCRIPTION :
Returns the projection mode used by pane <pane_no> of <window>.
==============================================================================*/

int Graphics_window_set_projection_mode(struct Graphics_window *window,
	int pane_no,enum Scene_viewer_projection_mode projection_mode);
/*******************************************************************************
LAST MODIFIED : 5 October 1998

DESCRIPTION :
Sets the <projection_mode> used by pane <pane_no> of <window>. Allowable values
are SCENE_VIEWER_PARALLEL,	SCENE_VIEWER_PERSPECTIVE and SCENE_VIEWER_CUSTOM.
Whether you can set this for a pane depends on current layout_mode of window.
==============================================================================*/

struct Scene *Graphics_window_get_Scene(struct Graphics_window *window);
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Returns the Scene for the <graphics_window>.
==============================================================================*/

struct Scene_viewer *Graphics_window_get_Scene_viewer(
	struct Graphics_window *window,int pane_no);
/*******************************************************************************
LAST MODIFIED : 5 October 1998

DESCRIPTION :
Returns the Scene_viewer in pane <pane_no> of <window>. Calling function can
then set view and other parameters for the scene_viewer directly.
==============================================================================*/

double Graphics_window_get_std_view_angle(struct Graphics_window *window);
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Returns the std_view_angle from the <window>.
==============================================================================*/

int Graphics_window_set_std_view_angle(struct Graphics_window *graphics_window,
	double std_view_angle);
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Sets the <std_view_angle> for the <graphics_window>. The std_view_angle (in
degrees) is used by the Graphics_window_view_all function which positions the
viewer the correct distance away to see the currently visible scene at that
angle.
==============================================================================*/

int Graphics_window_get_viewing_area_size(struct Graphics_window *window,
	int *viewing_width,int *viewing_height);
/*******************************************************************************
LAST MODIFIED : 12 October 1998

DESCRIPTION :
The viewing_width and viewing_height are the size of the viewing area when the
graphics window has only one pane. When multiple panes are used, they are
separated by 2 pixel borders within the viewing area.
==============================================================================*/

int Graphics_window_set_viewing_area_size(struct Graphics_window *window,
	int viewing_width,int viewing_height);
/*******************************************************************************
LAST MODIFIED : 12 October 1998

DESCRIPTION :
The viewing_width and viewing_height are the size of the viewing area when the
graphics window has only one pane. When multiple panes are used, they are
separated by 2 pixel borders within the viewing area.
==============================================================================*/

int Graphics_window_update(struct Graphics_window *graphics_window);
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Forces a redraw on <graphics_window> at the next idle moment.
==============================================================================*/

int Graphics_window_update_now(struct Graphics_window *graphics_window);
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Forces a redraw on <graphics_window>.
==============================================================================*/

int Graphics_window_update_now_iterator(struct Graphics_window *graphics_window,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Iterator function for forcing a redraw on <graphics_window>.
In future want a flag in the graphics window which if on delays the redraw
until an explicit "gfx update" is entered.
==============================================================================*/

int Graphics_window_update_now_without_swapbuffers(
	struct Graphics_window *graphics_window);
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Forces a redraw on <graphics_window>.  Allows the window to be updated but kept
in the backbuffer so that utility functions such as the movie extensions can get
the pixels out of the backbuffer before the frames are swapped.
==============================================================================*/

int Graphics_window_get_frame_pixels(struct Graphics_window *window,
	enum Texture_storage_type storage, int *width, int *height,
	unsigned char **frame_data, int force_onscreen);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Returns the contents of the graphics window as pixels.  <width> and <height>
will be respected if the window is drawn offscreen and they are non zero,
otherwise they are set in accordance with current size of the graphics window.
If <force_onscreen> is non zero then the pixels will always be grabbed from the
graphics window on screen.
==============================================================================*/

struct Cmgui_image *Graphics_window_get_image(struct Graphics_window *window,
	int force_onscreen, int preferred_width, int preferred_height,
	enum Texture_storage_type storage);
/*******************************************************************************
LAST MODIFIED : 23 April 2002

DESCRIPTION :
Creates and returns a Cmgui_image from the image in <window>, usually for
writing. The image has a single depth plane and is in RGBA format.
Up to the calling function to DESTROY the returned Cmgui_image.
If <force_onscreen> is set then the pixels are grabbed directly from the window
display and the <preferred_width> and <preferred_height> are ignored.
Currently limited to 1 byte per component -- may want to improve for HPC.
==============================================================================*/

int Graphics_window_view_all(struct Graphics_window *window);
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Finds the x, y and z ranges from the scene and sets the view parameters so
that everything can be seen, and with window's std_view_angle. Also adjusts
near and far clipping planes; if specific values are required, should follow
with commands for setting these.
==============================================================================*/

int Graphics_window_view_changed(struct Graphics_window *window,
	int changed_pane);
/*******************************************************************************
LAST MODIFIED : 9 October 1998

DESCRIPTION :
Call this function whenever the view in a pane has changed. Depending on the
current layout_mode, the function adjusts the view in all the panes tied to
<changed_pane> to maintain the relationship expected for it.
==============================================================================*/

int list_Graphics_window(struct Graphics_window *window,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
Writes the properties of the <window> to the command window.
==============================================================================*/

int list_Graphics_window_commands(struct Graphics_window *window,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
Writes the commands for creating the <window> and establishing the views in it
to the command window.
==============================================================================*/

int modify_Graphics_window(struct Parse_state *state,void *window_void,
	void *modify_graphics_window_data_void);
/*******************************************************************************
LAST MODIFIED : 11 December 1997

DESCRIPTION :
Parser commands for modifying graphics windows - views, lighting, etc.
See comments with struct Modify_graphics_window_data;
==============================================================================*/

int Graphics_window_set_antialias_mode(struct Graphics_window *graphics_window,
	int antialias_mode);
/*******************************************************************************
LAST MODIFIED : 13 June 2000

DESCRIPTION :
Sets the number of times the images is oversampled to antialias the image. Only
certain values are supported 0/1 = off, 2, 4 & 8 are on.
==============================================================================*/

int Graphics_window_set_perturb_lines(struct Graphics_window *graphics_window,
	int perturb_lines);
/*******************************************************************************
LAST MODIFIED :13 June 2000

DESCRIPTION :
Sets if the <graphics_window> perturbs lines or not, using <perturb_lines>
(1==TRUE,0==FALSE)
==============================================================================*/

int set_Graphics_window(struct Parse_state *state,void *window_address_void,
	void *graphics_window_manager_void);
/*******************************************************************************
LAST MODIFIED : 19 June 1996

DESCRIPTION :
Modifier function to set the graphics window from a command.
NOTE: Calling function must remember to ACCESS any window passed to this
function, and DEACCESS any returned window.
==============================================================================*/

char *Graphics_window_layout_mode_string(
	enum Graphics_window_layout_mode layout_mode);
/*******************************************************************************
LAST MODIFIED : 5 October 1998

DESCRIPTION :
Returns a string label for the <layout_mode>, used in widgets and parsing.
NOTE: Calling function must not deallocate returned string.
==============================================================================*/

char **Graphics_window_layout_mode_get_valid_strings(
	int *number_of_valid_strings);
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Returns an allocated array of pointers to all static strings for valid
Graphics_window_layout_modes - obtained from function
Graphics_window_layout_mode_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/

enum Graphics_window_layout_mode Graphics_window_layout_mode_from_string(
	char *layout_mode_string);
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Returns the <Graphics_window_layout_mode> described by <layout_mode_string>,
or GRAPHICS_WINDOW_LAYOUT_MODE_INVALID if not recognized.
==============================================================================*/

int Graphics_window_layout_mode_get_number_of_panes(
	enum Graphics_window_layout_mode layout_mode);
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Returns the number of panes in a graphics window with the given <layout_mode>.
==============================================================================*/

int Graphics_window_layout_mode_is_projection_mode_valid_for_pane(
	enum Graphics_window_layout_mode layout_mode,int pane_no,
	enum Scene_viewer_projection_mode projection_mode);
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Returns true if the <projection_mode> can be used with pane <pane_no> of a
graphics window with the given <layout_mode>.
==============================================================================*/

#endif /* !defined (GRAPHICS_WINDOW_H) */
