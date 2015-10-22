/**
 * @file lightid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_LIGHTID_H__
#define CMZN_LIGHTID_H__

/**
 * @brief Module managing all light objects.
 *
 * Module managing all light objects. It maintains separate default
 * lights for points and continuous graphics, the default points
 * light having only 1 point in each direction.
 */
struct cmzn_lightmodule;
typedef struct cmzn_lightmodule * cmzn_lightmodule_id;

/**
 * @brief The light controls how vertices will be lit on sceneviewer.
 *
 * The light controls how vertices will be lit on sceneviewer. Visuals
 * of the vertices can be affected by the light colours, direction, position
 * and the type of lighting.
 */
struct cmzn_light;
typedef struct cmzn_light * cmzn_light_id;

/**
 * @brief An iterator for looping through all the lights in a
 * light module.
 *
 * An iterator for looping through all the lights in a light
 * module.
 */
struct cmzn_lightiterator;
typedef struct cmzn_lightiterator * cmzn_lightiterator_id;

/**
 * Enumeration of render side for ambient light.
 * This only affects CMZN_LIGHT_TYPE_AMBIENT
 */
enum cmzn_light_render_side
{
	CMZN_LIGHT_RENDER_SIDE_INVALID = 0,
	/*!< Unspecified render side */
	CMZN_LIGHT_RENDER_SIDE_SINGLE = 1,
	/*!< Single side light will only lit faces with outward normal. */
	CMZN_LIGHT_RENDER_SIDE_DOUBLE = 2
	/*!< Double side light will lit faces with either inward or outward normal. */
};

/**
 * Enumeration of render viewer mode for ambient light.
 * This only affects CMZN_LIGHT_TYPE_AMBIENT
 */
enum cmzn_light_render_viewer_mode
/**
 * Enumeration of render viewer mode for ambient light.
 * CMZN_LIGHT_RENDER_VIEWER_MODE_LOCAL calculations take into account the
 * angle from the vertex to the viewer and are more accurate with some
 * speed penalty.
 * CMZN_LIGHT_RENDER_VIEWER_MODE_INFINITE is better on performance.
 */
{
	CMZN_LIGHT_RENDER_VIEWER_MODE_INVALID = 0,
	/*!< Unspecified viewer mode */
	CMZN_LIGHT_RENDER_VIEWER_MODE_LOCAL = 1,
	/*!< calculations take into account the
	 * angle from the vertex to the viewer and are more accurate with some
	 * speed penalty. */
	CMZN_LIGHT_RENDER_VIEWER_MODE_INFINITE = 2
	/*!< Faster but not as accurate as CMZN_LIGHT_RENDER_VIEWER_MODE_LOCAL */
};

/**
 * Enumeration of light type.
 * CMZN_LIGHT_TYPE_AMBIENT controls the ambient lighting of a scene, it is
 * known as light model on OpenGL.
 * CMZN_LIGHT_TYPE_SPOT light as position and direction,
 * CMZN_LIGHT_TYPE_DIRECTIONAL light has direction only (parallel light from a
 * source at infinity) and CMZN_LIGHT_TYPE_POINT light has position only.
*/
enum cmzn_light_type
{
	CMZN_LIGHT_TYPE_INVALID = 0,
	/*!< Unspecified type */
	CMZN_LIGHT_TYPE_AMBIENT = 1,
	/*!< controls the ambient lighting of a scene, it is
	 * known as light model on OpenGL. */
	CMZN_LIGHT_TYPE_DIRECTIONAL = 2,
	/*!< Directional only (parallel light from a source at infinity),
	 * it is formely known as infinite light. */
	CMZN_LIGHT_TYPE_POINT = 3,
	/*!< Positional light with no direction only. */
	CMZN_LIGHT_TYPE_SPOT = 4
	/*!< It takes position,direction and attenuation into account when
	 * calculating the final colour. */
};

#endif
