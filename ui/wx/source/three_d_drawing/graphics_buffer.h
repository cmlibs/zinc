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

#include "api/cmiss_zinc_configure.h"
#include "api/cmiss_zinc_ui_configure.h"

#if defined (OPENGL_API)
#  if defined (GTK_USER_INTERFACE)
#     define GRAPHICS_BUFFER_USE_BUFFERS
#     define GRAPHICS_BUFFER_USE_OFFSCREEN_BUFFERS
#  endif /* defined (GTK_USER_INTERFACE) */
#endif /* defined (OPENGL_API) */

#include "general/callback.h"
#include "three_d_drawing/abstract_graphics_buffer.h"

/*
Global functions
----------------
*/


struct Graphics_buffer_package *CREATE(Graphics_buffer_package)();
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

//-- PROTOTYPE_OBJECT_FUNCTIONS(Graphics_buffer);

//struct Graphics_buffer_wx *create_Graphics_buffer_offscreen(
//	struct Graphics_buffer_package *graphics_buffer_package,
//	int width, int height,
//	enum Graphics_buffer_buffering_mode buffering_mode,
//	enum Graphics_buffer_stereo_mode stereo_mode,
//	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
//	int minimum_accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 6 May 2004

DESCRIPTION :
==============================================================================*/

//struct Graphics_buffer_wx *create_Graphics_buffer_shared_offscreen(
//	struct Graphics_buffer_package *graphics_buffer_package,
//	int width, int height,
//	enum Graphics_buffer_buffering_mode buffering_mode,
//	enum Graphics_buffer_stereo_mode stereo_mode,
//	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
//	int minimum_accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 6 May 2004

DESCRIPTION :
==============================================================================*/

struct Graphics_buffer_wx *create_Graphics_buffer_offscreen_from_buffer(
	int width, int height, struct Graphics_buffer_wx *buffer_to_match);
/*******************************************************************************
LAST MODIFIED : 6 May 2004

DESCRIPTION :
==============================================================================*/

class wxPanel;

//struct Graphics_buffer_wx *create_Graphics_buffer_wx(
//	struct Graphics_buffer_package *graphics_buffer_package,
//	wxGLCanvas *canvas,
//	enum Graphics_buffer_buffering_mode buffering_mode,
//	enum Graphics_buffer_stereo_mode stereo_mode,
//	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
//	int minimum_accumulation_buffer_depth,
//	struct Graphics_buffer_wx *buffer_to_match);
/*******************************************************************************
LAST MODIFIED : 7 December 2006

DESCRIPTION :
==============================================================================*/

//-- int Graphics_buffer_make_current(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 1 July 2002
DESCRIPTION :
==============================================================================*/

//-- int Graphics_buffer_get_visual_id(struct Graphics_buffer *buffer, int *visual_id);
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the visual id used by the graphics buffer.
==============================================================================*/

//-- int Graphics_buffer_get_buffering_mode(struct Graphics_buffer *buffer,
//-- 	enum Graphics_buffer_buffering_mode *buffering_mode);
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the buffering mode being used by the graphics buffer.
==============================================================================*/

//-- int Graphics_buffer_get_stereo_mode(struct Graphics_buffer *buffer,
//-- 	enum Graphics_buffer_stereo_mode *stereo_mode);
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the stereo mode being used by the graphics buffer.
==============================================================================*/

//-- int Graphics_buffer_get_colour_buffer_depth(struct Graphics_buffer *buffer,
//-- 	int *colour_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the depth of the colour buffer used by the graphics buffer.
==============================================================================*/

//-- int Graphics_buffer_get_depth_buffer_depth(struct Graphics_buffer *buffer,
//-- 	int *depth_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the depth of the depth buffer used by the graphics buffer.
==============================================================================*/

//-- int Graphics_buffer_get_accumulation_buffer_depth(struct Graphics_buffer *buffer,
//-- 	int *accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the depth of the accumulation buffer used by the graphics buffer.
==============================================================================*/

//-- int Graphics_buffer_swap_buffers(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 2 July 2002

DESCRIPTION :
==============================================================================*/

//-- int Graphics_buffer_make_read_current(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 1 July 2002
DESCRIPTION :
Sets this buffer to be the source and the current ThreeDWindow (the one last
made current) to be the destination.
==============================================================================*/

//-- int Graphics_buffer_get_width(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Returns the width of buffer represented by <buffer>.
==============================================================================*/

//-- int Graphics_buffer_set_width(struct Graphics_buffer *buffer, int width);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Sets the width of buffer represented by <buffer>.
==============================================================================*/

//-- int Graphics_buffer_get_height(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Gets the height of buffer represented by <buffer>.
==============================================================================*/

//-- int Graphics_buffer_set_height(struct Graphics_buffer *buffer, int height);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Sets the height of buffer represented by <buffer>.
==============================================================================*/

//-- int Graphics_buffer_get_origin_x(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 16 February 2007

DESCRIPTION :
Returns the x origin of buffer represented by <buffer>.
==============================================================================*/

//-- int Graphics_buffer_get_origin_y(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 16 February 2007

DESCRIPTION :
Gets the y origin of buffer represented by <buffer>.
==============================================================================*/

//-- int Graphics_buffer_is_visible(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Returns 1 if the <buffer> is visible.  If the scene viewer gets zero from this
routine it will not bother rendering into it, allowing us to avoid rendering
into unmanaged or invisible widgets.
==============================================================================*/

//-- int Graphics_buffer_awaken(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Activates the graphics <buffer>.
==============================================================================*/

//-- enum Graphics_buffer_type Graphics_buffer_get_type(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 27 May 2004

DESCRIPTION :
Returns information about the type of buffer that was created.
==============================================================================*/

//-- int Graphics_buffer_add_initialise_callback(struct Graphics_buffer *buffer,
//-- 	CMISS_CALLBACK_FUNCTION(Graphics_buffer_callback) initialise_callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Adds an initialise callback to the graphics <buffer>.
==============================================================================*/

//-- int Graphics_buffer_add_resize_callback(struct Graphics_buffer *buffer,
//--	CMISS_CALLBACK_FUNCTION(Graphics_buffer_callback) resize_callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Adds an resize callback to the graphics <buffer>.
==============================================================================*/

//-- int Graphics_buffer_add_expose_callback(struct Graphics_buffer *buffer,
//-- 	CMISS_CALLBACK_FUNCTION(Graphics_buffer_callback) expose_callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Adds an expose callback to the graphics <buffer>.
==============================================================================*/

//-- int Graphics_buffer_add_input_callback(struct Graphics_buffer *buffer,
//-- 	CMISS_CALLBACK_FUNCTION(Graphics_buffer_input_callback) input_callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Adds an input callback to the graphics <buffer>.
==============================================================================*/

//-- #if defined (OPENGL_API) && defined (USE_MSAA)
//-- void Graphics_buffer_reset_multisample_framebuffer(struct Graphics_buffer *buffer);

//-- void Graphics_buffer_blit_framebuffer(struct Graphics_buffer *buffer);

//-- int Graphics_buffer_set_multisample_framebuffer(struct Graphics_buffer *buffer, int preferred_antialias);
//-- #endif /* defined (USE_MSAA) */

#endif /* !defined (GRAPHICS_BUFFER_H) */
