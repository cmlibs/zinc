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

#ifndef CMZN_GRAPHICID_H__
#define CMZN_GRAPHICID_H__

/**
 * A handle to cmiss graphic. cmzn graphic is individual graphic representation
 * such as lines, surfaces, node points and etc of a scene. These graphics
 * can be customised through a number of set functions.
 */
struct cmzn_graphic;
typedef struct cmzn_graphic * cmzn_graphic_id;

/**
 * The contours derived type specific handle to a cmzn_graphic.
 */
struct cmzn_graphic_contours;
typedef struct cmzn_graphic_contours *cmzn_graphic_contours_id;

/**
 * The lines derived type specific handle to a cmzn_graphic.
 * Used to visualise 1-D elements and lines/faces of elements.
 */
struct cmzn_graphic_lines;
typedef struct cmzn_graphic_points *cmzn_graphic_lines_id;

/**
 * The points derived type specific handle to a cmzn_graphic.
 * Used to visualise single points, nodes, data and element points.
 */
struct cmzn_graphic_points;
typedef struct cmzn_graphic_points *cmzn_graphic_points_id;

/**
 * The streamlines derived type specific handle to a cmzn_graphic.
 */
struct cmzn_graphic_streamlines;
typedef struct cmzn_graphic_streamlines *cmzn_graphic_streamlines_id;

/**
 * Enumeration giving the direction streamlines are tracked relative to
 * the stream vector field.
 */
enum cmzn_graphic_streamlines_track_direction
{
	CMISS_GRAPHIC_STREAMLINES_TRACK_DIRECTION_INVALID = 0,
	CMISS_GRAPHIC_STREAMLINES_FORWARD_TRACK = 1,
	CMISS_GRAPHIC_STREAMLINES_REVERSE_TRACK = 2
};

/**
 * The surfaces derived type specific handle to a cmzn_graphic.
 * Used to visualise 2-D elements and faces.
 */
struct cmzn_graphic_surfaces;
typedef struct cmzn_graphic_surfaces *cmzn_graphic_surfaces_id;

/**
 * Enumeration giving the algorithm used to create graphics.
 */
enum cmzn_graphic_type
{
	CMISS_GRAPHIC_TYPE_INVALID = 0,
	CMISS_GRAPHIC_POINTS = 1,
	CMISS_GRAPHIC_LINES = 2,
	CMISS_GRAPHIC_SURFACES = 3,
	CMISS_GRAPHIC_CONTOURS = 4,
	CMISS_GRAPHIC_STREAMLINES = 5
};

/**
 * Enumeration controlling how graphics interact with selection: whether the
 * objects can be picked, the selection highlighted or only the selected
 * or unselected primitives are drawn.
 */
enum cmzn_graphic_select_mode
{
	CMISS_GRAPHIC_SELECT_MODE_INVALID = 0,
	CMISS_GRAPHIC_SELECT_ON = 1,
		/*!< draw all objects with unselected objects drawn in standard material,
		  selected objects in selected_material, and with picking enabled.
		  Default mode for any new graphic. */
	CMISS_GRAPHIC_NO_SELECT = 2,
		/*!< object IDs are not generated so individual nodes/elements cannot be
		  picked nor highlighted. More efficient if picking and highlighting are not
		  required. */
	CMISS_GRAPHIC_DRAW_SELECTED = 3,
		/*!< draw only selected objects in selected_material, with picking enabled */
	CMISS_GRAPHIC_DRAW_UNSELECTED = 4
		/*!< draw only unselected objects in standard material, with picking enabled. */
};

/**
 * A handle to attributes specifying how points are visualised in a
 * cmzn_graphic including glyph, scaling, labels and font.
 */
struct cmzn_graphic_point_attributes;
typedef struct cmzn_graphic_point_attributes * cmzn_graphic_point_attributes_id;

/**
 * A handle to attributes specifying how lines are visualised in a
 * cmzn_graphic including section profile and scaling.
 */
struct cmzn_graphic_line_attributes;
typedef struct cmzn_graphic_line_attributes * cmzn_graphic_line_attributes_id;

/**
 * The shape or profile of graphics generated for lines.
 */
enum cmzn_graphic_line_attributes_shape
{
	CMISS_GRAPHIC_LINE_ATTRIBUTES_SHAPE_INVALID = 0,
	CMISS_GRAPHIC_LINE_ATTRIBUTES_SHAPE_LINE = 1,
	CMISS_GRAPHIC_LINE_ATTRIBUTES_SHAPE_RIBBON = 2,
	CMISS_GRAPHIC_LINE_ATTRIBUTES_SHAPE_CIRCLE_EXTRUSION = 3,
	CMISS_GRAPHIC_LINE_ATTRIBUTES_SHAPE_SQUARE_EXTRUSION = 4
};

/**
 * A handle to graphic attributes specifying how points are sampled in elements,
 * including sample mode, density, etc.
 */
struct cmzn_graphic_sampling_attributes;
typedef struct cmzn_graphic_sampling_attributes * cmzn_graphic_sampling_attributes_id;

/**
 * Enumeration controlling how polygons are rendered in GL.
 */
enum cmzn_graphic_render_polygon_mode
{
	CMISS_GRAPHIC_RENDER_POLYGON_MODE_INVALID = 0,
	CMISS_GRAPHIC_RENDER_POLYGON_SHADED = 1,   /*!< Draw filled polygons */
	CMISS_GRAPHIC_RENDER_POLYGON_WIREFRAME = 2 /*!< Draw polygon wireframe edge lines */
};

#endif
