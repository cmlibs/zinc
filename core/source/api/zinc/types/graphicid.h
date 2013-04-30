/**
 * FILE : cmiss_graphic_id.h
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
 * Portions created by the Initial Developer are Copyright (C) 2010-2011
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

#if !defined (CMISS_GRAPHICID_H)
#define CMISS_GRAPHICID_H

/**
 * A handle to cmiss graphic. Cmiss graphic is individual graphic representation
 * such as lines, surfaces, node points and etc of a rendition. These graphics
 * can be customised through a number of set functions.
 */
struct Cmiss_graphic;
typedef struct Cmiss_graphic * Cmiss_graphic_id;

/**
 * An enum type to define the type of a cmiss_graphic.
 */
enum Cmiss_graphic_type
{
	CMISS_GRAPHIC_TYPE_INVALID = 0,
	CMISS_GRAPHIC_NODE_POINTS = 1,
	CMISS_GRAPHIC_DATA_POINTS = 2,
	CMISS_GRAPHIC_LINES = 3,
	CMISS_GRAPHIC_CYLINDERS = 4,
	CMISS_GRAPHIC_SURFACES = 5,
	CMISS_GRAPHIC_ISO_SURFACES = 6,
	CMISS_GRAPHIC_ELEMENT_POINTS = 7,
	CMISS_GRAPHIC_STREAMLINES = 8,
	CMISS_GRAPHIC_POINT = 9 /*!< CMISS_GRAPHIC_POINT is different from others,
	 * as the graphics object of this is created by user instead of generated from
	 * finite element models, it does not require a coordinate field in the
	 * rendition. To get an idea of what graphics objects are, take a look at
	 * the glyphs used in points representation they are a set of preset graphics
	 * object in cmgui. This will be replaced in the future*/
}; /* enum Cmiss_graphics_type */

/**
 *	An enum defining the type of graphic glyph.
 */
enum Cmiss_graphics_glyph_type
{
	CMISS_GRAPHICS_GLYPH_TYPE_INVALID = 0,
	CMISS_GRAPHICS_GLYPH_NONE,
	CMISS_GRAPHICS_GLYPH_POINT,
	CMISS_GRAPHICS_GLYPH_LINE,
	CMISS_GRAPHICS_GLYPH_SPHERE,
	CMISS_GRAPHICS_GLYPH_AXES_SOLID
};

/**
 * An enum defining the type of element dimension to use.
 */
enum Cmiss_graphic_use_element_type
{
	CMISS_GRAPHIC_USE_ELEMENT_TYPE_INVALID = 0,
	CMISS_GRAPHIC_USE_ELEMENT_HIGHEST_DIMENSION,
	CMISS_GRAPHIC_USE_ELEMENT_FACES,
	CMISS_GRAPHIC_USE_ELEMENT_LINES
};

/**
 * An enum defining the faces a graphic may use.
 */
enum Cmiss_graphic_face_type
{
	CMISS_GRAPHIC_FACE_INVALID = -1,
	CMISS_GRAPHIC_FACE_ALL,   /*!< include all faces */
	CMISS_GRAPHIC_FACE_XI1_0, /*!< only faces where top-level xi1 == 0 */
	CMISS_GRAPHIC_FACE_XI1_1, /*!< only faces where top-level xi1 == 1 */
	CMISS_GRAPHIC_FACE_XI2_0, /*!< only faces where top-level xi2 == 0 */
	CMISS_GRAPHIC_FACE_XI2_1, /*!< only faces where top-level xi2 == 1 */
	CMISS_GRAPHIC_FACE_XI3_0, /*!< only faces where top-level xi3 == 0 */
	CMISS_GRAPHIC_FACE_XI3_1  /*!< only faces where top-level xi3 == 1 */
};

/**
 * A handle to attributes specifying how points are visualised in a
 * Cmiss_graphic including glyph, scaling, labels and font.
 */
struct Cmiss_graphic_point_attributes;
typedef struct Cmiss_graphic_point_attributes * Cmiss_graphic_point_attributes_id;

/**
 * A handle to attributes specifying how lines are visualised in a
 * Cmiss_graphic including section profile and scaling.
 */
struct Cmiss_graphic_line_attributes;
typedef struct Cmiss_graphic_line_attributes * Cmiss_graphic_line_attributes_id;

enum Cmiss_graphics_xi_discretization_mode
{
	CMISS_GRAPHICS_XI_DISCRETIZATION_INVALID_MODE = 0,
	CMISS_GRAPHICS_XI_DISCRETIZATION_CELL_CENTRES = 1,
	CMISS_GRAPHICS_XI_DISCRETIZATION_CELL_CORNERS = 2,
	CMISS_GRAPHICS_XI_DISCRETIZATION_CELL_DENSITY = 3,
	CMISS_GRAPHICS_XI_DISCRETIZATION_CELL_POISSON = 4,
	CMISS_GRAPHICS_XI_DISCRETIZATION_CELL_RANDOM = 5,
	CMISS_GRAPHICS_XI_DISCRETIZATION_EXACT_XI = 6
};

/**
 * A handle to attributes specifying how lines are visualised in a
 * Cmiss_graphic including section profile and scaling.
 */
struct Cmiss_graphic_element_attributes;
typedef struct Cmiss_graphic_element_attributes * Cmiss_graphic_element_attributes_id;

#endif /* CMISS_GRAPHICID_H */
