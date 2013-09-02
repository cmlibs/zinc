/***************************************************************************//**
 * FILE : scenecoordinatesystem.h
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
 * Portions created by the Initial Developer are Copyright (C) 2011
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

#ifndef CMZN_SCENECOORDINATESYSTEM_H__
#define CMZN_SCENECOORDINATESYSTEM_H__


#include "zinc/zincsharedobject.h"

/***************************************************************************//**
 * Enumerated type for identifying scene and window coordinate systems.
 * cmzn_graphic objects are given in one of these coordinate systems.
 */
enum cmzn_scene_coordinate_system
{
	CMZN_SCENE_COORDINATE_SYSTEM_INVALID = 0,
	CMZN_SCENE_COORDINATE_SYSTEM_LOCAL = 1,
	/*!< The local coordinate system of a scene, subject to its own
	 * transformation matrix and those of all parent scenes up to the root
	 * region of the cmzn_scene in use, which are world coordinates.*/
	CMZN_SCENE_COORDINATE_SYSTEM_WORLD = 2,
	/*!< The world coordinate system which scene_viewer viewing parameters
	 * are specified in, and which scene transformations (giving local
	 * coordinates) are ultimately relative to.*/
	CMZN_SCENE_COORDINATE_SYSTEM_NORMALISED_WINDOW_FILL = 3,
	/*!< Distorted normalised window coordinate system which varies from
	 * -1 to +1 from left to right, bottom to top, and far to near of window.
	 * If window is non-square, graphics in this space appear stretched. */
	CMZN_SCENE_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_CENTRE = 4,
	/*!< Undistorted normalised window coordinate system which varies from
	 * -1 to +1 from far to near, and from -1 to +1 from left-to-right and
	 * bottom-to-top in largest square that fits in centre of window.*/
	CMZN_SCENE_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_LEFT = 5,
	/*!< Undistorted normalised window coordinate system which varies from
	 * -1 to +1 from far to near, and from -1 to +1 from left-to-right and
	 * bottom-to-top in largest square that fits in left of window.*/
	CMZN_SCENE_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_RIGHT = 6,
	/*!< Undistorted normalised window coordinate system which varies from
	 * -1 to +1 from far to near, and from -1 to +1 from left-to-right and
	 * bottom-to-top in largest square that fits in right of window.*/
	CMZN_SCENE_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_BOTTOM = 7,
	/*!< Undistorted normalised window coordinate system which varies from
	 * -1 to +1 from far to near, and from -1 to +1 from left-to-right and
	 * bottom-to-top in largest square that fits in bottom of window.*/
	CMZN_SCENE_COORDINATE_SYSTEM_NORMALISED_WINDOW_FIT_TOP = 8,
	/*!< Undistorted normalised window coordinate system which varies from
	 * -1 to +1 from far to near, and from -1 to +1 from left-to-right and
	 * bottom-to-top in largest square that fits in top of window.*/
	CMZN_SCENE_COORDINATE_SYSTEM_WINDOW_PIXEL_BOTTOM_LEFT = 9,
	/*!< Window coordinate system in pixel units with 0,0 at bottom, left of
	 * bottom-left pixel in display window, and depth ranging from far = -1 to
	 * near = +1.*/
	CMZN_SCENE_COORDINATE_SYSTEM_WINDOW_PIXEL_TOP_LEFT = 10
	/*!< Window coordinate system in pixel units with 0,0 at top, left of
	 * top-left pixel in display window, and depth ranging from far = -1 to
	 * near = +1. Y coordinates are negative going down the window. */
};

#ifdef __cplusplus
extern "C" {
#endif
/***************************************************************************//**
 * Convert a short name into an enum if the name matches any of the members in
 * the enum.
 *
 * @param string  string of the short enumerator name
 * @return  the correct enum type if a match is found.
 */
ZINC_API enum cmzn_scene_coordinate_system
	cmzn_scene_coordinate_system_enum_from_string(const char *string);

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call cmzn_deallocate to destroy the successfully returned string.
 *
 * @param system  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *cmzn_scene_coordinate_system_enum_to_string(
	enum cmzn_scene_coordinate_system system);

#ifdef __cplusplus
}
#endif

#endif
