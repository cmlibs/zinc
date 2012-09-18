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

#define WX_USER_INTERFACE

#if defined (WIN32)
//#	define WINDOWS_LEAN_AND_MEAN
#	define NOMINMAX
#	include <windows.h>
#endif
#if defined (USE_GLEW)
#	include <GL/glew.h>
#endif
#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/debug.h>

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
#include "user_interface/event_dispatcher.h"
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

int DESTROY(Graphics_buffer_wx)(struct Graphics_buffer_wx **buffer);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Closes a Graphics buffer instance
==============================================================================*/

struct Graphics_buffer_package
/*******************************************************************************
LAST MODIFIED : 5 May 2004

DESCRIPTION :
==============================================================================*/
{
	int override_visual_id;
	wxGLContext* wxSharedContext;
};

//-- FULL_DECLARE_CMISS_CALLBACK_TYPES(Graphics_buffer_callback, struct Graphics_buffer *, void *);

//-- FULL_DECLARE_CMISS_CALLBACK_TYPES(Graphics_buffer_input_callback, struct Graphics_buffer *, struct Graphics_buffer_input *);

//struct Graphics_buffer_wx : public Graphics_buffer
///*******************************************************************************
//LAST MODIFIED : 5 May 2004

//DESCRIPTION :
//==============================================================================*/
//{
//	wxPanel *parent;
//	wxGLCanvas *canvas;
//	int *attrib_list, framebuffer_width, framebuffer_height;
//#if defined (OPENGL_API)
//	GLuint fbo, depthbuffer, img;
//#if defined (USE_MSAA)
//	GLuint msbuffer, multi_depthbuffer, multi_fbo;
//#endif
//#endif
//	int get_buffering_mode(enum Graphics_buffer_buffering_mode *graphics_buffer_buffering_mode) const;
//	int get_stereo_mode(enum Graphics_buffer_stereo_mode *graphics_buffer_stereo_mode) const;
//	int make_current();
//};

//int Graphics_buffer_wx::get_buffering_mode(enum Graphics_buffer_buffering_mode *graphics_buffer_buffering_mode) const
//{
//	int return_code = 0;
//	if (type == GRAPHICS_BUFFER_WX_TYPE)
//	{
//		*graphics_buffer_buffering_mode = GRAPHICS_BUFFER_DOUBLE_BUFFERING;
//		return_code = 1;
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,"Graphics_buffer_wx::get_buffering_mode.  "
//			"Graphics_bufffer type unknown or not supported.");
//	}

//	return return_code;
//}

//int Graphics_buffer_wx::get_stereo_mode(enum Graphics_buffer_stereo_mode *graphics_buffer_stereo_mode) const
//{
//	int return_code = 0;
//	if (type == GRAPHICS_BUFFER_WX_TYPE)
//	{
//		*graphics_buffer_stereo_mode = GRAPHICS_BUFFER_MONO;
//		return_code = 1;
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,"Graphics_buffer_wx::get_stereo_mode.  "
//			"Graphics_bufffer type unknown or not supported.");
//	}

//	return return_code;
//}

///*
//Module functions
//----------------
//*/

////-- DEFINE_CMISS_CALLBACK_MODULE_FUNCTIONS(Graphics_buffer_callback, void);

////-- DEFINE_CMISS_CALLBACK_FUNCTIONS(Graphics_buffer_callback, \
////-- 	struct Graphics_buffer_wx *,void *)

////-- DEFINE_CMISS_CALLBACK_MODULE_FUNCTIONS(Graphics_buffer_input_callback, void);

////-- DEFINE_CMISS_CALLBACK_FUNCTIONS(Graphics_buffer_input_callback, \
////-- 	struct Graphics_buffer_wx *, struct Graphics_buffer_input *)

////-- DECLARE_OBJECT_FUNCTIONS(Graphics_buffer)

//struct Graphics_buffer *CREATE(Graphics_buffer)(
//		struct Graphics_buffer_package *package, Graphics_buffer_type type)
//{
//	struct Graphics_buffer *buffer = new Graphics_buffer;
//	buffer->height = 0;
//	buffer->width = 0;
//	buffer->origin_x = 0;
//	buffer->origin_y = 0;
//	buffer->type = type;
//}

//struct Graphics_buffer_wx *CREATE(Graphics_buffer_wx)(
//	struct Graphics_buffer_package *package, Graphics_buffer_type type)
///*******************************************************************************
//LAST MODIFIED : 4 May 2004

//DESCRIPTION :
//This is static as it is designed to be called by the different constructors
//contained in the this module only.
//==============================================================================*/
//{
//	struct Graphics_buffer_wx *buffer;

//	ENTER(CREATE(Graphics_buffer));
//	buffer = new Graphics_buffer_wx();
//	if (buffer)
//	{
//		buffer->type = type;
//		buffer->package = package;
//		buffer->access_count = 0;

//		//-- buffer->initialise_callback_list=
//		//-- 	CREATE(LIST(CMISS_CALLBACK_ITEM(Graphics_buffer_callback)))();
//		//-- buffer->resize_callback_list=
//		//-- 	CREATE(LIST(CMISS_CALLBACK_ITEM(Graphics_buffer_callback)))();
//		//-- buffer->expose_callback_list=
//		//-- 	CREATE(LIST(CMISS_CALLBACK_ITEM(Graphics_buffer_callback)))();
//		//-- buffer->input_callback_list=
//		//-- 	CREATE(LIST(CMISS_CALLBACK_ITEM(Graphics_buffer_input_callback)))();

//		buffer->parent = 0;
//		buffer->canvas = 0;
//		buffer->attrib_list = NULL;
//#if defined (OPENGL_API)
//		buffer->fbo = 0;
//		buffer->depthbuffer = 0;
//		buffer->img = 0;
//#if defined (USE_MSAA)
//		buffer->msbuffer = 0;
//		buffer->multi_fbo = 0;
//		buffer->multi_depthbuffer = 0;
//#endif
//#endif
//		buffer->framebuffer_height = 0;
//		buffer->framebuffer_width = 0;
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,"CREATE(Graphics_buffer). Unable to allocate buffer structure");
//		buffer = 0;
//	}

//	LEAVE;
//	return (buffer);
//} /* CREATE(Graphics_buffer) */

//static const wxString canvas_name;

//class wxGraphicsBuffer : public wxGLCanvas
//{
//	Graphics_buffer_wx *graphics_buffer;
//	wxPanel *parent;
//	int key_code, cursor_x, cursor_y;
//public:

//	wxGraphicsBuffer(wxPanel *parent, wxGLContext* sharedContext,
//		Graphics_buffer_wx *graphics_buffer
//		, int *attrib_list)
//		: wxGLCanvas(parent, sharedContext, wxID_ANY, wxDefaultPosition, parent->GetSize(),
//			wxFULL_REPAINT_ON_RESIZE, canvas_name
//			, attrib_list)
//		, graphics_buffer(graphics_buffer)
//		, parent(parent)
//		, key_code(0)
//		, cursor_x(-1)
//		, cursor_y(-1)
//	{
//	};

//	~wxGraphicsBuffer()
//	{
//		if (graphics_buffer)
//		{
//			graphics_buffer->canvas = (wxGraphicsBuffer *)NULL;
//			if ((GetContext() == graphics_buffer->package->wxSharedContext))
//			{
//				graphics_buffer->package->wxSharedContext = (wxGLContext *)NULL;
//			}
//		}
//	};

//	void ClearGraphicsBufferReference()
//	{
//		graphics_buffer = 0;
//	}

//	void OnPaint( wxPaintEvent& WXUNUSED(event) )
//	{
//		/* Unfortunately can't find a better place to copy the shareable context */
//		if (!graphics_buffer->package->wxSharedContext)
//		{
//			graphics_buffer->package->wxSharedContext = GetContext();
//		}

//		/* must always be here */
//		wxPaintDC dc(this);
//		SetCurrent();

//			glMatrixMode(GL_PROJECTION);
//			glLoadIdentity();
//			glFrustum(-0.5f, 0.5f, -0.5f, 0.5f, 1.0f, 3.0f);
//			glMatrixMode(GL_MODELVIEW);

//			/* clear color and depth buffers */
//			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//			/* draw six faces of a cube */
//			glBegin(GL_QUADS);
//			glNormal3f( 0.0f, 0.0f, 1.0f);
//			glVertex3f( 0.5f, 0.5f, 0.5f); glVertex3f(-0.5f, 0.5f, 0.5f);
//			glVertex3f(-0.5f,-0.5f, 0.5f); glVertex3f( 0.5f,-0.5f, 0.5f);

//			glNormal3f( 0.0f, 0.0f,-1.0f);
//			glVertex3f(-0.5f,-0.5f,-0.5f); glVertex3f(-0.5f, 0.5f,-0.5f);
//			glVertex3f( 0.5f, 0.5f,-0.5f); glVertex3f( 0.5f,-0.5f,-0.5f);

//			glNormal3f( 0.0f, 1.0f, 0.0f);
//			glVertex3f( 0.5f, 0.5f, 0.5f); glVertex3f( 0.5f, 0.5f,-0.5f);
//			glVertex3f(-0.5f, 0.5f,-0.5f); glVertex3f(-0.5f, 0.5f, 0.5f);

//			glNormal3f( 0.0f,-1.0f, 0.0f);
//			glVertex3f(-0.5f,-0.5f,-0.5f); glVertex3f( 0.5f,-0.5f,-0.5f);
//			glVertex3f( 0.5f,-0.5f, 0.5f); glVertex3f(-0.5f,-0.5f, 0.5f);

//			glNormal3f( 1.0f, 0.0f, 0.0f);
//			glVertex3f( 0.5f, 0.5f, 0.5f); glVertex3f( 0.5f,-0.5f, 0.5f);
//			glVertex3f( 0.5f,-0.5f,-0.5f); glVertex3f( 0.5f, 0.5f,-0.5f);

//			glNormal3f(-1.0f, 0.0f, 0.0f);
//			glVertex3f(-0.5f,-0.5f,-0.5f); glVertex3f(-0.5f,-0.5f, 0.5f);
//			glVertex3f(-0.5f, 0.5f, 0.5f); glVertex3f(-0.5f, 0.5f,-0.5f);
//			glEnd();

//			glFlush();
//			SwapBuffers();
//		//-- CMISS_CALLBACK_LIST_CALL(Graphics_buffer_callback)(
//		//-- 	graphics_buffer->expose_callback_list, graphics_buffer, NULL);
//	}

//	void OnSize(wxSizeEvent& event)
//	{
//		// this is also necessary to update the context on some platforms
//		wxGLCanvas::OnSize(event);

//		//-- CMISS_CALLBACK_LIST_CALL(Graphics_buffer_callback)(
//		//-- 	graphics_buffer->resize_callback_list, graphics_buffer, NULL);
//	}

//	void OnEraseBackground(wxEraseEvent& WXUNUSED(event))
//	{
//		/* Do nothing, to avoid flashing on MSW */
//	}

//	void OnKeyEvent(struct Graphics_buffer_input *input)
//	{
//		input->button_number = 0;
//		input->position_x = cursor_x;
//		input->position_y = cursor_y;
//		//-- CMISS_CALLBACK_LIST_CALL(Graphics_buffer_input_callback)(
//		//-- 	graphics_buffer->input_callback_list, graphics_buffer, input);
//	}

//	void OnKeyUp( wxKeyEvent& event )
//	{
//		struct Graphics_buffer_input input;
//		input.type = GRAPHICS_BUFFER_KEY_RELEASE;
//		key_code = event.GetKeyCode();
//		input.key_code = key_code;
//		int input_modifier = 0;
//		if (event.ShiftDown())
//		{
//			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT;
//		}
//		if (event.ControlDown())
//		{
//			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL;
//		}
//		if (event.AltDown())
//		{
//			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_ALT;
//		}
//		input.input_modifier = static_cast<enum Graphics_buffer_input_modifier>
//			(input_modifier);
//		OnKeyEvent(&input);
//		event.Skip();
//	}

//	void OnKeyDown( wxKeyEvent& event )
//	{
//		struct Graphics_buffer_input input;
//		input.type = GRAPHICS_BUFFER_KEY_PRESS;
//		key_code = event.GetKeyCode();
//		input.key_code = key_code;
//		int input_modifier = 0;
//		if (event.ShiftDown())
//		{
//			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT;
//		}
//		if (event.ControlDown())
//		{
//			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL;
//		}
//		if (event.AltDown())
//		{
//			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_ALT;
//		}
//		input.input_modifier = static_cast<enum Graphics_buffer_input_modifier>
//			(input_modifier);
//		OnKeyEvent(&input);
//		event.Skip();
//	}

//	void OnMouse( wxMouseEvent& event )
//	{
//		int input_modifier, return_code;
//		struct Graphics_buffer_input input;

//		return_code = 1;
//		input.type = GRAPHICS_BUFFER_INVALID_INPUT;
//		input.button_number = 0;
//		input.key_code = key_code;
//		cursor_x = input.position_x = event.GetX();
//		cursor_y = input.position_y = event.GetY();
//		input_modifier = 0;
//		if (event.Leaving())
//		{
//			cursor_x = input.position_x = -1;
//			cursor_y = input.position_y = -1;
//		}
//		if (event.ShiftDown())
//		{
//			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT;
//		}
//		if (event.ControlDown())
//		{
//			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL;
//		}
//		if (event.AltDown())
//		{
//			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_ALT;
//		}

//		if (event.Dragging())
//		{
//			input.type = GRAPHICS_BUFFER_MOTION_NOTIFY;
//			if (event.LeftIsDown())
//			{
//				input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_BUTTON1;
//			}
//		}
//		else if (event.ButtonDown())
//		{
//			if (this != this->FindFocus())
//			{
//				input.key_code = 0;
//				this->SetFocus();
//			}
//			input.type = GRAPHICS_BUFFER_BUTTON_PRESS;
//			switch (event.GetButton())
//			{
//				case wxMOUSE_BTN_LEFT:
//				{
//					input.button_number = 1;
//				} break;
//				case wxMOUSE_BTN_MIDDLE:
//				{
//					input.button_number = 2;
//				} break;
//				case wxMOUSE_BTN_RIGHT:
//				{
//					input.button_number = 3;
//				} break;
//				case wxMOUSE_BTN_NONE:
//				default:
//				{
//					display_message(ERROR_MESSAGE,
//						"wxGraphicsBuffer_input_callback::OnMouse.  Invalid button");
//					return_code=0;
//				} break;
//			}
//		}
//		else if (event.ButtonUp())
//		{
//			input.type = GRAPHICS_BUFFER_BUTTON_RELEASE;
//			switch (event.GetButton())
//			{
//				case wxMOUSE_BTN_LEFT:
//				{
//					input.button_number = 1;
//				} break;
//				case wxMOUSE_BTN_MIDDLE:
//				{
//					input.button_number = 2;
//				} break;
//				case wxMOUSE_BTN_RIGHT:
//				{
//					input.button_number = 3;
//				} break;
//				case wxMOUSE_BTN_NONE:
//				default:
//				{
//					display_message(ERROR_MESSAGE,
//						"wxGraphicsBuffer_input_callback::OnMouse.  Invalid button");
//					return_code=0;
//				} break;
//			}
//		}
//		else
//		{
//		  /* Ignore other events */
//		  return_code=0;
//		}

//		input.input_modifier = static_cast<enum Graphics_buffer_input_modifier>
//			(input_modifier);

//		if (return_code)
//		{
//			//-- CMISS_CALLBACK_LIST_CALL(Graphics_buffer_input_callback)(
//			//-- 	graphics_buffer->input_callback_list, graphics_buffer, &input);
//		}
//	}

//   DECLARE_EVENT_TABLE();
//};

//BEGIN_EVENT_TABLE(wxGraphicsBuffer, wxGLCanvas)
//	EVT_SIZE(wxGraphicsBuffer::OnSize)
//	EVT_PAINT(wxGraphicsBuffer::OnPaint)
//	EVT_ERASE_BACKGROUND(wxGraphicsBuffer::OnEraseBackground)
//	EVT_KEY_UP(wxGraphicsBuffer::OnKeyUp)
//	EVT_KEY_DOWN(wxGraphicsBuffer::OnKeyDown)
//	EVT_MOUSE_EVENTS(wxGraphicsBuffer::OnMouse)
//END_EVENT_TABLE()

//class wxTestingBuffer : public wxGLCanvas
//{
//	 wxPanel *parent;
//	 Graphics_buffer *graphics_buffer;
//	 wxGLContext *sharedContext;

//public:
//	wxTestingBuffer(wxPanel *parent, Graphics_buffer_wx *graphics_buffer, wxGLContext* sharedContext, int *attrib_array)
//		: wxGLCanvas(parent, sharedContext, wxID_ANY, wxDefaultPosition, parent->GetSize(),
//			wxFULL_REPAINT_ON_RESIZE, canvas_name, attrib_array)
//		, parent(parent), graphics_buffer(graphics_buffer), sharedContext(sharedContext)
//	 {
//	 };

//	~wxTestingBuffer()
//	{
//	};

//	void Set_wx_SharedContext()
//	{
//		if (!sharedContext)
//		{
//			graphics_buffer->package->wxSharedContext = GetContext();
//		}
//	}
//};

//int Graphics_buffer_wx::make_current()
///*******************************************************************************
//LAST MODIFIED : 2 July 2002

//DESCRIPTION :
//==============================================================================*/
//{
//	int return_code = 0;

//	ENTER(Graphics_buffer_make_current);

//	switch (type)
//	{
//		case GRAPHICS_BUFFER_WX_TYPE:
//		{
//			if (canvas)
//			{
//				canvas->SetCurrent();
//				return_code = 1;
//			}
//		} break;
//		case GRAPHICS_BUFFER_WX_OFFSCREEN_TYPE:
//		{
//		} break;
//		case GRAPHICS_BUFFER_GL_EXT_FRAMEBUFFER_TYPE:
//		{
//#if defined (OPENGL_API) && defined (GL_EXT_framebuffer_object)
//			if (Graphics_library_check_extension(GL_EXT_framebuffer_object))
//			{
//				if (fbo && depthbuffer && img)
//				{
//					glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
//					glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,depthbuffer);
//					glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT,
//						framebuffer_width, framebuffer_height);
//					glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
//						GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D, img, 0);
//					glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
//						GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthbuffer);
//					GLenum status;
//					status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
//					switch(status)
//					{
//						case GL_FRAMEBUFFER_COMPLETE_EXT:
//						{
//							return_code = 1;
//						}
//						break;
//						case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
//						{
//							display_message(ERROR_MESSAGE,
//								"Graphics_buffer_make_current."
//								"Framebuffer object format not supported.\n");
//							return_code = 0;
//						}
//						break;
//						default:
//						{
//							display_message(ERROR_MESSAGE,
//								"Graphics_buffer_make_current."
//								"Framebuffer object not supported.\n");
//							return_code = 0;
//						}
//					}
//				}
//				else
//				{
//					return_code = 0;
//				}
//			}
//#endif
//		} break;
//		default:
//		{
//			display_message(ERROR_MESSAGE,"Graphics_buffer_make_current.  "
//				"Graphics_bufffer type unknown or not supported.");
//			return_code = 0;
//		} break;
//	}
//	LEAVE;

//	return (return_code);
//} /* Graphics_buffer_make_current */

//#if defined (OPENGL_API) && defined (USE_MSAA)
//void Graphics_buffer_reset_multisample_framebuffer(struct Graphics_buffer_wx *buffer)
//{
//	 if (buffer->multi_fbo)
//			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, buffer->multi_fbo);
//}

//void Graphics_buffer_blit_framebuffer(struct Graphics_buffer_wx *buffer)
//{
//#ifdef GL_EXT_framebuffer_blit
//	int renderbuffer_width, renderbuffer_height;
//	int max_renderbuffer_size = 2048;

//	if (buffer->framebuffer_width > max_renderbuffer_size)
//	{
//		renderbuffer_width = max_renderbuffer_size;
//	}
//	else
//	{
//		renderbuffer_width = buffer->framebuffer_width;
//	}

//	if (buffer->framebuffer_height > max_renderbuffer_size)
//	{
//		renderbuffer_height = max_renderbuffer_size;
//	}
//	else
//	{
//		renderbuffer_height = buffer->framebuffer_height;
//	}
//	if (Graphics_library_check_extension(GL_EXT_framebuffer_blit))
//	{
//		glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, buffer->multi_fbo);
//		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, buffer->fbo);
//		glBlitFramebufferEXT(0, 0, renderbuffer_width, renderbuffer_height,
//			0, 0, renderbuffer_width, renderbuffer_height,
//			GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT, GL_NEAREST);
//		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, buffer->fbo);
//	}
//	else
//	{
//		display_message(INFORMATION_MESSAGE,
//			"Graphics_buffer_blit_framebuffer. glBlitFramebufferEXT not supported\n");
//	}
//#endif /* defined (GL_EXT_framebuffer_blit) */
//}

//int Graphics_buffer_set_multisample_framebuffer(struct Graphics_buffer_wx *buffer, int preferred_antialias)
//{
//#ifdef GL_EXT_framebuffer_multisample
//	 int antialias;
//	 if (Graphics_library_check_extension(GL_EXT_framebuffer_multisample) &&
//			preferred_antialias > 0)
//	 {
//			int max_samples;
//			glGetIntegerv(GL_MAX_SAMPLES_EXT, &max_samples);
//			glGenFramebuffersEXT(1, &buffer->multi_fbo);
//			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, buffer->multi_fbo);
//			if (buffer->img > 0) {
//				 glBindTexture(GL_TEXTURE_2D, buffer->img);
//			}
//			if (preferred_antialias > max_samples)
//			{
//				 antialias = max_samples;
//				 display_message(INFORMATION_MESSAGE,
//						"Preferred antialias exceed the hardware capability.\n"
//						"Max number of multisample framebuffer is: %d\n"
//						"cmgui will set the antialiasing to max.\n",
//						antialias);
//			}
//			else
//			{
//				 antialias = preferred_antialias;
//			}
//			glGenRenderbuffersEXT(1, &buffer->msbuffer);
//			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,buffer->msbuffer);
//			glRenderbufferStorageMultisampleEXT(
//				 GL_RENDERBUFFER_EXT, antialias, GL_RGBA,
//				 buffer->framebuffer_width, buffer->framebuffer_height);
//			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
//				 GL_RENDERBUFFER_EXT, buffer->msbuffer);
//			glGenRenderbuffersEXT(1, &buffer->multi_depthbuffer);
//			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,buffer->multi_depthbuffer);
//			glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, antialias, GL_DEPTH_COMPONENT,
//				  buffer->framebuffer_width,  buffer->framebuffer_height);
//			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
//				 GL_RENDERBUFFER_EXT, buffer->multi_depthbuffer);
//			return 1;
//	 }
//	 else
//	 {
//			display_message(INFORMATION_MESSAGE,
//				 "multisample_framebuffer EXT not available\n");
//			return 0;
//	 }
//#endif /* defined (GL_EXT_framebuffer_multisample) */
//	 return 0;
//}
//#endif /* defined (OPENGL_API) */

//void Graphics_buffer_create_buffer_wx(
//	struct Graphics_buffer_wx *buffer,
//	struct Graphics_buffer_package *graphics_buffer_package,
//	wxGLCanvas *parent,
//	enum Graphics_buffer_buffering_mode buffering_mode,
//	enum Graphics_buffer_stereo_mode stereo_mode,
//	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
//	int minimum_accumulation_buffer_depth,
//	int width, int height, struct Graphics_buffer_wx  *buffer_to_match)
///*******************************************************************************
//LAST MODIFIED : 16 October  2007

//DESCRIPTION :
//==============================================================================*/
//{
//	 int *visual_attributes;
//	 int return_code;
//	ENTER(Graphics_buffer_create_buffer_wx);
//	wxLogNull logNo;
//	if (buffer)
//	{
////		 buffer->parent = parent;
//		 return_code = 0;
//		 if (buffer->type == GRAPHICS_BUFFER_GL_EXT_FRAMEBUFFER_TYPE)
//		 {
//#if defined (OPENGL_API) && defined (GL_EXT_framebuffer_object)
//			 if (Graphics_library_check_extension(GL_EXT_framebuffer_object))
//			 {
//				 GLint buffer_size;
//				 glGenFramebuffersEXT(1,&buffer->fbo);
//				 glGenRenderbuffersEXT(1, &buffer->depthbuffer);
//				 glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE_EXT, &buffer_size);
//				 if (height > buffer_size)
//				 {
//					 display_message(WARNING_MESSAGE,"Graphics_buffer_create_buffer_wx.  "
//						 "Request height is larger than allowed, set height to maximum possible"
//						 "height.");
//					 buffer->framebuffer_height = buffer_size;
//				 }
//				 else
//				 {
//					 buffer->framebuffer_height = height;
//				 }
//				 if (width > buffer_size)
//				 {
//					 display_message(WARNING_MESSAGE,"Graphics_buffer_create_buffer_wx.  "
//						 "Request width is larger than allowed, set width to maximum possible"
//						 "width.");
//					 buffer->framebuffer_width = buffer_size;
//				 }
//				 else
//				 {
//					 buffer->framebuffer_width = width;
//				 }
//				 glGenTextures(1, &buffer->img);
//				 glBindTexture(GL_TEXTURE_2D, buffer->img);
//				 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//				 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//				 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//				 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//				 glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA8, width, height, 0,
//					 GL_RGBA,GL_UNSIGNED_BYTE,NULL);
//			 }
//#endif
//		 }
//		 else if (buffer->type == GRAPHICS_BUFFER_WX_OFFSCREEN_TYPE)
//		 {
//		 }
//		 else
//		 {
//#if defined (UNIX)
//#if !defined (DARWIN)
//				wxGLCanvas *test_canvas;
//				int *attribute_ptr, number_of_visual_attributes, selection_level;
//				visual_attributes = NULL;
//				number_of_visual_attributes = 20;
//				return_code = 0;
//				/* test either there are visual attributes stored in the current
//					 buffer or not*/
//				if (buffer_to_match)
//				{
//					 if (buffer_to_match->attrib_list)
//					 {
//							return_code = 1;
//					 }
//				}
//				//-- if (!return_code)
//				if (0)
//				{
//					 /* if not, test, create a new visual attribute list and create a
//							new canvas, else use the current visual attribute list*/
//					 test_canvas = 0;//-- new wxTestingBuffer(parent, (Graphics_buffer_wx *)NULL,
//							//-- graphics_buffer_package->wxSharedContext,
//							//-- visual_attributes);
//					 if (ALLOCATE(visual_attributes, int, number_of_visual_attributes))
//					 {
//						selection_level = 5;
//						while ((selection_level > 0) && ((test_canvas->m_vi == NULL) || (selection_level == 5)))
//						{
//							 attribute_ptr = visual_attributes;
//							 *attribute_ptr = WX_GL_RGBA;
//							 attribute_ptr++;
//							 *attribute_ptr = WX_GL_MIN_RED;
//							 attribute_ptr++;
//							 *attribute_ptr = (minimum_colour_buffer_depth + 2) / 3;
//							 attribute_ptr++;
//							 *attribute_ptr = WX_GL_MIN_GREEN;
//							 attribute_ptr++;
//							 *attribute_ptr = (minimum_colour_buffer_depth + 2) / 3;
//							 attribute_ptr++;
//							 *attribute_ptr = WX_GL_MIN_BLUE;
//							 attribute_ptr++;
//							 *attribute_ptr = (minimum_colour_buffer_depth + 2) / 3;
//							 attribute_ptr++;
//							 if (selection_level > 3)
//							 {
//								*attribute_ptr = WX_GL_MIN_ALPHA;
//								attribute_ptr++;
//								*attribute_ptr = 1;
//								attribute_ptr++;
//							 }
//							 if (minimum_depth_buffer_depth > 0)
//							 {
//								*attribute_ptr = WX_GL_DEPTH_SIZE;
//								attribute_ptr++;
//								*attribute_ptr = minimum_depth_buffer_depth;
//								attribute_ptr++;
//							 }
//							 else
//							 {
//								if (selection_level > 2)
//								{
//									 /* Try to get a depth buffer anyway */
//									 *attribute_ptr = WX_GL_DEPTH_SIZE;
//									 attribute_ptr++;
//									 *attribute_ptr = 16;
//									 attribute_ptr++;
//								}
//							 }
//							 if (minimum_accumulation_buffer_depth > 0)
//							 {
//								*attribute_ptr = WX_GL_MIN_ACCUM_RED;
//								attribute_ptr++;
//								*attribute_ptr = (minimum_accumulation_buffer_depth + 2) / 3;
//								attribute_ptr++;
//								*attribute_ptr = WX_GL_MIN_ACCUM_GREEN;
//								attribute_ptr++;
//								*attribute_ptr = (minimum_accumulation_buffer_depth + 2) / 3;
//								attribute_ptr++;
//								*attribute_ptr = WX_GL_MIN_ACCUM_BLUE;
//								attribute_ptr++;
//								*attribute_ptr = (minimum_accumulation_buffer_depth + 2) / 3;
//								attribute_ptr++;
//							 }
//							 else
//							 {
//								if (selection_level > 4)
//								{
//									 /* Try to get an accumulation buffer anyway */
//									 *attribute_ptr = WX_GL_MIN_ACCUM_RED;
//									 attribute_ptr++;
//									 *attribute_ptr = (minimum_colour_buffer_depth + 2) / 3;
//									 attribute_ptr++;
//									 *attribute_ptr = WX_GL_MIN_ACCUM_GREEN;
//									 attribute_ptr++;
//									 *attribute_ptr = (minimum_colour_buffer_depth + 2) / 3;
//									 attribute_ptr++;
//									 *attribute_ptr = WX_GL_MIN_ACCUM_BLUE;
//									 attribute_ptr++;
//									 *attribute_ptr = (minimum_colour_buffer_depth + 2) / 3;
//									 attribute_ptr++;
//								}
//							 }
//							 switch (buffering_mode)
//							 {
//								case GRAPHICS_BUFFER_SINGLE_BUFFERING:
//								case GRAPHICS_BUFFER_DOUBLE_BUFFERING:
//								{
//									 *attribute_ptr = WX_GL_DOUBLEBUFFER;
//									 attribute_ptr++;
//								}break;
//								case GRAPHICS_BUFFER_ANY_BUFFERING_MODE:
//								case GRAPHICS_BUFFER_RENDER_OFFSCREEN_AND_COPY:
//								case GRAPHICS_BUFFER_RENDER_OFFSCREEN_AND_BLEND:
//								{
//									/* Do nothing */
//								} break;
//							 }
//							 switch (stereo_mode)
//							 {
//								case GRAPHICS_BUFFER_MONO:
//								case GRAPHICS_BUFFER_STEREO:
//								{
//									 *attribute_ptr = GL_STEREO;
//									 attribute_ptr++;
//								} break;
//								case GRAPHICS_BUFFER_ANY_STEREO_MODE:
//								{
//									/* default GRAPHICS_BUFFER_ANY_STEREO_MODE*/
//								} break;
//							 }
//							 *attribute_ptr = 0;
//							 attribute_ptr++;
//							 if (test_canvas)
//							 {
//									delete test_canvas;
//							 }
//							 test_canvas = 0;//-- new wxTestingBuffer(parent, (Graphics_buffer_wx *)NULL,
//									//-- graphics_buffer_package->wxSharedContext,
//									//-- visual_attributes);
//							 selection_level--;
//							 if ((selection_level == 0) && (test_canvas->m_vi == NULL))
//							 {
//									DEALLOCATE(visual_attributes);
//									visual_attributes = NULL;
//									buffer->attrib_list = visual_attributes;
//							 }
//							 else if(test_canvas->m_vi != NULL)
//							 {
//									buffer->attrib_list = visual_attributes;
//							 }
//						}
//						if (test_canvas)
//						{
//							 delete test_canvas;
//						}
//					 }
//				}
//				else
//				{
//					 if (0)//-- buffer_to_match->attrib_list)
//					 {
//							/* if attrib_list is found on the buffer to match, copy it
//								 into the new buffer, if not found, that means the
//								 current buffer does not have any special attributes
//								 setting, thus the new attributes will be default as NULL */
//							int count;
//							int *buffer_to_match_attribute_ptr;
//							if (ALLOCATE(buffer->attrib_list,int, number_of_visual_attributes))
//							{
//								 buffer_to_match_attribute_ptr = buffer_to_match->attrib_list;
//								 attribute_ptr = buffer->attrib_list;
//								 for (count = 0; count < number_of_visual_attributes; count++)
//								 {
//										*attribute_ptr = *buffer_to_match_attribute_ptr;
//										attribute_ptr++;
//										buffer_to_match_attribute_ptr++;
//								 }
//							}
//					 }
//				}
//#else /*defined (DARWIN) */
//				/* Mac will receive an argument from wxGLCanvas to get
//				   the best settings but requires the program to state
//					 all the desired settings with a minimum settings. */
//				visual_attributes = NULL;
//				if (ALLOCATE(buffer->attrib_list, int, 25))
//				{
//					buffer->attrib_list[0] = WX_GL_RGBA;
//					buffer->attrib_list[1] = WX_GL_DOUBLEBUFFER;
//					buffer->attrib_list[2] = WX_GL_DEPTH_SIZE;
//					buffer->attrib_list[3] = 1;
//					buffer->attrib_list[4] = WX_GL_MIN_RED;
//					buffer->attrib_list[5] = 1;
//					buffer->attrib_list[6] = WX_GL_MIN_GREEN;
//					buffer->attrib_list[7] = 1;
//					buffer->attrib_list[8] = WX_GL_MIN_BLUE;
//					buffer->attrib_list[9] = 1;
//					buffer->attrib_list[10] = WX_GL_MIN_ALPHA;
//					buffer->attrib_list[11] = 1;
//					buffer->attrib_list[12] = WX_GL_MIN_ACCUM_RED;
//					buffer->attrib_list[13] = 1;
//					buffer->attrib_list[14] = WX_GL_MIN_ACCUM_GREEN;
//					buffer->attrib_list[15] = 1;
//					buffer->attrib_list[16] = WX_GL_MIN_ACCUM_BLUE;
//					buffer->attrib_list[17] = 1;
//					buffer->attrib_list[18] = WX_GL_MIN_ACCUM_ALPHA;
//					buffer->attrib_list[19] = 1;
//					buffer->attrib_list[20] = WX_GL_DEPTH_SIZE;
//					buffer->attrib_list[21] = 1;
//					buffer->attrib_list[22] = WX_GL_STENCIL_SIZE;
//					buffer->attrib_list[23] = 1;
//					buffer->attrib_list[24] = 0;
//				};
//#endif /*defined (DARWIN) */
//#else /* defined (UNIX) */
//				USE_PARAMETER(buffer_to_match);
//				USE_PARAMETER(buffering_mode);
//				USE_PARAMETER(minimum_accumulation_buffer_depth);
//				USE_PARAMETER(minimum_colour_buffer_depth);
//				USE_PARAMETER(minimum_depth_buffer_depth);
//				USE_PARAMETER(stereo_mode);

//				/* The above routine does not work for win32 as it does not have the
//					 member m_vi in wxGLCanvas.
//					 should find a way to get the best buffer, but this default setting should work fine. */
//				visual_attributes = NULL;
//				if (ALLOCATE(buffer->attrib_list, int, 5))
//				{
//					 buffer->attrib_list[0] = WX_GL_DOUBLEBUFFER;
//					 buffer->attrib_list[1] = WX_GL_RGBA;
//					 buffer->attrib_list[2] = WX_GL_MIN_ALPHA;
//					 buffer->attrib_list[3] = 8;
//					 buffer->attrib_list[4] = 0;
//				}
//#endif /* defined (UNIX) */
//				if (!buffer->package->wxSharedContext)
//				{
//					wxFrame *frame = new wxFrame(parent, -1, wxString::FromAscii("temporary"));
//					wxPanel *temp = new wxPanel(frame);
//					wxTestingBuffer *testingbuffer;
//					struct Graphics_buffer_wx *temp_buffer;
//					temp_buffer = CREATE(Graphics_buffer_wx)(graphics_buffer_package);
//					temp_buffer->type= GRAPHICS_BUFFER_WX_TYPE;
//					temp_buffer->parent = temp;
//					temp_buffer->attrib_list = NULL;
//					testingbuffer = new wxTestingBuffer(temp, temp_buffer,
//						graphics_buffer_package->wxSharedContext,
//						buffer->attrib_list);
//					testingbuffer->Set_wx_SharedContext();
//					frame->Show(false);
//					DESTROY(Graphics_buffer_wx)(&temp_buffer);
//				}
//				buffer->canvas = new wxGraphicsBuffer(parent,
//					graphics_buffer_package->wxSharedContext,
//					buffer, buffer->attrib_list);
//				wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
//				topsizer->Add(buffer->canvas, 1, wxEXPAND);//wxSizerFlags(1).Align(wxALIGN_CENTER).Expand());
//				parent->SetSizer(topsizer);
//		 }
//	}
//	else
//	{
//		 display_message(ERROR_MESSAGE,"Graphics_buffer_create_buffer_wx.  "
//				"Unable to create generic Graphics_buffer.");
//		 buffer = 0;
//	}
//	LEAVE;

//} /* Graphics_buffer_create_buffer_wx */

/*
Global functions
----------------
*/

//struct Graphics_buffer_package *CREATE(Graphics_buffer_package)(
//	struct User_interface *user_interface)
///*******************************************************************************
//LAST MODIFIED : 6 May 2004

//DESCRIPTION :
//Creates a Graphics_buffer_package which enables Graphics_buffers created from
//it to share graphics contexts.
//==============================================================================*/
//{
//	struct Graphics_buffer_package *package;

//	ENTER(CREATE(Graphics_buffer_package));

//	USE_PARAMETER(user_interface);
//	if (ALLOCATE(package, struct Graphics_buffer_package, 1))
//	{
//		package->override_visual_id = 0;
//		package->wxSharedContext = (wxGLContext*)NULL;
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,"CREATE(Graphics_buffer_package). "
//			"Unable to allocate package structure");
//		package = (struct Graphics_buffer_package *)NULL;
//	}

//	LEAVE;
//	return (package);
//} /* CREATE(Graphics_buffer_package) */

//int DESTROY(Graphics_buffer_package)(struct Graphics_buffer_package **package_ptr)
///*******************************************************************************
//LAST MODIFIED : 6 May 2004

//DESCRIPTION :
//Closes the Graphics buffer package
//==============================================================================*/
//{
//	int return_code;
//	struct Graphics_buffer_package *package;

//	ENTER(DESTROY(Graphics_buffer_package));
//	if (package_ptr && (package = *package_ptr))
//	{
//		return_code=1;
//		DEALLOCATE(*package_ptr);
//		*package_ptr = (struct Graphics_buffer_package *)NULL;
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,
//			"DESTROY(Graphics_buffer_package).  Missing package");
//		return_code=0;
//	}
//	LEAVE;

//	return (return_code);
//} /* DESTROY(Graphics_buffer_package) */

//int Graphics_buffer_package_set_override_visual_id(
//	struct Graphics_buffer_package *graphics_buffer_package,
//	int override_visual_id)
///*******************************************************************************
//LAST MODIFIED : 21 May 2004

//DESCRIPTION :
//Sets a particular visual to be used by all graphics buffers.
//==============================================================================*/
//{
//	int return_code;

//	ENTER(Graphics_buffer_package_set_override_visual_id);
//	if (graphics_buffer_package)
//	{
//		graphics_buffer_package->override_visual_id = override_visual_id;
//		return_code = 1;
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,"Graphics_buffer_package_set_override_visual_id.  "
//			"Invalid argument");
//		return_code = 0;
//	}
//	LEAVE;

//	return (return_code);
//} /* Graphics_buffer_package_set_override_visual_id */

//struct Graphics_buffer_wx *create_Graphics_buffer_offscreen(
//	struct Graphics_buffer_package *graphics_buffer_package,
//	int width, int height,
//	enum Graphics_buffer_buffering_mode buffering_mode,
//	enum Graphics_buffer_stereo_mode stereo_mode,
//	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
//	int minimum_accumulation_buffer_depth)
///*******************************************************************************
//LAST MODIFIED : 6 May 2004

//DESCRIPTION :
//==============================================================================*/
//{
//	struct Graphics_buffer_wx *buffer;

//	ENTER(create_Graphics_buffer_offscreen);

//	buffer = CREATE(Graphics_buffer_wx)(graphics_buffer_package, GRAPHICS_BUFFER_WX_TYPE);
//	if (buffer != NULL)
//	{
//		USE_PARAMETER(width);
//		USE_PARAMETER(height);
//		USE_PARAMETER(buffering_mode);
//		USE_PARAMETER(stereo_mode);
//		USE_PARAMETER(minimum_colour_buffer_depth);
//		USE_PARAMETER(minimum_depth_buffer_depth);
//		USE_PARAMETER(minimum_accumulation_buffer_depth);
//		if (buffer->type == GRAPHICS_BUFFER_INVALID_TYPE)
//		{
//#if defined (DEBUG_CODE)
//			display_message(ERROR_MESSAGE,"create_Graphics_buffer_offscreen.  "
//				"Unable to create offscreen graphics buffer.");
//#endif /* defined (DEBUG_CODE) */
//			DESTROY(Graphics_buffer_wx)(&buffer);
//		}
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,"create_Graphics_buffer_offscreen.  "
//			"Unable to create generic Graphics_buffer.");
//		buffer = 0;
//	}
//	LEAVE;

//	return (buffer);
//} /* create_Graphics_buffer_offscreen */

//struct Graphics_buffer_wx *create_Graphics_buffer_shared_offscreen(
//	struct Graphics_buffer_package *graphics_buffer_package,
//	int width, int height,
//	enum Graphics_buffer_buffering_mode buffering_mode,
//	enum Graphics_buffer_stereo_mode stereo_mode,
//	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
//	int minimum_accumulation_buffer_depth)
///*******************************************************************************
//LAST MODIFIED : 6 May 2004

//DESCRIPTION :
//==============================================================================*/
//{
//	struct Graphics_buffer_wx *buffer;

//	ENTER(create_Graphics_buffer_offscreen);

//	buffer = CREATE(Graphics_buffer_wx)(graphics_buffer_package, GRAPHICS_BUFFER_WX_TYPE);
//	if (buffer != NULL)
//	{
//		USE_PARAMETER(width);
//		USE_PARAMETER(height);
//		USE_PARAMETER(buffering_mode);
//		USE_PARAMETER(stereo_mode);
//		USE_PARAMETER(minimum_colour_buffer_depth);
//		USE_PARAMETER(minimum_depth_buffer_depth);
//		USE_PARAMETER(minimum_accumulation_buffer_depth);
//		if (buffer->type == GRAPHICS_BUFFER_INVALID_TYPE)
//		{
//#if defined (DEBUG_CODE)
//			display_message(ERROR_MESSAGE,"create_Graphics_buffer_offscreen.  "
//				"Unable to create offscreen graphics buffer.");
//#endif /* defined (DEBUG_CODE) */
//			DESTROY(Graphics_buffer_wx)(&buffer);
//		}
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,"create_Graphics_buffer_offscreen.  "
//			"Unable to create generic Graphics_buffer.");
//		buffer = 0;
//	}
//	LEAVE;

//	return (buffer);
//} /* create_Graphics_buffer_offscreen */

//struct Graphics_buffer_wx *create_Graphics_buffer_offscreen_from_buffer(
//	int width, int height, struct Graphics_buffer_wx *buffer_to_match)
///*******************************************************************************
//LAST MODIFIED : 6 May 2004

//DESCRIPTION :
//==============================================================================*/
//{
//	struct Graphics_buffer_wx *buffer;

//	ENTER(create_Graphics_buffer_offscreen_from_buffer);

//	buffer = CREATE(Graphics_buffer_wx)(buffer_to_match->package, GRAPHICS_BUFFER_WX_TYPE);
//	if (buffer != NULL)
//	{
//		buffer->type = GRAPHICS_BUFFER_WX_OFFSCREEN_TYPE;
//#if defined (OPENGL_API) && (GL_EXT_framebuffer_object)
//		if (Graphics_library_load_extension("GL_EXT_framebuffer_object"))
//		{
//			buffer->type = GRAPHICS_BUFFER_GL_EXT_FRAMEBUFFER_TYPE;
//		}
//#endif
//		Graphics_buffer_create_buffer_wx(buffer, buffer_to_match->package,
//			NULL, GRAPHICS_BUFFER_ANY_BUFFERING_MODE,
//			GRAPHICS_BUFFER_ANY_STEREO_MODE,
//			0, 0, 0, width, height,
//			buffer_to_match);
//		if (buffer->type == GRAPHICS_BUFFER_INVALID_TYPE)
//		{
//#if defined (DEBUG_CODE)
//			display_message(ERROR_MESSAGE,"create_Graphics_buffer_offscreen_from_buffer.  "
//				"Unable to create offscreen_from_buffer graphics buffer.");
//			buffer = (struct Graphics_buffer *)NULL;
//#endif /* defined (DEBUG_CODE) */
//			DESTROY(Graphics_buffer_wx)(&buffer);
//		}
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,"create_Graphics_buffer_offscreen_from_buffer.  "
//			"Unable to create generic Graphics_buffer.");
//		buffer = 0;
//	}
//	LEAVE;

//	return (buffer);
//} /* create_Graphics_buffer_offscreen_from_buffer */

//struct Graphics_buffer_wx *create_Graphics_buffer_wx(
//	struct Graphics_buffer_package *graphics_buffer_package,
//	wxGLCanvas *canvas,
//	enum Graphics_buffer_buffering_mode buffering_mode,
//	enum Graphics_buffer_stereo_mode stereo_mode,
//	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
//	int minimum_accumulation_buffer_depth,
//	struct Graphics_buffer_wx  *buffer_to_match)
//{
//	 struct Graphics_buffer_wx *buffer;

//	ENTER(create_Graphics_buffer_wx);
//	buffer = CREATE(Graphics_buffer_wx)(graphics_buffer_package, GRAPHICS_BUFFER_WX_TYPE);
//	if (buffer != NULL)
//	{
//		 Graphics_buffer_create_buffer_wx(buffer, graphics_buffer_package,
//				canvas, buffering_mode, stereo_mode, minimum_colour_buffer_depth,
//				minimum_depth_buffer_depth, minimum_accumulation_buffer_depth, 0, 0,
//				buffer_to_match);
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,"create_Graphics_buffer_wx.  "
//			"Unable to create generic Graphics_buffer.");
//		buffer = 0;
//	}
//	LEAVE;

//	return (buffer);
//}

//struct Graphics_buffer *create_Graphics_buffer(
//		struct Graphics_buffer_package *graphics_buffer_package,
//		wxGLCanvas *canvas)
//{
//	struct Graphics_buffer *buffer = CREATE(Graphics_buffer_wx)(graphics_buffer_package, GRAPHICS_BUFFER_WX_TYPE);

//	return buffer;
//}

//int Graphics_buffer_make_current(struct Graphics_buffer_wx *buffer)
///*******************************************************************************
//LAST MODIFIED : 2 July 2002

//DESCRIPTION :
//==============================================================================*/
//{
//	int return_code = 0;

//	ENTER(Graphics_buffer_make_current);

//	if (buffer)
//	{
//#if defined (DEBUG_CODE)
//		printf("Graphics_buffer_make_current\n");
//#endif /* defined (DEBUG_CODE) */
//		switch (buffer->type)
//		{
//			case GRAPHICS_BUFFER_WX_TYPE:
//			{
//				 if (buffer->canvas)
//				 {
//						buffer->canvas->SetCurrent();
//						return_code = 1;
//				 }
//				 else
//				 {
//						return_code = 0;
//				 }
//			} break;
//			 case GRAPHICS_BUFFER_WX_OFFSCREEN_TYPE:
//			{
//				return_code = 0;
//			} break;
//			case GRAPHICS_BUFFER_GL_EXT_FRAMEBUFFER_TYPE:
//			{
//#if defined (OPENGL_API) && defined (GL_EXT_framebuffer_object)
//				if (Graphics_library_check_extension(GL_EXT_framebuffer_object))
//				{
//					if (buffer->fbo && buffer->depthbuffer && buffer->img)
//					{
//						glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, buffer->fbo);
//						glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,buffer->depthbuffer);
//						glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT,
//							buffer->framebuffer_width, buffer->framebuffer_height);
//						glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
//							GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D, buffer->img, 0);
//						glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
//							GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, buffer->depthbuffer);
//						GLenum status;
//						status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
//						switch(status)
//						{
//							case GL_FRAMEBUFFER_COMPLETE_EXT:
//							{
//								return_code = 1;
//							}
//							break;
//							case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
//							{
//								display_message(ERROR_MESSAGE,
//									"Graphics_buffer_make_current."
//									"Framebuffer object format not supported.\n");
//								return_code = 0;
//							}
//							break;
//							default:
//							{
//								display_message(ERROR_MESSAGE,
//									"Graphics_buffer_make_current."
//									"Framebuffer object not supported.\n");
//								return_code = 0;
//							}
//						}
//					}
//					else
//					{
//						return_code = 0;
//					}
//				}
//#endif
//			} break;
//			default:
//			{
//				display_message(ERROR_MESSAGE,"Graphics_buffer_make_current.  "
//					"Graphics_bufffer type unknown or not supported.");
//				return_code = 0;
//			} break;
//		}
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,"Graphics_buffer_make_current.  "
//			"Graphics_bufffer missing.");
//		return_code = 0;
//	}
//	LEAVE;

//	return (return_code);
//} /* Graphics_buffer_make_current */

//int Graphics_buffer_get_visual_id(struct Graphics_buffer_wx *buffer, int *visual_id)
///*******************************************************************************
//LAST MODIFIED : 19 September 2002

//DESCRIPTION :
//Returns the visual id used by the graphics buffer.
//==============================================================================*/
//{
//	int return_code;

//	ENTER(Graphics_buffer_get_visual_id);

//	if (buffer)
//	{
//		switch (buffer->type)
//		{
//			case GRAPHICS_BUFFER_WX_TYPE:
//			{
//#if defined (__WXGTK__)
//				*visual_id = (int)((XVisualInfo*)buffer->canvas->m_vi)
//					->visualid;
//				return_code = 1;
//#else /* if defined (__WXGTK__) */
//				*visual_id = 0;
//				return_code = 0;
//#endif /* if defined (__WXGTK__) */
//			} break;
//			default:
//			{
//				display_message(ERROR_MESSAGE,"Graphics_buffer_get_visual_id.  "
//					"Graphics_bufffer type unknown or not supported.");
//				return_code = 0;
//			} break;
//		}
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,"Graphics_buffer_get_visual_id.  "
//			"Graphics_bufffer missing.");
//		return_code = 0;
//	}
//	LEAVE;

//	return (return_code);
//} /* Graphics_buffer_get_visual_id */

//int Graphics_buffer_get_colour_buffer_depth(struct Graphics_buffer *buffer,
//	int *colour_buffer_depth)
///*******************************************************************************
//LAST MODIFIED : 19 September 2002

//DESCRIPTION :
//Returns the depth of the colour buffer used by the graphics buffer.
//==============================================================================*/
//{
//	int return_code;

//	ENTER(Graphics_buffer_get_colour_buffer_depth);
//	if (buffer)
//	{
//		return_code = 1;
//		switch (buffer->type)
//		{
//			case GRAPHICS_BUFFER_WX_TYPE:
//			{
//				GLint colour_buffer_bits;
//				glGetIntegerv(GL_RED_BITS, &colour_buffer_bits);
//				*colour_buffer_depth = colour_buffer_bits;
//				glGetIntegerv(GL_BLUE_BITS, &colour_buffer_bits);
//				*colour_buffer_depth += colour_buffer_bits;
//				glGetIntegerv(GL_GREEN_BITS, &colour_buffer_bits);
//				*colour_buffer_depth += colour_buffer_bits;
//				glGetIntegerv(GL_ALPHA_BITS, &colour_buffer_bits);
//				*colour_buffer_depth += colour_buffer_bits;
//				return_code = 1;
//			} break;
//			default:
//			{
//				display_message(ERROR_MESSAGE,"Graphics_buffer_get_colour_buffer_depth.  "
//					"Graphics_buffer type unknown or not supported.");
//				*colour_buffer_depth = 0;
//				return_code = 0;
//			} break;
//		}
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,"Graphics_buffer_get_colour_buffer_depth.  "
//			"Graphics_bufffer missing.");
//		return_code = 0;
//	}
//	LEAVE;

//	return (return_code);
//} /* Graphics_buffer_get_colour_buffer_depth */

//int Graphics_buffer_get_depth_buffer_depth(struct Graphics_buffer *buffer,
//	int *depth_buffer_depth)
///*******************************************************************************
//LAST MODIFIED : 19 September 2002

//DESCRIPTION :
//Returns the depth of the depth buffer used by the graphics buffer.
//==============================================================================*/
//{
//	int return_code;

//	ENTER(Graphics_buffer_get_depth_buffer_depth);
//	if (buffer)
//	{
//		return_code = 1;
//		switch (buffer->type)
//		{
//			 case GRAPHICS_BUFFER_WX_TYPE:
//			{
//				 GLint depth_bits;
//				 glGetIntegerv(GL_DEPTH_BITS, &depth_bits);
//				 *depth_buffer_depth = depth_bits;
//				 return_code = 1;
//			} break;
//			default:
//			{
//				display_message(ERROR_MESSAGE,"Graphics_buffer_get_depth_buffer_depth.  "
//					"Graphics_bufffer type unknown or not supported.");
//				*depth_buffer_depth = 0;
//				return_code = 0;
//			} break;
//		}
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,"Graphics_buffer_get_depth_buffer_depth.  "
//			"Graphics_bufffer missing.");
//		return_code = 0;
//	}
//	LEAVE;

//	return (return_code);
//} /* Graphics_buffer_get_depth_buffer_depth */

//int Graphics_buffer_get_accumulation_buffer_depth(struct Graphics_buffer *buffer,
//	int *accumulation_buffer_depth)
///*******************************************************************************
//LAST MODIFIED : 19 September 2002

//DESCRIPTION :
//Returns the depth of the accumulation buffer used by the graphics buffer.
//==============================================================================*/
//{
//	int return_code;

//	ENTER(Graphics_buffer_get_accumulation_buffer_depth);
//	if (buffer)
//	{
//		return_code = 1;
//		switch (buffer->type)
//		{
//			 case GRAPHICS_BUFFER_WX_TYPE:
//			{
//				 GLint accumulation_buffer_bits;
//				 glGetIntegerv(GL_ACCUM_RED_BITS, &accumulation_buffer_bits);
//				 *accumulation_buffer_depth = accumulation_buffer_bits;
//				 glGetIntegerv(GL_ACCUM_BLUE_BITS, &accumulation_buffer_bits);
//				 *accumulation_buffer_depth += accumulation_buffer_bits;
//				 glGetIntegerv(GL_ACCUM_GREEN_BITS, &accumulation_buffer_bits);
//				 *accumulation_buffer_depth += accumulation_buffer_bits;
//				 glGetIntegerv(GL_ACCUM_ALPHA_BITS, &accumulation_buffer_bits);
//				 *accumulation_buffer_depth += accumulation_buffer_bits;
//				 return_code = 1;
//			} break;
//			default:
//			{
//				display_message(ERROR_MESSAGE,"Graphics_buffer_get_accumulation_buffer_depth.  "
//					"Graphics_bufffer type unknown or not supported.");
//				*accumulation_buffer_depth = 0;
//				return_code = 0;
//			} break;
//		}
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,"Graphics_buffer_get_accumulation_buffer_depth.  "
//			"Graphics_bufffer missing.");
//		return_code = 0;
//	}
//	LEAVE;

//	return (return_code);
//} /* Graphics_buffer_get_accumulation_buffer_depth */

//int Graphics_buffer_get_buffering_mode(struct Graphics_buffer *buffer,
//	enum Graphics_buffer_buffering_mode *buffering_mode)
///*******************************************************************************
//LAST MODIFIED : 19 September 2002

//DESCRIPTION :
//Returns the buffering mode used by the graphics buffer.
//==============================================================================*/
//{
//	int return_code;

//	ENTER(Graphics_buffer_get_buffering_mode);
//	if (buffer)
//	{
//		switch (buffer->type)
//		{
//			case GRAPHICS_BUFFER_WX_TYPE:
//			{
//				*buffering_mode = GRAPHICS_BUFFER_DOUBLE_BUFFERING;
//				return_code = 1;
//			} break;
//			default:
//			{
//				display_message(ERROR_MESSAGE,"Graphics_buffer_get_buffering_mode.  "
//					"Graphics_bufffer type unknown or not supported.");
//				return_code = 0;
//			} break;
//		}
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,"Graphics_buffer_get_buffering_mode.  "
//			"Graphics_bufffer missing.");
//		return_code = 0;
//	}
//	LEAVE;

//	return (return_code);
//} /* Graphics_buffer_get_buffering_mode */

//int Graphics_buffer_get_stereo_mode(struct Graphics_buffer *buffer,
//	enum Graphics_buffer_stereo_mode *stereo_mode)
///*******************************************************************************
//LAST MODIFIED : 19 September 2002

//DESCRIPTION :
//Returns the stereo mode used by the graphics buffer.
//==============================================================================*/
//{
//	int return_code;
//	ENTER(Graphics_buffer_get_stereo_mode);
//	if (buffer)
//	{
//		switch (buffer->type)
//		{
//			case GRAPHICS_BUFFER_WX_TYPE:
//			{
//				*stereo_mode = GRAPHICS_BUFFER_MONO;
//				return_code = 1;
//			} break;
//			default:
//			{
//				display_message(ERROR_MESSAGE,"Graphics_buffer_get_stereo_mode.  "
//					"Graphics_bufffer type unknown or not supported.");
//				return_code = 0;
//			} break;
//		}
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,"Graphics_buffer_get_stereo_mode.  "
//			"Graphics_bufffer missing.");
//		return_code = 0;
//	}
//	LEAVE;

//	return (return_code);
//} /* Graphics_buffer_get_stereo_mode */

//int Graphics_buffer_swap_buffers(struct Graphics_buffer_wx *buffer)
///*******************************************************************************
//LAST MODIFIED : 2 July 2002

//DESCRIPTION :
//==============================================================================*/
//{
//	int return_code;

//	ENTER(Graphics_buffer_swap_buffers);

//	if (buffer)
//	{
//		switch (buffer->type)
//		{
//			case GRAPHICS_BUFFER_WX_TYPE:
//			{
//				buffer->canvas->SwapBuffers();
//				return_code = 1;
//			} break;
//			default:
//			{
//				display_message(ERROR_MESSAGE,"Graphics_buffer_swap_buffers.  "
//					"Graphics_bufffer type unknown or not supported.");
//				return_code = 0;
//			} break;
//		}
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,"Graphics_buffer_swap_buffers.  "
//			"Graphics_bufffer missing.");
//		return_code = 0;
//	}
//	LEAVE;

//	return (return_code);
//} /* Graphics_buffer_swap_buffers */

//int Graphics_buffer_make_read_current(struct Graphics_buffer *buffer)
///*******************************************************************************
//LAST MODIFIED : 28 May 2004

//DESCRIPTION :
//Sets this buffer to be the GLX source and the current ThreeDWindow (the one last
//made current) to be the GLX destination.
//==============================================================================*/
//{
//	int return_code = 0;

//	ENTER(Graphics_buffer_make_read_current);

//	if (buffer)
//	{

//		display_message(ERROR_MESSAGE,"Graphics_buffer_make_read_current.  "
//			"Graphics_bufffer type unknown or not supported.");

//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,"Graphics_buffer_make_read_current.  "
//			"Graphics_bufffer missing.");
//	}
//	LEAVE;

//	return (return_code);
//} /* Graphics_buffer_make_read_current */

//int Graphics_buffer_get_width(struct Graphics_buffer_wx *buffer)
///*******************************************************************************
//LAST MODIFIED : 1 July 2002

//DESCRIPTION :
//Returns the width of buffer represented by <buffer>.
//==============================================================================*/
//{
//	int width;

//	ENTER(Graphics_buffer_get_width);
//	if (buffer)
//	{
//		switch (buffer->type)
//		{
//			case GRAPHICS_BUFFER_WX_TYPE:
//			{
//				int height;
//				buffer->canvas->GetClientSize(&width, &height);
//			} break;
//			default:
//			{
//				display_message(ERROR_MESSAGE,"Graphics_buffer_get_width.  "
//					"Graphics_bufffer type unknown or not supported.");
//				width = 0;
//			} break;
//		}
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,
//			"Graphics_buffer_get_width.  Invalid buffer");
//		width = 0;
//	}
//	LEAVE;

//	return (width);
//} /* Graphics_buffer_get_width */

//int Graphics_buffer_set_width(struct Graphics_buffer_wx *buffer, int width)
///*******************************************************************************
//LAST MODIFIED : 1 July 2002

//DESCRIPTION :
//Sets the width of buffer represented by <buffer>.
//==============================================================================*/
//{
//	int return_code;

//	ENTER(Graphics_buffer_set_width);
//	USE_PARAMETER(width);
//	if (buffer)
//	{
//		switch (buffer->type)
//		{
//			case GRAPHICS_BUFFER_WX_TYPE:
//			{
//				int old_width, height;
//				buffer->canvas->GetClientSize(&old_width, &height);
//				buffer->canvas->SetClientSize(width, height);
//				return_code = 1;
//			} break;
//			default:
//			{
//				display_message(ERROR_MESSAGE,"Graphics_buffer_set_width.  "
//					"Graphics_bufffer type unknown or not supported.");
//				return_code = 0;
//			} break;
//		}
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,
//			"Graphics_buffer_set_width.  Invalid buffer");
//		return_code = 1;
//	}
//	LEAVE;

//	return (return_code);
//} /* Graphics_buffer_set_width */

//int Graphics_buffer_get_height(struct Graphics_buffer_wx *buffer)
///*******************************************************************************
//LAST MODIFIED : 1 July 2002

//DESCRIPTION :
//Returns the height of buffer represented by <buffer>.
//==============================================================================*/
//{
//	int height;

//	ENTER(Graphics_buffer_get_height);
//	if (buffer)
//	{
//		switch (buffer->type)
//		{
//			case GRAPHICS_BUFFER_WX_TYPE:
//			{
//				int width;
//				buffer->canvas->GetClientSize(&width, &height);
//			} break;
//			default:
//			{
//				display_message(ERROR_MESSAGE,"Graphics_buffer_get_height.  "
//					"Graphics_bufffer type unknown or not supported.");
//				height = 0;
//			} break;
//		}
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,
//			"Graphics_buffer_get_height.  Invalid buffer");
//		height = 0;
//	}
//	LEAVE;

//	return (height);
//} /* Graphics_buffer_get_height */

//int Graphics_buffer_set_height(struct Graphics_buffer_wx *buffer, int height)
///*******************************************************************************
//LAST MODIFIED : 1 July 2002

//DESCRIPTION :
//Sets the height of buffer represented by <buffer>.
//==============================================================================*/
//{
//	int return_code;

//	ENTER(Graphics_buffer_set_height);
//	USE_PARAMETER(height);
//	if (buffer)
//	{
//		switch (buffer->type)
//		{
//			case GRAPHICS_BUFFER_WX_TYPE:
//			{
//				int width, old_height;
//				buffer->canvas->GetClientSize(&width, &old_height);
//				buffer->canvas->SetClientSize(width, height);
//				return_code = 1;
//			} break;
//			default:
//			{
//				display_message(ERROR_MESSAGE,"Graphics_buffer_set_height.  "
//					"Graphics_bufffer type unknown or not supported.");
//				return_code = 0;
//			} break;
//		}
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,
//			"Graphics_buffer_set_height.  Invalid buffer");
//		return_code = 1;
//	}
//	LEAVE;

//	return (return_code);
//} /* Graphics_buffer_set_height */

//int Graphics_buffer_is_visible(struct Graphics_buffer *buffer)
///*******************************************************************************
//LAST MODIFIED : 1 July 2002

//DESCRIPTION :
//Returns 1 if the <buffer> is visible.  If the scene viewer gets zero from this
//routine it will not bother rendering into it, allowing us to avoid rendering
//into unmanaged or invisible widgets.
//==============================================================================*/
//{
//	int return_code;

//	ENTER(Graphics_buffer_is_visible);
//	if (buffer)
//	{
//		return_code = 0;
//		switch (buffer->type)
//		{
//			case GRAPHICS_BUFFER_WX_TYPE:
//			{
//				return_code = 1;
//			} break;
//			default:
//			{
//				display_message(ERROR_MESSAGE,"Graphics_buffer_is_visible.  "
//					"Graphics_bufffer type unknown or not supported.");
//				return_code = 0;
//			} break;
//		}
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,
//			"Graphics_buffer_is_visible.  Invalid buffer");
//		return_code=GRAPHICS_BUFFER_INVALID_TYPE;
//	}
//	LEAVE;

//	return (return_code);
//} /* Graphics_buffer_is_visible */

//int Graphics_buffer_awaken(struct Graphics_buffer *buffer)
///*******************************************************************************
//LAST MODIFIED : 1 July 2002

//DESCRIPTION :
//Activates the graphics <buffer>.
//==============================================================================*/
//{
//	int return_code = 0;

//	ENTER(Graphics_buffer_awaken);
//	if (buffer)
//	{
//		switch (buffer->type)
//		{
//			case GRAPHICS_BUFFER_WX_TYPE:
//			{
//				return_code = 1;
//			} break;
//			default:
//			{
//				display_message(ERROR_MESSAGE,"Graphics_buffer_awaken.  "
//					"Graphics_bufffer type unknown or not supported.");
//				return_code = 0;
//			} break;
//		}
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,
//			"Graphics_buffer_awaken.  Invalid buffer");
//		return_code=0;
//	}
//	LEAVE;

//	return (return_code);
//} /* Graphics_buffer_awaken */

//enum Graphics_buffer_type Graphics_buffer_get_type(struct Graphics_buffer *buffer)
///*******************************************************************************
//LAST MODIFIED : 27 May 2004

//DESCRIPTION :
//Returns information about the type of buffer that was created.
//==============================================================================*/
//{
//	enum Graphics_buffer_type buffer_type;

//	ENTER(Graphics_buffer_get_type);
//	if (buffer)
//	{
//		buffer_type = buffer->type;
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,
//			"Graphics_buffer_get_display.  Invalid buffer");
//		buffer_type = GRAPHICS_BUFFER_INVALID_TYPE;
//	}
//	LEAVE;

//	return (buffer_type);
//} /* Graphics_buffer_get_type */

//int Graphics_buffer_add_initialise_callback(struct Graphics_buffer *buffer,
//	CMISS_CALLBACK_FUNCTION(Graphics_buffer_callback) initialise_callback, void *user_data)
///*******************************************************************************
//LAST MODIFIED : 1 July 2002
//
//DESCRIPTION :
//Adds an initialise callback to the graphics <buffer>.
//==============================================================================*/
//{
//	int return_code;
//
//	ENTER(Graphics_buffer_awaken);
//	if (buffer)
//	{
//		return_code = 0;//-- CMISS_CALLBACK_LIST_ADD_CALLBACK(Graphics_buffer_callback)(
//			//-- buffer->initialise_callback_list, initialise_callback,
//			//-- user_data);
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,
//			"Graphics_buffer_add_initialise_callback.  Invalid buffer");
//		return_code=0;
//	}
//	LEAVE;
//
//	return (return_code);
//} /* Graphics_buffer_add_initialise_callback */

//int Graphics_buffer_add_resize_callback(struct Graphics_buffer *buffer,
//	CMISS_CALLBACK_FUNCTION(Graphics_buffer_callback) resize_callback, void *user_data)
///*******************************************************************************
//LAST MODIFIED : 1 July 2002
//
//DESCRIPTION :
//Adds an resize callback to the graphics <buffer>.
//==============================================================================*/
//{
//	int return_code;
//
//	ENTER(Graphics_buffer_awaken);
//	if (buffer)
//	{
//		return_code = CMISS_CALLBACK_LIST_ADD_CALLBACK(Graphics_buffer_callback)(
//			buffer->resize_callback_list, resize_callback,
//			user_data);
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,
//			"Graphics_buffer_add_resize_callback.  Invalid buffer");
//		return_code=0;
//	}
//	LEAVE;
//
//	return (return_code);
//} /* Graphics_buffer_add_resize_callback */

//int Graphics_buffer_add_expose_callback(struct Graphics_buffer *buffer,
//	CMISS_CALLBACK_FUNCTION(Graphics_buffer_callback) expose_callback, void *user_data)
///*******************************************************************************
//LAST MODIFIED : 1 July 2002
//
//DESCRIPTION :
//Adds an expose callback to the graphics <buffer>.
//==============================================================================*/
//{
//	int return_code;
//
//	ENTER(Graphics_buffer_awaken);
//	if (buffer)
//	{
//		return_code = CMISS_CALLBACK_LIST_ADD_CALLBACK(Graphics_buffer_callback)(
//			buffer->expose_callback_list, expose_callback,
//			user_data);
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,
//			"Graphics_buffer_add_expose_callback.  Invalid buffer");
//		return_code=0;
//	}
//	LEAVE;
//
//	return (return_code);
//} /* Graphics_buffer_add_expose_callback */

//int Graphics_buffer_add_input_callback(struct Graphics_buffer *buffer,
//	callback_function_Graphics_buffer_input_callback input_callback,
//	void *user_data)
///*******************************************************************************
//LAST MODIFIED : 1 July 2002
//
//DESCRIPTION :
//Adds an input callback to the graphics <buffer>.
//==============================================================================*/
//{
//	int return_code;
//
//	ENTER(Graphics_buffer_awaken);
//	if (buffer)
//	{
//		return_code = CMISS_CALLBACK_LIST_ADD_CALLBACK(Graphics_buffer_input_callback)(
//			buffer->input_callback_list, input_callback,
//			user_data);
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,
//			"Graphics_buffer_add_input_callback.  Invalid buffer");
//		return_code=0;
//	}
//	LEAVE;
//
//	return (return_code);
//} /* Graphics_buffer_add_input_callback */

//int DESTROY(Graphics_buffer_wx)(struct Graphics_buffer_wx **buffer_ptr)
///*******************************************************************************
//LAST MODIFIED : 1 July 2002

//DESCRIPTION :
//Closes a Graphics buffer instance
//===============================================================================*/
//{
//	int return_code;
//	struct Graphics_buffer_wx *buffer;

//	ENTER(DESTROY(Graphics_buffer));
//	if (buffer_ptr && (buffer = *buffer_ptr))
//	{
//		return_code=1;
//		//-- if (buffer->initialise_callback_list)
//		{
//			//-- DESTROY(LIST(CMISS_CALLBACK_ITEM(Graphics_buffer_callback)))(
//			//-- 	&buffer->initialise_callback_list);
//		}
//		//-- if (buffer->resize_callback_list)
//		{
//			//-- DESTROY(LIST(CMISS_CALLBACK_ITEM(Graphics_buffer_callback)))(
//			//-- 	&buffer->resize_callback_list);
//		}
//		//-- if (buffer->expose_callback_list)
//		{
//			//-- DESTROY(LIST(CMISS_CALLBACK_ITEM(Graphics_buffer_callback)))(
//			//-- 	&buffer->expose_callback_list);
//		}
//		//-- if (buffer->input_callback_list)
//		{
//			//-- DESTROY(LIST(CMISS_CALLBACK_ITEM(Graphics_buffer_input_callback)))(
//			//-- 	&buffer->input_callback_list);
//		}
//		/* Remove reference to this object in wxGraphicsBuffer */
//		if (buffer->canvas)
//		{
//			//-- buffer->canvas->ClearGraphicsBufferReference();
//			//-- delete buffer->canvas;
//		}
//		if (buffer->attrib_list != NULL)
//		{
//				DEALLOCATE(buffer->attrib_list);
//		}
//		if (buffer->type == GRAPHICS_BUFFER_GL_EXT_FRAMEBUFFER_TYPE)
//		 {
//#if defined (OPENGL_API) && defined (GL_EXT_framebuffer_object)
//			 GLint framebuffer_flag;
//			 if (Graphics_library_check_extension(GL_EXT_framebuffer_object))
//			 {
//				 glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &framebuffer_flag);
//				 if (framebuffer_flag != 0)
//				 {
//					 if (buffer->fbo != 0)
//					 {
//						 if (framebuffer_flag == (GLint)buffer->fbo)
//						 {
//							 glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
//						 }
//						 glDeleteFramebuffersEXT(1, &buffer->fbo);
//					 }
//					 if (buffer->depthbuffer != 0)
//					 {
//						 glDeleteRenderbuffersEXT(1, &buffer->depthbuffer);
//					 }
//#if defined (USE_MSAA)
//					 if (buffer->multi_fbo != 0)
//					 {
//						 glDeleteFramebuffersEXT(1, &buffer->multi_fbo);
//					 }
//					 if (buffer->multi_depthbuffer != 0)
//					 {
//						 glDeleteFramebuffersEXT(1, &buffer->multi_depthbuffer);
//					 }
//					 if (buffer->msbuffer != 0)
//					 {
//						 glDeleteRenderbuffersEXT(1, &buffer->msbuffer);
//					 }
//#endif
//					 if (buffer->img != 0)
//					 {
//						 glDeleteTextures(1, &buffer->img);
//					 }

//				 }
//			 }
//		 }
//#endif /* defined (OPENGL_API) && defined (GL_EXT_framebuffer_object) */
//		delete *buffer_ptr;//-- DEALLOCATE(*buffer_ptr);
//		*buffer_ptr = 0;
//	}
//	else
//	{
//		display_message(ERROR_MESSAGE,
//			"DESTROY(Graphics_buffer).  Missing buffer");
//		return_code=0;
//	}
//	LEAVE;

//	return (return_code);
//} /* DESTROY(Graphics_buffer) */
