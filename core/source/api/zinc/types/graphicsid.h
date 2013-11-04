/**
 * FILE : graphicsid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_GRAPHICSID_H__
#define CMZN_GRAPHICSID_H__

/**
 * A handle to zinc graphics, a visualisation of fields using an algorithm
 * i.e. points, lines, surfaces, contours and streamlines.
 * These graphics can be customised through a number of set functions.
 */
struct cmzn_graphics;
typedef struct cmzn_graphics * cmzn_graphics_id;

/**
 * The contours derived type specific handle to a cmzn_graphics.
 */
struct cmzn_graphics_contours;
typedef struct cmzn_graphics_contours *cmzn_graphics_contours_id;

/**
 * The lines derived type specific handle to a cmzn_graphics.
 * Used to visualise 1-D elements and lines/faces of elements.
 */
struct cmzn_graphics_lines;
typedef struct cmzn_graphics_points *cmzn_graphics_lines_id;

/**
 * The points derived type specific handle to a cmzn_graphics.
 * Used to visualise single points, nodes, data and element points.
 */
struct cmzn_graphics_points;
typedef struct cmzn_graphics_points *cmzn_graphics_points_id;

/**
 * The streamlines derived type specific handle to a cmzn_graphics.
 */
struct cmzn_graphics_streamlines;
typedef struct cmzn_graphics_streamlines *cmzn_graphics_streamlines_id;

/**
 * Enumeration giving the direction streamlines are tracked relative to
 * the stream vector field.
 */
enum cmzn_graphics_streamlines_track_direction
{
	CMZN_GRAPHICS_STREAMLINES_TRACK_DIRECTION_INVALID = 0,
	CMZN_GRAPHICS_STREAMLINES_TRACK_DIRECTION_FORWARD = 1,
	CMZN_GRAPHICS_STREAMLINES_TRACK_DIRECTION_REVERSE = 2
};

/**
 * The surfaces derived type specific handle to a cmzn_graphics.
 * Used to visualise 2-D elements and faces.
 */
struct cmzn_graphics_surfaces;
typedef struct cmzn_graphics_surfaces *cmzn_graphics_surfaces_id;

/**
 * Enumeration giving the algorithm used to create graphics.
 */
enum cmzn_graphics_type
{
	CMZN_GRAPHICS_TYPE_INVALID = 0,
	CMZN_GRAPHICS_TYPE_POINTS = 1,
	CMZN_GRAPHICS_TYPE_LINES = 2,
	CMZN_GRAPHICS_TYPE_SURFACES = 3,
	CMZN_GRAPHICS_TYPE_CONTOURS = 4,
	CMZN_GRAPHICS_TYPE_STREAMLINES = 5
};

/**
 * Enumeration controlling how graphics interact with selection: whether the
 * objects can be picked, the selection highlighted or only the selected
 * or unselected primitives are drawn.
 */
enum cmzn_graphics_select_mode
{
	CMZN_GRAPHICS_SELECT_MODE_INVALID = 0,
	CMZN_GRAPHICS_SELECT_MODE_ON = 1,
		/*!< draw all objects with unselected objects drawn in standard material,
		  selected objects in selected_material, and with picking enabled.
		  Default mode for any new graphics. */
	CMZN_GRAPHICS_SELECT_MODE_OFF = 2,
		/*!< object IDs are not generated so individual nodes/elements cannot be
		  picked nor highlighted. More efficient if picking and highlighting are not
		  required. */
	CMZN_GRAPHICS_SELECT_MODE_DRAW_SELECTED = 3,
		/*!< draw only selected objects in selected_material, with picking enabled */
	CMZN_GRAPHICS_SELECT_MODE_DRAW_UNSELECTED = 4
		/*!< draw only unselected objects in standard material, with picking enabled. */
};

/**
 * A handle to attributes specifying how points are visualised in a
 * cmzn_graphics including glyph, scaling, labels and font.
 */
struct cmzn_graphicspointattributes;
typedef struct cmzn_graphicspointattributes * cmzn_graphicspointattributes_id;

/**
 * A handle to attributes specifying how lines are visualised in a
 * cmzn_graphics including section profile and scaling.
 */
struct cmzn_graphicslineattributes;
typedef struct cmzn_graphicslineattributes * cmzn_graphicslineattributes_id;

/**
 * The shape or profile of graphics generated for lines.
 */
enum cmzn_graphicslineattributes_shape_type
{
	CMZN_GRAPHICSLINEATTRIBUTES_SHAPE_TYPE_INVALID = 0,
	CMZN_GRAPHICSLINEATTRIBUTES_SHAPE_TYPE_LINE = 1,
	CMZN_GRAPHICSLINEATTRIBUTES_SHAPE_TYPE_RIBBON = 2,
	CMZN_GRAPHICSLINEATTRIBUTES_SHAPE_TYPE_CIRCLE_EXTRUSION = 3,
	CMZN_GRAPHICSLINEATTRIBUTES_SHAPE_TYPE_SQUARE_EXTRUSION = 4
};

/**
 * A handle to graphics attributes specifying how points are sampled in elements,
 * including sample mode, density, etc.
 */
struct cmzn_graphicssamplingattributes;
typedef struct cmzn_graphicssamplingattributes * cmzn_graphicssamplingattributes_id;

/**
 * Enumeration controlling how polygons are rendered in GL.
 */
enum cmzn_graphics_render_polygon_mode
{
	CMZN_GRAPHICS_RENDER_POLYGON_MODE_INVALID = 0,
	CMZN_GRAPHICS_RENDER_POLYGON_MODE_SHADED = 1,   /*!< Draw filled polygons */
	CMZN_GRAPHICS_RENDER_POLYGON_MODE_WIREFRAME = 2 /*!< Draw polygon wireframe edge lines */
};

#endif
