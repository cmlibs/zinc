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

struct Cmiss_glyph_axes;
typedef struct Cmiss_glyph_axes *Cmiss_glyph_axes_id;

struct Cmiss_glyph_colour_bar;
typedef struct Cmiss_glyph_colour_bar *Cmiss_glyph_colour_bar_id;

/**
 * An enum defining how glyphs are repeatedly display at points.
 */
enum Cmiss_glyph_repeat_mode
{
	CMISS_GLYPH_REPEAT_MODE_INVALID = 0,
	CMISS_GLYPH_REPEAT_NONE = 1,
		/*!< normal single glyph display, no repeat */
	CMISS_GLYPH_REPEAT_AXES_2D = 2,
		/*!< draw glyph 2 times treating each axis as a separate vector. Base size
		 * and scale factors are applied separately to each axis.
		 */
	CMISS_GLYPH_REPEAT_AXES_3D = 3,
		/*!< draw glyph 3 times treating each axis as a separate vector. Base size
		 * and scale factors are applied separately to each axis.
		 */
	CMISS_GLYPH_REPEAT_MIRROR = 4
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
 */
enum Cmiss_glyph_type
{
	CMISS_GLYPH_TYPE_INVALID = 0,
	CMISS_GLYPH_NONE,             /*!< no glyph */
	CMISS_GLYPH_ARROW,            /*!< line arrow from 0,0,0 to 1,0,0, head 1/3 long unit width */
	CMISS_GLYPH_ARROW_SOLID,      /*!< solid arrow from 0,0,0 to 1,0,0, head 1/3 long unit width */
	CMISS_GLYPH_AXIS,             /*!< line arrow from 0,0,0 to 1,0,0, head 0.1 long unit width */
	CMISS_GLYPH_AXIS_SOLID,       /*!< solid arrow from 0,0,0 to 1,0,0, head 0.1 long unit width */
	CMISS_GLYPH_CONE,             /*!< unit diameter cone from base 0,0,0 to apex 1,0,0, open base */
	CMISS_GLYPH_CONE_SOLID,       /*!< unit diameter cone from base 0,0,0 to apex 1,0,0, closed base */
	CMISS_GLYPH_CROSS,            /*!< 3 crossed lines on each axis, centre 0,0,0 */
	CMISS_GLYPH_CUBE_SOLID,       /*!< solid unit cube centred at 0,0,0 and aligned with axes */
	CMISS_GLYPH_CUBE_WIREFRAME,   /*!< wireframe unit cube centred at 0,0,0 and aligned with axes */
	CMISS_GLYPH_CYLINDER,         /*!< unit diameter cylinder from 0,0,0 to 1,0,0, open ends */
	CMISS_GLYPH_CYLINDER_SOLID,   /*!< unit diameter cylinder from 0,0,0 to 1,0,0, closed ends */
	CMISS_GLYPH_DIAMOND,          /*!< unit regular octahedron centred at 0,0,0; a degenerate sphere */
	CMISS_GLYPH_LINE,             /*!< line from 0,0,0 to 1,0,0 */
	CMISS_GLYPH_POINT,            /*!< a single point at 0,0,0 */
	CMISS_GLYPH_SHEET,            /*!< unit square in 1-2 plane centred at 0,0,0 */
	CMISS_GLYPH_SPHERE,           /*!< unit sphere centred at 0,0,0 */
	CMISS_GLYPH_AXES,             /*!< unit line axes without labels */
	CMISS_GLYPH_AXES_123,         /*!< unit line axes with labels 1,2,3 */
	CMISS_GLYPH_AXES_XYZ,         /*!< unit line axes with labels x,y,z */
	CMISS_GLYPH_AXES_COLOUR,      /*!< unit line axes with materials red, green, blue */
	CMISS_GLYPH_AXES_SOLID,       /*!< unit solid arrow axes without labels */
	CMISS_GLYPH_AXES_SOLID_123,   /*!< unit solid arrow axes with labels 1,2,3 */
	CMISS_GLYPH_AXES_SOLID_XYZ,   /*!< unit solid arrow axes with labels x,y,z */
	CMISS_GLYPH_AXES_SOLID_COLOUR /*!< unit solid arrow axes with materials red, green, blue */
};

#endif /* ZINC_GLYPHID_H */
