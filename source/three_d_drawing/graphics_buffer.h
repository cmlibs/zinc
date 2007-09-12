/*******************************************************************************
FILE : graphics_buffer.h

LAST MODIFIED : 4 June 2004

DESCRIPTION :
This provides a Cmgui interface to the OpenGL contexts of many types.
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
#if !defined (GRAPHICS_BUFFER_H)
#define GRAPHICS_BUFFER_H

#include "general/callback.h"
#include "general/object.h"
#include "user_interface/user_interface.h"

#if defined (GTK_USER_INTERFACE)
/* #define GTK_USE_GTKGLAREA */
#endif /* defined (GTK_USER_INTERFACE) */

#if defined (OPENGL_API)
#  if defined (MOTIF) || defined (GTK_USER_INTERFACE)
#     define GRAPHICS_BUFFER_USE_BUFFERS
#     define GRAPHICS_BUFFER_USE_OFFSCREEN_BUFFERS
#  endif /* defined (MOTIF) || defined (GTK_USER_INTERFACE) */
#endif /* defined (OPENGL_API) */

/*
Global types
------------
*/

enum Graphics_buffer_type
{
	GRAPHICS_BUFFER_INVALID_TYPE,
	GRAPHICS_BUFFER_GLX_X3D_TYPE,
	GRAPHICS_BUFFER_GLX_DM_PBUFFER_TYPE, /* Special type available only on O2's */
	GRAPHICS_BUFFER_GLX_PBUFFER_TYPE, /* Accelerated offscreen rendering */
	GRAPHICS_BUFFER_GLX_PIXMAP_TYPE, /* Non shared offscreen, no good for our display lists but
											  can be used for find_xi_special buffer */
	GRAPHICS_BUFFER_GTKGLAREA_TYPE,
	GRAPHICS_BUFFER_GTKGLEXT_TYPE,
	GRAPHICS_BUFFER_WIN32_TYPE,
	GRAPHICS_BUFFER_WX_TYPE,
	GRAPHICS_BUFFER_CARBON_TYPE
};

enum Graphics_buffer_buffering_mode
{
	GRAPHICS_BUFFER_ANY_BUFFERING_MODE,
	GRAPHICS_BUFFER_SINGLE_BUFFERING,
	GRAPHICS_BUFFER_DOUBLE_BUFFERING
};

enum Graphics_buffer_stereo_mode
{
	GRAPHICS_BUFFER_ANY_STEREO_MODE,
	GRAPHICS_BUFFER_MONO,
	GRAPHICS_BUFFER_STEREO
};

enum Graphics_buffer_input_modifier
{
	GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT = 1,
	GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL = 2,
	GRAPHICS_BUFFER_INPUT_MODIFIER_ALT = 4,
	GRAPHICS_BUFFER_INPUT_MODIFIER_BUTTON1 = 8
};

enum Graphics_buffer_input_event_type
{
	GRAPHICS_BUFFER_INVALID_INPUT,
	GRAPHICS_BUFFER_MOTION_NOTIFY,
	GRAPHICS_BUFFER_BUTTON_PRESS,
	GRAPHICS_BUFFER_BUTTON_RELEASE,
	GRAPHICS_BUFFER_KEY_PRESS,
	GRAPHICS_BUFFER_KEY_RELEASE
};

struct Graphics_buffer_input
{
	enum Graphics_buffer_input_event_type type;
	int button_number;
	int key_code;
	int position_x;
	int position_y;
	/* flags indicating the state of the shift, control and alt keys - use
		 logical OR with GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT etc. */
	enum Graphics_buffer_input_modifier input_modifier;
};

struct Graphics_buffer_package;

struct Graphics_buffer;

DECLARE_CMISS_CALLBACK_TYPES(Graphics_buffer_callback, \
	struct Graphics_buffer *, void *, void);

DECLARE_CMISS_CALLBACK_TYPES(Graphics_buffer_input_callback, \
	struct Graphics_buffer *, struct Graphics_buffer_input *, void);

/*
Global functions
----------------
*/


struct Graphics_buffer_package *CREATE(Graphics_buffer_package)(
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 6 May 2004

DESCRIPTION :
Creates a Graphics_buffer_package which enables Graphics_buffers created from
it to share graphics contexts.
==============================================================================*/

int DESTROY(Graphics_buffer_package)(struct Graphics_buffer_package **package_ptr);
/*******************************************************************************
LAST MODIFIED : 5 May 2004

DESCRIPTION :
Closes the Graphics buffer package
==============================================================================*/

int Graphics_buffer_package_set_override_visual_id(
	struct Graphics_buffer_package *graphics_buffer_package,
	int override_visual_id);
/*******************************************************************************
LAST MODIFIED : 21 May 2004

DESCRIPTION :
Sets a particular visual to be used by all graphics buffers.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Graphics_buffer);

#if defined (GTK_USER_INTERFACE)
struct Graphics_buffer *create_Graphics_buffer_gtkgl(
	struct Graphics_buffer_package *graphics_buffer_package,
	GtkContainer *parent,
	enum Graphics_buffer_buffering_mode buffering_mode,
	enum Graphics_buffer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth, 
	int minimum_accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 2 June 2004

DESCRIPTION :
==============================================================================*/
#endif /* defined (GTK_USER_INTERFACE) */

struct Graphics_buffer *create_Graphics_buffer_offscreen(
	struct Graphics_buffer_package *graphics_buffer_package,
	int width, int height,
	enum Graphics_buffer_buffering_mode buffering_mode,
	enum Graphics_buffer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 6 May 2004

DESCRIPTION :
==============================================================================*/

struct Graphics_buffer *create_Graphics_buffer_shared_offscreen(
	struct Graphics_buffer_package *graphics_buffer_package,
	int width, int height,
	enum Graphics_buffer_buffering_mode buffering_mode,
	enum Graphics_buffer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 6 May 2004

DESCRIPTION :
==============================================================================*/

struct Graphics_buffer *create_Graphics_buffer_offscreen_from_buffer(
	int width, int height, struct Graphics_buffer *buffer_to_match);
/*******************************************************************************
LAST MODIFIED : 6 May 2004

DESCRIPTION :
==============================================================================*/

#if defined (WIN32_USER_INTERFACE)
struct Graphics_buffer *create_Graphics_buffer_win32(
	struct Graphics_buffer_package *graphics_buffer_package,
	HWND hWnd, HDC hDC,
	enum Graphics_buffer_buffering_mode buffering_mode,
	enum Graphics_buffer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth, 
	int minimum_accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 1 June 2007

DESCRIPTION :
==============================================================================*/
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
int Graphics_buffer_handle_windows_event(struct Graphics_buffer *graphics_buffer,
	UINT event,WPARAM first_message,LPARAM second_message);
/*******************************************************************************
LAST MODIFIED : 31 May 2007

DESCRIPTION:
Passes the supplied windows event on to the graphics buffer.
==============================================================================*/
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (MOTIF)
struct Graphics_buffer *create_Graphics_buffer_X3d(
	struct Graphics_buffer_package *graphics_buffer_package,
	Widget parent, int width, int height,
	enum Graphics_buffer_buffering_mode buffering_mode,
	enum Graphics_buffer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 5 May 2004

DESCRIPTION :
==============================================================================*/
#endif /* defined (MOTIF) */

#if defined (MOTIF)
struct Graphics_buffer *create_Graphics_buffer_X3d_from_buffer(
	Widget parent, int width, int height, 
	struct Graphics_buffer *buffer_to_match);
/*******************************************************************************
LAST MODIFIED : 6 May 2004

DESCRIPTION :
==============================================================================*/
#endif /* defined (MOTIF) */

#if defined (CARBON_USER_INTERFACE)
struct Graphics_buffer *create_Graphics_buffer_Carbon(
	struct Graphics_buffer_package *graphics_buffer_package,
	CGrafPtr port,
	int    portx,
	int    porty,
	enum Graphics_buffer_buffering_mode buffering_mode,
	enum Graphics_buffer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth, 
	int minimum_accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 21 November 2006

DESCRIPTION :
==============================================================================*/
#endif /* defined (CARBON_USER_INTERFACE) */

#if defined (WX_USER_INTERFACE) && defined (__cplusplus)
class wxPanel;

struct Graphics_buffer *create_Graphics_buffer_wx(
	struct Graphics_buffer_package *graphics_buffer_package,
	wxPanel *parent,
	enum Graphics_buffer_buffering_mode buffering_mode,
	enum Graphics_buffer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth, 
	int minimum_accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 7 December 2006

DESCRIPTION :
==============================================================================*/
#endif /* defined (WX_USER_INTERFACE) && defined (__cplusplus) */

int Graphics_buffer_make_current(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 1 July 2002
DESCRIPTION :
==============================================================================*/

int Graphics_buffer_get_visual_id(struct Graphics_buffer *buffer, int *visual_id);
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the visual id used by the graphics buffer.
==============================================================================*/

int Graphics_buffer_get_buffering_mode(struct Graphics_buffer *buffer,
	enum Graphics_buffer_buffering_mode *buffering_mode);
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the buffering mode being used by the graphics buffer.
==============================================================================*/

int Graphics_buffer_get_stereo_mode(struct Graphics_buffer *buffer,
	enum Graphics_buffer_stereo_mode *stereo_mode);
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the stereo mode being used by the graphics buffer.
==============================================================================*/

int Graphics_buffer_get_colour_buffer_depth(struct Graphics_buffer *buffer,
	int *colour_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the depth of the colour buffer used by the graphics buffer.
==============================================================================*/

int Graphics_buffer_get_depth_buffer_depth(struct Graphics_buffer *buffer,
	int *depth_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the depth of the depth buffer used by the graphics buffer.
==============================================================================*/

int Graphics_buffer_get_accumulation_buffer_depth(struct Graphics_buffer *buffer,
	int *accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the depth of the accumulation buffer used by the graphics buffer.
==============================================================================*/

int Graphics_buffer_swap_buffers(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 2 July 2002

DESCRIPTION :
==============================================================================*/

int Graphics_buffer_make_read_current(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 1 July 2002
DESCRIPTION :
Sets this buffer to be the source and the current ThreeDWindow (the one last
made current) to be the destination.
==============================================================================*/

int Graphics_buffer_get_width(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Returns the width of buffer represented by <buffer>.
==============================================================================*/

int Graphics_buffer_set_width(struct Graphics_buffer *buffer, int width);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Sets the width of buffer represented by <buffer>.
==============================================================================*/

int Graphics_buffer_get_height(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Gets the height of buffer represented by <buffer>.
==============================================================================*/

int Graphics_buffer_set_height(struct Graphics_buffer *buffer, int height);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Sets the height of buffer represented by <buffer>.
==============================================================================*/

int Graphics_buffer_get_origin_x(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 16 February 2007

DESCRIPTION :
Returns the x origin of buffer represented by <buffer>.
==============================================================================*/

int Graphics_buffer_get_origin_y(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 16 February 2007

DESCRIPTION :
Gets the y origin of buffer represented by <buffer>.
==============================================================================*/

int Graphics_buffer_get_border_width(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 2 July 2002

DESCRIPTION :
Returns the border width of buffer represented by <buffer>.
==============================================================================*/

int Graphics_buffer_set_border_width(struct Graphics_buffer *buffer,
	int border_width);
/*******************************************************************************
LAST MODIFIED : 2 July 2002

DESCRIPTION :
Sets the border width of buffer represented by <buffer>.
==============================================================================*/

int Graphics_buffer_is_visible(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Returns 1 if the <buffer> is visible.  If the scene viewer gets zero from this
routine it will not bother rendering into it, allowing us to avoid rendering
into unmanaged or invisible widgets.
==============================================================================*/

int Graphics_buffer_awaken(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Activates the graphics <buffer>.
==============================================================================*/

enum Graphics_buffer_type Graphics_buffer_get_type(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 27 May 2004

DESCRIPTION :
Returns information about the type of buffer that was created.
==============================================================================*/

#if defined (MOTIF)
Display *Graphics_buffer_X11_get_display(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 27 May 2004

DESCRIPTION :
Returns information about the type of buffer that was created.
==============================================================================*/
#endif /* defined (MOTIF) */

int Graphics_buffer_add_initialise_callback(struct Graphics_buffer *buffer,
	CMISS_CALLBACK_FUNCTION(Graphics_buffer_callback) initialise_callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Adds an initialise callback to the graphics <buffer>.
==============================================================================*/

int Graphics_buffer_add_resize_callback(struct Graphics_buffer *buffer,
	CMISS_CALLBACK_FUNCTION(Graphics_buffer_callback) resize_callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Adds an resize callback to the graphics <buffer>.
==============================================================================*/

int Graphics_buffer_add_expose_callback(struct Graphics_buffer *buffer,
	CMISS_CALLBACK_FUNCTION(Graphics_buffer_callback) expose_callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Adds an expose callback to the graphics <buffer>.
==============================================================================*/

int Graphics_buffer_add_input_callback(struct Graphics_buffer *buffer,
	CMISS_CALLBACK_FUNCTION(Graphics_buffer_input_callback) input_callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Adds an input callback to the graphics <buffer>.
==============================================================================*/

int DESTROY(Graphics_buffer)(struct Graphics_buffer **buffer);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Closes a Graphics buffer instance
==============================================================================*/

#if defined (OPENGL_API) && defined (MOTIF)
int query_glx_extension(char *extName, Display *display, int screen);
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Search for extName in the GLX extensions string. Use of strstr() is not sufficient
because extension names can be prefixes of other extension names. Could use
strtok() but the constant string returned by glGetString might be in read-only
memory.
???SAB.  Taken directly from above
==============================================================================*/
#endif /* defined (OPENGL_API) && defined (MOTIF) */

#if defined (WIN32_USER_INTERFACE)
int Graphics_buffer_win32_use_font_bitmaps(struct Graphics_buffer *buffer,
	HFONT font, int first_bitmap, int number_of_bitmaps, int display_list_offset);
/*******************************************************************************
LAST MODIFIED : 17 November 2005

DESCRIPTION :
Function provided for compiling graphics_fonts as a graphics context is 
required in the win32 case.
==============================================================================*/
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (MOTIF)
Widget Graphics_buffer_X3d_get_widget(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 17 November 2005

DESCRIPTION :
Private routine to facilitate the compilation of Graphics fonts with only
a Graphics_buffer.
==============================================================================*/
#endif /* defined (MOTIF) */
#endif /* !defined (GRAPHICS_BUFFER_H) */

#if defined (CARBON_USER_INTERFACE)
int Graphics_buffer_carbon_set_window_size(struct Graphics_buffer *graphics_buffer,
	int width, int height, int portx, int porty, int clip_width, int clip_height);
/*******************************************************************************
LAST MODIFIED : 16 February 2007

DESCRIPTION :
Sets the coordinates within the graphics port which the graphics_buffer should
respect.
==============================================================================*/
#endif /* defined (CARBON_USER_INTERFACE) */
