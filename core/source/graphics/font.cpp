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

#include <string.h>
#include "api/cmiss_zinc_configure.h"
#include "api/cmiss_graphics_font.h"
#include "general/debug.h"
#include "general/enumerator_conversion.hpp"
#include "general/enumerator_private.hpp"
#include "general/object.h"
#include "general/mystring.h"
#include "general/list_private.h"
#include "general/manager_private.h"
#include "graphics/font.h"
#include "graphics/graphics_library.h"
#include "three_d_drawing/graphics_buffer.h"
#include "general/message.h"
#include <FTGL/ftgl.h>
#include <ttf/font_types.h>

/*
Module types
------------
*/

struct Cmiss_graphics_font
/*******************************************************************************
LAST MODIFIED : 17 November 2005

DESCRIPTION :
==============================================================================*/
{
	char *name;

	int offset_x, offset_y;
	int size, italic, bold, changed;
	double depth;
	Cmiss_graphics_font_true_type true_type;
	Cmiss_graphics_font_type font_type;
	/* after clearing in create, following to be modified only by manager */
	struct MANAGER(Cmiss_graphics_font) *manager;
	int manager_change_status;
	int access_count;

	FTFont *ftFont;
};

FULL_DECLARE_LIST_TYPE(Cmiss_graphics_font);

FULL_DECLARE_MANAGER_TYPE_WITH_OWNER(Cmiss_graphics_font, Cmiss_graphics_module, void *);

/*
Module functions
----------------
*/

DECLARE_LOCAL_MANAGER_FUNCTIONS(Cmiss_graphics_font)

DECLARE_LIST_FUNCTIONS(Cmiss_graphics_font)
DECLARE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Cmiss_graphics_font,name,const char *,strcmp)
DECLARE_LIST_IDENTIFIER_CHANGE_FUNCTIONS(Cmiss_graphics_font,name)

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(Cmiss_graphics_font,name)
{
	char *name;
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(Cmiss_graphics_font,name));
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
"MANAGER_COPY_WITH_IDENTIFIER(Cmiss_graphics_font,name).  Insufficient memory");
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
			return_code = MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_graphics_font,name)(
				destination,source);
			if (return_code)
			{
				/* copy values */
				DEALLOCATE(destination->name);
				destination->name=name;
			}
			else
			{
				DEALLOCATE(name);
				display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Cmiss_graphics_font,name).  Could not copy without identifier");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Cmiss_graphics_font,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(Cmiss_graphics_font,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Cmiss_graphics_font,name)
{
	int return_code;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_graphics_font,name));
	if (source && destination)
	{
		destination->offset_x = 0;
		destination->offset_y = 0;
		destination->size = source->size;
		destination->depth = source->depth;
		destination->italic = source->italic;
		destination->bold = source->bold;
		destination->true_type = source->true_type;
		destination->font_type = source->font_type;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_graphics_font,name).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_graphics_font,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Cmiss_graphics_font,name,const char *)
{
	char *destination_name;
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(Cmiss_graphics_font,name));
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
				"MANAGER_COPY_IDENTIFIER(Cmiss_graphics_font,name).  Insufficient memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Cmiss_graphics_font,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(Cmiss_graphics_font,name) */

DECLARE_MANAGER_FUNCTIONS(Cmiss_graphics_font,manager)

DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Cmiss_graphics_font,manager)

DECLARE_MANAGER_IDENTIFIER_FUNCTIONS(Cmiss_graphics_font,name,const char *,manager)

DECLARE_MANAGER_OWNER_FUNCTIONS(Cmiss_graphics_font, struct Cmiss_graphics_module)

int Cmiss_graphics_font_manager_set_owner(struct MANAGER(Cmiss_graphics_font) *manager,
	struct Cmiss_graphics_module *graphics_module)
{
	return MANAGER_SET_OWNER(Cmiss_graphics_font)(manager, graphics_module);
}

struct Cmiss_graphics_font *CREATE(Cmiss_graphics_font)(const char *name);
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
Creates a font called <name> with the user interface dependent <font_string>.
==============================================================================*/

DECLARE_OBJECT_FUNCTIONS(Cmiss_graphics_font)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Cmiss_graphics_font)

struct Cmiss_graphics_font *CREATE(Cmiss_graphics_font)(const char *name)
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
==============================================================================*/
{
	Cmiss_graphics_font *font;

	ENTER(CREATE(Cmiss_graphics_font));

	if (ALLOCATE(font, struct Cmiss_graphics_font, 1))
	{
		font->name = duplicate_string(name);

		font->offset_x = 0;
		font->offset_y = 0;

		font->size = 15;
		font->italic = 0;
		font->bold = 0;
		font->depth = 0.1;
		font->true_type = CMISS_GRAPHICS_FONT_TRUE_TYPE_OpenSans;
		font->font_type = CMISS_GRAPHICS_FONT_TYPE_PIXMAP;

		font->manager = (struct MANAGER(Cmiss_graphics_font) *)NULL;
		font->manager_change_status = MANAGER_CHANGE_NONE(Cmiss_graphics_font);

		font->ftFont = 0;
		font->changed = 1;
		font->access_count = 1;

	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Cmiss_graphics_font). Unable to allocate font structure");
		font = (struct Cmiss_graphics_font *)NULL;
	}

	LEAVE;
	return (font);
} /* CREATE(Cmiss_graphics_font) */

int DESTROY(Cmiss_graphics_font)(struct Cmiss_graphics_font **font_address)
/*******************************************************************************
LAST MODIFIED : 17 November 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Cmiss_graphics_font *font;

	ENTER(DESTROY(Cmiss_graphics_font));
	if (font_address && (font = *font_address))
	{
		if (font->name)
		{
			DEALLOCATE(font->name);
		}
		if (font->ftFont)
		{
			delete font->ftFont;
		}

		DEALLOCATE(*font_address);
		*font_address = (struct Cmiss_graphics_font *)NULL;

		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Cmiss_graphics_font).  Missing package");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Cmiss_graphics_font) */

unsigned int Cmiss_graphics_font_get_font_buffer(struct Cmiss_graphics_font *font,
	unsigned char **true_type_buffer_out)
{
	unsigned char *true_type_buffer = 0;
	unsigned int true_type_length = 0;
	switch (font->true_type)
	{
		case CMISS_GRAPHICS_FONT_TRUE_TYPE_OpenSans:
		{
			if (font->bold)
			{
				if (font->italic)
				{
					true_type_buffer = OpenSans_BoldItalic_ttf;
					true_type_length = OpenSans_BoldItalic_ttf_len;
				}
				else
				{
					true_type_buffer = OpenSans_Bold_ttf;
					true_type_length = OpenSans_Bold_ttf_len;
				}
			}
			else if (font->italic)
			{
				true_type_buffer = OpenSans_Italic_ttf;
				true_type_length = OpenSans_Italic_ttf_len;
			}
			else
			{
				true_type_buffer = OpenSans_Regular_ttf;
				true_type_length = OpenSans_Regular_ttf_len;
			}
		} break;
		default :
		{
		}break;
	}
	*true_type_buffer_out = true_type_buffer;
	return true_type_length;
}

int Cmiss_graphics_font_compile(struct Cmiss_graphics_font *font,
	struct Graphics_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 12 March 2008

DESCRIPTION :
Compiles the specified <font> so it can be used by the graphics.  The
<buffer> is required so that we can determine what API is used by this buffer.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_graphics_font_compile);
	if (font && buffer)
	{
		return_code = 1;

		/* Can have multiple types compiled in at the same time (X and gtk) */
		switch (Graphics_buffer_get_type(buffer))
		{
			case GRAPHICS_BUFFER_ONSCREEN_TYPE:
			{
				if (font->ftFont && font->changed == 1)
				{
					delete font->ftFont;
					font->ftFont = 0;
				}
				if (font->ftFont == 0)
				{
					unsigned char *true_type_buffer = 0;
					unsigned int true_type_length = 0;
					true_type_length = Cmiss_graphics_font_get_font_buffer(font,
						&true_type_buffer);
					if ((true_type_buffer != 0) && (true_type_length > 0))
					{
						switch (font->font_type)
						{
							case CMISS_GRAPHICS_FONT_TYPE_BITMAP:
							case CMISS_GRAPHICS_FONT_TYPE_PIXMAP:
							{
								if (font->font_type == CMISS_GRAPHICS_FONT_TYPE_BITMAP)
									font->ftFont = new FTBitmapFont(true_type_buffer, true_type_length);
								else
									font->ftFont = new FTPixmapFont(true_type_buffer, true_type_length);
								if(font->ftFont->Error())
								{
									return_code = 0;
								}
								else
								{
									font->ftFont->FaceSize(font->size);
									font->ftFont->UseDisplayList(false);
								}
							} break;
							case CMISS_GRAPHICS_FONT_TYPE_POLYGON:
							case CMISS_GRAPHICS_FONT_TYPE_OUTLINE:
							case CMISS_GRAPHICS_FONT_TYPE_EXTRUDE:
							{
								if (font->font_type == CMISS_GRAPHICS_FONT_TYPE_POLYGON)
									font->ftFont = new FTPolygonFont(true_type_buffer, true_type_length);
								else if (font->font_type == CMISS_GRAPHICS_FONT_TYPE_OUTLINE)
									font->ftFont = new FTOutlineFont(true_type_buffer, true_type_length);
								else
									font->ftFont = new FTExtrudeFont(true_type_buffer, true_type_length);
								if(font->ftFont->Error())
								{
									return_code = 0;
								}
								else
								{
									font->ftFont->FaceSize(font->size,144);
									font->ftFont->Depth((float)font->depth);
									font->ftFont->UseDisplayList(false);
								}
							} break;
							default :
							{
								return_code = 0;
							}break;
						}
					}
					{
						return_code = 0;
					}
				}
			}break;
			default:
			{
				display_message(ERROR_MESSAGE,"Cmiss_graphics_font.  "
					"Graphics_bufffer type unknown or not supported.");
				return_code = 0;
			} break;
		}
		font->changed = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_graphics_font_compile.  "
			"Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphics_font_compile */

int Cmiss_graphics_font_rendergl_text(struct Cmiss_graphics_font *font, char *text,
	float x, float y, float z)
/*******************************************************************************
LAST MODIFIED : 17 November 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code = 0;

	ENTER(Cmiss_graphics_font_rendergl_text);

	if (font && text)
	{
		if (font->ftFont)
		{
			switch (font->font_type)
			{
				case CMISS_GRAPHICS_FONT_TYPE_BITMAP:
				case CMISS_GRAPHICS_FONT_TYPE_PIXMAP:
				{
					glRasterPos3f(x, y, z);
					font->ftFont->Render(text);
				} break;
				case CMISS_GRAPHICS_FONT_TYPE_POLYGON:
				case CMISS_GRAPHICS_FONT_TYPE_OUTLINE:
				case CMISS_GRAPHICS_FONT_TYPE_EXTRUDE:
				{
					glMatrixMode(GL_MODELVIEW);
					glPushMatrix();
					font->ftFont->Render(text, -1, FTPoint(x, y, z));
					glPopMatrix();
				} break;
				default :
				{
				}break;
			}
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_graphics_font_rendergl_text.  "
				"Font is being used to render text before being compiled.");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphics_font_rendergl_text.  "
			"Invalid arguments");
	}

	LEAVE;
	return (return_code);
} /* Cmiss_graphics_font_rendergl_text */

int Cmiss_graphics_font_set_name(
	Cmiss_graphics_font_id font, const char *name)
{
	int return_code = 0;

	if (font && font->manager && name)
	{
		return_code = MANAGER_MODIFY_IDENTIFIER(Cmiss_graphics_font, name)(
			font, name, font->manager);
	}

	return return_code;
}

char *Cmiss_graphics_font_get_name(Cmiss_graphics_font_id font)
{
	char *name = NULL;
	if (font)
	{
		name = duplicate_string(font->name);
	}

	return name;
}

/***************************************************************************//**
 * Broadcast changes in the graphis font to be propagated to objects that
 * uses it through manager that owns it.
 *
 * @param font  Modified Cmiss_graphics_font to be broadcast.
 * @return 1 on success, 0 on failure
 */
int Cmiss_graphics_font_changed(Cmiss_graphics_font_id font)
{
	if (font)
	{
		font->changed = 1;
		return MANAGED_OBJECT_CHANGE(Cmiss_graphics_font)(font,
			MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Cmiss_graphics_font));
	}
	else
	{
		return 0;
	}
}


Cmiss_graphics_font_true_type Cmiss_graphics_font_get_true_type(
	Cmiss_graphics_font_id font)
{
	if (font)
	{
		return font->true_type;
	}
	else
	{
		return CMISS_GRAPHICS_FONT_TRUE_TYPE_INVALID;
	}
}

int Cmiss_graphics_font_set_true_type(Cmiss_graphics_font_id font,
	Cmiss_graphics_font_true_type true_type)
{
	if (font)
	{
		if (font->true_type != true_type)
		{
			font->true_type = true_type;
			Cmiss_graphics_font_changed(font);
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

Cmiss_graphics_font_type Cmiss_graphics_font_get_type(
	Cmiss_graphics_font_id font)
{
	if (font)
	{
		return font->font_type;
	}
	else
	{
		return CMISS_GRAPHICS_FONT_TYPE_INVALID;
	}
}

int Cmiss_graphics_font_set_type(Cmiss_graphics_font_id font,
	Cmiss_graphics_font_type font_type)
{
	if (font)
	{
		if (font->font_type != font_type)
		{
			font->font_type = font_type;
			Cmiss_graphics_font_changed(font);
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

int Cmiss_graphics_font_get_bold(Cmiss_graphics_font_id font)
{
	if (font)
	{
		return font->bold;
	}
	else
	{
		return 0;
	}
}

int Cmiss_graphics_font_set_bold(Cmiss_graphics_font_id font, int bold)
{
	if (font)
	{
		if (font->bold != bold)
		{
			font->bold = bold;
			Cmiss_graphics_font_changed(font);
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

double Cmiss_graphics_font_get_depth(Cmiss_graphics_font_id font)
{
	if (font)
	{
		return font->depth;
	}
	else
	{
		return 0;
	}
}

int Cmiss_graphics_font_set_depth(Cmiss_graphics_font_id font, double depth)
{
	if (font)
	{
		if (font->depth != depth)
		{
			font->depth = depth;
			if (font->font_type == CMISS_GRAPHICS_FONT_TYPE_EXTRUDE)
				Cmiss_graphics_font_changed(font);
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

int Cmiss_graphics_font_get_italic(Cmiss_graphics_font_id font)
{
	if (font)
	{
		return font->italic;
	}
	else
	{
		return 0;
	}
}

int Cmiss_graphics_font_set_italic(Cmiss_graphics_font_id font, int italic)
{
	if (font)
	{
		if (font->italic != italic)
		{
			font->italic = italic;
			Cmiss_graphics_font_changed(font);
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

int Cmiss_graphics_font_get_size(Cmiss_graphics_font_id font)
{
	if (font)
	{
		return font->size;
	}
	else
	{
		return 0;
	}
}

int Cmiss_graphics_font_set_size(Cmiss_graphics_font_id font, int size)
{
	if (font)
	{
		if (font->size != size)
		{
			font->size = size;
			Cmiss_graphics_font_changed(font);
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

Cmiss_graphics_font_id Cmiss_graphics_font_access(Cmiss_graphics_font_id font)
{
	if (font)
	{
		font->access_count++;
	}

	return font;
}

int Cmiss_graphics_font_destroy(Cmiss_graphics_font_id *font_address)
{
	int return_code = 0;
	Cmiss_graphics_font_id font;

	if (font_address && (font = *font_address))
	{
		(font->access_count)--;
		if (font->access_count <= 0)
		{
			return_code = DESTROY(Cmiss_graphics_font)(font_address);
		}
		else
		{
			return_code = 1;
		}
		*font_address = NULL;
	}

	return return_code;
}

class Cmiss_graphics_font_type_conversion
{
public:
    static const char *to_string(enum Cmiss_graphics_font_type font_type)
    {
        const char *enum_string = 0;
        switch (font_type)
        {
        case CMISS_GRAPHICS_FONT_TYPE_BITMAP:
            enum_string = "BITMAP";
            break;
        case CMISS_GRAPHICS_FONT_TYPE_PIXMAP:
            enum_string = "PIXMAP";
            break;
        case CMISS_GRAPHICS_FONT_TYPE_POLYGON:
            enum_string = "POLYGON";
            break;
        case CMISS_GRAPHICS_FONT_TYPE_OUTLINE:
            enum_string = "OUTLINE";
            break;
        case CMISS_GRAPHICS_FONT_TYPE_EXTRUDE:
            enum_string = "EXTRUDE";
            break;
        default:
            break;
        }
        return enum_string;
    }
};

enum Cmiss_graphics_font_type Cmiss_graphics_font_type_enum_from_string(
	const char *string)
{
	return string_to_enum<enum Cmiss_graphics_font_type,
		Cmiss_graphics_font_type_conversion>(string);
}

char *Cmiss_graphics_font_type_enum_to_string(
	enum Cmiss_graphics_font_type font_type)
{
	const char *font_type_string =Cmiss_graphics_font_type_conversion::to_string(font_type);
	return (font_type_string ? duplicate_string(font_type_string) : 0);
}

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Cmiss_graphics_font_type)
{
	return Cmiss_graphics_font_type_conversion::to_string(enumerator_value);
}

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Cmiss_graphics_font_type)

class Cmiss_graphics_font_true_type_conversion
{
public:
    static const char *to_string(enum Cmiss_graphics_font_true_type true_type)
    {
        const char *enum_string = 0;
        switch (true_type)
        {
        case CMISS_GRAPHICS_FONT_TRUE_TYPE_OpenSans:
            enum_string = "OpenSans";
            break;
        default:
            break;
        }
        return enum_string;
    }
};

enum Cmiss_graphics_font_true_type Cmiss_graphics_font_true_type_enum_from_string(
	const char *string)
{
	return string_to_enum<enum Cmiss_graphics_font_true_type,
		Cmiss_graphics_font_true_type_conversion>(string);
}

char *Cmiss_graphics_font_true_type_enum_to_string(
	enum Cmiss_graphics_font_true_type true_type)
{
	const char *true_type_string =Cmiss_graphics_font_true_type_conversion::to_string(true_type);
	return (true_type_string ? duplicate_string(true_type_string) : 0);
}

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Cmiss_graphics_font_true_type)
{
	return Cmiss_graphics_font_true_type_conversion::to_string(enumerator_value);
}

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Cmiss_graphics_font_true_type)
