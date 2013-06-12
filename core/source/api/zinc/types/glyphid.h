/**
 * FILE : glyphid.h
 *
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

#ifndef ZINC_GLYPHID_H
#define ZINC_GLYPHID_H

struct Cmiss_glyph_module;
typedef struct Cmiss_glyph_module *Cmiss_glyph_module_id;

struct Cmiss_glyph;
typedef struct Cmiss_glyph *Cmiss_glyph_id;

/**
 * An enum defining how glyphs are repeatedly display at points.
 */
enum Cmiss_glyph_repeat_mode
{
	CMISS_GLYPH_REPEAT_MODE_INVALID = -1,
	CMISS_GLYPH_REPEAT_NONE = 0,
		/*!< normal single glyph display, no repeat */
	CMISS_GLYPH_REPEAT_AXES_2D = 1,
		/*!< draw glyph 2 times treating each axis as a separate vector. Base size
		 * and scale factors are applied separately to each axis.
		 */
	CMISS_GLYPH_REPEAT_AXES_3D = 2,
		/*!< draw glyph 3 times treating each axis as a separate vector. Base size
		 * and scale factors are applied separately to each axis.
		 */
	CMISS_GLYPH_REPEAT_MIRROR = 3
		/*!< draw glyph twice, second mirrored about axis1 == 0.0. Commonly used
		 * with a signed_scale_field to visualise stress and strains using pairs of
		 * arrow glyphs pointing inward for compression, outward for tension.
		 * Suitable glyphs (line, arrow, cone) span from 0 to 1 along their first
		 * axis. Not suitable for sphere, cube etc. which are symmetric about 0.0 on
		 * axis 1. In this mode the label is only shown for the first glyph and the
		 * label offset is not reversed with the glyph.
		 * @see Cmiss_graphic_point_attributes_set_signed_scale_field
		 */
};

/**
 * An enum defining the type of graphic glyph.
 * @deprecated
 */
enum Cmiss_graphics_glyph_type
{
	CMISS_GRAPHICS_GLYPH_TYPE_INVALID = -1,
	CMISS_GRAPHICS_GLYPH_NONE,
	CMISS_GRAPHICS_GLYPH_POINT,
	CMISS_GRAPHICS_GLYPH_LINE,
	CMISS_GRAPHICS_GLYPH_CROSS,
	CMISS_GRAPHICS_GLYPH_SPHERE,
	CMISS_GRAPHICS_GLYPH_AXES_SOLID
};

#endif /* ZINC_GLYPHID_H */
