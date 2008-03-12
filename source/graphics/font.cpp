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

extern "C" {
#include "general/debug.h"
#include "general/object.h"
#include "general/mystring.h"
#include "general/list_private.h"
#include "general/manager_private.h"
#include "graphics/font.h"
#include "graphics/graphics_library.h"
#include "three_d_drawing/graphics_buffer.h"
#include "user_interface/message.h"
#if defined (MOTIF)
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
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
}
#if defined (WX_USER_INTERFACE)
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/dcmemory.h>
#include <wx/font.h>
#endif /* defined (WX_USER_INTERFACE) */
/*
Module types
------------
*/

// DECLARE_LIST_TYPES(Graphics_font);

// DECLARE_MANAGER_TYPES(Graphics_font);

// PROTOTYPE_OBJECT_FUNCTIONS(Graphics_font);

// PROTOTYPE_LIST_FUNCTIONS(Graphics_font);
// PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Graphics_font,name,char *);

// PROTOTYPE_MANAGER_COPY_FUNCTIONS(Graphics_font,name,char *);
// PROTOTYPE_MANAGER_FUNCTIONS(Graphics_font);
// PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Graphics_font,name,char *);

struct Graphics_font_package
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
==============================================================================*/
{
	struct MANAGER(Graphics_font) *font_manager;
};

struct Graphics_font
/*******************************************************************************
LAST MODIFIED : 17 November 2005

DESCRIPTION :
==============================================================================*/
{
	char *name;
	char *font_string;

	int first_bitmap;
	int number_of_bitmaps;

	int display_list_offset;

	int offset_x, offset_y;

	 int access_count;

#if defined (WX_USER_INTERFACE)
	 wxFont *font_settings;
#endif /* defined (WX_USER_INTERFACE) */
};

FULL_DECLARE_LIST_TYPE(Graphics_font);

FULL_DECLARE_MANAGER_TYPE(Graphics_font);

/*
Module functions
----------------
*/

DECLARE_LOCAL_MANAGER_FUNCTIONS(Graphics_font)

DECLARE_LIST_FUNCTIONS(Graphics_font)
DECLARE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Graphics_font,name,char *,strcmp)
DECLARE_LIST_IDENTIFIER_CHANGE_FUNCTIONS(Graphics_font,name)

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(Graphics_font,name)
{
	char *name;
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(Graphics_font,name));
	/* check arguments */
	if (source&&destination)
	{
		if (source->name)
		{
			if (ALLOCATE(name,char,strlen(source->name)+1))
			{
				strcpy(name,source->name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Graphics_font,name).  Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			if (return_code = MANAGER_COPY_WITHOUT_IDENTIFIER(Graphics_font,name)(
				destination,source))
			{
				/* copy values */
				DEALLOCATE(destination->name);
				destination->name=name;
			}
			else
			{
				DEALLOCATE(name);
				display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Graphics_font,name).  Could not copy without identifier");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Graphics_font,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(Graphics_font,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Graphics_font,name)
{
	int return_code;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Graphics_font,name));
	if (source && destination)
	{
		destination->first_bitmap = 0;
		destination->number_of_bitmaps = 0;
		destination->display_list_offset = 0;

		destination->offset_x = 0;
		destination->offset_y = 0;

		if (destination->font_string)
		{
			DEALLOCATE(destination->font_string);
		}
		destination->font_string = duplicate_string
			(source->font_string);

		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITHOUT_IDENTIFIER(Graphics_font,name).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(Graphics_font,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Graphics_font,name,char *)
{
	char *destination_name;
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(Graphics_font,name));
	if (name&&destination)
	{
		if (ALLOCATE(destination_name,char,strlen(name)+1))
		{
			strcpy(destination_name,name);
			if (destination->name)
			{
				DEALLOCATE(destination->name);
			}
			destination->name=destination_name;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"MANAGER_COPY_IDENTIFIER(Graphics_font,name).  Insufficient memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Graphics_font,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(Graphics_font,name) */

DECLARE_MANAGER_FUNCTIONS(Graphics_font)

DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Graphics_font)

DECLARE_MANAGER_IDENTIFIER_FUNCTIONS(Graphics_font,name,char *)

struct Graphics_font *CREATE(Graphics_font)(char *name, char *font_string);
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
Creates a font called <name> with the user interface dependent <font_string>.
==============================================================================*/

struct Graphics_font_package *CREATE(Graphics_font_package)()
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
==============================================================================*/
{
	struct Graphics_font_package *font_package;

	ENTER(CREATE(Graphics_font_package));

	if (ALLOCATE(font_package, struct Graphics_font_package, 1))
	{
		font_package->font_manager = CREATE(MANAGER(Graphics_font))();
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Graphics_font_package). Unable to allocate font_package structure");
		font_package = (struct Graphics_font_package *)NULL;
	}

	LEAVE;
	return (font_package);
} /* CREATE(Graphics_font_package) */

int DESTROY(Graphics_font_package)(struct Graphics_font_package **font_package_address)
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Graphics_font_package *font_package;

	ENTER(DESTROY(Graphics_font_package));
	if (font_package_address && (font_package = *font_package_address))
	{
		DESTROY(MANAGER(Graphics_font))(&font_package->font_manager);

		DEALLOCATE(*font_package_address);
		*font_package_address = (struct Graphics_font_package *)NULL;

		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Graphics_font_package).  Missing package");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Graphics_font_package) */

struct MANAGER(Graphics_font)
	*Graphics_font_package_get_font_manager(
		struct Graphics_font_package *font_package)
/*******************************************************************************
LAST MODIFIED : 17 May 2007

DESCRIPTION :
Extracts the font_manager from the graphics_font_package. Note that
the rest of the program should use this sparingly - it is really only here to
allow interfacing to the choose_object widgets.
==============================================================================*/
{
	 struct MANAGER(Graphics_font) *font_manager

	ENTER(Graphics_font_package_get_font_manager);
	if (font_package)
	{
		font_manager=font_package->font_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_font_package_get_font_manager.  "
			"graphics_font_package");
		font_manager=(struct MANAGER(Graphics_font) *)NULL;
	}
	LEAVE;

	return (font_manager);
} /* Graphics_font_package_get_font_manager */

int Graphics_font_package_define_font(
	struct Graphics_font_package *font_package,
	char *font_name, char *font_string)
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
Defines an <alias_name> in the <font_package> which refers to the font 
<font_name>.
==============================================================================*/
{
	Graphics_font *existing_font, *font;
	int return_code;

	ENTER(Graphics_font_package_define_font);

	if (font_package && font_name && font_string)
	{
		existing_font = FIND_BY_IDENTIFIER_IN_MANAGER(Graphics_font,name)(font_name,
			font_package->font_manager);
		if (font = CREATE(Graphics_font)(font_name, font_string))
		{
			if (existing_font)
			{
				return_code = MANAGER_MODIFY_NOT_IDENTIFIER(Graphics_font,name)(existing_font,
					font, font_package->font_manager);
				DESTROY(Graphics_font)(&font);
			}
			else
			{
				return_code = ADD_OBJECT_TO_MANAGER(Graphics_font)(font, 
					font_package->font_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Graphics_font_package_define_font.  "
				"Could not create font.");
			return_code = 0;
		}
	}
	else
	{
 		display_message(ERROR_MESSAGE,
			"Graphics_font_package_define_font.  "
			"Invalid arguments");
		return_code = 0;
	}

	LEAVE;
	return (return_code);
} /* Graphics_font_package_define_font */

struct Graphics_font *Graphics_font_package_get_font(
	struct Graphics_font_package *font_package, char *font_name)
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
Finds a Graphics_font with name <font_name> in the <font_package>.
If it doesn't exist then a font is created using the <font_name>
as the user interface dependent font string. 
==============================================================================*/
{
	Graphics_font *font;

	ENTER(Graphics_font_package_get_font);

	font = (Graphics_font *)NULL;
	if (font_package && font_name)
	{
		if (!(font = FIND_BY_IDENTIFIER_IN_MANAGER(Graphics_font, name)(font_name,
			font_package->font_manager)))
		{
			if (font = CREATE(Graphics_font)(font_name, font_name))
			{
				ADD_OBJECT_TO_MANAGER(Graphics_font)(font, 
					font_package->font_manager);				
			}
		}
	}
	else
	{
 		display_message(ERROR_MESSAGE,
			"Graphics_font_package_define_font_alias.  "
			"Invalid arguments");
	}

	LEAVE;
	return (font);
} /* Graphics_font_package_define_font_alias */

DECLARE_OBJECT_FUNCTIONS(Graphics_font)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Graphics_font)

struct Graphics_font *CREATE(Graphics_font)(char *name, char *font_string)
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
==============================================================================*/
{
	Graphics_font *font;

	ENTER(CREATE(Graphics_font));

	if (ALLOCATE(font, struct Graphics_font, 1))
	{
		font->name = duplicate_string(name);
		font->font_string = duplicate_string(font_string);

		font->first_bitmap = 0;
		font->number_of_bitmaps = 0;

		font->display_list_offset = 0;

		font->offset_x = 0;
		font->offset_y = 0;

		font->access_count = 0;

#if defined (WX_USER_INTERFACE)
		font->font_settings = NULL;
		struct Parse_state *state;
		char *current_token;
		if (strcmp(font->font_string,"default"))
		{
			 if (state=create_Parse_state(font->font_string))
			 {
					int size = 8;
					
					enum wxFontFamily font_family = wxFONTFAMILY_DEFAULT;
					enum wxFontWeight font_weight = wxFONTWEIGHT_NORMAL;
					enum wxFontStyle font_style = wxFONTSTYLE_NORMAL;
					if (current_token = state->current_token)
					{
						 sscanf(current_token,"%u",&size);
						 if (shift_Parse_state(state,1)&&
						 (current_token=state->current_token))
						 {
								if (!strcmp(current_token, "roman"))
									 font_family=wxFONTFAMILY_ROMAN;
								else if (!strcmp(current_token, "decorative"))
									 font_family=wxFONTFAMILY_DECORATIVE;
								else if (!strcmp(current_token, "script"))
									 font_family=wxFONTFAMILY_SCRIPT;								
								else if (!strcmp(current_token, "swiss"))
									 font_family=wxFONTFAMILY_SWISS;
								else if (!strcmp(current_token, "modern"))
									 font_family=wxFONTFAMILY_MODERN;
								else if (!strcmp(current_token, "teletype"))
									 font_family=wxFONTFAMILY_TELETYPE;
								if (shift_Parse_state(state,1)&&
									 (current_token=state->current_token))
								{
									 if (!strcmp(current_token, "italic"))
											font_style = wxFONTSTYLE_ITALIC;
									 else if (!strcmp(current_token, "slant"))
											font_style = wxFONTSTYLE_SLANT;
									 if (shift_Parse_state(state,1)&&
											(current_token=state->current_token))
									 {
											if (!strcmp(current_token, "light"))
												 font_weight=wxFONTWEIGHT_LIGHT;
											else if (!strcmp(current_token, "bold"))
												 font_weight=wxFONTWEIGHT_BOLD;
									 }
								}
						 }
					}
					font->font_settings = new wxFont(size, font_family, 
						 font_style, font_weight);
					destroy_Parse_state(&state);
			 }
			 else
			 {
					display_message(ERROR_MESSAGE,"CREATE(Graphics_font). Cannot parse the statement. "
						 "Default font settings will be used instead. \n"
						 "Use the command gfx define font ? for more information.\n");
			 }
		}
#endif /* (WX_USER_INTERFACE) */
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
		if (font->font_string)
		{
			DEALLOCATE(font->font_string);
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
LAST MODIFIED : 12 March 2008

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
	XFontStruct *x_font;
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
	HFONT win32_font;
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (GTK_USER_INTERFACE)
#if GTK_MAJOR_VERSION >= 2
	gint font_height;
	PangoFontDescription *font_desc;
	PangoFont *pango_font;
	PangoFontMetrics *font_metrics;
#endif /* GTK_MAJOR_VERSION >= 2 */
#endif /* defined (GTK_USER_INTERFACE) */
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
					if (!strcmp(font->font_string,"default"))
					{
						x_font = XLoadQueryFont(Graphics_buffer_X11_get_display(buffer), 
							"-adobe-helvetica-medium-r-normal-*-12-*-*-*-*-*-*-*");
					}
					else
					{
						if (!(x_font = XLoadQueryFont(Graphics_buffer_X11_get_display(buffer),
							font->font_string)))
						{
							display_message(WARNING_MESSAGE,
								"Unable to get specified font \"%s\", falling back to system font.",
								font->font_string);
							x_font = XLoadQueryFont(Graphics_buffer_X11_get_display(buffer), 
								"-adobe-helvetica-medium-r-normal-*-12-*-*-*-*-*-*-*");
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
#  if defined GTK_USE_GTKGLAREA
				case GRAPHICS_BUFFER_GTKGLAREA_TYPE:
				{
		         /* Not implemented */
					display_message(WARNING_MESSAGE,"wrapperInitText.  "
						"Text display is not implemented for Gtk prior to version 2 or for gtkglarea.");
					return_code = 0;
				} break;
#  else /* ! defined GTK_USE_GTKGLAREA */
				case GRAPHICS_BUFFER_GTKGLEXT_TYPE:
				{
					if (!strcmp(font->font_string,"default"))
					{
						font_desc = pango_font_description_from_string ("courier 12");
					}
					else
					{
						font_desc = pango_font_description_from_string (font->font_string);
					}

					pango_font = gdk_gl_font_use_pango_font (font_desc, font->first_bitmap, 
						font->number_of_bitmaps, font->display_list_offset);
					if (pango_font == NULL)
					{
						display_message(WARNING_MESSAGE,"wrapperInitText.  "
							"Text display is not implemented for Gtk prior to version 2.");
					}
					
					font_metrics = pango_font_get_metrics (pango_font, NULL);
					
					font_height = pango_font_metrics_get_ascent (font_metrics) +
						pango_font_metrics_get_descent (font_metrics);
					font_height = PANGO_PIXELS (font_height);
					
					pango_font_description_free (font_desc);
					pango_font_metrics_unref (font_metrics);
				} break;
#  endif /* ! defined GTK_USE_GTKGLAREA */
#endif /* defined (GTK_USER_INTERFACE) */
#if defined (WIN32_USER_INTERFACE)
				case GRAPHICS_BUFFER_WIN32_TYPE:
				{
					if (!strcmp(font->font_string,"default"))
					{
						win32_font = static_cast<HFONT__*>(GetStockObject(DEFAULT_GUI_FONT));
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
									font->font_string   /* typeface name */
									)))
						{
							display_message(WARNING_MESSAGE,
								"Unable to get specified font \"%s\", falling back to system font.",
								font->font_string);
							win32_font = static_cast<HFONT__*>(GetStockObject(DEFAULT_GUI_FONT));
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
#if defined (WX_USER_INTERFACE)
				case GRAPHICS_BUFFER_WX_TYPE:
				{
					int i;
					int bitmap_size = 2;

					glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);

					glPixelStorei (GL_UNPACK_SWAP_BYTES, GL_FALSE);
					glPixelStorei (GL_UNPACK_LSB_FIRST, GL_FALSE);
					glPixelStorei (GL_UNPACK_ROW_LENGTH, 0);
					glPixelStorei (GL_UNPACK_SKIP_ROWS, 0);
					glPixelStorei (GL_UNPACK_SKIP_PIXELS, 0);
					glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

					wxBitmap *bitmap = new wxBitmap(bitmap_size, bitmap_size, /*depth*/-1);
					if (!bitmap->Ok())
					{
						display_message(WARNING_MESSAGE,
							"Graphics_font_compile.  "
							"Error making bitmap for wx font.");
						return_code = 0;
					}

					wxMemoryDC dc;
					dc.SelectObject(*bitmap);
					if (!dc.Ok())
					{
						display_message(WARNING_MESSAGE,
							"Graphics_font_compile.  "
							"Error making memoryDC for wx font.");
						return_code = 0;
					}
					dc.SetBackground(*wxBLACK_BRUSH);
					dc.SetTextForeground(*wxWHITE);
					dc.SetBrush(*wxWHITE_BRUSH);
					dc.SetPen(*wxWHITE_PEN);

					for (i = 0 ; return_code && (i < font->number_of_bitmaps) ; i++)
					{
						wxChar string[2];

						dc.Clear();

						string[0] = i;
						string[1] = 0;
						wxString wxstring(string);

						wxCoord char_width, char_height, char_descent, char_leading;
						dc.GetTextExtent(wxstring, &char_width, &char_height,
							&char_descent, &char_leading);

						if ((char_width > bitmap_size) || (char_height > bitmap_size))
						{
							while ((char_width > bitmap_size) || (char_height > bitmap_size))
							{
								bitmap_size *= 2;
							}
							dc.SelectObject(wxNullBitmap);
							delete bitmap;
							bitmap = new wxBitmap(bitmap_size, bitmap_size, /*depth*/-1);
							if (!bitmap->Ok())
							{
								display_message(WARNING_MESSAGE,
									"Graphics_font_compile.  "
									"Error making bitmap for wx font.");
								return_code = 0;
							}
							dc.SelectObject(*bitmap);
							if (!dc.Ok())
							{
								display_message(WARNING_MESSAGE,
									"Graphics_font_compile.  "
									"Error making memoryDC for wx font.");
								return_code = 0;
							}							
							dc.SetBackground(*wxBLACK_BRUSH);
							dc.SetTextForeground(*wxWHITE);
							dc.SetBrush(*wxWHITE_BRUSH);
							dc.SetPen(*wxWHITE_PEN);
						}
						if (!strcmp(font->font_string,"default"))
						{
							 dc.SetFont(*wxNORMAL_FONT);
						}
						else if (font->font_settings)
						{
							 dc.SetFont(*(font->font_settings)); 
						}
						dc.DrawText(wxstring, 0, 0);

						wxImage image = bitmap->ConvertToImage();

						glNewList (font->display_list_offset + i, GL_COMPILE);

						unsigned char *image_data = image.GetData();
						int xbitmap_width = (char_width + 7) / 8;
						unsigned char *xbitmap_data = new unsigned char[
							char_height*xbitmap_width];
						unsigned char *data_ptr = image_data;
						unsigned char *xbm_ptr = xbitmap_data;
						int j, k;

						for (j = 0 ; j < char_height ; j++)
						{
							xbm_ptr = xbitmap_data + xbitmap_width * j;
							data_ptr = image_data + (char_height - 1 - j) * bitmap_size * 3;
							for (k = 0 ; k < xbitmap_width ; k++)
							{
								*xbm_ptr = 
									((data_ptr[0] & 0x80) ? 0x80 : 0) + 
									((data_ptr[3] & 0x80) ? 0x40 : 0) + 
									((data_ptr[6] & 0x80) ? 0x20 : 0) + 
									((data_ptr[9] & 0x80) ? 0x10 : 0) + 
									((data_ptr[12] & 0x80) ? 0x08 : 0) + 
									((data_ptr[15] & 0x80) ? 0x04 : 0) + 
									((data_ptr[18] & 0x80) ? 0x02 : 0) + 
									((data_ptr[21] & 0x80) ? 0x01 : 0);
								xbm_ptr++;
								data_ptr+=8*3;
							}
						}
						glBitmap (char_width, char_height, 0, 0,
							char_leading + char_width, 0, xbitmap_data);

						delete xbitmap_data;

						glEndList ();
					}

					delete bitmap;

					glPopClientAttrib();

				} break;
#endif /* defined (WX_USER_INTERFACE) */
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
	
