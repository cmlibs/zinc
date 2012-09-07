/***************************************************************************//**
 * FILE: graphics_coordinate_system.hpp
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

#ifndef GRAPHICS_COORDINATE_SYSTEM_HPP
#define GRAPHICS_COORDINATE_SYSTEM_HPP

//-- extern "C" {
#include "api/types/cmiss_graphics_coordinate_system.h"
#include "general/enumerator.h"
//-- }

PROTOTYPE_ENUMERATOR_FUNCTIONS(Cmiss_graphics_coordinate_system);

/*******************************************************************************
 * Returns the orthographic viewport bounds for 'device' graphics coordinate
 * systems.
 *
 * @param coordinate_system  The graphics coordinate system. World, local and
 * other 'non-device' coordinate systems return error.
 * @param viewport_width  Viewport width in pixels.
 * @param viewport_height  Viewport height in pixels.
 * @param left  Address to return coordinate of left of left-most pixel.
 * @param right  Address to return coordinate of right of right-most pixel.
 * @param bottom  Address to return coordinate of bottom of bottom-most pixel.
 * @param top  Address to return coordinate of top of top-most pixel.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_graphics_coordinate_system_get_viewport(
	enum Cmiss_graphics_coordinate_system coordinate_system,
	double viewport_width, double viewport_height,
	double *left, double *right, double *bottom, double *top);

/*******************************************************************************
 * Returns true if the coordinate_system is window-relative, which currently
 * determines whether it is drawn as an overlay.
 *
 * @param coordinate_system  The graphics coordinate system.
 * @return  1 if window-relative, 0 if not.
 */
int Cmiss_graphics_coordinate_system_is_window_relative(
	enum Cmiss_graphics_coordinate_system coordinate_system);

#endif /* GRAPHICS_COORDINATE_SYSTEM_HPP */
