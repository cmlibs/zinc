/*******************************************************************************
FILE : graphics_buffer.cpp

LAST MODIFIED : 19 February 2007

DESCRIPTION :
This provides a Cmgui interface to the OpenGL contexts of many types.
******************************************************************************/
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

#include "api/cmiss_zinc_configure.h"

#if defined (WIN32)
//#	define WINDOWS_LEAN_AND_MEAN
#	define NOMINMAX
#	include <windows.h>
#endif
#if defined (USE_GLEW)
#	include <GL/glew.h>
#endif

#include "general/callback_private.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/object.h"
#include "general/message.h"
//#define GL_GLEXT_PROTOTYPES
//#define GRAPHICS_LIBRARY_C
//#include "graphics/graphics_library.h"
#include "graphics/scene_viewer.h"
#include "three_d_drawing/graphics_buffer.h"
#if defined (UNIX) && !defined (DARWIN)
//-- #include "user_interface/event_dispatcher.h"
#endif /* defined (UNIX) && !defined (DARWIN) */
//#include "three_d_drawing/window_system_extensions.h"

/* #define DEBUG_CODE */
#if defined DEBUG_CODE || defined (WIN32_USER_INTERFACE)
#  include <stdio.h>
#endif

#if !defined (AIX)
/* SAB 30 June 2004
	These calls should be available in every system with GLX 1.3 or greater
	but on the SGI the original code seems to work better with movies and
	with grabbing frames off the screen.  This is done by trying the SGI versions
	first on these machines.
	The SGI allows the creation of the correct Pbuffer using GLX commands
	and then generates a bad alloc error when I try to make it current.
	The code should still run on an older GLX even if it is compiled on a GLX 1.3 by
	falling through to section 4.
	AIX is still having problems with rendering triangles badly and the fbconfig code
	doesn't work well set displayed so it is back off again. */
/*???DB.  The old version of GLX (glx.h 1999/12/11), has GLX_VERSION_1_3
	defined, but doesn't define GLX_PBUFFER_WIDTH, GLX_PBUFFER_HEIGHT and
	GLX_RGBA_BIT */
#if defined (GLX_VERSION_1_3) && defined (GLX_PBUFFER_WIDTH) && \
	defined (GLX_PBUFFER_HEIGHT) && defined (GLX_RGBA_BIT)
#define USE_GLX_PBUFFER 1
#define USE_GLX_FBCONFIG 1
#endif /* defined (GLX_VERSION_1_3) */
#endif /* !defined (AIX) */
/* Despite being in the Mesa headers we do not want to use the SGI FB extensions at all,
   the GLX versions are preferable. */
#undef GLX_SGIX_pbuffer
#undef GLX_SGIX_dmbuffer

/*
Module types
------------
*/

/*
Module functions
----------------
*/
struct Graphics_buffer *CREATE(Graphics_buffer)(
		struct Graphics_buffer_package *package,
		enum Graphics_buffer_type type,
		enum Graphics_buffer_buffering_mode buffering_mode,
		enum Graphics_buffer_stereo_mode stereo_mode)
{
	struct Graphics_buffer *buffer = 0;
	if (ALLOCATE(buffer, struct Graphics_buffer, 1))
	{
		buffer->access_count = 1;
		buffer->attrib_list = 0;
		buffer->height = 0;
		buffer->width = 0;
#if defined (OPENGL_API)
		buffer->fbo = 0;
		buffer->depthbuffer = 0;
		buffer->img = 0;
#if defined (USE_MSAA)
		buffer->msbuffer = 0;
		buffer->multi_fbo = 0;
		buffer->multi_depthbuffer = 0;
#endif
#endif
		buffer->origin_x = 0;
		buffer->origin_y = 0;
		buffer->package = package;
		buffer->type = type;
		buffer->buffering_mode = buffering_mode;
		buffer->stereo_mode = stereo_mode;
	}

	return buffer;
}

/*
Global functions
----------------
*/

struct Graphics_buffer_package *CREATE(Graphics_buffer_package)()
/*******************************************************************************
LAST MODIFIED : 6 May 2004

DESCRIPTION :
Creates a Graphics_buffer_package which enables Graphics_buffers created from
it to share graphics contexts.
==============================================================================*/
{
	struct Graphics_buffer_package *package = 0;

	if (ALLOCATE(package, struct Graphics_buffer_package, 1))
	{
		package->override_visual_id = 0;
		//-- package->wxSharedContext = (wxGLContext*)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Graphics_buffer_package). "
			"Unable to allocate package structure");
		package = (struct Graphics_buffer_package *)NULL;
	}

	return (package);
} /* CREATE(Graphics_buffer_package) */

int DESTROY(Graphics_buffer_package)(struct Graphics_buffer_package **package_ptr)
/*******************************************************************************
LAST MODIFIED : 6 May 2004

DESCRIPTION :
Closes the Graphics buffer package
==============================================================================*/
{
	int return_code;
	struct Graphics_buffer_package *package;

	ENTER(DESTROY(Graphics_buffer_package));
	if (package_ptr && (package = *package_ptr))
	{
		return_code=1;
		DEALLOCATE(*package_ptr);
		*package_ptr = (struct Graphics_buffer_package *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Graphics_buffer_package).  Missing package");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Graphics_buffer_package) */

int Graphics_buffer_get_colour_buffer_depth(struct Graphics_buffer *buffer,
	int *colour_buffer_depth)
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the depth of the colour buffer used by the graphics buffer.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_get_colour_buffer_depth);
	if (buffer)
	{
		return_code = 1;
		GLint colour_buffer_bits;
		glGetIntegerv(GL_RED_BITS, &colour_buffer_bits);
		*colour_buffer_depth = colour_buffer_bits;
		glGetIntegerv(GL_BLUE_BITS, &colour_buffer_bits);
		*colour_buffer_depth += colour_buffer_bits;
		glGetIntegerv(GL_GREEN_BITS, &colour_buffer_bits);
		*colour_buffer_depth += colour_buffer_bits;
		glGetIntegerv(GL_ALPHA_BITS, &colour_buffer_bits);
		*colour_buffer_depth += colour_buffer_bits;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_buffer_get_colour_buffer_depth.  "
			"Graphics_bufffer missing.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_get_colour_buffer_depth */

int Graphics_buffer_get_depth_buffer_depth(Graphics_buffer *buffer,
	int *depth_buffer_depth)
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the depth of the depth buffer used by the graphics buffer.
==============================================================================*/
{
	int return_code;

	if (buffer)
	{
		GLint depth_bits;
		glGetIntegerv(GL_DEPTH_BITS, &depth_bits);
		*depth_buffer_depth = depth_bits;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_buffer_get_depth_buffer_depth.  "
			"Graphics_bufffer missing.");
		return_code = 0;
	}

	return (return_code);
} /* Graphics_buffer_get_depth_buffer_depth */

int Graphics_buffer_get_accumulation_buffer_depth(Graphics_buffer *buffer,
	int *accumulation_buffer_depth)
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the depth of the accumulation buffer used by the graphics buffer.
==============================================================================*/
{
	int return_code;

	if (buffer)
	{
		GLint accumulation_buffer_bits;
		glGetIntegerv(GL_ACCUM_RED_BITS, &accumulation_buffer_bits);
		*accumulation_buffer_depth = accumulation_buffer_bits;
		glGetIntegerv(GL_ACCUM_BLUE_BITS, &accumulation_buffer_bits);
		*accumulation_buffer_depth += accumulation_buffer_bits;
		glGetIntegerv(GL_ACCUM_GREEN_BITS, &accumulation_buffer_bits);
		*accumulation_buffer_depth += accumulation_buffer_bits;
		glGetIntegerv(GL_ACCUM_ALPHA_BITS, &accumulation_buffer_bits);
		*accumulation_buffer_depth += accumulation_buffer_bits;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_buffer_get_accumulation_buffer_depth.  "
			"Graphics_bufffer missing.");
		return_code = 0;
	}

	return (return_code);
} /* Graphics_buffer_get_accumulation_buffer_depth */

int Graphics_buffer_get_buffering_mode(struct Graphics_buffer *buffer,
	enum Graphics_buffer_buffering_mode *buffering_mode)
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the buffering mode used by the graphics buffer.
==============================================================================*/
{
	int return_code = 0;

	if (buffer)
	{
		*buffering_mode = buffer->buffering_mode;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_buffer_get_buffering_mode.  "
			"Graphics_bufffer missing.");
	}

	return (return_code);
} /* Graphics_buffer_get_buffering_mode */

int Graphics_buffer_get_stereo_mode(struct Graphics_buffer *buffer,
	enum Graphics_buffer_stereo_mode *stereo_mode)
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the stereo mode used by the graphics buffer.
==============================================================================*/
{
	int return_code = 0;

	if (buffer)
	{
		*stereo_mode = buffer->stereo_mode;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_buffer_get_stereo_mode.  "
			"Graphics_bufffer missing.");
	}

	return (return_code);
} /* Graphics_buffer_get_stereo_mode */

unsigned int Graphics_buffer_get_width(struct Graphics_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Returns the width of buffer represented by <buffer>.
==============================================================================*/
{
	unsigned int width = 0;

	if (buffer)
	{
		width = buffer->width;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_get_width.  Invalid buffer");
	}

	return (width);
} /* Graphics_buffer_get_width */

int Graphics_buffer_set_width(struct Graphics_buffer *buffer, unsigned int width)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Sets the width of buffer represented by <buffer>.
==============================================================================*/
{
	int return_code = 0;

	if (buffer)
	{
		buffer->width = width;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_set_width.  Invalid buffer");
	}

	return (return_code);
} /* Graphics_buffer_set_width */

unsigned int Graphics_buffer_get_height(struct Graphics_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Returns the height of buffer represented by <buffer>.
==============================================================================*/
{
	unsigned int height = 0;

	if (buffer)
	{
		height = buffer->height;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_get_height.  Invalid buffer");
	}

	return (height);
} /* Graphics_buffer_get_height */

int Graphics_buffer_set_height(struct Graphics_buffer *buffer, unsigned int height)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Sets the height of buffer represented by <buffer>.
==============================================================================*/
{
	int return_code = 0;

	if (buffer)
	{
		return_code = 1;
		buffer->height = height;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_set_height.  Invalid buffer");
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_set_height */

unsigned int Graphics_buffer_get_origin_x(struct Graphics_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Returns the height of buffer represented by <buffer>.
==============================================================================*/
{
	unsigned int origin_x = 0;

	if (buffer)
	{
		origin_x = buffer->origin_x;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_get_origin_x.  Invalid buffer");
	}

	return (origin_x);
} /* Graphics_buffer_get_origin_x */

unsigned int Graphics_buffer_get_origin_y(struct Graphics_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Returns the height of buffer represented by <buffer>.
==============================================================================*/
{
	unsigned int origin_y = 0;

	if (buffer)
	{
		origin_y = buffer->origin_y;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_get_origin_y.  Invalid buffer");
	}

	return (origin_y);
} /* Graphics_buffer_get_origin_y */

int Graphics_buffer_is_visible(struct Graphics_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Returns 1 if the <buffer> is visible.  If the scene viewer gets zero from this
routine it will not bother rendering into it, allowing us to avoid rendering
into unmanaged or invisible widgets.
==============================================================================*/
{
	int return_code = 0;

	if (buffer)
	{
		switch (buffer->type)
		{
			case GRAPHICS_BUFFER_WX_TYPE:
			case GRAPHICS_BUFFER_ONSCREEN_TYPE:
			{
				return_code = 1;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_is_visible.  "
					"Graphics_bufffer type unknown or not supported.");
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_is_visible.  Invalid buffer");
	}

	return (return_code);
} /* Graphics_buffer_is_visible */

enum Graphics_buffer_type Graphics_buffer_get_type(struct Graphics_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 27 May 2004

DESCRIPTION :
Returns information about the type of buffer that was created.
==============================================================================*/
{
	enum Graphics_buffer_type buffer_type = GRAPHICS_BUFFER_INVALID_TYPE;

	if (buffer)
	{
		buffer_type = buffer->type;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_get_display.  Invalid buffer");
	}

	return (buffer_type);
} /* Graphics_buffer_get_type */

int DESTROY(Graphics_buffer)(struct Graphics_buffer **buffer_ptr)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Closes a Graphics buffer instance
===============================================================================*/
{
	int return_code;
	struct Graphics_buffer *buffer;

	if (buffer_ptr && (buffer = *buffer_ptr))
	{
		return_code=1;
		if (buffer->attrib_list != NULL)
		{
				DEALLOCATE(buffer->attrib_list);
		}
		if (buffer->type == GRAPHICS_BUFFER_GL_EXT_FRAMEBUFFER_TYPE)
		 {
#if defined (OPENGL_API) && defined (GL_EXT_framebuffer_object)
			 GLint framebuffer_flag;
			 if (Graphics_library_check_extension(GL_EXT_framebuffer_object))
			 {
				 glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &framebuffer_flag);
				 if (framebuffer_flag != 0)
				 {
					 if (buffer->fbo != 0)
					 {
						 if (framebuffer_flag == (GLint)buffer->fbo)
						 {
							 glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
						 }
						 glDeleteFramebuffersEXT(1, &buffer->fbo);
					 }
					 if (buffer->depthbuffer != 0)
					 {
						 glDeleteRenderbuffersEXT(1, &buffer->depthbuffer);
					 }
#if defined (USE_MSAA)
					 if (buffer->multi_fbo != 0)
					 {
						 glDeleteFramebuffersEXT(1, &buffer->multi_fbo);
					 }
					 if (buffer->multi_depthbuffer != 0)
					 {
						 glDeleteFramebuffersEXT(1, &buffer->multi_depthbuffer);
					 }
					 if (buffer->msbuffer != 0)
					 {
						 glDeleteRenderbuffersEXT(1, &buffer->msbuffer);
					 }
#endif
					 if (buffer->img != 0)
					 {
						 glDeleteTextures(1, &buffer->img);
					 }

				 }
			 }
		 }
#endif /* defined (OPENGL_API) && defined (GL_EXT_framebuffer_object) */
		DEALLOCATE(*buffer_ptr);
		*buffer_ptr = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Graphics_buffer).  Missing buffer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Graphics_buffer) */

DECLARE_OBJECT_FUNCTIONS(Graphics_buffer)

