/**
 * @file scenecoordinatesystem.hpp
 *
 * Enumerated type for identifying scene and window coordinate systems.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_SCENECOORDINATESYSTEM_HPP__
#define CMZN_SCENECOORDINATESYSTEM_HPP__

#include "scenecoordinatesystem.h"

namespace OpenCMISS
{
namespace Zinc
{

/**
 * Enumerated type for identifying scene and window coordinate systems.
 */
enum Scenecoordinatesystem
{
	SCENECOORDINATESYSTEM_INVALID = CMZN_SCENECOORDINATESYSTEM_INVALID,
	SCENECOORDINATESYSTEM_LOCAL = CMZN_SCENECOORDINATESYSTEM_LOCAL,
	SCENECOORDINATESYSTEM_WORLD = CMZN_SCENECOORDINATESYSTEM_WORLD,
	SCENECOORDINATESYSTEM_NORMALISED_WINDOW_FILL = CMZN_SCENECOORDINATESYSTEM_NORMALISED_WINDOW_FILL,
	SCENECOORDINATESYSTEM_NORMALISED_WINDOW_FIT_CENTRE = CMZN_SCENECOORDINATESYSTEM_NORMALISED_WINDOW_FIT_CENTRE,
	SCENECOORDINATESYSTEM_NORMALISED_WINDOW_FIT_LEFT = CMZN_SCENECOORDINATESYSTEM_NORMALISED_WINDOW_FIT_LEFT,
	SCENECOORDINATESYSTEM_NORMALISED_WINDOW_FIT_RIGHT = CMZN_SCENECOORDINATESYSTEM_NORMALISED_WINDOW_FIT_RIGHT,
	SCENECOORDINATESYSTEM_NORMALISED_WINDOW_FIT_BOTTOM = CMZN_SCENECOORDINATESYSTEM_NORMALISED_WINDOW_FIT_BOTTOM,
	SCENECOORDINATESYSTEM_NORMALISED_WINDOW_FIT_TOP = CMZN_SCENECOORDINATESYSTEM_NORMALISED_WINDOW_FIT_TOP,
	SCENECOORDINATESYSTEM_WINDOW_PIXEL_BOTTOM_LEFT = CMZN_SCENECOORDINATESYSTEM_WINDOW_PIXEL_BOTTOM_LEFT,
	SCENECOORDINATESYSTEM_WINDOW_PIXEL_TOP_LEFT = CMZN_SCENECOORDINATESYSTEM_WINDOW_PIXEL_TOP_LEFT
};

inline Scenecoordinatesystem ScenecoordinatesystemEnumFromString(const char *name)
{
	return static_cast<Scenecoordinatesystem>(cmzn_scenecoordinatesystem_enum_from_string(name));
}

inline char *ScenecoordinatesystemEnumToString(Scenecoordinatesystem system)
{
	return cmzn_scenecoordinatesystem_enum_to_string(static_cast<cmzn_scenecoordinatesystem>(system));
}

} // namespace Zinc
}

#endif
