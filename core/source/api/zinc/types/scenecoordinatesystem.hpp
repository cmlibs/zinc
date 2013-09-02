/**
 * FILE : scenecoordinatesystem.hpp
 *
 * Enumerated type for identifying scene and window coordinate systems.
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

#ifndef CMZN_SCENECOORDINATESYSTEM_HPP__
#define CMZN_SCENECOORDINATESYSTEM_HPP__

#include "scenecoordinatesystem.h"

namespace zinc
{

/**
 * Enumerated type for identifying scene and window coordinate systems.
 */
enum SceneCoordinateSystem
{
	SCENE_COORDINATE_SYSTEM_INVALID = CMZN_SCENE_COORDINATE_SYSTEM_INVALID,
	SCENE_COORDINATE_SYSTEM_LOCAL = CMZN_SCENE_COORDINATE_SYSTEM_LOCAL,
	SCENE_COORDINATE_SYSTEM_WORLD = CMZN_SCENE_COORDINATE_SYSTEM_WORLD,
	SCENE_COORDINATE_SYSTEM_NORMALISED_WINDOW_FILL = CMZN_SCENE_COORDINATE_SYSTEM_NORMALISED_WINDOW_FILL,
	SCENE_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_CENTRE = CMZN_SCENE_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_CENTRE,
	SCENE_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_LEFT = CMZN_SCENE_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_LEFT,
	SCENE_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_RIGHT = CMZN_SCENE_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_RIGHT,
	SCENE_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_BOTTOM = CMZN_SCENE_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_BOTTOM,
	SCENE_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_TOP = CMZN_SCENE_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_TOP,
	SCENE_COORDINATE_SYSTEM_WINDOW_PIXEL_BOTTOM_LEFT = CMZN_SCENE_COORDINATE_SYSTEM_WINDOW_PIXEL_BOTTOM_LEFT,
	SCENE_COORDINATE_SYSTEM_WINDOW_PIXEL_TOP_LEFT = CMZN_SCENE_COORDINATE_SYSTEM_WINDOW_PIXEL_TOP_LEFT
};

} // namespace zinc

#endif
