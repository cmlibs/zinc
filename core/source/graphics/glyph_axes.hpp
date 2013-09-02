/**
 * glyph_axes.hpp
 *
 * Derived glyph type which creates 2 or 3 dimensional axes with optional
 * per axis labels and materials.
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
 * The Original Code is cmgui.
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
#ifndef GLYPH_AXES_HPP
#define GLYPH_AXES_HPP

#include "zinc/graphicsmaterial.h"
#include "graphics/glyph.hpp"

struct cmzn_glyph_axes : public cmzn_glyph
{
private:
	cmzn_glyph *axisGlyph;
	double axisWidth;
	char *axisLabels[3];
	cmzn_graphics_material *axisMaterials[3];
	GT_object *graphicsObject;

	cmzn_glyph_axes(cmzn_glyph *axisGlyphIn, double axisWidthIn);
	virtual ~cmzn_glyph_axes();
	void invalidate();

public:
	static cmzn_glyph_axes *create(cmzn_glyph *axisGlyphIn, double axisWidthIn)
	{
		if (axisGlyphIn && (axisWidthIn >= 0.0))
			return new cmzn_glyph_axes(axisGlyphIn, axisWidthIn);
		return 0;
	}

	virtual GT_object *getGraphicsObject(cmzn_tessellation *tessellation,
		cmzn_graphics_material *material, cmzn_font *font);

	virtual bool usesFont()
	{
		for (int i = 0; i < 3; ++i)
		{
			if (axisLabels[i])
				return true;
		}
		return false;
	}

	double getAxisWidth() const
	{
		return this->axisWidth;
	}

	int setAxisWidth(double axisWidthIn);

	/** @return  Allocated string or 0 */
	char *getAxisLabel(int axisNumber);

	int setAxisLabel(int axisNumber, const char *label);

	/** @return  ACCESSed material or 0 */
	cmzn_graphics_material *getAxisMaterial(int axisNumber);

	int setAxisMaterial(int axisNumber, cmzn_graphics_material *material);

	virtual void fontChange();

	virtual void materialChange(struct MANAGER_MESSAGE(Graphical_material) *message);

	virtual bool usesCircleDivisions()
	{
		return this->axisGlyph->usesCircleDivisions();
	}

};

#endif // GLYPH_AXES_HPP
