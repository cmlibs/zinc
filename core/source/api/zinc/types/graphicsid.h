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
 * Base graphics type: produces 3-D graphics visualising domains and fields from
 * the owning scene's region. Derived types (points, lines, surfaces, contours
 * and streamlines) specify the algorithm used to create the graphics and
 * may have additional type-specific attributes.
 */
struct cmzn_graphics;
typedef struct cmzn_graphics * cmzn_graphics_id;

/**
 * The contours derived graphics type. Produces graphics visualising where the
 * 'iso-scalar' field equals fixed values: iso-surfaces on 3-D domains,
 * iso-lines on 2-D domains.
 */
struct cmzn_graphics_contours;
typedef struct cmzn_graphics_contours *cmzn_graphics_contours_id;

/**
 * Lines visualise 1-D elements in the model, including lines/faces of
 * higher-dimensional elements (if read in or defined via the field module).
 * These can be displayed as lines or extruded circles, as specified by the
 * graphics line attributes.
 * @see cmzn_graphicslineattributes_id
 */
struct cmzn_graphics_lines;
typedef struct cmzn_graphics_points *cmzn_graphics_lines_id;

/**
 * Points graphics visualise discrete locations in the model with oriented and
 * scaled glyphs and text labels as specified by the graphics point attributes.
 * Points can be generated on any field DomainType. For mesh domains, points are
 * sampled in elements according to the graphics sampling attributes and
 * tessellation. The single point domain is used to draw a single glyph such as
 * axes or colour bar (and is the only case not requiring a coordinate field as
 * it defaults to the origin).
 * @see cmzn_graphicspointattributes_id
 * @see cmzn_graphicssamplingattributes_id
 */
struct cmzn_graphics_points;
typedef struct cmzn_graphics_points *cmzn_graphics_points_id;

/**
 * Streamlines visualise the path of a fluid particle tracking along a vector
 * field. 2-D and 3-D mesh domains are supported. Seed points for streamlines
 * are sampled from elements according to the graphics sampling attributes and
 * tessellation. Streamlines are drawn as lines, scalable ribbons or extruded
 * circles or squares, as specified by the graphics line attributes. The curl of
 * the stream vector field, or fibre sheet and normal, are visualised by the
 * rotation or lateral orientation of the streamline when viewed with non-line
 * shapes.
 * @see cmzn_graphicssamplingattributes_id
 * @see cmzn_graphicslineattributes_id
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
 * Surfaces visualise 2-D elements in the model, including faces of 3-D
 * elements (if read in or defined via the field module).
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
 * Attributes object specifying how points are visualised in the graphics
 * including glyph, scaling, labels and font.
 * @see cmzn_graphics_get_graphicspointattributes
 */
struct cmzn_graphicspointattributes;
typedef struct cmzn_graphicspointattributes * cmzn_graphicspointattributes_id;

/**
 * Attributes object specifying how lines are visualised in the graphics
 * including shape/profile and scaling.
 * @see cmzn_graphics_get_graphicslineattributes
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
 * Graphics attributes object specifying how points are sampled in elements,
 * including sampling mode, density, etc.
 * @see cmzn_graphics_get_graphicssamplingattributes
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
