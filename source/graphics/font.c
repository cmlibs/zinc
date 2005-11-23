/*******************************************************************************
FILE : graphics_font.c

LAST MODIFIED : 17 November 2005

DESCRIPTION :
This provides a Cmgui interface to the font contexts of many types.
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

#if defined (OPENGL_API)
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#if defined (WIN32_SYSTEM)
#include <GL/glext.h>
/* SAB On Win32 I think you have to load all OpenGL 1.2, 1.3 etc functions
	as extensions and keep pointer references to them.  I haven't done this
	yet so we will undefine the version symbols */
#undef GL_NV_vertex_program
#undef GL_NV_register_combiners2
#endif /* defined (WIN32_SYSTEM) */
#endif
#if defined (MOTIF)
#if defined (SGI)
/* Not compiling in as not being actively used and only available on O2's and
   cannot compile against Mesa without function pointer tables. */
/* #include <dmedia/dm_buffer.h> */
#endif /* defined (SGI) */
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include "three_d_drawing/ThreeDDraw.h"
#endif /* defined (MOTIF) */
#if defined (GTK_USER_INTERFACE)
#include <gtk/gtk.h>
#if ( GTK_MAJOR_VERSION < 2 ) || defined (WIN32_SYSTEM)
#define GTK_USE_GTKGLAREA
#endif /* ( GTK_MAJOR_VERSION < 2 ) || defined (WIN32_SYSTEM)*/
#if defined (GTK_USE_GTKGLAREA)
#include <gtkgl/gtkglarea.h>
#else /* defined (GTK_USE_GTKGLAREA) */
#include <gtk/gtkgl.h>
#endif /* defined (GTK_USE_GTKGLAREA) */
#endif /* defined (GTK_USER_INTERFACE) */
#if defined (WIN32_USER_INTERFACE)
#include <windows.h>
#endif /* defined (WIN32_USER_INTERFACE) */
#include "general/debug.h"
#include "general/object.h"
#include "general/mystring.h"
#include "graphics/font.h"
#include "three_d_drawing/graphics_buffer.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

struct Graphics_font
/*******************************************************************************
LAST MODIFIED : 17 November 2005

DESCRIPTION :
==============================================================================*/
{
	char *name;

	int first_bitmap;
	int number_of_bitmaps;

	int display_list_offset;

	int offset_x, offset_y;

	int access_count;
};

/*
Module functions
----------------
*/

DECLARE_OBJECT_FUNCTIONS(Graphics_font)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Graphics_font)

struct Graphics_font *CREATE(Graphics_font)(char *name)
/*******************************************************************************
LAST MODIFIED : 17 November 2005

DESCRIPTION :
==============================================================================*/
{
	struct Graphics_font *font;

	ENTER(CREATE(Graphics_font));

	if (ALLOCATE(font, struct Graphics_font, 1))
	{
		font->name = duplicate_string(name);
		font->first_bitmap = 0;
		font->number_of_bitmaps = 0;

		font->display_list_offset = 0;

		font->offset_x = 0;
		font->offset_y = 0;

		font->access_count = 0;

	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Graphics_font). Unable to allocate font structure");
		font = (struct Graphics_font *)NULL;
	}

	LEAVE;
	return (font);
} /* CREATE(Graphics_font) */

int DESTROY(Graphics_font)(struct Graphics_font **font_address)
/*******************************************************************************
LAST MODIFIED : 17 November 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Graphics_font *font;

	ENTER(DESTROY(Graphics_font));
	if (font_address && (font = *font_address))
	{
		if (font->name)
		{
			DEALLOCATE(font->name);
		}
		if (font->display_list_offset)
		{
			glDeleteLists(font->display_list_offset, font->number_of_bitmaps);
		}

		DEALLOCATE(*font_address);
		*font_address = (struct Graphics_font *)NULL;

		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Graphics_font).  Missing package");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Graphics_font) */

int Graphics_font_compile(struct Graphics_font *font,
	struct Graphics_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 17 November 2005

DESCRIPTION :
Compiles the specified <font> so it can be used by the graphics.  The 
<buffer> is required so that we can determine what API is used by this buffer.
==============================================================================*/
{
#if defined (MOTIF)
#define XmNgraphicsFont "graphicsFont"
#define XmCGraphicsFont "GraphicsFont"
#define XmNgraphicsTextOffsetX "graphicsTextOffsetX"
#define XmCGraphicsTextOffsetX "GraphicsTextOffsetX"
#define XmNgraphicsTextOffsetY "graphicsTextOffsetY"
#define XmCGraphicsTextOffsetY "GraphicsTextOffsetY"
	struct Text_defaults
	{
#if defined (MOTIF)
		XFontStruct *graphics_font;
#endif /* defined (MOTIF) */
		int offsetX, offsetY;
	} text_defaults;
	XtResource resources[]=
	{
		{
			XmNgraphicsFont,
			XmCGraphicsFont,
			XmRFontStruct,
			sizeof(XFontStruct *),
			XtOffsetOf(struct Text_defaults,graphics_font),
			XmRString,
			"-adobe-helvetica-medium-r-normal-*-12-*-*-*-*-*-*-*"
		},
		{
			XmNgraphicsTextOffsetX,
			XmCGraphicsTextOffsetX,
			XmRInt,
			sizeof(int),
			XtOffsetOf(struct Text_defaults,offsetX),
			XmRString,
			"0"
		},
		{
			XmNgraphicsTextOffsetY,
			XmCGraphicsTextOffsetY,
			XmRInt,
			sizeof(int),
			XtOffsetOf(struct Text_defaults,offsetY),
			XmRString,
			"0"
		}
	};
	Widget widget, application_shell;
	XFontStruct *x_font;
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
	HFONT win32_font;
#endif /* defined (WIN32_USER_INTERFACE) */
	int return_code;

	ENTER(Graphics_font_compile);
	if (font && buffer)
	{
		return_code = 1;
		if (!font->display_list_offset)
		{
			font->first_bitmap = 0;
			font->number_of_bitmaps = 256;
			font->display_list_offset = glGenLists (font->number_of_bitmaps);

			/* Can have multiple types compiled in at the same time (X and gtk) */
			switch (Graphics_buffer_get_type(buffer))
			{
#if defined (MOTIF)
				case GRAPHICS_BUFFER_GLX_X3D_TYPE:
#  if defined (USE_GLX_PBUFFER) || defined (GLX_SGIX_dmbuffer) || defined (GLX_SGIX_pbuffer)
				case GRAPHICS_BUFFER_GLX_PBUFFER_TYPE:
#  endif /* defined (USE_GLX_PBUFFER) || defined (GLX_SGIX_dmbuffer) || defined (GLX_SGIX_pbuffer) */
				case GRAPHICS_BUFFER_GLX_PIXMAP_TYPE:
				{
					widget = Graphics_buffer_X3d_get_widget(buffer);
					application_shell = widget;
					while (widget = XtParent(widget))
					{
						application_shell = widget;
					}
					XtVaGetApplicationResources(application_shell,
						&text_defaults, resources, XtNumber(resources), NULL);
					if (!strcmp(font->name,"default"))
					{
						x_font = text_defaults.graphics_font;
					}
					else
					{
						if (!(x_font = XLoadQueryFont(XtDisplay(application_shell), font->name)))
						{
							display_message(WARNING_MESSAGE,
								"Unable to get specified font \"%s\", falling back to system font.",
								font->name);
							x_font = text_defaults.graphics_font;
						}
					}
					if (x_font)
					{
						glXUseXFont(x_font->fid, font->first_bitmap,
							font->number_of_bitmaps, font->display_list_offset);
						return_code = 1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"Graphics_font.  "
							"Unable to get any font.");
						return_code = 0;
					}
				} break;
#endif /* defined (MOTIF) */
#if defined (GTK_USER_INTERFACE)
#if defined (GTK_USE_GTKGLAREA)
				case GRAPHICS_BUFFER_GTKGLAREA_TYPE:
				{
					return_code = 0;
				} break;
#else /* defined (GTK_USE_GTKGLAREA) */
				case GRAPHICS_BUFFER_GTKGLEXT_TYPE:
				{
					return_code = 0;
				} break;
#endif /* defined (GTK_USE_GTKGLAREA) */
#endif /* defined (GTK_USER_INTERFACE) */
#if defined (WIN32_USER_INTERFACE)
				case GRAPHICS_BUFFER_WIN32_TYPE:
				{
					if (!strcmp(font->name,"default"))
					{
						win32_font = GetStockObject(DEFAULT_GUI_FONT);
					}
					else
					{
						if (!(win32_font = CreateFont(
									20,               /* height of font */
									0,                /* average character width */
									0,                /* angle of escapement */
									0,                /* base-line orientation angle */
									FW_NORMAL,        /* font weight */
									FALSE,            /* italic attribute option */
									FALSE,            /* underline attribute option */
									FALSE,            /* strikeout attribute option */
									ANSI_CHARSET,     /* character set identifier */
									OUT_DEFAULT_PRECIS, /* output precision */
									CLIP_DEFAULT_PRECIS,  /* clipping precision */
									DEFAULT_QUALITY,  /* output quality */
									DEFAULT_PITCH | FF_DONTCARE, /* pitch and family */
									font->name   /* typeface name */
									)))
						{
							display_message(WARNING_MESSAGE,
								"Unable to get specified font \"%s\", falling back to system font.",
								font->name);
							win32_font = GetStockObject(DEFAULT_GUI_FONT);
						}
					}
					if (win32_font)
					{
						Graphics_buffer_win32_use_font_bitmaps(buffer,
							win32_font, font->first_bitmap, font->number_of_bitmaps, 
							font->display_list_offset);
						return_code = 1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"Graphics_font.  "
							"Unable to get any font.");
						return_code = 0;
					}
				} break;
#endif /* defined (WIN32_USER_INTERFACE) */
				default:
				{
					display_message(ERROR_MESSAGE,"Graphics_font.  "
						"Graphics_bufffer type unknown or not supported.");				
					return_code = 0;
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_font_compile.  "
			"Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_font_compile */

int Graphics_font_rendergl_text(struct Graphics_font *font, char *text)
/*******************************************************************************
LAST MODIFIED : 17 November 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_font_rendergl_text);

	if (font && text)
	{
		if (font->display_list_offset != -1)
		{
			glBitmap(0, 0, 0, 0, font->offset_x, font->offset_y, NULL);
			
			/* set the list base (i.e. the number that is added to each and every list
				call made from now on) */
			glListBase(font->display_list_offset);

			/* call a vector of lists, consisting of unsigned bytes (chars in C).  (Each
				char in the string therefore invokes a list call that draws the character
				that it represents to screen, and updates the current Raster Position state
				variable in OpenGL to advance the "print cursor").  */
			glCallLists(strlen(text), GL_UNSIGNED_BYTE, (GLubyte *)text);
			
			return_code = 1;
		}
		else
		{
		display_message(ERROR_MESSAGE,
			"Graphics_font_rendergl_text.  "
			"Font is being used to render text before being compiled.");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_font_rendergl_text.  "
			"Invalid arguments");
		return_code = 0;
	}

	LEAVE;
	return (return_code);
} /* Graphics_font_rendergl_text */
	
