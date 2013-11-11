/**
 * FILE : glyphid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_GLYPHID_H__
#define CMZN_GLYPHID_H__

struct cmzn_glyphmodule;
typedef struct cmzn_glyphmodule *cmzn_glyphmodule_id;

struct cmzn_glyph;
typedef struct cmzn_glyph *cmzn_glyph_id;

struct cmzn_glyph_axes;
typedef struct cmzn_glyph_axes *cmzn_glyph_axes_id;

struct cmzn_glyph_colour_bar;
typedef struct cmzn_glyph_colour_bar *cmzn_glyph_colour_bar_id;

/**
 * An enum defining how glyphs are repeatedly display at points.
 */
enum cmzn_glyph_repeat_mode
{
	CMZN_GLYPH_REPEAT_MODE_INVALID = 0,
	CMZN_GLYPH_REPEAT_MODE_NONE = 1,
		/*!< default cmzn_glyph_repeat_mode
		 * normal single glyph display, no repeat */
	CMZN_GLYPH_REPEAT_MODE_AXES_2D = 2,
		/*!< draw glyph 2 times treating each axis as a separate vector. Base size
		 * and scale factors are applied separately to each axis.
		 */
	CMZN_GLYPH_REPEAT_MODE_AXES_3D = 3,
		/*!< draw glyph 3 times treating each axis as a separate vector. Base size
		 * and scale factors are applied separately to each axis.
		 */
	CMZN_GLYPH_REPEAT_MODE_MIRROR = 4
		/*!< draw glyph twice, second mirrored about axis1 == 0.0. Commonly used
		 * with a signed_scale_field to visualise stress and strains using pairs of
		 * arrow glyphs pointing inward for compression, outward for tension.
		 * Suitable glyphs (line, arrow, cone) span from 0 to 1 along their first
		 * axis. Not suitable for sphere, cube etc. which are symmetric about 0.0 on
		 * axis 1. In this mode the label is only shown for the first glyph and the
		 * label offset is not reversed with the glyph.
		 * @see cmzn_graphicspointattributes_set_signed_scale_field
		 */
};

/**
 * An enum defining the type of graphics glyph.
 */
enum cmzn_glyph_shape_type
{
	CMZN_GLYPH_SHAPE_TYPE_INVALID = 0,
	CMZN_GLYPH_SHAPE_TYPE_NONE,             /*!< no glyph */
	CMZN_GLYPH_SHAPE_TYPE_ARROW,            /*!< line arrow from 0,0,0 to 1,0,0, head 1/3 long unit width */
	CMZN_GLYPH_SHAPE_TYPE_ARROW_SOLID,      /*!< solid arrow from 0,0,0 to 1,0,0, head 1/3 long unit width */
	CMZN_GLYPH_SHAPE_TYPE_AXIS,             /*!< line arrow from 0,0,0 to 1,0,0, head 0.1 long unit width */
	CMZN_GLYPH_SHAPE_TYPE_AXIS_SOLID,       /*!< solid arrow from 0,0,0 to 1,0,0, head 0.1 long unit width */
	CMZN_GLYPH_SHAPE_TYPE_CONE,             /*!< unit diameter cone from base 0,0,0 to apex 1,0,0, open base */
	CMZN_GLYPH_SHAPE_TYPE_CONE_SOLID,       /*!< unit diameter cone from base 0,0,0 to apex 1,0,0, closed base */
	CMZN_GLYPH_SHAPE_TYPE_CROSS,            /*!< 3 crossed lines on each axis, centre 0,0,0 */
	CMZN_GLYPH_SHAPE_TYPE_CUBE_SOLID,       /*!< solid unit cube centred at 0,0,0 and aligned with axes */
	CMZN_GLYPH_SHAPE_TYPE_CUBE_WIREFRAME,   /*!< wireframe unit cube centred at 0,0,0 and aligned with axes */
	CMZN_GLYPH_SHAPE_TYPE_CYLINDER,         /*!< unit diameter cylinder from 0,0,0 to 1,0,0, open ends */
	CMZN_GLYPH_SHAPE_TYPE_CYLINDER_SOLID,   /*!< unit diameter cylinder from 0,0,0 to 1,0,0, closed ends */
	CMZN_GLYPH_SHAPE_TYPE_DIAMOND,          /*!< unit regular octahedron centred at 0,0,0; a degenerate sphere */
	CMZN_GLYPH_SHAPE_TYPE_LINE,             /*!< line from 0,0,0 to 1,0,0 */
	CMZN_GLYPH_SHAPE_TYPE_POINT,            /*!< a single point at 0,0,0 */
	CMZN_GLYPH_SHAPE_TYPE_SHEET,            /*!< unit square in 1-2 plane centred at 0,0,0 */
	CMZN_GLYPH_SHAPE_TYPE_SPHERE,           /*!< unit sphere centred at 0,0,0 */
	CMZN_GLYPH_SHAPE_TYPE_AXES,             /*!< unit line axes without labels */
	CMZN_GLYPH_SHAPE_TYPE_AXES_123,         /*!< unit line axes with labels 1,2,3 */
	CMZN_GLYPH_SHAPE_TYPE_AXES_XYZ,         /*!< unit line axes with labels x,y,z */
	CMZN_GLYPH_SHAPE_TYPE_AXES_COLOUR,      /*!< unit line axes with materials red, green, blue */
	CMZN_GLYPH_SHAPE_TYPE_AXES_SOLID,       /*!< unit solid arrow axes without labels */
	CMZN_GLYPH_SHAPE_TYPE_AXES_SOLID_123,   /*!< unit solid arrow axes with labels 1,2,3 */
	CMZN_GLYPH_SHAPE_TYPE_AXES_SOLID_XYZ,   /*!< unit solid arrow axes with labels x,y,z */
	CMZN_GLYPH_SHAPE_TYPE_AXES_SOLID_COLOUR /*!< unit solid arrow axes with materials red, green, blue */
};

#endif
