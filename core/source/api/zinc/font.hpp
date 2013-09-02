/***************************************************************************//**
 * FILE : font.hpp
 */
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
 * The Original Code is libZinc.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2013
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
#ifndef CMZN_FONT_HPP__
#define CMZN_FONT_HPP__

#include "zinc/font.h"

namespace zinc
{

class Font
{
protected:
	cmzn_font_id id;

public:

	Font() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Font(cmzn_font_id font_id) : id(font_id)
	{  }

	Font(const Font& font) : id(cmzn_font_access(font.id))
	{  }

	Font& operator=(const Font& font)
	{
		cmzn_font_id temp_id = cmzn_font_access(font.id);
		cmzn_font_destroy(&id);
		id = temp_id;
		return *this;
	}

	~Font()
	{
		cmzn_font_destroy(&id);
	}

	enum RenderType
	{
		RENDER_TYPE_INVALID = CMISS_FONT_RENDER_TYPE_INVALID,
		RENDER_TYPE_BITMAP = CMISS_FONT_RENDER_TYPE_BITMAP,
		RENDER_TYPE_PIXMAP = CMISS_FONT_RENDER_TYPE_PIXMAP,
		RENDER_TYPE_POLYGON = CMISS_FONT_RENDER_TYPE_POLYGON,
		RENDER_TYPE_OUTLINE = CMISS_FONT_RENDER_TYPE_OUTLINE,
		RENDER_TYPE_EXTRUDE = CMISS_FONT_RENDER_TYPE_EXTRUDE
	};

	enum FontType
	{
		FONT_TYPE_INVALID = CMISS_FONT_TYPE_INVALID,
		FONT_TYPE_OpenSans = CMISS_FONT_TYPE_OpenSans
	};

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_font_id getId()
	{
		return id;
	}

	char *getName()
	{
		return cmzn_font_get_name(id);
	}

	int setName(const char *name)
	{
		return cmzn_font_set_name(id, name);
 	}

	int getBold()
	{
		return cmzn_font_get_bold(id);
	}

	int setBold(int bold)
	{
		return cmzn_font_set_bold(id, bold);
	}

	double getDepth()
	{
		return cmzn_font_get_depth(id);
	}

	int setDepth(double depth)
	{
		return cmzn_font_set_depth(id, depth);
	}

	int getItalic()
	{
		return cmzn_font_get_italic(id);
	}

	int setItalic(int italic)
	{
		return cmzn_font_set_italic(id, italic);
	}

	int getSize()
	{
		return cmzn_font_get_size(id);
	}

	int setSize(int size)
	{
		return cmzn_font_set_size(id, size);
	}

	enum RenderType getRenderType()
	{
		return static_cast<RenderType>(cmzn_font_get_render_type(id));
	}

	int setRenderType(RenderType renderType)
	{
		return cmzn_font_set_render_type(id,
			static_cast<cmzn_font_render_type>(renderType));
	}

	enum FontType getFontType()
	{
		return static_cast<FontType>(cmzn_font_get_font_type(id));
	}

	int setFontType(FontType fontType)
	{
		return cmzn_font_set_font_type(id, static_cast<cmzn_font_type>(fontType));
	}
};

class FontModule
{
protected:
	cmzn_font_module_id id;

public:

	FontModule() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit FontModule(cmzn_font_module_id in_font_module_id) :
		id(in_font_module_id)
	{  }

	FontModule(const FontModule& fontModule) :
		id(cmzn_font_module_access(fontModule.id))
	{  }

	FontModule& operator=(const FontModule& fontModule)
	{
		cmzn_font_module_id temp_id = cmzn_font_module_access(
			fontModule.id);
		if (0 != id)
		{
			cmzn_font_module_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~FontModule()
	{
		if (0 != id)
		{
			cmzn_font_module_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_font_module_id getId()
	{
		return id;
	}

	Font createFont()
	{
		return Font(cmzn_font_module_create_font(id));
	}

	Font findFontByName(const char *name)
	{
		return Font(cmzn_font_module_find_font_by_name(id, name));
	}

	int beginChange()
	{
		return cmzn_font_module_begin_change(id);
	}

	int endChange()
	{
		return cmzn_font_module_end_change(id);
	}

	Font getDefaultFont()
	{
		return Font(cmzn_font_module_get_default_font(id));
	}

	int setDefaultFont(Font &font)
	{
		return cmzn_font_module_set_default_font(id, font.getId());
	}
};

} // namespace zinc

#endif
