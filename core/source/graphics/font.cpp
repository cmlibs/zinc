/*******************************************************************************
FILE : font.c

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
#include "zinc/zincconfigure.h"
#include "zinc/font.h"
#include "zinc/status.h"
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
struct Cmiss_font *Cmiss_font_create_private();

struct Cmiss_font_module
{

private:

	struct MANAGER(Cmiss_font) *fontManager;
	Cmiss_font *defaultFont;
	int access_count;

	Cmiss_font_module() :
		fontManager(CREATE(MANAGER(Cmiss_font))()),
		defaultFont(0),
		access_count(1)
	{
	}

	~Cmiss_font_module()
	{
		if (defaultFont)
		{
			DEACCESS(Cmiss_font)(&(this->defaultFont));
		}
		DESTROY(MANAGER(Cmiss_font))(&(this->fontManager));
	}

public:

	static Cmiss_font_module *create()
	{
		return new Cmiss_font_module();
	}

	Cmiss_font_module *access()

	{
		++access_count;
		return this;
	}

	static int deaccess(Cmiss_font_module* &font_module)
	{
		if (font_module)
		{
			--(font_module->access_count);
			if (font_module->access_count <= 0)
			{
				delete font_module;
			}
			font_module = 0;
			return CMISS_OK;
		}
		return CMISS_ERROR_ARGUMENT;
	}

	struct MANAGER(Cmiss_font) *getManager()
	{
		return this->fontManager;
	}

	int beginChange()
	{
		return MANAGER_BEGIN_CACHE(Cmiss_font)(this->fontManager);
	}

	int endChange()
	{
		return MANAGER_END_CACHE(Cmiss_font)(this->fontManager);
	}

	Cmiss_font_id createFont()
	{
		Cmiss_font_id font = NULL;
		char temp_name[20];
		int i = NUMBER_IN_MANAGER(Cmiss_font)(this->fontManager);
		do
		{
			i++;
			sprintf(temp_name, "temp%d",i);
		}
		while (FIND_BY_IDENTIFIER_IN_MANAGER(Cmiss_font,name)(temp_name,
			this->fontManager));
		font = Cmiss_font_create_private();
		Cmiss_font_set_name(font, temp_name);
		if (!ADD_OBJECT_TO_MANAGER(Cmiss_font)(font, this->fontManager))
		{
			DEACCESS(Cmiss_font)(&font);
		}
		return font;
	}

	Cmiss_font *findFontByName(const char *name)
	{
		Cmiss_font *font = FIND_BY_IDENTIFIER_IN_MANAGER(Cmiss_font,name)(name,
			this->fontManager);
		if (font)
		{
			return ACCESS(Cmiss_font)(font);
		}
		return 0;
	}

	Cmiss_font *getDefaultFont()
	{
		if (this->defaultFont)
		{
			ACCESS(Cmiss_font)(this->defaultFont);
			return this->defaultFont;
		}
		else
		{
			const char *default_font_name = "default";
			struct Cmiss_font *font = findFontByName(default_font_name);
			if (NULL == font)
			{
				font = createFont();
				Cmiss_font_set_name(font, "default");
			}
			if (font)
				setDefaultFont(font);
			return font;
		}
		return 0;
	}

	int setDefaultFont(Cmiss_font *font)
	{
		REACCESS(Cmiss_font)(&this->defaultFont, font);
		return CMISS_OK;
	}

};

Cmiss_font_module_id Cmiss_font_module_create()
{
	return Cmiss_font_module::create();
}

Cmiss_font_module_id Cmiss_font_module_access(
	Cmiss_font_module_id font_module)
{
	if (font_module)
		return font_module->access();
	return 0;
}

int Cmiss_font_module_destroy(Cmiss_font_module_id *font_module_address)
{
	if (font_module_address)
		return Cmiss_font_module::deaccess(*font_module_address);
	return CMISS_ERROR_ARGUMENT;
}

Cmiss_font_id Cmiss_font_module_create_font(
	Cmiss_font_module_id font_module)
{
	if (font_module)
		return font_module->createFont();
	return 0;
}

struct MANAGER(Cmiss_font) *Cmiss_font_module_get_manager(
	Cmiss_font_module_id font_module)
{
	if (font_module)
		return font_module->getManager();
	return 0;
}

int Cmiss_font_module_begin_change(Cmiss_font_module_id font_module)
{
	if (font_module)
		return font_module->beginChange();
   return CMISS_ERROR_ARGUMENT;
}

int Cmiss_font_module_end_change(Cmiss_font_module_id font_module)
{
	if (font_module)
		return font_module->endChange();
   return CMISS_ERROR_ARGUMENT;
}

Cmiss_font_id Cmiss_font_module_find_font_by_name(
	Cmiss_font_module_id font_module, const char *name)
{
	if (font_module)
		return font_module->findFontByName(name);
   return 0;
}

Cmiss_font_id Cmiss_font_module_get_default_font(
	Cmiss_font_module_id font_module)
{
	if (font_module)
		return font_module->getDefaultFont();
	return 0;
}

int Cmiss_font_module_set_default_font(
	Cmiss_font_module_id font_module,
	Cmiss_font_id font)
{
	if (font_module)
		return font_module->setDefaultFont(font);
	return 0;
}

struct Cmiss_font
/*******************************************************************************
LAST MODIFIED : 17 November 2005

DESCRIPTION :
==============================================================================*/
{
	char *name;

	int offset_x, offset_y;
	int size, italic, bold, changed;
	double depth;
	Cmiss_font_type font_type;
	Cmiss_font_render_type render_type;
	/* after clearing in create, following to be modified only by manager */
	struct MANAGER(Cmiss_font) *manager;
	int manager_change_status;
	int access_count;

	FTFont *ftFont;
};

FULL_DECLARE_LIST_TYPE(Cmiss_font);

FULL_DECLARE_MANAGER_TYPE_WITH_OWNER(Cmiss_font, Cmiss_font_module, void *);

/*
Module functions
----------------
*/

DECLARE_LOCAL_MANAGER_FUNCTIONS(Cmiss_font)

DECLARE_LIST_FUNCTIONS(Cmiss_font)
DECLARE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Cmiss_font,name,const char *,strcmp)
DECLARE_LIST_IDENTIFIER_CHANGE_FUNCTIONS(Cmiss_font,name)

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(Cmiss_font,name)
{
	char *name;
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(Cmiss_font,name));
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
"MANAGER_COPY_WITH_IDENTIFIER(Cmiss_font,name).  Insufficient memory");
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
			return_code = MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_font,name)(
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
"MANAGER_COPY_WITH_IDENTIFIER(Cmiss_font,name).  Could not copy without identifier");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Cmiss_font,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(Cmiss_font,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Cmiss_font,name)
{
	int return_code;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_font,name));
	if (source && destination)
	{
		destination->offset_x = 0;
		destination->offset_y = 0;
		destination->size = source->size;
		destination->depth = source->depth;
		destination->italic = source->italic;
		destination->bold = source->bold;
		destination->font_type = source->font_type;
		destination->render_type = source->render_type;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_font,name).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_font,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Cmiss_font,name,const char *)
{
	char *destination_name;
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(Cmiss_font,name));
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
				"MANAGER_COPY_IDENTIFIER(Cmiss_font,name).  Insufficient memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Cmiss_font,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(Cmiss_font,name) */

DECLARE_MANAGER_FUNCTIONS(Cmiss_font,manager)

DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Cmiss_font,manager)

DECLARE_MANAGER_IDENTIFIER_FUNCTIONS(Cmiss_font,name,const char *,manager)

DECLARE_MANAGER_OWNER_FUNCTIONS(Cmiss_font, struct Cmiss_font_module)

int Cmiss_font_manager_set_owner(struct MANAGER(Cmiss_font) *manager,
	struct Cmiss_font_module *font_module)
{
	return MANAGER_SET_OWNER(Cmiss_font)(manager, font_module);
}

DECLARE_OBJECT_FUNCTIONS(Cmiss_font)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Cmiss_font)

struct Cmiss_font *Cmiss_font_create_private()
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
==============================================================================*/
{
	Cmiss_font *font;

	if (ALLOCATE(font, struct Cmiss_font, 1))
	{
		font->name = 0;
		font->offset_x = 0;
		font->offset_y = 0;
		font->size = 15;
		font->italic = 0;
		font->bold = 0;
		font->depth = 0.1;
		font->font_type = CMISS_FONT_TYPE_OpenSans;
		font->render_type = CMISS_FONT_RENDER_TYPE_BITMAP;
		font->manager = (struct MANAGER(Cmiss_font) *)NULL;
		font->manager_change_status = MANAGER_CHANGE_NONE(Cmiss_font);
		font->ftFont = 0;
		font->changed = 1;
		font->access_count = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Cmiss_font). Unable to allocate font structure");
		font = (struct Cmiss_font *)NULL;
	}

	return (font);
} /* CREATE(Cmiss_font) */

int DESTROY(Cmiss_font)(struct Cmiss_font **font_address)
/*******************************************************************************
LAST MODIFIED : 17 November 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Cmiss_font *font;

	ENTER(DESTROY(Cmiss_font));
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
		*font_address = (struct Cmiss_font *)NULL;

		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Cmiss_font).  Missing package");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Cmiss_font) */

unsigned int Cmiss_font_get_font_buffer(struct Cmiss_font *font,
	unsigned char **font_type_buffer_out)
{
	unsigned char *font_type_buffer = 0;
	unsigned int font_type_length = 0;
	switch (font->font_type)
	{
		case CMISS_FONT_TYPE_OpenSans:
		{
			if (font->bold)
			{
				if (font->italic)
				{
					font_type_buffer = OpenSans_BoldItalic_ttf;
					font_type_length = OpenSans_BoldItalic_ttf_len;
				}
				else
				{
					font_type_buffer = OpenSans_Bold_ttf;
					font_type_length = OpenSans_Bold_ttf_len;
				}
			}
			else if (font->italic)
			{
				font_type_buffer = OpenSans_Italic_ttf;
				font_type_length = OpenSans_Italic_ttf_len;
			}
			else
			{
				font_type_buffer = OpenSans_Regular_ttf;
				font_type_length = OpenSans_Regular_ttf_len;
			}
		} break;
		default :
		{
		}break;
	}
	*font_type_buffer_out = font_type_buffer;
	return font_type_length;
}

int Cmiss_font_compile(struct Cmiss_font *font)
/*******************************************************************************
LAST MODIFIED : 12 March 2008

DESCRIPTION :
Compiles the specified <font> so it can be used by the graphics.  The
<buffer> is required so that we can determine what API is used by this buffer.
==============================================================================*/
{
	int return_code;

	if (font)
	{
		return_code = 1;

		/* Can have multiple types compiled in at the same time (X and gtk) */
		if (font->ftFont && font->changed == 1)
		{
			delete font->ftFont;
			font->ftFont = 0;
		}
		if (font->ftFont == 0)
		{
			unsigned char *font_type_buffer = 0;
			unsigned int font_type_length = 0;
			font_type_length = Cmiss_font_get_font_buffer(font,
				&font_type_buffer);
			if ((font_type_buffer != 0) && (font_type_length > 0))
			{
				switch (font->render_type)
				{
				case CMISS_FONT_RENDER_TYPE_BITMAP:
				case CMISS_FONT_RENDER_TYPE_PIXMAP:
				{
					if (font->render_type == CMISS_FONT_RENDER_TYPE_BITMAP)
						font->ftFont = new FTBitmapFont(font_type_buffer, font_type_length);
					else
						font->ftFont = new FTPixmapFont(font_type_buffer, font_type_length);
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
				case CMISS_FONT_RENDER_TYPE_POLYGON:
				case CMISS_FONT_RENDER_TYPE_OUTLINE:
				case CMISS_FONT_RENDER_TYPE_EXTRUDE:
				{
					if (font->render_type == CMISS_FONT_RENDER_TYPE_POLYGON)
						font->ftFont = new FTPolygonFont(font_type_buffer, font_type_length);
					else if (font->render_type == CMISS_FONT_RENDER_TYPE_OUTLINE)
						font->ftFont = new FTOutlineFont(font_type_buffer, font_type_length);
					else
						font->ftFont = new FTExtrudeFont(font_type_buffer, font_type_length);
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
		font->changed = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_font_compile.  "
			"Invalid argument");
		return_code = 0;
	}

	return (return_code);
} /* Cmiss_font_compile */

int Cmiss_font_rendergl_text(struct Cmiss_font *font, char *text,
	float x, float y, float z)
/*******************************************************************************
LAST MODIFIED : 17 November 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code = 0;

	ENTER(Cmiss_font_rendergl_text);

	if (font && text)
	{
		if (font->ftFont)
		{
			switch (font->render_type)
			{
				case CMISS_FONT_RENDER_TYPE_BITMAP:
				case CMISS_FONT_RENDER_TYPE_PIXMAP:
				{
					glRasterPos3f(x, y, z);
					font->ftFont->Render(text);
				} break;
				case CMISS_FONT_RENDER_TYPE_POLYGON:
				case CMISS_FONT_RENDER_TYPE_OUTLINE:
				case CMISS_FONT_RENDER_TYPE_EXTRUDE:
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
				"Cmiss_font_rendergl_text.  "
				"Font is being used to render text before being compiled.");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_font_rendergl_text.  "
			"Invalid arguments");
	}

	LEAVE;
	return (return_code);
} /* Cmiss_font_rendergl_text */

int Cmiss_font_set_name(
	Cmiss_font_id font, const char *name)
{
	int return_code = 0;

	if (font && name)
	{
		return_code = 1;
		if (font->manager)
		{
			return_code = MANAGER_MODIFY_IDENTIFIER(Cmiss_font, name)(
				font, name, font->manager);
		}
		else
		{
			char *new_name = duplicate_string(name);
			if (new_name)
			{
				DEALLOCATE(font->name);
				font->name = new_name;
			}
			else
			{
				return_code = 0;
			}
		}
	}

	return return_code;
}

char *Cmiss_font_get_name(Cmiss_font_id font)
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
 * @param font  Modified Cmiss_font to be broadcast.
 * @return 1 on success, 0 on failure
 */
int Cmiss_font_changed(Cmiss_font_id font)
{
	if (font)
	{
		font->changed = 1;
		return MANAGED_OBJECT_CHANGE(Cmiss_font)(font,
			MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Cmiss_font));
	}
	else
	{
		return 0;
	}
}


Cmiss_font_type Cmiss_font_get_font_type(
	Cmiss_font_id font)
{
	if (font)
	{
		return font->font_type;
	}
	else
	{
		return CMISS_FONT_TYPE_INVALID;
	}
}

int Cmiss_font_set_font_type(Cmiss_font_id font,
	Cmiss_font_type font_type)
{
	if (font)
	{
		if (font->font_type != font_type)
		{
			font->font_type = font_type;
			Cmiss_font_changed(font);
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

Cmiss_font_render_type Cmiss_font_get_render_type(
	Cmiss_font_id font)
{
	if (font)
	{
		return font->render_type;
	}
	else
	{
		return CMISS_FONT_RENDER_TYPE_INVALID;
	}
}

int Cmiss_font_set_render_type(Cmiss_font_id font,
	Cmiss_font_render_type render_type)
{
	if (font)
	{
		if (font->render_type != render_type)
		{
			font->render_type = render_type;
			Cmiss_font_changed(font);
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

int Cmiss_font_get_bold(Cmiss_font_id font)
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

int Cmiss_font_set_bold(Cmiss_font_id font, int bold)
{
	if (font)
	{
		if (font->bold != bold)
		{
			font->bold = bold;
			Cmiss_font_changed(font);
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

double Cmiss_font_get_depth(Cmiss_font_id font)
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

int Cmiss_font_set_depth(Cmiss_font_id font, double depth)
{
	if (font)
	{
		if (font->depth != depth)
		{
			font->depth = depth;
			if (font->render_type == CMISS_FONT_RENDER_TYPE_EXTRUDE)
				Cmiss_font_changed(font);
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

int Cmiss_font_get_italic(Cmiss_font_id font)
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

int Cmiss_font_set_italic(Cmiss_font_id font, int italic)
{
	if (font)
	{
		if (font->italic != italic)
		{
			font->italic = italic;
			Cmiss_font_changed(font);
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

int Cmiss_font_get_size(Cmiss_font_id font)
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

int Cmiss_font_set_size(Cmiss_font_id font, int size)
{
	if (font)
	{
		if (font->size != size)
		{
			font->size = size;
			Cmiss_font_changed(font);
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

Cmiss_font_id Cmiss_font_access(Cmiss_font_id font)
{
	if (font)
	{
		font->access_count++;
	}

	return font;
}

int Cmiss_font_destroy(Cmiss_font_id *font_address)
{
	int return_code = 0;
	Cmiss_font_id font;

	if (font_address && (font = *font_address))
	{
		(font->access_count)--;
		if (font->access_count <= 0)
		{
			return_code = DESTROY(Cmiss_font)(font_address);
		}
		else
		{
			return_code = 1;
		}
		*font_address = NULL;
	}

	return return_code;
}

class Cmiss_font_render_type_conversion
{
public:
	static const char *to_string(enum Cmiss_font_render_type render_type)
	{
		const char *enum_string = 0;
		switch (render_type)
		{
		case CMISS_FONT_RENDER_TYPE_BITMAP:
			enum_string = "BITMAP";
			break;
		case CMISS_FONT_RENDER_TYPE_PIXMAP:
			enum_string = "PIXMAP";
			break;
		case CMISS_FONT_RENDER_TYPE_POLYGON:
			enum_string = "POLYGON";
			break;
		case CMISS_FONT_RENDER_TYPE_OUTLINE:
			enum_string = "OUTLINE";
			break;
		case CMISS_FONT_RENDER_TYPE_EXTRUDE:
			enum_string = "EXTRUDE";
			break;
		default:
			break;
		}
		return enum_string;
	}
};

enum Cmiss_font_render_type Cmiss_font_render_type_enum_from_string(
	const char *string)
{
	return string_to_enum<enum Cmiss_font_render_type,
		Cmiss_font_render_type_conversion>(string);
}

char *Cmiss_font_render_type_enum_to_string(
	enum Cmiss_font_render_type render_type)
{
	const char *render_type_string =Cmiss_font_render_type_conversion::to_string(render_type);
	return (render_type_string ? duplicate_string(render_type_string) : 0);
}

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Cmiss_font_render_type)
{
	return Cmiss_font_render_type_conversion::to_string(enumerator_value);
}

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Cmiss_font_render_type)

class Cmiss_font_type_conversion
{
public:
	static const char *to_string(enum Cmiss_font_type font_type)
	{
		const char *enum_string = 0;
		switch (font_type)
		{
		case CMISS_FONT_TYPE_OpenSans:
			enum_string = "OpenSans";
			break;
		default:
			break;
		}
		return enum_string;
	}
};

enum Cmiss_font_type Cmiss_font_type_enum_from_string(
	const char *string)
{
	return string_to_enum<enum Cmiss_font_type,
		Cmiss_font_type_conversion>(string);
}

char *Cmiss_font_type_enum_to_string(
	enum Cmiss_font_type font_type)
{
	const char *font_type_string =Cmiss_font_type_conversion::to_string(font_type);
	return (font_type_string ? duplicate_string(font_type_string) : 0);
}

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Cmiss_font_type)
{
	return Cmiss_font_type_conversion::to_string(enumerator_value);
}

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Cmiss_font_type)
