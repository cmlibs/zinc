/*******************************************************************************
 * cmiss_scene.h
 * 
 * Public interface to the Cmiss_scene object which represents a set of graphics
 * able to be output to the Cmiss_scene_viewer or other outputs/devices.
 * It broadly comprises a reference to a region sub-tree and filters controlling
 * which graphics are displayed from its renditions.
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
 * Portions created by the Initial Developer are Copyright (C) 2010
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

#ifndef __CMISS_SCENE_H__
#define __CMISS_SCENE_H__

#include "api/cmiss_scene_filter.h"

struct Cmiss_scene;

#ifndef CMISS_SCENE_ID_DEFINED
/***************************************************************************//**
 * A handle to cmiss scene, cmiss scene contains a top region to display 
 * rendition of itself and all of its child regions, it also store a a 
 * collections of objects that make up a 3-D graphical model - lights, 
 * materials, primitives, etc. Also contains interface rouitines for having 
 * these converted to display lists and assembled into a single display list.
 * This single display list, however it is up to others - ie. the Scene_viewer
 * to display.
 */
   typedef struct Cmiss_scene * Cmiss_scene_id;
   #define CMISS_SCENE_ID_DEFINED
#endif /* CMISS_SCENE_ID_DEFINED */

struct Cmiss_region;

#ifndef CMISS_REGION_ID_DEFINED
   typedef struct Cmiss_region * Cmiss_region_id;
   #define CMISS_REGION_ID_DEFINED
#endif /* CMISS_REGION_ID_DEFINED */

/***************************************************************************//** 
 * Set the top region of the scene. Rendition from this top region and its child
 * region will be compiled into a display list for rendering.
 *
 * @param scene Handle to the scene to be edited
 * @param root_region Handle to the region to be set in the scene.
 * @return If successfully set region for scene returns 1, otherwise 0
 */
int Cmiss_scene_set_region(Cmiss_scene_id scene, Cmiss_region_id root_region);

/***************************************************************************//**
 * Return the name of the scene. 
 * 
 * @param scene  The scene whose name is requested.
 * @return  On success: allocated string containing scene name.
 */
char *Cmiss_scene_get_name(Cmiss_scene_id scene);

/***************************************************************************//**
 * Removes all filters from the scene. No graphics will be shown until a filter
 * showing graphics is created for the scene.
 *
 * @param scene  Scene to clear filters from.
 * @return  1 if filters successfully cleared, otherwise 0.
 */
int Cmiss_scene_clear_filters(Cmiss_scene_id scene);

/***************************************************************************//**
 * Creates a Cmiss_scene_filter which matches all graphics.
 * The new filter defaults to being active with action to show matched graphics.
 * Caller must call Cmiss_scene_filter_destroy to clean up the returned handle.
 *
 * @param scene  Scene to add filter to.
 * @return  Handle to the new filter, or NULL on failure.
 */
Cmiss_scene_filter_id Cmiss_scene_create_filter_all(Cmiss_scene_id scene);

/***************************************************************************//**
 * Creates a Cmiss_scene_filter which matches any graphic with the supplied
 * name. The new filter is placed last in the scene's filter priority list.
 * The new filter defaults to being active with action to show matched graphics.
 * Caller must call Cmiss_scene_filter_destroy to clean up the returned handle. 
 *
 * @param scene  Scene to add filter to.
 * @param match_name  The name a graphic must have to be matched by this filter.
 * @return  Handle to the new filter, or NULL on failure.
 */
Cmiss_scene_filter_id Cmiss_scene_create_filter_graphic_name(
	Cmiss_scene_id scene, const char *match_name);

/***************************************************************************//**
 * Creates a Cmiss_scene_filter which matches any graphic with visibility flag
 * set AND its owning rendition visibility flag set.
 * The new filter defaults to being active with action to show matched graphics.
 * Caller must call Cmiss_scene_filter_destroy to clean up the returned handle.
 * This filter is present and active in the default scene available through the
 * command interface.
 *
 * @param scene  Scene to add filter to.
 * @return  Handle to the new filter, or NULL on failure.
 */
Cmiss_scene_filter_id Cmiss_scene_create_filter_visibility_flags(
	Cmiss_scene_id scene);

#endif /*__CMISS_SCENE_H__*/
