/**
 * FILE : graphicid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_GRAPHICID_H__
#define CMZN_GRAPHICID_H__

/**
 * A handle to zinc graphic. zinc graphic is individual graphic representation
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
	CMZN_GRAPHIC_STREAMLINES_TRACK_DIRECTION_INVALID = 0,
	CMZN_GRAPHIC_STREAMLINES_FORWARD_TRACK = 1,
	CMZN_GRAPHIC_STREAMLINES_REVERSE_TRACK = 2
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
	CMZN_GRAPHIC_TYPE_INVALID = 0,
	CMZN_GRAPHIC_POINTS = 1,
	CMZN_GRAPHIC_LINES = 2,
	CMZN_GRAPHIC_SURFACES = 3,
	CMZN_GRAPHIC_CONTOURS = 4,
	CMZN_GRAPHIC_STREAMLINES = 5
};

/**
 * Enumeration controlling how graphics interact with selection: whether the
 * objects can be picked, the selection highlighted or only the selected
 * or unselected primitives are drawn.
 */
enum cmzn_graphic_select_mode
{
	CMZN_GRAPHIC_SELECT_MODE_INVALID = 0,
	CMZN_GRAPHIC_SELECT_ON = 1,
		/*!< draw all objects with unselected objects drawn in standard material,
		  selected objects in selected_material, and with picking enabled.
		  Default mode for any new graphic. */
	CMZN_GRAPHIC_NO_SELECT = 2,
		/*!< object IDs are not generated so individual nodes/elements cannot be
		  picked nor highlighted. More efficient if picking and highlighting are not
		  required. */
	CMZN_GRAPHIC_DRAW_SELECTED = 3,
		/*!< draw only selected objects in selected_material, with picking enabled */
	CMZN_GRAPHIC_DRAW_UNSELECTED = 4
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
	CMZN_GRAPHIC_LINE_ATTRIBUTES_SHAPE_INVALID = 0,
	CMZN_GRAPHIC_LINE_ATTRIBUTES_SHAPE_LINE = 1,
	CMZN_GRAPHIC_LINE_ATTRIBUTES_SHAPE_RIBBON = 2,
	CMZN_GRAPHIC_LINE_ATTRIBUTES_SHAPE_CIRCLE_EXTRUSION = 3,
	CMZN_GRAPHIC_LINE_ATTRIBUTES_SHAPE_SQUARE_EXTRUSION = 4
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
	CMZN_GRAPHIC_RENDER_POLYGON_MODE_INVALID = 0,
	CMZN_GRAPHIC_RENDER_POLYGON_SHADED = 1,   /*!< Draw filled polygons */
	CMZN_GRAPHIC_RENDER_POLYGON_WIREFRAME = 2 /*!< Draw polygon wireframe edge lines */
};

#endif
